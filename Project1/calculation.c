#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "calculation.h"

// ��¥ �� �Լ� (�� ��¥�� �������� Ȯ��)
int compare_dates(struct tm* date1, struct tm* date2) {
    if (date1->tm_year == date2->tm_year &&
        date1->tm_mon == date2->tm_mon &&
        date1->tm_mday == date2->tm_mday) {
        return 1;
    }
    return 0;
}

// Ư�� ��¥�� �ش��ϴ� ������ ���� ã��
double get_closing_price(Stock* stock, struct tm* date) {
    for (int i = 0; i < stock->price_data_size; i++) {
        if (compare_dates(&stock->price_data[i].date, date)) {
            return stock->price_data[i].closing_price;
        }
    }
    return -1; // �ش� ��¥�� �����Ͱ� ���� ���
}

// �־��� �ֱ�� ��¥ ������ �´� ���ͷ� ��� �Լ�
double calculateReturn(Portfolio* portfolio, Stock* stocks) {
    double total_initial_investment = 0.0;
    double total_final_value = 0.0;

    // �� ���� ���� ���
    for (int i = 0; i < portfolio->stock_count; i++) {
        int stock_id = portfolio->stocks[i];
        double weight = portfolio->weights[i];
        Stock* stock = &stocks[stock_id];

        // �����ϰ� �������� ������ ã��
        double initial_price = get_closing_price(stock, &portfolio->start_date);
        double final_price = get_closing_price(stock, &portfolio->end_date);

        // ���� �������̳� �����Ͽ� ������ ���ٸ�, �ش� ������ ����
        if (initial_price == -1 || final_price == -1) {
            printf("���� %d�� ���� ���� �����͸� ã�� �� �����ϴ�.\n", stock_id);
            continue;
        }

        // �ʱ� ���ھ� ���
        double initial_investment = portfolio->amount * weight;
        total_initial_investment += initial_investment;

        // �ش� ������ ���� ��ġ�� ���
        double final_value = (initial_investment / initial_price) * final_price;
        total_final_value += final_value;
    }

    // ���� ���ͷ� ���
    double total_return = (total_final_value - total_initial_investment) / total_initial_investment;
    return total_return;
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