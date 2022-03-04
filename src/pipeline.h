#ifndef PIPELINE_H
#define PIPELINE_H

#include <vector>
#include <memory>
#include <mutex>
#include <future>
#include <condition_variable>

struct pipeline_link {
    std::mutex mtx;
    std::condition_variable cv;
    void *buffer;
    bool has_data;
};

class GenericStage {
public:
    GenericStage(pipeline_link &link_out);
    virtual ~GenericStage();

    GenericStage(const GenericStage&) = delete;
    GenericStage operator=(const GenericStage&) = delete;

protected:
    pipeline_link &link_out;
    unsigned int stage_index;
    std::thread *th;
    std::atomic<bool> running;
};

class FirstStage: public GenericStage {
public:
    FirstStage(std::function<void*()> func, pipeline_link &link_out);
    void run();

private:
    std::function<void*()> func;
};

class Stage: public GenericStage {
public:
    Stage(std::function<void*(void*)> func, pipeline_link &link_in, pipeline_link &link_out, unsigned int stage_index);
    ~Stage() = default;
    void run();

private:
    pipeline_link &link_in;
    std::function<void*(void*)> func;
};


/* A pipeline is initialized as
 * Pipeline p{f0, {f1, ..., fn}};
 * where f0 is a function (or rather a callable object) taking nothing and 
 * returning a pointer and f1-fn are functions taking pointers and returning
 * pointers */
class Pipeline {
public:
    Pipeline(std::function<void*()> f0, std::initializer_list<std::function<void*(void*)>> f_list);
    ~Pipeline();
    std::future<void*> get_future();

private:
    std::array<pipeline_link, 32> pipeline_links;
    std::vector<GenericStage*> stages;
};

#endif  // PIPELINE_H
