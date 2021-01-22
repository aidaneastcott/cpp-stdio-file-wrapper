
#pragma once
#ifndef CFILE_HPP
#define CFILE_HPP


#include <type_traits>
#include <utility>

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdio>


#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4996)
#define _CRT_SECURE_NO_WARNINGS
#endif


// 'extra' namespace
namespace xtr {

class cfile {
private:
	std::FILE *m_stream;

public:
	enum class mode
	{
		read = 0b0000'0001,
		write = 0b0000'0010,
		append = 0b0000'0100,
		binary = 0b0000'1000,
		extended = 0b0001'0000
	};

	[[nodiscard]] friend constexpr cfile::mode operator|(cfile::mode rhs,
	                                                     cfile::mode lhs) noexcept {

		using underlying_type = std::underlying_type_t<cfile::mode>;

		underlying_type result =
		    static_cast<underlying_type>(rhs) | static_cast<underlying_type>(lhs);

		return static_cast<cfile::mode>(result);
	}

private:
	[[nodiscard]] static const char *to_access_mode_string(const char *access_mode) noexcept {
		return access_mode;
	}

	[[nodiscard]] static const char *to_access_mode_string(mode access_mode) noexcept {

		switch (access_mode) {
		case mode::read:
			return "r";

		case mode::write:
			return "w";

		case mode::append:
			return "a";

		case mode::read | mode::extended:
			return "r+";

		case mode::write | mode::extended:
			return "w+";

		case mode::append | mode::extended:
			return "a+";

		case mode::read | mode::binary:
			return "rb";

		case mode::write | mode::binary:
			return "wb";

		case mode::append | mode::binary:
			return "ab";

		case mode::read | mode::binary | mode::extended:
			return "rb+";

		case mode::write | mode::binary | mode::extended:
			return "wb+";

		case mode::append | mode::binary | mode::extended:
			return "ab+";
		}

		assert(false);
		return nullptr;
	}

public:
	explicit cfile() noexcept = default;

	explicit cfile(std::FILE *other) noexcept : m_stream{other} {}

	cfile(std::nullptr_t) = delete;

	template <typename Type>
	cfile(const char *filename, const Type &mode) noexcept :
	    m_stream{std::fopen(filename, to_access_mode_string(mode))} {}

	cfile(const cfile &) = delete;

	cfile(cfile &&other) noexcept : m_stream{std::exchange(other.m_stream, nullptr)} {}

	cfile &operator=(const cfile &) = delete;

	cfile &operator=(cfile &&other) noexcept {

		if (m_stream != nullptr) {
			std::fclose(m_stream);
		}

		m_stream = std::exchange(other.m_stream, nullptr);

		return *this;
	}

	~cfile() {

		if (m_stream != nullptr) {
			std::fclose(m_stream);
			m_stream = nullptr;
		}
	}

	int reset() noexcept {

		int result = 0;

		if (m_stream != nullptr) {
			result = std::fclose(m_stream);
			m_stream = nullptr;
		}

		return result;
	}

	int reset(std::FILE *other) noexcept {

		int result = 0;

		if (m_stream != nullptr) {
			result = std::fclose(m_stream);
		}

		m_stream = other;

		return result;
	}

	int reset(std::nullptr_t) = delete;

	[[nodiscard]] std::FILE *release() noexcept {

		std::FILE *result = m_stream;
		m_stream = nullptr;

		return result;
	}

	[[nodiscard]] std::FILE *get() const noexcept {
		return m_stream;
	}

	[[nodiscard]] explicit operator bool() const noexcept {
		return m_stream != nullptr;
	}

	[[nodiscard]] friend bool operator==(const cfile &object, std::nullptr_t) noexcept {
		return object.m_stream == nullptr;
	}

	[[nodiscard]] friend bool operator==(std::nullptr_t, const cfile &object) noexcept {
		return object.m_stream == nullptr;
	}

	[[nodiscard]] friend bool operator!=(const cfile &object, std::nullptr_t) noexcept {
		return object.m_stream != nullptr;
	}

	[[nodiscard]] friend bool operator!=(std::nullptr_t, const cfile &object) noexcept {
		return object.m_stream != nullptr;
	}

	// File access

	template <typename Type>
	cfile &fopen(const char *filename, const Type &mode) noexcept {

		assert(m_stream == nullptr);

		m_stream = std::fopen(filename, to_access_mode_string(mode));

		return *this;
	}

	template <typename Type>
	cfile &freopen(const char *filename, const Type &mode) noexcept {

		m_stream = std::freopen(filename, to_access_mode_string(mode), m_stream);

		return *this;
	}

	int fclose() noexcept {

		int result = std::fclose(m_stream);
		m_stream = nullptr;

		return result;
	}

	int fflush() noexcept {
		return std::fflush(m_stream);
	}

	void setbuf(char *buffer) noexcept {
		std::setbuf(m_stream, buffer);
	}

	int setvbuf(std::nullptr_t) noexcept {
		return std::setvbuf(m_stream, nullptr, _IONBF, 0);
	}

	int setvbuf(char *buffer) noexcept {

		int mode = _IOFBF;
		std::size_t size = BUFSIZ;

		if (buffer == nullptr) {
			mode = _IONBF;
			size = 0;
		}

		return std::setvbuf(m_stream, buffer, mode, size);
	}

	int setvbuf(char *buffer, int mode, std::size_t size) noexcept {
		return std::setvbuf(m_stream, buffer, mode, size);
	}

	// Direct input/output

	std::size_t fread(void *buffer, std::size_t size, std::size_t count) noexcept {
		return std::fread(buffer, size, count, m_stream);
	}

	template <typename Type>
	std::size_t fread(Type *buffer, std::size_t count) noexcept {
		return fread(buffer, sizeof(Type), count);
	}

	template <typename Type, std::size_t Size>
	std::size_t fread(Type (&buffer)[Size]) noexcept {
		return fread(buffer, sizeof(Type), Size);
	}

	std::size_t fwrite(const void *buffer, std::size_t size, std::size_t count) noexcept {
		return std::fwrite(buffer, size, count, m_stream);
	}

	template <typename Type>
	std::size_t fwrite(const Type *buffer, std::size_t count) noexcept {
		return fwrite(buffer, sizeof(Type), count);
	}

	template <typename Type, std::size_t Size>
	std::size_t fwrite(const Type (&buffer)[Size]) noexcept {
		return fwrite(buffer, sizeof(Type), Size);
	}

	// Unformatted input/output

	[[nodiscard]] int fgetc() noexcept {
		return std::fgetc(m_stream);
	}

	char *fgets(char *buffer, int count) noexcept {
		return std::fgets(buffer, count, m_stream);
	}

	template <std::size_t Size>
	char *fgets(char (&buffer)[Size]) noexcept {
		return fgets(buffer, Size);
	}

	int fputc(int character) noexcept {
		return std::fputc(character, m_stream);
	}

	int fputs(const char *buffer) noexcept {
		return std::fputs(buffer, m_stream);
	}

	int ungetc(int character) noexcept {
		return std::ungetc(character, m_stream);
	}

	// Formatted input/output

	template <typename... Args>
	int fscanf(const char *format, Args &&...args) noexcept {
		return std::fscanf(m_stream, format, std::forward<Args>(args)...);
	}

	int vfscanf(const char *format, va_list list) noexcept {
		return std::vfscanf(m_stream, format, list);
	}

	template <typename... Args>
	int fprintf(const char *format, Args &&...args) noexcept {
		return std::fprintf(m_stream, format, std::forward<Args>(args)...);
	}

	int vfprintf(const char *format, va_list list) noexcept {
		return std::vfprintf(m_stream, format, list);
	}

	// File positioning

	[[nodiscard]] long ftell() noexcept {
		return std::ftell(m_stream);
	}

	int fgetpos(std::fpos_t *position) noexcept {
		return std::fgetpos(m_stream, position);
	}

	int fseek(long offset, int origin) noexcept {
		return std::fseek(m_stream, offset, origin);
	}

	int fsetpos(const std::fpos_t *position) noexcept {
		return std::fsetpos(m_stream, position);
	}

	void rewind() noexcept {
		std::rewind(m_stream);
	}

	// Error handling

	void clearerr() noexcept {
		std::clearerr(m_stream);
	}

	[[nodiscard]] int feof() noexcept {
		return std::feof(m_stream);
	}

	[[nodiscard]] int ferror() noexcept {
		return std::ferror(m_stream);
	}

	static void perror(const char *message) noexcept {
		std::perror(message);
	}

	// Operations on files

	static int remove(const char *filename) noexcept {
		return std::remove(filename);
	}

	static int rename(const char *old_filename, const char *new_filename) noexcept {
		return std::rename(old_filename, new_filename);
	}

	[[nodiscard]] static cfile tmpfile() noexcept {
		return cfile{std::tmpfile()};
	}

	static char *tmpnam(char *buffer) noexcept {
		return std::tmpnam(buffer);
	}
};

using mode = cfile::mode;

} // namespace xtr


#ifdef _MSC_VER
#pragma warning(pop)
#endif


#endif // CFILE_HPP
