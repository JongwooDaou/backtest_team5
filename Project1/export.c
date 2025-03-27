#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "cJSON.h"
#include "export.h"
#include "calculation.h"

// tm 구조체를 "YYYY-MM" 문자열로 변환하는 함수
void format_month(char* buffer, size_t size, struct tm date) {
    strftime(buffer, size, "%Y-%m", &date);
}

// start_date와 end_date의 차이를 계산하여 개월 수 반환
int calculate_months(struct tm start_date, struct tm end_date) {
    return (end_date.tm_year - start_date.tm_year) * 12 + (end_date.tm_mon - start_date.tm_mon) + 1;
}

// JSON으로 변환 및 파일 저장 함수
void export_json(ResultData* data, struct tm start_date, struct tm end_date, Portfolio* portfolio) {
    // 개월 수 계산
    int months = calculate_months(start_date, end_date);

    // 최상위 JSON 객체 생성
    cJSON* root = cJSON_CreateObject();

    // 포트폴리오 정보 추가
    cJSON* portfolio_obj = cJSON_CreateObject();
    cJSON_AddNumberToObject(portfolio_obj, "id", portfolio->id);
    cJSON_AddNumberToObject(portfolio_obj, "stock_count", portfolio->stock_count);
    
    // stocks 배열을 생성하고 값 추가
    cJSON* stocks_array = cJSON_CreateArray();
    for (int i = 0; i < portfolio->stock_count; i++) {
        cJSON_AddItemToArray(stocks_array, cJSON_CreateNumber(portfolio->stocks[i]));
    }
    cJSON_AddItemToObject(portfolio_obj, "stocks", stocks_array);

    // weights 배열을 생성하고 값 추가
    cJSON* weights_array = cJSON_CreateArray();
    for (int i = 0; i < portfolio->stock_count; i++) {
        cJSON_AddItemToArray(weights_array, cJSON_CreateNumber(portfolio->weights[i]));
    }
    cJSON_AddItemToObject(portfolio_obj, "weights", weights_array);

    cJSON_AddNumberToObject(portfolio_obj, "frequency", portfolio->frequency);
    cJSON_AddNumberToObject(portfolio_obj, "amount", portfolio->amount);

    // start_date와 end_date를 "YYYY-MM"으로 변환하여 추가
    char start_date_str[8], end_date_str[8];
    format_month(start_date_str, sizeof(start_date_str), portfolio->start_date);
    format_month(end_date_str, sizeof(end_date_str), portfolio->end_date);
    cJSON_AddStringToObject(portfolio_obj, "start_date", start_date_str);
    cJSON_AddStringToObject(portfolio_obj, "end_date", end_date_str);

    // 포트폴리오 정보를 root에 추가
    cJSON_AddItemToObject(root, "portfolio", portfolio_obj);

    // 기본 데이터 추가
    cJSON_AddNumberToObject(root, "portfolio_id", data->portfolio_id);
    cJSON_AddNumberToObject(root, "total_amount", data->total_amount);
    cJSON_AddNumberToObject(root, "total_return", data->total_return);
    cJSON_AddNumberToObject(root, "max_drawdown", data->max_drawdown);

    // best_month, worst_month를 "YYYY-MM" 문자열로 변환하여 추가
    char best_month_str[8], worst_month_str[8];
    format_month(best_month_str, sizeof(best_month_str), data->best_month);
    format_month(worst_month_str, sizeof(worst_month_str), data->worst_month);
    cJSON_AddStringToObject(root, "best_month", best_month_str);
    cJSON_AddStringToObject(root, "worst_month", worst_month_str);

    // start_date와 end_date를 "YYYY-MM"으로 변환하여 추가
    char portfolio_start_date_str[8], portfolio_end_date_str[8];
    format_month(portfolio_start_date_str, sizeof(portfolio_start_date_str), start_date);
    format_month(portfolio_end_date_str, sizeof(portfolio_end_date_str), end_date);
    cJSON_AddStringToObject(root, "start_date", portfolio_start_date_str);
    cJSON_AddStringToObject(root, "end_date", portfolio_end_date_str);

    // 월별 수익률 추가 (배열)
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

    // JSON 문자열 변환
    char* json_string = cJSON_Print(root);

    // JSON을 파일에 저장
    FILE* file = fopen("result.json", "w");
    if (file) {
        fprintf(file, "%s", json_string);
        fclose(file);
        printf("JSON 파일 저장 완료: result.json\n");
    }
    else {
        printf("파일 열기 실패!\n");
    }

    // 메모리 해제
    cJSON_free(json_string);
    cJSON_Delete(root);
}