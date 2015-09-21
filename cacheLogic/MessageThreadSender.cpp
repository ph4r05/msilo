//
// Created by Dusan Klinec on 20.09.15.
//

#include "MessageThreadSender.hpp"
#include <boost/interprocess/smart_ptr/unique_ptr.hpp>
#include <boost/thread/locks.hpp>

void SenderThreadWorker::work(SenderThreadArg *arg) const{
    PH_DBG("Sender thread %d in rank %d started\n", arg->threadId, arg->rank);

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
        SenderQueueJob * job = NULL;
        int signaled = 0;

        // <critical_section> monitor queue, poll one job from queue.
        boost::unique_lock<bip::interprocess_mutex> ulock(queue->mutex);
        ulock.lock();
        senderPt->senderThreadWaiters += 1;
        {
            // Maximum wait time in condition wait is x seconds so we dont deadlock (soft deadlock).
            const boost::system_time timeout = boost::get_system_time() + boost::posix_time::milliseconds(SENDER_THREAD_WAIT_MS);

            // If queue is empty, wait for insertion signal.
            if (queue->isEmpty()) {
                // Wait signaling, note mutex is atomically unlocked while waiting.
                // CPU cycles are saved here since thread blocks while waiting for new jobs.
                signaled = queue->cond_newjob.timed_wait(ulock, timeout);
                senderPt->senderThreadWaiters -= 1;
            }

            // Check job queue again.
            job = queue->popFront();
        }
        ulock.unlock();
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
                case JOB_TYPE_EMPTY:
                    PH_ERR("Empty job occurred\n");
                    break;

                case JOB_TYPE_LAMBDA:
                    if (job->lambda != nullptr){
                        job->lambda();
                    }
                    break;

                case JOB_TYPE_SEND_RECEIVER:
                    this->send1(job, senderPt.get(), mgrPt.get());
                    break;

                case JOB_TYPE_SEND_RECEIVER_SENDER:
                    this->send2(job, senderPt.get(), mgrPt.get());
                    break;

                case JOB_TYPE_CLEAN:
                    // TODO: implement.
                    break;

                default:
                    break;
            }
        } catch (...) {
            PH_ERR("Exception in job processing\n");
        }

        // Deallocate job, using shared memory allocator.
        queue->destroyJob(job);
    }

    PH_DBG("Sender thread %d in rank %d finished\n", arg->threadId, arg->rank);
}

void SenderThreadWorker::send1(SenderQueueJob *job, MessageThreadSender *sender, MessageThreadManager *manager) const{
    manager->send1(job, sender);
}

void SenderThreadWorker::send2(SenderQueueJob *job, MessageThreadSender *sender, MessageThreadManager *manager) const{
    manager->send1(job, sender);
}

int MessageThreadSender::spawnSenderThreads() {
    int t = 0;
    int rc = 0;
    this->senderThreadsRunning = 1;

    for(t = 0; t < SENDER_THREAD_NUM && rc == 0; t++){
        try {
            this->workers[t].setWSender(sSelf);
            this->workers[t].setWThreadMgr(mgr);
            this->senderThreadsArgs[t].rank = t;
            if (this->senderThreads[t] != NULL){
                delete this->senderThreads[t];
                this->senderThreads[t] = NULL;
            }

            this->workers[t].setArg(&(this->senderThreadsArgs[t]));
            this->senderThreads[t] = new std::thread(this->workers[t]); //, std::ref(this->senderThreadsArgs[t]));

        } catch(const std::system_error& e){
            rc = -1;
            PH_ERR("Exception in creating a thread, code: %d, info: %s\n", e.code().value(), e.what());
        }
    }

    // Check for fails, if any, terminate sender.
    if (rc != 0){
        this->terminateSenderThreads();
        return -1;
    }

    return 0;

}
