#include "PL0.h"
#include "ErrorHandler.h"
#include "Interpreter.h"
#include "Lexer.h"
#include "PCode.h"
#include "Parser.h"
#include "SymTable.h"

// 所有部件的全局变量
size_t level; //  层差
size_t glo_offset; // 全局偏移量
wstring progm_w_str; // 源程序代码的wchar字符串形式

// first集
unsigned long first_block = CONST_SYM | VAR_SYM | PROC_SYM | BEGIN_SYM;
unsigned long first_stmt = IDENT | IF_SYM | WHILE_SYM | CALL_SYM | BEGIN_SYM | READ_SYM | WRITE_SYM;
unsigned long first_factor = IDENT | NUMBER | LPAREN;
unsigned long first_term = first_factor;
unsigned long first_exp = first_term | PLUS | MINUS;
unsigned long first_lexp = first_exp | ODD_SYM;
unsigned long first_lop = EQL | NEQ | LSS | LEQ | GRT | GEQ;

// follow集
unsigned long follow_condecl = VAR_SYM | PROC_SYM | BEGIN_SYM;
unsigned long follow_vardecl = PROC_SYM | BEGIN_SYM;
unsigned long follow_body = SEMICOLON | END_SYM | ELSE_SYM;
unsigned long follow_stmt = END_SYM | SEMICOLON | ELSE_SYM;
unsigned long follow_lexp = THEN_SYM | DO_SYM;
unsigned long follow_exp = follow_stmt | COMMA | RPAREN | first_lop | follow_lexp;
unsigned long follow_term = follow_exp | PLUS | MINUS;
unsigned long follow_factor = MULTI | DIVIS | follow_term;
unsigned long follow_ident = COMMA | SEMICOLON | LPAREN | RPAREN | follow_factor;
unsigned long follow_block = NUL | SEMICOLON;
unsigned long follow_constdef = COMMA | SEMICOLON;
unsigned long follow_proc = BEGIN_SYM;

// 局部变量
ifstream file;

void init()
{
    // 全局变量初始化
    level = 0;
    glo_offset = 0;
    // 清空原来的文件字符串
    progm_w_str.clear();
    initLexer();
    initErrorHandler();
    // 清空符号表
    SymTable::clear();
    // 清空中间代码单元
    PCodeList::clear();
    // 清空运行单元
    Interpreter::clear();
    // 以Unicode方式打开输入输出流
    _setmode(_fileno(stdout), _O_U16TEXT);
}

/*UTF8 编码格式（xxx 是用来填充二进制 Unicode 码点的）
1字节	0xxxxxxx
2字节	110xxxxx 10xxxxxx
3字节	1110xxxx 10xxxxxx 10xxxxxx
4字节	11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
有效的 Unicode 码点范围:
0000 0000 ~ 0000 007F | 0xxxxxxx
0000 0080 ~ 0000 07FF | 110xxxxx 10xxxxxx
0000 0800 ~ 0000 FFFF | 1110xxxx 10xxxxxx 10xxxxxx
0001 0000 ~ 0010 FFFF | 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
*/
// 读取 UTF8 文件, 返回Unicode（UCS-2）字符串
void readFile2USC2(string filename)
{
    // 打开文件
    file.open(filename);
    if (!file.is_open()) {
        wcout << L"cannot open file!" << endl;
        return;
    }
    wcout << L"\e[32mCompiling file '" << filename.c_str() << L"'!\e[0m" << endl;

    // 禁止过滤空白符
    file >> noskipws;
    // 跳过 UTF8 BOM（0xEFBBBF）
    if (file.get() != 0xEF || file.get() != 0xBB || file.get() != 0xBF) {
        file.seekg(0, ios::beg);
    }

    byte B; // 1字节
    wchar_t wchar; // 2字节存储UCS-2码点
    wstring w_str(L""); // 用于存储转换结果的 Unicode 码点序列
    int len; // 单个 UTF8 字符的编码长度

    while ((file >> B) && !file.eof()) {
        // 单字节编码 0xxxxxxx
        if (B < 0b10000000) {
            wchar = B;
        } // 多字节编码，获取编码长度
        else {
            // 超出可用 Unicode 范围
            if (B > 0b11110100) {
                wcout << L"Invalid unicode range" << endl;
                return;
            } // 4字节编码 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
            else if (B >= 0b11110000) {
                len = 4;
            }
            // 1110xxxx 10xxxxxx 10xxxxxx
            else if (B >= 0b11100000) {
                len = 3;
            }
            // 110xxxxx 10xxxxxx
            else if (B >= 0b11000000) {
                len = 2;
            } else {
                // 除单字节外，首字节不能小于 0b11000000
                wcout << L"Invalid utf8 leading code" << endl;
                return;
            }
            // 通过左移再右移的方法去掉首字节中的 UTF8 标记
            B = B << (len + 1);
            wchar = B >> (len + 1);
            // 处理后续字节
            while (len > 1) {
                B = file.get();
                // 如果 f 到达 eof，则 c 会返回 255
                // 后续编码必须是 0b10xxxxxx 格式
                if (B >= 0b11000000) {
                    wcout << L"Invalid utf8 tailing code" << endl;
                    return;
                }
                len--;
                B = B & 0b00111111; // 去掉 UTF8 标记
                wchar = wchar << 6; // 腾出 6 个 bit 的位置
                wchar += B; // 将去掉了 UTF8 标记的编码合并进来
            }
        }
        // 存储解解析结果
        w_str.push_back(wchar);
    }
    w_str.push_back(L'#');
    progm_w_str = w_str;
    file.clear();
    // file.seekg(0, file.beg);
    file.close();
}

// // 读取待编译源代码文件
// void readFile2Str()
// {
//     fstream file(PRGM_PATH, ios::in);
//     stringstream stream;
//     if (!file.is_open()) {
//         cout << "cannot open file!" << endl;
//         exit(0);
//         cout << "cannot open file!" << endl;
//     }
//     stream << file.rdbuf() << '#';
//     progm_str = stream.str();
//     progm_lenth = progm_str.length();
//     progm_c_str = new wchar_t[progm_lenth];
//     strcpy(progm_c_str, progm_str.c_str());
// }