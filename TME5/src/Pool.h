#pragma once

#include <vector>
#include <thread>
#include <functional>
#include "Queue.h"
#include "Job.h"

namespace pr {

using Callback = std::function<void()>;

class Pool {
  Queue<Callback> queue;
  std::vector<std::thread> threads;

 public:
  // create a pool with a queue of given size
  Pool(size_t qsize) : queue{qsize} {}

  // start the pool with nbthread workers
  void start(int nbthread) {
    for (int i = 0; i < nbthread; i++) {
      // syntaxe pour passer une methode membre au thread
      threads.emplace_back(&Pool::worker, this);
    }
  }

  // submit a job to be executed by the pool,
  // call new, and memory is freed by the worker
  void submit(Callback *f) { queue.push(f); }

  // initiate shutdown, wait for threads to finish
  void stop() {
    queue.setBlock(false);
    for (auto &t : threads)
      t.join();
    threads.clear();
  }

 private:
  // worker thread function
  void worker() {
    for (Callback *f = queue.pop(); f; f = queue.pop()) {
      auto fun = *f;
      fun();
      delete f;  // new is made when calling submit()
    }
  }
};

}  // namespace pr
