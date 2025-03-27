#pragma once
#ifndef DB_H
#define DB_H
#include <oci.h>
#include "calculation.h"

void check_error(OCIError* errhp);

void connect_db(OCIEnv** envhp, OCIError** errhp, OCISvcCtx** svchp, OCISession** usrhp, OCIServer** srvhp,
    const char* username, const char* password, const char* dbname);

void select_closing_price(OCIEnv* envhp, OCISvcCtx* svchp, OCIError* errhp,
    int stock_id, struct tm start_date, struct tm end_date, int max_size,
    int* closing_prices, int* num_results);

double select_weight(OCIEnv* envhp, OCISvcCtx* svchp, OCIError* errhp, int stock_id, int portfolio_id);

void disconnect_db(OCIEnv* envhp, OCIError* errhp, OCISvcCtx* svchp, OCISession* usrhp, OCIServer* srvhp);

#endif