//
// Created by Hippolyte Barraud on 5/5/18.
//

#include "core/graph.hpp"

enum class color {
    black,
    red,
    other
};

int main(int argc, char const *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "usage: conductance [path] [memory budget in GB]\n");
        exit(-1);
    }

    std::string path = argv[1];

    Graph graph(path);
    graph.set_memory_bytes(std::atoi(argv[2]) * std::giga::num);

    VertexId black_count = 0;
    VertexId red_count = 0;

    uint64_t other_count = graph.stream_edges<uint64_t>([&](Edge& e) {
        switch (e.source % 2 && e.target % 2 ? color::black : !(e.source % 2) && !(e.target % 2) ? color::red : color::other) {
            case color::other:
                return 1;
            case color::black:
                write_add(&black_count, 1);
                break;
            case color::red:
                write_add(&red_count, 1);
                break;
        }
        return 0;
    });

    double conductance = other_count / (static_cast<double>(std::min(black_count, red_count)) + other_count);
    printf("conductance: %lf\n", conductance);

    return 0;
}