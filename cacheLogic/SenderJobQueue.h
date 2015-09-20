//
// Created by Dusan Klinec on 20.09.15.
//

#ifndef OPENSIPS_1_11_2_TLS_SENDERJOBQUEUE_H
#define OPENSIPS_1_11_2_TLS_SENDERJOBQUEUE_H

#include "logic.h"
#include "SipsSHMAllocator.hpp"

/**
 * Type of the job queued to worker queue.
 */
typedef enum t_job_queue_type {
    JOB_TYPE_LAMBDA=0,
    JOB_TYPE_SEND_RECEIVER,
    JOB_TYPE_SEND_RECEIVER_SENDER,
    JOB_TYPE_CLEAN,
}t_job_queue_type;

/**
 * Sender job queue element. Has to be allocated on shared memory.
 */
class SenderQueueJob {
public:
    // List structure.
    SenderQueueJob * next;
    SenderQueueJob * prev;

    // Sequential job id.
    unsigned long jobId;

    // Time of job creation.
    time_t jobCreated;

    // Type of the job.
    t_job_queue_type type;

    // Aux user data, optional.
    void *userData;

    // Lambda function to be executed - general job.
    std::function<void(void)> lambda;

    // body of the request.
    union body {
        // Send message struct
        struct send {
            ShmString * receiver;
            ShmString * sender;
            int fromRegistration;
        };

    };

};

// Allocator for jobs.
typedef SipsSHMAllocator<SenderQueueJob> jobAllocator;

// Sender thread job queue, to be allocated on shared memory.
class SenderJobQueue
{
private:
    //
public:
    // Allocator for allocating jobs.
    jobAllocator allocator;

    //Mutex to protect access to the queue
    boost::interprocess::interprocess_mutex      mutex;

    //Condition to wait for a new queue job.
    boost::interprocess::interprocess_condition  cond_newjob;

    //Is there any message
    int queue_size;

    // Flag indicating whether queue is working.
    volatile bool queue_working;

    // Job queue list.
    SenderQueueJob * queue_head;
    SenderQueueJob * queue_tail;

    /**
     * Default constructor.
     */
    SenderJobQueue(jobAllocator alloc = {}) :
            queue_working{1},
            queue_size{0},
            queue_head{NULL},
            queue_tail{NULL},
            allocator{alloc}
    {}

    /**
     * Returns true if empty.
     */
    bool isEmpty(){
        return this->queue_size == 0;
    }

    /**
     * Broadcasts signal to all threads using condition variable.
     * Used when some queue threads are about to terminate.
     */
    int signalAll(){
        scoped_lock<interprocess_mutex> lock(this->mutex);
        this->cond_newjob.notify_all();
        return 0;
    }

    /**
     * Allocates & creates a new job.
     * This must be the only way jobs are being created.
     */
    SenderQueueJob * newJob(){
        SenderQueueJob * job = allocator.allocate(1, NULL);
        allocator.construct(job, SenderQueueJob());
        return job;
    }

    /**
     * Destroys the job using internal allocator.
     * This must be the only way jobs are being destroyed.
     */
    void destroyJob(SenderQueueJob *job){
        allocator.destroy(job);
        allocator.deallocate(job, 1);
    }

    /**
     * Returns first element in the queue.
     * Unsafe version, user have to lock the queue when pop-ing().
     */
    SenderQueueJob * popFront(){
        if (this->isEmpty() || this->queue_head == NULL){
            return NULL;
        }

        SenderQueueJob * ret = this->queue_head;
        this->queue_head = ret->next;

        // Was this only one element?
        if (this->queue_tail == ret){
            this->queue_tail = NULL;
        }

        // If there is a next element, update his pointers.
        if (this->queue_head){
            this->queue_head->prev = NULL;
        }

        ret->prev = NULL;
        ret->next = NULL;
        --this->queue_size;
        return ret;
    }

    /**
     * Inserts a new job to the job queue.
     * Unsafe version, user have to lock the queue.
     */
    void pushBack(SenderQueueJob * job){
        // If head is empty, add as head.
        if (this->queue_head == NULL){
            this->queue_head = job;
        }

        job->next = NULL;
        job->prev = this->queue_tail;
        this->queue_tail = job;

        if (job->prev != NULL){
            job->prev->next = job;
        }

        ++this->queue_size;
    }

    /**
     * Locks the queue and pushbacks.
     */
    void lockAndPushBack(SenderQueueJob * job){
        scoped_lock<interprocess_mutex> lock(this->mutex);
        this->pushBack(job);
    }
};


#endif //OPENSIPS_1_11_2_TLS_SENDERJOBQUEUE_H
