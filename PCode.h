#ifndef _P_CODE_H
#define _P_CODE_H
#include <stack>
#include <string>
#include <vector>
using namespace std;
#define P_CODE_CNT 10 // P-Code的种类数
#define UNIT_SIZE 4 // 一个内存单元的字节大小
#define ACT_PRE_REC_SIZE 3 // 活动记录的预先大小（RA、DL、全局Display）

#define OPR_RETURN 0
#define OPR_NEGTIVE 1
#define OPR_ADD 2
#define OPR_SUB 3
#define OPR_MULTI 4
#define OPR_DIVIS 5
#define OPR_ODD 6
#define OPR_EQL 7
#define OPR_NEQ 8
#define OPR_LSS 9
#define OPR_GEQ 10
#define OPR_GRT 11
#define OPR_LEQ 12
#define OPR_PRINT 13
#define OPR_PRINTLN 14

// 中间代码指令集
enum Operation {
    lit, // 取常量a放入数据栈栈顶
    opr, // 执行运算，a表示执行某种运算
    load, // 取变量（相对地址为a，层差为L）放到数据栈的栈顶
    store, // 将数据栈栈顶的内容存入变量（相对地址为a，层次差为L）
    call, // 调用过程（转子指令）（入口地址为a，层次差为L）
    alloc, // 数据栈栈顶指针增加a
    jmp, // 条件转移到地址为a的指令
    jpc, // 条件转移指令，转移到地址为a的指令
    red, // 读数据并存入变量（相对地址为a，层次差为L）
    wrt, // 将栈顶内容输出
};

class PCode {
public:
    Operation op; // 伪操作码
    int L; // 层级
    int a; // 相对地址
    PCode(Operation op, int L, int a)
    {
        this->op = op;
        this->L = L;
        this->a = a;
    };
};

class PCodeList {
public:
    static vector<PCode> code_list;

    static int emit(Operation op, int L, int a);
    static void backpatch(size_t target, size_t addr);
    static void printCode();
    static void clear();
};

#endif