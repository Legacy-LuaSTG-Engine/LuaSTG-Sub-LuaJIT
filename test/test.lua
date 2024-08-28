local try = require("try")

---@param name string
---@param f fun(): boolean
local function test(name, f)
    if f() then
        print(string.format("[%s] PASS", name))
    else
        print(string.format("[%s] FAIL", name))
    end
end

test("disable goto", function()
    local result = true
    try({
        function()
            require("language.goto")
            result = false
        end,
    })
    return result
end)

test("disable goto label", function()
    local result = true
    try({
        function()
            require("language.goto_label")
            result = false
        end,
    })
    return result
end)
