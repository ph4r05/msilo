//
// Created by Dusan Klinec on 18.09.15.
//

#ifndef OPENSIPS_1_11_2_TLS_MESSAGECACHEELEMENT_H
#define OPENSIPS_1_11_2_TLS_MESSAGECACHEELEMENT_H

/**
 * Structure holding loaded message from the database to be sent.
 * Linked list element. Used in message cache, in message send ctl class.
 */
class MessageListElement {
private:
    int msgid;
    int flag;
    int retry_ctr; // Retry counter for failed message.

    // Message reconstruction, for sending.
    ShmString msg_from;
    ShmString msg_to;
    ShmString msg_body;
    ShmString msg_ctype;
    time_t msg_rtime;

    // List elements, double linking.
    MessageListElement * prev;
    MessageListElement * next;

public:
    MessageListElement() :
            msgid{-1},
            flag{0},
            retry_ctr{0},
            prev{NULL},
            next{NULL}
    { }
};


#endif //OPENSIPS_1_11_2_TLS_MESSAGECACHEELEMENT_H
