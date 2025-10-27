#pragma once

namespace pr {

class Job {
 public:
  virtual void run() = 0;
  virtual ~Job(){};
};

}  // namespace pr
