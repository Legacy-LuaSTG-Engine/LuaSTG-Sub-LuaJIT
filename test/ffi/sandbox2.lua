local lstg = {
    Vector2 = require("lstg.Vector2"),
    Vector2Unchecked = require("lstg.Vector2Unchecked")
}

local type = type
local ffi = require("ffi")

ffi.cdef [[
typedef struct Vector2 {
    double x;
    double y;
} Vector2;
typedef void (*Vector2_random)(Vector2* self);
typedef void (*Vector2_assign)(Vector2* self, double x, double y);
typedef double (*getPerformanceCounter)();
]]

local getPerformanceCounter = ffi.cast("getPerformanceCounter", getPerformanceCounter)

local Vector2_random = ffi.cast("Vector2_random", Vector2_random)
local Vector2_assign = ffi.cast("Vector2_assign", Vector2_assign)

local Vector2
local Vector2_mt = {}
Vector2_mt.__index = {
    random = Vector2_random,
    assign = Vector2_assign,
}
Vector2 = ffi.metatype("Vector2", Vector2_mt)

do
    local t1 = getPerformanceCounter()
    local v1 = Vector2(114, 514)
    for i = 1, 10000000 do
        v1:assign(i, i + 1)
    end
    local t2 = getPerformanceCounter()
    print("luajit ffi API time: " .. (t2 - t1))
end

do
    local t1 = getPerformanceCounter()
    local v1 = lstg.Vector2Unchecked.create(114, 514)
    for i = 1, 10000000 do
        v1:assign(i, i + 1)
    end
    local t2 = getPerformanceCounter()
    print("lua C API (unchecked) time: " .. (t2 - t1))
end

do
    local t1 = getPerformanceCounter()
    local v1 = lstg.Vector2.create(114, 514)
    for i = 1, 10000000 do
        v1:assign(i, i + 1)
    end
    local t2 = getPerformanceCounter()
    print("lua C API time: " .. (t2 - t1))
end
