//
// Created by konstantin on 25.06.24.
//

#include <benchmark/benchmark.h>

int main(int argc, char** argv) {
    ::benchmark::Initialize(&argc, argv);
    ::benchmark::RunSpecifiedBenchmarks();
    return 0;
}