#pragma once

#define GUARD_STRINGIFY(...) #__VA_ARGS__
#define GUARD_STRINGIFY2(...) GUARD_STRINGIFY_IMPL(__VA_ARGS__)

#define GUARD_CONCAT(a, b) a##b
#define GUARD_CONCAT2(a, b) GUARD_CONCAT(a, b)