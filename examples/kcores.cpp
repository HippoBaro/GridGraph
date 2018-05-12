//
// Created by Hippolyte Barraud on 5/6/18.
//

#include "core/graph.hpp"

int main(int argc, char const *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "usage: k-cores [path] [k] [max iteration] [memory budget in GB] [thread number]\n");
        exit(-1);
    }

    std::string path = argv[1];
    int k = atoi(argv[2]);
    int max_it = atoi(argv[3]);

    Graph graph(path);
    graph.set_memory_bytes(std::atoi(argv[4])*std::giga::num);

    auto threads = std::atoi(argv[5]);
    printf("Thread number = %d\n", threads);
    omp_set_num_threads(threads);

    Bitmap* active_in = graph.alloc_bitmap();
    Bitmap* active_out = graph.alloc_bitmap();
    BigVector<VertexId> degree(graph.path + "/degree", graph.vertices);
    graph.set_vertex_data_bytes(graph.vertices * sizeof(VertexId));

    active_out->fill();

    VertexId to_remove;
    VertexId to_remove_old = -1;

    for (auto iter = 0; iter < max_it; iter++) {
        degree.fill(0);
        std::swap(active_in, active_out);
        active_out->clear();
        graph.hint(degree);
        graph.stream_edges<VertexId>([&](Edge& e) {
            if (active_in->get_bit(e.target)) {
                write_add(&degree[e.source], 1);
                write_add(&degree[e.target], 1);
            }
            return 0;
        }, active_in);

        to_remove = graph.stream_vertices<VertexId>([&](VertexId i){
            if (degree[i] <= k ) {
                return 1;
            }
            active_out->set_bit(i);
            return 0;
        });

        if (to_remove == to_remove_old) break;

        printf("deleted: %d\n", to_remove - to_remove_old);
        to_remove_old = to_remove;
    }
    return 0;
}