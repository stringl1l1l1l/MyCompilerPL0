#include "ErrorHandler.h"
#include "Interpreter.h"
#include "PCode.h"
#include "PL0.h"
#include "Parser.h"
#include "SymTable.h"

int main()
{
    init();
    string str, filename;
    wcout << L"请输入待编译的文件名称：" << endl;
    while (cin >> str) {
        init();
        if (str != "r")
            filename = str;
        readFile2USC2("E:\\Programming\\GitHub\\MyCodesInNUAA\\complierPL0\\test\\"
            + filename + ".txt");
        if (progm_w_str.empty()) {
            wcout << L"请输入下一个待编译的文件名称, 或输入'r'重复, 或按Ctrl+C结束" << endl;
            continue;
        }
        analyze();
        // 存在错误，则跳过本循环
        if (err_cnt) {
            wcout << L"请输入下一个待编译的文件名称, 或输入'r'重复, 或按Ctrl+C结束" << endl;
            continue;
        }
        symTableTest();
        // PCodeList::printCode();
        Interpreter::run();
        wcout << L"请输入下一个待编译的文件名称, 或输入'r'重复, 或按Ctrl+C结束" << endl;
    }
}