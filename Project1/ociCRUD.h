#pragma once
#ifndef OCI_CRUD_H
#define OCI_CRUD_H

int login_user(const char* user_id, const char* password);
int register_user(const char* user_id, const char* password);
int get_all_stocks(char stock_names[][32], int max_count);
int get_popular_stocks(char stock_names[][32], int stock_counts[], int max_count);
int insert_stock(const char* stock_name);
int update_stock(int stock_id, const char* new_name);
int delete_stock(int stock_id);



#endif