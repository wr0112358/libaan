#!/bin/bash

function clean
{
    rm -rf .build_gtest
}

function build
{
    mkdir .build_gtest
    cd .build_gtest
    wget https://googletest.googlecode.com/files/gtest-1.7.0.zip
    unzip gtest-1.7.0.zip
    cd gtest-1.7.0
    echo "Running: ./configure CXX=$GTEST_CXX CXXFLAGS=$GTEST_CXXFLAGS"
    ./configure CXX="$GTEST_CXX" CXXFLAGS="$GTEST_CXXFLAGS"
    make all
}

[ $# -ne 1 ] && exit 1

case "$1" in
    "clean")
	clean;;
    "build")
	build;;
esac
