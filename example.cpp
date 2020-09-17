#include <iostream>
#include "ThreadPool.h"

using namespace std;
int main(){
	using threadpool::ThreadPool;
	ThreadPool tp(4);

	function<int(char)> f=[](char a){
			std::cout<<a<<'\n';
			return 1;
	};
	char a='a';
		
	future<int> fut[4];
	for(int i=0;i<4;i++)
		fut[i]=tp.emplace_back(f,a);
	for(int i=0;i<4;i++)
		cout<<fut[i].get()<<endl;
	cout<<"get finnished\n";
	return 0;
}

