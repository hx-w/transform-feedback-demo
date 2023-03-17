cmake -S . -B build/
cmake --build build/ --config Release

if [ "$1" = "run" ]; then
    ./build/demo
fi
