#ifndef _QUEUEL_H
#define _QUEUEL_H
#include "Headers.hpp"
#include "Semaphore.hpp"
// Single Producer - Multiple Consumer queue
template <typename T>class PCQueue
{

public:
    PCQueue(): con_in(0),pro_in(0),pro_waiting(0){
        pthread_cond_init(&pro_c_, nullptr);
        pthread_cond_init(&con_c_, nullptr);
        pthread_mutex_init(&mutex_, nullptr);
    }
    ~PCQueue(){
        pthread_cond_destroy(&con_c_);
        pthread_cond_destroy(&pro_c_);
        pthread_mutex_destroy(&mutex_);
    }
	// Blocks while queue is empty. When queue holds items, allows for a single
	// thread to enter and remove an item from the front of the queue and return it.
	// Assumes multiple consumers.
	T pop(){
        pthread_mutex_lock(&mutex_);
        while (pro_in > 0 || pro_waiting > 0 || queue_.empty()){
            pthread_cond_wait(&con_c_,&mutex_);
        }
        con_in++;
        pthread_mutex_unlock(&mutex_);
        T retval = queue_.front();
        queue_.pop();
        pthread_mutex_lock(&mutex_);
        con_in--;
        qsize--;
        if(con_in == 0)
            pthread_cond_signal(&pro_c_);
        pthread_mutex_unlock(&mutex_);
        return retval;

    }
	// Allows for producer to enter with *minimal delay* and push items to back of the queue.
	// Hint for *minimal delay* - Allow the consumers to delay the producer as little as possible.  
	// Assumes single producer 
    void push(const T& item){
        pthread_mutex_lock(&mutex_);
        pro_waiting++;
        while(con_in + pro_in > 0 )
            pthread_cond_wait(&pro_c_,&mutex_);
        pro_waiting--;
        pro_in++;
        pthread_mutex_unlock(&mutex_);
        queue_.push(item);
        pthread_mutex_lock(&mutex_);
        pro_in--;
        if(pro_in == 0){
            pthread_cond_broadcast(&con_c_);
            pthread_cond_signal(&pro_c_);
        }
        pthread_mutex_unlock(&mutex_);
    }


private:
    std::queue<T> queue_;
    pthread_mutex_t mutex_;
    pthread_cond_t con_c_;
    pthread_cond_t pro_c_;
    int con_in,pro_in,pro_waiting;






































































































































































































































































































































    int qsize = 0;
    // Add your class memebers here
};
// Recommendation: Use the implementation of the std::queue for this exercise
#endif
