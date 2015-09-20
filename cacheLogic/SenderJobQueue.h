//
// Created by Dusan Klinec on 20.09.15.
//

#ifndef OPENSIPS_1_11_2_TLS_SENDERJOBQUEUE_H
#define OPENSIPS_1_11_2_TLS_SENDERJOBQUEUE_H

#include "logic.h"

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
typedef struct t_senderQueueJob_ {
    // List structure.
    t_senderQueueJob * next;
    t_senderQueueJob * prev;

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

}t_senderQueueJob, *t_pSenderQueueJob;

// Sender thread job queue, to be allocated on shared memory.
class SenderJobQueue
{
private:
    //
public:
    //Mutex to protect access to the queue
    boost::interprocess::interprocess_mutex      mutex;

    //Condition to wait for a new queue job.
    boost::interprocess::interprocess_condition  cond_newjob;

    //Is there any message
    int queue_size;

    // Flag indicating whether queue is working.
    volatile bool queue_working;

    // Job queue list.
    t_senderQueueJob * queue_head;
    t_senderQueueJob * queue_tail;

    /**
     * Default constructor.
     */
    SenderJobQueue() :
            queue_working{1},
            queue_size{0},
            queue_head{NULL},
            queue_tail{NULL}
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
    void signalAll(){
        scoped_lock<interprocess_mutex> lock(this->mutex);
        this->cond_newjob.notify_all();
        return 0;
    }
};


#endif //OPENSIPS_1_11_2_TLS_SENDERJOBQUEUE_H
