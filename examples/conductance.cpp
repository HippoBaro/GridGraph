//
// Created by Hippolyte Barraud on 5/5/18.
//

#include "core/graph.hpp"
#include <algorithm>
#include <numeric>
#include <iostream>

enum class color {
    black,
    red,
    other
};

int main(int argc, char const *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "usage: conductance [path] [memory budget in GB] [thread number]\n");
        exit(-1);
    }

    std::string path = argv[1];

    auto threads = std::atoi(argv[3]);
    printf("Thread number = %d\n", threads);
    omp_set_num_threads(threads);

    Graph graph(path);
    graph.set_memory_bytes(std::atoi(argv[2]) * std::giga::num);

    std::vector<VertexId> black_count(threads, 0);
    std::vector<VertexId> red_count(threads, 0);

    uint64_t other_count = graph.stream_edges<uint64_t>([&](Edge& e, int thread_id) {
        switch (e.source % 2 && e.target % 2 ? color::black : !(e.source % 2) && !(e.target % 2) ? color::red : color::other) {
            case color::other:
                return 1;
            case color::black:
                black_count[thread_id]++;
                break;
            case color::red:
                red_count[thread_id]++;
                break;
        }
        return 0;
    });

    VertexId black_count_total = std::accumulate(black_count.begin(), black_count.end(), 0);
    VertexId red_count_total =  std::accumulate(red_count.begin(), red_count.end(), 0);

    double conductance = other_count / (static_cast<double>(std::min(black_count_total, red_count_total)) + other_count);
    printf("conductance: %lf\n", conductance);

    return 0;
}