#include "String.h"

namespace pr {

// TODO: Implement constructor e.g. using initialization list
String::String(const char *s) : data{newcopy(s)} {
  std::cout << "String constructor called for: " << s << std::endl;
}

String::~String() {
  std::cout << "String destructor called for: " << (data ? data : "(null)") << std::endl;
  delete[] data;
}

// TODO

String::String(const String &other) : data{newcopy(other.data)} {}

std::ostream &operator<<(std::ostream &os, const String &str) {
  os << str.data;
  return os;
}

String &String::operator=(const String &other) {
  if (data != other.data)
    data = newcopy(other.data);
  return *this;
}

bool operator==(const String &a, const String &b) { return 0 == compare(a.data, b.data); }

bool String::operator<(const String &other) const { return compare(data, other.data) < 0; }

String operator+(const String &a, const String &b) { return String{newcat(a.data, b.data)}; }

String::String(String &&other) noexcept : data{other.data} { other.data = nullptr; }

String &String::operator=(String &&other) noexcept {
  const char *s = other.data;
  other.data    = nullptr;
  data          = s;
  return *this;
}

}  // namespace pr
