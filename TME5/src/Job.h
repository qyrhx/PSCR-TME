#pragma once

namespace pr {

class Job {
 public:
  virtual void operator()() = 0;
  virtual ~Job(){};
};

}  // namespace pr
