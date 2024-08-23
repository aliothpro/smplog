//
// Created by sxc on 2019/1/7.
//
#ifdef _WIN32
#include <io.h>
#else
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#include <string>
#include <io/io_director.h>
#include <iostream>

using smp::io::file_info;
using smp::io::file_attribute;
/**
 * 遍历当前目录下的文件夹和文件,默认是按字母顺序遍历
 * **/
#ifdef _WIN32
std::vector<smp::io::file_info> traverse_files(const std::string& path, const std::string& suffix)
{
    std::vector<smp::io::file_info> fileInfoArray;
    _finddata_t file_info;
    std::string current_path = path+"/*.";
    current_path.append(suffix);
    /*
    ////可以定义后面的后缀为*.exe，*.txt等来查找特定后缀的文件，*.*是通配符，匹配所有类型,路径连接符最好是左斜杠/，可跨平台
    ////打开文件查找句柄
    */
    intptr_t handle=_findfirst(current_path.c_str(),&file_info);
    /*
     * 返回值为-1则查找失败
     * */
    if(-1==handle)
        return fileInfoArray;
    do
    {
        /*
         * 判断是否子目录
         * */
        std::string attribute;
        if(file_info.attrib==(int)file_attribute::A_SUBDIR) /*    ////是目录 ////   */
            attribute="dir";
        else
            attribute="file";
        /*
         * 输出文件信息并计数,文件名(带后缀)、文件最后修改时间、文件字节数(文件夹显示0)、文件是否目录
         * */

        if(strcmp(file_info.name,".") != 0 && strcmp(file_info.name,"..") != 0 && file_info.attrib != (int)file_attribute::A_SUBDIR){
            file_info fileInfo;
            fileInfo.set_attribute(file_info.attrib);
            fileInfo.set_create_time(file_info.time_create);
            fileInfo.set_access_time(file_info.time_access);
            fileInfo.set_write_time(file_info.time_write);
            fileInfo.set_file_size(file_info.size);
            fileInfo.set_name(path + "//" + file_info.name);
            fileInfoArray.push_back(fileInfo);
        }

        /*
         * 获得的最后修改时间是time_t格式的长整型，需要用其他方法转成正常时间显示
         * */
        //file_num++;

    }while(!_findnext(handle,&file_info));  /*    返回0则遍历完    */
    /*
     * 关闭文件句柄
     * */
    _findclose(handle);
    return fileInfoArray;
}
#else
std::vector<smp::io::file_info> traverse_files(const std::string& path, const std::string& suffix)
{
    DIR* pDir = nullptr;
    struct dirent* ent = nullptr;
    std::vector<smp::io::file_info> fileInfoArray;
    std::string pathtmp = path+"/*.";
    pathtmp.append(suffix);
    pDir = opendir(path.c_str());
    //pDir = opendir(".");
    if (!pDir) {
        printf("opendir error.errno = [%d]\n",errno);
        if (errno == ENOENT){
            printf("No such file or directory.\n");
        }
        return fileInfoArray;
    }
    ent = readdir(pDir);
    while (ent != nullptr) {
        if(strcmp(ent->d_name,".") != 0
        && strcmp(ent->d_name,"..") != 0
        && ent->d_type != DT_DIR){
            file_info fileInfo;
            fileInfo.set_attribute(ent->d_type);
            std::string filename(ent->d_name);
            size_t pos = filename.find_last_of('.');
            std::string extname = filename.substr(pos+1);
            if (!extname.compare(suffix)){
                struct stat sb;
                stat((path + "//" + ent->d_name).c_str(),&sb);
                #if !defined(_POSIX_C_SOURCE) || defined(_DARWIN_C_SOURCE)
                fileInfo.set_create_time(sb.st_ctimespec.tv_sec);
                fileInfo.set_access_time(sb.st_atimespec.tv_sec);
                fileInfo.set_write_time(sb.st_mtimespec.tv_sec);                
                #else
                fileInfo.set_create_time(sb.st_ctim.tv_sec);
                fileInfo.set_access_time(sb.st_atim.tv_sec);
                fileInfo.set_write_time(sb.st_mtim.tv_sec);
                #endif
                fileInfo.set_file_size(sb.st_size);
		fileInfo.set_name(path + "//" + ent->d_name);
                fileInfoArray.push_back(fileInfo);
            }
        }
        ent = readdir(pDir);
    }
    closedir(pDir);
    pDir = nullptr;
    ent = nullptr;

    return fileInfoArray;
}
#endif

file_info::file_info()
: 
attrib((uint32_t)file_attribute::A_NORMAL)
, time_create(0)
, time_access(0)
, time_write(0)
, file_size(0)
, name("")  
{
}

file_info::file_info(uint32_t attrib, uint32_t time_create, uint32_t time_access, uint32_t time_write,
                       uint64_t file_size, const std::string &name) 
: attrib(attrib)
, time_create(time_create)
, time_access(time_access)
, time_write(time_write)
, file_size(file_size)
, name(name) 
{}

uint32_t file_info::get_attribute() const {
    return attrib;
}

void file_info::set_attribute(uint32_t attrib) {
    file_info::attrib = attrib;
}

uint32_t file_info::get_create_time() const {
    return time_create;
}

void file_info::set_create_time(uint32_t time_create) {
    file_info::time_create = time_create;
}

uint32_t file_info::get_access_time() const {
    return time_access;
}

void file_info::set_access_time(uint32_t time_access) {
    file_info::time_access = time_access;
}

uint32_t file_info::get_write_time() const {
    return time_write;
}

void file_info::set_write_time(uint32_t time_write) {
    file_info::time_write = time_write;
}

uint64_t file_info::get_file_size() const {
    return file_size;
}

void file_info::set_file_size(uint64_t file_size) {
    file_info::file_size = file_size;
}

const std::string &file_info::get_name() const {
    return name;
}

void file_info::set_name(const std::string &name) {
    file_info::name = name;
}


