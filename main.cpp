#include "config.h"

int main(int argc, char *argv[])
{
    //需要修改的数据库信息,登录名,密码,库名
    string user = "debian-sys-maint";
    string passwd = "CuS3erqAg8bIHe8w";
    string databasename = "upDB";

    //命令行解析
    Config config;
    config.parse_arg(argc, argv);

    WebServer server;   

    //初始化
    // 初始化服务器监听端口号9006，数据库用户名密码和数据库名，日志写入方式LOGWrite配置为0同步，优雅关闭OPT_LINGER设置为0不使用
    // 触发模式TRIGMode配置为0 LT+LT，数据库数量和线程池数量都为8，日志close_log为打开，并发事件模型为0 Proactor
    server.init(config.PORT, user, passwd, databasename, config.LOGWrite, 
                config.OPT_LINGER, config.TRIGMode,  config.sql_num,  config.thread_num,  // 线程池，动态扩容--》美团，，， QueryPerSecond,2000QPS:硬件和软件：线程池（同步、互斥）、互斥锁（读写锁，自旋锁，CAS（无锁编程））、定时器（升序双链表：时间轮）
                config.close_log, config.actor_model);  // Reactor和Proactor


    //日志
    // 单例模式获取日志对象，调用Log::init，init的参数为日志缓存大小和日志最大行数，以及基于锁和条件变量(push/pop)的线程安全的日志循环队列的大小（普通的数组搭配前后指针）
    // 如果设置了日志队列的大小就等于日志为异步模式，便会在Init里面创建一个线程，调用flush_log_thread
    // flush_log_thread通过单例对象调用async_write_log 死循环 的从日志循环队列中用pop获取一条日志，
    // pop其实是加锁安全且日志循环队列没有日志时阻塞在条件变量处，将线程放入条件变量的等待队列中；要是有日志，直接pop没有条件变量，直接写
    // 日志循环队列的长度是800，且自己实现的线程安全的循环队列
    // 日志的写，主线程和线程池的工作线程都可以调用LOG系列来写，单例模式可以保证8+1个线程获取同一个文件来随机IO，调用LOG系列异步时就是push，调用唤醒线程
    // 然后，若是同步：则直接在主线程中将缓存中的日志写入文件；若是异步：则将缓存string消息放入日志循环队列中，
    // 然后push操作会调用broadcast唤醒所有条件变量的等待队列中的线程（其实只有一个,刚创建就被阻塞在pop的条件变量那里）,线程写完后又会继续阻塞在pop的条件变量
    // push和pop都是由block_queue的lock和cond管理的，cond基于循环队列自身含量来决定条件变量的状态，任何对循环队列的操作都要先加锁
    // push是先Lock，再根据队列数列是否满来，肯定调用broadcast，在unlock，或者成功push，在unlock
    // pop是先lock，如果队列空，则循环中先wait再unlock，如果不为空，就直接unlock，没有wait
    server.log_write();  //单例模式创建同步/异步日志

    //数据库
    // 单例模式获取数据库链接池对象m_connPool，其调用init创建数据库连接池，传入"localhost"和上面server.init传入的数据库参数来初始化每一条数据库链接，链接到服务器本地的数据库
    // 每个数据库链接创建好后，放入list管理的双向链表中，并用链接的数量初始化sem信号量变量reserve=8
    // 然后sql_pool利用initmysql_result取一个链接更新sql_pool管理的map 用户名和密码，又释放链接，这个操作使用reserve信号量和lock完成的
    // Getconnection是先wait，再lock,unlock；  RealeaseConnection是先lock,unlock，然后post
    server.sql_pool();  //创建数据库连接池，从其中根据sem、locker取出、存入数据库连接

    //线程池
    // 这个方法其实在new了一个threadpool线程池对象
    // 线程池类里面有pthread_t*的真正的线程池数组（数目为8），还有list<http_conn *>的双向链表管理所有连接请求的请求队列,请求队列最大10000个http_conn请求，同时带了信号量和锁
    // 依次创建8个线程，每个线程创建后便回调run方法，里面 死循环 监测着http_con0n这个list请求队列，wait信号量(初始0所以直接阻塞)，获取到信号量后lock取http_conn请求，然后unlock，
    // 然后根据并发模型做不同的事，注意现在都是在工作线程中，默认Proactor：(1)Reactor模型的话，先由工作线程读取IO，读事件则LT读取，然后调用process处理HTTP请求，生成EPOLLOUT事件；写事件则工作线程writev集中写发送
    // (2)Proactor模型，取出一个数据库链接，调用process处理HTTP请求生成EPOLLOUT。因为主线程已经帮忙读取了IO，所以process分为先来process_read再来process_write \
    //    process_read根据主从状态机请求行：解析GET,POST, url, 和版本号，只支持HTTP/1.1；请求头：包含Connection,Host,Keep-alive；请求数据：POST的话带了用户名和密码
    //    process_read完成后，调用do_request来写响应。若是POST,则把用户名和密码写入到数据库中；若是GET，则将对应文件内容通过mmap映射到m_file_address中，并返回FILE_REQUEST
    //    process_read完成后，开始调用process_write来写响应，包括响应行，若请求行、请求头和请求数据有格式错误，则404；若文件资源你没权限，则403
    //    若正常响应，则200ok，把响应行和响应头写到一块连续内存，响应数据即文件写到另一块内存。然后epoll_ctl修改内核事件表,EPOLLOUT事件就绪
    //    然后调用webserver.cpp::dealwithwrite()
    // dealwithwrite调用http_conn::write函数将m_iv中存放的响应(响应行和响应头部存在m_iv[0],文件存在m_iv[1])
    // 通过writev透过sockfd发射给客户端
    server.thread_pool(); //创建线程池，每个线程创建开始便直接调用worker-->run()监视请求队列，并从中取出http_conn调用http_conn->process处理请求

    //触发模式
    // listenfd和connfd默认都是LT   // epoll EdegeT和LevelT的区别
    server.trig_mode();  //设置m_LISTENTrigmode和m_CONNTrigmode都为LT

    //监听
    // 创建服务器端socket，绑定到服务器本机的所有网卡的9006端口，创建epollfd，创建监听listenfd添加到epollfd中，只监听listenfd的可读事件，设置LT模式，非阻塞，没有设置EPOLLONESHOT
    // 主程序里面添加SIGTERM和SIGALARM信号，设置信号处理函数，信号处理函数里面使用创建的管道把信号发到0读端，由epoll_wait统一事件源监听
    server.eventListen(); //创建了一个监听fd,创建了epoll内核事件表，监听listenfd和socketpair上的可读事件

    //运行
    // 主线程是在死循环while(!stop_server){}中，其中调用epoll_wait监听所有的listenfd,connfd和读端管道，最多同时就绪10000个事件
    // 若是listenfd, 主线程调用dealclinetdata使用accept将connfd取出来，（加入定时升序（按到期时间升序）链表，有一次可读可写事件就会重新调整位置，定时器链表的每个节点包含connfd的地址，connfd和到期时间以及对应的回调函数，这个回调函数就是此定时节点到期时，从epollfd里面删除自己，关闭对应的connfd并且http_conn-1）若m_user_count>65535则打印服务器繁忙日志，不会加入epollfd，继续处理其他就绪事件，利用connfd的值初始化了http_conn *users，并加到内核事件表中，非阻塞LT+EPOLLONESHOT
    // 若是connfd的可读事件，调用dealwithread，若是Reactor，主线程直接将该socket（connfd对应http_conn* users）对应放入到list<http_conn *>请求队列中，lock,unlock,post唤醒线程，所以thread_pool中的信号量最大可以到达10000
    //                                       若是Proactor,主线程先读取IO到对应http_conn的内存中，然后再添加到请求队列中
    //                                        但是，当Reactor模式下，请求队列满了10000时，不会将该读事件加入线程池请求队列，等于将该事件先堵在门口，等待后续处理；Proactor只是会先从内核缓冲区读到用户缓冲区，但也不会将事件加入请求队列，相当于后续再一起解析
    // 若是connfd的可写事件，调用dealwithwrite，若是Reactor，主线程直接将该socket（connfd对应http_conn* users）对应放入请求队列中，lock,unlock,post唤醒线程，右工作线程写
    //                                       若是Proactor，主线程直接调用write使用writev写出去
    //                                        但是，当Reactor模式下，请求队列满了10000时，不会将该写事件加入线程池请求队列，等于将该事件先堵在门口，等待后续处理；Proactor会直接在主线程中while(1){writev}出去
    server.eventLoop();  // 可读事件是recv接收,可写事件是writev发送

    // 非常重要的，如何接收大包
    // 如果是一次包到来后，便会触发epoll内核时间表上connfd的EPOLLIN事件，如果是Reactor的话，主线程直接将connfd索引的http_conn对象放入请求队列
    // http_conn对象包含读缓冲区及对应的idx、写缓冲区、定时器以及socket，和读标志0、写标志1
    // 然后线程池就会拿到对应的http_conn，然后去read_once
    // read_once读一次（无论读不读的完），就去process_Read，及parse_line\parse header，如果read_once没读完，后面这个事件肯定会继续放到epoll里面继续处于触发状态
    // 并且返回NO_REQUEST，此时Process_read结束，并不会调用process_write生成响应，而是等待下一次读缓冲区，继续解析，直接bad_request生成错误响应和get request生成正常响应
    // 如果read_once全读完了，还是NO_REQUEST，说明一个包不够，还得等后面的包，此时其实状态和上面的缓冲区没读完的结果一致，继续等待此sockfd上到来EPOLLIN事件，然后继续读缓冲，在
    // 之前已经处理的结果之上继续处理请求

    return 0;
}





