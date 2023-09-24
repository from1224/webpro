

个人项目
------------
* 项目背景
	* 基于游双的书做的，一年前做的，还是很感慨当初从一个队列开始手写，现在要不是注释我都有点忘了内容，我希望能从此踏实走上技术的路
	* 我做完后，看到其他人有用智能指针做优化的，如果以后有机会，我接着往下写一些
	* 目前感觉项目种最大优化点可能是动态线程池，美团用java实现了并给出了文章，c++应该也行，后续有机会可以看看
	* 另外，我感觉很多人做这个项目的细节没思考，比如定时器，实测并没有那么大的改善，直接用webbench测qps的波动性会比较大，而且理论上单机资源也不会友很大的消耗
 * 后续研究
 	* 所有试图用C++制作服务器网络通信的，一般两条出路，一种，做成云盘，接入分布式文件存储+nignx负载均衡，完成文件传输；另外一种，往RPC框架拓展，实现具体性能
  	* 市面上不管是muduo库还是WebServe已经泛滥了，muduo作为网络库，本质上只是一个demo；webserve我做到后来发现这里有两个无法解决问题，一个是单点故障，一个是输出没有采用消息队列，所以非常简陋，因此后续不打算继续更新，但还是很多人选择入门，就当作一种课后作业吧。
   	* 打算等秋招结束，把Svrkit框架和DDD领域设计两个方向做的，本质上不管是TableKV、WQ还是其他中间件，技术复杂度和业务复杂度是需要全盘考虑的
 * 安装
	* Linux环境下，配置MySQL即可
   	* 如有需要，可以自行安装webbench，如果有编译需求，自行安装VScode
* 使用
	* 编译参考makefile文件
 	* 直接看每个代码里注释就行，非常详细了
* Badge

  *  [![standard-readme compliant](https://img.shields.io/badge/readme%20style-standard-brightgreen.svg?style=flat-square)](https://github.com/RichardLitt/standard-readme)
* 相关项目
	* GitHub中有大量的TinyWebServer，其中这个是写的比较好的https://github.com/qinguoyi/TinyWebServer
 	* 我在写完后对照过和他的区别，他采用了一些智能指针的方式解决内存管理问题，我则把业务做的更加细致 		
* 主要项目负责人
	* [@from1224](https://github.com/from1224) 


