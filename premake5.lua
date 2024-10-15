workspace("MP3Reader")
configurations({ "Debug", "Release" })

project("MP3Reader")
kind("ConsoleApp")
language("C")
targetdir("bin/%{cfg.buildcfg}")

files({ "./src/**.h", "./src/**.c" })

filter("configurations:Debug")
defines({ "DEBUG" })
symbols("On")

filter("configurations:Release")
defines({ "NDEBUG" })
optimize("On")
