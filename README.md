# MCLog
MCLog封装为dll，LogTest项目为MCLog的使用示例。
MCLog适用于Windows下的C++开发，适用于多线程日志生产的线程安全的单例日志系统。
## 使用方法
VS项目属性中分别添加"MCLog.h","MCLOG.dll","MCLOG.lib"。
###在代码中
LOG_INIT()：该宏可以提前为日志单例进行初始化(但即使不初始化，其他操作也会为未初始化单例的情况下执行初始化，不过建议在使用日志前先执行一次LOG_INIT());
SET_LOGPATH(log_path)：使用该宏设置日志路径，如果不设置路径，Log文件夹将设置在根目录，即："./Log"。*目前不支持在日志系统运行时改变日志路径*。
                       示例：<h1>SET_LOGPATH("../../Path/LogDoc")</h1> 或 <h1>SET_LOGPATH("../../Path/LogDoc/")</h1>,其中'/'都可使用'\\'替代，末尾的'/''\\'可缺省;
WRITE_LOG(log_name,log_str)：使用该宏写日志，可以在多线程环境下使用，MCLog保证线程安全。
                             示例：<h1>WRITE_LOG("LogFile.txt","this is log test\n")</h1> 或<h1> WRITE_LOG("LogFile.txt","this is log test")</h1>，换行符可缺省，*但日志名目前不可缺省*。
