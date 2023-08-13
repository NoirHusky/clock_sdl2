#include "SDL_render.h"
#include <SDL2/SDL.h>
#include <stdbool.h>
#include <math.h>

const char TITLE[] = "The Clock";
const int WIDTH = 420;
const int HEIGHT = 420;

SDL_Window* g_window;
SDL_Renderer* g_renderer;
bool g_running = false;

/* struct tvector { */
/*     SDL_Point** data; */
/*     int len; */
/*     int cap; */
/* }; */
/* typedef struct tvector vector; */

/* vector* vector_new(int size) { */
/*     vector* v = malloc(sizeof(vector)); */
/*     v->data = malloc(sizeof(SDL_Point*) * size); */
/*     v->len = 0; */
/*     v->cap = size; */
/*     return v; */
/* } */
/* void vector_resize(vector* v) { } */
/* void vector_pushback(vector* v, SDL_Point* data) { */
/*     if ( v->len > v->cap ) { */
/*         vector_resize(v); */
/*     } */
/*     v->data[v->len] = data; */
/*     v->len++; */
/* } */

/* SDL_Point* vector_at(vector* v, int at) { */
/*     return v->data[at]; */
/* } */

SDL_FPoint vector_rotate(float x, float y, float angle) {
    SDL_FPoint p;
    float theta = angle * (M_PI / 180.0);

    float cs = cosf(theta);
    float sn = sinf(theta);

    p.x = x * cs - y * sn;
    p.y = x * sn + y * cs;
    return p;
}


void draw_clock() {
    /* SDL_Point points[20]; */

    /* for ( int i = 0; i < 20; i++ ) { */
    /*     SDL_FPoint p = vector_rotate(0, 1, 10*i); */
    /*     points[i].x = p.x * 100; */
    /*     points[i].y = p.y * 100; */
    /* } */
    /* SDL_RenderDrawLines(g_renderer, points, 19); */
}

void update(float dt) {

}

void render() {
    SDL_SetRenderDrawColor(g_renderer, 255, 150, 150, 255);
    SDL_RenderClear(g_renderer);
    // TODO: do your logic here
    /* draw_clock(); */
    SDL_RenderPresent(g_renderer);
}

void close() {
    SDL_DestroyWindow(g_window);
    SDL_DestroyRenderer(g_renderer);
    SDL_Quit();
}

void handle_events() {
    SDL_Event e;
    while ( SDL_PollEvent(&e)) {
        switch (e.type) {
            case SDL_QUIT:
                g_running = false;
                break;
        }
    }
}
bool init(const char* title, int w, int h, bool fullscreen) {
    int flags = fullscreen ? SDL_WINDOW_FULLSCREEN : 0;
    SDL_CreateWindowAndRenderer(w, h, flags, &g_window, &g_renderer);
    if ( !g_window || !g_renderer )  {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

void die(const char* err) {
    printf("ERROR: an error happened -> %s\n", err);
    exit(1);
}

int intialization_failed() {
    printf("ERROR: an error happened -> %s\n", SDL_GetError());
    return 1;
}

int run() {
    atexit(close);
    g_running = true;

    const Uint32 FPS = 1000 / 30;
    float dt = 0;

    while ( g_running ) {
        Uint32 start = SDL_GetTicks();

        update(dt);
        render();
        handle_events();

        Uint32 frametime = SDL_GetTicks() - start;
        if ( frametime < FPS ) SDL_Delay(FPS - frametime);
        dt = (SDL_GetTicks() - start) / 1000.f;
    }
    return 0;
}

int main() {
    return init(TITLE, WIDTH, HEIGHT, false) == EXIT_SUCCESS ? run() : intialization_failed();
}
