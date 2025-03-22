local type = type
local ffi = require("ffi")

ffi.cdef [[
typedef struct Vector2 {
    double x;
    double y;
} Vector2;
typedef void (*printVector2)(Vector2*);
typedef void (*randomVector2)(Vector2*);
typedef double (*getPerformanceCounter)();
]]

local Vector2
local Vector2_mt = {}
function Vector2_mt:__add(right)
    if type(right) == "number" then
        return Vector2(self.x + right, self.y + right)
    else
        return Vector2(self.x + right.x, self.y + right.y)
    end
end
Vector2 = ffi.metatype("Vector2", Vector2_mt)

local a = Vector2(1, 2)
local b = Vector2(3, 4)
local c = a + b

--print("---------- FFI struct ----------")

--print(a.x, a.y)
--print(b.x, b.y)
--print(c.x, c.y)

--print("---------- FFI array ----------")

local Vector2Array = ffi.typeof("Vector2[?]")

local arr = Vector2Array(10)
local total = Vector2(0, 0)
for i = 0, 9 do
    arr[i].x = i
    arr[i].y = i * i
    total = total + arr[i]
    --print(arr[i].x, arr[i].y)
end
--print("total", total.x, total.y)

--print("---------- FFI type name ----------")

--print(Vector2)
--print(Vector2Array)

--print("---------- FFI function pointer from C/C++ ----------")

local _printVector2 = ffi.cast("printVector2", printVector2)
--_printVector2(c)

local _getPerformanceCounter = ffi.cast("getPerformanceCounter", getPerformanceCounter)
--print("getPerformanceCounter()", _getPerformanceCounter())

local _randomVector2 = ffi.cast("randomVector2", randomVector2)

do
    local t1 = _getPerformanceCounter()
    local v1 = Vector2(0, 0)
    for _ = 1, 10000000 do
        _randomVector2(v1)
    end
    local t2 = _getPerformanceCounter()
    print("ffi time: " .. (t2 - t1))
end
