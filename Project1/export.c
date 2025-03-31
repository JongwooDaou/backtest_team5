#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "cJSON.h"
#include "export.h"
#include "calculation.h"
#include "database.h"
#include "ociCRUD.h"

// DB�� �����ؼ� stock_id�� stock �̸��� ��ȯ�ϴ� �Լ�
char* find_stock_name(int stock_id) {
    
    char* stock_name = (char*)malloc(256);
    const char* sql = "SELECT STOCK_NAME FROM STOCKS WHERE STOCK_ID = :1";
    OCIStmt* stmthp; OCIDefine* defnp = NULL;
    OCIHandleAlloc(envhp, (void**)&stmthp, OCI_HTYPE_STMT, 0, NULL);
    OCIStmtPrepare(stmthp, errhp, (text*)sql, strlen(sql), OCI_NTV_SYNTAX, OCI_DEFAULT);
    OCIBind* bnd1 = NULL;
    OCIBindByPos(stmthp, &bnd1, errhp, 1, (void*)&stock_id, sizeof(stock_id), SQLT_INT, NULL, NULL, NULL, 0, NULL, OCI_DEFAULT);
    OCIDefineByPos(stmthp, &defnp, errhp, 1, stock_name, 256, SQLT_STR, NULL, NULL, NULL, OCI_DEFAULT);
    sword status = OCIStmtExecute(svchp, stmthp, errhp, 1, 0, NULL, NULL, OCI_DEFAULT);
    OCIHandleFree(stmthp, OCI_HTYPE_STMT);
    return stock_name;

    /*
    switch (stock_id) {
    case 1: return "Samsung Electronics";
    case 2: return "SK Hynix";
    case 3: return "LG Energy Solution";
    case 4: return "Samsung Biologics";
    case 5: return "Hyundai Motor";
    case 6: return "Kia";
    case 7: return "Celltrion";
    case 8: return "Naver";
    case 9: return "KB Financial Group";
    case 10: return "Hanwha Aerospace";
    case 11: return "Hyundai Mobis";
    case 12: return "HD Hyundai Heavy Industries";
    case 13: return "POSCO Holdings";
    case 14: return "Shinhan Financial Group";
    case 15: return "Meritz Financial Group";
    case 16: return "Samsung C and T";
    case 17: return "Hanwha Ocean";
    case 18: return "LG Chem";
    case 19: return "SK Innovation";
    case 20: return "Kakao";
    case 21: return "Daou Technology";
    case 22: return "Kiwoom Securities";
    case 23: return "Kiwoomhah";
    default: return "Unknown Company";
    }
    */
}

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
    oci_init();
    for (int i = 0; i < portfolio->stock_count; i++) {
        const char* stock_name = find_stock_name(portfolio->stocks[i]); // ����� ��ȸ
        cJSON_AddItemToArray(stocks_array, cJSON_CreateString(stock_name)); // ���ڿ� �߰�
    }
    oci_cleanup();
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
        //cJSON_AddNumberToObject(profit_obj, "proceed", data->monthly_profit[i].proceed);
        cJSON_AddNumberToObject(profit_obj, "valuation", data->monthly_profit[i].valuation);
        cJSON_AddNumberToObject(profit_obj, "total_investment", data->monthly_profit[i].total_investment);
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

void add_month(struct tm* date, int months) {
    struct tm new_date = *date;  // ���� ��¥ ����
    new_date.tm_mon += months;   // �� ����

    // mktime�� ����Ͽ� �ڵ����� ��¥ ����
    mktime(&new_date);

    // ������ ���� ���� (���ڴ� ������ 1�Ϸ� ����)
    date->tm_year = new_date.tm_year;
    date->tm_mon = new_date.tm_mon;
    date->tm_mday = 1;  // YYYY-MM ���� ���� (1�Ϸ� ����)

    // ���������� ��¥�� ������ ���
    printf("New Date: %d-%02d-%02d\n", date->tm_year + 1900, date->tm_mon + 1, date->tm_mday);
}



ResultData* create_result_data(const Portfolio* portfolio, const ReturnResult* returns, struct tm start_date, struct tm end_date) {
    ResultData* result = (ResultData*)malloc(sizeof(ResultData));
    if (result == NULL) {
        fprintf(stderr, "Memory allocation failed for ResultData.\n");
        return NULL;
    }

    // ���� �� ���
    int months = calculate_months(start_date, end_date);

    result->portfolio_id = portfolio->id;
    result->total_amount = 0;
    result->total_return = 0;
    result->max_drawdown = returns->mdd;

    // ���� ���ͷ� �� �򰡾��� ������ �迭 ���� �Ҵ�
    result->monthly_profit = (MonthlyProfit*)malloc(months * sizeof(MonthlyProfit));
    if (result->monthly_profit == NULL) {
        fprintf(stderr, "Memory allocation failed for MonthlyProfit array.\n");
        free(result);
        return NULL;
    }

    // ���� ���ͷ�, �򰡾�, ���� �򰡾��� ���

    int best_month_index = 0, worst_month_index = 0;
    double best_month_profit = returns->monthly_returns[0], worst_month_profit = returns->monthly_returns[0];

    for (int month = 0; month < months; month++) {
        result->monthly_profit[month].date = start_date;
        add_month(&result->monthly_profit[month].date, month);

        // ���� ���ͷ� ����
        result->monthly_profit[month].profit_rate = returns->monthly_returns[month];

        // ���� �򰡾� ���
        // ���ۿ��ϰ��
        if (month == 0) {
        double monthly_investment = portfolio->amount * (returns->monthly_returns[month] / 100.0) * returns->monthly_trade_count[month];
        result->monthly_profit[month].valuation = monthly_investment;
        }
        // ���ۿ��� �ƴҰ��
        else {
            double monthly_investment = portfolio->amount * (returns->monthly_returns[month] / 100.0) * (returns->monthly_trade_count[month] - returns->monthly_trade_count[month - 1]);
            result->monthly_profit[month].valuation = monthly_investment;
        }
        
        // ���� ���� ���ھ� ���
        result->monthly_profit[month].total_investment = portfolio->amount * returns->monthly_trade_count[month] * (returns->cum_monthly_returns[month] / 100.0);

        // �ִ�/�ּ� ���� �� ���
        if (result->monthly_profit[month].profit_rate > best_month_profit) {
            best_month_profit = result->monthly_profit[month].profit_rate;
            best_month_index = month;
        }
        if (result->monthly_profit[month].profit_rate < worst_month_profit) {
            worst_month_profit = result->monthly_profit[month].profit_rate;
            worst_month_index = month;
        }
    }

    // �ִ� ���� �ް� �ּ� ���� ���� tm ����ü ���
    result->best_month = result->monthly_profit[best_month_index].date;
    result->worst_month = result->monthly_profit[worst_month_index].date;

    // ���� ���� ���
    result->total_amount = portfolio->amount * returns->monthly_trade_count[months - 1];
    result->total_return = result->monthly_profit[months - 1].total_investment;
    return result;
}