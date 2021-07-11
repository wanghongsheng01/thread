# thread
C++11 thread

thread.cpp
```.cpp
/*
 * [C++ thread]
 * @Author   wanghongsheng01
 * @DateTime 2021-07-07T22:02:30+0800
 */
#include<iostream>
#include<thread>
using namespace std;

void func1(){
	cout<<"do some work"<<endl;
}

void func2(int i, double d, string s){
	cout<<"i="<<i<<"; d="<<d<<"; s="<<s<<endl;
}

int main(){

    /**
     * 创建线程
     */
	std::thread t1(func1); // do some work，提供线程函数（或函数对象）
	t1.join(); // join 函数阻塞线程，直到线程函数执行结束，如果有返回值，返回值将被忽略

	std::thread t2(func2, 100, 23.00, "string"); //i=100; d=23; s=string，还可以指定函数参数
	t2.join();

	std::thread t3(func1);
	t3.detach(); //无返回值，如果不希望线程被阻塞，调用 detach 将线程和线程对象分离，detach 后和线程失联
	
	// std::bind 创建线程
	std::thread t6(std::bind(func1));
	t6.join();

	// lambda 表达式创建线程
	std::thread t7([](int i, double d){std::cout<<"i="<<i<<"; d="<<d<<endl;}, 1, 2);
	t7.join(); // i=1; d=2

	/**
	 * 移动线程，线程不复制，但可移动
	 */
	std::thread t4(func1);
	t4.join();
	std::thread t5(std::move(t4)); // 线程被移动后，线程对象 t4 不代表任何线程了
	



	return 0;
}


```
