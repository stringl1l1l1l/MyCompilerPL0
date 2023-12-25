#ifndef _ERROR_HANDLER_H
#define _ERROR_HANDLER_H

// 错误种类宏定义
#define EXPECT_STH_FIND_ANTH 0
#define EXPECT 1
#define EXPECT_NUMEBR_AFTER_BECOMES 2
#define ILLEGAL_DEFINE 3
#define ILLEGAL_WORD 4
#define ILLEGAL_RVALUE_ASSIGN 5
#define MISSING 6
#define REDUNDENT 7
#define UNDECLARED_IDENT 8
#define UNDECLARED_PROC 9
#define REDECLEARED_IDENT 10
#define REDECLEARED_PROC 11
#define INCOMPATIBLE_VAR_LIST 12
#define UNDEFINED_PROC 13

extern unsigned int err_cnt; // 出错总次数

void error(unsigned int n);
void error(unsigned int n, const wchar_t*);
void error(unsigned int n, const wchar_t*, const wchar_t*);
void initErrorHandler();
void over();

#endif