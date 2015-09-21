//
// Created by Dusan Klinec on 20.09.15.
//

#ifndef OPENSIPS_1_11_2_TLS_MESSAGETHREADSENDER_H
#define OPENSIPS_1_11_2_TLS_MESSAGETHREADSENDER_H

#define SENDER_THREAD_NUM 1
#define SENDER_THREAD_WAIT_MS 100

#include <memory>
#include <thread>
#include <string>
#include <system_error>
#include <boost/date_time/time.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread/thread_time.hpp>
#include "logic.hpp"
#include "SenderJobQueue.hpp"
#include "MessageThreadManager.hpp"

// Forward declaration, circular dependencies.
class MessageThreadManager;
class MessageThreadSender;

/**
 * Thread argument.
 */
class SenderThreadArg {
public:
    int rank;
    int threadId;
};

/**
 * Main method of the sender thread.
 * Allocate only on heap!
 */
class SenderThreadWorker {
public:
    // Typedef for std::thread
    typedef void type;

    // For synchronization, sender pointer for callbacks from worker queue.
    std::weak_ptr<MessageThreadSender> wSender;
    std::weak_ptr<MessageThreadManager> wThreadMgr;

    // Parameter.
    SenderThreadArg * arg;

    // Constructor.
    SenderThreadWorker() { }

    /**
     * Main worker method.
     */
    void operator()() const{
        this->work(this->arg);
    }

    void work(SenderThreadArg * arg) const;

    void send1(SenderQueueJob * job, MessageThreadSender * sender, MessageThreadManager * manager) const;

    void send2(SenderQueueJob * job, MessageThreadSender * sender, MessageThreadManager * manager) const;

    const std::weak_ptr<MessageThreadSender> &getWSender() const {
        return wSender;
    }

    void setWSender(const std::shared_ptr<MessageThreadSender> &sender) {
        this->wSender = sender;
    }

    const std::weak_ptr<MessageThreadManager> &getWThreadMgr() {
        return wThreadMgr;
    }

    void setWThreadMgr(const std::shared_ptr<MessageThreadManager> & mgr) {
        this->wThreadMgr = mgr;
    }

    SenderThreadArg *getArg() const {
        return arg;
    }

    void setArg(SenderThreadArg *arg) {
        SenderThreadWorker::arg = arg;
    }
};

/**
 * Manager of threads for sending messages.
 * To be allocated on heap, per process worker.
 */
class MessageThreadSender {
private:
    std::shared_ptr<MessageThreadManager> mgr;
    std::shared_ptr<MessageThreadSender> sSelf;

    // Job queue here, allocated on SHM, needs to be added on initialization of this sender.
    SenderJobQueue * queue;

    // Sender threads themselves.
    std::thread * senderThreads[SENDER_THREAD_NUM];

    // Sender thread arguments.
    SenderThreadArg senderThreadsArgs[SENDER_THREAD_NUM];

    // Worker objects, executing jobs from the queue.
    SenderThreadWorker workers[SENDER_THREAD_NUM];

public:
    volatile int senderThreadsRunning;
    int senderThreadWaiters;

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
    MessageThreadSender(MessageThreadManager * aMgr, SenderJobQueue *queue) :
            queue{queue},
            senderThreadsRunning{1},
            senderThreadWaiters{0},
            sSelf{this},
            mgr{aMgr}
    {
        for(int i=0; i<SENDER_THREAD_NUM; i++){
            this->senderThreads[i] = NULL;
        }
    }

    /**
     * Default constructor, using null queue.
     */
    MessageThreadSender() : MessageThreadSender(NULL, NULL) {}

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
    int spawnSenderThreads();

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
