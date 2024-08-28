---@class internal.test
local test = {}

---@type { name: string, f: fun(): boolean }[]
local tests = {}

---@param name string
---@param f fun(): boolean
function test.add(name, f)
    table.insert(tests, {
        name = name,
        f = f,
    })
end

function test.run()
    ---@type osdate
    local t = os.date("*t")
    print(string.format("=========================== test -- %04d-%02d-%02d %02d:%02d ===========================", t.year, t.month, t.day, t.hour, t.min))
    local n = #tests
    local passed = 0
    local failed = 0
    for i, v in ipairs(tests) do
        if v.f() then
            passed = passed + 1
            print(string.format("%d/%d [%s] PASS", i, n, v.name))
        else
            failed = failed + 1
            print(string.format("%d/%d [%s] FAIL", i, n, v.name))
        end
    end
    print(string.format("==================================== summary ===================================\nTOTAL  %d\nPASSED %d/%d %.2f%%\nFAILED %d/%d %.2f%%",
        n, passed, n, 100 * passed / n, failed, n, 100 * failed / n))
end

return test
