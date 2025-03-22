#include <cstdio>
#include <utility>
#include "lua.hpp"
#include <windows.h>

namespace {
	struct Vector2 {
		double x;
		double y;

		void random() {
			x = static_cast<double>(rand());
			y = static_cast<double>(rand());
		}
		void assign(double const x1, double const y1) {
			x = x1;
			y = y1;
		}
	};

	double getPerformanceCounter() {
		LARGE_INTEGER f{};
		LARGE_INTEGER t{};
		QueryPerformanceFrequency(&f);
		QueryPerformanceCounter(&t);
		return static_cast<double>(t.QuadPart) / static_cast<double>(f.QuadPart);
	}

	struct Vector2Binding {
		static constexpr char class_name[]{ "lstg.Vector2" };

		static int random(lua_State* vm) {
			auto self = as(vm, 1);
			self->random();
			return 0;
		}
		static int assign(lua_State* vm) {
			auto self = as(vm, 1);
			auto const x = luaL_checknumber(vm, 1 + 1);
			auto const y = luaL_checknumber(vm, 1 + 2);
			self->assign(x, y);
			return 0;
		}
		static int create(lua_State* vm) {
			if (lua_gettop(vm) >= 2) {
				auto const x = luaL_checknumber(vm, 1);
				auto const y = luaL_checknumber(vm, 2);
				auto self = createInstance(vm);
				*self = Vector2{ x, y };
			}
			else {
				createInstance(vm);
			}
			return 1;
		}

		static Vector2* createInstance(lua_State* vm) {
			auto self = static_cast<Vector2*>(lua_newuserdata(vm, sizeof(Vector2)));
			std::memset(self, 0, sizeof(Vector2));
			luaL_setmetatable(vm, class_name);
			return self;
		}
		static Vector2* as(lua_State* vm, int const idx) {
			return static_cast<Vector2*>(luaL_checkudata(vm, idx, class_name));
		}
		static bool is(lua_State* vm, int const idx) {
			return luaL_checkudata(vm, idx, class_name) != nullptr;
		}
		static void registerClass(lua_State* vm) {
			auto const n = lua_gettop(vm);

			luaL_newmetatable(vm, class_name);
			constexpr luaL_Reg mt[]{
				{},
			};
			luaL_register(vm, nullptr, mt);

			constexpr luaL_Reg m[]{
				{"random", &random},
				  {"assign", &assign},
				 {"create", &create},
				{},
			};
			luaL_register(vm, "lstg.Vector2", m);
			lua_setfield(vm, -2, "__index");

			lua_settop(vm, n);
		}
	};

	struct Vector2BindingUnchecked {
		static constexpr char class_name[]{ "lstg.Vector2Unchecked" };

		static int random(lua_State* vm) {
			auto self = as(vm, 1);
			self->random();
			return 0;
		}
		static int assign(lua_State* vm) {
			auto self = as(vm, 1);
			auto const x = lua_tonumber(vm, 1 + 1);
			auto const y = lua_tonumber(vm, 1 + 2);
			self->assign(x, y);
			return 0;
		}
		static int create(lua_State* vm) {
			if (lua_gettop(vm) >= 2) {
				auto const x = luaL_checknumber(vm, 1);
				auto const y = luaL_checknumber(vm, 2);
				auto self = createInstance(vm);
				*self = Vector2{ x, y };
			}
			else {
				createInstance(vm);
			}
			return 1;
		}

		static Vector2* createInstance(lua_State* vm) {
			auto self = static_cast<Vector2*>(lua_newuserdata(vm, sizeof(Vector2)));
			std::memset(self, 0, sizeof(Vector2));
			luaL_setmetatable(vm, class_name);
			return self;
		}
		static Vector2* as(lua_State* vm, int const idx) {
			return static_cast<Vector2*>(lua_touserdata(vm, idx));
		}
		static bool is(lua_State* vm, int const idx) {
			return luaL_checkudata(vm, idx, class_name) != nullptr;
		}
		static void registerClass(lua_State* vm) {
			auto const n = lua_gettop(vm);

			luaL_newmetatable(vm, class_name);
			constexpr luaL_Reg mt[]{
				{},
			};
			luaL_register(vm, nullptr, mt);

			constexpr luaL_Reg m[]{
				{"random", &random},
				 {"assign", &assign},
				 {"create", &create},
				{},
			};
			luaL_register(vm, "lstg.Vector2Unchecked", m);
			lua_setfield(vm, -2, "__index");

			lua_settop(vm, n);
		}
	};

	void Vector2_random(Vector2* self) {
		self->random();
	}
	void Vector2_assign(Vector2* self, double const x1, double const y1) {
		self->assign(x1, y1);
	}

#pragma optimize("", off)
	void test() {
		Vector2 v1{};
		auto const t1 = getPerformanceCounter();
		for (int i = 0; i < 1000'0000; ++i) {
			//Vector2_random(&v1);
			Vector2_assign(&v1, i, i + 1);
		}
		auto const t2 = getPerformanceCounter();
		std::printf("native time: %f\n", t2 - t1);
	}
#pragma optimize("", on)
}

int main() {
	lua_State* vm = luaL_newstate();
	if (luaJIT_setmode(vm, 0, LUAJIT_MODE_ENGINE | LUAJIT_MODE_OFF)) {
		std::printf("JIT engine started.\n");
	}
	luaL_openlibs(vm);
	lua_pushlightuserdata(vm, &getPerformanceCounter);
	lua_setglobal(vm, "getPerformanceCounter");
	lua_pushlightuserdata(vm, &Vector2_random);
	lua_setglobal(vm, "Vector2_random");
	lua_pushlightuserdata(vm, &Vector2_assign);
	lua_setglobal(vm, "Vector2_assign");
	Vector2Binding::registerClass(vm);
	Vector2BindingUnchecked::registerClass(vm);
	test();
	luaL_dofile(vm, "main.lua");
	lua_close(vm);
	return 0;
}
