#include "latexclient.h"
#include <QtCore/QDebug>

QT_USE_NAMESPACE

latexClient::latexClient(texSetting &setting, QObject *parent)
    : QObject(parent)
	, m_setting(setting)
    , m_userId(setting.value("userId","").toString())
    , m_docId(setting.value("docId","").toString())
	, m_valid(false)
	, m_connected(false)
	, m_docConnected(false)
    , m_update(false)
{//TODO change innerID
    connect(&setting,&texSetting::settingChanged,
            this,&latexClient::onSettingChanged
            , (Qt::ConnectionType)(Qt::AutoConnection | Qt::UniqueConnection)  );
    //qDebug()<<setting.value("userId");
    m_url=setting.value("serverUrl").toString().isEmpty()?"localhost":setting.value("serverUrl").toString();
    m_port=!setting.value("servelPort").toInt()?3435:setting.value("servelPort").toInt();

    toConnect();
}


latexClient::~latexClient()
{
	m_webSocket.close();
}

void latexClient::toConnect()
{
    connect(&m_webSocket, &QWebSocket::connected,
            this, &latexClient::onConnected
            , (Qt::ConnectionType)(Qt::AutoConnection | Qt::UniqueConnection));
    connect(&m_webSocket, QOverload<const QList<QSslError>&>::of(&QWebSocket::sslErrors),
            this, &latexClient::onSslErrors
            , (Qt::ConnectionType)(Qt::AutoConnection | Qt::UniqueConnection));

    QList<QSslCertificate> cert = QSslCertificate::fromPath(QLatin1String(":/tex.crt"));
    QSslError error1(QSslError::SelfSignedCertificate, cert.at(0));
    QSslError error2(QSslError::HostNameMismatch, cert.at(0));
    QList<QSslError> expectedSslErrors;
    expectedSslErrors.append(error1);
    expectedSslErrors.append(error2);

    m_webSocket.ignoreSslErrors(expectedSslErrors);
    m_webSocket.open(QUrl("wss://"+m_url.toString()+":"+QString::number(m_port)));
}

void latexClient::toDisconnect()
{
    socketDisconnected();
    m_webSocket.close();
}

void latexClient::onConnected()
{
    qDebug() << "WebSocket connected";
    setValid(true);
    if(!m_userId.isEmpty()){
        sendLogin();
    }
    connect(&m_webSocket, &QWebSocket::binaryMessageReceived,
            this, &latexClient::onByteMessageReceived
            , (Qt::ConnectionType)(Qt::AutoConnection | Qt::UniqueConnection));
    connect(&m_webSocket, &QWebSocket::disconnected,
            this, &latexClient::socketDisconnected
            , (Qt::ConnectionType)(Qt::AutoConnection | Qt::UniqueConnection));
}

void latexClient::socketDisconnected()
{
    setValid(false);
    setLogin(false);
    setConnected(false);
    disconnect(&m_webSocket, &QWebSocket::connected,
            this, &latexClient::onConnected);
    disconnect(&m_webSocket, QOverload<const QList<QSslError>&>::of(&QWebSocket::sslErrors),
            this, &latexClient::onSslErrors);
    disconnect(&m_webSocket, &QWebSocket::binaryMessageReceived,
            this, &latexClient::onByteMessageReceived);
    disconnect(&m_webSocket, &QWebSocket::disconnected,
            this, &latexClient::socketDisconnected);
}

void latexClient::sendLogin()
        //QString userId,QString password)
{
    QJsonObject json,jsonIn;
    json.insert("req", "login");
    jsonIn.insert("user",m_userId);
    jsonIn.insert("password",m_setting.value("password").toString());

    json.insert("data",jsonIn);
    QJsonDocument jsonDoc(json);
    QByteArray jsonData = jsonDoc.toJson();

    m_webSocket.sendBinaryMessage(jsonData);
}

void latexClient::sendLogout()
{
    QJsonObject json,jsonIn;
    json.insert("req", "logout");
    jsonIn.insert("userId",m_userId);
    json.insert("data",jsonIn);
    QJsonDocument jsonDoc(json);
    QByteArray jsonData = jsonDoc.toJson();

    m_webSocket.sendBinaryMessage(jsonData);
    //m_connected=false;
    //m_docConnected=false;
}

//update is decided by login
void latexClient::sendDoc(QString doc, bool update, bool convert, bool commit)
{
    QJsonObject json,jsonIn;
    json.insert("req", "document");
    jsonIn.insert("doc", doc);
    jsonIn.insert("userId",m_userId);
    jsonIn.insert("docId",m_docId);
    jsonIn.insert("innerId",m_innerId);
    jsonIn.insert("update",update);
    jsonIn.insert("commit",commit);
    jsonIn.insert("convet",convert);

    json.insert("data",jsonIn);
    QJsonDocument jsonDoc(json);
    QByteArray jsonData= jsonDoc.toJson();

    m_webSocket.sendBinaryMessage(jsonData);
}

//TODO connect to others
//at present only register doc
void latexClient::sendConnect(QList<QString> userList)
{
	QJsonObject json, jsonIn;
	QJsonArray jsonA;
    json.insert("req", "connect");
	jsonIn.insert("docId", m_docId);
    jsonIn.insert("userId",m_userId);
    jsonIn.insert("innerId",m_innerId);
    //qDebug()<<m_innerId;
	if (!userList.isEmpty())
	{
		foreach(QString userId, userList)
		{
			jsonA.append(userId);
		}
	}
    jsonIn.insert("peerId", jsonA);

	json.insert("data", jsonIn);
	QJsonDocument jsonDoc(json);
	QByteArray jsonData = jsonDoc.toJson();

	m_webSocket.sendBinaryMessage(jsonData);
}

//TODO connect to others
//at present only register doc
void latexClient::sendDisconnect(QList<QString> userList)
{
    QJsonObject json, jsonIn;
    QJsonArray jsonA;
    json.insert("req", "disconnect");
    jsonIn.insert("docId", m_docId);
    jsonIn.insert("userId",m_userId);
    //qDebug()<<m_innerId;
    if (!userList.isEmpty())
    {
        foreach(QString userId, userList)
        {
            jsonA.append(userId);
        }
    }
    jsonIn.insert("user", jsonA);

    json.insert("data", jsonIn);
    QJsonDocument jsonDoc(json);
    QByteArray jsonData = jsonDoc.toJson();

    m_webSocket.sendBinaryMessage(jsonData);
}


void latexClient::onSettingChanged(QString key)
{
    if(key=="docId"&&
            m_setting.value(key,"").toString()!=m_docId){        
        sendDisconnect();
        m_docId=m_setting.value(key,"").toString();
        //sendConnect();
    }
    else if (key=="userId"&&
             m_setting.value(key,"").toString()!=m_userId)
    {      
        if(isLogin()){
            sendLogout();
            //toDisconnect();
            //toConnect();
        }
        m_userId=m_setting.value(key,"").toString();
    }
}

void latexClient::onByteMessageReceived(QByteArray message)
{
    QJsonDocument document = QJsonDocument::fromJson(message);
    QJsonObject rootObj = document.object();
    if(rootObj.empty()||!rootObj.contains("res")&&!rootObj.contains("req")) {
        qDebug() << "Message error" << message;
        return;
    }
    else if(rootObj.contains("res"))
    {
        QJsonObject jsonData=rootObj["data"].toObject();
        if(jsonData.isEmpty()||!jsonData.contains("state"))
            qDebug() << rootObj["data"].toString() << " Message error" << jsonData;

        if(rootObj["res"]=="login")
            loginMessageReceived(jsonData);
        else if(rootObj["res"]=="logout")
        {
            errorCode state = (errorCode)jsonData["state"].toInt();
            if(state==CODE_SUCCESS||state==CODE_NOT_LOGIN){
                //m_setting.setValue("userId","");
                //m_userId = nullptr;
                setLogin(false);
                setConnected(false);
//                m_connected = false;
//                m_docConnected=false;
                emit logout();
            }
            else {
                //TODO show in ui
                qDebug()<<"logout error";
            }
        }
        else if(rootObj["res"]=="convert")
            convertMessageReceived(jsonData);
        else if(rootObj["res"]=="connect")
            connectMessageReceived(jsonData);
        else if(rootObj["res"]=="disconnect")
        {
            errorCode state = (errorCode)jsonData["state"].toInt();
            if(state==CODE_SUCCESS
                    ||state==CODE_NO_DOC
                    ||state==CODE_NOT_EXIST
                    ||state==CODE_NOT_LOGIN)
            {
                setConnected(false);
            }
            else {
                //TODO show in ui
                qDebug()<<"disconnect error"<<state;
            }
        }else if(rootObj["res"]=="update")
            updateMessageReceived(jsonData);
        else if(rootObj["res"]=="commit")
            commitMessageReceived(jsonData);
        else
            qDebug() << "Message contains unknown res" << rootObj;
    }
    else if (rootObj.contains("req"))
    {
        QJsonObject jsonData=rootObj["data"].toObject();
        if(jsonData.isEmpty()||!jsonData.contains("userId"))
            qDebug() << rootObj["data"].toString() << " Message error" << jsonData;

        if(rootObj["req"]=="rupdate")
           rupdateMessageReceived(jsonData);
        else if(rootObj["req"]=="rcommit")
           rcommitMessageReceived(jsonData);
        else if(rootObj["req"]=="rconnect")
            rcommitMessageReceived(jsonData);
        else if(rootObj["req"]=="rdisconnect")
            rcommitMessageReceived(jsonData);
        else if(rootObj["req"]=="rdisconnected")
            rdisconnectedMessageReceived(jsonData);
        else if(rootObj["req"]=="rlogout")
        {
            //TODO setting
//            setConnected(false);
//            setLogin(false);
            logout();
//            m_userId=nullptr;
//            m_connected=false;
//            m_docConnected=false;
            emit logout();
            //rlogoutMessageReceived(jsonData);
        }
        else
            qDebug() << "Message contains unknown req" << rootObj;
    }
}

void latexClient::loginMessageReceived(QJsonObject data)
{//TODO
    if(data.isEmpty()||!data.contains("state"))
        qDebug() << "login Message error" << data;
    errorCode state = (errorCode)data["state"].toInt();
    if(state == CODE_SUCCESS||state == CODE_NEW){
        setLogin(true);
        QString userId=data["userId"].toString();
        if(m_setting.value("userId")!=userId)
            m_setting.setValueSilent("userId",userId);
    }
    else {
        setLogin(false);
        setConnected(false);
        qDebug()<<"pass incorrect" << state;
    }
}

void latexClient::convertMessageReceived(QJsonObject data)
{
    if(data.isEmpty()||!data.contains("state"))
        qDebug() << "convert Message error" << data;
    errorCode state = (errorCode)data["state"].toInt();
    if(state==CODE_SUCCESS)
    {
        emit convertResult(data["result"].toString().toUtf8());
    }
    else {
        qDebug()<<"convert error";
        emit convertError(data["convertresult"].toString().toUtf8());
    }
}

void latexClient::connectMessageReceived(QJsonObject data)
{
    if(data.isEmpty()||!data.contains("state"))
        qDebug() << "connect Message error" << data;
    errorCode ec= (errorCode)data["state"].toInt();
    if( ec == CODE_SUCCESS|| ec == CODE_PEERS)//TODO continue commit
    {//TODO users
        setUpdate(true);
        setConnected(true);
    }
    else {
        setConnected(false);
        qDebug()<<"connect doc error" <<ec ;
        emit connectFailed();
    }
}

void latexClient::updateMessageReceived(QJsonObject data)//TODO
{
    if(data.isEmpty()||!data.contains("state"))
        qDebug() << "connect Message error" << data;
    errorCode state = (errorCode)data["state"].toInt();
    if(state == CODE_SUCCESS)
        qDebug()<<"update success";
    else {
        qDebug()<<"update failed";
        //TODO resend?
    }
}

void latexClient::commitMessageReceived(QJsonObject data)
{
    //TODO
}
//TODO to be improved
void latexClient::rupdateMessageReceived(QJsonObject data)
{
    if(!data.contains("doc"))
        qDebug()<<"rupdate invalid, no doc";
    QString patch=data.value("doc").toString();
    emit updateDoc(patch);
}

void latexClient::rcommitMessageReceived(QJsonObject data)
{
    //TODO
}

void latexClient::rconnectMessageReceived(QJsonObject data)
{
    m_docConnected=true;
    //TODO choose id...
}

void latexClient::rdisconnectMessageReceived(QJsonObject data)
{
    m_docConnected=false;

    //TODO switch to offline mode?
}

void latexClient::rdisconnectedMessageReceived(QJsonObject data)
{
    //TODO
}

//void latexClient::rloginMessageReceived(QJsonArray data)
//{

//}

void latexClient::onSslErrors(const QList<QSslError> &errors)
{
	Q_UNUSED(errors);

	foreach(const QSslError & e, errors)
	{
		qDebug() << "Ssl errors occurred" << e;
	}
    //Q_UNUSED(errors);
	//
    // WARNING: Never ignore SSL errors in production code.
    // The proper way to handle self-signed certificates is to add a custom root
    // to the CA store.
	//
	//m_webSocket.ignoreSslErrors();
	
}

bool latexClient::isValid()
{
	return m_valid;
}

bool latexClient::isLogin()
{
    return m_connected;
}

bool latexClient::isConnected()
{
    return m_docConnected;
}

bool latexClient::isUpdate()
{
    return m_update;
}

void latexClient::setValid(bool state)
{
    m_valid=state;
    emit validChanged();
}
void latexClient::setLogin(bool state)
{
    m_connected=state;
    emit loginChanged();
}

void latexClient::setConnected(bool state)
{
    m_docConnected=state;
    emit connectedChanged();
}

void latexClient::setUpdate(bool state)
{
    m_update=state;
    emit updateChanged();
}
