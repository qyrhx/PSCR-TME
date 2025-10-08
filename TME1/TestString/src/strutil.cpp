// strutil.cpp
#include "strutil.h"

namespace pr {

size_t length(const char *s) {
  const char *p = s;
  for (; *p; ++p) /*-*/
    ;
  return p - s;
}

char *newcopy(const char *s) {
  if (! s)
    return nullptr;
  size_t len = length(s);
  char *n    = new char[len + 1];
  for (size_t i = 0; i <= len; ++i)
    n[i] = s[i];
  return n;
}

int compare(const char *a, const char *b) {
  if (! a || ! b)
    return -2;
  for (int i = 0; a[i] or b[i]; ++i) {
    if (a[i] < b[i])
      return -1;
    if (a[i] > b[i])
      return 1;
  }
  return 0;
}

char *newcat(const char *a, const char *b) {
  if (! a || ! b)
    return nullptr;
  size_t len = length(a) + length(b);
  char *res  = new char[len + 1];
  size_t i   = 0;
  for (size_t j = 0; a[j]; ++i, ++j)
    res[i] = a[j];
  for (size_t k = 0; b[k]; ++i, ++k)
    res[i] = b[k];
  res[i] = 0;
  return res;
}

}  // namespace pr
