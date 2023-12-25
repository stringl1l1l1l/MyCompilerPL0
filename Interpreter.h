#ifndef _INTER_H
#define _INTER_H

#include "PCode.h"
using namespace std;

#define RA 0
#define DL 1
#define GLO_DIS 2
#define DISPLAY 3
// P代码的解释器
class Interpreter {
public:
    static size_t pc; // 指令寄存器
    static size_t top; // 活动记录栈顶，并非实际开辟的空间栈顶
    static size_t sp; // 当前活动记录基地址
    static vector<int> running_stack; // 数据运行栈

    static void run();
    static void clear();

private:
    static void lit(Operation op, int L, int a);
    static void opr(Operation op, int L, int a);
    static void lod(Operation op, int L, int a);
    static void sto(Operation op, int L, int a);
    static void cal(Operation op, int L, int a);
    static void alc(Operation op, int L, int a);
    static void jmp(Operation op, int L, int a);
    static void jpc(Operation op, int L, int a);
    static void red(Operation op, int L, int a);
    static void wrt(Operation op, int L, int a);
};

#endif