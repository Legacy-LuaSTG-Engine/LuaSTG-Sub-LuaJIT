local fs = require("std.fs")
print("----1")
for path in fs.directory_iterator("build") do
    print(path)
end
print("----2")
for path in fs.recursive_directory_iterator("build") do
    print(path)
end
