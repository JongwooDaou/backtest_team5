#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "cJSON.h"
#include "export.h"
#include "calculation.h"

// tm ����ü�� "YYYY-MM" ���ڿ��� ��ȯ�ϴ� �Լ�
void format_month(char* buffer, size_t size, struct tm date) {
    strftime(buffer, size, "%Y-%m", &date);
}

// start_date�� end_date�� ���̸� ����Ͽ� ���� �� ��ȯ
int calculate_months(struct tm start_date, struct tm end_date) {
    return (end_date.tm_year - start_date.tm_year) * 12 + (end_date.tm_mon - start_date.tm_mon) + 1;
}

// JSON���� ��ȯ �� ���� ���� �Լ�
void export_json(ResultData* data, struct tm start_date, struct tm end_date, Portfolio* portfolio) {
    // ���� �� ���
    int months = calculate_months(start_date, end_date);

    // �ֻ��� JSON ��ü ����
    cJSON* root = cJSON_CreateObject();

    // ��Ʈ������ ���� �߰�
    cJSON* portfolio_obj = cJSON_CreateObject();
    cJSON_AddNumberToObject(portfolio_obj, "id", portfolio->id);
    cJSON_AddNumberToObject(portfolio_obj, "stock_count", portfolio->stock_count);
    
    // stocks �迭�� �����ϰ� �� �߰�
    cJSON* stocks_array = cJSON_CreateArray();
    for (int i = 0; i < portfolio->stock_count; i++) {
        cJSON_AddItemToArray(stocks_array, cJSON_CreateNumber(portfolio->stocks[i]));
    }
    cJSON_AddItemToObject(portfolio_obj, "stocks", stocks_array);

    // weights �迭�� �����ϰ� �� �߰�
    cJSON* weights_array = cJSON_CreateArray();
    for (int i = 0; i < portfolio->stock_count; i++) {
        cJSON_AddItemToArray(weights_array, cJSON_CreateNumber(portfolio->weights[i]));
    }
    cJSON_AddItemToObject(portfolio_obj, "weights", weights_array);

    cJSON_AddNumberToObject(portfolio_obj, "frequency", portfolio->frequency);
    cJSON_AddNumberToObject(portfolio_obj, "amount", portfolio->amount);

    // start_date�� end_date�� "YYYY-MM"���� ��ȯ�Ͽ� �߰�
    char start_date_str[8], end_date_str[8];
    format_month(start_date_str, sizeof(start_date_str), portfolio->start_date);
    format_month(end_date_str, sizeof(end_date_str), portfolio->end_date);
    cJSON_AddStringToObject(portfolio_obj, "start_date", start_date_str);
    cJSON_AddStringToObject(portfolio_obj, "end_date", end_date_str);

    // ��Ʈ������ ������ root�� �߰�
    cJSON_AddItemToObject(root, "portfolio", portfolio_obj);

    // �⺻ ������ �߰�
    cJSON_AddNumberToObject(root, "portfolio_id", data->portfolio_id);
    cJSON_AddNumberToObject(root, "total_amount", data->total_amount);
    cJSON_AddNumberToObject(root, "total_return", data->total_return);
    cJSON_AddNumberToObject(root, "max_drawdown", data->max_drawdown);

    // best_month, worst_month�� "YYYY-MM" ���ڿ��� ��ȯ�Ͽ� �߰�
    char best_month_str[8], worst_month_str[8];
    format_month(best_month_str, sizeof(best_month_str), data->best_month);
    format_month(worst_month_str, sizeof(worst_month_str), data->worst_month);
    cJSON_AddStringToObject(root, "best_month", best_month_str);
    cJSON_AddStringToObject(root, "worst_month", worst_month_str);

    // start_date�� end_date�� "YYYY-MM"���� ��ȯ�Ͽ� �߰�
    char portfolio_start_date_str[8], portfolio_end_date_str[8];
    format_month(portfolio_start_date_str, sizeof(portfolio_start_date_str), start_date);
    format_month(portfolio_end_date_str, sizeof(portfolio_end_date_str), end_date);
    cJSON_AddStringToObject(root, "start_date", portfolio_start_date_str);
    cJSON_AddStringToObject(root, "end_date", portfolio_end_date_str);

    // ���� ���ͷ� �߰� (�迭)
    cJSON* monthly_profit_array = cJSON_CreateArray();
    for (int i = 0; i < months; i++) {
        cJSON* profit_obj = cJSON_CreateObject();
        char date_str[8];
        format_month(date_str, sizeof(date_str), data->monthly_profit[i].date);
        cJSON_AddStringToObject(profit_obj, "date", date_str);
        cJSON_AddNumberToObject(profit_obj, "profit_rate", data->monthly_profit[i].profit_rate);
        cJSON_AddItemToArray(monthly_profit_array, profit_obj);
    }
    cJSON_AddItemToObject(root, "monthly_profit", monthly_profit_array);

    // JSON ���ڿ� ��ȯ
    char* json_string = cJSON_Print(root);

    // JSON�� ���Ͽ� ����
    FILE* file = fopen("result.json", "w");
    if (file) {
        fprintf(file, "%s", json_string);
        fclose(file);
        printf("JSON ���� ���� �Ϸ�: result.json\n");
    }
    else {
        printf("���� ���� ����!\n");
    }

    // �޸� ����
    cJSON_free(json_string);
    cJSON_Delete(root);
}