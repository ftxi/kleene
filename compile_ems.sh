#!/bin/sh

emcc docs/library.cpp src/parser.cpp \
  -std=c++20 \
  -I include \
  -o docs/library.js \
  -s EXPORTED_FUNCTIONS='["_run_program"]' \
  -s EXPORTED_RUNTIME_METHODS='["ccall","cwrap"]' \
  -s MODULARIZE=0 \
  -s EXPORT_NAME='Module' \
  -O3 \
  -fexceptions \
  -s ALLOW_MEMORY_GROWTH=1 \
  -s ASSERTIONS=0

if [ $? -ne 0 ]; then
    echo "Emscripten compilation failed"
    exit 1
fi
echo "seems Emscripten compilation has succeeded"
echo "Do you want to run the local server? ([y]/n)"
read -r -n1 key

if [[ -z "$key" || "$key" == "y" || "$key" == "Y" ]]; then
    python docs/generate_examples.py
    emrun docs/
else
    echo "exit."
fi

