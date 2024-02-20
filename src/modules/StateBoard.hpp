#pragma once

#include <condition_variable>

class StateBoard
{
    private:

        std::mutex mut;
        std::condition_variable condition;
        bool state;
    public:

        void write(bool new_state) {
            std::unique_lock<std::mutex> lk(mut);
            state = new_state;
            lk.unlock();
        }

        void read(bool& value) {
            std::unique_lock<std::mutex> lk(mut);
            value=state;
            lk.unlock();
        }
};