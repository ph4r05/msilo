//
// Created by Dusan Klinec on 20.09.15.
//

#ifndef OPENSIPS_1_11_2_TLS_MESSAGETHREADSENDER_H
#define OPENSIPS_1_11_2_TLS_MESSAGETHREADSENDER_H

#define SENDER_THREAD_NUM 1
#define SENDER_THREAD_WAIT_MS 100

#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/sync/interprocess_condition.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <memory>
#include <thread>
#include <string>
#include "SenderJobQueue.hpp"
#include "MessageThreadManager.hpp"

/**
 * Main method of the sender thread.
 * Allocate only on heap!
 */
class SenderThreadWorker {
public:
    // For synchronization, sender pointer for callbacks from worker queue.
    std::weak_ptr<MessageThreadSender> wSender;
    std::weak_ptr<MessageThreadManager> wThreadMgr;

    // Constructor.
    SenderThreadWorker(const std::weak_ptr<MessageThreadSender> &sender,
                       const std::weak_ptr<MessageThreadManager> &tMgr) : wSender(sender), wThreadMgr(tMgr) { }

    // Main worker method.
    void operator()(senderThreadArg * arg) {
        std::cout << "functor\n";
        LM_DBG("Sender thread %d in rank %d started\n", arg->threadId, arg->rank);

        // Work loop.
        while(1){
            // Get strong pointer to the sender manager, determine if running the thread is allowed.
            auto senderPt = wSender.lock();
            auto mgrPt = wThreadMgr.lock();
            if (!senderPt || !mgrPt){
                // Deallocated menawhile.
                break;
            }

            // Still running?
            if (!senderPt->senderThreadsRunning){
                break;
            }

            // Get job queue.
            SenderJobQueue * queue = senderPt->getQueue();

            t_senderQueueJob * job = NULL;
            int signaled = 0;

            // <critical_section> monitor queue, poll one job from queue.
            queue->mutex.lock();
            senderThreadWaiters += 1;
            {
                // Maximum wait time in condition wait is x seconds so we dont deadlock (soft deadlock).
                const boost::system_time timeout = boost::get_system_time() + milliseconds(SENDER_THREAD_WAIT_MS);

                // If queue is empty, wait for insertion signal.
                if (queue->isEmpty()) {
                    // Wait signaling, note mutex is atomically unlocked while waiting.
                    // CPU cycles are saved here since thread blocks while waiting for new jobs.
                    signaled = queue->cond_newjob.timed_wait(queue->mutex, timeout);
                    senderThreadWaiters -= 1;
                }

                // Check job queue again.
                job = queue->popFront();
            }
            queue->mutex.unlock();
            // </critical_section>

            // If signaling ended with command to quit.
            if (!senderPt->senderThreadsRunning){
                break;
            }

            // Job may be nil. If is, continue with waiting.
            if (job == nullptr){
                continue;
            }

            // Execute block here, in try-catch to protect executor from fails.
            try {
                switch (job->type) {
                    case JOB_TYPE_LAMBDA:
                        if (job->lambda != nullptr){
                            job->lambda();
                        }
                        break;

                    case JOB_TYPE_SEND_RECEIVER:
                        this->send1(job, senderPt, mgrPt);
                        break;

                    case JOB_TYPE_SEND_RECEIVER_SENDER:
                        this->send2(job, senderPt, mgrPt);
                        break;

                    case JOB_TYPE_CLEAN:
                        // TODO: implement.
                        break;

                    default:
                        break;
                }
            } catch (...) {
                cout << "default exception";
            }

            // Deallocate job, using shared memory allocator.
            queue->destroyJob(job);
        }

        LM_DBG("Sender thread %d in rank %d finished\n", arg->threadId, arg->rank);
    }

    // TODO: implement.
    void send1(t_senderQueueJob * job, MessageThreadSender * sender, MessageThreadManager * manager){

    }

    // TODO: implement.
    void send2(t_senderQueueJob * job, MessageThreadSender * sender, MessageThreadManager * manager){

    }

};

/**
 * Manager of threads for sending messages.
 * To be allocated on heap, per process worker.
 */
class MessageThreadSender {
private:
    static volatile int senderThreadsRunning;
    static int senderThreadWaiters;

    std::shared_ptr<MessageThreadSender> sSelf;

    // Job queue here, allocated on SHM, needs to be added on initialization of this sender.
    SenderJobQueue * queue;

    // Sender threads themselves.
    std::thread senderThreads[SENDER_THREAD_NUM];

    // Sender thread arguments.
    t_senderThreadArg senderThreadsArgs[SENDER_THREAD_NUM];

    // Worker objects, executing jobs from the queue.
    SenderThreadWorker workers[SENDER_THREAD_NUM];

public:
    /**
     * Returns job queue.
     */
    SenderJobQueue *getQueue() const {
        return queue;
    }

    // Sets job queue.
    void setQueue(SenderJobQueue *queue) {
        MessageThreadSender::queue = queue;
    }

    /**
     * Queue constuctor.
     */
    MessageThreadSender(SenderJobQueue *queue) :
            queue{queue},
            senderThreadsRunning{1},
            senderThreadWaiters{0},
            sSelf(this)
    {

    }

    /**
     * Default constructor, using null queue.
     */
    MessageThreadSender() : MessageThreadSender(NULL) {}

    /**
     * Called when server starts so shared variables for all processes are allocated and initialized.
     */
    int initSenderThreads(){
        senderThreadsRunning = 1;
        senderThreadWaiters = 0;
        return 0;
    }

    /**
     * Terminates running sender threads.
     */
    int destroySenderThreads(){
        this->terminateSenderThreads();
        return 0;
    }

    /**
     * Called when worker process is initialized. Spawns sender threads.
     */
    int initChildSenderThreads(){
        return this->spawnSenderThreads();
    }

    /**
     * Routine starts given amount of sender threads.
     */
    int spawnSenderThreads(){
        int t = 0;
        int rc = 0;
        this->senderThreadsRunning = 1;

        for(t = 0; t < SENDER_THREAD_NUM && rc == 0; t++){
            try {
                this->workers[t](std::weak_ptr<MessageThreadSender>(this));
                this->senderThreadsArgs[t].rank = t;
                this->senderThreads[t](this->workers[t], this->senderThreadsArgs[t]);

            } catch(const std::system_error& e){
                LM_ERR("Exception in creating a thread, code: %d, info: %s", e.code().value(), e.what());
                rc = -1;
            }
        }

        // Check for fails, if any, terminate sender.
        if (rc){
            this->terminateSenderThreads();
            return -1;
        }

        return 0;
    }

    /**
     * Sets all sender threads to terminate.
     */
    int terminateSenderThreads(){
        // Running flag set to false.
        senderThreadsRunning = 0;
        if (this->queue != NULL){
            this->queue->signalAll();
        }

        return 0;
    }
};


#endif //OPENSIPS_1_11_2_TLS_MESSAGETHREADSENDER_H
