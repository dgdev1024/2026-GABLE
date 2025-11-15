#!/bin/bash

set -e
premake5 gmake
make -j$(nproc) $@
echo "Build OK"
