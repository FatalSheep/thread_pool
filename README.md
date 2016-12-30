# thread_pool
A basic implementation of a thread pool.

```C++
/*
  Test function...
*/
void function() {
  cout << "Goodbye World!" << endl;
}

//Constructs a new thread_pool with the specified number of threads to to do work.
thread_pool pool(thread_count);

//Adds more threads to the thread_pool.
pool.threads(thread_count);

//Adds a task/function for the thread pool to run.
pool.push([] { cout << "Hello World!" << endl; });
pool.push(function);

//Stops the thread pool and all threads running on it.
pool.halt();
```
