#include <cstdio>
#include <cstdlib>

#include "core/panic.hpp"

namespace beyond {

[[noreturn]] auto panic(std::string_view msg) -> void
{
  std::fputs(msg.data(), stderr);
  std::fflush(stderr);
  std::abort();
}

} // namespace beyond
