#include "SymTable.h"
#include "ErrorHandler.h"
#include "PL0.h"
#include <iomanip>
#include <iostream>
#include <ostream>
#include <stdint.h>
#include <stdlib.h>
#include <string>
#include <vector>
size_t SymTable::sp = 0;
vector<SymTableItem> SymTable::table; // 一个程序唯一的符号表
vector<size_t> SymTable::display; // 过程的嵌套层次表

wstring cat_map[6] = {
    L"null",
    L"array",
    L"var",
    L"procedure",
    L"const",
    L"formal var"
};
// wstring 转 int
int w_str2int(wstring num_str)
{
    if (num_str.empty()) {
        wcout << L"Cannot transfer empty string!" << endl;
        return 0;
    }
    int num = 0;
    // 先遍历一遍字符串，判断合法性
    size_t size = num_str.size();
    for (wchar_t w_ch : num_str) {
        if (!(w_ch <= L'9' && w_ch >= L'0')) {
            wcout << L"Illegal string to transfer!" << endl;
            return 0;
        }
    }
    for (size_t i = 0; i < size; i++) {
        num = (num << 3) + (num << 1); // num*10
        num += (num_str[i] - L'0');
    }
    return num;
}

Information::Information()
{
    this->offset = 0;
    this->cat = Category::NIL;
    this->level = 0;
}

void Information::show()
{
    wcout << setw(10) << L"cat: " << setw(13) << cat_map[cat]
          << setw(10) << L"offset: " << setw(5) << offset
          << setw(10) << L"level: " << setw(5) << level;
}
VarInfo::VarInfo()
    : Information()
{
    this->value = 0;
    this->type = Type::INTERGER;
}

void VarInfo::setValue(wstring val_str) { this->value = w_str2int(val_str); }

int VarInfo::getValue() { return this->value; }

void VarInfo::show()
{
    wcout << setw(10) << L"cat:" << setw(15) << cat_map[cat]
          << setw(10) << L"offset:" << setw(5) << offset
          << setw(10) << L"level:" << setw(5) << level
          << setw(10) << L"value:" << setw(5) << value;
}

ProcInfo::ProcInfo()
    : Information()
{
    this->entry = 0;
    this->isDefined = false;
}

void ProcInfo::setEntry(size_t entry) { this->entry = entry; }

size_t ProcInfo::getEntry() { return this->entry; }

void ProcInfo::show()
{
    wcout << setw(10) << L"cat:" << setw(15) << cat_map[cat]
          << setw(10) << L"size:" << setw(5) << offset
          << setw(10) << L"level:" << setw(5) << level
          << setw(10) << L"entry:" << setw(5) << entry
          << setw(17) << L"form var list:";
    if (form_var_list.empty())
        wcout << setw(5) << L"null";
    for (size_t mem : form_var_list) {
        wcout << setw(5) << SymTable::table[mem].name;
    }
}

void SymTableItem::show()
{
    wcout << setw(10) << name << setw(10) << pre_item;
    info->show();
    wcout << endl;
}

void SymTable::mkTable()
{
    sp = table.size();
}

int SymTable::enter(wstring name, size_t offset, Category cat)
{
    int pos = lookUpVar(name);
    // 如果查找到重复符号，且必须在同一层级，不为形参、过程名，则说明出现变量名重定义
    if (pos != -1 && table[pos].info->level == level) {
        error(REDECLEARED_IDENT, name.c_str());
        return -1;
    }
    // 记录当前即将登入的符号表项的地址
    size_t cur_addr = table.size();
    SymTableItem item;
    // 当前符号表项的前一项是display[level]
    item.pre_item = display[level];
    item.name = name;
    // 更新display[level]为当前符号表项的地址
    display[level] = cur_addr;
    VarInfo* varInfo = new VarInfo;
    varInfo->offset = offset;
    varInfo->cat = cat;
    varInfo->level = level;
    varInfo->value = 0;
    item.info = varInfo;
    table.push_back(item);
    // wcout << setw(5) << table[cur_addr].name << setw(5) << table[cur_addr].pre_item << endl;
    // 返回当前符号表项的地址
    return cur_addr;
}

int SymTable::enterProc(wstring name)
{
    // 若查找到重复符号，且为同一层级的过程名，则出现过程重定义
    int pos = lookUpProc(name);
    if (pos != -1 && table[pos].info->level == level + 1) {
        error(REDECLEARED_PROC, name.c_str());
        return -1;
    }
    size_t cur_addr = table.size();
    SymTableItem item;
    // 当前符号表项的前一项是display[level]
    item.pre_item = display[level];
    item.name = name;
    // 更新display[level]为当前符号表项的地址
    display[level] = cur_addr;
    ProcInfo* procInfo = new ProcInfo;
    procInfo->offset = 0;
    procInfo->cat = Category::PROCE;
    procInfo->level = level + 1;
    procInfo->entry = 0;
    item.info = procInfo;
    table.push_back(item);
    // wcout << setw(5) << table[cur_addr].name << setw(5) << table[cur_addr].pre_item << endl;
    // 返回当前符号表项的地址
    return cur_addr;
}

void SymTable::enterProgm(wstring name)
{
    SymTableItem item;
    item.pre_item = 0;
    item.name = name;
    ProcInfo* procInfo = new ProcInfo;
    procInfo->offset = 0;
    procInfo->cat = Category::PROCE;
    procInfo->level = 0;
    item.info = procInfo;
    table.push_back(item);
}

int SymTable::lookUpProc(wstring name)
{
    unsigned int curAddr = 0;
    // i代表访问display的指针
    // 若查找主过程名，直接返回-1
    if (level == 0 && display[0] == 0)
        return -1;
    for (int i = level; i >= 0; i--) {
        curAddr = display[i];
        // 遍历当前display指针指向的过程下的所有过程符号，直到遇到最后一个符号(pre == 0)
        while (1) {
            if (table[curAddr].info->cat == Category::PROCE
                && table[curAddr].name == name) {
                return curAddr;
            }
            if (table[curAddr].pre_item == 0)
                break;
            curAddr = table[curAddr].pre_item;
        }
    }
    return -1;
}

int SymTable::lookUpVar(wstring name)
{
    unsigned int curAddr = 0;
    // i代表访问display的指针
    // 若查找主过程名，直接返回-1
    if (level == 0 && display[0] == 0)
        return -1;
    for (int i = level; i >= 0; i--) {
        curAddr = display[i];
        // 遍历当前display指针指向的过程下的所有变量符号，直到遇到最后一个符号(pre == 0)
        while (1) {
            if (table[curAddr].info->cat != Category::PROCE && table[curAddr].name == name) {
                return curAddr;
            }
            if (table[curAddr].pre_item == 0)
                break;
            curAddr = table[curAddr].pre_item;
        }
    }
    return -1;
}

void SymTable::addWidth(size_t addr, size_t width)
{
    table[addr].info->offset = width;
    glo_offset = 0;
}

void SymTable::clear()
{
    sp = 0;
    table.clear();
    display.clear();
    table.reserve(SYM_ITEMS_CNT);
    display.resize(1, 0);
}

void symTableTest()
{
    // wcout << L"SymTable____________________" << endl;
    // wcout << setw(10) << L"name" << setw(10) << L"next" << endl;
    // for (SymTableItem mem : SymTable::table) {
    //     wcout << setw(10) << mem.name << setw(10) << mem.next_item << endl;
    // }

    // wcout << L"display_____________________" << endl;
    // wcout << setw(10) << L"addr" << setw(10) << L"proc" << endl;
    // for (int i = 0; i < PROC_CNT; i++) {
    //     int mem = SymTable::display[i];
    //     if (mem != -1)
    //         wcout << setw(10) << mem << setw(10) << SymTable::table[mem].name << endl;
    // }
    // wcout << L"proc_size_____________________" << endl;
    // wcout << setw(10) << L"proc" << setw(10) << L"size" << endl;
    // for (int i = 0; i < SymTable::proc_addrs.size(); i++) {
    //     wcout << setw(10)
    //           << SymTable::table[SymTable::proc_addrs[i]].name
    //           << setw(10)
    //           << SymTable::table[SymTable::proc_addrs[i]].info.offset << endl;
    // }
    wcout << L"____________________________________________________SymTable_______________________________________________" << endl;
    for (SymTableItem mem : SymTable::table) {
        mem.show();
    }
    wcout << L"___________________________________________________________________________________________________________" << endl;
    // wcout << L"________________display________________" << endl;
    // wcout << setw(10) << L"addr" << setw(10) << L"proc" << endl;
    // for (int i = 0; i < SymTable::display.size(); i++) {
    //     int mem = SymTable::display[i];
    //     if (mem != -1)
    //         wcout << setw(10) << mem << setw(10) << SymTable::table[mem].name << endl;
    // }
}