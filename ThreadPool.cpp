#include <future>
#include <cerrno>
#include <exception>
#include <iostream>
#include <system_error>
#include <type_traits>
#include "ThreadPool.h"
namespace threadpool {
ThreadPool::ThreadPool(int workercnt_):
	workercnt(workercnt_),mvec(workercnt_),cv_jobaddvec(workercnt_),
	list(workercnt_,std::list<std::function<void()>>())
{
	for(int i = 0; i < workercnt; i++) {
		worker.emplace_back([&, i]()noexcept {
			do {
				try {
					if(list[i].empty()){
						std::unique_lock<std::mutex> l(mvec[i]);
						cv_jobaddvec[i].wait(l);
					}
					if(taskcnt.load()!=-1){
						list[i].front()();
						list[i].pop_front();
						taskcnt--;
					}else {
						workercnt--;
						break; //exit thread
					}
				} catch(const std::system_error &e) {
					if(e.code() == std::error_code(EINTR, std::system_category()))
						break; //break for block io task
					else throw std::current_exception();
				} catch(...) {
					throw std::current_exception();
				}
			} while(true);
		});
	}
	setcleaner();

}

void ThreadPool::setcleaner(bool soft) {
	if(soft) {
		cleaner = [](std::thread & t) {
			t.join();
		};
	} else {
		cleaner = [](std::thread & t) {
			t.~thread();
		};
	}
}
ThreadPool::~ThreadPool() {
	taskcnt.store(-1);
	int maxwokercnt=workercnt.load();
	while(workercnt.load()){
		for(int i=0;i<maxwokercnt;i++)
			cv_jobaddvec[i].notify_one();
	}
	for(auto &it : worker) {
		cleaner(it);
	}
}
}
