# LuaSTG Sub LuaJIT

这是 LuaSTG Sub 的定制版 LuaJIT，主要包含以下修改：

* 添加 CMake 支持
* `io`、`os` 库支持 `utf-8` 编码
* 移植 Lua 5.4 的 `utf8` 库
* 移植 Lua 5.4 的 `string.pack`、`string.unpack` 功能
* 移除 `goto`

限制：

* CMake 脚本仅适配 Windows + MSVC
* 本人比较懒，更新可能不及时
