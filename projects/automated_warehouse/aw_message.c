#include "projects/automated_warehouse/aw_message.h"
#include <stdbool.h>

void initMessageBox(struct message_box *box){
    box->dirtyBit = 0;
}

bool sendMessage(struct message_box *box, struct message *msg){
    if(checkMSGBox(box)){
        return false;
    }
    box->dirtyBit = 1;
    box->msg = *msg;
    return true;
}

bool checkMSGBox(struct message_box *box){
    if(box->dirtyBit = 1){
        return true;
    }else{
        return false;
    }
}

struct message hasReceivedMSG(struct message_box *box){
    if(checkMSGBox(box)){
        box->dirtyBit = 0;
        return box->msg;
    }
}