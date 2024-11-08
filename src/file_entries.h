#pragma once

#include "file_path.h"
#include "virt_exceptions.h"
#include <cwctype>
#include <string>
#include <vector>


namespace virtfiles
{
	class file_t;
	class folder_t;

	class base_entry
	{
	protected:
		folder_t* parent;
		const char* name;

	public:
		const char* get_name() const
		{
			return name;
		}

		folder_t* get_parent() const
		{
			return parent;
		}

		base_entry(const char* entry_name,
			folder_t* parent = nullptr)
			: parent(parent)
		{
			size_t name_size = strlen(entry_name);

			if (!check_name(entry_name, name_size))
			{
				throw invalid_path_error();
			}

			this->name = new char[name_size + 1] {};
			memcpy(const_cast<char*>(name), entry_name, name_size);
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

		virtual folder_t& as_folder()
		{
			throw not_a_directory_error();
		}

		virtual file_t& as_file()
		{
			throw permission_error();
		}

		static bool check_name(const char* name, size_t size)
		{
			std::mbstate_t state{};

			wchar_t ch;
			const char* i = name;

			while (size > 0)
			{
				// Get next char
				size_t count = std::mbrtowc(&ch, i, size, &state);

				switch (count)
				{
				case static_cast<size_t>(-1):
				case static_cast<size_t>(-2):
					return false;
				}

				i += count;
				size -= count;

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

			return true;
		}

		bool is_named(const std::string& name) const
		{
			std::mbstate_t state{};

			wchar_t lch, rch;
			const char* li = this->name;
			const char* ri = name.c_str();
			size_t lleft = strlen(this->name);
			size_t rleft = name.length();

			while (lleft > 0 && rleft > 0)
			{
				_is_named_next_char(lch, li, lleft, &state);
				_is_named_next_char(rch, ri, rleft, &state);

				// Null characters cannot be just in strings
				// So this can only consider to be an error
				if (lch == 0 || rch == 0)
				{
					return false;
				}

				// Case independently comparing characters
				if (std::towlower(static_cast<wint_t>(lch)) != std::towlower(static_cast<wint_t>(rch)))
				{
					return false;
				}
			}

			return true;
		}

	private:
		static void _is_named_next_char(wchar_t& ch,
			const char*& src, size_t& src_left,
			std::mbstate_t* state)
		{
			// Convert next character
			size_t count = std::mbrtowc(&ch, src, src_left, state);

			// If error, set null char
			switch (count)
			{
			case static_cast<size_t>(-1):
			case static_cast<size_t>(-2):
				ch = 0;
				return;
			}

			src += count;
			src_left -= count;
		}
	};

	class file_t : public base_entry
	{
	protected:
		const char* content;
		size_t file_size;

	public:
		std::string getContent() const
		{
			return std::string(content, file_size);
		}

		file_t(const char* name,
			folder_t* parent = nullptr)
			: base_entry(name, parent),
			content(new char[1] {0}), file_size(0)
		{
		}

		virtual ~file_t()
		{
			delete[] content;
		}

		bool is_file() const override
		{
			return true;
		}

		file_t& as_file() override
		{
			return *this;
		}

		void empty()
		{
			delete[] content;
			content = new char[1] {0};
			file_size = 0;
		}

		void writeBytes(const char* bytes, size_t count)
		{
			char* buf = new char[count + 1] {};
			memcpy(buf, bytes, count);

			delete[] content;
			content = buf;
			file_size = count;
		}

		void writeBytes(const char* bytes)
		{
			writeBytes(bytes, strlen(bytes));
		}

		void writeBytes(const std::string& bytes)
		{
			writeBytes(bytes.c_str(), bytes.size());
		}

		void appendBytes(const char* bytes, size_t count)
		{
			char* buf = new char[file_size + count + 1] {};
			memcpy(buf, content, file_size);
			memcpy(buf + file_size, bytes, count);

			delete[] content;
			content = buf;
			file_size += count;
		}

		void appendBytes(const char* bytes)
		{
			appendBytes(bytes, strlen(bytes));
		}

		void appendBytes(const std::string& bytes)
		{
			appendBytes(bytes.c_str(), bytes.size());
		}
	};

	class folder_t : public base_entry
	{
	protected:
		std::vector<base_entry*> entries;

	public:
		const std::vector<base_entry*>& get_items()
		{
			return entries;
		}

		folder_t(const char* name, folder_t* parent = nullptr)
			: base_entry(name, parent)
		{
		}

		virtual ~folder_t()
		{
			for (base_entry* entry : entries)
			{
				delete entry;
			}
		}

		bool is_folder() const override
		{
			return true;
		}

		folder_t& as_folder() override
		{
			return *this;
		}

		base_entry* get_entry(const std::string& name)
		{
			if (name == "" || name == ".")
			{
				return this;
			}

			if (name == "..")
			{
				if (!this->parent)
				{
					throw file_not_found_error();
				}

				return this->parent;
			}

			for (base_entry* entry : entries)
			{
				if (entry->is_named(name))
				{
					return entry;
				}
			}

			throw file_not_found_error();
		}

		bool name_is_free(const std::string& name)
		{
			if (name.length() <= 2) // likea super mega effective check on "", "." and ".."
			{
				bool all_dot = true;
				const char* i = name.c_str();

				for (const char* end = i + name.length(); i < end; ++i)
				{
					if (*i != L'.')
					{
						all_dot = false;
					}
				}

				if (all_dot)
				{
					return false;
				}
			}

			for (base_entry* entry : entries)
			{
				if (entry->is_named(name))
				{
					return false;
				}
			}

			return true;
		}

		base_entry& lookup(const path_t& path)
		{
			base_entry* out = this;

			for (std::string part : path.parts)
			{
				out = out->as_folder().get_entry(part);
			}

			return *out;
		}

		folder_t* _Approach(const path_t& path,
			const char*& out_name, bool create_parents = false)
		{
			folder_t* dir = this;
			auto i = path.parts.begin();
			auto back = path.parts.end() - 1;
			out_name = *back;

			if (create_parents)
			{
				for (; i != back; ++i)
				{
					folder_t* prev_dir = dir;

					try {
						dir = &dir->get_entry(*i)->as_folder();
					}
					catch (const file_not_found_error&)
					{
						dir = &prev_dir->_createFolder(*i);
					}
				}
			}
			else
			{
				for (; i != back; ++i)
				{
					dir = &dir->get_entry(*i)->as_folder();
				}
			}

			return dir;
		}

		file_t& createFile(const path_t& path, bool parents = false)
		{
			const char* name;
			folder_t* dir = _Approach(path, name, parents);

			return dir->_createFile(name);
		}

		file_t& _createFile(const char* name)
		{
			if (!name_is_free(name))
			{
				throw file_exists_error();
			}

			file_t* file = new file_t(name, this);

			entries.push_back(file);
			return *file;
		}

		folder_t& createFolder(const path_t& path, bool parents = false)
		{
			const char* name;
			folder_t* dir = _Approach(path, name, parents);

			return dir->_createFolder(name);
		}

		folder_t& _createFolder(const char* name)
		{
			if (!name_is_free(name))
			{
				throw file_exists_error();
			}

			folder_t* folder = new folder_t(name, this);

			entries.push_back(folder);
			return *folder;
		}
	};

	class filesystem
	{
	protected:
		folder_t* root;
	public:
		folder_t* get_root() const
		{
			return root;
		}

		filesystem()
			: filesystem(new folder_t("."))
		{
		}

		filesystem(folder_t* root)
			: root(root)
		{
			init();
		}

		~filesystem()
		{
			before_uninit();

			delete root;
		}

		filesystem(const filesystem&) = delete;
		filesystem(filesystem&&) = delete;

		filesystem& operator=(const filesystem&) = delete;
		filesystem& operator=(filesystem&&) = delete;

		void init();
		void before_uninit();
	};

	static filesystem fs{};
};
