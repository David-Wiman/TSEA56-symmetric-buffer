#ifndef PIPELINE_H
#define PIPELINE_H

#include <vector>
#include <memory>
#include <mutex>
#include <condition_variable>

struct data_package {
    void *data;
    bool had_data;
};

struct pipeline_link {
    std::mutex mtx;
    std::condition_variable cv;
    void *buffer;
    bool has_data;
};

class GenericStage {
public:
    GenericStage(pipeline_link &link_out): link_out{link_out}, stage_index{} {};
    virtual ~GenericStage() {};

    GenericStage(const GenericStage&) = delete;
    GenericStage operator=(const GenericStage&) = delete;

protected:
    pipeline_link &link_out;
    unsigned int stage_index;
};

class FirstStage: public GenericStage {
public:
    FirstStage(void* (*func)(), pipeline_link &link_out);
    void run();

private:
    void* (*func)();  // A pointer to a function which takes nothing but returns a pointer to void
};

class Stage: public GenericStage {
public:
    Stage(void* (*func)(void*), pipeline_link &link_in, pipeline_link &link_out, unsigned int stage_index);
    ~Stage() = default;
    void run();

private:
    pipeline_link &link_in;
    void* (*func)(void*);  // A pointer to a function which takes and returns a pointer to void
};


/* A pipeline is initialized as
 * Pipeline p{f0, {f1, ..., fn}};
 * where f0 is a function taking nothing and returning a pointer
 * and f1-fn is functions taking pointers and returning pointers */
class Pipeline {
public:
    Pipeline(void* (*f0)(), std::initializer_list<void* (*)(void*)> f_list);
    ~Pipeline();

private:
    std::array<pipeline_link, 32> pipeline_links;
    std::vector<GenericStage*> stages;
};

#endif  // PIPELINE_H
