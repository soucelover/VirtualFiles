#include "file_entries.h"
#include "file_path.h"
#include "virt_fstream.h"
#include <locale>

void virtfiles::filesystem::_Init()
{
	setlocale(LC_ALL, "en_US.utf8");
}
