#include <mutex>

#include "pipeline.h"

using namespace std;

Pipeline::Pipeline(void* (*f)(), initializer_list<void* (*)(void*)> f_list): pipeline_links{}, stages{} {
    for (pipeline_link &pl : pipeline_links )
        pl.has_data = false;
    stages.push_back(new FirstStage(f, pipeline_links[0]));
    int i{0};
    for (auto fn : f_list) {
        stages.push_back(new Stage(fn, pipeline_links[i], pipeline_links[i+1], i+1));
        ++i;
    }
}

Pipeline::~Pipeline() {
    for (auto stage : stages)
        delete stage;
}
