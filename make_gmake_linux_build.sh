#!/bin/bash

python Psybrus/reflection_parse.py LD31Game

Psybrus/Tools/genie/genie-linux --platform=x64 --toolchain=linux-clang --boostpath=$BOOST_PATH gmake
