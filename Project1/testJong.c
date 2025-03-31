#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include "mainUi.h"


void clear_screen() {
    system("cls");
}

const char* logo_lines[] = {
    " ____                                  ______                    __         ",
    "/\\  _`\\                               /\\__  _\\                  /\\ \\        ",
    "\\ \\ \\/\\ \\     __       ___    __  __  \\/_/\\ \\/     __     ___   \\ \\ \\___    ",
    " \\ \\ \\ \\ \\  /'__`\\    / __`\\ /\\ \\/\\ \\    \\ \\ \\   /'__`\\  /'___\\  \\ \\  _ `\\  ",
    "  \\ \\ \\_\\ \\/\\ \\L\\.\\_ /\\ \\L\\ \\\\ \\ \\_\\ \\    \\ \\ \\\\ \\  __/ /\\ \\__/   \\ \\ \\ \\ \\ ",
    "   \\ \\____/\\ \\__/\\.\\\\  \\____/ \\ \\____/     \\ \\_\\\\ \\____\\\\ \\____\\   \\ \\_\\ \\_\\",
    "    \\/___/  \\/__/\\/_/ \\/___/   \\/___/       \\/_/ \\/____/ \\/____/    \\/_/\\/_/"
};
#define LOGO_HEIGHT 7
#define LOGO_WIDTH  75

void print_frame(int phase) {
    system("cls");
    for (int i = 0; i < LOGO_HEIGHT; i++) {
        const char* line = logo_lines[i];
        int center = LOGO_WIDTH / 2;
        int offset = phase - center;

        for (int j = 0; j < LOGO_WIDTH; j++) {
            int mirrored = 2 * center - j;
            if (phase < center && j >= phase && j < LOGO_WIDTH - phase) {
                char ch = logo_lines[i][mirrored];
                putchar(ch == ' ' ? ' ' : '.');
            }
            else {
                putchar(line[j]);
            }
        }
        putchar('\n');
    }
}

void animate_spin_like_earth() {
    const int total_phases = LOGO_WIDTH;
    for (int loop = 0; loop < 1; loop++) {
        for (int phase = 0; phase < total_phases; phase++) {
            print_frame(phase);
            Sleep(10);
        }
    }
}
int main() {
    int running = 1;
    animate_spin_like_earth();

    while (running) {

        show_main_menu();

        int choice;
        scanf("%d", &choice);

        switch (choice) {
        case 1:
            show_login_menu();
            break;
        case 2:
            handle_register();
            break;
        case 0:
            printf("프로그램을 종료합니다.\n");
            running = 0;
            break;
        default:
            printf("잘못된 선택입니다. 다시 시도해주세요.\n\n");
        }
    }

    return 0;
}