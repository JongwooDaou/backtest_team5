#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "calculation.h"

// 날짜 비교 함수 (두 날짜가 동일한지 확인)
int compare_dates(struct tm* date1, struct tm* date2) {
    if (date1->tm_year == date2->tm_year &&
        date1->tm_mon == date2->tm_mon &&
        date1->tm_mday == date2->tm_mday) {
        return 1;
    }
    return 0;
}

// 특정 날짜에 해당하는 종목의 종가 찾기
double get_closing_price(Stock* stock, struct tm* date) {
    for (int i = 0; i < stock->price_data_size; i++) {
        if (compare_dates(&stock->price_data[i].date, date)) {
            return stock->price_data[i].closing_price;
        }
    }
    return -1; // 해당 날짜에 데이터가 없는 경우
}

// 주어진 주기와 날짜 범위에 맞는 수익률 계산 함수
double calculateReturn(Portfolio* portfolio, Stock* stocks) {
    double total_initial_investment = 0.0;
    double total_final_value = 0.0;

    // 각 종목에 대해 계산
    for (int i = 0; i < portfolio->stock_count; i++) {
        int stock_id = portfolio->stocks[i];
        double weight = portfolio->weights[i];
        Stock* stock = &stocks[stock_id];

        // 시작일과 종료일의 종가를 찾기
        double initial_price = get_closing_price(stock, &portfolio->start_date);
        double final_price = get_closing_price(stock, &portfolio->end_date);

        // 만약 시작일이나 종료일에 종가가 없다면, 해당 종목을 제외
        if (initial_price == -1 || final_price == -1) {
            printf("종목 %d에 대한 가격 데이터를 찾을 수 없습니다.\n", stock_id);
            continue;
        }

        // 초기 투자액 계산
        double initial_investment = portfolio->amount * weight;
        total_initial_investment += initial_investment;

        // 해당 종목의 최종 가치를 계산
        double final_value = (initial_investment / initial_price) * final_price;
        total_final_value += final_value;
    }

    // 최종 수익률 계산
    double total_return = (total_final_value - total_initial_investment) / total_initial_investment;
    return total_return;
}

// 포트폴리오 비중 검증 함수 (합계가 1이어야 함)
int validateWeights(Portfolio* p) {
    double sum = 0;

    for (int i = 0; i < p->stock_count; i++) {
        sum += p->weights[i];
    }

    if (sum != 1.0) {
        printf("전체 비중의 합계가 100%가 되어야 합니다.");
        return 0;
    }

    return 1;
}