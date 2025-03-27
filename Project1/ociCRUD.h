#pragma once
#ifndef OCI_CRUD_H
#define OCI_CRUD_H

int login_user(const char* user_id, const char* password);
int register_user(const char* user_id, const char* password);
int get_all_stocks(char stock_names[][32], int max_count);


#endif