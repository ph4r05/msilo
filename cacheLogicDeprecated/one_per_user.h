//
// Created by Dusan Klinec on 16.09.15.
//

#ifndef OPENSIPS_1_11_2_TLS_ONE_PER_USER_H
#define OPENSIPS_1_11_2_TLS_ONE_PER_USER_H

#include "common.h"
#include "message_cache.h"

// Sender job queue element.
typedef struct t_senderQueueJob_ {
    // Sequential job id.
    unsigned long jobId;
    // Time of job creation.
    time_t jobCreated;
    // Type of the job.
    int type;
    // Aux user data.
    void *userData;

    // body of the request.
    union body {
        // Send message struct
        struct sendMsg {
            str receiver;
            str sender;
            int fromRegistration;
        };
    };

}t_senderQueueJob, *t_pSenderQueueJob;




#endif //OPENSIPS_1_11_2_TLS_ONE_PER_USER_H
