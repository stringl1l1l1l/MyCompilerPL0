#include "Interpreter.h"
#include "PL0.h"

size_t Interpreter::pc = 0;
size_t Interpreter::top = 0;
size_t Interpreter::sp = 0;
vector<int> Interpreter::running_stack;
// 取常量a放入数据栈栈顶
void Interpreter::lit(Operation op, int L, int a)
{
    // 内存栈顶等于数据栈顶，需要额外开辟空间
    if (top == running_stack.size())
        running_stack.push_back(a);
    // 内存栈顶大于数据栈顶，直接赋值
    else
        running_stack[top] = a;
    top++;
    pc++;
}

// 执行运算，a表示执行某种运算
void Interpreter::opr(Operation op, int L, int a)
{
    // top-1为数据栈顶，top-2为数据次栈顶
    // opr 0 0 执行断点返回并弹栈
    if (a == OPR_RETURN) {
        // 恢复断点，此处-1是因为这个函数末尾有个+1，debug了半天才发现
        pc = running_stack[sp + RA] - 1;
        int old_sp = running_stack[sp + DL];
        // top指针还原到上一个活动记录位置
        top -= top - sp;
        // 恢复老sp
        sp = old_sp;
    } else if (a == OPR_NEGTIVE) {
        // 栈顶取反(反码 + 1)
        running_stack[top - 1] = ~running_stack[top - 1] + 1;
    } else if (a == OPR_ADD) {
        // 次栈顶加栈顶
        int res = running_stack[top - 2] + running_stack[top - 1];
        // 直接在次栈顶赋值
        running_stack[top - 2] = res;
        top--;
    } else if (a == OPR_SUB) {
        // 次栈顶减栈顶
        int res = running_stack[top - 2] - running_stack[top - 1];
        // 直接在次栈顶赋值
        running_stack[top - 2] = res;
        top--;
    } else if (a == OPR_MULTI) {
        // 次栈顶乘栈顶
        int res = running_stack[top - 2] * running_stack[top - 1];
        // 直接在次栈顶赋值
        running_stack[top - 2] = res;
        top--;
    } else if (a == OPR_DIVIS) {
        // 次栈顶除栈顶
        int res = running_stack[top - 2] / running_stack[top - 1];
        // 直接在次栈顶赋值
        running_stack[top - 2] = res;
        top--;
    } else if (a == OPR_ODD) {
        // 栈顶元素为奇数结果为真
        running_stack[top - 1] = (running_stack[top - 1] & 0b1) == 1;
    } else if (a == OPR_EQL) {
        // 栈顶与次栈顶相等时结果为真
        bool res = running_stack[top - 2] == running_stack[top - 1];
        // 直接在次栈顶赋值
        running_stack[top - 2] = res;
        top--;
    } else if (a == OPR_NEQ) {
        // 栈顶与次栈顶不相等时结果为真
        bool res = running_stack[top - 2] != running_stack[top - 1];
        // 直接在次栈顶赋值
        running_stack[top - 2] = res;
        top--;
    } else if (a == OPR_LSS) {
        // 次栈顶 < 栈顶时结果为真
        bool res = running_stack[top - 2] < running_stack[top - 1];
        // 直接在次栈顶赋值
        running_stack[top - 2] = res;
        top--;
    } else if (a == OPR_LEQ) {
        // 次栈顶 <= 栈顶时结果为真
        bool res = running_stack[top - 2] <= running_stack[top - 1];
        // 直接在次栈顶赋值
        running_stack[top - 2] = res;
        top--;
    } else if (a == OPR_GRT) {
        // 次栈顶 > 栈顶时结果为真
        bool res = running_stack[top - 2] > running_stack[top - 1];
        // 直接在次栈顶赋值
        running_stack[top - 2] = res;
        top--;
    } else if (a == OPR_GEQ) {
        // 次栈顶 >= 栈顶时结果为真
        bool res = running_stack[top - 2] >= running_stack[top - 1];
        // 直接在次栈顶赋值
        running_stack[top - 2] = res;
        top--;
    }
    pc++;
}

// 根据层级和偏移量，取指定单元的数据放入栈顶
void Interpreter::lod(Operation op, int L, int a)
{
    // 根据层级和偏移量，查找display表
    // running_stack[sp + DISPLAY + L]即指定层级L的活动记录基地址
    // 内存栈顶等于数据栈顶，需要额外开辟空间
    if (top == running_stack.size())
        running_stack.push_back(running_stack[running_stack[sp + DISPLAY + L] + a]);
    // 内存栈顶大于数据栈顶，直接赋值
    else
        running_stack[top] = running_stack[running_stack[sp + DISPLAY + L] + a];
    top++;
    pc++;
}

// 根据层级和偏移量，将栈顶数据存入指定单元，并弹栈
void Interpreter::sto(Operation op, int L, int a)
{
    if (L >= 0) {
        // 根据层级和偏移量，查找display表
        // running_stack[sp + DISPLAY + L]即指定层级L的活动记录基地址
        running_stack[running_stack[sp + DISPLAY + L] + a] = running_stack[top - 1];
        top--;
    }
    // L为-1，说明这是形参传递的代码，需要预先开辟空间
    else {
        size_t cur_size = running_stack.size();
        // 取出栈顶值(top-1处)
        int val = running_stack[top - 1];
        top--;
        // 预先开辟空间，个数为a+1-（cur_size-top）（当前已额外开辟的空间）
        for (int i = cur_size - top; i <= a; i++)
            running_stack.push_back(0);
        // 将形参传递至指定位置
        running_stack[top + a] = val;
    }
    pc++;
}

// 调用过程，先保存断点，然后调整sp
void Interpreter::cal(Operation op, int L, int a)
{

    // 保存断点
    running_stack[top + RA] = pc + 1;
    // 复制全局display的L+1个单元到即将开辟的活动记录
    // running_stack[sp + GLO_DIS]表示当前全局display表的基地址
    // top + DISPLAY表示即将开辟的活动记录的display表基地址
    for (int i = 0; i <= L; i++)
        running_stack[top + DISPLAY + i] = running_stack[running_stack[sp + GLO_DIS] + i];
    // 第L+1个单元是即将开辟的活动记录的基地址
    running_stack[top + DISPLAY + L] = top;
    // 记录老sp，并调整sp到即将开辟的活动记录
    running_stack[top + DL] = sp;
    sp = top;
    // 跳转
    pc = a;
}

// 在当前栈top处开辟a个内存单元，
void Interpreter::alc(Operation op, int L, int a)
{
    size_t cur_size = running_stack.size();
    // 若当前额外空间满足要求，直接移动数据栈顶指针
    if (a <= cur_size - top)
        top += a;
    else {
        // 开辟空间时减去已经额外开辟的空间
        for (int i = 0; i < a - (cur_size - top); i++)
            running_stack.push_back(0);
        // 内存栈顶与数据栈顶对齐
        top = running_stack.size();
    }
    // 将新的display地址送到新的活动记录中的全局display处
    running_stack[sp + GLO_DIS] = sp + DISPLAY;
    pc++;
}

// 无条件跳转
void Interpreter::jmp(Operation op, int L, int a)
{
    pc = a;
}

// 当前栈顶条件为假时跳转
void Interpreter::jpc(Operation op, int L, int a)
{
    // 栈顶条件为假
    if (!running_stack[top - 1])
        pc = a;
    // 栈顶条件为真
    else
        pc++;
    top--;
}

// 从命令行读取一个数据到栈顶
void Interpreter::red(Operation op, int L, int a)
{
    int data;
    wcout << "read: ";
    wcin >> data;
    // 数据入栈
    if (top == running_stack.size())
        running_stack.push_back(data);
    else
        running_stack[top] = data;
    top++;
    pc++;
}

// 将当前栈顶输出
void Interpreter::wrt(Operation op, int L, int a)
{
    wcout << "write: " << running_stack[top - 1] << endl;
    top--;
    pc++;
}

void Interpreter::run()
{
    // 按照pc的指示运行程序
    for (int i = 0; i < PCodeList::code_list.size(); i = pc) {
        // wcout << pc << endl;
        PCode code = PCodeList::code_list[i];
        switch (code.op) {
        case Operation::lit:
            lit(code.op, code.L, code.a);
            break;
        case Operation::opr:
            opr(code.op, code.L, code.a);
            break;
        case Operation::load:
            lod(code.op, code.L, code.a);
            break;
        case Operation::store:
            sto(code.op, code.L, code.a);
            break;
        case Operation::call:
            cal(code.op, code.L, code.a);
            break;
        case Operation::alloc:
            alc(code.op, code.L, code.a);
            break;
        case Operation::jmp:
            jmp(code.op, code.L, code.a);
            break;
        case Operation::jpc:
            jpc(code.op, code.L, code.a);
            break;
        case Operation::red:
            red(code.op, code.L, code.a);
            break;
        case Operation::wrt:
            wrt(code.op, code.L, code.a);
            break;
        default:
            break;
        }
    }
}

void Interpreter::clear()
{
    running_stack.clear();
    sp = 0;
    top = 0;
    pc = 0;
}