#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "mainUi.h"
#include "ociCRUD.h"
#include "calculation.h"
#include "export.h"

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
        Portfolio p = show_user_portfolio_menu();
        ReturnResult rr = calculateReturn(&p);
        ResultData* data = create_result_data(&p, &rr, p.start_date, p.end_date);
        export_json(data, p.start_date, p.end_date, &p);
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

//// 날짜 유효성 검사 함수
//int validate_date_format(const char* date) {
//    // yyyy/mm 형식인지 체크
//    if (strlen(date) != 7) return 0;
//    if (date[4] != '/') return 0;
//    for (int i = 0; i < 4; i++) {
//        if (date[i] < '0' || date[i] > '9') return 0;
//    }
//    for (int i = 5; i < 7; i++) {
//        if (date[i] < '0' || date[i] > '9') return 0;
//    }
//    return 1;
//}

// 날짜 문자열을 tm 구조체로 변환하는 함수
void parse_date1(const char* date_str, struct tm* tm_date) {
    sscanf(date_str, "%4d/%2d", &tm_date->tm_year, &tm_date->tm_mon);
    tm_date->tm_year -= 1900;  // tm_year는 1900을 기준으로 해야 함
    tm_date->tm_mon -= 1;      // 월은 0부터 시작
    tm_date->tm_mday = 1;
}
void parse_date2(const char* date_str, struct tm* tm_date) {
    sscanf(date_str, "%4d/%2d", &tm_date->tm_year, &tm_date->tm_mon);
    tm_date->tm_year -= 1900;  // tm_year는 1900을 기준으로 해야 함
    tm_date->tm_mon -= 1;      // 월은 0부터 시작
    tm_date->tm_mday = 30;
}

// 포트폴리오 입력 함수
Portfolio show_user_portfolio_menu() {
    Portfolio p = { 0, 0, {0}, {0.0}, 0, 0, {0}, {0} };
    printf("\n[사용자 포트폴리오 입력 화면]\n\n");

    char stock_names[50][32];
    int stock_count = get_all_stocks(stock_names, 50);
    if (stock_count <= 0) {
        printf("[오류] 종목 정보를 불러오지 못했습니다.\n");
        return p; // Portfolio를 반환
    }

    printf("\n[ 종목 리스트 ]\n\n");
    int columns = 3;

    for (int i = 0; i < stock_count; i++) {
        printf("%2d. %-32s", i + 1, stock_names[i]);
        if ((i + 1) % columns == 0 || i == stock_count - 1) {
            printf("\n");
        }
    }

    int count; // 종목 수
    printf("\n포트폴리오에 넣을 종목 수 (최대 10개): ");
    scanf("%d", &count);
    if (count < 1 || count > MAX_STOCKS) {
        printf("잘못된 입력입니다.\n");
        return p; // Portfolio를 반환
    }

    // 사용자가 선택한 종목과 비중
    for (int i = 0; i < count; i++) {
        printf("%d번째 종목 번호: ", i + 1);
        scanf("%d", &p.stocks[i]);
        if (p.stocks[i] < 1 || p.stocks[i] > stock_count) {
            printf("잘못된 종목 번호입니다.\n");
            return p; // Portfolio를 반환
        }
        printf("비중 (0 ~ 1): ");
        scanf("%lf", &p.weights[i]);
    }

    // 매수 주기(frequency)
    printf("매수 주기 (1=매일, 7=주간, 30=월간): ");
    scanf("%d", &p.frequency);
    if (p.frequency != 1 && p.frequency != 7 && p.frequency != 30) {
        printf("잘못된 입력입니다.\n");
        return p; // Portfolio를 반환
    }

    // 매수 금액
    printf("매수 금액 (원): ");
    scanf("%d", &p.amount);

    char start_date[8], end_date[8];  // yyyy/mm -> 최대 7글자 + null

    do {
        printf("시작일 (yyyy/mm): ");
        scanf("%7s", start_date);
        if (!validate_date_format(start_date)) {
            printf("잘못된 형식입니다. 예: 2024/01\n");
        }
    } while (!validate_date_format(start_date));

    do {
        printf("종료일 (yyyy/mm): ");
        scanf("%7s", end_date);
        if (!validate_date_format(end_date)) {
            printf("잘못된 형식입니다. 예: 2024/12\n");
        }
    } while (!validate_date_format(end_date));

    // 날짜 처리
    parse_date1(start_date, &p.start_date);
    parse_date2(end_date, &p.end_date);
    
    printf("날짜처리 완?");

    // 입력한 포트폴리오 요약 출력
    printf("\n[입력한 포트폴리오 요약]\n");
    for (int i = 0; i < count; i++) {
        printf("- 종목 %d: %.0f%%\n", p.stocks[i], p.weights[i]*100);
    }
    printf("매수 주기: %d일마다\n", p.frequency);
    printf("매수 금액: %d\n", p.amount);
    printf("시작일: %d/%02d\n", p.start_date.tm_year + 1900, p.start_date.tm_mon + 1);
    printf("종료일: %d/%02d\n\n", p.end_date.tm_year + 1900, p.end_date.tm_mon + 1);

    p.stock_count = count; // 종목 수 저장

    return p;
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

