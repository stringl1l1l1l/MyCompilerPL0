#ifndef _LEXER_H
#define _LEXER_H
#include <string>
using namespace std;

#define NUL 0x0 /* 空 */
#define EQL 0x1 /* =  1*/
#define NEQ 0x2 /* <> 2*/
#define LSS 0x4 /* < 4*/
#define LEQ 0x8 /* <= 8*/
#define GRT 0x10 /* > 16*/
#define GEQ 0x20 /* >= 32*/
#define ODD_SYM 0x40 /* 奇数判断 64*/
#define IDENT 0x80 /* 标识符 */
#define NUMBER 0x100 /* 数值 */
#define PLUS 0x200 /* + */
#define MINUS 0x400 /* - */
#define MULTI 0x800 /* * */
#define DIVIS 0x1000 /* / */
#define LPAREN 0x2000 /* ( */
#define RPAREN 0x4000 /* ) */
#define COMMA 0x8000 /* , */
#define SEMICOLON 0x10000 /* ; */
#define ASSIGN 0x20000 /*:=*/

#define BEGIN_SYM 0x40000
#define END_SYM 0x80000
#define IF_SYM 0x100000
#define THEN_SYM 0x200000
#define WHILE_SYM 0x400000
#define DO_SYM 0x800000
#define CALL_SYM 0x1000000
#define CONST_SYM 0x2000000
#define VAR_SYM 0x4000000
#define PROC_SYM 0x8000000
#define WRITE_SYM 010000000
#define READ_SYM 0x20000000
#define PROGM_SYM 0x40000000
#define ELSE_SYM 0x80000000

extern wchar_t w_ch; // 用于词法分析器，存放最近一次从文件中读出的字符
extern unsigned long sym; // 用于词法分析器，存放最近一次从文件中读出的字符
extern wstring strToken; // 最近一次识别出来的token的名字
extern size_t col_pos; // 列指针
extern size_t row_pos; // 行指针
extern size_t pre_word_col; // 上一个非空白合法词尾列指针
extern size_t pre_word_row; // 上一个非空白合法词行指针
extern size_t ch_ptr; // 源程序字符串读指针

bool isDigit(wchar_t ch);
bool isLetter(wchar_t ch);
bool isTerminate(wchar_t ch);
int isOprator(wchar_t ch);
void skipBlank();
void getCh();
void retract();
void contract();
int reserve(wstring str);
void getWord();
void initLexer();

#endif