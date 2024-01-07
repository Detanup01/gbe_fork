#!/usr/bin/env bash

my_dir="$(cd "$(dirname "$0")" && pwd)"

pushd "$my_dir" > /dev/null

clang++ -x c++ -rdynamic -std=c++17 -fvisibility=hidden -fexceptions -fno-jump-tables -Og -g3 -fPIE -I../ ../linux.cpp ../common.cpp test_linux_sa_handler.cpp -otest_linux_sa_handler && {
    ./test_linux_sa_handler ;
    echo "exit code = $?" ;
    rm -f ./test_linux_sa_handler ;
}

clang++ -x c++ -rdynamic -std=c++17 -fvisibility=hidden -fexceptions -fno-jump-tables -Og -g3 -fPIE -I../ ../linux.cpp ../common.cpp test_linux_sa_sigaction.cpp -otest_linux_sa_sigaction && {
    ./test_linux_sa_sigaction ;
    echo "exit code = $?" ;
    rm -f ./test_linux_sa_sigaction ;
}

clang++ -m32 -x c++ -rdynamic -std=c++17 -fvisibility=hidden -fexceptions -fno-jump-tables -Og -g3 -fPIE -I../ ../linux.cpp ../common.cpp test_linux_sa_handler.cpp -otest_linux_sa_handler && {
    ./test_linux_sa_handler ;
    echo "exit code = $?" ;
    rm -f ./test_linux_sa_handler ;
}

clang++ -m32 -x c++ -rdynamic -std=c++17 -fvisibility=hidden -fexceptions -fno-jump-tables -Og -g3 -fPIE -I../ ../linux.cpp ../common.cpp test_linux_sa_sigaction.cpp -otest_linux_sa_sigaction && {
    ./test_linux_sa_sigaction ;
    echo "exit code = $?" ;
    rm -f ./test_linux_sa_sigaction ;
}

rm -f -r ./crash_test

popd > /dev/null
