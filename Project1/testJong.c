#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include "mainUi.h"

int main() {
    int running = 1;

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