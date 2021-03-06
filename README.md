# thread
## C++11 thread

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

# mutex<br>
## C++ thread && mutex && lock<br>

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


// 
类型 std::atomic

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

# producer && consumer<br>

## 生产者-消费者模型是经典的多线程并发协作模型
 
 * 生产者用于生产数据，生产一个就往共享数据区存一个，若共享数据区已满的话，生产者就暂停生产，等待消费者的通知后再启动
 * 消费者用于消费数据，一个一个的从共享数据区取，如果共享数据区为空的话，消费者就暂停取数据，等待生产者的通知后再启动
 * 生产者与消费者不能直接交互，它们之间所共享的数据使用队列结构来实现
 

consumer_producer.cpp<br>
```.cpp
/**
 * @theme: Producer && Consumer
 * @Author   wanghongsheng01
 * @DateTime 2021-07-12T00:29:09+0800
 */
#include<iostream>
#include<thread>
#include<mutex>
#include<queue>
#include<condition_variable>

using namespace std;


struct CacheData{
	int id;
	string data;
};

queue<CacheData> Q;
const int MAX_CACHE_LENGTH = 3;
std::mutex m;
std::condition_variable condConsumer;
std::condition_variable condProducer;
int ID = 1;


/**
 * 生产者-消费者的动作过程：
 * 
 * 先执行 ProducerTask：当 Q.size() 不大于 MAX_CACHE_LENGTH，ProducerActor 一直往队列里添加元素。
 * 直至 Q.size() > MAX_CACHE_LENGTH，condProducer.wait(lockerProducer); 
 * 
 * 紧接着执行 ConsumerTask：当 ！Q.empty() 队列不空时，一直消费队列里的元素。
 * 直到 Q.empty() 为 True，condConsumer.wait(lockerConsumer)
 */

void ProducerActor(){
	cout<<"ProducerActor:"<<endl;
	std::unique_lock<std::mutex> lockerProducer(m);
	cout<<"["<<std::this_thread::get_id()<<"] ProducerActor 获取了锁"<<endl;

	// 终止条件：Q.size() > MAX_CACHE_LENGTH
	while(Q.size() > MAX_CACHE_LENGTH){// 如果共享数据区已满的话，生产者就暂停生产，等待消费者的通知后再启动。
		cout<<"因为队列已满，生产者 sleep"<<endl;
		condProducer.wait(lockerProducer); 
		cout<<this_thread::get_id()<<" ProducerActor 重新获取了锁"<<endl;
	}
    
    cout<<"ProducerActor 开始生产 ... "<<endl;
	CacheData cache_data;
	cache_data.id = ID++;
	cache_data.data = to_string(cache_data.id);
	cout<<"  id:"<<cache_data.id<<"\n"<<"data:"<<cache_data.data<<endl;
	Q.push(cache_data);

	condConsumer.notify_one(); // 当 Q.size() 不大于 MAX_CACHE_LENGTH，ProducerActor 一直往队列里添加对象。

	cout<<"["<<this_thread::get_id()<<"] 释放了锁"<<"\n"<<endl;
}


// 消费者动作
void ConsumerActor(){
	cout<<"ConsumerActor:"<<endl;
	std::unique_lock<mutex> lockerConsumer(m);
	cout<<"["<<this_thread::get_id()<<"] ConsumerActor 获取了锁"<<endl;
	while(Q.empty()){
		cout<<"因为队列已空，消费者 sleep"<<endl;
		condConsumer.wait(lockerConsumer);
		cout<<"["<<this_thread::get_id()<<"] ConsumerActor 重新获取了锁"<<endl;
	}

	cout<<"["<<this_thread::get_id()<<"]"<<endl;
	cout<<"ConsumerActor 开始消费 ..."<<endl;
	CacheData cache_data = Q.front();
	cout<<"id:"<<cache_data.id<<"\n"<<"data:"<<cache_data.data<<endl;
	Q.pop();

	condProducer.notify_one();

	cout<<"["<<this_thread::get_id()<<"] ConsumerActor 释放了锁"<<"\n"<<endl;
}


// 生产者
void ProducerTask(){
	while(1){
		ProducerActor();
	}
}

// 消费者
void ConsumerTask(){
	while(1){
		ConsumerActor();
	}
}

// 管理线程的函数
void Dispach(int ProducerNum, int ConsumerNum){

	// 多个生产者线程
	std::vector<std::thread> thread_producer_vec;
	for(int i=0; i<ProducerNum; ++i){
		thread_producer_vec.push_back(std::thread(ProducerTask));
	}

	// 多个消费者线程
	std::vector<std::thread> thread_consumer_vec;
	for(int i=0; i<ConsumerNum; ++i){
		thread_consumer_vec.push_back(std::thread(ConsumerTask));
	}

	// 开启多个生产者线程
	for(int i=0; i<ProducerNum; ++i){
		if(thread_producer_vec[i].joinable()){
			thread_producer_vec[i].join();
		}
	}

	// 开启多个消费者线程
	for(int i=0; i<ConsumerNum; ++i){
		if(thread_consumer_vec[i].joinable()){
			thread_consumer_vec[i].join();
		}
	}

}

int main(){
	Dispach(1, 0); // 因为队列已满，生产者 sleep
	
	Dispach(0, 1); // 因为队列已空，消费者 sleep
	
	Dispach(1, 2); // 1个生产者线程，5个消费者线程，则消费者经常要等待生产者
	/*
		0x7000088e2000 ProducerActor 重新获取了锁
		ProducerActor 开始生产 ... 
		  id:13500
		data:13500
		[0x7000088e2000] 释放了锁

		ProducerActor:
		[0x7000088e2000] ProducerActor 获取了锁
		ProducerActor 开始生产 ... 
		  id:13501
		data:13501
		[0x7000088e2000] 释放了锁

		ProducerActor:
		[0x7000088e2000] ProducerActor 获取了锁
		ProducerActor 开始生产 ... 
		  id:13502
		data:13502
		[0x7000088e2000] 释放了锁

		ProducerActor:
		[0x7000088e2000] ProducerActor 获取了锁
		ProducerActor 开始生产 ... 
		  id:13503
		data:13503
		[0x7000088e2000] 释放了锁

		ProducerActor:
		[0x7000088e2000] ProducerActor 获取了锁
		因为队列已满，生产者 sleep
		[0x7000089e8000] ConsumerActor 重新获取了锁
		[0x7000089e8000]
		ConsumerActor 开始消费 ...
		id:13500
		data:13500
		[0x7000089e8000] ConsumerActor 释放了锁

		ConsumerActor:
		[0x7000089e8000] ConsumerActor 获取了锁
		[0x7000089e8000]
		ConsumerActor 开始消费 ...
		id:13501
		data:13501
		[0x7000089e8000] ConsumerActor 释放了锁

		ConsumerActor:
		[0x7000089e8000] ConsumerActor 获取了锁
		[0x7000089e8000]
		ConsumerActor 开始消费 ...
		id:13502
		data:13502
		[0x7000089e8000] ConsumerActor 释放了锁

		ConsumerActor:
		[0x7000089e8000] ConsumerActor 获取了锁
		[0x7000089e8000]
		ConsumerActor 开始消费 ...
		id:13503
		data:13503
		[0x7000089e8000] ConsumerActor 释放了锁

		ConsumerActor:
		[0x7000089e8000] ConsumerActor 获取了锁
		因为队列已空，消费者 sleep
		[0x700008965000] ConsumerActor 重新获取了锁
		因为队列已空，消费者 sleep
		0x7000088e2000 ProducerActor 重新获取了锁
		ProducerActor 开始生产 ... 
		  id:13504
		data:13504
		[0x7000088e2000] 释放了锁

	 */
	
	return 0;
}


```

# 原子操作 atomic<br>
C++11 中，不需要为原子数据类型（需要互斥地进行访问的变量）显式地声明互斥锁、调用加锁、解锁的 API，线程就能够对互斥量（原子数据）互斥地进行访问。


## 定义原子类型<br>
使用 atomic 类模版，定义需要的原子类型 `std::atomic<T> t`
如， std::atomic<float> af {1.2f};


## C++11 中的 memory_order 枚举值
	 memory_order_relaxed 不对执行顺序做任何保证

## 原子操作的行为
1. 执行读 load
	 ```.cpp
	 std::atomic<int> a;
	 int b = a.load(); // 相当于 b = a
	 ```
2. 执行写 store
	 ```.cpp
	 std::atomic<int> a;
	 a.store(1); // 相当于 a = 1
	 ```
3. 执行算术加法 fech_add(用于添加算术加法的另一个参数， 用于强制执行值的内存顺序)
	 ```.cpp
	#include <iostream>
	#include <thread>
	#include <atomic>

	std::atomic<long long> data;

	void do_work() {
		 data.fetch_add(1, std::memory_order_relaxed);
	}

	int main() {
		 std::thread th1(do_work);
		 std::thread th2(do_work);
		 std::thread th3(do_work);
		 std::thread th4(do_work);
		 std::thread th5(do_work);
		 th1.join();
		 th2.join();
		 th3.join();
		 th4.join();
		 th5.join();
		 std::cout << "Ans:" << data << '\n'; // Ans:5
	}
	 ```
	


