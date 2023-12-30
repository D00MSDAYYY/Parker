#pragma once

enum class FSM               // FSM states
{
    USER_NOT_AUTHORIZED = 0,  // starting
    USER_AUTHORIZED,          // USER_AUTHORIZED
    USER_INFO_HELP,          // information and help
    USER_CHNG_BIO,           // change info about user
    USER_RQST_PRKNG,         // requesting for parking
    USER_CNCL_RQST,          // cancelling request for parking
    USER_SBMT_RQST,          // submiting request for parking
    USER_PRKNG_VLDTN,        //!

    ADMIN,                   // only for admins to show special commands
    ADMIN_ADD_RM_USER,   // admin add/removes user
    ADMIN_CHNG_MAIL,         // admin changes mail to send report
    ADMIN_CHNG_NUM           // admin changes num of parking place
};
