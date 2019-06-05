#include "httprequest.h"

httpRequest::httpRequest(const QString url)
{
    m_manager = new QNetworkAccessManager();
    connect(m_manager, &QNetworkAccessManager::finished,
            this, &httpRequest::getReply
            , (Qt::ConnectionType)(Qt::AutoConnection | Qt::UniqueConnection));
    //m_url="http://127.0.0.1:3434/mathjax";
    m_url=url;
//    QNetworkRequest request(m_url);
//    m_manager->get(request);
}

void httpRequest::math2svg(QString math)//, int poz, QTemporaryFile* file,bool isinline)
{
    //processing=true;
//    m_inline=isinline;
//    m_lastpoz=poz;
//    m_lastfile=file;

    qDebug() << math;
    QNetworkRequest request(m_url);
    QJsonObject json;
    json.insert("data", math);
    QJsonDocument jsonDoc(json);
    QByteArray jsonData = jsonDoc.toJson();
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");
    request.setHeader(QNetworkRequest::ContentLengthHeader,QByteArray::number(jsonData.size()));
    //qDebug() << request.url();
    m_manager->post(request,jsonData);
//    int times;
//    while(processing){
//        if(times++%6)
//            QThread::msleep(50);
//        else{
//            m_manager->post(request,jsonData);
//        }
//    }
    //get:
    //request.setUrl(QUrl(m_url+"?data="+math));
    //m_manager->get(request);
    qDebug() << "send finished";
    //qDebug() << jsonData <<m_currentSvg;
    //return m_currentSvg;
}

void httpRequest::getReply(QNetworkReply *reply)
{
    //qDebug() << reply;
    qDebug() << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (reply->error()) {
        qDebug() << reply->errorString();
        //processing=false;
        return;
    }

    QJsonDocument document = QJsonDocument::fromJson(reply->readAll());
    QJsonObject rootObj = document.object();

    if(rootObj.empty()||!rootObj.contains("svg")) {
        qDebug() << "error" << rootObj.keys();
        //processing=false;
        return;
    }
    else {
        m_currentSvg = rootObj["svg"].toString().toUtf8();
        emit displaySvg(m_currentSvg);
        //processing=false;
        //qDebug() << rootObj["svg"].toString();
    }
    //processing=false;
    return;
}

httpRequest::~httpRequest()
{
    delete m_manager;
}
