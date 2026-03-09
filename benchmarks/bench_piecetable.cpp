#include "core/PieceTable.h"
#include <benchmark/benchmark.h>

using namespace lighttex::core;

static void BM_InsertAtEnd(benchmark::State& state) {
    for (auto _ : state) {
        PieceTable pt;
        for (int i = 0; i < state.range(0); ++i) {
            pt.insert(pt.length(), "x");
        }
        benchmark::DoNotOptimize(pt.length());
    }
}
BENCHMARK(BM_InsertAtEnd)->Range(100, 10000);

static void BM_InsertAtBeginning(benchmark::State& state) {
    for (auto _ : state) {
        PieceTable pt;
        for (int i = 0; i < state.range(0); ++i) {
            pt.insert(0, "x");
        }
        benchmark::DoNotOptimize(pt.length());
    }
}
BENCHMARK(BM_InsertAtBeginning)->Range(100, 10000);

static void BM_InsertAtRandom(benchmark::State& state) {
    for (auto _ : state) {
        PieceTable pt("Hello World");
        for (int i = 0; i < state.range(0); ++i) {
            size_t pos = static_cast<size_t>(i) % (pt.length() + 1);
            pt.insert(pos, "x");
        }
        benchmark::DoNotOptimize(pt.length());
    }
}
BENCHMARK(BM_InsertAtRandom)->Range(100, 10000);

static void BM_Text(benchmark::State& state) {
    PieceTable pt;
    for (int i = 0; i < state.range(0); ++i) {
        pt.insert(pt.length(), "Hello World\n");
    }
    for (auto _ : state) {
        auto text = pt.text();
        benchmark::DoNotOptimize(text.data());
    }
}
BENCHMARK(BM_Text)->Range(100, 10000);

static void BM_CharToLineCol(benchmark::State& state) {
    std::string content;
    for (int i = 0; i < 1000; ++i) {
        content += "This is line " + std::to_string(i) + "\n";
    }
    PieceTable pt(content);

    for (auto _ : state) {
        auto [line, col] = pt.charToLineCol(content.size() / 2);
        benchmark::DoNotOptimize(line);
        benchmark::DoNotOptimize(col);
    }
}
BENCHMARK(BM_CharToLineCol);

static void BM_DeleteInsert(benchmark::State& state) {
    std::string base(1000, 'a');
    for (auto _ : state) {
        PieceTable pt(base);
        for (int i = 0; i < state.range(0); ++i) {
            size_t pos = static_cast<size_t>(i) % pt.length();
            pt.erase(pos, 1);
            pt.insert(pos, "b");
        }
        benchmark::DoNotOptimize(pt.length());
    }
}
BENCHMARK(BM_DeleteInsert)->Range(100, 5000);

BENCHMARK_MAIN();
