//
// Created by sxc on 2013/1/7.
//

#ifndef SMPUTIL_XXIODIRECTOR_H
#define SMPUTIL_XXIODIRECTOR_H

#include <stdint.h>
#include <string>
#include <vector>
#include <common/global.h>

/*
 *
 struct _finddata32_t
{
    unsigned    attrib;
    __time32_t  time_create;    // -1 for FAT file systems
    __time32_t  time_access;    // -1 for FAT file systems
    __time32_t  time_write;
    _fsize_t    size;
    char        name[260];
};
 */

namespace smp{namespace io{
    
        typedef enum class st_FileAttribute{
            // File attribute constants for the _findfirst() family of functions
            A_NORMAL = 0x00, // Normal file - No read/write restrictions
            A_RDONLY = 0x01, // Read only file
            A_HIDDEN = 0x02, // Hidden file
            A_SYSTEM = 0x04, // System file
            A_SUBDIR = 0x10, // Subdirectory
            A_ARCH   = 0x20, // Archive file
        }file_attribute;

class file_info{
private:
    uint32_t  attrib;
    uint32_t  time_create;
    uint32_t time_access;
    uint32_t time_write;
    uint64_t file_size;
    std::string name;

public:
    file_info();

    file_info(uint32_t attrib, uint32_t time_create, uint32_t time_access, uint32_t time_write, uint64_t file_size,
               const std::string &name);

    uint32_t get_attribute() const;

    void set_attribute(uint32_t attrib);

    uint32_t get_create_time() const;

    void set_create_time(uint32_t time_create);

    uint32_t get_access_time() const;

    void set_access_time(uint32_t time_access);

    uint32_t get_write_time() const;

    void set_write_time(uint32_t time_write);

    uint64_t get_file_size() const;

    void set_file_size(uint64_t file_size);

    const std::string &get_name() const;

    void set_name(const std::string &name);
};


}}

std::vector<smp::io::file_info> traverse_files(const std::string& path, const std::string& suffix="log");
#endif //SMPUTIL_XXIODIRECTOR_H
