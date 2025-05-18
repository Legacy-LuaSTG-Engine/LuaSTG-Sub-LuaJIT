local table = require("table")
local string = require("string")
local io = require("io")
local ffi = require("ffi")

ffi.cdef [[
typedef int32_t BOOL;
BOOL __stdcall QueryPerformanceCounter(int64_t* value);
BOOL __stdcall QueryPerformanceFrequency(int64_t* value);
]]

local kernel32 = ffi.load("kernel32.dll")

local frequency = ffi.new("int64_t[?]", 1, 0)
local current = ffi.new("int64_t[?]", 1, 0)
kernel32.QueryPerformanceFrequency(frequency)
local function getTimer()
    kernel32.QueryPerformanceCounter(current)
    return tonumber(current[0]) / tonumber(frequency[0])
end

print("benchmark...")
local file_discard = assert(io.open("discard.txt", "w"))

local function scene1()
    local objects = {}
    for i = 1, 1000 do
        objects[i] = {
            x = 0.0,
            y = 0.0,
            vx = i / 100.0,
            vy = i / 1000.0,
        }
    end
    local t1 = getTimer()
    for _ = 1, (60 * 60) do
        for i = 1, 1000 do
            objects[i].x = objects[i].x + objects[i].vx
            objects[i].y = objects[i].y + objects[i].vy
        end
    end
    local t2 = getTimer()
    print("scene1: ", t2 - t1)
    local results = {}
    for i = 1, 1000 do
        table.insert(results, string.format("[i]{x=%f,y=%f}", i, objects[i].x, objects[i].y))
    end
    file_discard:write(table.concat(results, " "))
end

for _ = 1, 10 do
    scene1()
end
