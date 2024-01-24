//      888                                               888
//      888                                               888
//      888                                               888
//  .d88888  .d88b.   .d88b.  88888b.d88b.  .d8888b   .d88888  8888b.  888  888
// d88" 888 d88""88b d88""88b 888 "888 "88b 88K      d88" 888     "88b 888  888
// 888  888 888  888 888  888 888  888  888 "Y8888b. 888  888 .d888888 888  888
// Y88b 888 Y88..88P Y88..88P 888  888  888      X88 Y88b 888 888  888 Y88b 888
//  "Y88888  "Y88P"   "Y88P"  888  888  888  88888P'  "Y88888 "Y888888  "Y88888
//                                                                          888
//                                                                     Y8b d88P
//                                                                      "Y88P"

#pragma once

enum class FSM              // FSM states
{
    USER_BOT_NOT_WORKING,   // time then bot is not working
    USER_NOT_AUTHORIZED,    // starting
    USER_AUTHORIZED,        // user can send queries
    USER_INFO_HELP,         // information and help
    USER_CHNG_BIO,          // change info about user
    USER_RQST_PRKNG,        // requesting for parking

    ADMIN,                  // only for admins to show special commands
    ADMIN_ADD_RM_USER,      // admin add/removes user
    ADMIN_CHNG_MAIL,        // admin changes mail to send report
    ADMIN_CHNG_NUM,         // admin changes num of parking place
    ADMIN_CHNG_WKDY,        // admin changes working days when parking allowed
    ADMIN_CHNG_WRKNG_TIME,  // change active time for bot
    ADMIN_CHNG_EMAIL_TIME   // change time for email sending
};
