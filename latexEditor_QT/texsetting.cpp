#include "texsetting.h"

texSetting::texSetting(const QString & organization,
                       const QString & application,
                       QObject * parent):
    QSettings(organization,application,parent)
{

}

texSetting::texSetting(const QString &fileName
                       , QSettings::Format format
                       , QObject *parent):
    QSettings(fileName,format,parent)
{

}

texSetting::~texSetting() {
}

QVariant texSetting::value(const QString &key, const QVariant &defaultValue) {
  return QSettings::value(key, defaultValue);
}

bool texSetting::boolValue(const QString &key, bool defaultValue) {
  return QSettings::value(key, defaultValue).toBool();
}

bool texSetting::contains(const QString &key)
{
    return QSettings::contains(key);
}

void texSetting::setValue(const QString &key, const QVariant &value) {

  // change the setting and emit a changed signal
  // (we are not checking if the value really changed before emitting for simplicity)
  QSettings::setValue(key, value);
  emit settingChanged(key);
}

void texSetting::setValueSilent(const QString &key, const QVariant &value) {

  // change the setting and emit a changed signal
  // (we are not checking if the value really changed before emitting for simplicity)
  QSettings::setValue(key, value);
  emit settingChanged(key);
}

void texSetting::setValueIfNotSet(const QString &key, const QVariant &value) {

  // change the setting and emit a changed signal
  if( !QSettings::contains(key) ) {
    QSettings::setValue(key, value);
    // (we are not checking if the value really changed before emitting for simplicity)
    emit settingChanged(key);
  }
}
