#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "calculation.h"
#include "database.h"

// 날짜 비교 함수 (두 날짜가 동일한지 확인)
int compare_dates(struct tm* date1, struct tm* date2) {
    if (date1->tm_year == date2->tm_year &&
        date1->tm_mon == date2->tm_mon &&
        date1->tm_mday == date2->tm_mday) {
        return 1;
    }
    return 0;
}

int calculate_days_between(struct tm start, struct tm end) {
    // struct tm을 time_t로 변환
    time_t start_time = mktime(&start);
    time_t end_time = mktime(&end);

    if (start_time == -1 || end_time == -1) {
        fprintf(stderr, "Error converting time.\n");
        return -1;
    }

    // 초 단위 차이를 일 단위로 변환
    double difference = difftime(start_time, end_time);
    return (int)(difference / (60 * 60 * 24));  // 초 -> 일 변환
}

// 월 차이를 계산하는 함수
int calculate_month_difference(struct tm start_date, struct tm end_date) {
    return (end_date.tm_year - start_date.tm_year) * 12 + (end_date.tm_mon - start_date.tm_mon) + 1;
}

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
    int month_diff = calculate_month_difference(portfolio->start_date, portfolio->end_date);
    int stockCount = portfolio->stock_count;
    StockPrice* stock_prices = (StockPrice*)malloc(days * sizeof(StockPrice));
    if (stock_prices == NULL) {
        fprintf(stderr, "Memory allocation failed: stock_prices\n");
        return NAN;  // NaN으로 변경
    }

    // 이차원 배열 초기화
    int** end_date_prices = (int**)malloc(stockCount * sizeof(int*));
    if (end_date_prices == NULL) {
        fprintf(stderr, "Memory allocation failed: portfolio_amounts\n");
        free(stock_prices);
        return NAN;  // NaN으로 변경
    }
    for (int i = 0; i < stockCount; i++) {
        end_date_prices[i] = (int*)calloc(month_diff, sizeof(int)); // 개월 수 크기로 설정
        if (end_date_prices[i] == NULL) {
            fprintf(stderr, "Memory allocation failed: end_date_prices[%d]\n", i);
            for (int j = 0; j < i; j++) free(end_date_prices[j]);
            free(stock_prices);
            free(end_date_prices);
            return NAN;  // NaN으로 변경
        }
    }

    // 이차원 배열 초기화
    double** portfolio_amounts = (double**)malloc(stockCount * sizeof(double*));
    if (portfolio_amounts == NULL) {
        fprintf(stderr, "Memory allocation failed: portfolio_amounts\n");
        free(stock_prices);
        free(end_date_prices);
        return NAN;  // NaN으로 변경
    }
    for (int i = 0; i < stockCount; i++) {
        portfolio_amounts[i] = (double*)calloc(month_diff, sizeof(double)); // 개월 수 크기로 설정
        if (portfolio_amounts[i] == NULL) {
            fprintf(stderr, "Memory allocation failed: portfolio_amounts[%d]\n", i);
            for (int j = 0; j < i; j++) free(portfolio_amounts[j]);
            free(portfolio_amounts);
            free(stock_prices);
            free(end_date_prices);
            return NAN;  // NaN으로 변경
        }
    }

    // 이차원 배열 초기화
    double** cum_portfolio_amounts = (double**)malloc(stockCount * sizeof(double*));
    if (cum_portfolio_amounts == NULL) {
        fprintf(stderr, "Memory allocation failed: portfolio_amounts\n");
        free(stock_prices);
        free(end_date_prices);
        return NAN;  // NaN으로 변경
    }
    for (int i = 0; i < stockCount; i++) {
        cum_portfolio_amounts[i] = (double*)calloc(month_diff, sizeof(double)); // 개월 수 크기로 설정
        if (cum_portfolio_amounts[i] == NULL) {
            fprintf(stderr, "Memory allocation failed: portfolio_amounts[%d]\n", i);
            for (int j = 0; j < i; j++) free(cum_portfolio_amounts[j]);
            free(cum_portfolio_amounts);
            free(portfolio_amounts);
            for (int j = 0; j < i; j++) free(portfolio_amounts[j]);
            free(stock_prices);
            free(end_date_prices);
            return NAN;  // NaN으로 변경
        }
    }

    // 월별 수익률
    double* monthly_return_rates = (double*)calloc(month_diff, sizeof(double));
    if (monthly_return_rates == NULL) {
        fprintf(stderr, "Memory allocation failed: portfolio_amounts\n");
        free(stock_prices);
        free(end_date_prices);
        return NAN;  // NaN으로 변경
    }

    // 월별 누적 수익률
    double* cum_monthly_return_rates = (double*)calloc(month_diff, sizeof(double));
    if (cum_monthly_return_rates == NULL) {
        fprintf(stderr, "Memory allocation failed: portfolio_amounts\n");
        free(stock_prices);
        free(end_date_prices);
        return NAN;  // NaN으로 변경
    }

    int* monthly_trade_count = (int*)calloc(month_diff, sizeof(int));
    int* cum_monthly_trade_count = (int*)calloc(month_diff, sizeof(int));

    // 종목별로 가격 및 구매량 계산
    for (int i = 0; i < stockCount; i++) {
        select_closing_price(envhp, svchp, errhp, portfolio->stocks[i], portfolio->start_date, portfolio->end_date, days, stock_prices);

        if (days <= 0) {
            fprintf(stderr, "Error: No closing price data for stock %d.\n", portfolio->stocks[i]);
            free(stock_prices);
            free(end_date_prices);
            free(portfolio_amounts);
            disconnect_db(envhp, errhp, svchp, usrhp, srvhp);
            return NAN;  // NaN으로 변경
        }

        // 월말 종가를 마지막 날 종가로 설정
        end_date_prices[i][month_diff - 1] = stock_prices[days - 1].closing_price;

        double weight = select_weight(envhp, svchp, errhp, portfolio->stocks[i], portfolio->id);
        double weighted_amount = portfolio->amount * weight;

        int prev_month = stock_prices[0].closing_date.tm_mon;
        int k = 0;
        int next_trade = 0;

        for (int j = 0; j < days; j++) {
            int curr_month = stock_prices[j].closing_date.tm_mon;
            if (curr_month != prev_month) {
                k++;  // 월이 바뀌면 k 증가
            }
            if (k < month_diff) {  // 배열 범위 초과 방지
                if (j == next_trade) {
                    portfolio_amounts[i][k] += weighted_amount / stock_prices[j].closing_price;
                    next_trade += portfolio->frequency;
                    if (i == 0) {
                        monthly_trade_count[k] += 1;
                    }
                }
                end_date_prices[i][k] = stock_prices[j].closing_price;
                
            }
            prev_month = curr_month;
        }
    }

    printf("월별 구매 횟수: ");
    for (int i = 0; i < month_diff; i++) {
        printf("%d ", monthly_trade_count[i]);
        if (i == 0) cum_monthly_trade_count[i] = monthly_trade_count[i];
        else cum_monthly_trade_count[i] = cum_monthly_trade_count[i - 1] + monthly_trade_count[i];
        
    }
    printf("\n");

    for (int i = 0; i < stockCount; i++) {
        printf("%d번 종목 보유량: [ ", i + 1);
        for (int j = 0; j < month_diff; j++) {
            printf("%.2lf ", portfolio_amounts[i][j]);
        }
        printf("]\n");
    }

    for (int i = 0; i < stockCount; i++) {
        printf("%d번 종목 월말 종가: [ ", i + 1);
        for (int j = 0; j < month_diff; j++) {
            printf("%d ", end_date_prices[i][j]);
        }
        printf("]\n");
    }

    // 월별 누적 종목 보유량
    for (int i = 0; i < stockCount; i++) {
        cum_portfolio_amounts[i][0] = portfolio_amounts[i][0];
        for (int j = 1; j < month_diff; j++) {
            cum_portfolio_amounts[i][j] = cum_portfolio_amounts[i][j - 1] + portfolio_amounts[i][j];
        }
    }

    for (int i = 0; i < stockCount; i++) {
        printf("%d번 종목 누적 보유량: [ ", i + 1);
        for (int j = 0; j < month_diff; j++) {
            printf("%.2lf ", cum_portfolio_amounts[i][j]);
        }
        printf("]\n");
    }

    disconnect_db(envhp, errhp, svchp, usrhp, srvhp);

    printf("월별 수익률: [ ");
    for (int j = 0; j < month_diff; j++) {
        for (int i = 0; i < stockCount; i++) {
            monthly_return_rates[j] += (portfolio_amounts[i][j] * end_date_prices[i][j]);   
        }
        monthly_return_rates[j] = monthly_return_rates[j] / ((portfolio->amount * monthly_trade_count[j])) * 100;
        printf("%.1lf%% ", monthly_return_rates[j] - 100);
    }
    printf("]\n");

    printf("월별 누적 수익률: [ ");
    for (int j = 0; j < month_diff; j++) {
        for (int i = 0; i < stockCount; i++) {
            cum_monthly_return_rates[j] += cum_portfolio_amounts[i][j] * end_date_prices[i][j];
        }
        cum_monthly_return_rates[j] = cum_monthly_return_rates[j] / ((portfolio->amount * cum_monthly_trade_count[j])) * 100;
        printf("%.1lf%% ", cum_monthly_return_rates[j] - 100);
    }
    printf("]\n");


    free(stock_prices);
    free(end_date_prices);
    for (int i = 0; i < portfolio->stock_count; i++) {
        free(portfolio_amounts[i]);
    }
    free(portfolio_amounts);
    free(monthly_trade_count);

    return cum_monthly_return_rates[month_diff-1];
}

int main() {
    Portfolio p = {
         .id = 999,
         .stock_count = 3,
         .stocks = {1, 2, 3},
         .weights = {0.1, 0.8, 0.1},
         .frequency = 7,
         .amount = 1000000
    };

    // 시작 날짜 (2022-01-01)
    p.start_date.tm_year = 2023 - 1900;
    p.start_date.tm_mon = 0;
    p.start_date.tm_mday = 1;

    // 종료 날짜 (2023-12-31)
    p.end_date.tm_year = 2023 - 1900;
    p.end_date.tm_mon = 11;
    p.end_date.tm_mday = 31;

    double total_return = calculateReturn(&p);
    if (isnan(total_return)) {
        printf("수익률 계산에 실패했습니다.\n");
    }
    else {
        printf("총 수익률: %.2lf%%\n", total_return);
    }

    return 0;
}
