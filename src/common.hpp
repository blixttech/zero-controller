#pragma once

#include <QMap>
#include <QString>
#include <QVariant>


namespace zero {

    typedef QMap<QString, QVariant> Config;

}


#define GET_CONF(var, config)               var.value(CONF_KEY_##config, CONF_DEF_##config)

#define CONF_KEY_LOG_LEVEL                  "log/level"
#define CONF_DEF_LOG_LEVEL                  4 // 1:fatal, 2:critical, 3:warning, 4:info, 5:debug  

#define CONF_KEY_CFG_FILE                   "files/config"
#define CONF_DEF_CFG_FILE                   "config.ini"
#define CONF_KEY_SAVE_FILE_DIR              "files/data_dir"
#define CONF_DEF_SAVE_FILE_DIR              "data"

#define CONF_KEY_SEARCH_INT                 "search/interval"
#define CONF_DEF_SEARCH_INT                 1

#define CONF_KEY_UPDATE_INT                 "update/interval"
#define CONF_DEF_UPDATE_INT                 100

