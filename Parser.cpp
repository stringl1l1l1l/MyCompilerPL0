#include "Parser.h"
#include "ErrorHandler.h"
#include "Lexer.h"
#include "PCode.h"
#include "PL0.h"
#include "SymTable.h"
#include <iostream>

// 用于错误恢复的函数，若当前符号在s1中，则读取下一符号；若当前符号不在s1中，则报错，接着循环查找下一个在中s1 ∪ s2的符号
template <class... T>
void judge(unsigned long s1, unsigned long s2, unsigned int n, T... extra)
{
    if (!(sym & s1)) // 当前符号不在s1中
    {
        error(n, extra...);
        unsigned long s3 = s1 | s2; // 把s2补充进s1

        while (!(sym & s3)) // 循环找到下一个合法的符号
        {
            if (w_ch == L'\0')
                over();
            getWord(); // 继续词法分析
        }
        if (sym & s1)
            getWord();
    } else
        getWord();
}
// 开始语法分析
// const -> id:=number
void constDef()
{
    if (sym == IDENT) {
        // 将常量登入符号表，常量属于右值，不需要记录offset
        wstring name = strToken;
        SymTable::enter(name, 0, Category::CST);
        getWord();
        // const -> id:=
        if (sym & (ASSIGN | EQL)) {
            if (sym == EQL) {
                error(EXPECT_STH_FIND_ANTH, L"':='", L"'='");
            }
            getWord();
            // const -> id:=number
            if (sym == NUMBER) {
                SymTable::table[SymTable::table.size() - 1].info->setValue(strToken);
                getWord();
            } else {
                error(EXPECT_NUMEBR_AFTER_BECOMES);
            }
        }
    } else {
        // 没有找到id则跳转到第一个constDef的follow集中的符号
        judge(0, follow_constdef, ILLEGAL_DEFINE, L"constDef");
    }
}

// <condecl> -> const <constDef>{,<constDef>};
void condecl()
{
    // const
    if (sym == CONST_SYM) {
        // const <constDef>
        getWord();
        constDef();
        // const <constDef> {,<constDef>}
        while (sym & (COMMA | IDENT)) {
            judge(COMMA, IDENT, MISSING, L"','");
            if (sym == IDENT) // FIRST(<constDef>)
            {
                constDef();
            } else {
                judge(0, IDENT | SEMICOLON, EXPECT, L"constDef"); // todo expect常量定义式
            }
        }
        // const <constDef> {,<constDef>};
        if (sym == SEMICOLON) {
            getWord();
            return;
        } else {
            judge(0, follow_condecl, MISSING, L"';'"); // todo expect ;
        }
    } else {
        judge(0, follow_condecl, ILLEGAL_DEFINE, L"condecl");
    }
}

// <vardecl> -> var <id>{,<id>};
void vardecl()
{
    // var
    if (sym == VAR_SYM) {
        getWord();
        // var <id>
        if (sym == IDENT) {
            // 将标识符登入到符号表
            SymTable::enter(strToken, glo_offset, Category::VAR);
            glo_offset += VAR_WIDTH;
            getWord();
        } else {
            judge(0, COMMA, MISSING, L"identifier");
        }
        // var <id>{,<id>}
        while (sym == COMMA) {
            getWord();
            if (sym == IDENT) {
                // 将标识符登入到符号表
                SymTable::enter(strToken, glo_offset, Category::VAR);
                glo_offset += VAR_WIDTH;
                getWord();
            } else {
                error(REDUNDENT, L"','"); // todo expect 标识符
            }
        }
        // var <id>{,<id>};
        if (sym == SEMICOLON) {
            getWord();
        } else {
            judge(0, follow_vardecl, MISSING, L"';'");
        }
    } else {
        judge(0, follow_vardecl, ILLEGAL_DEFINE, L"vardecl");
    }
}

// <proc> -> procedure id ([id {,id}]);<block> {;<proc>}
void proc()
{
    if (sym == PROC_SYM) {
        getWord();
        // <proc> -> procedure id
        ProcInfo* cur_info = nullptr; // 临时变量，记录当前过程符号表项的信息
        if (sym == IDENT) {
            // 将过程名登入符号表
            SymTable::mkTable();
            int cur_proc = SymTable::enterProc(strToken);
            if (cur_proc != -1) {
                cur_info = (ProcInfo*)SymTable::table[cur_proc].info;
                // 子过程的入口地址登入符号表，待回填
                size_t entry = PCodeList::emit(jmp, 0, 0);
                SymTable::table[SymTable::table.size() - 1].info->setEntry(entry);
            }
            getWord();
        } else {
            judge(0, LPAREN, EXPECT_STH_FIND_ANTH, L"identifier", (L"'" + strToken + L"'").c_str());
        }
        // <proc> -> procedure id (
        if (sym == LPAREN) {
            // 层级增加，display表扩张
            SymTable::display.push_back(0);
            level++;
            getWord();
        } else {
            judge(0, IDENT | RPAREN, MISSING, L"'('");
        }
        // <proc> -> procedure id ([id {,id}]
        // 分析至形参列表
        if (sym == IDENT) {
            // 将过程的形参登入符号表，并与相应的过程绑定
            int form_var = SymTable::enter(strToken, glo_offset, Category::FORM);
            glo_offset += VAR_WIDTH;
            if (cur_info)
                cur_info->form_var_list.push_back(form_var);
            getWord();
            while (sym == COMMA) {
                getWord();
                if (sym == IDENT) {
                    // 将过程的形参登入符号表，并与相应的过程绑定
                    int form_var = SymTable::enter(strToken, glo_offset, Category::FORM);
                    glo_offset += VAR_WIDTH;
                    if (cur_info)
                        cur_info->form_var_list.push_back(form_var);
                    getWord();
                } else {
                    error(REDUNDENT, L"','");
                }
            }
        }
        // <proc> -> procedure id ([id {,id}])
        if (sym == RPAREN) {
            getWord();
        } else {
            judge(0, SEMICOLON, MISSING, L"')'");
        }
        // <proc> -> procedure id ([id {,id}]);
        if (sym == SEMICOLON) {
            getWord();
        } else {
            judge(0, first_block, MISSING, L"';'");
        }
        // <proc> -> procedure id ([id {,id}]);<block> {;<proc>}
        if (sym & first_block) {
            block();
            // 执行返回，并弹栈
            PCodeList::emit(opr, 0, OPR_RETURN);
            // 层级减少，display表弹出
            SymTable::display.pop_back();
            level--;
            // 当前过程结束，开始分析下面的过程
            while (sym == SEMICOLON) {
                getWord();
                // FIRST(proc)
                if (sym == PROC_SYM) {
                    proc();
                } else {
                    error(REDUNDENT, L"';'");
                }
            }
        } else {
            judge(0, follow_block, ILLEGAL_DEFINE, L"block");
        }
    } else {
        judge(0, follow_proc, ILLEGAL_DEFINE, L"procedure");
    }
}

// <exp> -> [+|-] <term>{[+|-] <term>}
void exp()
{
    // <exp> -> [+|-]
    unsigned long aop = NUL;
    if (sym == PLUS || sym == MINUS) {
        aop = sym;
        getWord();
    }
    // <exp> -> <term>{[+|-] <term>}
    if (sym & first_term) // FIRST(term) 、 FIRST(factor)
    {
        term();
        // 若有负号，栈顶取反
        if (aop == MINUS)
            PCodeList::emit(opr, 0, OPR_NEGTIVE);
        while (sym == PLUS || sym == MINUS) {
            aop = sym;
            getWord();
            // FIRST(term)
            if (sym & first_term) {
                term();
                // 减
                if (aop == MINUS)
                    PCodeList::emit(opr, 0, OPR_SUB);
                // 加
                else
                    PCodeList::emit(opr, 0, OPR_ADD);
            } else {
                error(REDUNDENT, strToken.c_str());
            }
        }
    } else {
        judge(0, follow_exp, ILLEGAL_DEFINE, L"expression");
    }
}

// <factor> -> id|number|(<exp>)
void factor()
{
    if (sym == IDENT) {
        // 查找变量符号
        int pos = SymTable::lookUpVar(strToken);
        VarInfo* cur_info = nullptr;
        if (pos == -1)
            error(UNDECLARED_IDENT, strToken.c_str());
        // 若为常量，直接获取其符号表中的右值
        // 用临时变量记录当前查到的信息
        else
            cur_info = (VarInfo*)SymTable::table[pos].info;
        if (cur_info) {
            if (cur_info->cat == Category::CST) {
                int val = cur_info->getValue();
                PCodeList::emit(lit, cur_info->level, val);
            }
            // 若为变量，取左值
            else {
                PCodeList::emit(load, cur_info->level, cur_info->offset / UNIT_SIZE + ACT_PRE_REC_SIZE + cur_info->level + 1);
            }
        }
        getWord();
    } else if (sym == NUMBER) {
        // 数值，直接入栈
        PCodeList::emit(lit, 0, w_str2int(strToken));
        getWord();
    } else if (sym == LPAREN) {
        getWord();
        exp();
        if (sym == RPAREN) {
            getWord();
        } else {
            judge(0, follow_factor, MISSING, L"')'");
        }
    } else {
        judge(0, follow_factor, ILLEGAL_DEFINE, L"factor");
    }
}

// <term> -> <factor>{[*|/] <factor>}
void term()
{
    if (sym & first_term) {
        factor();
        // factor()执行完毕后，当前栈顶即为factor的值
        while (sym == MULTI || sym == DIVIS) {
            unsigned long nop = sym;
            getWord();
            if (sym & first_term) {
                factor();
                // 乘
                if (nop == MULTI)
                    PCodeList::emit(opr, 0, OPR_MULTI);
                // 除
                else
                    PCodeList::emit(opr, 0, OPR_DIVIS);
            } else {
                error(REDUNDENT, strToken.c_str());
            }
        }
    } else {
        judge(0, follow_term, ILLEGAL_DEFINE, L"term");
    }
}

// <lexp> -> <exp> <lop> <exp> | odd <exp>
void lexp()
{
    // <lexp> -> odd <exp>
    if (sym == ODD_SYM) {
        getWord();
        if (sym & first_exp) {
            exp();
            // odd
            PCodeList::emit(opr, 0, OPR_ODD);
        } else {
            error(EXPECT, L"expression");
        }
    }
    // <lexp> -> <exp> <lop> <exp>
    else if (sym & first_exp) {
        exp();
        if (sym & first_lop) {
            unsigned int lop = sym;
            getWord();
            exp();
            switch (lop) {
                // <
            case LSS:
                PCodeList::emit(opr, 0, OPR_LSS);
                break;
                // <=
            case LEQ:
                PCodeList::emit(opr, 0, OPR_LEQ);
                break;
                // >
            case GRT:
                PCodeList::emit(opr, 0, OPR_GRT);
                break;
                // >=
            case GEQ:
                PCodeList::emit(opr, 0, OPR_GEQ);
                break;
                // <>
            case NEQ:
                PCodeList::emit(opr, 0, OPR_NEQ);
                break;
                // =
            case EQL:
                PCodeList::emit(opr, 0, OPR_EQL);
                break;
            default:
                break;
            }
        } else {
            judge(0, first_exp, MISSING, L"'<' or '<=' or '>' or '>=' or '<>' or '='");
            exp();
        }
    } else {
        judge(0, follow_lexp, ILLEGAL_DEFINE, L"lexp");
    }
}

void statement()
{
    // <statement> -> id := <exp>
    if (sym == IDENT) {
        // 查找当前变量是否在符号表中
        int pos = SymTable::lookUpVar(strToken);
        VarInfo* cur_info = nullptr;
        // 未查找到符号
        if (pos == -1)
            error(UNDECLARED_IDENT, strToken.c_str());
        else
            cur_info = (VarInfo*)SymTable::table[pos].info;
        getWord();
        if (sym == ASSIGN) {
            // 查找到右值，右值不可被赋值
            if (cur_info && cur_info->cat == Category::CST)
                error(ILLEGAL_RVALUE_ASSIGN);
            getWord();
        } // 不是赋值号：=而是等于号=
        else if (sym == EQL) {
            error(EXPECT_STH_FIND_ANTH, L"'='", L"':='");
            getWord();
        } else {
            // 跳过非法符号，直到遇到exp的follow集
            judge(0, follow_exp, MISSING, L"':='");
        }
        exp();
        if (cur_info)
            // 赋值的P代码，当前栈顶为计算出的表达式
            PCodeList::emit(store, cur_info->level, cur_info->offset / UNIT_SIZE + ACT_PRE_REC_SIZE + cur_info->level + 1);
    }
    // <statement> -> if <lexp> then <statement> [else <statement>]
    else if (sym == IF_SYM) {
        getWord();
        lexp();
        int entry_jpc = -1, entry_jmp = -1;

        // 当前栈顶为条件表达式的布尔值
        if (sym == THEN_SYM) {
            // 条件为假跳转，待回填else入口地址或if外地址
            entry_jpc = PCodeList::emit(jpc, 0, 0);
            getWord();
        } else {
            judge(0, first_stmt, MISSING, L"then");
        }
        // <statement> -> if <lexp> then <statement>
        statement();

        if (sym == ELSE_SYM) {
            // 待回填if外的入口地址
            entry_jmp = PCodeList::emit(jmp, 0, 0);
            getWord();
            // 将else入口地址回填至jpc
            PCodeList::backpatch(entry_jpc, PCodeList::code_list.size());
            statement();
            // 有else，则将if外入口地址回填至jmp
            PCodeList::backpatch(entry_jmp, PCodeList::code_list.size());
        } else
            // 没有else，则将if外入口地址回填至jpc
            PCodeList::backpatch(entry_jpc, PCodeList::code_list.size());
    }
    // <statement> -> while <lexp> do <statement>
    else if (sym == WHILE_SYM) {
        getWord();
        // FIRST(lexp)
        size_t condition = PCodeList::code_list.size();
        lexp();
        // 当前栈顶为条件表达式的布尔值
        // 条件为假跳转，待回填循环出口地址
        size_t loop = PCodeList::emit(jpc, 0, 0);
        if (sym == DO_SYM) {
            getWord();
            statement();
            // 无条件跳转至循环条件判断前
            PCodeList::emit(jmp, 0, condition);
        } else {
            judge(0, first_stmt, MISSING, L"do");
        }
        // 将下一条语句回填至jpc
        PCodeList::backpatch(loop, PCodeList::code_list.size());
    }
    // <statement> -> call id ([{<exp>{,<exp>}])
    else if (sym == CALL_SYM) {
        getWord();
        ProcInfo* cur_info = nullptr;
        // <statement> -> call id
        if (sym == IDENT) {
            // 查找过程的符号名
            int pos = SymTable::lookUpProc(strToken);
            // 未查找到过程名
            if (pos == -1)
                error(UNDECLARED_PROC, strToken.c_str());
            else
                cur_info = (ProcInfo*)SymTable::table[pos].info;
            // 若调用未定义的过程
            if (cur_info && !cur_info->isDefined)
                error(UNDEFINED_PROC, strToken.c_str());
            getWord();
        } else {
            judge(0, LPAREN, EXPECT_STH_FIND_ANTH, L"identifier", (L"'" + strToken + L"'").c_str());
        }
        // <statement> -> call id (
        if (sym == LPAREN) {
            getWord();
        } else {
            judge(0, first_exp | RPAREN, MISSING, L"'('");
        }
        // <statement> -> call id ([{<exp>
        if (sym & first_exp) {
            exp();
            // 将实参传入即将调用的子过程
            if (cur_info)
                PCodeList::emit(store, -1, ACT_PRE_REC_SIZE + cur_info->level + 1);
            size_t i = 1;
            // <statement> -> call id ([{<exp>{,<exp>}]
            while (sym == COMMA) {
                getWord();
                if (sym & first_exp) {
                    exp();
                    // 将实参传入即将调用的子过程
                    if (cur_info)
                        PCodeList::emit(store, -1, ACT_PRE_REC_SIZE + cur_info->level + 1 + i++);
                } else {
                    judge(0, first_exp, REDUNDENT, L"','");
                    exp();
                }
            }
            // 若实参列表与形参列表变量数不兼容，报错
            if (cur_info && i != cur_info->form_var_list.size()) {
                error(INCOMPATIBLE_VAR_LIST);
            }
        }
        // <statement> -> call id ([{<exp>{,<exp>}])
        if (sym == RPAREN) {
            getWord();
            // 调用子过程
            if (cur_info)
                PCodeList::emit(call, cur_info->level, cur_info->entry);
        }
    }
    // <statement> -> <body>
    else if (sym == BEGIN_SYM) {
        body();
    }
    // <statement> -> read (id{,id})
    else if (sym == READ_SYM) {
        getWord();
        if (sym == LPAREN) {
            getWord();
        } else {
            judge(0, IDENT, MISSING, L"'('");
        }
        // <statement> -> read (id
        if (sym == IDENT) {
            // 查找变量符号
            int pos = SymTable::lookUpVar(strToken);
            // 未查找到符号
            VarInfo* cur_info = nullptr;
            if (pos == -1)
                error(UNDECLARED_IDENT, strToken.c_str());
            // 用临时变量记录当前查到的信息
            else
                cur_info = (VarInfo*)SymTable::table[pos].info;
            // 右值不可被赋值
            if (cur_info) {
                if (cur_info->cat == Category::CST)
                    error(ILLEGAL_RVALUE_ASSIGN);
                // 从命令行读一个数据到栈顶
                PCodeList::emit(red, 0, 0);
                // 将栈顶值送入变量所在地址
                PCodeList::emit(store, cur_info->level, cur_info->offset / UNIT_SIZE + ACT_PRE_REC_SIZE + cur_info->level + 1);
            }
            getWord();
        } else {
            judge(0, COMMA | RPAREN, EXPECT_STH_FIND_ANTH, L"identifier", (L"'" + strToken + L"'").c_str());
        }
        // <statement> -> read (id{,
        while (sym == COMMA) {
            getWord();
            if (sym == IDENT) {
                int pos = SymTable::lookUpVar(strToken);
                // 未查找到符号
                VarInfo* cur_info = nullptr;
                if (pos == -1)
                    error(UNDECLARED_IDENT, strToken.c_str());
                // 用临时变量记录当前查到的信息
                else
                    cur_info = (VarInfo*)SymTable::table[pos].info;
                // 右值不可被赋值
                if (cur_info) {
                    if (cur_info->cat == Category::CST)
                        error(ILLEGAL_RVALUE_ASSIGN);
                    // 从命令行读一个数据到栈顶
                    PCodeList::emit(red, 0, 0);
                    // 将栈顶值送入变量所在地址
                    PCodeList::emit(store, cur_info->level, cur_info->offset / UNIT_SIZE + ACT_PRE_REC_SIZE + cur_info->level + 1);
                }
                getWord();
            } else {
                judge(0, IDENT, REDUNDENT, strToken.c_str());
            }
        }
        if (sym == RPAREN) {
            getWord();
        } else {
            judge(0, follow_stmt, MISSING, L"')'");
        }
    }
    // <statement> -> write(<exp> {,<exp>})
    else if (WRITE_SYM) {
        getWord();
        // <statement> -> write(
        if (sym == LPAREN) {
            getWord();
        } else {
            judge(0, first_exp, MISSING, L"'('");
        }
        // <statement> -> write(<exp>
        exp();
        PCodeList::emit(wrt, 0, 0);
        // <statement> -> write(<exp> {,<exp>}
        while (sym == COMMA) {
            getWord();
            if (sym == RPAREN)
                error(REDUNDENT, L"','");
            else {
                exp();
                PCodeList::emit(wrt, 0, 0);
            }
        }
        // <statement> -> write(<exp> {,<exp>})
        if (sym == RPAREN) {
            getWord();
        } else {
            judge(0, follow_stmt, MISSING, L"')'");
        }
    } else
        judge(0, follow_stmt, ILLEGAL_DEFINE, L"statement");
}

// <body> -> begin <statement> {;<statement>} end
void body()
{
    // 判断是否存在begin,是否仅缺少begin
    judge(BEGIN_SYM, first_stmt, MISSING, L"begin");
    // begin之后为新的作用域
    // level++;
    statement();
    // 这里如果end前多一个分号会多进行一次循环，并进入else分支
    while (sym & (SEMICOLON | COMMA | first_stmt)) {
        // 判断是否存在分号，是否仅缺少分号,是否错写为逗号
        if (sym == COMMA) {
            error(EXPECT_STH_FIND_ANTH, L"';'", L"','");
            getWord();
        } else
            judge(SEMICOLON, first_stmt, MISSING, L"';'");
        if (sym & first_stmt) {
            statement();
        } else {
            error(REDUNDENT, L"';'");
        }
    }
    // 判断是否缺少end
    judge(END_SYM, 0, MISSING, L"end");
    // end之后作用域结束
    // level--;
}

// <block> -> [<condecl>][<vardecl>][<proc>]<body>
void block()
{
    // <block> -> [<condecl>]
    if (sym == CONST_SYM) {
        condecl();
    }
    // <block> -> [<condecl>][<vardecl>]
    if (sym == VAR_SYM) {
        vardecl();
    }
    // 将过程所需的内存大小写入符号表
    size_t cur_proc = SymTable::sp;
    ProcInfo* cur_info = (ProcInfo*)SymTable::table[cur_proc].info;
    SymTable::addWidth(
        cur_proc,
        glo_offset);
    // <block> -> [<condecl>][<vardecl>][<proc>]]
    if (sym == PROC_SYM) {
        proc();
    }
    // 为子过程开辟活动记录空间，其中为display开辟level + 1个单元
    size_t entry = PCodeList::emit(alloc, 0, cur_info->offset / UNIT_SIZE + ACT_PRE_REC_SIZE + level + 1);
    size_t target = cur_info->entry;
    // 将过程入口地址回填至过程的跳转语句
    PCodeList::backpatch(target, entry);
    // 过程体开始，过程已定义
    if (cur_proc)
        cur_info->isDefined = true;
    // <block> -> [<condecl>][<vardecl>][<proc>]<body>
    body();
}

// <prog> -> program id; <block>
void prog()
{
    // 找到第一次出现的program
    judge(PROGM_SYM, 0, MISSING, L"program");
    // <prog> -> program id
    if (sym == IDENT) {
        // 将过程名登入符号表
        SymTable::mkTable();
        SymTable::enterProgm(strToken);
        getWord();
    }
    // 缺失 <prog> -> program ;
    else if (sym == SEMICOLON) {
        error(MISSING, L"identifier");
    }
    //  <prog> -> program {~id} ;
    else {
        error(EXPECT_STH_FIND_ANTH, L"identifier", (L"'" + strToken + L"'").c_str());
        getWord();
    }
    // <prog> -> program id;
    if (sym == SEMICOLON) {
        getWord();
    }
    // <prog> -> program id {~';'} <block>
    else {
        // 判断是否仅仅是缺失分号
        judge(SEMICOLON, first_block, MISSING, L"';'");
    }
    // 主过程的入口地址登入符号表，待回填
    size_t entry = PCodeList::emit(jmp, 0, 0);
    SymTable::table[0].info->setEntry(entry);
    //<prog> -> program id; <block>
    block();
    //<prog> -> program id; <block>#
    // 执行返回，并弹栈
    // PCodeList::emit(opr, 0, 0);
    if (sym == NUL) {
        // 程序终止
        return;
    }
}

void analyze()
{
    getWord();
    prog();
    if (w_ch != L'#') {
        error(60);
    }
    over();
}