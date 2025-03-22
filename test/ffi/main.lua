local function protected()
    dofile("sandbox2.lua")
end

local r, m = pcall(protected)
if not r then
    print(m)
end
