#include "Lexer.h"
#include "ErrorHandler.h"
#include "PL0.h"

wchar_t w_ch; // 用于词法分析器，存放最近一次从文件中读出的字符
unsigned long sym; // 最近一次识别出来的 token 的类型
wstring strToken; // 最近一次识别出来的标识符的名字
size_t ch_ptr; // 字符指针，指向词法分析当前读取的字符
size_t col_pos; // 列指针
size_t row_pos; // 行指针
size_t pre_word_col; // 上一个非空白合法词尾列指针
size_t pre_word_row; // 上一个非空白合法词行指针

unordered_map<unsigned long, wstring> sym_map; // 保留字编号与字符串的映射
// 保留字表
wstring resv_table[RSV_WORD_MAX] = {
    L"odd", L"begin", L"end", L"if", L"then", L"while", L"do", L"call",
    L"const", L"var", L"procedure", L"write", L"read", L"program", L"else"
};

// 运算符号表
wchar_t opr_table[OPR_MAX] = { L'+', L'-', L'*', L'/', L'=', L'<',
    L'>', L'(', L')', L',', L';' };

void initLexer()
{
    // 变量初始化
    w_ch = L' '; // 用于词法分析器，存放最近一次从文件中读出的字符
    sym = NUL; // 最近一次识别出来的 token 的类型
    strToken = L""; // 最近一次识别出来的标识符的名字
    col_pos = 0; // 列指针
    row_pos = 1; // 行指针
    pre_word_col = 0; // 上一个非空白合法词尾列指针
    pre_word_row = 1; // 上一个非空白合法词行指针
    ch_ptr = 0;

    // 符号名表初始化
    sym_map[NUL] = L"NUL";
    sym_map[IDENT] = L"IDENT";
    sym_map[NUMBER] = L"NUMBER";
    sym_map[PLUS] = L"PLUS";
    sym_map[MINUS] = L"MINUS";
    sym_map[MULTI] = L"MULTI";
    sym_map[DIVIS] = L"DIVIS";
    sym_map[ODD_SYM] = L"ODD_SYM";
    sym_map[EQL] = L"EQL";
    sym_map[NEQ] = L"NEQ";
    sym_map[LSS] = L"LSS";
    sym_map[LEQ] = L"LEQ";
    sym_map[GRT] = L"GRT";
    sym_map[GEQ] = L"GEQ";
    sym_map[LPAREN] = L"LPAREN";
    sym_map[RPAREN] = L"RPAREN";
    sym_map[COMMA] = L"COMMA";
    sym_map[SEMICOLON] = L"SEMICOLON";
    sym_map[ASSIGN] = L"BECOMES";
    sym_map[BEGIN_SYM] = L"BEGIN_SYM";
    sym_map[END_SYM] = L"END_SYM";
    sym_map[IF_SYM] = L"IF_SYM";
    sym_map[THEN_SYM] = L"THEN_SYM";
    sym_map[WHILE_SYM] = L"WHILE_SYM";
    sym_map[DO_SYM] = L"DO_SYM";
    sym_map[CALL_SYM] = L"CALL_SYM";
    sym_map[CONST_SYM] = L"CONST_SYM";
    sym_map[VAR_SYM] = L"VAR_SYM";
    sym_map[PROC_SYM] = L"PROC_SYM";
    sym_map[WRITE_SYM] = L"WRITE_SYM";
    sym_map[READ_SYM] = L"READ_SYM";
    sym_map[PROGM_SYM] = L"PROGM_SYM";
    sym_map[ELSE_SYM] = L"ELSE_SYM";
}

// 判断是否为数字
bool isDigit(wchar_t ch)
{
    if (ch >= L'0' && ch <= L'9')
        return true;
    else
        return false;
}

// 判断是否为字母
bool isLetter(wchar_t ch)
{
    if ((ch >= L'a' && ch <= L'z') || (ch >= L'A' && ch <= L'Z'))
        return true;
    else
        return false;
}

// 判断是否为终止符
bool isTerminate(wchar_t ch)
{
    return ch == L' ' || ch == L'\t' || ch == L'\n' || ch == L'#' || ch == L'\0' || ch == L';' || ch == L',';
}

// 判断是否为运算符，是则返回在表中位置，否则返回-1
int isOprator(wchar_t ch)
{
    wchar_t* p = opr_table;
    while (p - opr_table != OPR_MAX) {
        if (*p == ch) {
            return p - opr_table;
        }
        p++;
    }
    return -1;
}

// 跳过所有空白符，并将读指针置于空白符下一个位置，但并不装载下一个字符
void skipBlank()
{
    while (progm_w_str[ch_ptr] && (progm_w_str[ch_ptr] == L' ' || progm_w_str[ch_ptr] == L'\t')) {
        col_pos++;
        ch_ptr++;
    }
}

// 获取ch_ptr对应位置的字符，并移动读指针到下一位置。若当前字符为回车符，则跳过当前字符
void getCh()
{
    w_ch = progm_w_str[ch_ptr++];
    col_pos++;
}

// 退一个字符
void retract()
{
    w_ch = progm_w_str[--ch_ptr];
    col_pos--;
}

// 将当前字符追加到token中
void contract()
{
    strToken += w_ch;
}

// 查询str是否为保留字，是保留字返回保留字在表中位置，否则返回-1
int reserve(wstring str)
{
    for (int i = 0; i < RSV_WORD_MAX; i++) {
        if (resv_table[i] == str) {
            return i;
        }
    }
    return -1;
}

// 词法分析器
void getWord()
{
    if (w_ch != L'\n') {
        pre_word_col = col_pos;
        pre_word_row = row_pos;
    }
    strToken.clear();
    skipBlank();
    getCh();
    if (w_ch == L'\0')
        return;
    // 跳过连续的回车符
    if (w_ch == L'\n') {
        col_pos = 0;
        row_pos++;
        getWord();
        return;
    } else if (w_ch == L'#') {
        contract();
        sym = NUL;
    }
    // 纯字母
    else if (isLetter(w_ch)) {
        while (isLetter(w_ch) || isDigit(w_ch)) {
            contract();
            getCh();
        }
        // 查表，判断是否为保留字
        int code = reserve(strToken);
        switch (code) {
        case -1:
            sym = IDENT;
            break;
        case 0:
            sym = ODD_SYM;
            break;
        case 1:
            sym = BEGIN_SYM;
            break;
        case 2:
            sym = END_SYM;
            break;
        case 3:
            sym = IF_SYM;
            break;
        case 4:
            sym = THEN_SYM;
            break;
        case 5:
            sym = WHILE_SYM;
            break;
        case 6:
            sym = DO_SYM;
            break;
        case 7:
            sym = CALL_SYM;
            break;
        case 8:
            sym = CONST_SYM;
            break;
        case 9:
            sym = VAR_SYM;
            break;
        case 10:
            sym = PROC_SYM;
            break;
        case 11:
            sym = WRITE_SYM;
            break;
        case 12:
            sym = READ_SYM;
            break;
        case 13:
            sym = PROGM_SYM;
            break;
        case 14:
            sym = ELSE_SYM;
            break;
        default:
            sym = NUL;
            break;
        }
        retract();
    }
    // 开头为数字，判断是否为数值类型
    else if (isDigit(w_ch)) {
        while (isDigit(w_ch)) {
            contract();
            getCh();
        }
        // 遇到字母
        if (isLetter(w_ch)) {
            error(ILLEGAL_WORD, (L"'" + strToken + L"'").c_str());
            // 跳过错误至下一个终止符
            while (!isTerminate(w_ch))
                getCh();
            retract();
            strToken.clear();
            sym = NUL;
        }
        // 遇到其他字符
        else {
            sym = NUMBER;
            retract();
        }
    } // 遇到:判断是否为赋值符号
    else if (w_ch == L':') {
        contract();
        getCh();
        if (w_ch == L'=') {
            contract();
            pre_word_col++;
            sym = ASSIGN;
        } else {
            error(MISSING, L"'='");
            strToken.clear();
            sym = NUL;
        }
    } else if (w_ch == L'<') {
        contract();
        getCh();
        if (w_ch == L'=') {
            contract();
            pre_word_col++;
            sym = LEQ;
        } else if (w_ch == L'>') {
            contract();
            pre_word_col++;
            sym = NEQ;
        } else {
            sym = LSS;
            retract();
        }
    } else if (w_ch == L'>') {
        contract();
        getCh();
        if (w_ch == L'=') {
            contract();
            pre_word_col++;
            sym = GEQ;
        } else {
            sym = GRT;
            retract();
        }
    } else {
        int code = isOprator(w_ch);
        if (code != -1) {
            contract();
            switch (code) {
            case 0:
                sym = PLUS;
                break;
            case 1:
                sym = MINUS;
                break;
            case 2:
                sym = MULTI;
                break;
            case 3:
                sym = DIVIS;
                break;
            case 4:
                sym = EQL;
                break;
            // 这里注释掉是因为前面判断是不是复合符号的时候已经判断过了，
            // 能进入到这段逻辑肯定不是'<'和'>'了
            // case 5:
            //     sym = LSS;
            //     break;
            // case 6:
            //     sym = GRT;
            //     break;
            case 7:
                sym = LPAREN;
                break;
            case 8:
                sym = RPAREN;
                break;
            case 9:
                sym = COMMA;
                break;
            case 10:
                sym = SEMICOLON;
                break;
            default:
                break;
            }
        } else {
            contract();
            error(ILLEGAL_WORD, (L"'" + strToken + L"'").c_str());
            sym = NUL;
        }
    }
    // wcout << L"(" << row_pos << L"," << col_pos << L")\t" << setw(15)
    //       << strToken << setw(20) << sym_map[sym] << endl;
}