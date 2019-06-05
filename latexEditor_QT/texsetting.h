#ifndef TEXSETTING_H
#define TEXSETTING_H

#include <QObject>
#include <QSettings>

/*
  userId
  password
  pandocUrl
    setting.setValue("serverUrl","localhost");
    setting.setValue("servelPort",3435);
    setting.setValue("mathJaxUrl","http://127.0.0.1:3434/mathjax");
    setting.setValue("session",session.toVariantMap());//reference?
    setting.setValue("defalut_mode",DISPLAY_VIEW);

*/

class texSetting : public QSettings
{
      Q_OBJECT
public:

    explicit texSetting(const QString & organization,
                        const QString & application = QString(),
                        QObject * parent = nullptr);
    explicit texSetting(const QString &fileName,
                        QSettings::Format format,
                        QObject *parent = nullptr);
    virtual ~texSetting();

    Q_INVOKABLE
    void setValue(const QString &key, const QVariant &value);

    Q_INVOKABLE
    void setValueSilent(const QString &key, const QVariant &value);

    Q_INVOKABLE
    bool contains(const QString &key);

    Q_INVOKABLE
    void setValueIfNotSet(const QString &key, const QVariant &value);

    Q_INVOKABLE
    QVariant value(const QString &key, const QVariant &defaultValue= QVariant());

    Q_INVOKABLE
    bool boolValue(const QString &key, const bool defaultValue );



signals:
    void settingChanged(const QString& key);
};

#endif // TEXSETTING_H
