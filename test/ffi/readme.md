##

|       test name       |  time (seconds)  |   %   |
|-----------------------|------------------|-------|
| C native              | 0.109471         | 100%  |
| LuaJIT FFI API        | 0.12172190001002 | 111%  |
| Lua C API (unchecked) | 0.22695499996189 | 207%  |
| Lua C API             | 0.35635429999093 | 326%  |

## Test 2

pseudo-code:

```c++
struct Vector2 { double x; double y };
void Vector2_assign(Vector2* self, double x, double y) { self->x = x; self->y = y; }
Vector2 v1{};
for (int i = 0; i < 1000'0000; ++i) {
	Vector2_assign(&v1, i, i + 1);
}
```

```lua
local v1 = Vector2()
for i = 1, 10000000 do
    v1:assign(i, i + 1)
end
```

|       test name       |  time (seconds)  |   %   |
|-----------------------|------------------|-------|
| C native              | 0.013229         | 100%  |
| LuaJIT FFI API        | 0.01682750001783 | 127%  |
| Lua C API (unchecked) | 0.20465690002311 | 1547% |
| Lua C API             | 0.30829059996177 | 2330% |
