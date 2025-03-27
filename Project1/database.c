#include "database.h"
#include <stdio.h>
#include <stdlib.h>
#include <oci.h>
#include <time.h>

// 에러 확인 함수 추가
void check_error(OCIError* errhp) {
    char errbuf[512];
    sb4 errcode = 0;
    OCIErrorGet(errhp, 1, NULL, &errcode, (OraText*)errbuf, sizeof(errbuf), OCI_HTYPE_ERROR);
    fprintf(stderr, "OCI Error: %s\n", errbuf);
}

// DB 연결
void connect_db(OCIEnv** envhp, OCIError** errhp, OCISvcCtx** svchp, OCISession** usrhp, OCIServer** srvhp,
    const char* username, const char* password, const char* dbname) {
    OCIEnvCreate(envhp, OCI_DEFAULT, NULL, NULL, NULL, NULL, 0, NULL);
    OCIHandleAlloc(*envhp, (void**)errhp, OCI_HTYPE_ERROR, 0, NULL);
    OCIHandleAlloc(*envhp, (void**)srvhp, OCI_HTYPE_SERVER, 0, NULL);
    OCIServerAttach(*srvhp, *errhp, (text*)dbname, strlen(dbname), OCI_DEFAULT);
    OCIHandleAlloc(*envhp, (void**)svchp, OCI_HTYPE_SVCCTX, 0, NULL);
    OCIAttrSet(*svchp, OCI_HTYPE_SVCCTX, *srvhp, 0, OCI_ATTR_SERVER, *errhp);
    OCIHandleAlloc(*envhp, (void**)usrhp, OCI_HTYPE_SESSION, 0, NULL);
    OCIAttrSet(*usrhp, OCI_HTYPE_SESSION, (void*)username, strlen(username), OCI_ATTR_USERNAME, *errhp);
    OCIAttrSet(*usrhp, OCI_HTYPE_SESSION, (void*)password, strlen(password), OCI_ATTR_PASSWORD, *errhp);
    if (OCISessionBegin(*svchp, *errhp, *usrhp, OCI_CRED_RDBMS, OCI_DEFAULT) != OCI_SUCCESS) {
        check_error(*errhp);
    }
    OCIAttrSet(*svchp, OCI_HTYPE_SVCCTX, *usrhp, 0, OCI_ATTR_SESSION, *errhp);
}

// DB 연결 해제
void disconnect_db(OCIEnv* envhp, OCIError* errhp, OCISvcCtx* svchp, OCISession* usrhp, OCIServer* srvhp) {
    OCISessionEnd(svchp, errhp, usrhp, OCI_DEFAULT);
    OCIServerDetach(srvhp, errhp, OCI_DEFAULT);
    OCIHandleFree(svchp, OCI_HTYPE_SVCCTX);
    OCIHandleFree(usrhp, OCI_HTYPE_SESSION);
    OCIHandleFree(srvhp, OCI_HTYPE_SERVER);
    OCIHandleFree(errhp, OCI_HTYPE_ERROR);
    OCIHandleFree(envhp, OCI_HTYPE_ENV);
}

void select_closing_price(OCIEnv* envhp, OCISvcCtx* svchp, OCIError* errhp,
    int stock_id, struct tm start_date, struct tm end_date, int max_size,
    int* closing_prices, int* num_results) {
    OCIStmt* stmthp = NULL;
    OCIDefine* def1 = NULL;
    OCIBind* bnd1 = NULL, * bnd2 = NULL, * bnd3 = NULL;
    int closing_price;
    char* select_sql =
        "SELECT closing_price FROM stock_price WHERE stock_id = :1 AND closing_date BETWEEN TO_DATE(:2, 'YYYY-MM-DD') AND TO_DATE(:3, 'YYYY-MM-DD')";

    // 날짜 문자열 변환
    char start_date_str[11];
    char end_date_str[11];
    snprintf(start_date_str, sizeof(start_date_str), "%04d-%02d-%02d",
        start_date.tm_year + 1900, start_date.tm_mon + 1, start_date.tm_mday);
    snprintf(end_date_str, sizeof(end_date_str), "%04d-%02d-%02d",
        end_date.tm_year + 1900, end_date.tm_mon + 1, end_date.tm_mday);

    *num_results = 0;  // 저장된 데이터 개수 초기화

    // 1. Statement 핸들 할당
    if (OCIHandleAlloc(envhp, (void**)&stmthp, OCI_HTYPE_STMT, 0, NULL) != OCI_SUCCESS) {
        fprintf(stderr, "Failed to allocate statement handle.\n");
        return;
    }

    // 2. SQL 문 준비
    if (OCIStmtPrepare(stmthp, errhp, (text*)select_sql, (ub4)strlen(select_sql), OCI_NTV_SYNTAX, OCI_DEFAULT) != OCI_SUCCESS) {
        fprintf(stderr, "Failed to prepare SQL statement.\n");
        OCIHandleFree(stmthp, OCI_HTYPE_STMT);
        return;
    }

    // 3. 바인딩
    if (OCIBindByPos(stmthp, &bnd1, errhp, 1, (void*)&stock_id, sizeof(stock_id), SQLT_INT, NULL, NULL, NULL, 0, NULL, OCI_DEFAULT) != OCI_SUCCESS ||
        OCIBindByPos(stmthp, &bnd2, errhp, 2, (void*)start_date_str, strlen(start_date_str) + 1, SQLT_CHR, NULL, NULL, NULL, 0, NULL, OCI_DEFAULT) != OCI_SUCCESS ||
        OCIBindByPos(stmthp, &bnd3, errhp, 3, (void*)end_date_str, strlen(end_date_str) + 1, SQLT_CHR, NULL, NULL, NULL, 0, NULL, OCI_DEFAULT) != OCI_SUCCESS) {
        fprintf(stderr, "Failed to bind parameters.\n");
        OCIHandleFree(stmthp, OCI_HTYPE_STMT);
        return;
    }

    // 4. SQL 실행
    if (OCIStmtExecute(svchp, stmthp, errhp, 0, 0, NULL, NULL, OCI_DEFAULT) != OCI_SUCCESS) {
        fprintf(stderr, "Failed to execute SQL statement.\n");
        OCIHandleFree(stmthp, OCI_HTYPE_STMT);
        return;
    }

    // 5. 결과 정의
    if (OCIDefineByPos(stmthp, &def1, errhp, 1, &closing_price, sizeof(closing_price), SQLT_INT, NULL, NULL, NULL, OCI_DEFAULT) != OCI_SUCCESS) {
        fprintf(stderr, "Failed to define output.\n");
        OCIHandleFree(stmthp, OCI_HTYPE_STMT);
        return;
    }

    // 6. 결과 가져와 배열에 저장
    while (OCIStmtFetch(stmthp, errhp, 1, OCI_FETCH_NEXT, OCI_DEFAULT) == OCI_SUCCESS) {
        if (*num_results < max_size) {
            closing_prices[*num_results] = closing_price;
            (*num_results)++;
        }
        else {
            fprintf(stderr, "Warning: Result set exceeds buffer size (%d). Truncating results.\n", max_size);
            break;
        }
    }

    // 7. 정리
    OCIHandleFree(stmthp, OCI_HTYPE_STMT);
}

double select_weight(OCIEnv* envhp, OCISvcCtx* svchp, OCIError* errhp, int stock_id, int portfolio_id) {
    OCIStmt* stmthp = NULL;
    OCIDefine* def1 = NULL;
    OCIBind* bnd1 = NULL, * bnd2 = NULL;
    double weight = -1.0; // 기본값 설정 (조회 실패 시 -1 반환)

    char* select_sql =
        "SELECT ratio FROM holdings WHERE stock_id = :1 AND portfolio_id = :2";

    // 1. Statement 핸들 할당
    if (OCIHandleAlloc(envhp, (void**)&stmthp, OCI_HTYPE_STMT, 0, NULL) != OCI_SUCCESS) {
        fprintf(stderr, "Failed to allocate statement handle.\n");
        return weight;
    }

    // 2. SQL 문 준비
    if (OCIStmtPrepare(stmthp, errhp, (text*)select_sql, (ub4)strlen(select_sql), OCI_NTV_SYNTAX, OCI_DEFAULT) != OCI_SUCCESS) {
        fprintf(stderr, "Failed to prepare SQL statement.\n");
        OCIHandleFree(stmthp, OCI_HTYPE_STMT);
        return weight;
    }

    // 3. 바인딩
    if (OCIBindByPos(stmthp, &bnd1, errhp, 1, (void*)&stock_id, sizeof(stock_id), SQLT_INT, NULL, NULL, NULL, 0, NULL, OCI_DEFAULT) != OCI_SUCCESS ||
        OCIBindByPos(stmthp, &bnd2, errhp, 2, (void*)&portfolio_id, sizeof(portfolio_id), SQLT_INT, NULL, NULL, NULL, 0, NULL, OCI_DEFAULT) != OCI_SUCCESS) {
        fprintf(stderr, "Failed to bind parameters.\n");
        OCIHandleFree(stmthp, OCI_HTYPE_STMT);
        return weight;
    }

    // 4. 결과 정의 (SQLT_FLT 사용)
    if (OCIDefineByPos(stmthp, &def1, errhp, 1, &weight, sizeof(weight), SQLT_FLT, NULL, NULL, NULL, OCI_DEFAULT) != OCI_SUCCESS) {
        fprintf(stderr, "Failed to define output.\n");
        OCIHandleFree(stmthp, OCI_HTYPE_STMT);
        return weight;
    }

    // 5. SQL 실행
    if (OCIStmtExecute(svchp, stmthp, errhp, 1, 0, NULL, NULL, OCI_DEFAULT) != OCI_SUCCESS) {
        fprintf(stderr, "Failed to execute SQL statement.\n");
        OCIHandleFree(stmthp, OCI_HTYPE_STMT);
        return weight;
    }

    // 6. 결과 가져오기
    if (OCIStmtFetch(stmthp, errhp, 1, OCI_FETCH_NEXT, OCI_DEFAULT) != OCI_SUCCESS) {
        fprintf(stderr, "No data found for stock_id: %d, portfolio_id: %d\n", stock_id, portfolio_id);
        weight = -1.0; // 조회 결과가 없으면 -1 반환
    }

    // 7. 정리
    OCIHandleFree(stmthp, OCI_HTYPE_STMT);

    return weight; // 조회한 weight 값 반환
}
