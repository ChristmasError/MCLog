#include "clog_class.h"

int main()
{
	// 自定义log名称与log路径，必需路径下有 log 文件夹
	CREATE_LOG("mylog.log",".\\log\\");

	LOG_INFO("%s\n", "test!!");
	LOG_WARN("%s\n", "test!!");
	LOG_ERROR("%s\n", "test!!");
	LOG_DEBUG("%s\n", "test!!");
	
	while (1);
	return 0;
}