#pragma once
#include <cstdint>
#include <cstddef>
#include <type_traits>

#ifdef CARS_BIND
inline auto operator_new = (void* (__cdecl*)(std::size_t))(0x0063eb87);
inline auto operator_delete = (void(__cdecl*)(void*))(0x0063f514);

/*
A base class to replace the new and delete operators with Mater-National's. Mater-National classes should extend this.
*/
class MaterManagedObject {
public:
	__forceinline void* operator new(std::size_t sz) {
		return operator_new(sz);
	}
	__forceinline void operator delete(void* ptr) {
		return operator_delete(ptr);
	}
};
#else

__forceinline void* operator_new(std::size_t sz) {
	return ::operator new(sz);
}

__forceinline void operator_delete(void* ptr) {
	::operator delete(ptr);
}

class MaterManagedObject {};
#endif // CARS_BIND

#ifdef CARS_BIND
#define UNIMPLEMENTED(ptr) constexpr std::uint32_t addr = ##ptr; __asm jmp addr
#else
#define UNIMPLEMENTED(ptr) (void)(0)
#endif // CARS_BIND

/*
A thin wrapper around a 32bit signed integer, designed to mimic the BOOL type in the Windows API.
*/
struct Bool {
private:
	std::int32_t inner;
public:
	constexpr Bool() : inner(false) {};
	template<typename T,
		typename std::enable_if_t<std::is_integral_v<T>> = 0>
	explicit constexpr Bool(T value) : inner(value > 0) {};
	constexpr Bool(bool value) : inner(value) {};
	constexpr operator bool() { return inner > 0; }
};

static_assert(sizeof(Bool) == 4);

static constexpr Bool FALSE{ 0 };
static constexpr Bool TRUE{ 1 };