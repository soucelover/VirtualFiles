#pragma once

#include "file_entries.h"
#include <locale>
#include <streambuf>


namespace virtfiles
{
	template <class CharT, class Traits = std::char_traits<CharT>>
	class basic_filebuf : public std::basic_streambuf<CharT, Traits>
	{
	public:
		using _Mybase = std::basic_streambuf<CharT, Traits>;
		using _Myios = std::ios_base;
		using _State_t = typename Traits::state_type;
		using _Cvt = std::codecvt<CharT, char, typename Traits::state_type>;

		using char_type = CharT;
		using traits_type = Traits;
		using int_type = typename Traits::int_type;
		using pos_type = typename Traits::pos_type;
		using off_type = typename Traits::off_type;

		basic_filebuf()
			: _Mybase()
		{
			_init(nullptr);
		}

		basic_filebuf(const basic_filebuf&) = delete;

		basic_filebuf(basic_filebuf&& other) noexcept
			: _Mybase(std::move(other))
		{
			_init(other);
			other._init(nullptr);
		}

		basic_filebuf& operator=(const basic_filebuf&) = delete;

		basic_filebuf& operator=(basic_filebuf&& other)
		{
			// close opened file
			close();

			_init(other);
			other._init(nullptr);
		}

		virtual ~basic_filebuf() noexcept
		{
			close();
		}

		void swap(basic_filebuf& other)
		{
			basic_filebuf state_save = std::move(other);

			other._init(*this);
			this->_init(state_save);

			state_save._init(nullptr);
		}

		bool is_open() const
		{
			return myfile != nullptr;
		}

		basic_filebuf* open(const char* filepath, _Myios::openmode mode)
		{
			if (myfile || !handle_openmode(mode))
			{
				return nullptr;
			}

			try
			{
				myfile = &fs.get_root()->lookup(filepath).as_file();
			}
			catch (const filesystem_exception&)
			{
				myfile = nullptr;
			}

			bool _only_out = (mode & (_Myios::out | _Myios::in)) == _Myios::out;

			_init_mycvt(std::use_facet<_Cvt>(_Mybase::getloc()));

			if (myfile) // file exists
			{
				// if shouldn't truncate
				if (!(mode & _Myios::trunc || _only_out) || mode & _Myios::app)
				{
					// read content
					// NOTE: when file::read_bytes will handle text mode, fix this line
					if (!_init_buffer_from(myfile, mode))
					{
						myfile = nullptr;
						return nullptr;
					}

					return this;
				}

				myfile->empty(); // truncate file
			}
			else if (mode & (_Myios::trunc | _Myios::app) || _only_out)
			{
				// create new file
				try
				{
					myfile = &fs.get_root()->createFile(filepath);
				}
				catch (const filesystem_exception&)
				{
					return nullptr;
				}
			}
			else // need file to exists before opening
			{
				return nullptr;
			}

			// Just create empty buffer
			create_buffer();

			put_area_start = buffer_fend = buffer_pos = buffer_start;
			this->mode = mode;
			return this;
		}

		basic_filebuf* close()
		{
			// it's better not to call virtual methods in constructor/destructor, so here is copy of sync
			if (!myfile)
			{
				return nullptr;
			}

			bool out = true;

			if (mode & _Myios::out)
			{
				out = flush_buffer();
			}

			delete[] buffer_start;
			_init(nullptr);

			return out ? this : nullptr;
		}

	private:
		void _init(std::nullptr_t)
		{
			myfile = nullptr;
			mycvt = nullptr;
			static _State_t state_init; // initial state
			convstate = state_init;
			posstate = _pos_initial;
			mode = _Myios::openmode{};
			pbackchar = Traits::eof();

			// pointers
			buffer_start = nullptr;
			buffer_end = nullptr;
			buffer_pos = nullptr;
			buffer_fend = nullptr;

			put_area_start = nullptr;

			// Always nullptr
			_Mybase::setp(nullptr, nullptr);
			_Mybase::setg(nullptr, nullptr, nullptr);
		}

		void _init(const basic_filebuf& other)
		{
			myfile = other.myfile;
			mycvt = other.mycvt;
			convstate = other.convstate;
			posstate = other.posstate;
			mode = 0;
			pbackchar = other.pbackchar;

			// pointers
			buffer_start = other.buffer_start;
			buffer_end = other.buffer_end;
			buffer_pos = other.buffer_pos;
			buffer_fend = other.buffer_fend;

			put_area_start = other.put_area_start;

			// Always nullptr
			_Mybase::setp(nullptr, nullptr);
			_Mybase::setg(nullptr, nullptr, nullptr);
		}

		static bool handle_openmode(_Myios::openmode& mode)
		{
			if (!(mode & ~(_Myios::ate))													// empty mode
				|| (mode & (_Myios::trunc | _Myios::out)) == _Myios::trunc					// trunc can be only with out
				|| (mode & (_Myios::app | _Myios::trunc)) == (_Myios::app | _Myios::trunc)) // trunc and app are incompatible
			{
				return false;
			}

			if (mode & _Myios::app)
			{
				mode |= _Myios::out; // if app, it is out mode anyway
			}

			return true;
		}

		void _init_mycvt(const _Cvt& newcvt)
		{
			mycvt = newcvt.always_noconv() ? nullptr : std::addressof(newcvt);
		}

		bool _init_buffer_from(file_t* file, _Myios::openmode mode)
		{
			// Convert file content to CharT array
			CharT* converted = nullptr;
			size_t count = convert_from_char(this, file->getContent(), converted);

			if (!converted)
			{
				return false;
			}

			// Create buffer, copy converted
			create_buffer(count);
			memcpy(buffer_start, converted, count * sizeof(CharT));
			delete[] converted;

			// Set pointers
			buffer_fend = buffer_start + count;

			buffer_pos = mode & _Myios::ate ? buffer_fend : buffer_start;
			put_area_start = mode & _Myios::app ? buffer_fend : buffer_start;

			this->mode = mode;

			return true;
		}

	protected:
		// stream: how many chars are available to read?
		virtual std::streamsize showmanyc() override
		{
			if (!(myfile && mode & _Myios::in))
			{
				return 0;
			}

			return buffer_fend - buffer_pos;
		}

		// put an element back to get area
		virtual int_type pbackfail(int_type ch = Traits::eof()) override
		{
			if (!(myfile && mode & _Myios::in)) // cannot perform input operations
			{
				return Traits::eof();
			}

			if (buffer_pos <= buffer_start	 // cannot decrease buffer_pos
				|| posstate != _pos_initial) // smth put back or broken or at the end
			{
				posstate |= _pos_broken;
				return Traits::eof();
			}

			--buffer_pos;

			if (Traits::eq_int_type(Traits::eof(), ch))
			{
				// just decrement buffer_pos
				return Traits::not_eof(ch);
			}

			if (!Traits::eq_int_type(Traits::to_int_type(*buffer_pos), ch))
			{
				pbackchar = Traits::to_char_type(ch); // put back char
				posstate |= _pbackwas;
			}

			return ch;
		}

		// put an element to stream
		virtual int_type overflow(int_type ch = Traits::eof()) override
		{
			if (Traits::eq_int_type(Traits::eof(), ch))
			{
				return Traits::not_eof(ch); // nothing to put, return success
			}

			if (!(myfile && mode & _Myios::out) // cannot perform output operations
				|| posstate & _pos_broken)		// position broken
			{
				return Traits::eof();
			}

			CharT* p = mode & _Myios::app ? buffer_fend : buffer_pos;

			if (p >= buffer_end) // buffer is too small, extend it
			{
				extend_buffer();
				p = mode & _Myios::app ? buffer_fend : buffer_pos;
			}

			*(p++) = Traits::to_char_type(ch); // finally put element to put area

			if (mode & _Myios::app)
			{
				posstate = _pos_ate;
			}

			if (p > buffer_fend) // increase content size
			{
				buffer_fend = p;
			}

			++buffer_pos;

			return ch;
		}

		// get an element from stream, but don't point past it
		virtual int_type underflow() override
		{
			if (posstate & _pos_broken)
			{
				return Traits::eof();
			}

			if (posstate & _pos_ate)
			{
				posstate |= _pos_broken;
				return Traits::eof();
			}

			if (posstate & _pbackwas)
			{
				return pbackchar;
			}

			if (!(myfile && mode & _Myios::in) // cannot perform input operations
				|| buffer_pos >= buffer_fend)  // pending input is empty
			{
				return Traits::eof();
			}

			return Traits::to_int_type(*buffer_pos); // peek char
		}

		// get an element from stream, point past it
		virtual int_type uflow() override
		{
			if (posstate & _pos_broken)
			{
				return Traits::eof();
			}

			if (posstate & _pos_ate)
			{
				posstate |= _pos_broken;
				return Traits::eof();
			}

			if (posstate & _pbackwas)
			{
				int_type out = pbackchar;
				posstate &= ~_pbackwas;
				++buffer_pos; // buffer_pos < buffer_fend because pbackfail was called
				return out;
			}

			if (!(myfile && mode & _Myios::in) // cannot perform input operations
				|| buffer_pos >= buffer_fend)  // pending input is empty
			{
				return Traits::eof();
			}

			int_type ch = Traits::to_int_type(*(buffer_pos++)); // get char, increase pointers
			return ch;
		}

		virtual _Mybase* setbuf(char_type* buf, std::streamsize count) override
		{
			if (!buf || count == 0					 // empty buffer
				|| count < buffer_fend - buffer_end) // not capable for existing buffer contents
			{
				return this;
			}

			memcpy(buf, buffer_start, (buffer_fend - buffer_start) * sizeof(CharT));
			delete[] buffer_start;
			size_t area_size = buffer_fend - buffer_start;
			size_t pos = buffer_pos - buffer_start;
			size_t put_area_shift = put_area_start - buffer_start;

			buffer_start = buf;
			buffer_end = buf + count;
			buffer_pos = buf + pos;
			buffer_fend = buf + area_size;

			put_area_start = buf + put_area_shift;

			return this;
		}

		// change position by off in direction of dir
		virtual pos_type seekoff(off_type off, _Myios::seekdir dir,
			_Myios::openmode which = _Myios::in | _Myios::out) override
		{
			if (!myfile)
			{
				return pos_type{ off_type{-1} }; // no file, nowhere to change position
			}

			// do seek
			switch (dir)
			{
			case _Myios::beg:
				buffer_pos = buffer_start + off;
				break;
			case _Myios::cur:
				if (off == 0)
				{
					if (posstate & _pos_broken)
					{
						return pos_type{ off_type{-1} };
					}

					pos_type pos{ buffer_pos - buffer_start };
					pos.state(convstate);

					return pos; // just tell the position
				}
				buffer_pos += off;
				break;
			case _Myios::end:
				buffer_pos = buffer_fend + off;
				break;
			default:
				return pos_type{ off_type{-1} };
			}

			if (buffer_pos > buffer_fend)
			{
				buffer_pos = buffer_fend;
			}

			posstate = _pos_initial; // reset position state

			pos_type pos{ buffer_pos - buffer_start };
			pos.state(convstate);

			return pos;
		}

		virtual pos_type seekpos(pos_type pos,
			_Myios::openmode which = _Myios::in | _Myios::out) override
		{
			if (!myfile)
			{
				return pos_type{ off_type{-1} }; // no file, nowhere to change position
			}

			posstate = _pos_initial;
			buffer_pos = buffer_start + pos;

			if (buffer_pos > buffer_fend)
			{
				buffer_pos = buffer_fend;
			}

			return pos;
		}

		virtual int sync() override
		{
			if (!myfile)
			{
				return 0;
			}

			if (mode & _Myios::out)
			{
				return flush_buffer();
			}

			return 1;
		}

		virtual void imbue(const std::locale& loc) override
		{
			_init_mycvt(std::use_facet<_Cvt>(loc));
		}

	private:
		const _Cvt* mycvt;	// ptr to codecvt facet (can be nullptr)
		_State_t convstate; // current convertion state

		constexpr static unsigned char _pos_initial = 0; // set by default
		constexpr static unsigned char _pos_broken = 1;	 // set if pbackfail called twice or uflow called while _pos_ate was set
		constexpr static unsigned char _pbackwas = 2;	 // set when pbackfail called
		constexpr static unsigned char _pos_ate = 4;	 // set when overflow called while _Myios::app is set

		int_type pbackchar; // char stored by pbackfail
		unsigned char posstate;

		// controlled buffer pointers
		CharT* buffer_start;
		CharT* buffer_end;
		CharT* buffer_pos;
		CharT* buffer_fend;

		CharT* put_area_start;

		file_t* myfile;
		_Myios::openmode mode;

		static constexpr size_t buffer_chunk_size = 256;

		size_t create_buffer(size_t min_size = buffer_chunk_size)
		{
			size_t size = (min_size / buffer_chunk_size + (min_size % buffer_chunk_size != 0)) * buffer_chunk_size;

			buffer_start = new CharT[size]{};
			buffer_end = buffer_start + size;

			return size;
		}

		void extend_buffer()
		{
			CharT* ob_start = buffer_start;
			CharT* ob_end = buffer_end;
			CharT* ob_pos = buffer_pos;
			CharT* ob_fend = buffer_fend;

			CharT* ob_put_area_start = put_area_start;

			create_buffer(buffer_end - buffer_start + 1);

			memcpy(buffer_start, ob_start, ob_fend - ob_start);
			delete[] ob_start;

			buffer_pos = ob_pos - ob_start + buffer_start;
			buffer_fend = ob_fend - ob_start + buffer_start;

			put_area_start = ob_put_area_start - ob_start + buffer_start;
		}

		bool flush_buffer()
		{
			char* converted = nullptr;
			size_t size = convert_buffer_to_char(converted);

			if (!converted)
			{
				return false;
			}

			if (size == 0)
			{
				delete[] converted;
				return false;
			}

			if (mode & _Myios::app)
			{
				myfile->appendBytes(converted, size);
			}
			else
			{
				myfile->writeBytes(converted, size);
			}

			delete[] converted;
			return true;
		}

		size_t convert_buffer_to_char(char*& converted)
		{
			if (!mycvt) // just copy raw
			{
			noconv:
				const size_t size = buffer_fend - put_area_start;
				char* out = new char[size + 1] {'\0'};
				memcpy(out, put_area_start, size * sizeof(CharT));

				converted = out;
				return size;
			}

			std::string out_buf;
			const CharT* from_next = put_area_start;

			while (true)
			{
				constexpr size_t tmp_buf_size = 256;
				char tmp_buf[tmp_buf_size]{};
				char* tmp_buf_next = nullptr;

				switch (mycvt->out(
					convstate,
					from_next, buffer_fend, from_next,
					tmp_buf, tmp_buf + tmp_buf_size, tmp_buf_next
				))
				{
				case std::codecvt_base::ok:
				case std::codecvt_base::partial:
					if (from_next == buffer_fend) // converted all
					{
						const size_t size = out_buf.size();
						char* out = new char[size + 1] {'\0'};
						memcpy(out, out_buf.data(), size);

						converted = out;
						return size;
					}

					out_buf.append(tmp_buf, tmp_buf_next - tmp_buf);
					break;

				case std::codecvt_base::noconv: // no convertion needed
					goto noconv;

				default: // failed convertion
					converted = nullptr;
					return 0;
				}
			}
		}

		static size_t convert_from_char(
			basic_filebuf* buffer,
			const std::string& from,
			CharT*& converted)
		{
			if (!buffer->mycvt) // just copy raw
			{
			noconv:
				const size_t size = from.size();
				CharT* out = new CharT[size];
				memcpy(out, from.c_str(), size);

				converted = out;
				return size;
			}

			std::basic_string<CharT> out_buf;
			const char* from_next = from.c_str();
			const char* from_end = from.c_str() + from.size();

			while (true)
			{
				constexpr size_t tmp_buf_size = 256;
				CharT tmp_buf[tmp_buf_size]{};
				CharT* tmp_buf_next = nullptr;

				switch (buffer->mycvt->in(
					buffer->convstate,
					from_next, from_end, from_next,
					tmp_buf, tmp_buf + tmp_buf_size, tmp_buf_next
				))
				{
				case std::codecvt_base::ok:
				case std::codecvt_base::partial:
					if (from_next == from_end) // converted all
					{
						const size_t count = out_buf.size();
						CharT* out = new CharT[count];
						memcpy(out, out_buf.data(), count * sizeof(CharT));

						converted = out;
						return count;
					}

					// Append converted part
					out_buf.append(tmp_buf, tmp_buf_next - tmp_buf);
					break;

				case std::codecvt_base::noconv: // no convertion needed
					goto noconv;

				default: // failed convertion
					converted = nullptr;
					return 0;
				}
			}
		}
	};

	typedef basic_filebuf<char> filebuf;
	typedef basic_filebuf<wchar_t> wfilebuf;
};

namespace std
{
	template <class CharT, class Traits>
	void swap(
		virtfiles::basic_filebuf<CharT, Traits>& lhs,
		virtfiles::basic_filebuf<CharT, Traits>& rhs
	)
	{
		lhs.swap(rhs);
	}
};
