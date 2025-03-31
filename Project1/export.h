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

// 월별 수익률을 받기 위한 구조체
typedef struct {
    struct tm date; // 수익을 얻은 달을 표현
    double profit_rate; // 수익률
    //double proceed; // 월별 수익금
    double valuation; // 월말 평가액
    double total_investment; // 월말 누적 투자액
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

// DB에 접근해서 stock_id의 stock 이름 반환 함수
char* find_stock_name(int stock_id);

// tm 구조체 'YYYY-MM' 변환 함수
void format_month(char* buffer, size_t size, struct tm date);

// 개월 수 계산 함수
int calculate_months(struct tm start_date, struct tm end_date);

// JSON 변환 및 저장 함수
void export_json(ResultData*, struct tm start_date, struct tm end_date, Portfolio* portfolio);

// 월을 증가시키는 함수
void add_month(struct tm* date, int months);

// ResultData 생성 함수
ResultData* create_result_data(const Portfolio* portfolio, const ReturnResult* returns, struct tm start_date, struct tm end_date);

#endif
