#include "ErrorHandler.h"
#include "Lexer.h"
#include "PL0.h"

unsigned int err_cnt; // 出错总次数
wstring err_msg[ERR_CNT]; // 错误信息表

void initErrorHandler()
{
    err_cnt = 0;
    // 报错信息初始化
    // missing错误
    err_msg[MISSING] = L"Missing %s";
    // undeclare错误
    err_msg[UNDECLARED_IDENT] = L"Undeclared identifier '%s'";
    err_msg[UNDECLARED_PROC] = L"Undeclared procedure name '%s'";
    // redefined错误
    err_msg[REDECLEARED_IDENT] = L"Redecleared identifier '%s'";
    err_msg[REDECLEARED_PROC] = L"Redecleared procedure name '%s'";
    // illegal错误
    err_msg[ILLEGAL_DEFINE] = L"Illegal %s definition ";
    err_msg[ILLEGAL_WORD] = L"Illegal word %s";
    err_msg[ILLEGAL_RVALUE_ASSIGN] = L"Cannot assign a rvalue";
    // expect错误
    err_msg[EXPECT] = L"Expecting %s";
    err_msg[EXPECT_STH_FIND_ANTH] = L"Expecting %s but %s was found";
    // redundant错误
    err_msg[REDUNDENT] = L"Redundent %s";
    // 其他错误
    err_msg[INCOMPATIBLE_VAR_LIST] = L"The real variable list is incompatible with formal variable list";
    err_msg[UNDEFINED_PROC] = L"Calling undefined procedure '%s'";
}

// 打印错误信息
void printPreWord(const wchar_t msg[])
{
    wcout << L"\e[31m(" << pre_word_row << "," << pre_word_col << L")"
          << L" Error: " << msg << L"\e[0m " << endl;
}

void printCurWord(const wchar_t msg[])
{
    wcout << L"\e[31m(" << row_pos << "," << col_pos << L")"
          << L" Error: " << msg << L"\e[0m " << endl;
}

template <class... T>
void error(unsigned int n, T... extra)
{
    wchar_t msg[200] = L"";
    wsprintf(msg, err_msg[n].c_str(), extra...);
    err_cnt++;
    if (n == REDUNDENT || n == MISSING || n == UNDECLARED_PROC)
        printPreWord(msg);
    else
        printCurWord(msg);
}

void error(unsigned int n)
{
    wchar_t msg[200] = L"";
    wsprintfW(msg, err_msg[n].c_str());
    err_cnt++;
    if (n == REDUNDENT || n == MISSING || n == UNDECLARED_PROC)
        printPreWord(msg);
    else
        printCurWord(msg);
}

void error(unsigned int n, const wchar_t* extra)
{
    wchar_t msg[200] = L"";
    wsprintfW(msg, err_msg[n].c_str(), extra);
    err_cnt++;
    if (n == REDUNDENT || n == MISSING || n == UNDECLARED_PROC)
        printPreWord(msg);
    else
        printCurWord(msg);
}

void error(unsigned int n, const wchar_t* extra1, const wchar_t* extra2)
{
    wchar_t msg[200] = L"";
    wsprintfW(msg, err_msg[n].c_str(), extra1, extra2);
    err_cnt++;
    if (n == REDUNDENT || n == MISSING || n == UNDECLARED_PROC)
        printPreWord(msg);
    else
        printCurWord(msg);
}
// 格式化输出错误分析结果
void over()
{
    if (err_cnt == 0) {
        wcout << L"\e[32mNo error. Congratulations!\e[0m" << endl;
        wcout << L"\e[32m______________________________Compile compelete!________________________________\e[0m\n"
              << endl;
    } else {
        wcout << L"\e[31mTotol: " << err_cnt << L" errors\e[0m" << endl;
        wcout << L"\e[33m_______________________________Compile failed!_________________________________\e[0m\n"
              << endl;
    }
}