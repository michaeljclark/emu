#pragma once

void *alloca (size_t __size);

#define alloca(size) __builtin_alloca (size)
