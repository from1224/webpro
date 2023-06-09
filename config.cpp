#include "config.h"

Config::Config(){
    //端口号,默认9006
    PORT = 9006;

    //日志写入方式，默认同步, 0同步，1异步
    LOGWrite = 0;

    //触发组合模式,默认listenfd LT + connfd LT 为0
    // LT+ET 1
    // ET+LT 2
    // ET+ET 3
    TRIGMode = 0;

    //listenfd触发模式，默认LT
    LISTENTrigmode = 0;

    //connfd触发模式，默认LT
    CONNTrigmode = 0;

    //优雅关闭链接，默认0不使用,1使用
    OPT_LINGER = 0;

    //数据库连接池数量,默认8
    sql_num = 2;

    //线程池内的线程数量,默认8
    thread_num = 2;

    //关闭日志,默认0打开，1关闭
    close_log = 1;

    //并发模型,默认0是Proactor，1是Reactor模型
    actor_model = 0;
}

void Config::parse_arg(int argc, char* argv[]){
    int opt;
    const char *str = "p:l:m:o:s:t:c:a:";
    while ((opt = getopt(argc, argv, str)) != -1)
    {
        switch (opt)
        {
        case 'p':
        {
            PORT = atoi(optarg);
            break;
        }
        case 'l':
        {
            LOGWrite = atoi(optarg);
            break;
        }
        case 'm':
        {
            TRIGMode = atoi(optarg);
            break;
        }
        case 'o':
        {
            OPT_LINGER = atoi(optarg);
            break;
        }
        case 's':
        {
            sql_num = atoi(optarg);
            break;
        }
        case 't':
        {
            thread_num = atoi(optarg);
            break;
        }
        case 'c':
        {
            close_log = atoi(optarg);
            break;
        }
        case 'a':
        {
            actor_model = atoi(optarg);
            break;
        }
        default:
            break;
        }
    }
}