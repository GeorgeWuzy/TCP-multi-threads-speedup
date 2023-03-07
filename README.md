# TCP-multi-threads-speedup
本项目利用多线程加速手段以及TCP通讯技术实现两台计算机协作执行，挖掘两个计算机的潜在算力。
This project utilizes multi-threaded acceleration and TCP communication technology to collaborate the execution of two computers and exploit the potential computing power of both computers.
1. spring为单机非加速及单机加速程序，直接运行即可。
2. Group_work_server为双机执行发送数据方（报告中B机），需将Srv.cpp中282行与283行分别设置你主机的端口号和IPv4地址。
3. Group_work_client为双机执行接收数据方（报告中A机），需将Clt.cpp中287行与288行分别设置你主机（发射机）的端口号和IPv4地址，即与2中相同地址与端口号。
4. 双机执行需要同时运行2和3（建议先运行Srv.cpp）
