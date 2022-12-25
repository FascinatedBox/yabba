#ifndef YSETTINGS_H
# define YSETTINGS_H
# include <QObject>

#define SETTER_FN(fieldname, v) \
{ \
    if (fieldname != v) \
        _updated = true; \
\
    fieldname = v; \
}

class YSettings : public QObject
{
    Q_OBJECT

public:
    YSettings();

    static YSettings *loadSettings();
    void saveSettingsIfChanged();

    void setChannel(QString c) { SETTER_FN(_channel, c); }
    void setPassword(QString p) { SETTER_FN(_password, p); }
    void setTemp_ld(QString t) { SETTER_FN(_temp_ld, t); }
    void setUsername(QString u) { SETTER_FN(_username, u); }

    QString channel() { return _channel; }
    QString password() { return _password; }
    QString temp_ld() { return _temp_ld; }
    QString username() { return _username; }

private:
    void buildSettingsObject(QJsonObject &);

    bool _updated;
    QString _channel;
    QString _password;
    QString _temp_ld;
    QString _username;
};

#endif
