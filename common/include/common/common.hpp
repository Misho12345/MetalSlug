#pragma once

#include "string.hpp"
#include "string_view.hpp"

#include "vector.hpp"
#include "span.hpp"

#include "list.hpp"

#include "unique_ptr.hpp"

#include "memory.hpp"
#include "algorithm.hpp"

#include "file_io.hpp"


constexpr size_t operator ""_zu(const unsigned long long n) { return n; }
