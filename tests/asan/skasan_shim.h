// Force-included (-include) into every translation unit of the asan preset.
//
// Skia is built without AddressSanitizer, but its headers change TArray's layout when they see it
// (include/private/SkASAN.h defines SK_SANITIZE_ADDRESS, and SkTArray.h adds a field under it), so our sanitized
// code sees TypefaceFontProvider as 96 bytes where Skia allocated 88, and ASan stops on a false
// new-delete-type-mismatch. Defining SkASAN_DEFINED first, with SkASAN.h's no-op hooks, keeps Skia's unsanitized
// layout. A Skia built with ASan wouldn't need it.
#pragma once
#define SkASAN_DEFINED
#include <cstddef>
static inline void sk_asan_poison_memory_region(void const volatile *, size_t) {}
static inline void sk_asan_unpoison_memory_region(void const volatile *, size_t) {}
static inline int sk_asan_address_is_poisoned(void const volatile *) { return 0; }
