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

mutex<br>
C++ thread && mutex && lock<br>
mutex.cpp<br>
```.cpp
/**
 * date: 2021-7-11
 * autor: hswang
 * ref: https://zhuanlan.zhihu.com/p/194198073
 */
#include<iostream>
#include<thread>
#include<mutex>
#include<shared_mutex>
using namespace std;

std::mutex mtx;
std::shared_mutex sm;
std::string book = "C++ Primer";

/**
 * multi-reader lock：允许多个读线程同时占用一个共享资源
 */


// mutex 调用 lock/unlock 锁定和解锁
void read1(){
	mtx.lock();
	cout<<"read1 book"<<book<<endl; // 临界区
	mtx.unlock();

}

void read2(){
	mtx.lock();
	cout<<"read2 book"<<book<<endl; // 临界区
	mtx.unlock();

}


// shared_mutex 调用 lock_shared/unlock_shared 锁定/解锁
void read3(){
	sm.lock_shared();
	cout<<"read3 book"<<book<<endl;  // 临界区
	sm.unlock_shared();

}

void read4(){
	sm.unlock_shared();
	cout<<"read4 book"<<book<<endl; // 临界区
	sm.unlock_shared();

}


/**
 * single-writer lock：shared_mutex 调用 lock/unlock 锁定和解锁
 */

void write(){
	sm.lock();
	book = "Thred"; // 临界区
	sm.unlock();

}


/**
 * lock_guard 管理 mutex:
 * 不推荐实直接去调用成员函数 lock()，因为如果忘记 unlock()，将导致锁无法释放。
 * 使用 lock_guard 或者 unique_lock 则能避免忘记解锁带来的问题。
 * std::lock_guard() 就像一个保姆，职责就是帮你管理互斥量（mutex）。
 *
 * 其原理是：声明一个局部的 std::lock_guard 对象，在其构造函数中进行加锁，在其析构函数中进行解锁。
 * 效果：创建即加锁，作用域结束自动解锁。从而使用 std::lock_guard()就可以替代lock()与unlock()。
 *
 * 临界区：
 * 在互斥量锁定到互斥量解锁之间的代码叫做临界区。需要互斥访问共享资源的那段代码称为临界区。
 * 临界区范围应该尽可能的小，即 lock 互斥量后应该尽早 unlock。通过设定作用域，
 * 使得 std::lock_guard 在合适的地方被析构，通过使用{}来调整作用域范围，
 * 可使得互斥量m在合适的地方被解锁：
 */

std::mutex m; // 实例化一个互斥量对象
void func1(int a){
	// 作用域：通过使用{}来调整作用域范围
	{
		std::lock_guard<std::mutex> guard(m); // 该句替换了 m.lock()，lock_guard 传入互斥量参数时，触发调用 lock_guard 构造函数，申请锁定 mutex 对象
		cout<<"writer to a:"<<endl;
		a += 100;
		cout<<"a:"<<a<<endl;

	} //此时不用调用 m.unlock()，guard 出了作用域被自动释放，自动调用析构函数，解锁 m
	cout<<"作用域外的内容1"<<endl;
	cout<<"作用域外的内容2"<<endl;
	cout<<"作用域外的内容3"<<endl;
} 


/**
 *                   std::unique_lock        std::lock_guard
 *                   
 * 手动 lock/unlock        支持                     不支持
 * 
 * 参数               支持 adopt_lock        支持 adopt_lock/try_to_lock/defer_lock
 *
 * std::unique_lock(mutex 对象，defer)
 * defer: unique_lock objects constructed with defer_lock do not lock the mutex object 
 *        automatically on construction, initializing them as not owning a lock.
 */

std::mutex m2;
void func2(int a){
	std::unique_lock<std::mutex> g2(m2, defer_lock); // 初始化一个没有加锁的 mutex
	g2.lock(); // 手动加锁。g2 接管了 mutex，代替了 m.lock()
	cout<<"write to a"<<endl;
	a *= 100;
	cout<<"a = "<<a<<endl;
	g2.unlock(); 
}


/**
 * try_to_lock:
 * 使用 try_to_lock 要小心，因为 try_to_lock 尝试锁失败后不会阻塞线程，而是继续往下执行程序，
 * 因此，需要使用 if-else 语句来判断是否锁成功,只有锁成功后才能去执行互斥代码段。
 * 而且需要注意的是，因为 try_to_lock 尝试锁失败后代码继续往下执行了，因此该语句不会再次去尝试锁。
 */
std::mutex m3;
void func3(int a){
	// m3.lock(); // 若没解锁 m3，try_to_lock 尝试 g3 锁失败后，不会阻塞线程，继续往下执行
	std::unique_lock<std::mutex> g3(m3, try_to_lock);
	if(g3.owns_lock()){ //  锁成功
		cout<<"write to a"<<endl;
		a *= 1000;
		cout<<"a = "<<a<<endl;

	}
	else{ // 锁失败
		cout<<"failed lock"<<endl;

	}

}


/**
 * std::unique_lock 所有权的转移：
 * 注意，这里的转移指的是 std::unique_lock 对象间的转移；
 * std::mutex 对象的所有权不需要手动转移给 std::unique_lock
 * std::unique_lock 对象实例化后会直接接管 std::mutex。
 */
std::mutex m4;
void func4(int a){
	std::unique_lock<std::mutex> g1(m4, defer_lock);
	std::unique_lock<std::mutex> g2(std::move(g1));
	g2.lock(); // 若没有 defer_lock 参数
	// 则报错：system_error: unique_lock::lock: already locked: Resource deadlock avoided
	cout<<"write to a"<<endl;
	a *= 900;
	cout<<"a = "<<a<<endl;
	g2.unlock();
}


/**
 * 异步线程：
 * std::async (异步任务，参数) 是一个函数模版，返回一个 std::future<T> 类模版对象
 * std::future 对象起到了占位的作用。刚实例化的 future 是没有储存值的，
 * 但在调用 std::future 对象的 get() 成员函数时，主线程会被阻塞直到异步线程执行结束，
 * 并把返回结果传递给 std::future，即通过 FutureObject.get() 获取函数返回值。
 */
#include<future>
#include<unistd.h>
double async_func(double a, double b){
	cout<<"async_func thread id = "<<std::this_thread::get_id()<<endl;
	double c = a + b;
	sleep(30); // 假设 async_func 任务耗时 3 秒
	return c;
}


/**
 * 异步多线程
 * std::shared_future
 * std::future 与 std::shard_future 的用途都是为了占位，但是两者有些许差别。
 * std::future的get() 成员函数是转移数据所有权;
 * std::shared_future 的 get() 成员函数是复制数据。 
 * 因此： future 对象的 get() 只能调用一次；
 * 无法实现多个线程等待同一个异步线程，一旦其中一个线程获取了异步线程的返回值，其他线程就无法再次获取。 
 * std::shared_future 对象的 get() 可以调用多次；
 * 可以实现多个线程等待同一个异步线程，每个线程都可以获取异步线程的返回值。
 */
// 同 double async_func(double a, double b){...}


// 原子类型 std::atomic

// 注意： g++ mutex.cpp -std=c++17 -Wall -o mutex
int main(){
	cout<<"main thread id = "<<std::this_thread::get_id()<<endl; // main thread id = 0x10e687e00
	// multi-reader lock
	read1(); // read1 bookC++ Primer
	read2(); // read2 bookC++ Primer
	read3(); // read3 bookC++ Primer
	read4(); // read4 bookC++ Primer


	// single-writer lock
	write();
	cout<<"执行 write() 后："<<endl;
	read4(); 
	func1(99); // a:199

	int a = 30;


	// lock_guard 管理 mutex
	std::thread t1(func1, a);
	t1.join(); // a:130


        // defer_lock
	std::thread t2(func2, a);
	t2.join(); // a = 3000
    

    	// try_to_lock
	std::thread t3(func3, a);
	t3.join(); // a = 30000
	

	// std::unique_lock 所有权的转移
	std::thread t4(func4, a);
	t4.join(); // a = 27000
	


	// 异步线程之 std::future
	double x = 30.9;
	double y = 40.9;
	std::future<double> fu = std::async(async_func, x, y); // 刚实例化的 future 是没有储存值的
	cout<<"阻塞主线程，进行异步任务，直至异步线程 return"<<fu.get()<<endl; // 调用 std::future 对象的 get() 成员函数时，获取函数返回值
	// 阻塞主线程，进行异步任务，直至异步线程 return 71.8
	

	// 异步线程之 std::shared_future
	double p = 29.0;
	double q = 39.0;
	double x1 = 39.0;
	double y1 = 49.0;
	std::shared_future<double> sfu1, sfu2;
	sfu1 = std::async(async_func, p, q);
	sfu2 = std::async(async_func, x1, y1);
	cout<<"阻塞主线程，进行异步任务，直至异步线程 return "<<"\n"<<sfu1.get()<<endl; //  return 68
	cout<<"阻塞主线程，进行异步任务，直至异步线程 return "<<"\n"<<sfu2.get()<<endl; // return 88
	// 阻塞主线程，进行异步任务，直至异步线程 return 
        // async_func thread id = 0x70000ee95000
	// async_func thread id = 0x70000ee12000
	// 68
	// 阻塞主线程，进行异步任务，直至异步线程 return 
	// 88


	// 原子类型 std::atomic<>
	#include<condition_variable> // 包含了 std::condition_variable 类
	/**
	 * condition_variable:
	 * 如何使用 ？std::condition_variable 类 搭配 std::mutex类 来使用，
	 * std::condition_variable 对象的作用不是用来管理互斥量的，它的作用是用来同步线程，它的用法相当于编程中常见的 flag 标志。
	 * std::condition_variable，A、B 两个人约定 notify_one 为行动号角，A 就等着（调用 wait(), 阻塞，
	 * 只要 B 一调用 notify_one，A 就开始行动（不再阻塞）。
	 *
	 * wait(locker) : unlock 状态 -> 被 notify -> lock 状态
	 * wait 函数需要传入一个 std::mutex（一般会传入 std::unique_lock 对象）。
	 * wait函数会自动调用 locker.unlock() 释放锁（因为需要释放锁，所以要传入 mutex）并阻塞当前线程，
	 * 本线程释放锁使得其他的线程得以继续竞争锁。一旦当前线程获得 notify
	 * (通常是另外某个线程调用 notify_* 唤醒了当前线程)，wait() 函数此时再自动调用 locker.lock()上锁。
	 *
	 * std::condition_variable 对象.notify_one(): 随机唤醒一个等待的线程
	 * std::condition_variable 对象.notify_all(): 唤醒所有等待线程
	 * 
	 */

	return 0;

}

```

