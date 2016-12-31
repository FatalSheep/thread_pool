#ifndef THREAD_POOL
#define THREAD_POOL
    #include <thread>
    #include <functional>
    #include <mutex>
    #include <condition_variable>
    #include <stack>
    #include <queue>

    class thread_pool {
    private:
        //The pool of worker threads.
        std::stack<std::thread> pool;
        //The queue of work to be done by the workers.
        std::queue<std::function<void()>> work_queue;
        //The mutex used to lock the work if it exists.
        std::mutex work_mutex;
        //The condition for workers to check if work is available or not.
        std::condition_variable work_condition;
        //Whether or not the pool has been halted.
        bool work_halted = false;

    public:
        thread_pool(size_t thread_count) {
            threads(thread_count);
        };

        void threads(size_t thread_count) {
            /*
                Launch threads that wait for work to be queued and run work if they can lock it.
            */
            for (size_t i = 0; i < thread_count; i++) {
                auto thread_wait = [this] {
                    std::function<void()> work;
                    std::mutex _mutex;

                    //Keep the thread running indefinitely.
                    while (true) {
                        //Wait for work and make sure to run only if the thread_pool is not halted.
                        std::unique_lock<std::mutex> thread_lock(_mutex);
                        work_condition.wait(thread_lock, [this] { return (work_halted || !work_queue.empty()); });

                        //Halt this thread and exit if work was halted.
                        if (work_halted)
                            return;

                        //Lock the work and pull a job from the queue and unlock the work again.
                        std::unique_lock<std::mutex> work_lock(work_mutex);
                        work = work_queue.front();
                        work_queue.pop();
                        work_lock.unlock();

                        //Finally run the job.
                        work();
                    }
                };

                //Add the new thread to the threead_pool.
                pool.push(std::thread(thread_wait));
            }
        }

        template<class _Fx>
        void push(_Fx&& func) {
            //Create a new task function
            std::function<void()> task(std::bind(func));

            if (work_halted)
                return;

            //Lock the work queue, add work to it, then unlock it and notify one thread for work.
            work_mutex.lock();
            work_queue.push(task);
            work_mutex.unlock();
            work_condition.notify_one();
        };
        
        template<typename T, class _Fx>
        void push(T* obj, _Fx&& func) {
            //Create a new task function
            std::function<void()> task(std::bind(func, obj));

            if (work_halted)
                return;

            //Lock the work queue, add work to it, then unlock it and notify one thread for work.
            work_mutex.lock();
            work_queue.push(task);
            work_mutex.unlock();
            work_condition.notify_one();
        };

        void halt() {
            //Notify all threads to halt work.
            //Lock the thread pool and remove all threads from it and unlock it.
            std::unique_lock<std::mutex> lock(work_mutex);

            //If work has already been halted, unlock and do nothing.
            if (work_halted) {
                lock.unlock();
                return;
            }

            //Halt work and empty the work queue.
            work_halted = true;
            work_condition.notify_one();
            while(work_queue.size() > 0) work_queue.pop();

            //Empty the thread pool anbd safely free up threads with join().
            while (!pool.empty()) {
                pool.top().join();
                pool.pop();
            }

            lock.unlock();
        }

        //destroy the thread pool.
        ~thread_pool() {
            halt();
        };
    };
#endif
