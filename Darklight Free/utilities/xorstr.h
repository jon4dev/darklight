#pragma once

#include <algorithm>
#include <array>

#ifdef _KERNEL_MODE
namespace std
{

	template <class _Ty>
	struct remove_reference {
		using type = _Ty;
	};

	template <class _Ty>
	struct remove_reference<_Ty&> {
		using type = _Ty;
	};

	template <class _Ty>
	struct remove_reference<_Ty&&> {
		using type = _Ty;
	};

	template <class _Ty>
	using remove_reference_t = typename remove_reference<_Ty>::type;


	template <class _Ty>
	struct remove_const {
		using type = _Ty;
	};

	template <class _Ty>
	struct remove_const<const _Ty> {
		using type = _Ty;
	};

	template <class _Ty>
	using remove_const_t = typename remove_const<_Ty>::type;
}
#else
#include <type_traits>
#endif

namespace tec
{
	template<class _Ty>
	using clean_type = typename std::remove_const_t<std::remove_reference_t<_Ty>>;

	template <int _size, char _key1, char _key2, typename T>
	class teCrypter
	{
	public:
		__forceinline constexpr teCrypter(T* data)
		{
			crypt(data);
		}

		__forceinline T* get()
		{
			return _storage;
		}

		__forceinline int size()
		{
			return _size;
		}

		__forceinline  char key()
		{
			return _key1;
		}

		__forceinline  T* encrypt()
		{
			if (!isEncrypted())
				crypt(_storage);

			return _storage;
		}

		__forceinline  T* decrypt()
		{
			if (isEncrypted())
				crypt(_storage);

			return _storage;
		}

		__forceinline bool isEncrypted()
		{
			return _storage[_size - 1] != 0;
		}

		__forceinline void clear()
		{
			for (int i = 0; i < _size; ++i)
			{
				_storage[i] = 0;
			}
		}

		__forceinline operator T* ()
		{
			decrypt();

			return _storage;
		}

	private:
		__forceinline constexpr void crypt(T* data)
		{
			for (int i = 0; i < _size; ++i)
			{
				_storage[i] = data[i] ^ (_key1 + i % (1 + _key2));
			}
		}

		T _storage[_size]{};
	};
}

#define encrypt(str) teCrypt_key(str, __TIME__[4], __TIME__[7])
#define teCrypt_key(str, key1, key2) []() { \
			constexpr static auto crypted = tec::teCrypter \
				<sizeof(str) / sizeof(str[0]), key1, key2, tec::clean_type<decltype(str[0])>>((tec::clean_type<decltype(str[0])>*)str); \
					return crypted; }()

namespace random {
	constexpr auto time = __TIME__;
	constexpr auto seed = static_cast<unsigned>(time[7]) + static_cast<unsigned>(time[6]) * 10 + static_cast<unsigned>(time[4]) * 60 + static_cast<unsigned>(time[3]) * 600 + static_cast<unsigned>(time[1]) * 3600 + static_cast<unsigned>(time[0]) * 36000;

	template <int n>
	struct gen {
	private:
		static constexpr unsigned a = 16807;
		static constexpr unsigned m = 2147483647;

		static constexpr unsigned s = gen<n - 1>::value;
		static constexpr unsigned lo = a * (s & 0xFFFFu);
		static constexpr unsigned hi = a * (s >> 16u);
		static constexpr unsigned lo2 = lo + ((hi & 0x7FFFu) << 16u);
		static constexpr unsigned hi2 = hi >> 15u;
		static constexpr unsigned lo3 = lo2 + hi;

	public:
		static constexpr unsigned max = m;
		static constexpr unsigned value = lo3 > m ? lo3 - m : lo3;
	};

	template <>
	struct gen<0> {
		static constexpr unsigned value = seed;
	};

	template <int n, int m>
	struct _int {
		static constexpr auto value = gen<n + 1>::value % m;
	};

	template <int n>
	struct _char {
		static const char value = static_cast<char>(1 + _int<n, 0x7F - 1>::value);
	};
};

template <size_t n, char k>
struct xorstr {
private:
	static constexpr char enc(const char c) {
		return c ^ k;
	}

public:
	template <size_t... s>
	constexpr __forceinline xorstr(const char* str, std::index_sequence<s...>) : encrypted{ enc(str[s])... } { }

	__forceinline std::string dec() {
		std::string dec;
		dec.resize(n);

		for (auto i = 0; i < n; i++)
			dec[i] = encrypted[i] ^ k;

		return dec;
	}

	__forceinline std::string ot(bool decrypt = true) {
		std::string dec;
		dec.resize(n);

		for (auto i = 0; i < n; i++) {
			dec[i] = decrypt ? (encrypted[i] ^ k) : encrypted[i];
		}

		return dec;
	}

	std::array<char, n> encrypted{};
};

#define _(s) xorstr<sizeof(s), random::_char<__COUNTER__>::value>(s, std::make_index_sequence<sizeof(s)>()).dec().c_str()