#include <list>
#include <thread>
#include <memory>
#include <vector>
#include <future>
#include <functional>
#include <condition_variable>

namespace threadpool {
template< class T >
using result_of_t = typename std::result_of<T>::type;
class ThreadPool {
public:
	ThreadPool(int workercnt_ = 5);
	template<class F, class ...Args>
	std::future<result_of_t<F(Args...)>> emplace_back(F&& f, Args&&... args) ;
	void setcleaner(bool soft = true);
	~ThreadPool();
private:
	std::atomic<int> workercnt;
	std::vector<std::mutex> mvec;
	std::list<std::thread> worker;
	std::vector<std::condition_variable> cv_jobaddvec;
	std::atomic<int> taskcnt;
	std::vector<std::list<std::function<void()>>> list;
	std::function<void(std::thread &)> cleaner;
};
template<class F, class ...Args>
std::future<result_of_t<F(Args...)>> ThreadPool::emplace_back(F&& f, Args&&... args) 
{
    using RetType=typename std::result_of<F(Args...)>::type;
	std::shared_ptr<std::packaged_task<RetType()>> tp=std::make_shared<std::packaged_task<RetType()>>(std::bind(f,args...));
	list[taskcnt.load()%workercnt].push_back([tp]{ (*tp)(); });
	cv_jobaddvec[taskcnt.load()%workercnt].notify_one();
	taskcnt++;
	return tp->get_future();
}
}
