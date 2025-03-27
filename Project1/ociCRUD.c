#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <oci.h>
#include "ociCRUD.h"

#define DB_USER "C##CPJT"
#define DB_PASS "1234"
#define DB_CONN "//192.168.31.101:1521/xe"

static OCIEnv* envhp = NULL;
static OCIError* errhp = NULL;
static OCISvcCtx* svchp = NULL;
static OCIServer* srvhp = NULL;
static OCISession* usrhp = NULL;

void print_oci_error(OCIError* errhp, sword status) {
    text errbuf[512]; sb4 errcode = 0;
    OCIErrorGet(errhp, 1, NULL, &errcode, errbuf, sizeof(errbuf), OCI_HTYPE_ERROR);
    printf("[OCI ERROR %d] %s\n", errcode, errbuf);
}

int oci_init() {
    if (OCIEnvNlsCreate(&envhp, OCI_DEFAULT, NULL, NULL, NULL, NULL, 0, NULL, 873, 873) != OCI_SUCCESS) return 0;
    OCIHandleAlloc(envhp, (void**)&errhp, OCI_HTYPE_ERROR, 0, NULL);
    OCIHandleAlloc(envhp, (void**)&srvhp, OCI_HTYPE_SERVER, 0, NULL);
    OCIHandleAlloc(envhp, (void**)&svchp, OCI_HTYPE_SVCCTX, 0, NULL);

    if (OCIServerAttach(srvhp, errhp, (text*)DB_CONN, strlen(DB_CONN), OCI_DEFAULT) != OCI_SUCCESS) {
        print_oci_error(errhp, OCI_ERROR);
        return 0;
    }
    OCIAttrSet(svchp, OCI_HTYPE_SVCCTX, srvhp, 0, OCI_ATTR_SERVER, errhp);
    OCIHandleAlloc(envhp, (void**)&usrhp, OCI_HTYPE_SESSION, 0, NULL);
    OCIAttrSet(usrhp, OCI_HTYPE_SESSION, (void*)DB_USER, strlen(DB_USER), OCI_ATTR_USERNAME, errhp);
    OCIAttrSet(usrhp, OCI_HTYPE_SESSION, (void*)DB_PASS, strlen(DB_PASS), OCI_ATTR_PASSWORD, errhp);

    if (OCISessionBegin(svchp, errhp, usrhp, OCI_CRED_RDBMS, OCI_DEFAULT) != OCI_SUCCESS) {
        print_oci_error(errhp, OCI_ERROR);
        return 0;
    }
    OCIAttrSet(svchp, OCI_HTYPE_SVCCTX, usrhp, 0, OCI_ATTR_SESSION, errhp);
    return 1;
}

void oci_cleanup() {
    if (usrhp) OCISessionEnd(svchp, errhp, usrhp, OCI_DEFAULT);
    if (srvhp) OCIServerDetach(srvhp, errhp, OCI_DEFAULT);
    if (usrhp) OCIHandleFree(usrhp, OCI_HTYPE_SESSION);
    if (svchp) OCIHandleFree(svchp, OCI_HTYPE_SVCCTX);
    if (srvhp) OCIHandleFree(srvhp, OCI_HTYPE_SERVER);
    if (errhp) OCIHandleFree(errhp, OCI_HTYPE_ERROR);
    if (envhp) OCIHandleFree(envhp, OCI_HTYPE_ENV);
}

int login_user(const char* user_id, const char* password) {
    if (!oci_init()) return 0;
    const char* sql = "SELECT COUNT(*) FROM USERS WHERE USER_ID = :1 AND PASSWORD = :2";
    OCIStmt* stmthp; OCIDefine* defnp = NULL; int count = 0;
    OCIHandleAlloc(envhp, (void**)&stmthp, OCI_HTYPE_STMT, 0, NULL);
    OCIStmtPrepare(stmthp, errhp, (text*)sql, strlen(sql), OCI_NTV_SYNTAX, OCI_DEFAULT);
    OCIBind* bnd1 = NULL, * bnd2 = NULL;
    OCIBindByPos(stmthp, &bnd1, errhp, 1, (void*)user_id, strlen(user_id) + 1, SQLT_STR, NULL, NULL, NULL, 0, NULL, OCI_DEFAULT);
    OCIBindByPos(stmthp, &bnd2, errhp, 2, (void*)password, strlen(password) + 1, SQLT_STR, NULL, NULL, NULL, 0, NULL, OCI_DEFAULT);
    OCIDefineByPos(stmthp, &defnp, errhp, 1, &count, sizeof(count), SQLT_INT, NULL, NULL, NULL, OCI_DEFAULT);
    sword status = OCIStmtExecute(svchp, stmthp, errhp, 1, 0, NULL, NULL, OCI_DEFAULT);
    OCIHandleFree(stmthp, OCI_HTYPE_STMT);
    oci_cleanup();
    return count > 0;
}

int register_user(const char* user_id, const char* password) {
    if (!oci_init()) return 0;
    OCIStmt* stmthp; OCIDefine* defnp = NULL; int exists = 0;
    const char* check_sql = "SELECT COUNT(*) FROM USERS WHERE USER_ID = :1";
    OCIHandleAlloc(envhp, (void**)&stmthp, OCI_HTYPE_STMT, 0, NULL);
    OCIStmtPrepare(stmthp, errhp, (text*)check_sql, strlen(check_sql), OCI_NTV_SYNTAX, OCI_DEFAULT);
    OCIBind* bnd_check = NULL;
    OCIBindByPos(stmthp, &bnd_check, errhp, 1, (void*)user_id, strlen(user_id) + 1, SQLT_STR, NULL, NULL, NULL, 0, NULL, OCI_DEFAULT);
    OCIDefineByPos(stmthp, &defnp, errhp, 1, &exists, sizeof(exists), SQLT_INT, NULL, NULL, NULL, OCI_DEFAULT);
    OCIStmtExecute(svchp, stmthp, errhp, 1, 0, NULL, NULL, OCI_DEFAULT);
    OCIHandleFree(stmthp, OCI_HTYPE_STMT);
    if (exists > 0) { oci_cleanup(); return 0; }

    const char* insert_sql = "INSERT INTO USERS (USER_ID, PASSWORD) VALUES (:1, :2)";
    OCIHandleAlloc(envhp, (void**)&stmthp, OCI_HTYPE_STMT, 0, NULL);
    OCIStmtPrepare(stmthp, errhp, (text*)insert_sql, strlen(insert_sql), OCI_NTV_SYNTAX, OCI_DEFAULT);
    OCIBind* bnd1 = NULL, * bnd2 = NULL;
    OCIBindByPos(stmthp, &bnd1, errhp, 1, (void*)user_id, strlen(user_id) + 1, SQLT_STR, NULL, NULL, NULL, 0, NULL, OCI_DEFAULT);
    OCIBindByPos(stmthp, &bnd2, errhp, 2, (void*)password, strlen(password) + 1, SQLT_STR, NULL, NULL, NULL, 0, NULL, OCI_DEFAULT);
    sword status = OCIStmtExecute(svchp, stmthp, errhp, 1, 0, NULL, NULL, OCI_DEFAULT);
    if (status != OCI_SUCCESS) {
        print_oci_error(errhp, status);
        OCIHandleFree(stmthp, OCI_HTYPE_STMT);
        oci_cleanup();
        return 0;
    }
    OCITransCommit(svchp, errhp, OCI_DEFAULT);
    OCIHandleFree(stmthp, OCI_HTYPE_STMT);
    oci_cleanup();
    return 1;
}

int get_all_stocks(char stock_names[][32], int max_count) {
    if (!oci_init()) return 0;
    const char* sql = "SELECT STOCK_NAME FROM STOCKS ORDER BY STOCK_ID";
    OCIStmt* stmthp; OCIDefine* defnp = NULL;
    OCIHandleAlloc(envhp, (void**)&stmthp, OCI_HTYPE_STMT, 0, NULL);
    OCIStmtPrepare(stmthp, errhp, (text*)sql, strlen(sql), OCI_NTV_SYNTAX, OCI_DEFAULT);
    sword status = OCIStmtExecute(svchp, stmthp, errhp, 0, 0, NULL, NULL, OCI_DEFAULT);
    if (status != OCI_SUCCESS) {
        print_oci_error(errhp, status);
        OCIHandleFree(stmthp, OCI_HTYPE_STMT);
        oci_cleanup();
        return 0;
    }
    int i = 0;
    char name[32];
    OCIDefineByPos(stmthp, &defnp, errhp, 1, name, sizeof(name), SQLT_CHR, NULL, NULL, NULL, OCI_DEFAULT);
    while ((status = OCIStmtFetch2(stmthp, errhp, 1, OCI_FETCH_NEXT, 0, OCI_DEFAULT)) != OCI_NO_DATA && i < max_count) {
        if (status != OCI_SUCCESS && status != OCI_SUCCESS_WITH_INFO) break;
        strncpy(stock_names[i], name, 32);
        stock_names[i][31] = '\0';
        i++;
    }
    OCIHandleFree(stmthp, OCI_HTYPE_STMT);
    oci_cleanup();
    return i;
}