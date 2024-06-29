#include "file_entries.h"
#include "file_path.h"
#include "virt_fstream.h"
#include <locale>

namespace virtfiles {
    void filesystem::init()
    {
        setlocale(LC_ALL, "en_US.utf8");

        // Place here f/s setup code
    }

    void filesystem::before_uninit()
    {
        // Place here code to check file system
        // state before the program destroys it
    }
}
