local test = require("test")
local try = require("try")

test.add("disable goto", function()
    local result = true
    try({
        function()
            require("language.broken.goto")
            result = false
        end,
    })
    return result
end)

test.add("disable goto label", function()
    local result = true
    try({
        function()
            require("language.broken.goto_label")
            result = false
        end,
    })
    return result
end)
