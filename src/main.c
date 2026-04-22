#define SDL_MAIN_HANDLED
#include "app.h"

int main(int argc, char *argv[]) {
    App app;

    if (!app_init(&app)) {
        return 1;
    }

    app_run(&app);

    app_shutdown(&app);

    return 0;
}