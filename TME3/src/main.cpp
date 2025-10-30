#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>
#include <string>
#include <mutex>
#include <atomic>
#include <vector>
#include <unordered_map>
#include <ios>
#include <cstdlib>
#include "HashMap.h"
#include "HashMapMT.h"
#include "HashMapFine.h"
#include "FileUtils.h"

using namespace std;

int main(int argc, char **argv) {
  using namespace std::chrono;

  // Allow filename as optional first argument, default to project-root/WarAndPeace.txt
  // Optional second argument is mode (e.g. "freqstd" or "freq").
  // Mode "partition"
  // Optional third argument is num_threads (default 4).
  string filename = "../WarAndPeace.txt";
  string mode     = "freqstd";
  int num_threads = 4;
  if (argc > 1)
    filename = argv[1];
  if (argc > 2)
    mode = argv[2];
  if (argc > 3)
    num_threads = atoi(argv[3]);

  // Check if file is readable
  ifstream check(filename, std::ios::binary);
  if (! check.is_open()) {
    cerr << "Could not open '" << filename
         << "'. Please provide a readable text file as the first argument." << endl;
    cerr << "Usage: " << (argc > 0 ? argv[0] : "TME3") << " [path/to/textfile] [mode] [num threads]"
         << endl;
    return 2;
  }
  check.seekg(0, std::ios::end);
  std::streamoff file_size = check.tellg();
  check.close();

  cout << "Preparing to parse " << filename << " (mode=" << mode << " N=" << num_threads
       << "), containing " << file_size << " bytes" << endl;

  auto start = steady_clock::now();

  std::vector<std::pair<std::string, int>> pairs;

  if (mode == "freqstd") {
    ifstream input(filename, std::ios::binary);
    size_t total_words  = 0;
    size_t unique_words = 0;
    std::unordered_map<std::string, int> um;
    std::string word;
    while (input >> word) {
      word = pr::cleanWord(word);
      if (! word.empty()) {
        total_words++;
        ++um[word];
      }
    }
    unique_words = um.size();
    pairs.reserve(unique_words);
    for (const auto &p : um)
      pairs.emplace_back(p);
    pr::printResults(total_words, unique_words, pairs, mode + ".freq");
  } else if (mode == "freqstdf") {
    size_t total_words  = 0;
    size_t unique_words = 0;
    std::unordered_map<std::string, int> um;
    pr::processRange(filename, 0, file_size, [&](const std::string &word) {
      total_words++;
      um[word]++;
    });
    unique_words = um.size();
    pairs.reserve(unique_words);
    for (const auto &p : um)
      pairs.emplace_back(p);
    pr::printResults(total_words, unique_words, pairs, mode + ".freq");
  } else if (mode == "freq") {
    size_t total_words  = 0;
    size_t unique_words = 0;
    HashMap<std::string, int> hm;
    pr::processRange(filename, 0, file_size, [&](const std::string &word) {
      total_words++;
      hm.incrementFrequency(word);
    });
    pairs        = hm.toKeyValuePairs();
    unique_words = pairs.size();
    pr::printResults(total_words, unique_words, pairs, mode + ".freq");
  } else if (mode == "partition") {
    size_t total_words  = 0;
    size_t unique_words = 0;
    std::unordered_map<std::string, int> um;

    auto partitions = pr::partition(filename, file_size, num_threads);
    for (size_t i = 0; i < partitions.size() - 1; ++i) {
      pr::processRange(filename, partitions[i], partitions[i + 1], [&](const std::string &word) {
        total_words++;
        um[word]++;
      });
    }
    unique_words = um.size();
    pairs.reserve(unique_words);
    for (const auto &p : um)
      pairs.emplace_back(p);
    pr::printResults(total_words, unique_words, pairs, mode + ".freq");
  } else if (mode == "mt_naive") {
    size_t total_words  = 0;
    size_t unique_words = 0;
    std::unordered_map<std::string, int> um;

    auto partitions = pr::partition(filename, file_size, num_threads);
    std::vector<std::thread> threads;
    threads.reserve(num_threads);

    for (size_t i = 0; i < partitions.size() - 1; ++i) {
      threads.emplace_back(
          pr::processRange, filename, partitions[i], partitions[i + 1],
          [&](const std::string &word) {
            total_words++;
            um[word]++;
          }
      );
    }
    for (auto &t : threads)
      t.join();

    unique_words = um.size();
    pairs.reserve(unique_words);
    for (const auto &p : um)
      pairs.emplace_back(p);
    pr::printResults(total_words, unique_words, pairs, mode + ".freq");
  } else if (mode == "mt_atomic") {
    atomic_size_t total_words = 0;
    size_t unique_words       = 0;
    std::unordered_map<std::string, int> um;

    auto partitions = pr::partition(filename, file_size, num_threads);
    std::vector<std::thread> threads;
    threads.reserve(num_threads);

    for (size_t i = 0; i < partitions.size() - 1; ++i) {
      threads.emplace_back(
          pr::processRange, filename, partitions[i], partitions[i + 1],
          [&](const std::string &word) {
            total_words++;
            um[word]++;
          }
      );
    }
    for (auto &t : threads)
      t.join();

    unique_words = um.size();
    pairs.reserve(unique_words);
    for (const auto &p : um)
      pairs.emplace_back(p);
    pr::printResults(total_words, unique_words, pairs, mode + ".freq");
  } else if (mode == "mt_mutex") {
    size_t total_words  = 0;
    size_t unique_words = 0;
    std::unordered_map<std::string, int> um;

    auto partitions = pr::partition(filename, file_size, num_threads);
    std::vector<std::thread> threads;
    threads.reserve(num_threads);

    std::mutex m;

    for (size_t i = 0; i < partitions.size() - 1; ++i) {
      threads.emplace_back(
          pr::processRange, filename, partitions[i], partitions[i + 1],
          [&](const std::string &word) {
            std::lock_guard l{m};
            total_words++;
            um[word]++;
          }
      );
    }
    for (auto &t : threads)
      t.join();

    unique_words = um.size();
    pairs.reserve(unique_words);
    for (const auto &p : um)
      pairs.emplace_back(p);
    pr::printResults(total_words, unique_words, pairs, mode + ".freq");
  } else if (mode == "mt_hmutex") {
    size_t total_words  = 0;
    size_t unique_words = 0;
    HashMapMT<std::string, int> um;

    auto partitions = pr::partition(filename, file_size, num_threads);
    std::vector<std::thread> threads;
    threads.reserve(num_threads);

    for (size_t i = 0; i < partitions.size() - 1; ++i) {
      threads.emplace_back(
          pr::processRange, filename, partitions[i], partitions[i + 1],
          [&](const std::string &word) {
            um.incrementFrequency(word);
          }
      );
    }
    for (auto &t : threads)
      t.join();

    auto kvs     = um.toKeyValuePairs();
    unique_words = kvs.size();
    pairs.reserve(unique_words);
    for (const auto &p : kvs) {
      total_words += p.second;
      pairs.emplace_back(p);
    }
    pr::printResults(total_words, unique_words, pairs, mode + ".freq");
  } else if (mode == "mt_hashes") {
    size_t total_words  = 0;
    size_t unique_words = 0;
    // Main Hashtable
    std::unordered_map<std::string, int> um;
    auto partitions = pr::partition(filename, file_size, num_threads);
    std::vector<std::thread> threads;
    threads.reserve(num_threads);
    // HashTable for each thread
    std::vector<std::unordered_map<std::string, int>> hashes;
    hashes.reserve(num_threads);
    // launch threads
    for (size_t i = 0; i < partitions.size() - 1; ++i) {
      hashes.emplace_back();
      threads.emplace_back(
          pr::processRange, filename, partitions[i], partitions[i + 1],
          [&, i](const std::string &word) { hashes[i][word]++; }
      );
    }
    for (auto &t : threads)
      t.join();
    // Merge hashtables
    for (auto &h : hashes) {
      for (auto &[word, cnt] : h) {
        um[word] += cnt;
        total_words += cnt;
      }
    }

    unique_words = um.size();
    pairs.reserve(unique_words);
    for (const auto &p : um)
      pairs.emplace_back(p);
    pr::printResults(total_words, unique_words, pairs, mode + ".freq");
  } else if (mode == "mt_hhashes") {
    size_t total_words  = 0;
    size_t unique_words = 0;
    std::unordered_map<std::string, int> um;

    auto partitions = pr::partition(filename, file_size, num_threads);
    std::vector<std::thread> threads;
    threads.reserve(num_threads);

    std::vector<HashMap<std::string, int>> hashes;
    hashes.reserve(num_threads);

    for (size_t i = 0; i < partitions.size() - 1; ++i) {
      hashes.emplace_back();
      threads.emplace_back(
          pr::processRange, filename, partitions[i], partitions[i + 1],
          [&, i](const std::string &word) { hashes[i].incrementFrequency(word); }
      );
    }
    for (auto &t : threads)
      t.join();

    for (auto &h : hashes) {
      for (auto &[word, cnt] : h.toKeyValuePairs()) {
        um[word] += cnt;
        total_words += cnt;
      }
    }

    unique_words = um.size();
    pairs.reserve(unique_words);
    for (const auto &p : um)
      pairs.emplace_back(p);
    pr::printResults(total_words, unique_words, pairs, mode + ".freq");
  } else if (mode == "mt_hash_fine") {
    size_t total_words  = 0;
    size_t unique_words = 0;
    HashMapFine<std::string, int> um{};

    auto partitions = pr::partition(filename, file_size, num_threads);
    std::vector<std::thread> threads;
    threads.reserve(num_threads);

    for (size_t i = 0; i < partitions.size() - 1; ++i) {
      threads.emplace_back(
          pr::processRange, filename, partitions[i], partitions[i + 1],
          [&](const std::string &word) { um.incrementFrequency(word); }
      );
    }
    for (auto &t : threads)
      t.join();

    auto kvs     = um.toKeyValuePairs();
    unique_words = kvs.size();
    pairs.reserve(unique_words);
    for (const auto &p : kvs) {
      total_words += p.second;
      pairs.emplace_back(p);
    }
    pr::printResults(total_words, unique_words, pairs, mode + ".freq");
  } else {
    cerr << "Unknown mode '" << mode << "'. Supported modes: freqstd, freq, freqstdf" << endl;
    return 1;
  }

  // print a single total runtime for successful runs
  auto end = steady_clock::now();
  cout << "Total runtime (wall clock) : " << duration_cast<milliseconds>(end - start).count()
       << " ms" << endl;

  return 0;
}
