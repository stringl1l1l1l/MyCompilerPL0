#include "PCode.h"
#include "PL0.h"
#include <iomanip>
#include <iostream>
using namespace std;

vector<PCode> PCodeList::code_list;

wstring op_map[P_CODE_CNT] = {
    L"LIT",
    L"OPR",
    L"LOD",
    L"STO",
    L"CAL",
    L"INT",
    L"JMP",
    L"JPC",
    L"RED",
    L"WRT"
};

int PCodeList::emit(Operation op, int L, int a)
{
    code_list.push_back(PCode(op, L, a));
    return code_list.size() - 1;
}

void PCodeList::backpatch(size_t target, size_t addr)
{
    if (addr == -1)
        return;
    else
        code_list[target].a = addr;
}

void PCodeList::printCode()
{
    for (size_t i = 0; i < code_list.size(); i++) {
        wcout << setw(4) << i << L"  " << op_map[code_list[i].op] << L", " << code_list[i].L << L", " << code_list[i].a << endl;
    }
}

void PCodeList::clear()
{
    code_list.clear();
}