#ifndef MCLOG_H
#define MCLOG_H

// 下列 ifdef 块是创建使从 DLL 导出更简单的
// 宏的标准方法。此 DLL 中的所有文件都是用命令行上定义的 MCLOG_EXPORTS
// 符号编译的。在使用此 DLL 的
// 任何项目上不应定义此符号。这样，源文件中包含此文件的任何其他项目都会将
// MCLOG_API 函数视为是从 DLL 导入的，而此 DLL 则将用此宏定义的
// 符号视为是被导出的。
#ifdef MCLOG_EXPORTS
#define MCLOG_API __declspec(dllexport)
#else
#define MCLOG_API __declspec(dllimport)
#endif

#include <io.h>     //access函数的头文件
#include <stdarg.h>
#include <direct.h>
#include <assert.h>
#include <windows.h>
#include <thread>
#include <tchar.h>
#include <map>
#include <vector>
#ifdef  _DEBUG
	#include <iostream>
#endif //  DEBUG

#include "LogBuffer.h"

#endif // MCLOG_H