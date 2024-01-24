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
// D00MSDAYY
// 8888888b.   .d8888b.   .d8888b.  888b     d888  .d8888b.  8888888b.        d8888 Y88b   d88P Y88b   d88P Y88b   d88P 
// 888  "Y88b d88P  Y88b d88P  Y88b 8888b   d8888 d88P  Y88b 888  "Y88b      d88888  Y88b d88P   Y88b d88P   Y88b d88P  
// 888    888 888    888 888    888 88888b.d88888 Y88b.      888    888     d88P888   Y88o88P     Y88o88P     Y88o88P   
// 888    888 888    888 888    888 888Y88888P888  "Y888b.   888    888    d88P 888    Y888P       Y888P       Y888P    
// 888    888 888    888 888    888 888 Y888P 888     "Y88b. 888    888   d88P  888     888         888         888     
// 888    888 888    888 888    888 888  Y8P  888       "888 888    888  d88P   888     888         888         888     
// 888  .d88P Y88b  d88P Y88b  d88P 888   "   888 Y88b  d88P 888  .d88P d8888888888     888         888         888     
// 8888888P"   "Y8888P"   "Y8888P"  888       888  "Y8888P"  8888888P" d88P     888     888         888         888     
 