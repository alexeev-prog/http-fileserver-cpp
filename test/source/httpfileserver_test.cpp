#include "lib.hpp"

auto main() -> int
{
  auto const lib = library {};

  return lib.name == "httpfileserver" ? 0 : 1;
}
