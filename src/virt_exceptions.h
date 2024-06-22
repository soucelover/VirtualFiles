#pragma once

#include <system_error>

namespace virtfiles
{
	class filesystem_exception
		: public std::system_error
	{
		using std::system_error::system_error;
	};

	class file_exists_error
		: public filesystem_exception
	{
	public:
		file_exists_error()
			: filesystem_exception(EEXIST, std::generic_category())
		{
		}

		file_exists_error(const char* what_arg)
			: filesystem_exception(EEXIST, std::generic_category(), what_arg)
		{
		}

		file_exists_error(const std::string& what_arg)
			: filesystem_exception(EEXIST, std::generic_category(), what_arg)
		{
		}
	};

	class file_not_found_error
		: public filesystem_exception
	{
	public:
		file_not_found_error()
			: filesystem_exception(ENOENT, std::generic_category())
		{
		}

		file_not_found_error(const char* what_arg)
			: filesystem_exception(ENOENT, std::generic_category(), what_arg)
		{
		}

		file_not_found_error(const std::string& what_arg)
			: filesystem_exception(ENOENT, std::generic_category(), what_arg)
		{
		}
	};

	class not_a_directory_error
		: public filesystem_exception
	{
	public:
		not_a_directory_error()
			: filesystem_exception(ENOTDIR, std::generic_category())
		{
		}

		not_a_directory_error(const char* what_arg)
			: filesystem_exception(ENOTDIR, std::generic_category(), what_arg)
		{
		}

		not_a_directory_error(const std::string& what_arg)
			: filesystem_exception(ENOTDIR, std::generic_category(), what_arg)
		{
		}
	};

	class permission_error
		: public filesystem_exception
	{
	public:
		permission_error()
			: filesystem_exception(EPERM, std::generic_category())
		{
		}

		permission_error(const char* what_arg)
			: filesystem_exception(EPERM, std::generic_category(), what_arg)
		{
		}

		permission_error(const std::string& what_arg)
			: filesystem_exception(EPERM, std::generic_category(), what_arg)
		{
		}
	};

	class invalid_path_error
		: public filesystem_exception
	{
	public:
		invalid_path_error()
			: filesystem_exception(EINVAL, std::generic_category())
		{
		}

		invalid_path_error(const char* what_arg)
			: filesystem_exception(EINVAL, std::generic_category(), what_arg)
		{
		}

		invalid_path_error(const std::string& what_arg)
			: filesystem_exception(EINVAL, std::generic_category(), what_arg)
		{
		}
	};
};
