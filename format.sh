find ./ -iname "*.h" -o -iname "*.cc" | xargs clang-format -i 2> /tmp/null