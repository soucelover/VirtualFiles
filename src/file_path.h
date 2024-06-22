#include <stdexcept>
#include <string>
#include <vector>

namespace virtfiles
{
	class path_t
	{
	protected:
		std::string path_string;

	public:
		class parts_t
		{
		protected:
			const char* const* items;
			size_t count;
			const char* items_str;
			size_t str_size;

		public:
			class iterator;

			constexpr size_t get_count() const
			{
				return count;
			}

			constexpr const char* str() const
			{
				return items_str;
			}

			parts_t()
				: items(nullptr), count(0), items_str(nullptr), str_size(0)
			{
			}

			parts_t(const parts_t& other)
			{
				init(other);
			}

			parts_t(parts_t&& other) noexcept
				: items(other.items), count(other.count),
				items_str(other.items_str), str_size(other.str_size)
			{
				other.items = nullptr;
				other.items_str = nullptr;
			}

			parts_t& operator=(const parts_t& other)
			{
				init(other);

				return *this;
			}

			parts_t& operator=(parts_t&& other) noexcept
			{
				items = other.items;
				count = other.count;
				items_str = other.items_str;
				str_size = other.str_size;

				other.items = nullptr;
				other.items_str = nullptr;

				return *this;
			}

			void init(const std::string& path)
			{
				// Copy path string
				str_size = path.length();
				char* str = new char[str_size + 1] {};
				items_str = str;

				memcpy(str, path.data(), str_size);

				// First item starts at first string character
				std::vector<const char*> out = { str };

				// Iterate over path
				for (char* end_str = str + str_size; str < end_str; ++str)
				{
					// If it's part separator
					// then append new part
					switch (*str)
					{
					case '/':
					case '\\':
						// Finish previous part by NULL character
						*str = NULL;
						out.push_back(str + 1);
					}
				}

				// Copy vector to items
				count = out.size();
				const char** items;
				this->items = items = new const char* [count];

				memcpy(items, out.data(), count * sizeof(const char*));
			}

			void init(const parts_t& other)
			{
				if (items_str)
				{
					delete[] items_str;
					delete[] items;
				}

				count = other.count;
				str_size = other.str_size;

				const char** items;
				this->items = items = new const char* [count];

				char* items_str;
				this->items_str = items_str = new char[str_size];

				memcpy(items_str, other.items_str, str_size);

				for (size_t i = 0; i < count; ++i)
				{
					items[i] = items_str + (other.items_str - other.items[i]);
				}
			}

			constexpr const char* const& operator[] (long long index) const
			{
				return items[index];
			}

			iterator begin() const;

			iterator end() const;
		};

		parts_t parts;

		path_t() = default;

		path_t(const std::string& path)
		{
			path_string = path;
			parts.init(path);
		}

		path_t& operator=(const std::string& path)
		{
			path_string = path;
			parts.init(path);

			return *this;
		}

		constexpr const std::string& str() const
		{
			return path_string;
		}
	};

	class path_t::parts_t::iterator : public std::iterator<
		std::random_access_iterator_tag,
		const char*,
		long long,
		const char* const*,
		const char* const&>
	{
		friend iterator operator+(long long, const iterator&);
	protected:
		const path_t::parts_t* parts;

		long long index;

	public:
		explicit iterator(const path_t::parts_t& parts)
			: parts(std::addressof(parts)), index(0)
		{
		}

		iterator(const path_t::parts_t* parts, size_t index)
			: parts(parts), index(index)
		{
		}

		iterator(const iterator& other)
			: parts(other.parts), index(other.index)
		{
		}

		iterator(iterator&& other) noexcept
			: parts(other.parts), index(other.index)
		{
		}

		iterator& operator=(const iterator& other)
		{
			parts = other.parts;
			index = other.index;

			return *this;
		}

		iterator& operator=(iterator&& other) noexcept
		{
			parts = other.parts;
			index = other.index;

			return *this;
		}

		~iterator()
		{
		}

		void swap(const iterator& other)
		{
			const parts_t* parent_sav = parts;
			size_t index_sav = index;

			*this = other;

			const_cast<const parts_t*&>(other.parts) = parent_sav;
			const_cast<long long&>(other.index) = index_sav;
		}

		constexpr iterator& operator++()
		{
			++index;
			return *this;
		}

		iterator operator++(int)
		{
			iterator sav(*this);
			++index;
			return sav;
		}

		constexpr iterator& operator--()
		{
			--index;
			return *this;
		}

		iterator operator--(int)
		{
			iterator sav(*this);
			--index;
			return sav;
		}

		constexpr reference operator*() const
		{
			return (*parts)[index];
		}

		constexpr bool operator==(const iterator& other) const
		{
			return parts == other.parts && index == other.index;
		}

		constexpr bool operator!=(const iterator& other) const
		{
			return !(*this == other);
		}

		constexpr iterator& operator+=(long long diff)
		{
			index += diff;
			return *this;
		}

		iterator operator+(long long diff)
		{
			return iterator(parts, index + diff);
		}

		constexpr iterator& operator-=(long long diff)
		{
			index -= diff;
			return *this;
		}

		iterator operator-(long long diff)
		{
			return iterator(parts, index - diff);
		}

		long long operator-(const iterator& other)
		{
			return index - other.index;
		}

		constexpr reference operator[](long long idx)
		{
			return (*parts)[index + idx];
		}

		constexpr bool operator<(const iterator& other)
		{
			return index < other.index;
		}

		constexpr bool operator>(const iterator& other)
		{
			return index > other.index;
		}

		constexpr bool operator<=(const iterator& other)
		{
			return index <= other.index;
		}

		constexpr bool operator>=(const iterator& other)
		{
			return index >= other.index;
		}

		constexpr const path_t::parts_t& get_path() const
		{
			return *parts;
		}

		constexpr long long get_index() const
		{
			return index;
		}
	};

	path_t::parts_t::iterator path_t::parts_t::begin() const
	{
		return iterator(*this);
	}

	path_t::parts_t::iterator path_t::parts_t::end() const
	{
		return iterator(this, get_count());
	}

	extern path_t::parts_t::iterator operator+(long long diff, const path_t::parts_t::iterator& iter)
	{
		return virtfiles::path_t::parts_t::iterator(iter.parts, iter.index + diff);
	}
};

namespace std
{
	void swap(virtfiles::path_t::parts_t::iterator left, virtfiles::path_t::parts_t::iterator right)
	{
		left.swap(right);
	}
};
