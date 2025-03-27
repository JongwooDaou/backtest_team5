#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "mainUi.h"
#include "ociCRUD.h"

#define ADMIN_ID "1"
#define ADMIN_PW "1"

int validate_date_format(const char* date) {
    if (strlen(date) != 10) return 0;
    return isdigit(date[0]) && isdigit(date[1]) && isdigit(date[2]) && isdigit(date[3]) &&
        date[4] == '/' &&
        isdigit(date[5]) && isdigit(date[6]) &&
        date[7] == '/' &&
        isdigit(date[8]) && isdigit(date[9]);
}

void show_main_menu() {
    printf("========================\n");
    printf(" 1. �α���\n");
    printf(" 2. ȸ������\n");
    printf(" 0. ����\n");
    printf("========================\n");
    printf("����: ");
}

void show_login_menu() {
    char id[32], pw[32];
    printf("\n[ �α��� ȭ�� ]\n");
    printf("ID: "); scanf("%s", id);
    printf("PW: "); scanf("%s", pw);

    if (strcmp(id, ADMIN_ID) == 0 && strcmp(pw, ADMIN_PW) == 0) {
        printf("[������ �α��� ����]\n\n");
        show_admin_menu();
    }
    else if (login_user(id, pw)) {
        printf("[�Ϲ� ����� �α��� ����]\n\n");
        show_user_portfolio_menu();
    }
    else {
        printf("[�α��� ����: ID �Ǵ� ��й�ȣ�� �ùٸ��� �ʽ��ϴ�]\n\n");
    }
}

void handle_register() {
    char id[32], pw1[32], pw2[32];
    printf("\n[ ȸ������ ȭ�� ]\n");
    printf("ID: "); scanf("%s", id);
    printf("��й�ȣ: "); scanf("%s", pw1);
    printf("��й�ȣ Ȯ��: "); scanf("%s", pw2);

    if (strcmp(pw1, pw2) != 0) {
        printf("[����] ��й�ȣ�� ��ġ���� �ʽ��ϴ�.\n\n");
        return;
    }

    if (register_user(id, pw1)) {
        printf("[ȸ������ ����]\n\n");
    }
    else {
        printf("[ȸ������ ����] �̹� �����ϴ� ID�Դϴ�.\n\n");
    }
}

void show_user_portfolio_menu() {
    printf("\n[����� ��Ʈ������ �Է� ȭ��]\n\n");

    char stock_names[50][32];
    int stock_count = get_all_stocks(stock_names, 50);
    if (stock_count <= 0) {
        printf("[����] ���� ������ �ҷ����� ���߽��ϴ�.\n");
        return;
    }

    printf("\n[ ���� ����Ʈ ]\n\n");

    int columns = 3;

    for (int i = 0; i < stock_count; i++) {
        printf("%2d. %-32s", i + 1, stock_names[i]);
        
        if ((i + 1) % columns == 0 || i == stock_count - 1) {
            printf("\n");
        }
    }

    int count;
    printf("\n��Ʈ�������� ���� ���� �� (�ִ� 10��): ");
    scanf("%d", &count);
    if (count < 1 || count > 10) {
        printf("�߸��� �Է��Դϴ�.\n");
        return;
    }

    int selected[10] = { 0 };
    double weights[10] = { 0 };

    for (int i = 0; i < count; i++) {
        printf("%d��° ���� ��ȣ: ", i + 1);
        scanf("%d", &selected[i]);
        if (selected[i] < 1 || selected[i] > stock_count) {
            printf("�߸��� ���� ��ȣ�Դϴ�.\n");
            return;
        }
        printf("����(%%): ");
        scanf("%lf", &weights[i]);
    }

    int cycle;
    printf("�ż� �ֱ� (1=����, 7=�ְ�, 30=����): ");
    scanf("%d", &cycle);
    if (cycle != 1 && cycle != 7 && cycle != 30) {
        printf("�߸��� �Է��Դϴ�.\n");
        return;
    }

    double amount;
    printf("�ż� �ݾ� (��): ");
    scanf("%lf", &amount);

    char start_date[11], end_date[11];
    do {
        printf("������ (yyyy/mm/dd): ");
        scanf("%10s", start_date);
        if (!validate_date_format(start_date)) {
            printf("�߸��� �����Դϴ�. ��: 2024/01/01\n");
        }
    } while (!validate_date_format(start_date));

    do {
        printf("������ (yyyy/mm/dd): ");
        scanf("%10s", end_date);
        if (!validate_date_format(end_date)) {
            printf("�߸��� �����Դϴ�. ��: 2024/12/31\n");
        }
    } while (!validate_date_format(end_date));

    printf("\n[�Է��� ��Ʈ������ ���]\n");
    for (int i = 0; i < count; i++) {
        printf("- %s: %.2f%%\n", stock_names[selected[i] - 1], weights[i]);
    }
    printf("�ż� �ֱ�: %d�ϸ���\n", cycle);
    printf("�ż� �ݾ�: %.0f\n", amount);
    printf("������: %s\n", start_date);
    printf("������: %s\n\n", end_date);
}

void show_admin_menu() {
    printf("[������ ���� ���� ȭ��]\n");
    printf("1. ���� �߰�\n");
    printf("2. ���� ����\n");
    printf("3. ���� ����\n");
    printf("0. �ڷΰ���\n");
}