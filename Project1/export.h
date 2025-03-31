#pragma once
#ifndef EXPORT_H
#define EXPORT_H

#include <time.h>
#include <oci.h>
#include "calculation.h"
#include "ociCRUD.h"

extern OCIEnv* envhp;
extern OCIError* errhp;
extern OCIServer* srvhp;
extern OCISvcCtx* svchp;
extern OCISession* usrhp;

// ���� ���ͷ��� �ޱ� ���� ����ü
typedef struct {
    struct tm date; // ������ ���� ���� ǥ��
    double profit_rate; // ���ͷ�
    //double proceed; // ���� ���ͱ�
    double valuation; // ���� �򰡾�
    double total_investment; // ���� ���� ���ھ�
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

// DB�� �����ؼ� stock_id�� stock �̸� ��ȯ �Լ�
char* find_stock_name(int stock_id);

// tm ����ü 'YYYY-MM' ��ȯ �Լ�
void format_month(char* buffer, size_t size, struct tm date);

// ���� �� ��� �Լ�
int calculate_months(struct tm start_date, struct tm end_date);

// JSON ��ȯ �� ���� �Լ�
void export_json(ResultData*, struct tm start_date, struct tm end_date, Portfolio* portfolio);

// ���� ������Ű�� �Լ�
void add_month(struct tm* date, int months);

// ResultData ���� �Լ�
ResultData* create_result_data(const Portfolio* portfolio, const ReturnResult* returns, struct tm start_date, struct tm end_date);

#endif
