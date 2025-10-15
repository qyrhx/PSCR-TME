#include "Tasks.h"
#include "util/ImageUtils.h"
#include "util/thread_timer.h"
#include <sstream>
#include <thread>

namespace pr {

void treatImage(FileQueue &fileQueue, const std::filesystem::path &outputFolder) {
  // measure CPU time in this thread
  pr::thread_timer timer;

  while (true) {
    std::filesystem::path file = fileQueue.pop();
    if (file == pr::FILE_POISON)
      break; // poison pill
    QImage original = pr::loadImage(file);
    if (!original.isNull()) {
      QImage resized = pr::resizeImage(original);
      std::filesystem::path outputFile = outputFolder / file.filename();
      pr::saveImage(resized, outputFile);
    }
  }

  // trace
  std::stringstream ss;
  ss << "Thread " << std::this_thread::get_id() << " (treatImage): " << timer << " ms CPU"
     << std::endl;
  std::cout << ss.str();
}

void reader(FileQueue &fileQueue, ImageTaskQueue &imageQueue) {
  pr::thread_timer timer;
  while (true) {
    std::filesystem::path filepath = fileQueue.pop();
    QImage img;
    TaskData td{};
    if (filepath != pr::FILE_POISON) {
      img = pr::loadImage(filepath);
      td = TaskData{std::move(img), std::move(filepath)};
      if (!td.img.isNull()) {
        imageQueue.push(std::move(td));
      }
    } else {
      break;
    }
  }
  std::stringstream ss;
  ss << "Thread " << std::this_thread::get_id() << " (reader): " << timer << " ms CPU" << std::endl;
  std::cout << ss.str();
}

void resizer(ImageTaskQueue &imageQueue, ImageTaskQueue &resizedQueue) {
  pr::thread_timer timer;
  while (true) {
    TaskData td = imageQueue.pop();
    QImage res{};
    if (td.img.isNull()) { // = FILE_POISON
      break;
    }
    res = pr::resizeImage(td.img);
    resizedQueue.push(TaskData{std::move(res), std::move(td.filepath)});
  }
  std::stringstream ss;
  ss << "Thread " << std::this_thread::get_id() << " (resizer): " << timer << " ms CPU"
     << std::endl;
  std::cout << ss.str();
}

void saver(ImageTaskQueue &resizedQueue, const std::filesystem::path &outputFolder) {
  pr::thread_timer timer;
  while (true) {
    TaskData td = resizedQueue.pop();
    if (td.img.isNull()) { // = FILE_POISON
      break;
    }
    std::filesystem::path outputFile = outputFolder / td.filepath.filename();
    pr::saveImage(td.img, outputFile);
  }
  std::stringstream ss;
  ss << "Thread " << std::this_thread::get_id() << " (saver): " << timer << " ms CPU" << std::endl;
  std::cout << ss.str();
}

} // namespace pr
