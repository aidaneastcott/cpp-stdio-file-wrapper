
#pragma once
#ifndef CFILE_HPP
#define CFILE_HPP


#include <string>
#include <type_traits>
#include <utility>

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdio>


#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4996)
#endif


namespace xtr { // 'extra' namespace

class CFile {
private:

	std::FILE *stream{};

public:

	enum class Mode {
		Read     = 0b0000'0001,
		Write    = 0b0000'0010,
		Append   = 0b0000'0100,
		Binary   = 0b0000'1000,
		Extended = 0b0001'0000
	};


	[[nodiscard]] friend constexpr CFile::Mode operator|(CFile::Mode rhs, CFile::Mode lhs) noexcept {
		using underlying_type = std::underlying_type_t<CFile::Mode>;
		return static_cast<CFile::Mode>(
			static_cast<underlying_type>(rhs) |
			static_cast<underlying_type>(lhs)
		);
	}


private:

	[[nodiscard]] static const char *to_cstring(Mode mode) noexcept {

		switch (mode) {
		case Mode::Read:
			return "r";

		case Mode::Write:
			return "w";

		case Mode::Append:
			return "a";

		case Mode::Read | Mode::Extended:
			return "r+";

		case Mode::Write | Mode::Extended:
			return "w+";

		case Mode::Append | Mode::Extended:
			return "a+";

		case Mode::Read | Mode::Binary:
			return "rb";

		case Mode::Write | Mode::Binary:
			return "wb";

		case Mode::Append | Mode::Binary:
			return "ab";

		case Mode::Read | Mode::Binary | Mode::Extended:
			return "rb+";

		case Mode::Write | Mode::Binary | Mode::Extended:
			return "wb+";

		case Mode::Append | Mode::Binary | Mode::Extended:
			return "ab+";
		}

		assert(false);
		return nullptr;
	}


	[[nodiscard]] static const char *to_cstring(const std::string &value) noexcept {
		return value.c_str();
	}


	[[nodiscard]] static const char *to_cstring(const char *value) noexcept {
		return value;
	}


	template <typename T>
	struct is_string :
		std::disjunction<
			std::is_same<std::decay_t<T>, const char *>,
			std::is_same<std::decay_t<T>,       char *>,
			std::conjunction<
				std::is_same<std::decay_t<T>, std::string>,
				std::negation<std::is_volatile<std::remove_reference_t<T>>>
			>
		> {};


	template <typename T>
	static constexpr bool is_string_v = is_string<T>::value;


	template <typename T>
	struct is_access_mode :
		std::disjunction<
			std::is_same<std::decay_t<T>, Mode>,
			is_string<T>
		> {};


	template <typename T>
	static constexpr bool is_access_mode_v = is_access_mode<T>::value;


public:

	explicit CFile() noexcept = default;


	explicit CFile(std::FILE *other) noexcept :
		stream{other} {}


	CFile(std::nullptr_t) = delete;


	template <typename T, typename U,
		std::enable_if_t<
			std::conjunction_v<
				is_string<T>,
				is_access_mode<U>
			>,
		int> = 0>
	CFile(const T &filename, const U &mode) noexcept :
		stream{std::fopen(
				to_cstring(filename),
				to_cstring(mode)
			)} {}


	CFile(const CFile &) = delete;


	CFile(CFile &&other) noexcept :
		stream{std::exchange(other.stream, nullptr)} {}


	CFile &operator=(const CFile &) = delete;


	CFile &operator=(CFile &&other) noexcept {
		if (stream != nullptr) {
			std::fclose(stream);
		}
		stream = std::exchange(other.stream, nullptr);
		return *this;
	}


	~CFile() {
		if (stream != nullptr) {
			std::fclose(stream);
			stream = nullptr;
		}
	}


	int reset() noexcept {
		int result = 0;
		if (stream != nullptr) {
			result = std::fclose(stream);
			stream = nullptr;
		}
		return result;
	}


	int reset(std::FILE *other) noexcept {
		int result = 0;
		if (stream != nullptr) {
			result = std::fclose(stream);
		}
		stream = other;
		return result;
	}


	int reset(std::nullptr_t) = delete;


	[[nodiscard]] std::FILE *release() noexcept {
		std::FILE *result = stream;
		stream = nullptr;
		return result;
	}


	[[nodiscard]] std::FILE *&get() noexcept {
		return stream;
	}


	[[nodiscard]] friend bool operator==(const CFile &object, std::nullptr_t) noexcept {
		return object.stream == nullptr;
	}


	[[nodiscard]] friend bool operator==(std::nullptr_t, const CFile &object) noexcept {
		return object.stream == nullptr;
	}


	[[nodiscard]] friend bool operator!=(const CFile &object, std::nullptr_t) noexcept {
		return object.stream != nullptr;
	}


	[[nodiscard]] friend bool operator!=(std::nullptr_t, const CFile &object) noexcept {
		return object.stream != nullptr;
	}


	// File access

	template <typename T, typename U,
		std::enable_if_t<
			std::conjunction_v<
				is_string<T>,
				is_access_mode<U>
			>,
		int> = 0>
	CFile &fopen(const T &filename, const U &mode) noexcept {
		assert(stream == nullptr);
		stream = std::fopen(
			to_cstring(filename),
			to_cstring(mode)
		);
		return *this;
	}


	template <typename T, typename U,
		std::enable_if_t<
			std::conjunction_v<
				is_string<T>,
				is_access_mode<U>
			>,
		int> = 0>
	CFile &freopen(const T &filename, const U &mode) noexcept {
		stream = std::freopen(
			to_cstring(filename),
			to_cstring(mode),
			stream
		);
		return *this;
	}


	int fclose() noexcept {
		const int result = std::fclose(stream);
		stream = nullptr;
		return result;
	}


	int fflush() noexcept {
		return std::fflush(stream);
	}


	void setbuf(char *buffer) noexcept {
		std::setbuf(stream, buffer);
	}


	int setvbuf(std::nullptr_t) noexcept {
		return std::setvbuf(stream, nullptr, _IONBF, 0);
	}


	int setvbuf(char *buffer) noexcept {
		int mode = _IOFBF;
		std::size_t size = BUFSIZ;
		if (buffer == nullptr) {
			mode = _IONBF;
			size = 0;
		}
		return std::setvbuf(stream, buffer, mode, size);
	}


	int setvbuf(char *buffer, int mode, std::size_t size) noexcept {
		return std::setvbuf(stream, buffer, mode, size);
	}


	// Direct input/output

	std::size_t fread(void *buffer, std::size_t size, std::size_t count) noexcept {
		return std::fread(buffer, size, count, stream);
	}


	template <typename T>
	std::size_t fread(T *buffer, std::size_t count) noexcept {
		return fread(buffer, sizeof(T), count);
	}


	template <typename T, std::size_t N>
	std::size_t fread(T(&buffer)[N]) noexcept {
		return fread(buffer, sizeof(T), N);
	}


	std::size_t fwrite(const void *buffer, std::size_t size, std::size_t count) noexcept {
		return std::fwrite(buffer, size, count, stream);
	}


	template <typename T>
	std::size_t fwrite(const T *buffer, std::size_t count) noexcept {
		return fwrite(buffer, sizeof(T), count);
	}


	template <typename T, std::size_t N>
	std::size_t fwrite(const T(&buffer)[N]) noexcept {
		return fwrite(buffer, sizeof(T), N);
	}


	// Unformatted input/output

	[[nodiscard]] int fgetc() noexcept {
		return std::fgetc(stream);
	}


	char *fgets(char *buffer, int count) noexcept {
		return std::fgets(buffer, count, stream);
	}


	template <std::size_t N>
	char *fgets(char(&buffer)[N]) noexcept {
		return fgets(buffer, N);
	}


	int fputc(int character) noexcept {
		return std::fputc(character, stream);
	}


	template <typename T,
		std::enable_if_t<
			is_string_v<T>,
		int> = 0>
	int fputs(T &&buffer) noexcept {
		return std::fputs(
			to_cstring(std::forward<T>(buffer)),
			stream
		);
	}


	int ungetc(int character) noexcept {
		return std::ungetc(character, stream);
	}


	// Formatted input/output

	template <typename T, typename ...Args,
		std::enable_if_t<
			is_string_v<T>,
		int> = 0>
	int fscanf(const T &format, Args &&...args) noexcept {
		return std::fscanf(
			stream,
			to_cstring(format),
			std::forward<Args>(args)...
		);
	}


	template <typename T,
		std::enable_if_t<
			is_string_v<T>,
		int> = 0>
	int vfscanf(const T &format, va_list list) noexcept {
		return std::vfscanf(
			stream,
			to_cstring(format),
			list
		);
	}


	template <typename T, typename ...Args,
		std::enable_if_t<
			is_string_v<T>,
		int> = 0>
	int fprintf(const T &format, Args &&...args) noexcept {
		return std::fprintf(
			stream,
			to_cstring(format),
			std::forward<Args>(args)...
		);
	}


	template <typename T,
		std::enable_if_t<
			is_string_v<T>,
		int> = 0>
	int vfprintf(const T &format, va_list list) noexcept {
		return std::vfprintf(
			stream,
			to_cstring(format),
			list
		);
	}


	// File positioning

	[[nodiscard]] long ftell() noexcept {
		return std::ftell(stream);
	}


	int fgetpos(std::fpos_t *position) noexcept {
		return std::fgetpos(stream, position);
	}


	int fseek(long offset, int origin) noexcept {
		return std::fseek(stream, offset, origin);
	}


	int fsetpos(const std::fpos_t *position) noexcept {
		return std::fsetpos(stream, position);
	}


	void rewind() noexcept {
		std::rewind(stream);
	}


	// Error handling

	void clearerr() noexcept {
		std::clearerr(stream);
	}


	[[nodiscard]] int feof() noexcept {
		return std::feof(stream);
	}


	[[nodiscard]] int ferror() noexcept {
		return std::ferror(stream);
	}


	template <typename T,
		std::enable_if_t<
			is_string_v<T>,
		int> = 0>
	static void perror(const T &message) noexcept {
		std::perror(to_cstring(message));
	}


	// Operations on files

	template <typename T,
		std::enable_if_t<
			is_string_v<T>,
		int> = 0>
	static int remove(const T &filename) noexcept {
		return std::remove(to_cstring(filename));
	}


	template <typename T, typename U,
		std::enable_if_t<
			std::conjunction_v<
				is_string<T>,
				is_string<U>
			>,
		int> = 0>
	static int rename(const T &oldfilename, const U &newfilename) noexcept {
		return std::rename(
			to_cstring(oldfilename),
			to_cstring(newfilename)
		);
	}


	[[nodiscard]] static CFile tmpfile() noexcept {
		return CFile{std::tmpfile()};
	}


	static char *tmpnam(char *buffer) noexcept {
		return std::tmpnam(buffer);
	}
};

}


#ifdef _MSC_VER
#pragma warning(pop)
#endif


#endif // CFILE_HPP
