//
// Created by Dusan Klinec on 22.09.15.
//

#ifndef OPENSIPS_1_11_2_TLS_APIDEPS_H
#define OPENSIPS_1_11_2_TLS_APIDEPS_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>

// Hackery aparatus to include c++ incompatible c headers.
#define class xclass
#define delete xdelete
//#define lock xlock
//#define unlock xunlock
#pragma GCC diagnostic push
#pragma GCC diagnostic warning "-fpermissive"
#pragma GCC diagnostic warning "-pedantic"
#pragma GCC diagnostic warning "-w"

#include "../../../config.h"
#include "../../../dprint.h"
//#include "../../../ut.h"
#include "../../../db/db.h"
#include "../../../resolve.h"
#include "../../../mod_fix.h"
#include "../../tm/t_hooks.h"

#pragma GCC diagnostic pop
#undef class
#undef delete
#undef lock
#undef unlock

#endif //OPENSIPS_1_11_2_TLS_APIDEPS_H
