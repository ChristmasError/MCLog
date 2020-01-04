#include "clog_class.h"

int main()
{
	CLog::LogInstance()->init("mylog.log");
	// 默认log路径为项目路径
	// 自定义log路径，必需路径下有 log 文件夹
	// CLog::LogInstance()->init("mylog.log","..\\log\\");
	LOG_INFO("%s\n", "test!!");

	LOG_WARN("%s\n", "test!!");

	LOG_ERROR("%s\n", "test!!");

	LOG_DEBUG("%s\n", "test!!");
	
	while (1);
	return 0;
}