#ifndef _SYMBOL_TABLE_H
#define _SYMBOL_TABLE_H

#include <iomanip>
#include <string>
#include <sys/stat.h>
#include <vector>
using namespace std;

enum Type { INTERGER };
enum Category {
    NIL, // 空
    ARR, // 数组
    VAR, // 变量
    PROCE, // 过程
    CST, // 常量
    FORM, // 形参
};
// 父类信息类型
class Information {
public:
    enum Category cat; // 种属
    size_t offset;
    size_t level;
    Information();
    virtual void setValue(wstring val_str) { }
    virtual int getValue() { return 0; }
    virtual void setEntry(size_t entry) { }
    virtual size_t getEntry() { return -1; }
    virtual void show();
};

// 变量信息，继承信息类型
class VarInfo : public Information {
public:
    enum Type type; // 类型
    int value; // is

    VarInfo();
    void setValue(wstring val_str) override;
    int getValue() override;
    void show() override;
};

// 过程信息，继承信息类型
class ProcInfo : public Information {
public:
    bool isDefined; // 过程是否定义的标识
    size_t entry; // 过程的中间代码入口地址
    vector<size_t> form_var_list; // 过程的形参入口地址列表

    ProcInfo();
    void show() override;
    void setEntry(size_t entry) override;
    size_t getEntry() override;
};

// 符号表项
class SymTableItem {
public:
    size_t pre_item;
    Information* info;
    wstring name; // 符号名
    void show();
};

// 符号表
class SymTable {
public:
    static size_t sp; // 指向当前子过程符号表的首地址
    static vector<SymTableItem> table; // 一个程序唯一的符号表
    static vector<size_t> display; // 过程的嵌套层次表

public:
    // 创建子符号表
    static void mkTable();
    // 将变量名登入符号表
    static int enter(wstring name, size_t offset, Category cat);
    static void addWidth(size_t addr, size_t width);
    // 将过程名登入符号表
    static int enterProc(wstring name);
    static void enterProgm(wstring name);
    // 查找符号在符号表中位置
    static int lookUpVar(wstring name);
    static int lookUpProc(wstring name);
    // 清空符号表
    static void clear();
};

void symTableTest();
int w_str2int(wstring);
#endif