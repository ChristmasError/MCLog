#include "clog_class.h"

//void *f(void * args)
//{
//	for (int i = 0; i < 1000; i++)
//	{
//		CLog::Instance()->write_log(INFO, "d=%d,c=%c,s=%s,f=%f", i, 'a', "log", 1.000);
//		CLog::Instance()->write_log(WARN, "d=%d,c=%c,s=%s,f=%f", i, 'a', "log", 1.000);
//		CLog::Instance()->write_log(ERROR, "d=%d,c=%c,s=%s,f=%f", i, 'a', "log", 1.000);
//		LOG_INFO("%d", 1234567);
//		LOG_DEBUG("%d", 1234567);
//		LOG_WARN("%d", 1234567);
//		LOG_ERROR("%d", 1234567);
//	}
//}

int main()
{
	CLog::Instance()->init("mylog.log", 100, 2000000, 10);
	LOG_INFO("%d", 12345);
	printf("111111111111111\n");
	
	while (1);
	return 0;
}