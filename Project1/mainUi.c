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
        Portfolio p = show_user_portfolio_menu();
        ReturnResult rr = calculateReturn(&p);
        ResultData* data = create_result_data(&p, &rr, p.start_date, p.end_date);
        export_json(data, p.start_date, p.end_date, &p);
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

//// ��¥ ��ȿ�� �˻� �Լ�
//int validate_date_format(const char* date) {
//    // yyyy/mm �������� üũ
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

// ��¥ ���ڿ��� tm ����ü�� ��ȯ�ϴ� �Լ�
void parse_date1(const char* date_str, struct tm* tm_date) {
    sscanf(date_str, "%4d/%2d", &tm_date->tm_year, &tm_date->tm_mon);
    tm_date->tm_year -= 1900;  // tm_year�� 1900�� �������� �ؾ� ��
    tm_date->tm_mon -= 1;      // ���� 0���� ����
    tm_date->tm_mday = 1;
}
void parse_date2(const char* date_str, struct tm* tm_date) {
    sscanf(date_str, "%4d/%2d", &tm_date->tm_year, &tm_date->tm_mon);
    tm_date->tm_year -= 1900;  // tm_year�� 1900�� �������� �ؾ� ��
    tm_date->tm_mon -= 1;      // ���� 0���� ����
    tm_date->tm_mday = 30;
}

// ��Ʈ������ �Է� �Լ�
Portfolio show_user_portfolio_menu() {
    Portfolio p = { 0, 0, {0}, {0.0}, 0, 0, {0}, {0} };
    printf("\n[����� ��Ʈ������ �Է� ȭ��]\n\n");

    char stock_names[50][32];
    int stock_count = get_all_stocks(stock_names, 50);
    if (stock_count <= 0) {
        printf("[����] ���� ������ �ҷ����� ���߽��ϴ�.\n");
        return p; // Portfolio�� ��ȯ
    }

    printf("\n[ ���� ����Ʈ ]\n\n");
    int columns = 3;

    for (int i = 0; i < stock_count; i++) {
        printf("%2d. %-32s", i + 1, stock_names[i]);
        if ((i + 1) % columns == 0 || i == stock_count - 1) {
            printf("\n");
        }
    }

    int count; // ���� ��
    printf("\n��Ʈ�������� ���� ���� �� (�ִ� 10��): ");
    scanf("%d", &count);
    if (count < 1 || count > MAX_STOCKS) {
        printf("�߸��� �Է��Դϴ�.\n");
        return p; // Portfolio�� ��ȯ
    }

    // ����ڰ� ������ ����� ����
    for (int i = 0; i < count; i++) {
        printf("%d��° ���� ��ȣ: ", i + 1);
        scanf("%d", &p.stocks[i]);
        if (p.stocks[i] < 1 || p.stocks[i] > stock_count) {
            printf("�߸��� ���� ��ȣ�Դϴ�.\n");
            return p; // Portfolio�� ��ȯ
        }
        printf("���� (0 ~ 1): ");
        scanf("%lf", &p.weights[i]);
    }

    // �ż� �ֱ�(frequency)
    printf("�ż� �ֱ� (1=����, 7=�ְ�, 30=����): ");
    scanf("%d", &p.frequency);
    if (p.frequency != 1 && p.frequency != 7 && p.frequency != 30) {
        printf("�߸��� �Է��Դϴ�.\n");
        return p; // Portfolio�� ��ȯ
    }

    // �ż� �ݾ�
    printf("�ż� �ݾ� (��): ");
    scanf("%d", &p.amount);

    char start_date[8], end_date[8];  // yyyy/mm -> �ִ� 7���� + null

    do {
        printf("������ (yyyy/mm): ");
        scanf("%7s", start_date);
        if (!validate_date_format(start_date)) {
            printf("�߸��� �����Դϴ�. ��: 2024/01\n");
        }
    } while (!validate_date_format(start_date));

    do {
        printf("������ (yyyy/mm): ");
        scanf("%7s", end_date);
        if (!validate_date_format(end_date)) {
            printf("�߸��� �����Դϴ�. ��: 2024/12\n");
        }
    } while (!validate_date_format(end_date));

    // ��¥ ó��
    parse_date1(start_date, &p.start_date);
    parse_date2(end_date, &p.end_date);
    
    printf("��¥ó�� ��?");

    // �Է��� ��Ʈ������ ��� ���
    printf("\n[�Է��� ��Ʈ������ ���]\n");
    for (int i = 0; i < count; i++) {
        printf("- ���� %d: %.0f%%\n", p.stocks[i], p.weights[i]*100);
    }
    printf("�ż� �ֱ�: %d�ϸ���\n", p.frequency);
    printf("�ż� �ݾ�: %d\n", p.amount);
    printf("������: %d/%02d\n", p.start_date.tm_year + 1900, p.start_date.tm_mon + 1);
    printf("������: %d/%02d\n\n", p.end_date.tm_year + 1900, p.end_date.tm_mon + 1);

    p.stock_count = count; // ���� �� ����

    return p;
}

void show_popular_stocks() {
    printf("\n[ �α� ���� Ȯ�� ]\n\n");

    char stock_names[50][32];
    int stock_counts[50];
    int count = get_popular_stocks(stock_names, stock_counts, 50);
    if (count <= 0) {
        printf("�α� ������ �ҷ����� ���߽��ϴ�.\n");
        return;
    }

    
    int columns = 2;

    for (int i = 0; i < count; i++) {
        printf("%2d. %-32s(%d) ", i + 1, stock_names[i], stock_counts[i]);

        if ((i + 1) % columns == 0 || i == count - 1) {
            printf("\n");
        }
    }

    // ���� 3�� ���
    printf("\n[ ���� �α� ���� Top 3 ]\n");
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
        printf("- %s (%dȸ)\n", stock_names[i], stock_counts[i]);
    }
}


void show_admin_menu() {
    int choice;

    while (1) {
        printf("\n[������ ���� ���� ȭ��]\n");
        printf("1. �α� ���� Ȯ��\n");
        printf("2. ���� �߰�\n");
        printf("3. ���� ����\n");
        printf("4. ���� ����\n");
        printf("0. �ڷΰ���\n");
        printf("����: ");
        scanf("%d", &choice);

        if (choice == 0) break;

        if (choice == 1) {
            char stock_names[50][32];
            int stock_counts[50];
            int count = get_popular_stocks(stock_names, stock_counts, 50);

            if (count <= 0) {
                printf("[����] �α� ���� ������ �ҷ����� ���߽��ϴ�.\n");
                continue;
            }

            printf("\n[ �α� ���� ����Ʈ ]\n\n");
            int columns = 2;

            for (int i = 0; i < count; i++) {
                printf("%2d. %-32s(%d) ", i + 1, stock_names[i], stock_counts[i]);

                if ((i + 1) % columns == 0 || i == count - 1) {
                    printf("\n");
                }
            }

            // ���� 3�� ���
            printf("\n[ ���� 3�� �α� ���� ]\n");
            for (int i = 0; i < 3 && i < count; i++) {
                printf("- %s (%dȸ)\n", stock_names[i], stock_counts[i]);
            }

        }
        else if (choice == 2) {
            char name[32];
            printf("\n[���� �߰�] �� ���� �̸� �Է� (���� ���� ����): ");
            getchar();  
            fgets(name, sizeof(name), stdin);

            size_t len = strlen(name);
            if (len > 0 && name[len - 1] == '\n') {
                name[len - 1] = '\0';
            }

            if (strlen(name) == 0) {
                printf("[����] ���� �̸��� ����� �� �����ϴ�.\n");
            }
            else if (insert_stock(name)) {
                printf("[�Ϸ�] ���� '%s' �߰� ����.\n", name);
            }
            else {
                printf("[����] ���� �߰� �� ������ �߻��߽��ϴ�.\n");
            }

        }
        else if (choice == 3) {
            int id;
            char new_name[32];
            printf("\n[���� ����] ������ ���� ID �Է�: ");
            scanf("%d", &id);

            getchar();  
            printf("�� ���� �̸� �Է� (���� ���� ����): ");
            fgets(new_name, sizeof(new_name), stdin);

            size_t len = strlen(new_name);
            if (len > 0 && new_name[len - 1] == '\n') {
                new_name[len - 1] = '\0';
            }

            if (strlen(new_name) == 0) {
                printf("[����] ���� �̸��� ����� �� �����ϴ�.\n");
            }
            else if (update_stock(id, new_name)) {
                printf("[�Ϸ�] ID %d�� ������ '%s'(��)�� �����Ǿ����ϴ�.\n", id, new_name);
            }
            else {
                printf("[����] ���� ���� �� ������ �߻��߽��ϴ�.\n");
            }
        }
        else if (choice == 4) {
            char stock_names[50][32];
            int stock_count = get_all_stocks(stock_names, 50);

            if (stock_count <= 0) {
                printf("[����] ���� ������ �ҷ����� ���߽��ϴ�.\n");
                continue;
            }

            printf("\n[ ���� ����Ʈ ]\n");
            for (int i = 0; i < stock_count; i++) {
                printf("%2d. %-32s\n", i + 1, stock_names[i]);
            }

            int id;
            printf("\n������ ���� ��ȣ: ");
            scanf("%d", &id);
            if (id < 1 || id > stock_count) {
                printf("�߸��� ��ȣ�Դϴ�.\n");
                continue;
            }

            if (delete_stock(id))
                printf("[�Ϸ�] ���� ���� ����.\n");
            else
                printf("[����] ���� �� ������ �߻��߽��ϴ�.\n");

        }
        else {
            printf("�߸��� �Է��Դϴ�. �ٽ� �õ����ּ���.\n");
        }
    }
}

