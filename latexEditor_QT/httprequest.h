#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H

#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonDocument>
#include <QTemporaryFile>
#include <QThread>

class httpRequest : public QObject
{
    Q_OBJECT
public:
    explicit httpRequest(const QString url);
    virtual ~httpRequest();

    void math2svg(QString math);

signals:
    void displaySvg(QByteArray svg);
private:
    void getReply(QNetworkReply *reply);

    QString m_url;

    QNetworkAccessManager *m_manager;
    //QNetworkRequest m_request;

    //bool processing=false;
//    int m_lastpoz;
//    QTemporaryFile* m_lastfile;
//    bool m_inline;
    QByteArray m_currentSvg;
};

#endif // HTTPREQUEST_H
