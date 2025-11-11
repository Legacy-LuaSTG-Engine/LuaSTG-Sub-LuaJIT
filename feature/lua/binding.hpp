#pragma once
#include <cstdint>
#include <string_view>
#include <string>
#include <optional>
#include <limits>
#include "lua.hpp"

namespace lua {
    class stack_index {
    public:
        stack_index() noexcept = default;
        constexpr explicit stack_index(int32_t const value) noexcept : m_value(value) {}
        stack_index(stack_index const&) noexcept = default;
        stack_index(stack_index&&) noexcept = default;
        ~stack_index() noexcept = default;

        stack_index& operator=(stack_index const&) noexcept = default;
        stack_index& operator=(stack_index&&) noexcept = default;

        [[nodiscard]] int32_t value() const noexcept { return m_value; }
    private:
        int32_t m_value{};
    };

    inline constexpr stack_index operator""_stack_index(unsigned long long const value) noexcept {
        return stack_index(static_cast<int32_t>(value));
    }

    static_assert(sizeof(stack_index) == sizeof(int32_t));

    //struct store_as_pointer {
    //    void* unused{};
    //};

    class map {
    public:
        map() = delete;
        map(lua_State* const vm, stack_index const index) noexcept : m_state(vm), m_index(index) {}
        map(map const&) = delete;
        map(map&&) noexcept = default;
        ~map() noexcept = default;

        map& operator=(map const&) = delete;
        map& operator=(map&&) noexcept = default;

        void set(std::string_view const key, lua_CFunction const value) const noexcept {
            lua_pushlstring(m_state, key.data(), key.size());
            lua_pushcfunction(m_state, value);
            lua_settable(m_state, m_index.value());
        }
        void set(std::string_view const key, map const& value) const noexcept {
            lua_pushlstring(m_state, key.data(), key.size());
            lua_pushvalue(m_state, value.m_index.value());
            lua_settable(m_state, m_index.value());
        }

    private:
        lua_State* m_state{};
        stack_index m_index;
    };

    class stack {
    public:
        stack() = delete;
        explicit stack(lua_State* const vm) noexcept : m_state(vm) {}
        stack(stack const&) = delete;
        stack(stack&&) = delete;
        ~stack() noexcept = default;

        stack& operator=(stack const&) = delete;
        stack& operator=(stack&&) = delete;

        // pop

        void pop() const noexcept {
            lua_pop(m_state, 1);
        }
        void pop(size_t const count) const noexcept {
            lua_pop(m_state, static_cast<int>(count));
        }

        // size

        [[nodiscard]] size_t size() const noexcept {
            return static_cast<size_t>(lua_gettop(m_state));
        }

        // type

        [[nodiscard]] bool is_none(stack_index const index) const noexcept {
            return lua_isnone(m_state, index.value());
        }
        [[nodiscard]] bool is_nil(stack_index const index) const noexcept {
            return lua_isnil(m_state, index.value());
        }
        [[nodiscard]] bool is_none_or_nil(stack_index const index) const noexcept {
            return lua_isnoneornil(m_state, index.value());
        }
        [[nodiscard]] bool is_boolean(stack_index const index) const noexcept {
            return lua_isboolean(m_state, index.value());
        }
        [[nodiscard]] bool is_number(stack_index const index) const noexcept {
            return lua_type(m_state, index.value()) == LUA_TNUMBER;
        }
        [[nodiscard]] bool is_string(stack_index const index) const noexcept {
            return lua_type(m_state, index.value()) == LUA_TSTRING;
        }
        [[nodiscard]] bool is_table(stack_index const index) const noexcept {
            return lua_istable(m_state, index.value());
        }
        [[nodiscard]] bool is_function(stack_index const index) const noexcept {
            return lua_isfunction(m_state, index.value()) && !lua_iscfunction(m_state, index.value());
        }
        [[nodiscard]] bool is_c_function(stack_index const index) const noexcept {
            return lua_iscfunction(m_state, index.value());
        }
        [[nodiscard]] bool is_userdata(stack_index const index) const noexcept {
            return lua_isuserdata(m_state, index.value()) && !lua_islightuserdata(m_state, index.value());
        }
        [[nodiscard]] bool is_light_userdata(stack_index const index) const noexcept {
            return lua_islightuserdata(m_state, index.value());
        }
        [[nodiscard]] bool is_coroutine(stack_index const index) const noexcept {
            return lua_isthread(m_state, index.value());
        }

        [[nodiscard]] bool has_number(stack_index const index) const noexcept {
            return lua_isnumber(m_state, index.value());
        }
        [[nodiscard]] bool has_string(stack_index const index) const noexcept {
            return lua_isstring(m_state, index.value());
        }
        [[nodiscard]] bool has_function(stack_index const index) const noexcept {
            return lua_isfunction(m_state, index.value());
        }
        [[nodiscard]] bool has_userdata(stack_index const index) const noexcept {
            return lua_isuserdata(m_state, index.value());
        }

        // push

        void push(std::nullopt_t) const noexcept {
            lua_pushnil(m_state);
        }
        void push(bool const value) const noexcept {
            lua_pushboolean(m_state, value ? 1 : 0);
        }
        void push(int8_t const value) const noexcept {
            lua_pushinteger(m_state, value);
        }
        void push(int16_t const value) const noexcept {
            lua_pushinteger(m_state, value);
        }
        void push(int32_t const value) const noexcept {
            lua_pushinteger(m_state, value);
        }
        [[nodiscard]] bool push(int64_t const value) const noexcept {
            if (value >= std::numeric_limits<int32_t>::min() && value <= std::numeric_limits<int32_t>::max()) {
                lua_pushinteger(m_state, value);
                return true;
            }
            if constexpr (sizeof(lua_Number) == sizeof(double)) {
                // 2 ^ 53 - 1
                if (value >= -9007199254740991ll && value <= 9007199254740991ll) {
                    lua_pushnumber(m_state, static_cast<double>(value));
                    return true;
                }
            }
            return false;
        }
        void push(uint8_t const value) const noexcept {
            lua_pushinteger(m_state, static_cast<int32_t>(value));
        }
        void push(uint16_t const value) const noexcept {
            lua_pushinteger(m_state, static_cast<int32_t>(value));
        }
        [[nodiscard]] bool push(uint32_t const value) const noexcept {
            if (value <= static_cast<uint32_t>(std::numeric_limits<int32_t>::max())) {
                lua_pushinteger(m_state, static_cast<int32_t>(value));
                return true;
            }
            if constexpr (sizeof(lua_Number) == sizeof(double)) {
                lua_pushnumber(m_state, static_cast<double>(value));
                return true;
            }
            return false;
        }
        [[nodiscard]] bool push(uint64_t const value) const noexcept {
            if (value <= static_cast<uint64_t>(std::numeric_limits<int32_t>::max())) {
                lua_pushinteger(m_state, static_cast<int32_t>(value));
                return true;
            }
            if constexpr (sizeof(lua_Number) == sizeof(double)) {
                // 2 ^ 53 - 1
                if (value <= 9007199254740991ll) {
                    lua_pushnumber(m_state, static_cast<double>(value));
                    return true;
                }
            }
            return false;
        }
        void push(std::string_view const value) const noexcept {
            lua_pushlstring(m_state, value.data(), value.size());
        }

        void push(lua_CFunction const value) const noexcept {
            lua_pushcfunction(m_state, value);
        }
        void push(void* const value) const noexcept {
            lua_pushlightuserdata(m_state, value);
        }

        //template<typename Tag> void push(int64_t const value) const noexcept;
        //template<> void push<store_as_pointer>(int64_t const value) const noexcept {
        //    static_assert(sizeof(void*) >= sizeof(int64_t));
        //    lua_pushlightuserdata(m_state, reinterpret_cast<void*>(value));
        //}
        //template<typename Tag> void push(uint64_t const value) const noexcept;
        //template<> void push<store_as_pointer>(uint64_t const value) const noexcept {
        //    static_assert(sizeof(void*) >= sizeof(uint64_t));
        //    lua_pushlightuserdata(m_state, reinterpret_cast<void*>(value));
        //}

        // access

        template<typename T> T at(stack_index const index) const noexcept;
        template<> bool at(stack_index const index) const noexcept {
            return lua_toboolean(m_state, index.value()) == 0 ? false : true;
        }
        template<> int8_t at(stack_index const index) const noexcept {
            return static_cast<int8_t>(luaL_checkinteger(m_state, index.value()));
        }
        template<> int16_t at(stack_index const index) const noexcept {
            return static_cast<int16_t>(luaL_checkinteger(m_state, index.value()));
        }
        template<> int32_t at(stack_index const index) const noexcept {
            return static_cast<int32_t>(luaL_checkinteger(m_state, index.value()));
        }
        template<> int64_t at(stack_index const index) const noexcept {
            if constexpr (sizeof(lua_Number) == sizeof(double)) {
                return static_cast<int64_t>(luaL_checknumber(m_state, index.value())); // TODO: check
            }
            return luaL_checkinteger(m_state, index.value());
        }
        template<> uint8_t at(stack_index const index) const noexcept {
            return static_cast<uint8_t>(luaL_checkinteger(m_state, index.value()));
        }
        template<> uint16_t at(stack_index const index) const noexcept {
            return static_cast<uint16_t>(luaL_checkinteger(m_state, index.value()));
        }
        template<> uint32_t at(stack_index const index) const noexcept {
            if constexpr (sizeof(lua_Number) == sizeof(double)) {
                return static_cast<uint32_t>(luaL_checknumber(m_state, index.value())); // TODO: check
            }
            return static_cast<uint32_t>(luaL_checkinteger(m_state, index.value()));
        }
        template<> uint64_t at(stack_index const index) const noexcept {
            if constexpr (sizeof(lua_Number) == sizeof(double)) {
                return static_cast<uint64_t>(luaL_checknumber(m_state, index.value())); // TODO: check
            }
            return static_cast<uint64_t>(luaL_checkinteger(m_state, index.value()));
        }
        template<> std::string_view at(stack_index const index) const noexcept {
            size_t len{};
            auto const str = luaL_checklstring(m_state, index.value(), &len);
            return { str, len };
        }

        template<> void* at(stack_index const index) const noexcept {
            return lua_touserdata(m_state, index.value());
        }

        //template<typename T, typename Tag> T at(stack_index const index) const noexcept;
        //template<> int64_t at<int64_t, store_as_pointer>(stack_index const index) const noexcept {
        //    static_assert(sizeof(void*) >= sizeof(int64_t));
        //    auto const value = lua_touserdata(m_state, index.value());
        //    return reinterpret_cast<int64_t>(value);
        //}
        //template<> uint64_t at<uint64_t, store_as_pointer>(stack_index const index) const noexcept {
        //    static_assert(sizeof(void*) >= sizeof(uint64_t));
        //    auto const value = lua_touserdata(m_state, index.value());
        //    return reinterpret_cast<uint64_t>(value);
        //}

        // access top

        template<typename T> T top() const noexcept { return at<T>(stack_index(-1)); }
        template<typename T, typename Tag> T top() const noexcept { return at<T, Tag>(stack_index(-1)); }

        // map

        [[nodiscard]] map create_map() const noexcept {
            lua_newtable(m_state);
            return map(m_state, stack_index(lua_gettop(m_state)));
        }

        // module

        [[nodiscard]] map create_module(std::string_view const name) const noexcept {
            constexpr luaL_Reg empty[]{{}};
            std::string const c_name(name);
            luaL_register(m_state, c_name.c_str(), empty);
            return map(m_state, stack_index(lua_gettop(m_state)));
        }
        [[nodiscard]] map get_module(std::string_view const name) const noexcept {
            std::string const c_name(name);
            auto const n = lua_gettop(m_state);                  // 1..n |  n + 1  |  n + 2  |
            lua_getfield(m_state, LUA_REGISTRYINDEX, "_LOADED"); // .... | _LOADED |         | from lua 5.1/luajit source code
            lua_getfield(m_state, n + 1, c_name.c_str());        // .... | _LOADED | module  |
            lua_remove(m_state, n + 1);                          // .... | module  |         |
            if (!lua_istable(m_state, n + 1)) {
                return map(m_state, stack_index(luaL_error(m_state, "module '%s' not found", c_name.c_str())));
            }
            return map(m_state, stack_index(n + 1));
        }

        // metatable

        [[nodiscard]] map create_metatable(std::string_view const name) const noexcept {
            std::string const c_name(name);
            luaL_newmetatable(m_state, c_name.c_str());
            return map(m_state, stack_index(lua_gettop(m_state)));
        }

    private:
        lua_State* m_state{};
    };

    static_assert(sizeof(stack) == sizeof(lua_State*));

    class stack_balancer {
    public:
        stack_balancer() = delete;
        explicit stack_balancer(lua_State* const vm) noexcept : m_state(vm), m_size(lua_gettop(vm)) {}
        stack_balancer(stack_balancer const&) = delete;
        stack_balancer(stack_balancer&&) = delete;
        ~stack_balancer() noexcept { lua_settop(m_state, m_size); }

        stack_balancer operator=(stack_balancer const&) = delete;
        stack_balancer operator=(stack_balancer&&) = delete;

    private:
        lua_State* m_state{};
        int32_t m_size{};
    };
}
