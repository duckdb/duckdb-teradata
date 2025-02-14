#pragma once

#include "duckdb/common/exception.hpp"
#include <type_traits>
#include <cstdint>

namespace duckdb {

class BinaryReader {
public:
	BinaryReader(const char* ptr, size_t len) : ptr(ptr), end(ptr + len), beg(ptr) {
	}

	template<class T>
	T Read() {
		static_assert(std::is_trivial<T>::value, "Type must be trivial");

		CheckSize(sizeof(T));

		T result;
		memcpy(&result, ptr, sizeof(T));
		ptr += sizeof(T);

		return result;
	}

	const char* ReadBytes(size_t size) {
		CheckSize(size);

		const char* result = ptr;
		ptr += size;

		return result;
	}

	void Skip(size_t size) {
		CheckSize(size);
		ptr += size;
	}

private:
	void CheckSize(size_t size) const {
		if (ptr + size > end) {
			throw InvalidInputException("BinaryReader: read out of bounds");
		}
	}

	const char* ptr;
	const char* end;
	const char* beg;
};

}