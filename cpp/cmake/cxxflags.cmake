# C++17
set(CMAKE_CXX_STANDARD 17)

# OpenSSL 3.1
string(APPEND
    CMAKE_CXX_FLAGS
    " -DOPENSSL_NO_DEPRECATED"
)

# g++ release flags
string(APPEND
    CMAKE_CXX_FLAGS_RELEASE
    " -O3"
)

# g++ debug flags
string(APPEND
    CMAKE_CXX_FLAGS_DEBUG
    " -fabi-version=0"
    " -fdiagnostics-color"

    " -pedantic"
    " -pedantic-errors"

    " -Wall"
    " -Werror"
    " -Wextra"

    " -Wabi=18"
    " -Wcast-align"
    " -Wcast-qual"
    " -Wconversion"
    " -Wdisabled-optimization"
    " -Wduplicated-branches"
    " -Wfloat-equal"
    " -Wformat-nonliteral"
    " -Wformat-security"
    " -Wformat-y2k"
    " -Wformat=2"
    " -Wimport"
    " -Winit-self"
    " -Winvalid-pch"
    " -Wlogical-op"
    " -Wmissing-declarations"
    " -Wmissing-field-initializers"
    " -Wmissing-format-attribute"
    " -Wmissing-include-dirs"
    " -Wmissing-noreturn"
    " -Wno-unused"
    " -Wnoexcept"
    " -Wnormalized"
    " -Wold-style-cast"
    " -Woverloaded-virtual"
    " -Wpacked"
    " -Wpessimizing-move"
    " -Wpointer-arith"
    " -Wredundant-decls"
    " -Wsign-conversion"
    " -Wsign-promo"
    " -Wstack-protector"
    " -Wstrict-aliasing=2"
    " -Wstrict-null-sentinel"
    " -Wstrict-overflow=5"
    " -Wswitch-default"
    " -Wswitch-enum"
    " -Wundef"
    " -Wunreachable-code"
    " -Wunsafe-loop-optimizations"
    " -Wunused"
    " -Wunused-parameter"
    " -Wvariadic-macros"
    " -Wwrite-strings"
    " -Wzero-as-null-pointer-constant"

    " -Wno-aggregate-return"
    " -Wno-c++20-extensions"
    " -Wno-effc++"
    " -Wno-inline"
    " -Wno-padded"
    " -Wno-shadow"
    " -Wno-suggest-attribute=format"

    " -Og -ggdb"
)
