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
    double difference = difftime(end_time, start_time);
    return (int)(difference / (60 * 60 * 24));  // �� -> �� ��ȯ
}

// �־��� �ֱ�� ��¥ ������ �´� ���ͷ� ��� �Լ�
double calculateReturn(Portfolio* portfolio) {
    int days = calculate_days_between(portfolio->end_date, portfolio->start_date);
    int max_size = days / (portfolio->frequency);
    double weight = 0;

    // �Ⱓ �� ���� ������ �迭
    int* closing_prices = (int*)malloc(days * sizeof(int));
    if (closing_prices == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return -1.0; // ���� ó��
    }

    // end_date ���� ���� ������ �迭
    int* end_date_prices = (int*)malloc(portfolio->stock_count * sizeof(int));
    if (closing_prices == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return -1.0; // ���� ó��
    }

    // �Ⱓ �� ���ŷ� ������ �迭
    double** amounts = (double**)malloc(portfolio->stock_count * sizeof(double*));
    if (amounts == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return -1.0;
    }
    for (int i = 0; i < portfolio->stock_count; i++) {
        amounts[i] = (int*)malloc(max_size * sizeof(double));
        if (amounts[i] == NULL) {
            fprintf(stderr, "Memory allocation failed for row %d\n", i);
            // �̹� �Ҵ�� �޸� ����
            for (int j = 0; j < i; j++) {
                free(amounts[j]);
            }
            free(amounts);
            return -1.0;
        }
    }

    int num_result = 0;
    OCIEnv* envhp = NULL;
    OCIError* errhp = NULL;
    OCISvcCtx* svchp = NULL;
    OCISession* usrhp = NULL;
    OCIServer* srvhp = NULL;
    char* username = "C##CPJT";
    char* password = "1234";
    char* dbname = "192.168.31.101:1521/xe";

    connect_db(&envhp, &errhp, &svchp, &usrhp, &srvhp, username, password, dbname);

    for (int i = 0; i < portfolio->stock_count; i++) {
        // �Ⱓ �� ���� ��ȸ
        select_closing_price(envhp, svchp, errhp, portfolio->stocks[i], portfolio->start_date, portfolio->end_date, days, closing_prices, num_result);
        end_date_prices[i] = closing_prices[days - 1];
        weight = select_weight(envhp, svchp, errhp, portfolio->stocks[i], portfolio->id);
        double weighted_amount = (portfolio->amount) * weight;
        int k = 0;

        // �ֱ⿡ ���� �޶���
        // �ֱ⺰ ���� ���ŷ� ������ �迭
        for (int j = 0; j < max_size; j + (portfolio->frequency)) {
            amounts[i][k] = weighted_amount / closing_prices[j];
        }

    }

    disconnect_db(envhp, errhp, svchp, usrhp, srvhp);

    int total_amount = (portfolio->amount) * max_size;
    double total_return = 0.0;
    int total_holdings;
    for (int i = 0; i < (portfolio->stock_count); i++) {
        total_holdings = 0;
        for (int j = 0; j < max_size; j++) {
            total_holdings += amounts[i][j];
        }
        total_return += total_holdings * end_date_prices[i];
    }

    // �޸� ����
    free(closing_prices); 
    for (int i = 0; i < portfolio->stock_count; i++) {
        free(amounts[i]);
    }
    free(amounts);

    return total_return / total_amount * 100;

}

// ��Ʈ������ ���� ���� �Լ� (�հ谡 1�̾�� ��)
int validateWeights(Portfolio* p) {
    double sum = 0;

    for (int i = 0; i < p->stock_count; i++) {
        sum += p->weights[i];
    }

    if (sum != 1.0) {
        printf("��ü ������ �հ谡 100%�� �Ǿ�� �մϴ�.");
        return 0;
    }

    return 1;
}

int main() {
    Portfolio p = {}
}