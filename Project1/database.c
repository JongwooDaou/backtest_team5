#define _CRT_SECURE_NO_WARNINGS
#include "database.h"
#include <stdio.h>
#include <stdlib.h>
#include <oci.h>
#include <time.h>
#include <string.h>
#include "ociCRUD.h"

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

int count_closing_prices(OCIEnv* envhp, OCISvcCtx* svchp, OCIError* errhp, int stock_id, struct tm start_date, struct tm end_date) {
    OCIStmt* stmthp = NULL;
    OCIDefine* def1 = NULL;
    OCIBind* bnd1 = NULL, * bnd2 = NULL, * bnd3 = NULL;
    int count = 0;
    char* select_sql =
        "SELECT count(closing_price) FROM stock_price WHERE stock_id = :1 AND closing_date BETWEEN TO_DATE(:2, 'YYYY-MM-DD') AND TO_DATE(:3, 'YYYY-MM-DD')";

    char start_date_str[11], end_date_str[11];
    snprintf(start_date_str, sizeof(start_date_str), "%04d-%02d-%02d",
        start_date.tm_year + 1900, start_date.tm_mon + 1, start_date.tm_mday);
    snprintf(end_date_str, sizeof(end_date_str), "%04d-%02d-%02d",
        end_date.tm_year + 1900, end_date.tm_mon + 1, end_date.tm_mday);

    if (OCIHandleAlloc(envhp, (void**)&stmthp, OCI_HTYPE_STMT, 0, NULL) != OCI_SUCCESS) {
        fprintf(stderr, "1.Failed to allocate statement handle.\n");
        return count;
    }

    if (OCIStmtPrepare(stmthp, errhp, (text*)select_sql, (ub4)strlen(select_sql), OCI_NTV_SYNTAX, OCI_DEFAULT) != OCI_SUCCESS) {
        fprintf(stderr, "2.Failed to prepare SQL statement.\n");
        OCIHandleFree(stmthp, OCI_HTYPE_STMT);
        return count;
    }

    // 바인딩
    if (OCIBindByPos(stmthp, &bnd1, errhp, 1, (void*)&stock_id, sizeof(stock_id), SQLT_INT, NULL, NULL, NULL, 0, NULL, OCI_DEFAULT) != OCI_SUCCESS ||
        OCIBindByPos(stmthp, &bnd2, errhp, 2, (void*)start_date_str, strlen(start_date_str) + 1, SQLT_CHR, NULL, NULL, NULL, 0, NULL, OCI_DEFAULT) != OCI_SUCCESS ||
        OCIBindByPos(stmthp, &bnd3, errhp, 3, (void*)end_date_str, strlen(end_date_str) + 1, SQLT_CHR, NULL, NULL, NULL, 0, NULL, OCI_DEFAULT) != OCI_SUCCESS) {
        fprintf(stderr, "3.Failed to bind parameters.\n");
        OCIHandleFree(stmthp, OCI_HTYPE_STMT);
        return count;
    }

    // Define (SQL 실행 전)
    if (OCIDefineByPos(stmthp, &def1, errhp, 1, &count, sizeof(count), SQLT_INT, NULL, NULL, NULL, OCI_DEFAULT) != OCI_SUCCESS) {
        fprintf(stderr, "4.Failed to define output.\n");
        OCIHandleFree(stmthp, OCI_HTYPE_STMT);
        return count;
    }

    // SQL 실행
    sword status = OCIStmtExecute(svchp, stmthp, errhp, 0, 0, NULL, NULL, OCI_DEFAULT);
    if (status != OCI_SUCCESS) {
        print_oci_error(errhp, status);
        OCIHandleFree(stmthp, OCI_HTYPE_STMT);
        return count;
    }

    // 결과 가져오기
    if (OCIStmtFetch(stmthp, errhp, 1, OCI_FETCH_NEXT, OCI_DEFAULT) != OCI_SUCCESS) {
        fprintf(stderr, "6.No data found or fetch failed.\n");
        count = 0;
    }

    // 리소스 해제
    OCIHandleFree(stmthp, OCI_HTYPE_STMT);

    return count;
}


void select_closing_price(OCIEnv* envhp, OCISvcCtx* svchp, OCIError* errhp,
    int stock_id, struct tm start_date, struct tm end_date, int max_size,
    StockPrice* closing_prices) {

    OCIStmt* stmthp = NULL;
    OCIDefine* def1 = NULL, * def2 = NULL;
    OCIBind* bnd1 = NULL, * bnd2 = NULL, * bnd3 = NULL;
    int closing_price;
    char closing_date_str[11];  // YYYY-MM-DD 형식 (문자열)

    char* select_sql =
        "SELECT closing_price, TO_CHAR(closing_date, 'YYYY-MM-DD') FROM stock_price "
        "WHERE stock_id = :1 AND closing_date BETWEEN TO_DATE(:2, 'YYYY-MM-DD') AND TO_DATE(:3, 'YYYY-MM-DD') ORDER BY closing_date";

    // 날짜 문자열 변환
    char start_date_str[11];
    char end_date_str[11];
    snprintf(start_date_str, sizeof(start_date_str), "%04d-%02d-%02d",
        start_date.tm_year + 1900, start_date.tm_mon + 1, start_date.tm_mday);
    snprintf(end_date_str, sizeof(end_date_str), "%04d-%02d-%02d",
        end_date.tm_year + 1900, end_date.tm_mon + 1, end_date.tm_mday);

    // 1. Statement 핸들 할당
    if (OCIHandleAlloc(envhp, (void**)&stmthp, OCI_HTYPE_STMT, 0, NULL) != OCI_SUCCESS) {
        fprintf(stderr, "1. Failed to allocate statement handle.\n");
        return;
    }

    // 2. SQL 문 준비
    if (OCIStmtPrepare(stmthp, errhp, (text*)select_sql, (ub4)strlen(select_sql), OCI_NTV_SYNTAX, OCI_DEFAULT) != OCI_SUCCESS) {
        fprintf(stderr, "2. Failed to prepare SQL statement.\n");
        OCIHandleFree(stmthp, OCI_HTYPE_STMT);
        return;
    }

    // 3. 바인딩
    if (OCIBindByPos(stmthp, &bnd1, errhp, 1, (void*)&stock_id, sizeof(stock_id), SQLT_INT, NULL, NULL, NULL, 0, NULL, OCI_DEFAULT) != OCI_SUCCESS ||
        OCIBindByPos(stmthp, &bnd2, errhp, 2, (void*)start_date_str, strlen(start_date_str) + 1, SQLT_CHR, NULL, NULL, NULL, 0, NULL, OCI_DEFAULT) != OCI_SUCCESS ||
        OCIBindByPos(stmthp, &bnd3, errhp, 3, (void*)end_date_str, strlen(end_date_str) + 1, SQLT_CHR, NULL, NULL, NULL, 0, NULL, OCI_DEFAULT) != OCI_SUCCESS) {
        fprintf(stderr, "3. Failed to bind parameters.\n");
        OCIHandleFree(stmthp, OCI_HTYPE_STMT);
        return;
    }

    // 4. SQL 실행
    sword status = OCIStmtExecute(svchp, stmthp, errhp, 0, 0, NULL, NULL, OCI_DEFAULT);
    if (status != OCI_SUCCESS) {
        print_oci_error(errhp, status);
        OCIHandleFree(stmthp, OCI_HTYPE_STMT);
        return;
    }

    // 5. 결과 정의
    if (OCIDefineByPos(stmthp, &def1, errhp, 1, &closing_price, sizeof(closing_price), SQLT_INT, NULL, NULL, NULL, OCI_DEFAULT) != OCI_SUCCESS ||
        OCIDefineByPos(stmthp, &def2, errhp, 2, closing_date_str, sizeof(closing_date_str), SQLT_STR, NULL, NULL, NULL, OCI_DEFAULT) != OCI_SUCCESS) {
        fprintf(stderr, "5. Failed to define output.\n");
        OCIHandleFree(stmthp, OCI_HTYPE_STMT);
        return;
    }

    // 6. 결과 가져와 배열에 저장
    int i = 0;
    while (OCIStmtFetch(stmthp, errhp, 1, OCI_FETCH_NEXT, OCI_DEFAULT) == OCI_SUCCESS) {
        if (i < max_size) {
            closing_prices[i].closing_price = closing_price;

            // 날짜 변환 (YYYY-MM-DD → struct tm)
            sscanf(closing_date_str, "%4d-%2d-%2d",
                &closing_prices[i].closing_date.tm_year,
                &closing_prices[i].closing_date.tm_mon,
                &closing_prices[i].closing_date.tm_mday);
            closing_prices[i].closing_date.tm_year -= 1900;
            closing_prices[i].closing_date.tm_mon -= 1;

            i++;
        }
        else {
            fprintf(stderr, "Warning: Result set exceeds buffer size (%d). Truncating results.\n", max_size);
            break;
        }
    }

    // 7. 정리
    OCIHandleFree(stmthp, OCI_HTYPE_STMT);
}


//double select_weight(OCIEnv* envhp, OCISvcCtx* svchp, OCIError* errhp, int stock_id, int portfolio_id) {
//    OCIStmt* stmthp = NULL;
//    OCIDefine* def1 = NULL;
//    OCIBind* bnd1 = NULL, * bnd2 = NULL;
//    double weight = -1.0; // 기본값 설정 (조회 실패 시 -1 반환)
//
//    char* select_sql =
//        "SELECT ratio FROM holdings WHERE stock_id = :1 AND portfolio_id = :2";
//
//    // 1. Statement 핸들 할당
//    if (OCIHandleAlloc(envhp, (void**)&stmthp, OCI_HTYPE_STMT, 0, NULL) != OCI_SUCCESS) {
//        OCIErrorGet(errhp, 1, NULL, NULL, NULL, 0, OCI_HTYPE_ERROR);
//        fprintf(stderr, "1.Failed to allocate statement handle.\n");
//        return weight;
//    }
//
//    // 2. SQL 문 준비
//    if (OCIStmtPrepare(stmthp, errhp, (text*)select_sql, (ub4)strlen(select_sql), OCI_NTV_SYNTAX, OCI_DEFAULT) != OCI_SUCCESS) {
//        OCIErrorGet(errhp, 1, NULL, NULL, NULL, 0, OCI_HTYPE_ERROR);
//        fprintf(stderr, "2.Failed to prepare SQL statement.\n");
//        OCIHandleFree(stmthp, OCI_HTYPE_STMT);
//        return weight;
//    }
//
//    // 3. 바인딩
//    if (OCIBindByPos(stmthp, &bnd1, errhp, 1, (void*)&stock_id, sizeof(stock_id), SQLT_INT, NULL, NULL, NULL, 0, NULL, OCI_DEFAULT) != OCI_SUCCESS ||
//        OCIBindByPos(stmthp, &bnd2, errhp, 2, (void*)&portfolio_id, sizeof(portfolio_id), SQLT_INT, NULL, NULL, NULL, 0, NULL, OCI_DEFAULT) != OCI_SUCCESS) {
//        OCIErrorGet(errhp, 1, NULL, NULL, NULL, 0, OCI_HTYPE_ERROR);
//        fprintf(stderr, "3.Failed to bind parameters.\n");
//        OCIHandleFree(stmthp, OCI_HTYPE_STMT);
//        return weight;
//    }
//
//    // 4. SQL 실행
//    sword status = OCIStmtExecute(svchp, stmthp, errhp, 0, 0, NULL, NULL, OCI_DEFAULT);
//    if (status != OCI_SUCCESS) {
//        print_oci_error(errhp, status);
//        //fprintf(stderr, "4.Failed to execute SQL statement.\n");
//        OCIHandleFree(stmthp, OCI_HTYPE_STMT);
//        return;
//    }
//
//    // 5. 결과 정의 (SQLT_FLT 사용)
//    if (OCIDefineByPos(stmthp, &def1, errhp, 1, &weight, sizeof(weight), SQLT_FLT, NULL, NULL, NULL, OCI_DEFAULT) != OCI_SUCCESS) {
//        OCIErrorGet(errhp, 1, NULL, NULL, NULL, 0, OCI_HTYPE_ERROR);
//        fprintf(stderr, "5.Failed to define output.\n");
//        OCIHandleFree(stmthp, OCI_HTYPE_STMT);
//        return weight;
//    }
//
//    // 6. 결과 가져오기
//    if (OCIStmtFetch(stmthp, errhp, 1, OCI_FETCH_NEXT, OCI_DEFAULT) != OCI_SUCCESS) {
//        if (OCIStmtFetch(stmthp, errhp, 1, OCI_FETCH_NEXT, OCI_DEFAULT) == OCI_NO_DATA) {
//            fprintf(stderr, "No data found for stock_id: %d, portfolio_id: %d\n", stock_id, portfolio_id);
//        }
//        else {
//            OCIErrorGet(errhp, 1, NULL, NULL, NULL, 0, OCI_HTYPE_ERROR);
//            fprintf(stderr, "Error fetching data for stock_id: %d, portfolio_id: %d\n", stock_id, portfolio_id);
//        }
//        weight = -1.0; // 조회 결과가 없으면 -1 반환
//    }
//
//    // 7. 리소스 해제
//    OCIHandleFree(stmthp, OCI_HTYPE_STMT);
//    OCIHandleFree(bnd1, OCI_HTYPE_BIND);
//    OCIHandleFree(bnd2, OCI_HTYPE_BIND);
//    OCIHandleFree(def1, OCI_HTYPE_DEFINE);
//
//    return weight; // 조회한 weight 값 반환
//}
