#pragma once

#include <cstdint>

using u64 = uint64_t;
using u32 = uint32_t;
using u16 = uint16_t;
using u8  = uint8_t;

using i64 = int64_t;
using i32 = int32_t;
using i16 = int16_t;
using i8  = int8_t;

using f64 = double;
using f32 = float;

/**
 * \brief define number of registers which can be used for evaluating expression
 */
#ifndef ARCH
	#error "Architecture not specified"
#elif ARCH == SILENT
	#define ARCH_REG_NUM 16
#elif
	#error "Unknown architecture"
#endif

/**
 * \brief custom extension
 */
namespace std
{
	namespace ext
	{
		static inline bool isprintable(int c) { return (c > 31 && c < 127); }
		static inline bool ischar(int c)      { return (c > 32 && c < 127); }
	}
}
