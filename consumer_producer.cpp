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
 * 生产者-消费者模型是经典的多线程并发协作模型。
 * 
 * 生产者用于生产数据，生产一个就往共享数据区存一个，如果共享数据区已满的话，生产者就暂停生产，等待消费者的通知后再启动。
 * 消费者用于消费数据，一个一个的从共享数据区取，如果共享数据区为空的话，消费者就暂停取数据，等待生产者的通知后再启动。
 * 生产者与消费者不能直接交互，它们之间所共享的数据使用队列结构来实现。
 * 
 */
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

