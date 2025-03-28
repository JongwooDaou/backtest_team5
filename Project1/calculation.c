#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "calculation.h"
#include "database.h"

// ��¥ �� �Լ� (�� ��¥�� �������� Ȯ��)
int compare_dates(struct tm* date1, struct tm* date2) {
    if (date1->tm_year == date2->tm_year &&
        date1->tm_mon == date2->tm_mon &&
        date1->tm_mday == date2->tm_mday) {
        return 1;
    }
    return 0;
}

int calculate_days_between(struct tm start, struct tm end) {
    // struct tm�� time_t�� ��ȯ
    time_t start_time = mktime(&start);
    time_t end_time = mktime(&end);

    if (start_time == -1 || end_time == -1) {
        fprintf(stderr, "Error converting time.\n");
        return -1;
    }

    // �� ���� ���̸� �� ������ ��ȯ
    double difference = difftime(start_time, end_time);
    return (int)(difference / (60 * 60 * 24));  // �� -> �� ��ȯ
}

// �־��� �ֱ�� ��¥ ������ �´� ���ͷ� ��� �Լ�
double calculateReturn(Portfolio* portfolio) {
    OCIEnv* envhp = NULL;
    OCIError* errhp = NULL;
    OCISvcCtx* svchp = NULL;
    OCISession* usrhp = NULL;
    OCIServer* srvhp = NULL;
    char* username = "C##CPJT";
    char* password = "1234";
    char* dbname = "192.168.31.101:1521/xe";

    connect_db(&envhp, &errhp, &svchp, &usrhp, &srvhp, username, password, dbname);

    int days = count_closing_prices(envhp, svchp, errhp, portfolio->stocks[0], portfolio->start_date, portfolio->end_date);
    printf("days: %d\n", days);
    int max_size = days / (portfolio->frequency);
    printf("max_size: %d\n", max_size);
    double weight = 0;

    // �Ⱓ �� ���� ������ �迭
    int* closing_prices = (int*)malloc(days * sizeof(int));
    if (closing_prices == NULL) {
        fprintf(stderr, "Memory allocation failed: closing_prices\n");
        return -1.0;
    }

    // end_date ���� ���� ������ �迭
    int* end_date_prices = (int*)malloc(portfolio->stock_count * sizeof(int));
    if (end_date_prices == NULL) {
        fprintf(stderr, "Memory allocation failed: end_date_prices\n");
        free(closing_prices);
        return -1.0;
    }


    // �Ⱓ �� ���ŷ� ������ �迭 (������ �迭)
    double** amounts = (double**)malloc(portfolio->stock_count * sizeof(double*));
    if (amounts == NULL) {
        fprintf(stderr, "Memory allocation failed: amounts\n");
        free(closing_prices);
        free(end_date_prices);
        return -1.0;
    }
    for (int i = 0; i < portfolio->stock_count; i++) {
        amounts[i] = (double*)malloc(max_size * sizeof(double));
        if (amounts[i] == NULL) {
            fprintf(stderr, "Memory allocation failed: amount[%d]\n", i);
            for (int j = 0; j < i; j++) free(amounts[j]);
            free(amounts);
            free(closing_prices);
            free(end_date_prices);
            return -1.0;
        }
    }


    for (int i = 0; i < portfolio->stock_count; i++) {
        // �Ⱓ �� ���� ��ȸ
        select_closing_price(envhp, svchp, errhp, portfolio->stocks[i], portfolio->start_date, portfolio->end_date, days, closing_prices);
        end_date_prices[i] = closing_prices[days - 1];
        weight = select_weight(envhp, svchp, errhp, portfolio->stocks[i], portfolio->id);
        double weighted_amount = (portfolio->amount) * weight;

        int k = 0;
        for (int j = 0; j < days; j += portfolio->frequency) {
            printf("weighted_amount: %.2lf\n", weighted_amount);
            printf("%d�� ������ %d��° ����: %d\n", i+1, j+1, closing_prices[j]);
            double temp = weighted_amount / closing_prices[j];
            printf("���ŷ�: %.2lf\n", temp);
            amounts[i][k] = temp;
            printf("%d�� %d��° amount ��: %.2lf\n", i + 1, k + 1, amounts[i][k]);
            k++;
        }
    }
    free(closing_prices);

    disconnect_db(envhp, errhp, svchp, usrhp, srvhp);

    int total_amount = (portfolio->amount) * max_size;
    double total_return = 0.0;
    double total_holdings = 0.0; // ������ �κ�

    for (int i = 0; i < (portfolio->stock_count); i++) {
        total_holdings = 0.0;
        for (int j = 0; j < max_size; j++) {
            total_holdings += amounts[i][j];
        }
        total_return += total_holdings * end_date_prices[i];
    }

    // �޸� ����
    free(end_date_prices);

    for (int i = 0; i < portfolio->stock_count; i++) {
        free(amounts[i]);
    }
    free(amounts);

    return total_return / total_amount * 100;
}


int main() {
    Portfolio p = {
         .id = 999,
         .stock_count = 3,
         .stocks = {1, 2, 3},
         .weights = {0.2, 0.3, 0.5},
         .frequency = 7,
         .amount = 1000000
    };

    // ���� ��¥ (2022-01-01)
    p.start_date.tm_year = 2023 - 1900;
    p.start_date.tm_mon = 0;
    p.start_date.tm_mday = 1;

    // ���� ��¥ (2023-12-31)
    p.end_date.tm_year = 2023 - 1900;
    p.end_date.tm_mon = 11;
    p.end_date.tm_mday = 31;

    double total_return = calculateReturn(&p);
    printf("�� ���ͷ�: %.2lf%%\n", total_return);

    return 0;
}