#pragma once
#ifndef EXPORT_H
#define EXPORT_H

#include <time.h>
#include "calculation.h"

// 월별 수익률을 받기 위한 구조체
typedef struct {
    struct tm date; // 수익을 얻은 달을 표현
    double profit_rate; // 수익률
} MonthlyProfit;

// 결과 데이터를 받기 위한 구조체
typedef struct {
    int portfolio_id; // 포트폴리오 ID
    double total_amount; // 총 투자금
    double total_return; // 최종 수익
    double max_drawdown; // 최대 낙폭
    struct tm best_month; // 최대 수익 달
    struct tm worst_month; // 최소 수익 달
    MonthlyProfit* monthly_profit;
} ResultData;

// 개월 수 계산 함수
int calculate_months(struct tm start_date, struct tm end_date);

// JSON 변환 및 저장 함수
void export_json(ResultData*, struct tm start_date, struct tm end_date, Portfolio* portfolio);

#endif
