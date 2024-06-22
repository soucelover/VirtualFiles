#include <cwctype>
#include <string>

namespace virtfiles
{
	class file_t;
	class folder_t;

	class base_entry
	{
	protected:
		folder_t* parent;
		size_t name_size;
		const char* name;

	public:
		constexpr const char* getName() const
		{
			return name;
		}

		constexpr folder_t* getParent() const
		{
			return parent;
		}

		base_entry(const char* entry_name,
			folder_t* parent = nullptr)
			: parent(parent)
		{
			name_size = strlen(entry_name);

			char* name;
			this->name = name = new char[name_size + 1] {};
			memcpy(name, entry_name, name_size);
		}

		base_entry(const std::string& entry_name,
			folder_t* parent = nullptr)
			: parent(parent)
		{
			name_size = entry_name.length();

			char* name;
			this->name = name = new char[name_size + 1] {};
			memcpy(name, entry_name.c_str(), name_size);
		}

		base_entry(const base_entry&) = delete;
		base_entry(base_entry&&) = delete;

		virtual ~base_entry()
		{
			delete[] name;
		}

		base_entry& operator=(const base_entry&) = delete;
		base_entry& operator=(base_entry&&) = delete;

		virtual bool is_file() const
		{
			return false;
		}

		virtual bool is_folder() const
		{
			return false;
		}

		virtual folder_t* as_folder()
		{
			return nullptr;
		}

		virtual file_t* as_file()
		{
			return nullptr;
		}

		static bool check_name(const char* name)
		{
			size_t len = strlen(name);
			std::mbstate_t state{};

			wchar_t ch;
			const char* i = name;
			const char* end = name + len;

			while (true)
			{
				size_t count = std::mbrtowc(&ch, i, end - name, &state);

				switch (count)
				{
				case 0:
					return true;
				case static_cast<size_t>(-1):
				case static_cast<size_t>(-2):
					return false;
				}

				i += count;

				if (ch >= 0x00 && ch <= 0x1f) // special characters
				{
					return false;
				}
				switch (ch)
				{
				case L'<':
				case L'>':
				case L':':
				case L'"':
				case L'/':
				case L'\\':
				case L'|':
				case L'?':
				case L'*':
					return false;
				}
			}
		}

		bool is_named(const std::string& name) const
		{
			std::mbstate_t state{};

			wchar_t lch, rch;
			const char* li = this->name;
			const char* ri = name.c_str();
			const char* lend = li + name_size;
			const char* rend = ri + name.length();

			while (true)
			{
				if (!(_is_named_next_char(lch, li, lend, &state) && _is_named_next_char(rch, ri, rend, &state)))
				{
					if (lch == L'?') // it's last char and it's incomplete
					{
						if (lend - li != rend - ri)
						{
							return false;
						}
						if (memcmp(li, ri, lend - li) != 0)
						{
							return false;
						}
						return true;
					}
					else // some encoding error or null character
					{
						if (li == lend && ri == rend)
						{
							return true;
						}

						return false;
					}
				}

				if (std::towlower(static_cast<wint_t>(lch)) != std::towlower(static_cast<wint_t>(rch)))
				{
					return false;
				}
			}
		}

	private:
		static bool _is_named_next_char(wchar_t& ch,
			const char*& src, const char* src_end,
			std::mbstate_t* state)
		{
			wchar_t _ch;
			size_t count = std::mbrtowc(&_ch, src, src_end - src, state);

			if (count == 0 || count == static_cast<size_t>(-1))
			{
				ch = L'\0';
				return false;
			}
			else if (count == static_cast<size_t>(-2))
			{
				ch = L'?';
				return false;
			}

			ch = _ch;
			src += count;
			return true;
		}
	};
};
