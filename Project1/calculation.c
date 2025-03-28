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

// 주어진 주기와 날짜 범위에 맞는 수익률 계산 함수
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
    printf("days: %d\n", days);
    int max_size = days / (portfolio->frequency);
    printf("max_size: %d\n", max_size);
    double weight = 0;

    // 기간 내 종가 데이터 배열
    int* closing_prices = (int*)malloc(days * sizeof(int));
    if (closing_prices == NULL) {
        fprintf(stderr, "Memory allocation failed: closing_prices\n");
        return -1.0;
    }

    // end_date 종목별 종가 데이터 배열
    int* end_date_prices = (int*)malloc(portfolio->stock_count * sizeof(int));
    if (end_date_prices == NULL) {
        fprintf(stderr, "Memory allocation failed: end_date_prices\n");
        free(closing_prices);
        return -1.0;
    }


    // 기간 내 구매량 데이터 배열 (이차원 배열)
    double** amounts = (double**)malloc(portfolio->stock_count * sizeof(double*));
    if (amounts == NULL) {
        fprintf(stderr, "Memory allocation failed: amounts\n");
        free(closing_prices);
        free(end_date_prices);
        return -1.0;
    }
    for (int i = 0; i < portfolio->stock_count; i++) {
        amounts[i] = (double*)malloc(max_size * sizeof(double));
        if (amounts[i] == NULL) {
            fprintf(stderr, "Memory allocation failed: amount[%d]\n", i);
            for (int j = 0; j < i; j++) free(amounts[j]);
            free(amounts);
            free(closing_prices);
            free(end_date_prices);
            return -1.0;
        }
    }


    for (int i = 0; i < portfolio->stock_count; i++) {
        // 기간 내 종가 조회
        select_closing_price(envhp, svchp, errhp, portfolio->stocks[i], portfolio->start_date, portfolio->end_date, days, closing_prices);
        end_date_prices[i] = closing_prices[days - 1];
        weight = select_weight(envhp, svchp, errhp, portfolio->stocks[i], portfolio->id);
        double weighted_amount = (portfolio->amount) * weight;

        int k = 0;
        for (int j = 0; j < days; j += portfolio->frequency) {
            printf("weighted_amount: %.2lf\n", weighted_amount);
            printf("%d번 종목의 %d번째 종가: %d\n", i+1, j+1, closing_prices[j]);
            double temp = weighted_amount / closing_prices[j];
            printf("구매량: %.2lf\n", temp);
            amounts[i][k] = temp;
            printf("%d의 %d번째 amount 값: %.2lf\n", i + 1, k + 1, amounts[i][k]);
            k++;
        }
    }
    free(closing_prices);

    disconnect_db(envhp, errhp, svchp, usrhp, srvhp);

    int total_amount = (portfolio->amount) * max_size;
    double total_return = 0.0;
    double total_holdings = 0.0; // 수정된 부분

    for (int i = 0; i < (portfolio->stock_count); i++) {
        total_holdings = 0.0;
        for (int j = 0; j < max_size; j++) {
            total_holdings += amounts[i][j];
        }
        total_return += total_holdings * end_date_prices[i];
    }

    // 메모리 해제
    free(end_date_prices);

    for (int i = 0; i < portfolio->stock_count; i++) {
        free(amounts[i]);
    }
    free(amounts);

    return total_return / total_amount * 100;
}


int main() {
    Portfolio p = {
         .id = 999,
         .stock_count = 3,
         .stocks = {1, 2, 3},
         .weights = {0.2, 0.3, 0.5},
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
    printf("총 수익률: %.2lf%%\n", total_return);

    return 0;
}