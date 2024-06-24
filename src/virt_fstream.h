#pragma once

#include "file_entries.h"
#include "virt_filebuf.h"
#include <istream>

namespace virtfiles
{
	template <class CharT, class Traits = std::char_traits<CharT>>
	class basic_ifstream : public std::basic_istream<CharT, Traits>
	{
	public:
		using _Mybase = std::basic_istream<CharT, Traits>;
		using _Mybuf = basic_filebuf<CharT, Traits>;
		using _Basebuf = typename _Mybuf::_Mybase;
		using _Myios = std::basic_ios<CharT, Traits>;

		using char_type = CharT;
		using traits_type = Traits;
		using int_type = typename Traits::int_type;
		using pos_type = typename Traits::pos_type;
		using off_type = typename Traits::off_type;

		basic_ifstream()
			: _Mybase(new _Mybuf)
		{
		}

		explicit basic_ifstream(
			const char* filename,
			_Myios::openmode mode = _Myios::in
		) : basic_ifstream()
		{
			if (!rdbuf()->open(filename, mode | _Myios::in))
			{
				_Myios::setstate(_Myios::failbit);
			}
		}

		explicit basic_ifstream(
			const std::string& filename,
			_Myios::openmode mode = _Myios::in
		) : basic_ifstream(filename.c_str(), mode)
		{
		}

		basic_ifstream(const basic_ifstream&) = delete;

		basic_ifstream(basic_ifstream&& other) noexcept
			: _Mybase(std::move(other))
		{
			_Myios::set_rdbuf(new _Mybuf(std::move(*other.rdbuf())));
		}

		virtual ~basic_ifstream()
		{
			delete _Myios::rdbuf();
		}

		basic_ifstream& operator=(basic_ifstream&& other) noexcept
		{
			_Mybase::operator=(std::move(other));
			rdbuf()->operator=(std::move(*other.rdbuf()));

			return *this;
		}

		_Mybuf* rdbuf() const
		{
			return reinterpret_cast<_Mybuf*>(_Myios::rdbuf());
		}

		void swap(basic_ifstream& other)
		{
			_Mybase::swap(other);
			rdbuf()->swap(other.rdbuf());
		}

		bool is_open() const
		{
			return rdbuf->is_open();
		}

		void open(const char* filename,
			_Myios::openmode mode = _Myios::in)
		{
			rdbuf()->open(filename, mode | _Myios::in);
		}

		void open(const std::string& filename,
			_Myios::openmode mode = _Myios::in)
		{
			rdbuf()->open(filename.c_str(), mode | _Myios::in);
		}

		void close()
		{
			if (!rdbuf()->close())
			{
				_Myios::setstate(_Myios::failbit);
			}
		}
	};

	typedef basic_ifstream<char> ifstream;
	typedef basic_ifstream<wchar_t> iwfstream;

	template <class CharT, class Traits = std::char_traits<CharT>>
	class basic_ofstream : public std::basic_ostream<CharT, Traits>
	{
	public:
		using _Mybase = std::basic_ostream<CharT, Traits>;
		using _Mybuf = basic_filebuf<CharT, Traits>;
		using _Basebuf = typename _Mybuf::_Mybase;
		using _Myios = std::basic_ios<CharT, Traits>;

		using char_type = CharT;
		using traits_type = Traits;
		using int_type = typename Traits::int_type;
		using pos_type = typename Traits::pos_type;
		using off_type = typename Traits::off_type;

		basic_ofstream()
			: _Mybase(new _Mybuf)
		{
		}

		explicit basic_ofstream(
			const char* filename,
			_Myios::openmode mode = _Myios::out
		) : basic_ofstream()
		{
			if (!rdbuf()->open(filename, mode | _Myios::out))
			{
				_Myios::setstate(_Myios::failbit);
			}
		}

		explicit basic_ofstream(
			const std::string& filename,
			_Myios::openmode mode = _Myios::out
		) : basic_ofstream(filename.c_str(), mode)
		{
		}

		basic_ofstream(const basic_ofstream&) = delete;

		basic_ofstream(basic_ofstream&& other) noexcept
			: _Mybase(std::move(other))
		{
			_Myios::set_rdbuf(new _Mybuf(std::move(*other.rdbuf())));
		}

		virtual ~basic_ofstream()
		{
			delete _Myios::rdbuf();
		}

		basic_ofstream& operator=(basic_ofstream&& other) noexcept
		{
			_Mybase::operator=(std::move(other));
			rdbuf()->operator=(std::move(*other.rdbuf()));

			return *this;
		}

		_Mybuf* rdbuf() const
		{
			return reinterpret_cast<_Mybuf*>(_Myios::rdbuf());
		}

		void swap(basic_ofstream& other)
		{
			_Mybase::swap(other);
			rdbuf()->swap(other.rdbuf());
		}

		bool is_open() const
		{
			return rdbuf->is_open();
		}

		void open(const char* filename,
			_Myios::openmode mode = _Myios::out)
		{
			rdbuf()->open(filename, mode | _Myios::out);
		}

		void open(const std::string& filename,
			_Myios::openmode mode = _Myios::out)
		{
			rdbuf()->open(filename.c_str(), mode | _Myios::out);
		}

		void close()
		{
			if (!rdbuf()->close())
			{
				_Myios::setstate(_Myios::failbit);
			}
		}
	};

	typedef basic_ofstream<char> ofstream;
	typedef basic_ofstream<wchar_t> owfstream;

	template <class CharT, class Traits = std::char_traits<CharT>>
	class basic_fstream : public std::basic_iostream<CharT, Traits>
	{
	public:
		using _Mybase = std::basic_iostream<CharT, Traits>;
		using _Mybuf = basic_filebuf<CharT, Traits>;
		using _Basebuf = typename _Mybuf::_Mybase;
		using _Myios = std::basic_ios<CharT, Traits>;

		using char_type = CharT;
		using traits_type = Traits;
		using int_type = typename Traits::int_type;
		using pos_type = typename Traits::pos_type;
		using off_type = typename Traits::off_type;

		basic_fstream()
			: _Mybase(new _Mybuf)
		{
		}

		explicit basic_fstream(
			const char* filename,
			_Myios::openmode mode = _Myios::in | _Myios::out
		) : basic_fstream()
		{
			if (!rdbuf()->open(filename, mode))
			{
				_Myios::setstate(_Myios::failbit);
			}
		}

		explicit basic_fstream(
			const std::string& filename,
			_Myios::openmode mode = _Myios::in | _Myios::out
		) : basic_fstream(filename.c_str(), mode)
		{
		}

		basic_fstream(const basic_fstream&) = delete;

		basic_fstream(basic_fstream&& other) noexcept
			: _Mybase(std::move(other))
		{
			_Myios::set_rdbuf(new _Mybuf(std::move(*other.rdbuf())));
		}

		virtual ~basic_fstream()
		{
			delete _Myios::rdbuf();
		}

		basic_fstream& operator=(basic_fstream&& other) noexcept
		{
			_Mybase::operator=(std::move(other));
			rdbuf()->operator=(std::move(*other.rdbuf()));

			return *this;
		}

		_Mybuf* rdbuf() const
		{
			return reinterpret_cast<_Mybuf*>(_Myios::rdbuf());
		}

		void swap(basic_fstream& other)
		{
			_Mybase::swap(other);
			rdbuf()->swap(other.rdbuf());
		}

		bool is_open() const
		{
			return rdbuf->is_open();
		}

		void open(const char* filename,
			_Myios::openmode mode = _Myios::in | _Myios::out)
		{
			rdbuf()->open(filename, mode);
		}

		void open(const std::string& filename,
			_Myios::openmode mode = _Myios::in | _Myios::out)
		{
			rdbuf()->open(filename.c_str(), mode);
		}

		void close()
		{
			if (!rdbuf()->close())
			{
				_Myios::setstate(_Myios::failbit);
			}
		}
	};

	typedef basic_fstream<char> fstream;
	typedef basic_fstream<wchar_t> wfstream;
};

namespace std
{
	template <class CharT, class Traits>
	void swap(
		basic_ifstream<CharT, Traits>& lhs,
		basic_ifstream<CharT, Traits>& rhs
	)
	{
		lhs.swap(rhs);
	}

	template <class CharT, class Traits>
	void swap(
		basic_ofstream<CharT, Traits>& lhs,
		basic_ofstream<CharT, Traits>& rhs
	)
	{
		lhs.swap(rhs);
	}

	template <class CharT, class Traits>
	void swap(
		basic_fstream<CharT, Traits>& lhs,
		basic_fstream<CharT, Traits>& rhs
	)
	{
		lhs.swap(rhs);
	}
};

#define ifstream ::virtfiles::ifstream
#define ofstream ::virtfiles::ofstream
#define fstream ::virtfiles::fstream
