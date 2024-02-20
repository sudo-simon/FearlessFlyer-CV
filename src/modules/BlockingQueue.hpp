#pragma once

#include <queue>
#include <condition_variable>

template<typename T>

class BlockingQueue
{
    private:

        std::mutex mut;
        std::queue<T> private_std_queue;
        std::condition_variable condNotEmpty;
        std::condition_variable condNotFull;
        int count; // Guard with Mutex
        const int MAX{10};

    public:

        bool changed = false;

        unsigned int size(){
            return private_std_queue.size();
        }

        void put(T new_value) {
            std::unique_lock<std::mutex> lk(mut);
            //Condition takes a unique_lock and waits given the false condition
            condNotFull.wait(lk,[this]{
                if (count== MAX) {
                    return false;
                }else{
                    return true;
                }
            
            });
            changed = true;
            private_std_queue.push(new_value);
            count++;
            condNotEmpty.notify_one();
        }

        void take(T& value) {
            std::unique_lock<std::mutex> lk(mut);
            //Condition takes a unique_lock and waits given the false condition
            condNotEmpty.wait(lk,[this]{return !private_std_queue.empty();});
            value=private_std_queue.front();
            changed = false;
            private_std_queue.pop();
            count--;
            condNotFull.notify_one();
        }
};