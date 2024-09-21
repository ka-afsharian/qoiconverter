#!/bin/bash

cmake -B buildtree
cmake --build buildtree
cmake --install buildtree

#time ./buildtree/bin/run
