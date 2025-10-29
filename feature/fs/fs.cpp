extern "C" {
#define LUA_LIB
#include "fs/fs.h"
}
#include <vector>
#include <filesystem>
#include <fstream>
#include "lua.hpp"
#include "lua/binding.hpp"

using std::string_view_literals::operator""sv;
using lua::operator""_stack_index;

#define TRY \
    try {

#define CATCH_RETURN(x) \
    } catch (std::exception const& e) { \
        ctx.push((x)); \
        ctx.push(std::string_view(e.what())); \
        ctx.push(-1); \
        return 3; \
    }

#define RETURN_BOOLEAN(x) \
    if ((x)) { \
        ctx.push(true); \
        return 1; \
    } \
    else { \
        ctx.push(false); \
        ctx.push(ec.message()); \
        ctx.push(ec.value()); \
        return 3; \
    }

#define RETURN_ERROR(ERROR_VALUE, ERROR_MESSAGE, ERROR_CODE) { \
        ctx.push((ERROR_VALUE)); \
        ctx.push((ERROR_MESSAGE)); \
        ctx.push((ERROR_CODE)); \
        return 3; \
    }

namespace {
    std::u8string_view utf8(std::string_view const s) noexcept {
        return { reinterpret_cast<char8_t const*>(s.data()), s.size() };
    }
    std::string_view utf8(std::u8string_view const s) noexcept {
        return { reinterpret_cast<char const*>(s.data()), s.size() };
    }

    int copy_file(lua_State* const vm) {
        lua::stack const ctx(vm);
        auto const from = ctx.at<std::string_view>(1_stack_index);
        auto const to = ctx.at<std::string_view>(2_stack_index);
        TRY
        std::error_code ec;
        constexpr auto options = std::filesystem::copy_options::overwrite_existing;
        auto const result = std::filesystem::copy_file(utf8(from), utf8(to), options, ec);
        RETURN_BOOLEAN(result)
        CATCH_RETURN(false)
    }

    int copy_directory(lua_State* const vm) {
        lua::stack const ctx(vm);
        auto const from = ctx.at<std::string_view>(1_stack_index);
        auto const to = ctx.at<std::string_view>(2_stack_index);
        TRY
        std::error_code ec;
        constexpr auto options = std::filesystem::copy_options::overwrite_existing;
        std::filesystem::copy(utf8(from), utf8(to), options, ec);
        ctx.push(true);
        return 1;
        CATCH_RETURN(false)
    }

    int copy_directories(lua_State* const vm) {
        lua::stack const ctx(vm);
        auto const from = ctx.at<std::string_view>(1_stack_index);
        auto const to = ctx.at<std::string_view>(2_stack_index);
        TRY
        std::error_code ec;
        constexpr auto options = std::filesystem::copy_options::overwrite_existing
            | std::filesystem::copy_options::recursive;
        std::filesystem::copy(utf8(from), utf8(to), options, ec);
        ctx.push(true);
        return 1;
        CATCH_RETURN(false)
    }

    int create_directory(lua_State* const vm) {
        lua::stack const ctx(vm);
        auto const path = ctx.at<std::string_view>(1_stack_index);
        TRY
        std::error_code ec;
        auto const result = std::filesystem::create_directory(utf8(path), ec);
        RETURN_BOOLEAN(result)
        CATCH_RETURN(false)
    }

    int create_directories(lua_State* const vm) {
        lua::stack const ctx(vm);
        auto const path = ctx.at<std::string_view>(1_stack_index);
        TRY
        std::error_code ec;
        auto const result = std::filesystem::create_directories(utf8(path), ec);
        RETURN_BOOLEAN(result)
        CATCH_RETURN(false)
    }

    int is_regular_file(lua_State* const vm) {
        lua::stack const ctx(vm);
        auto const path = ctx.at<std::string_view>(1_stack_index);
        TRY
        std::error_code ec;
        auto const result = std::filesystem::is_regular_file(utf8(path), ec);
        RETURN_BOOLEAN(result)
        CATCH_RETURN(false)
    }

    int is_directory(lua_State* const vm) {
        lua::stack const ctx(vm);
        auto const path = ctx.at<std::string_view>(1_stack_index);
        TRY
        std::error_code ec;
        auto const result = std::filesystem::is_directory(utf8(path), ec);
        RETURN_BOOLEAN(result)
        CATCH_RETURN(false)
    }

    int get_file_size(lua_State* const vm) {
        lua::stack const ctx(vm);
        auto const path = ctx.at<std::string_view>(1_stack_index);
        TRY
        std::error_code ec;
        auto const result = std::filesystem::file_size(utf8(path), ec);
        if (result != static_cast<std::uintmax_t>(-1)) {
            if (ctx.push((result))) {
                return 1;
            }
            ec = std::make_error_code(std::errc::value_too_large);
        }
        RETURN_ERROR(std::nullopt, ec.message(), ec.value())
        CATCH_RETURN(std::nullopt)
    }

    int read_file(lua_State* const vm) {
        lua::stack const ctx(vm);
        auto const path = ctx.at<std::string_view>(1_stack_index);
        TRY
        auto const options = std::ifstream::in | std::ifstream::binary;
        std::ifstream file(std::filesystem::path(utf8(path)), options);
        if (!file.is_open()) {
            RETURN_ERROR(std::nullopt, "open file failed"sv, -1)
        }
        if (!file.seekg(0, std::ifstream::end)) {
            RETURN_ERROR(std::nullopt, "seek file failed"sv, -1)
        }
        auto const end = file.tellg();
        if (end == static_cast<std::streampos>(-1)) {
            RETURN_ERROR(std::nullopt, "tell file failed"sv, -1)
        }
        if (!file.seekg(0, std::ifstream::beg)) {
            RETURN_ERROR(std::nullopt, "seek file failed"sv, -1)
        }
        auto const beg = file.tellg();
        if (beg == static_cast<std::streampos>(-1)) {
            RETURN_ERROR(std::nullopt, "tell file failed"sv, -1)
        }
        auto const size = end - beg;
        std::vector<char> buffer(static_cast<size_t>(size));
        if (!file.read(buffer.data(), size)) {
            RETURN_ERROR(std::nullopt, "read file failed"sv, -1)
        }
        ctx.push(std::string_view(buffer.data(), buffer.size()));
        return 1;
        CATCH_RETURN(std::nullopt)
    }

    int write_file(lua_State* const vm) {
        lua::stack const ctx(vm);
        auto const path = ctx.at<std::string_view>(1_stack_index);
        auto const content = ctx.at<std::string_view>(2_stack_index);
        TRY
        auto const options = std::ofstream::out | std::ofstream::trunc | std::ofstream::binary;
        std::ofstream file(std::filesystem::path(utf8(path)), options);
        if (!file.is_open()) {
            RETURN_ERROR(false, "open file failed"sv, -1)
        }
        if (!file.write(content.data(), content.size())) {
            RETURN_ERROR(false, "write file failed"sv, -1)
        }
        ctx.push(true);
        return 1;
        CATCH_RETURN(false)
    }

    int rename(lua_State* const vm) {
        lua::stack const ctx(vm);
        auto const from = ctx.at<std::string_view>(1_stack_index);
        auto const to = ctx.at<std::string_view>(2_stack_index);
        TRY
        std::error_code ec;
        std::filesystem::rename(utf8(from), utf8(to), ec);
        RETURN_BOOLEAN(!ec);
        CATCH_RETURN(false)
    }

    int remove_file(lua_State* const vm) {
        lua::stack const ctx(vm);
        auto const path = ctx.at<std::string_view>(1_stack_index);
        TRY
        std::error_code ec;
        auto const result = std::filesystem::remove(utf8(path), ec);
        RETURN_BOOLEAN(result)
        CATCH_RETURN(false)
    }

    int remove_directories(lua_State* const vm) {
        lua::stack const ctx(vm);
        auto const path = ctx.at<std::string_view>(1_stack_index);
        TRY
        std::error_code ec;
        auto const result = std::filesystem::remove_all(utf8(path), ec);
        RETURN_BOOLEAN(result != static_cast<std::uintmax_t>(-1))
        CATCH_RETURN(false)
    }

    struct directory_iterator {
        static inline std::string_view const class_name{ "std.fs.directory_iterator"sv };

        std::filesystem::directory_iterator current;
        std::filesystem::directory_iterator end;

        directory_iterator() = default;
        directory_iterator(directory_iterator const&) = delete;
        directory_iterator(directory_iterator&&) = delete;
        ~directory_iterator() = default;

        directory_iterator& operator=(directory_iterator const&) = delete;
        directory_iterator& operator=(directory_iterator&&) = delete;

        static int meta_gc(lua_State* const vm) {
            auto const self = as(vm, 1);
            self->~directory_iterator();
            return 0;
        }
        static int meta_to_string(lua_State* const vm) {
            lua::stack const ctx(vm);
            [[maybe_unused]] auto const self = as(vm, 1);
            ctx.push(class_name);
            return 1;
        }
        static int meta_call(lua_State* const vm) {
            lua::stack const ctx(vm);
            auto const self = as(vm, 1);
            if (self->current != self->end) {
                auto const& entry = self->current;
                auto const path = entry->path().lexically_normal().generic_u8string();
                std::error_code ec;
                self->current.increment(ec);
                ctx.push(utf8(path));
            }
            else {
                ctx.push(std::nullopt);
            }
            return 1;
        }

        static int close(lua_State* const vm) {
            auto const self = as(vm, 1);
            self->current = self->end;
            return 0;
        }

        static directory_iterator* as(lua_State* const vm, int const idx) {
            return static_cast<directory_iterator*>(luaL_checkudata(vm, idx, class_name.data()));
        }
        static directory_iterator* create(lua_State* const vm) {
            auto const self = static_cast<directory_iterator*>(lua_newuserdata(vm, sizeof(directory_iterator)));
            new(self) directory_iterator();
            auto const self_index = lua_gettop(vm);
            luaL_getmetatable(vm, class_name.data());
            lua_setmetatable(vm, self_index);
            return self;
        }
        static void register_class(lua_State* const vm) {
            lua::stack_balancer const sb(vm);
            lua::stack const ctx(vm);

            auto const method = ctx.create_map();
            method.set("next"sv, &meta_call);
            method.set("close"sv, &close);

            auto const metatable = ctx.create_metatable(class_name);
            metatable.set("__gc"sv, &meta_gc);
            metatable.set("__tostring"sv, &meta_to_string);
            metatable.set("__call"sv, &meta_call);
            metatable.set("__index"sv, method);
        }
    };

    int create_directory_iterator(lua_State* const vm) {
        lua::stack const ctx(vm);
        auto const path = ctx.at<std::string_view>(1_stack_index);
        auto const self = directory_iterator::create(vm);
        auto const options =  std::filesystem::directory_options::skip_permission_denied;
        TRY
        std::error_code ec;
        self->current = std::filesystem::directory_iterator(utf8(path), options, ec);
        if (ec) {
            ctx.push(std::nullopt);
            ctx.push(ec.message());
            ctx.push(ec.value());
            return 3;
        }
        return 1;
        CATCH_RETURN(std::nullopt)
    }

    struct recursive_directory_iterator {
        static inline std::string_view const class_name{ "std.fs.recursive_directory_iterator"sv };

        std::filesystem::recursive_directory_iterator current;
        std::filesystem::recursive_directory_iterator end;

        recursive_directory_iterator() = default;
        recursive_directory_iterator(recursive_directory_iterator const&) = delete;
        recursive_directory_iterator(recursive_directory_iterator&&) = delete;
        ~recursive_directory_iterator() = default;

        recursive_directory_iterator& operator=(recursive_directory_iterator const&) = delete;
        recursive_directory_iterator& operator=(recursive_directory_iterator&&) = delete;

        static int meta_gc(lua_State* const vm) {
            auto const self = as(vm, 1);
            self->~recursive_directory_iterator();
            return 0;
        }
        static int meta_to_string(lua_State* const vm) {
            lua::stack const ctx(vm);
            [[maybe_unused]] auto const self = as(vm, 1);
            ctx.push(class_name);
            return 1;
        }
        static int meta_call(lua_State* const vm) {
            lua::stack const ctx(vm);
            auto const self = as(vm, 1);
            if (self->current != self->end) {
                auto const& entry = self->current;
                auto const path = entry->path().lexically_normal().generic_u8string();
                std::error_code ec;
                self->current.increment(ec);
                ctx.push(utf8(path));
            }
            else {
                ctx.push(std::nullopt);
            }
            return 1;
        }

        static recursive_directory_iterator* as(lua_State* const vm, int const idx) {
            return static_cast<recursive_directory_iterator*>(luaL_checkudata(vm, idx, class_name.data()));
        }
        static recursive_directory_iterator* create(lua_State* const vm) {
            auto const self = static_cast<recursive_directory_iterator*>(lua_newuserdata(vm, sizeof(recursive_directory_iterator)));
            new(self) recursive_directory_iterator();
            auto const self_index = lua_gettop(vm);
            luaL_getmetatable(vm, class_name.data());
            lua_setmetatable(vm, self_index);
            return self;
        }
        static void register_class(lua_State* const vm) {
            lua::stack_balancer const sb(vm);
            lua::stack const ctx(vm);

            auto const metatable = ctx.create_metatable(class_name);
            metatable.set("__gc"sv, &meta_gc);
            metatable.set("__tostring"sv, &meta_to_string);
            metatable.set("__call"sv, &meta_call);
        }
    };

    int create_recursive_directory_iterator(lua_State* const vm) {
        lua::stack const ctx(vm);
        auto const path = ctx.at<std::string_view>(1_stack_index);
        auto const self = recursive_directory_iterator::create(vm);
        auto const options =  std::filesystem::directory_options::skip_permission_denied;
        TRY
        std::error_code ec;
        self->current = std::filesystem::recursive_directory_iterator(utf8(path), options, ec);
        if (ec) {
            ctx.push(std::nullopt);
            ctx.push(ec.message());
            ctx.push(ec.value());
            return 3;
        }
        return 1;
        CATCH_RETURN(std::nullopt)
    }
}

extern "C" {
    LUALIB_API int luaopen_std_fs(lua_State* const vm) {
        lua::stack const ctx(vm);

        auto const fs = ctx.create_module("std.fs"sv);
        // file
        fs.set("is_regular_file"sv, &is_regular_file);
        fs.set("get_file_size"sv, &get_file_size);
        fs.set("read_file"sv, &read_file);
        fs.set("write_file"sv, &write_file);
        fs.set("copy_file"sv, &copy_file);
        fs.set("remove_file"sv, &remove_file);
        // directory
        fs.set("is_directory"sv, &is_directory);
        fs.set("create_directory"sv, &create_directory);
        fs.set("create_directories"sv, &create_directories);
        fs.set("copy_directory"sv, &copy_directory);
        fs.set("copy_directories"sv, &copy_directories);
        fs.set("remove_directory"sv, &remove_file);
        fs.set("remove_directories"sv, &remove_directories);
        fs.set("directory_iterator"sv, &create_directory_iterator);
        fs.set("recursive_directory_iterator"sv, &create_recursive_directory_iterator);
        // file & directory
        fs.set("rename"sv, &rename);

        directory_iterator::register_class(vm);
        recursive_directory_iterator::register_class(vm);

        lua_pushnil(vm);
        lua_setglobal(vm, "std.fs");

        return 1;
    }
}
