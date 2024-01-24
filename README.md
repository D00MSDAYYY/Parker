# **🅿️arker** is a tg-bot for parking organization
 
## User abilities : 
1. Make parking requests
1. Remove parking requests

## Admin abilities : 
1. Add/remove users to/from list of allowed ones
1. Change **number of** available parking **slots**
1. Change **weekdays** when parking is open
1. Change **time** when bot **accepting requests**
1. Change **email information** to which report is sent 
1. Change **time when report is sent**


## Build : 
To build bot you will need to download/install : 
- [ **CMake** ](https://cmake.org/download/)
- [ **PostgreSQL** ](https://www.postgresql.org/download/)
- [ **Mailio** ](https://github.com/karastojko/mailio.git) lib (if you have trouble with one which is already in "src/3rdParty" directory)
- [ **OpenXLSX** ](https://github.com/troldal/OpenXLSX.git) lib (same)
- [ **tgbot-cpp** ](https://github.com/reo7sp/tgbot-cpp.git) lib (same)

## Launch : 
Launch command: 
```
Paker YOUR_TG_BOT_TOKEN ADMIN_TG_ID
```