#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "mainUi.h"
#include "ociCRUD.h"

#define ADMIN_ID "1"
#define ADMIN_PW "1"

void flush_stdin() {
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF);
}

int validate_date_format(const char* date) {
    if (strlen(date) != 7) return 0;
    return isdigit(date[0]) && isdigit(date[1]) && isdigit(date[2]) && isdigit(date[3]) &&
        date[4] == '/' &&
        isdigit(date[5]) && isdigit(date[6]);
}


void show_main_menu() {
    printf("========================\n");
    printf(" 1. 로그인\n");
    printf(" 2. 회원가입\n");
    printf(" 0. 종료\n");
    printf("========================\n");
    printf("선택: ");
}

void show_login_menu() {
    char id[32], pw[32];
    printf("\n[ 로그인 화면 ]\n");
    printf("ID: "); scanf("%s", id);
    printf("PW: "); scanf("%s", pw);

    if (strcmp(id, ADMIN_ID) == 0 && strcmp(pw, ADMIN_PW) == 0) {
        printf("[관리자 로그인 성공]\n\n");
        show_admin_menu();
    }
    else if (login_user(id, pw)) {
        printf("[일반 사용자 로그인 성공]\n\n");
        show_user_portfolio_menu();
    }
    else {
        printf("[로그인 실패: ID 또는 비밀번호가 올바르지 않습니다]\n\n");
    }
}

void handle_register() {
    char id[32], pw1[32], pw2[32];
    printf("\n[ 회원가입 화면 ]\n");
    printf("ID: "); scanf("%s", id);
    printf("비밀번호: "); scanf("%s", pw1);
    printf("비밀번호 확인: "); scanf("%s", pw2);

    if (strcmp(pw1, pw2) != 0) {
        printf("[실패] 비밀번호가 일치하지 않습니다.\n\n");
        return;
    }

    if (register_user(id, pw1)) {
        printf("[회원가입 성공]\n\n");
    }
    else {
        printf("[회원가입 실패] 이미 존재하는 ID입니다.\n\n");
    }
}

void show_user_portfolio_menu() {
    printf("\n[사용자 포트폴리오 입력 화면]\n\n");

    char stock_names[50][32];
    int stock_count = get_all_stocks(stock_names, 50);
    if (stock_count <= 0) {
        printf("[오류] 종목 정보를 불러오지 못했습니다.\n");
        return;
    }

    printf("\n[ 종목 리스트 ]\n\n");

    int columns = 3;

    for (int i = 0; i < stock_count; i++) {
        printf("%2d. %-32s", i + 1, stock_names[i]);
        
        if ((i + 1) % columns == 0 || i == stock_count - 1) {
            printf("\n");
        }
    }

    int count;
    printf("\n포트폴리오에 넣을 종목 수 (최대 10개): ");
    scanf("%d", &count);
    if (count < 1 || count > 10) {
        printf("잘못된 입력입니다.\n");
        return;
    }

    int selected[10] = { 0 };
    double weights[10] = { 0 };

    for (int i = 0; i < count; i++) {
        printf("%d번째 종목 번호: ", i + 1);
        scanf("%d", &selected[i]);
        if (selected[i] < 1 || selected[i] > stock_count) {
            printf("잘못된 종목 번호입니다.\n");
            return;
        }
        printf("비중(%%): ");
        scanf("%lf", &weights[i]);
    }

    int cycle;
    printf("매수 주기 (1=매일, 7=주간, 30=월간): ");
    scanf("%d", &cycle);
    if (cycle != 1 && cycle != 7 && cycle != 30) {
        printf("잘못된 입력입니다.\n");
        return;
    }

    double amount;
    printf("매수 금액 (원): ");
    scanf("%lf", &amount);

    char start_date[8], end_date[8];  // yyyy/mm -> 최대 7글자 + null

    do {
        printf("시작일 (yyyy/mm): ");
        scanf("%7s", start_date);
        flush_stdin();  // 버퍼 비우기
        if (!validate_date_format(start_date)) {
            printf("잘못된 형식입니다. 예: 2024/01\n");
        }
    } while (!validate_date_format(start_date));

    do {
        printf("종료일 (yyyy/mm): ");
        scanf("%7s", end_date);
        flush_stdin();  // 버퍼 비우기
        if (!validate_date_format(end_date)) {
            printf("잘못된 형식입니다. 예: 2024/12\n");
        }
    } while (!validate_date_format(end_date));

    printf("\n[입력한 포트폴리오 요약]\n");
    for (int i = 0; i < count; i++) {
        printf("- %s: %.2f%%\n", stock_names[selected[i] - 1], weights[i]);
    }
    printf("매수 주기: %d일마다\n", cycle);
    printf("매수 금액: %.0f\n", amount);
    printf("시작일: %s\n", start_date);
    printf("종료일: %s\n\n", end_date);
}

void show_popular_stocks() {
    printf("\n[ 인기 종목 확인 ]\n\n");

    char stock_names[50][32];
    int stock_counts[50];
    int count = get_popular_stocks(stock_names, stock_counts, 50);
    if (count <= 0) {
        printf("인기 종목을 불러오지 못했습니다.\n");
        return;
    }

    
    int columns = 2;

    for (int i = 0; i < count; i++) {
        printf("%2d. %-32s(%d) ", i + 1, stock_names[i], stock_counts[i]);

        if ((i + 1) % columns == 0 || i == count - 1) {
            printf("\n");
        }
    }

    // 상위 3개 출력
    printf("\n[ 상위 인기 종목 Top 3 ]\n");
    for (int i = 0; i < 3 && i < count; i++) {
        int max_idx = i;
        for (int j = i + 1; j < count; j++) {
            if (stock_counts[j] > stock_counts[max_idx])
                max_idx = j;
        }
        if (i != max_idx) {
            int tmp_count = stock_counts[i];
            stock_counts[i] = stock_counts[max_idx];
            stock_counts[max_idx] = tmp_count;

            char tmp_name[32];
            strcpy(tmp_name, stock_names[i]);
            strcpy(stock_names[i], stock_names[max_idx]);
            strcpy(stock_names[max_idx], tmp_name);
        }
        printf("- %s (%d회)\n", stock_names[i], stock_counts[i]);
    }
}


void show_admin_menu() {
    int choice;

    while (1) {
        printf("\n[관리자 종목 관리 화면]\n");
        printf("1. 인기 종목 확인\n");
        printf("2. 종목 추가\n");
        printf("3. 종목 수정\n");
        printf("4. 종목 삭제\n");
        printf("0. 뒤로가기\n");
        printf("선택: ");
        scanf("%d", &choice);

        if (choice == 0) break;

        if (choice == 1) {
            char stock_names[50][32];
            int stock_counts[50];
            int count = get_popular_stocks(stock_names, stock_counts, 50);

            if (count <= 0) {
                printf("[오류] 인기 종목 정보를 불러오지 못했습니다.\n");
                continue;
            }

            printf("\n[ 인기 종목 리스트 ]\n\n");
            int columns = 2;

            for (int i = 0; i < count; i++) {
                printf("%2d. %-32s(%d) ", i + 1, stock_names[i], stock_counts[i]);

                if ((i + 1) % columns == 0 || i == count - 1) {
                    printf("\n");
                }
            }

            // 상위 3개 출력
            printf("\n[ 상위 3개 인기 종목 ]\n");
            for (int i = 0; i < 3 && i < count; i++) {
                printf("- %s (%d회)\n", stock_names[i], stock_counts[i]);
            }

        }
        else if (choice == 2) {
            char name[32];
            printf("\n[종목 추가] 새 종목 이름 입력 (공백 포함 가능): ");
            getchar();  
            fgets(name, sizeof(name), stdin);

            size_t len = strlen(name);
            if (len > 0 && name[len - 1] == '\n') {
                name[len - 1] = '\0';
            }

            if (strlen(name) == 0) {
                printf("[실패] 종목 이름은 비워둘 수 없습니다.\n");
            }
            else if (insert_stock(name)) {
                printf("[완료] 종목 '%s' 추가 성공.\n", name);
            }
            else {
                printf("[실패] 종목 추가 중 오류가 발생했습니다.\n");
            }

        }
        else if (choice == 3) {
            int id;
            char new_name[32];
            printf("\n[종목 수정] 수정할 종목 ID 입력: ");
            scanf("%d", &id);

            getchar();  
            printf("새 종목 이름 입력 (공백 포함 가능): ");
            fgets(new_name, sizeof(new_name), stdin);

            size_t len = strlen(new_name);
            if (len > 0 && new_name[len - 1] == '\n') {
                new_name[len - 1] = '\0';
            }

            if (strlen(new_name) == 0) {
                printf("[실패] 종목 이름은 비워둘 수 없습니다.\n");
            }
            else if (update_stock(id, new_name)) {
                printf("[완료] ID %d의 종목이 '%s'(으)로 수정되었습니다.\n", id, new_name);
            }
            else {
                printf("[실패] 종목 수정 중 오류가 발생했습니다.\n");
            }
        }
        else if (choice == 4) {
            char stock_names[50][32];
            int stock_count = get_all_stocks(stock_names, 50);

            if (stock_count <= 0) {
                printf("[오류] 종목 정보를 불러오지 못했습니다.\n");
                continue;
            }

            printf("\n[ 종목 리스트 ]\n");
            for (int i = 0; i < stock_count; i++) {
                printf("%2d. %-32s\n", i + 1, stock_names[i]);
            }

            int id;
            printf("\n삭제할 종목 번호: ");
            scanf("%d", &id);
            if (id < 1 || id > stock_count) {
                printf("잘못된 번호입니다.\n");
                continue;
            }

            if (delete_stock(id))
                printf("[완료] 종목 삭제 성공.\n");
            else
                printf("[실패] 삭제 중 오류가 발생했습니다.\n");

        }
        else {
            printf("잘못된 입력입니다. 다시 시도해주세요.\n");
        }
    }
}

