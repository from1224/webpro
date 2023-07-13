

个人项目
------------
* 项目背景
	* 基于游双的书做的，一年前做的，还是很感慨当初从一个队列开始手写，现在要不是注释我都有点忘了内容，我希望能从此踏实走上技术的路
	* 我做完后，看到其他人有用智能指针做优化的，如果以后有机会，我接着往下写一些
	* 目前感觉项目种最大优化点可能是动态线程池，美团用java实现了并给出了文章，c++应该也行，后续有机会可以看看
	* 另外，我感觉很多人做这个项目的细节没思考，比如定时器，实测并没有那么大的改善，直接用webbench测qps的波动性会比较大，而且理论上单机资源也不会友很大的消耗
 * 安装
        * Linux环境下，配置MySQL即可
   	* 如有需要，可以自行安装webbench，如果有编译需求，自行安装VScode
* 使用
	* 编译参考makefile文件
 	* 直接看每个代码里注释就行，非常详细了
* Badge
···
  [![standard-readme compliant](https://img.shields.io/badge/readme%20style-standard-brightgreen.svg?style=flat-square)](https://github.com/RichardLitt/standard-readme)
* 相关项目
	* GitHub中有大量的TinyWebServer，其中这个是写的比较好的https://github.com/qinguoyi/TinyWebServer
 	* 我在写完后对照过和他的区别，他采用了一些智能指针的方式解决内存管理问题，我则把业务做的更加细致 		
* 主要项目负责人
	* @from1224 


