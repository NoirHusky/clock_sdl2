#include "SDL_render.h"
#include <SDL2/SDL.h>
#include <math.h>
#include <stdbool.h>

const char TITLE[] = "The Clock";
const int WIDTH = 420;
const int HEIGHT = 420;

SDL_Window *g_window;
SDL_Renderer *g_renderer;
bool g_running = false;

SDL_FPoint vector_rotate(float x, float y, float angle) {
    SDL_FPoint p;
    float theta = angle * (M_PI / 180.0);

    float cs = cosf(theta);
    float sn = sinf(theta);

    p.x = x * cs - y * sn;
    p.y = x * sn + y * cs;
    return p;
}

int pixel(SDL_Renderer *renderer, Sint16 x, Sint16 y) {
    return SDL_RenderDrawPoint(renderer, x, y);
}
int hline(SDL_Renderer *renderer, Sint16 x1, Sint16 x2, Sint16 y) {
    return SDL_RenderDrawLine(renderer, x1, y, x2, y);
    ;
}
int vline(SDL_Renderer *renderer, Sint16 x, Sint16 y1, Sint16 y2) {
    return SDL_RenderDrawLine(renderer, x, y1, x, y2);
    ;
}

int _drawQuadrants(SDL_Renderer *renderer, Sint16 x, Sint16 y, Sint16 dx,
                   Sint16 dy, Sint32 f) {
    int result = 0;
    Sint16 xpdx, xmdx;
    Sint16 ypdy, ymdy;

    if (dx == 0) {
        if (dy == 0) {
            result |= pixel(renderer, x, y);
        } else {
            ypdy = y + dy;
            ymdy = y - dy;
            if (f) {
                result |= vline(renderer, x, ymdy, ypdy);
            } else {
                result |= pixel(renderer, x, ypdy);
                result |= pixel(renderer, x, ymdy);
            }
        }
    } else {
        xpdx = x + dx;
        xmdx = x - dx;
        ypdy = y + dy;
        ymdy = y - dy;
        if (f) {
            result |= vline(renderer, xpdx, ymdy, ypdy);
            result |= vline(renderer, xmdx, ymdy, ypdy);
        } else {
            result |= pixel(renderer, xpdx, ypdy);
            result |= pixel(renderer, xmdx, ypdy);
            result |= pixel(renderer, xpdx, ymdy);
            result |= pixel(renderer, xmdx, ymdy);
        }
    }

    return result;
}

#define ELLIPSE_OVERSCAN 4
int _ellipseRGBA(SDL_Renderer *renderer, Sint16 x, Sint16 y, Sint16 rx,
                 Sint16 ry, Uint8 r, Uint8 g, Uint8 b, Uint8 a, Sint32 f) {
    int result;
    Sint32 rx2, ry2, rx22, ry22;
    Sint32 error;
    Sint32 curX, curY, curXp1, curYm1;
    Sint32 scrX, scrY, oldX, oldY;
    Sint32 deltaX, deltaY;

    /*
     * Sanity check radii
     */
    if ((rx < 0) || (ry < 0)) {
        return (-1);
    }

    /*
     * Set color
     */
    result = 0;
    result |= SDL_SetRenderDrawBlendMode(
        renderer, (a == 255) ? SDL_BLENDMODE_NONE : SDL_BLENDMODE_BLEND);
    result |= SDL_SetRenderDrawColor(renderer, r, g, b, a);

    /*
     * Special cases for rx=0 and/or ry=0: draw a hline/vline/pixel
     */
    if (rx == 0) {
        if (ry == 0) {
            return (pixel(renderer, x, y));
        } else {
            return (vline(renderer, x, y - ry, y + ry));
        }
    } else {
        if (ry == 0) {
            return (hline(renderer, x - rx, x + rx, y));
        }
    }

    /*
     * Top/bottom center points.
     */
    oldX = scrX = 0;
    oldY = scrY = ry;
    result |= _drawQuadrants(renderer, x, y, 0, ry, f);

    /* Midpoint ellipse algorithm with overdraw */
    rx *= ELLIPSE_OVERSCAN;
    ry *= ELLIPSE_OVERSCAN;
    rx2 = rx * rx;
    rx22 = rx2 + rx2;
    ry2 = ry * ry;
    ry22 = ry2 + ry2;
    curX = 0;
    curY = ry;
    deltaX = 0;
    deltaY = rx22 * curY;

    /* Points in segment 1 */
    error = ry2 - rx2 * ry + rx2 / 4;
    while (deltaX <= deltaY) {
        curX++;
        deltaX += ry22;

        error += deltaX + ry2;
        if (error >= 0) {
            curY--;
            deltaY -= rx22;
            error -= deltaY;
        }

        scrX = curX / ELLIPSE_OVERSCAN;
        scrY = curY / ELLIPSE_OVERSCAN;
        if ((scrX != oldX && scrY == oldY) || (scrX != oldX && scrY != oldY)) {
            result |= _drawQuadrants(renderer, x, y, scrX, scrY, f);
            oldX = scrX;
            oldY = scrY;
        }
    }

    /* Points in segment 2 */
    if (curY > 0) {
        curXp1 = curX + 1;
        curYm1 = curY - 1;
        error = ry2 * curX * curXp1 + ((ry2 + 3) / 4) + rx2 * curYm1 * curYm1 -
                rx2 * ry2;
        while (curY > 0) {
            curY--;
            deltaY -= rx22;

            error += rx2;
            error -= deltaY;

            if (error <= 0) {
                curX++;
                deltaX += ry22;
                error += deltaX;
            }

            scrX = curX / ELLIPSE_OVERSCAN;
            scrY = curY / ELLIPSE_OVERSCAN;
            if ((scrX != oldX && scrY == oldY) ||
                (scrX != oldX && scrY != oldY)) {
                oldY--;
                for (; oldY >= scrY; oldY--) {
                    result |= _drawQuadrants(renderer, x, y, scrX, oldY, f);
                    /* prevent overdraw */
                    if (f) {
                        oldY = scrY - 1;
                    }
                }
                oldX = scrX;
                oldY = scrY;
            }
        }

        /* Remaining points in vertical */
        if (!f) {
            oldY--;
            for (; oldY >= 0; oldY--) {
                result |= _drawQuadrants(renderer, x, y, scrX, oldY, f);
            }
        }
    }

    return (result);
}

int filledCircleRGBA(SDL_Renderer *renderer, Sint16 x, Sint16 y, Sint16 rad,
                     Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    return _ellipseRGBA(renderer, x, y, rad, rad, r, g, b, a, 1);
}

void draw_clock() {
    // draw the circle
    filledCircleRGBA(g_renderer, WIDTH / 2, HEIGHT / 2, WIDTH / 2.f * 0.9f, 255,
                     255, 255, 255);
    filledCircleRGBA(g_renderer, WIDTH / 2, HEIGHT / 2, WIDTH / 2.f * 0.1f, 0,
                     0, 0, 255);
    // draw the piecies left and right and top and bottom
    SDL_Rect r_top, r_bot, r_rgt, r_lft;
    r_top.w = 10;
    r_top.h = 50;
    r_top.x = WIDTH / 2 - r_top.w / 2;
    r_top.y = 10;
    SDL_RenderDrawRect(g_renderer, &r_top);

    r_bot.w = 10;
    r_bot.h = 50;
    r_bot.x = WIDTH / 2 - r_bot.w / 2;
    r_bot.y = HEIGHT - 10 - r_bot.h;
    SDL_RenderDrawRect(g_renderer, &r_bot);

    r_rgt.w = 50;
    r_rgt.h = 10;
    r_rgt.x = WIDTH - r_rgt.w - 10;
    r_rgt.y = HEIGHT / 2;
    SDL_RenderDrawRect(g_renderer, &r_rgt);

    r_lft.w = 50;
    r_lft.h = 10;
    r_lft.x = 10;
    r_lft.y = HEIGHT / 2;
    SDL_RenderDrawRect(g_renderer, &r_lft);

    // draw the hands of the clock
    SDL_Rect sec_hand;
    sec_hand.w = 10;
    sec_hand.h = 70;
    sec_hand.x = WIDTH / 2 - sec_hand.w / 2;
    sec_hand.y = HEIGHT / 2 - (sec_hand.h + 50);
    SDL_RenderDrawRect(g_renderer, &sec_hand);

    SDL_Rect min_hand;
    min_hand.w = 10;
    min_hand.h = 70;
    min_hand.x = WIDTH / 2 - min_hand.w / 2;
    min_hand.y = HEIGHT / 2 - (min_hand.h + 50);
    SDL_RenderFillRect(g_renderer, &min_hand);

    SDL_Rect hour_hand;
    hour_hand.w = 10;
    hour_hand.h = 50;
    hour_hand.x = WIDTH / 2 - hour_hand.w / 2;
    hour_hand.y = HEIGHT / 2 - (hour_hand.h + 50);
    SDL_RenderDrawRect(g_renderer, &hour_hand);
}

void update(float dt) {}

void render() {
    SDL_SetRenderDrawColor(g_renderer, 0, 0, 0, 255);
    SDL_RenderClear(g_renderer);
    // TODO:
    draw_clock();
    SDL_RenderPresent(g_renderer);
}

void app_close() {
    SDL_DestroyWindow(g_window);
    SDL_DestroyRenderer(g_renderer);
    SDL_Quit();
}

void handle_events() {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        switch (e.type) {
        case SDL_QUIT:
            g_running = false;
            break;
        }
    }
}
bool init(const char *title, int w, int h, bool fullscreen) {
    int flags = fullscreen ? SDL_WINDOW_FULLSCREEN : 0;
    SDL_CreateWindowAndRenderer(w, h, flags, &g_window, &g_renderer);
    if (!g_window || !g_renderer) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

void die(const char *err) {
    printf("ERROR: an error happened -> %s\n", err);
    exit(1);
}

int intialization_failed() {
    printf("ERROR: an error happened -> %s\n", SDL_GetError());
    return 1;
}

int run() {
    atexit(app_close);
    g_running = true;

    const Uint32 FPS = 1000 / 30;
    float dt = 0;

    while (g_running) {
        Uint32 start = SDL_GetTicks();

        update(dt);
        render();
        handle_events();

        Uint32 frametime = SDL_GetTicks() - start;
        if (frametime < FPS)
            SDL_Delay(FPS - frametime);
        dt = (SDL_GetTicks() - start) / 1000.f;
    }
    return 0;
}

int main() {
    return init(TITLE, WIDTH, HEIGHT, false) == EXIT_SUCCESS
               ? run()
               : intialization_failed();
}
