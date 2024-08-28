---@param t { [1]: function, catch: fun(e: any)?, finally: function? }
local function try(t)
    assert(type(t[1]) == "function", "try: invalid body function")
    if t.catch ~= nil then
        assert(type(t.catch) == "function", "try: invalid catch function")
    end
    if t.finally ~= nil then
        assert(type(t.finally) == "function", "try: invalid finally function")
    end
    local r, e = pcall(t[1])
    if not r and t.catch then
        t.catch(e)
    end
    if t.finally then
        t.finally()
    end
end

return try
