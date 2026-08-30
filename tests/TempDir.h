#pragma once

#include "lua_interface.h"

#include <atomic>
#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>

// A scratch directory that is wiped when it goes out of scope. Every test that
// touches the lua interface needs real files on disk: LuaInterface only knows
// how to load a script by path, and the ekt.* helpers walk the filesystem.
class TempDir
{
public:
    TempDir()
    {
        static std::atomic<unsigned> counter{0};
        m_path = std::filesystem::temp_directory_path() /
                 ("ekt_tests_" + std::to_string(counter++));

        std::error_code ec;
        std::filesystem::remove_all(m_path, ec);
        std::filesystem::create_directories(m_path);
    }

    ~TempDir()
    {
        std::error_code ec;
        std::filesystem::remove_all(m_path, ec);
    }

    TempDir(const TempDir&) = delete;
    TempDir& operator=(const TempDir&) = delete;

    const std::filesystem::path& path() const { return m_path; }

    std::filesystem::path write(const std::filesystem::path& relative, std::string_view content) const
    {
        auto full = m_path / relative;
        std::filesystem::create_directories(full.parent_path());

        std::ofstream out(full, std::ios::binary);
        out << content;

        return full;
    }

    // The script name matters: ekt only picks up files ending in ekt.lua
    std::filesystem::path write_script(std::string_view content) const
    {
        return write("test." + LuaInterface::script_ext, content);
    }

private:
    std::filesystem::path m_path;
};
