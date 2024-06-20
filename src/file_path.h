#include <string>
#include <stdexcept>

namespace virtfiles
{
	class path_t
	{
	protected:
		std::string path_string;
		size_t parts_count;
		const char* const* parts = nullptr;

	public:
		size_t get_parts_count()
		{
			return parts_count;
		}

		class iterator;

		path_t()
			: path_string(""), parts_count(1),
			parts(new char* [1]) {}

		path_t(const char* path)
		{
			init(static_cast<std::string>(path));
		}

		path_t(const std::string& path)
		{
			init(path);
		}

		path_t(const path_t& other)
		{
			init(other);
		}

		path_t(path_t&& other) noexcept
		{
			path_string = other.path_string;
			parts_count = other.parts_count;
			parts = other.parts;
			other.parts = nullptr;
			other.path_string = "";
			other.parts_count = 0;
		}

		~path_t()
		{
			delete[] parts[0];
			delete[] parts;
		}

		path_t& operator=(_In_z_ const char* path)
		{
			init(static_cast<std::string>(path));

			return *this;
		}

		path_t& operator=(_In_z_ const std::string& path)
		{
			init(path);

			return *this;
		}

		path_t& operator=(_In_ const path_t& other)
		{
			init(other);

			return *this;
		}

		path_t& operator=(_In_ path_t&& other) noexcept
		{
			if (parts)
			{
				delete[] parts[0];
				delete[] parts;
			}

			path_string = other.path_string;
			parts_count = other.parts_count;
			parts = other.parts;

			other.parts = nullptr;
			other.path_string = "";
			other.parts_count = 0;

			return *this;
		}

		void init(const std::string& path)
		{
			path_string = path;

			if (parts)
			{
				delete[] parts[0];
				delete[] parts;
			}

			parts_count = get_parts_count(path);
			const char** parts = new const char* [parts_count];

			parse_parts(parts, path, parts_count);
			this->parts = parts;
		}

		void init(const path_t& other)
		{
			if (parts)
			{
				delete[] parts[0];
				delete[] parts;
			}

			path_string = other.path_string;
			parts_count = other.parts_count;

			char** parts = new char* [parts_count];
			size_t size = other.parts[parts_count - 1] - other.parts[0] + strlen(other.parts[parts_count - 1]);

			parts[0] = new char[size];
			memcpy(parts[0], other.parts[0], size);

			for (size_t i = 1; i < parts_count; ++i)
			{
				parts[i] = parts[i - 1] + (other.parts[i] - other.parts[i - 1]);
			}

			this->parts = parts;
		}

		static size_t get_parts_count(const std::string& path)
		{
			size_t out = 1;

			for (char c : path)
			{
				if (c == '\\' || c == '/')
				{
					out++;
				}
			}

			return out;
		}

		static void parse_parts(_Out_writes_(count) const char** out,
			_In_ const std::string& path, _In_ size_t count)
		{
			// copy path string
			char* out_str = new char[path.length() + 1];
			memcpy(out_str, path.data(), path.length());
			out_str[path.length()] = '\0';

			// initialize pointers
			char** ptr = const_cast<char**>(out);
			char** last = ptr + count - 1;

			// first element point to start of out_str
			*ptr = out_str;

			if (count <= 1)
			{
				return;
			}

			*(ptr + 1) = out_str; // second element point to it too

			while (true)
			{
				// handle current char
				switch (**(ptr + 1))
				{
				case '/':
				case '\\':
					**(ptr + 1) = '\0'; // replace separator by null char
					++ptr;				// next ptr
					++*ptr;				// set ptr to next past null char

					if (ptr < last)
					{
						*(ptr + 1) = *ptr; // set next pointer
					}
					else
					{
						return;
					}
				}
				++ * (ptr + 1); // next symbol
			}
		}

		iterator begin() const;

		iterator end() const;

		const char* operator[](long long index)
		{
			if (0 > index || index >= parts_count)
			{
				throw std::out_of_range("fs_path::operator[] index arg is out of range.");
			}

			return parts[index];
		}

		const std::string& str() const
		{
			return path_string;
		}

		size_t get_parts_count() const
		{
			return parts_count;
		}
	};

	class path_t::iterator : public std::iterator<
		std::random_access_iterator_tag,
		const char*,
		long long,
		const char* const*,
		const char* const&>
	{
		friend path_t;
		friend iterator operator+(long long, const iterator&);

	public:
		explicit iterator(const path_t& path)
			: path(std::addressof(path)), index(0)
		{
		}

		iterator(const path_t* path, size_t index)
			: path(path), index(index)
		{
		}

		iterator(const iterator& other)
			: path(other.path), index(other.index)
		{
		}

		iterator(iterator&& other) noexcept
			: path(other.path), index(other.index)
		{
		}

		iterator& operator=(const iterator& other)
		{
			path = other.path;
			index = other.index;

			return *this;
		}

		iterator& operator=(iterator&& other) noexcept
		{
			path = other.path;
			index = other.index;

			return *this;
		}

		~iterator()
		{
		}

		void swap(const iterator& other)
		{
			const path_t* parent_sav = path;
			size_t index_sav = index;

			*this = other;

			const_cast<const path_t*&>(other.path) = parent_sav;
			const_cast<long long&>(other.index) = index_sav;
		}

		iterator& operator++()
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

		iterator& operator--()
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

		reference operator*() const
		{
			return path->parts[index];
		}

		bool operator==(const iterator& other) const
		{
			return path == other.path && index == other.index;
		}

		bool operator!=(const iterator& other) const
		{
			return !(*this == other);
		}

		iterator& operator+=(long long diff)
		{
			index += diff;
			return *this;
		}

		iterator operator+(long long diff)
		{
			return iterator(path, index + diff);
		}

		iterator& operator-=(long long diff)
		{
			index -= diff;
			return *this;
		}

		iterator operator-(long long diff)
		{
			return iterator(path, index - diff);
		}

		long long operator-(const iterator& other)
		{
			return index - other.index;
		}

		reference operator[](long long idx)
		{
			return path->parts[index + idx];
		}

		bool operator<(const iterator& other)
		{
			return index < other.index;
		}

		bool operator>(const iterator& other)
		{
			return index > other.index;
		}

		bool operator<=(const iterator& other)
		{
			return index <= other.index;
		}

		bool operator>=(const iterator& other)
		{
			return index >= other.index;
		}

		const path_t& get_path() const
		{
			return *path;
		}

		long long get_index() const
		{
			return index;
		}

	protected:
		const path_t* path;

		long long index;
	};

	path_t::iterator path_t::begin() const
	{
		return path_t::iterator(*this);
	}

	path_t::iterator path_t::end() const
	{
		return path_t::iterator(this, parts_count);
	}

	extern path_t::iterator operator+(long long diff, const path_t::iterator& iter)
	{
		return virtfiles::path_t::iterator(iter.path, iter.index + diff);
	}
};

namespace std
{
	void swap(virtfiles::path_t::iterator left, virtfiles::path_t::iterator right)
	{
		left.swap(right);
	}
};
