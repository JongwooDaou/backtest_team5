#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include "export.h"

int main() {

    //oci_init();

    //int num;
    //scanf("%d", &num);

    //char* name = find_stock_name(num);

    //printf("%s", name);

    Portfolio portfolio = {
    .id = 1,
    .stock_count = 3,
    .stocks = {3, 1, 19},
    .weights = {0.5, 0.3, 0.2},
    .frequency = 6,
    .amount = 500000,
    .start_date = {.tm_year = 2023 - 1900, .tm_mon = 0, .tm_mday = 1 }, 
    .end_date = {.tm_year = 2024 - 1900, .tm_mon = 11, .tm_mday = 31 } 
    };

    ReturnResult rr = calculateReturn(&portfolio);

    
    printf("\n"); printf("\n");
    printf("mdd\n");
    printf("%lf\n", rr.mdd);

    printf("\n"); printf("\n");
    printf("trade count\n");
    for (int i = 0; i < 24; i++) {
        printf("%5d", rr.monthly_trade_count[i]);
    }
    printf("\n");
    printf("\n"); printf("\n");
    printf("montly returns\n");
    for (int i = 0; i <24; i++) {
        printf("%lf\n", rr.monthly_returns[i]);
    }
    printf("\n"); printf("\n"); printf("cum_monthly_returns\n");
    for (int i = 0; i <24; i++) {
        printf("%lf\n", rr.cum_monthly_returns[i]);
    }

    printf("\n");
    */


    ResultData* data = create_result_data(&portfolio, &rr, portfolio.start_date, portfolio.end_date);

    //printf("%lf\n", rr.mdd);
    /*
     for (int i = 0; i < 48; i++) {
        printf("%d %d\n", data->monthly_profit[i].date.tm_year + 1900, data->monthly_profit[i].date.tm_mon + 1);
    }
    printf("\n");
    printf("profit rate\n");
    for (int i = 0; i <= 48; i++) {
        printf("%lf          \n", data->monthly_profit[i].profit_rate);
    }
    printf("\n");
    printf("\n");
    printf("total_investment\n");
    for (int i = 0; i <= 48; i++) {
        printf("%lf                \n", data->monthly_profit[i].total_investment);
    }
    printf("\n");
    printf("\n");
    printf("\n");
    printf("valuation\n");
    for (int i = 0; i <= 48; i++) {
        printf("%lf              \n", data->monthly_profit[i].valuation);
    }
    printf("\n");
    */


    export_json(data, portfolio.start_date, portfolio.end_date, &portfolio);

    //oci_cleanup();

    return 0;
}