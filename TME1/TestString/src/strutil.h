#pragma once

#include <cstddef>  // for size_t

namespace pr {

size_t length(const char *s);

char *newcopy(const char *s);

int compare(const char *a, const char *b);

char *newcat(const char *a, const char *b);

}  // namespace pr
