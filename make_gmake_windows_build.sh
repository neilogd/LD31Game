#!/bin/bash

python Psybrus/reflection_parse.py DevelopmentGame

Psybrus/Tools/genie/genie-linux --platform=x32 --toolchain=windows-mingw-gcc gmake
