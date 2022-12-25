#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

#include "ysettings.h"

#define YABBA_DIR ".config/yabba/"
#define YABBA_JSON_PATH (QDir::homePath() + ("/" YABBA_DIR "yabba.json"))

YSettings::YSettings()
{
    _updated = false;
}

YSettings *YSettings::loadSettings()
{
    QFile f(YABBA_JSON_PATH);

    if (f.open(QIODevice::ReadOnly) == false)
        return new YSettings;

    QByteArray ba = f.readAll();
    QJsonDocument d(QJsonDocument::fromJson(ba));
    QJsonObject o = d.object();

    YSettings *s = new YSettings;

    s->_channel = o["channel"].toString("");
    s->_password = o["password"].toString("");
    s->_temp_ld = o["temp.ld"].toString("");
    s->_username = o["username"].toString("");

    f.close();
    return s;
}

void YSettings::buildSettingsObject(QJsonObject &o)
{
    o["channel"] = _channel;
    o["password"] = _password;
    o["temp.ld"] = _temp_ld;
    o["username"] = _username;
}

void YSettings::saveSettingsIfChanged()
{
    if (_updated == false)
        return;

    QFile f(YABBA_JSON_PATH);

    if (f.open(QIODevice::WriteOnly) == false) {
        /* Maybe the config dir hasn't been made yet. */
        QDir configDir = QDir(QDir::homePath());

        if (configDir.exists(YABBA_DIR) == true ||
            configDir.mkdir(YABBA_DIR) == false ||
            f.open(QIODevice::WriteOnly) == false)
            /* Out of ideas. Give up on saving prefs. */
            return;
    }

    QJsonObject o;

    buildSettingsObject(o);
    f.write(QJsonDocument(o).toJson());
    f.close();
}
