#include <OpenCLProgram.h>

#ifdef _WIN32
    #include <direct.h>
    #define get_current_dir _getcwd
#else
    #include <unistd.h>
    #define get_current_dir getcwd
#endif

std::string OpenCLProgram::debug_option(const std::string& path_source)
{
    #ifdef INTELSDK
        char current_path[FILENAME_MAX]; 
        if (!get_current_dir(current_path, sizeof(current_path))) return "";
        //return
        return "-g -s \"" + std::string(current_path) + "/" + path_source + "\" ";
    #else
        return "";
    #endif
}

std::string OpenCLProgram::include_option(const std::string& path_dir)
{
    char current_path[FILENAME_MAX];
    if (!get_current_dir(current_path, sizeof(current_path))) return "";
    //return
#if 0 && _WIN32
	std::string tmp = std::string(current_path) + "\\" + path_dir;
	StringUtils::replace_all(tmp, " ", "\\ ");
	return " -I \"" + tmp + "\"";
#else
    std::string tmp=std::string(current_path)+"/"+path_dir;
    StringUtils::replace_all(tmp, " ", "\\ ");
    return " -I"+tmp;
#endif
}
