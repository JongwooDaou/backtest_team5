#pragma once
#ifndef EXPORT_H
#define EXPORT_H

#include <time.h>
#include "calculation.h"

// ���� ���ͷ��� �ޱ� ���� ����ü
typedef struct {
    struct tm date; // ������ ���� ���� ǥ��
    double profit_rate; // ���ͷ�
} MonthlyProfit;

// ��� �����͸� �ޱ� ���� ����ü
typedef struct {
    int portfolio_id; // ��Ʈ������ ID
    double total_amount; // �� ���ڱ�
    double total_return; // ���� ����
    double max_drawdown; // �ִ� ����
    struct tm best_month; // �ִ� ���� ��
    struct tm worst_month; // �ּ� ���� ��
    MonthlyProfit* monthly_profit;
} ResultData;

// ���� �� ��� �Լ�
int calculate_months(struct tm start_date, struct tm end_date);

// JSON ��ȯ �� ���� �Լ�
void export_json(ResultData*, struct tm start_date, struct tm end_date, Portfolio* portfolio);

#endif
