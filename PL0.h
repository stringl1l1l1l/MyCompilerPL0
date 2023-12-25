#ifndef _PL0_H
#define _PL0_H
#include <cstddef>
#include <fcntl.h>
#include <fstream>
#include <io.h>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <tchar.h>
#include <unordered_map>
#include <vector>
#include <windows.h>
#include <winuser.h>
#ifndef UNICODE
#define UNICODE
#endif

#ifndef _UNICODE
#define _UNICODE
#endif
using namespace std;

#define PROGM_PATH \
    "E:\\Programming\\GitHub\\repository\\DataStruct\\complierPL0\\testfib.txt"
#define RSV_WORD_MAX 15 /* 保留字的数量 */
#define N_MAX 14 /* 数字允许的最长位数 */
#define PROGM_CH_MAX 5000 /*源程序的最大字符数*/
#define ID_MAX 20 /* 标识符最大长度 */
#define OPR_MAX 11 /* 运算符号表最大长度*/
#define VAR_WIDTH 4 /*变量大小*/
#define CST_WIDTH 4 /*常量大小*/
#define SYM_ITEMS_CNT 100 // 符号表项数
#define PROC_CNT 40 // 过程嵌套数
#define ERR_CNT 70 // 报错种数

// 全局变量声明
extern size_t level; // 层差
extern size_t glo_offset; // 全局offset
extern wstring progm_w_str; // 源程序代码的wchar字符串形式

// first集声明
extern unsigned long first_block;
extern unsigned long first_stmt;
extern unsigned long first_factor;
extern unsigned long first_term;
extern unsigned long first_exp;
extern unsigned long first_lexp;
extern unsigned long first_lop;
// follow集声明
extern unsigned long follow_condecl;
extern unsigned long follow_vardecl;
extern unsigned long follow_body;
extern unsigned long follow_stmt;
extern unsigned long follow_lexp;
extern unsigned long follow_exp;
extern unsigned long follow_term;
extern unsigned long follow_factor;
extern unsigned long follow_ident;
extern unsigned long follow_block;
extern unsigned long follow_constdef;
extern unsigned long follow_proc;

// 函数声明
void init();
void readFile2USC2(string);
void PL0Test();
#endif