#include "syncserver.h"
#include <QDebug>

syncServer::syncServer(quint16 port, QObject *parent) :
    QObject(parent),
    m_pWebSocketServer(nullptr)
{
    m_pWebSocketServer = new QWebSocketServer(QStringLiteral("Latex Sync Server"),
	QWebSocketServer::SecureMode,this);
    QSslConfiguration sslConfiguration;
    QFile certFile(QStringLiteral(":/resource/tex.crt"));
    QFile keyFile(QStringLiteral(":/resource/tex.key"));
    certFile.open(QIODevice::ReadOnly);
    keyFile.open(QIODevice::ReadOnly);
    QSslCertificate certificate(&certFile, QSsl::Pem);
    QSslKey sslKey(&keyFile, QSsl::Rsa, QSsl::Pem);
    certFile.close();
    keyFile.close();
    sslConfiguration.setPeerVerifyMode(QSslSocket::VerifyNone);
    sslConfiguration.setLocalCertificate(certificate);
    sslConfiguration.setPrivateKey(sslKey);
    sslConfiguration.setProtocol(QSsl::TlsV1SslV3);
    m_pWebSocketServer->setSslConfiguration(sslConfiguration);

    if (m_pWebSocketServer->listen(QHostAddress::LocalHost, port))
    {
        qDebug() << "Server started, listening on port" << port;
        connect(m_pWebSocketServer, &QWebSocketServer::newConnection,
                this, &syncServer::onNewConnection);
        connect(m_pWebSocketServer, &QWebSocketServer::sslErrors,
                this, &syncServer::onSslErrors);
    }
}

syncServer::~syncServer()
{
    m_pWebSocketServer->close();
    qDeleteAll(m_clients.begin(), m_clients.end());
	//qDeleteAll(m_openDocuments.begin(), m_openDocuments.end());
	//qDeleteAll(m_userInfos.begin(), m_userInfos.end());
}

void syncServer::onNewConnection()
{
    QWebSocket *pSocket = m_pWebSocketServer->nextPendingConnection();
	qDebug() << "Client connected:" << pSocket;

    connect(pSocket, &QWebSocket::binaryMessageReceived,
            this, &syncServer::binaryMessageReceived);
    connect(pSocket, &QWebSocket::disconnected,
            this, &syncServer::socketDisconnected);

    m_clients << pSocket;
}

void syncServer::binaryMessageReceived(QByteArray byte)
{
    qDebug()<< "binaryMessageReceived:" << byte;
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    if (pClient)
    {
        QJsonDocument document = QJsonDocument::fromJson(byte);
        QJsonObject rootObj = document.object();
        if(rootObj.contains("req"))
        {
			QJsonObject data = rootObj["data"].toObject();
            if(rootObj["req"].toString() == "login")
            {
				if(data.contains("user"))
					loginMessageReceived(
                        pClient,
						data["user"].toString(),
						data["password"].toString());
            }
			else if (rootObj["req"].toString() == "logout")
			{

				QString userId = data["userId"].toString();
                if (m_userIdTb.contains(userId)){
                    errorCode ec=m_sql.logout(userId);
                    if(ec==CODE_SUCCESS||ec==CODE_NOT_LOGIN)
                    {
                        QWebSocket* pClient = qobject_cast<QWebSocket*>(sender());
                        if (pClient)
                        {
                            if(m_socketTb[pClient]==userId)
                                m_socketTb.remove(pClient);
                            m_userIdTb.remove(userId);
                        }
                    }
                     reply(pClient, "logout", ec, userId);
                     qDebug() << "User logged out:" << userId;
                }
                else{
					reply(pClient, "logout", CODE_NOT_LOGIN, userId);
                    qDebug() << "User logout not found" << userId;
                }
			}
			else if(rootObj["req"].toString() == "connect")
			{
				connectMessageReceived(pClient, data);
			}			
			else if(rootObj["req"].toString() == "disconnect")
			{
				disconnectMessageReceived(pClient, data);
			}

			else if (rootObj["req"].toString() == "document")
			{
				documentMessageReceived(pClient,data);
			}
        }
        else
        {
            qDebug()<<"invalid json:"<<byte;
        }
    }
}

void syncServer::loginMessageReceived(
	QWebSocket* client,
	const QString &userId, 
	const QString password)
{
	QString muserId = userId;
	if (muserId.isEmpty()) {
		reply(client, "login", CODE_INVALID);
	}
	else if (password != nullptr)
	{
		//TODO pw
	}

	if (m_userIdTb.contains(muserId))
	{
		QWebSocket* oldClient = m_userIdTb[muserId];
		//already login, at present old one will be disconnected;
		if (oldClient == client);
		else
		{
			send(oldClient, "logout", muserId);
			m_sql.logout(muserId);
			m_socketTb.remove(oldClient);

			//tell peers
			QStringList peers = m_sql.getPeers(muserId);
			QStringList::const_iterator itr=peers.constBegin();
			do {
				if (!m_userIdTb.contains(*itr))
				{
					qDebug() << "db error" << *itr;
					continue;
				}
				send(m_userIdTb[*itr], "rdisconnected", *itr, nullptr, muserId);
			} while (++itr!= peers.constEnd());
		}	
	}

	errorCode ec = m_sql.login(muserId);

    reply(client, "login", ec, muserId);
	if (ec != CODE_DATABASE_ERROR) {
		//m_userInfos["client"] = new QSet<QString>;
		m_userIdTb[muserId] = client;
		m_socketTb[client] = muserId;

		qDebug() << "User logged in:" << muserId << userId;
	}
	else
	{
        qDebug() << errno << ": CODE_DATABASE_ERROR";
	}
}

void syncServer::documentMessageReceived(
	QWebSocket* client,
	QJsonObject &data)
{
    QString doc = data["doc"].toString(), userId = data["userId"].toString(), docId = data["docId"].toString(), innerId = data["innerId"].toString();
	if (data.isEmpty() 
		|| !data["commit"].isBool() 
		|| !data["update"].isBool() 
		|| !data["convert"].isBool() 
		|| (userId.isEmpty()
			|| docId.isEmpty())
			&& (data["commit"].toBool()
				|| data["update"].toBool())
	)
	{
		//invalid
		reply(client,"",CODE_INVALID);
	}
	
	// only one true allowed
	//we only send patch here
	if (data["commit"].toBool())
	{
		QPair<errorCode, QString> res = m_sql.uorhDoc(doc, userId, docId, true, true);
		reply(client, "commit", res.first, docId);
	}
	if (data["update"].toBool()) 
	{
		QPair<errorCode, QString> res = m_sql.uorhDoc(doc, userId, docId, false, false);
		reply(client, "update", res.first, docId);
		//tell peers
		QStringList peers = m_sql.getPeers(userId, docId);
		QStringList::const_iterator itr = peers.constBegin();
        for ( ; itr != peers.constEnd(); ++itr ) {
            if(itr->isNull()||itr->isEmpty())
                break;
            if (!m_userIdTb.contains(*itr))
            {
                QString str=*itr;
                disconnect(str);
                qDebug() << "db error" << *itr;
                continue;
            }
            sendDoc(m_userIdTb[*itr], "rupdate", *itr, userId, res.second, nullptr);
        }
	}
	if (data["convert"].toBool())
	{
		QString id=QString::number(m_sql.newDocId());
		QPair<errorCode, QByteArray> result = convert(doc);
		reply(client,"convert",result.first,docId,innerId,result.second);
	}
}

//consider innerId
void syncServer::connectMessageReceived(
	QWebSocket* client,
	QJsonObject &data)
{
	QString docId = data["docId"].toString(),
        innerId = QString::number(data["innerId"].toInt());
	QString usrId = m_socketTb[client];
	if (usrId.isEmpty())
	{
		qDebug() << "login required";
        replyConnect(client, "connect", CODE_NOT_LOGIN, usrId,innerId, docId);
        return;
	}

	if (docId.isEmpty())
	{
		if (innerId.isEmpty())
		{
			replyConnect(client, "connect", CODE_INVALID);
			return;
		}
		//doc to id, add to sql
        QString docId = QString::number(m_sql.newDocId());
        //-1 will call error 9
        //replyConnect(client, "connect", m_sql.toconnect(usrId, docId), usrId ,docId, innerId);
	}
    errorCode ec=m_sql.toconnect(usrId, docId);
    if(ec==CODE_SUCCESS||ec==CODE_PEERS||ec==CODE_NEW)
        qDebug() << usrId << " " <<docId <<" "<< innerId << " connected";
    else
        qDebug() << usrId << " " <<docId <<" "<< innerId << " connect failed"
                 <<"\n error code: " <<ec;
    replyConnect(client, "connect", ec , usrId, innerId, docId);
    return;

}

//the commented part is version 0.5
void syncServer::disconnectMessageReceived(
	  QWebSocket* client
	, QJsonObject &data)
{
    QString userId = data["userId"].toString(), docId = data["docId"].toString();
	QJsonArray users = data["user"].toArray();
	if (data.isEmpty() || userId.isEmpty() 
		|| !data["user"].isNull()&& !data["user"].isArray())
	{
		//invalid
        qDebug() << "disconnect invalid";
        replyConnect(client, "disconnect", CODE_INVALID);
		return;
	}
	else //delete start
	{
		//TODO authenticate user identity
		if (!m_userIdTb.contains(userId))
		{
			//invalid
            qDebug() << "disconnect invalid";
            replyConnect(client, "disconnect", CODE_INVALID, userId);
			return;
		}

		if (!users.isEmpty())
		{
			QSet<QString> peers;
			foreach(QJsonValue u, users)
			{
				QString user = u.toString();
				QWebSocket* peer = m_userIdTb[user];
				if (!m_userIdTb.contains(user)||!disconnect(user, docId))
					peers.insert(user);
				else
					send(peer, "rdisconnect", user, docId, userId);
			}
			if (!peers.isEmpty())
				replyConnect(client, "disconnect", CODE_PEERS, userId,docId,nullptr, peers);
			else
                replyConnect(client, "disconnect", CODE_SUCCESS, userId, docId, nullptr, peers);
            return;
		}
		else 
		{
            errorCode ec= disconnect(userId,docId);
            qDebug() << "disconnect result:" << ec;
            replyConnect(client, "disconnect", ec, userId);
			return;
		}

		//if (docId.isEmpty())//delete user
		//{
		//	reply(client, "logout", disconnect(userId), userId);
		//	return;
		//}
		//else {
			//find user linked doc
		//	if (!m_userIdTb.contains(userId))
		//	{
		//		reply(client, "disconnect", CODE_NOT_LOGIN, userId);
		//	}
		//	else if (!m_userInfos[userId]->contains(docId))
		//	{
		//		reply(client, "disconnect", CODE_NO_DOC, userId, docId);
		//	}
		//	
			//if (users.isEmpty())//delete docId
			//{
		//		if (disconnectUser(userId, docId))
		//		{
		//			reply(client, "disconnect", CODE_SUCCESS, userId, docId);
		//		}
		//		else {
		//			reply(client, "success", CODE_NO_DOC, userId, docId);
		//		}
		//		//TODO emit
			//}
			//else//delete peers
			//{
		//		QSet<QString> peers;
		//		foreach(QJsonValue u, users)
		//		{
		//			QString user = u.toString();
		//			if (!disconnectUser(user, docId))
		//				peers.insert(user);
		//		}
		//		if (!peers.isEmpty())
		//			replyConnect(client, "disconnect", CODE_PEERS, peers);
		//		else
		//			replyConnect(client, "dsiconnect", CODE_SUCCESS, peers);
		//		//TODO emit
			//}
		//}
	}
}

//res:"login","logout","update","commit"
void syncServer::reply(
	  QWebSocket* client
	, QString res
	, errorCode code
	, QString userId
	, QString innerId
	, QString docId
	, QString result)
{
	QJsonObject json,data;
	json["res"] = res;
	data["state"] = code;
    data["userId"] = userId;
    //if (!docId.isEmpty())
		data["docId"] = docId;
    //if (!innerId.isEmpty())
		data["innerId"] = innerId;
    //if(!result.isEmpty())
        data["result"] = result;//TODO now utf8?

	json["data"] = data;
	QJsonDocument jsonDoc(json);

	client->sendBinaryMessage(jsonDoc.toJson());
}

//data may be change before it, just reply the request state, 
//	not to update it
void syncServer::replyConnect(
	QWebSocket* client,
    QString res,
	errorCode code, 
	QString userId,
    QString innerId,
    QString docId,
	QSet<QString> users)
{
	QJsonObject json, data;
    json["res"] = res;

    data["userId"] = userId;
    //if (!docId.isEmpty())
        data["docId"] = docId;
    //if (!innerId.isEmpty())
        data["innerId"] = innerId;
	data["state"] = code;
    data["users"] = QJsonArray::fromStringList(QStringList(users.toList()));//TODO check?

    json["data"] = data;
	QJsonDocument jsonDoc(json);

	client->sendBinaryMessage(jsonDoc.toJson());
}

//call the client to modify state
//"rconnect","rdisconnect","rdisconnected","rlogout"
void syncServer::send(
	  QWebSocket* client
	, QString req
	, QString userId
	, QString docId
	, QString userFrom
)
{
	QJsonObject json, data;
	json["req"] = req;

	data["userId"] = userId;
	data["docId"] = docId;
	data["userFrom"] = userFrom;

	json["data"] = data;
	QJsonDocument jsonDoc(json);

	client->sendBinaryMessage(jsonDoc.toJson());
}


void syncServer::sendDoc(
	QWebSocket* client, 
	QString req, 
	QString docId,
	QString userId, 
	QString doc, 
	QString time)
{
	QJsonObject json, data;
	json["req"] = req;

	data["userId"] = userId;
    data["docId"] = docId;
    data["doc"] = doc;
    data["time"] = time;

	json["data"] = data;
	QJsonDocument jsonDoc(json);

	client->sendBinaryMessage(jsonDoc.toJson());
}

//the commented part is version 0.5
errorCode syncServer::disconnect(QString &userId,
	QString docId)
{
	if (userId.isEmpty() && docId.isEmpty())
		return CODE_INVALID;
	if (userId.isEmpty()) {
        if (!m_userIdTb.contains(userId)){
            m_sql.removeConnection(userId, docId);
			return CODE_NOT_LOGIN;
        }
    }
    return m_sql.removeConnection(userId, docId);
	//if (docId.isEmpty())
	//{
	//
	//	//if (!m_socketTb.remove(m_userIdTb[userId]) || m_userIdTb.remove(userId))
	//	//	return false;
	//}
	//
	//if (!m_userInfos.contains(userId))
	//{
	//	//no docs registed
	//}
	//else
	//{
	//	QSet<QString> docs = *m_userInfos[userId];
	//
	//	if (docId.isEmpty()) {
	//		foreach(QString doc, docs)
	//		{
	//			if (!m_openDocuments.contains(doc))
	//			{
	//				//error in server
	//				qDebug() << doc << "not connected";
	//				docs.remove(doc);
	//			}
	//			else if (!m_openDocuments[doc]->remove(userId))
	//			{
	//				//error in server
	//				qDebug() << userId << "not connected to" << doc;
	//			}
	//			else {
	//				foreach (const QString &u, m_openDocuments[doc])
	//					if(u!=userId)
	//						sendConnect(m_userIdTb[u],"rdisconnected",doc,u, userId);
	//			}
	//		}
	//		m_userInfos.remove(userId);
	//		sendConnect(m_userIdTb[userId], "rlogout", userId);
	//	}
	//	else
	//	{
	//		if (docs.isEmpty()||!docs.contains(docId))
	//		{
	//			qDebug() << docId << "not connected";
	//			return false;
	//		}
	//
	//		docs.remove(docId);
	//		if (!m_openDocuments.contains(docId))
	//		{
	//			//error in server
	//			qDebug() << docId << "not connected in server";
	//			return false;
	//		}
	//		else if (!m_openDocuments[docId]->remove(userId))
	//		{
	//			//error in server
	//			qDebug() << userId << "not connected to" << docId;
	//			return false;
	//		}
	//		else {
	//			foreach(const QString & u, m_openDocuments[docId])
	//				if (u != userId)
	//					sendConnect(m_userIdTb[u], "rdisconnected", docId, u, userId);
	//		}
	//
	//		sendConnect(m_userIdTb[userId], "rdisconnect", docId, userId);
	//	}
	//}
	//
	//emit userListChanged();
	//emit userCountChanged();
}

//errorCode syncServer::disconnectDoc(
//	QString &docId, 
//	QString userId)
//{
//	//if (!m_openDocuments.contains(docId))
//	//	return false;
//	//if (userId.isEmpty())
//	//{
//	//	bool flag = false;
//	//	foreach(QString u, m_userIdTb.keys())
//	//	{
//	//		if (disconnectUser(u, docId))
//	//			flag = true;
//	//	}
//	//	return flag;
//	//}
//	//else
//	//{
//	//	return disconnectUser(userId, docId);
//	//}
//
//}

void syncServer::socketDisconnected()
{
	qDebug() << "Client disconnected";
	QWebSocket* pClient = qobject_cast<QWebSocket*>(sender());
	if (pClient)
    {
        QString userId=m_socketTb[pClient];
        if(!userId.isEmpty())
        {
            m_sql.logout(userId);
            //disconnect(userId);
            if(m_userIdTb[userId]==pClient)
                m_userIdTb.remove(userId);
        }

        m_socketTb.remove(pClient);
		m_clients.remove(pClient);
		pClient->deleteLater();
	}
	//TODO emit?
}

QPair<errorCode,QByteArray> syncServer::convert(QString doc)
{
	QTemporaryFile file;
	QPair<errorCode, QByteArray> res;
	if (!QDir(QDir::tempPath() + "/latexQTserver/tex").exists())
	{
		QDir(QDir::tempPath()).mkdir("latexQTserver");
		QDir(QDir::tempPath() + "/latexQTserver").mkdir("tex");
	}
	file.setFileTemplate(QDir::tempPath() + "/latexQTserver/tex/originXXX.tex");

	QByteArray r;
	if (file.open()) {
		file.setAutoRemove(false);

		QString name = file.fileName(); //returns the unique file name
		QFileInfo fi("C:/bla/blah/x/y/file.xls");
		QString fileName = fi.completeBaseName(), dir = fi.absolutePath();
		file.write(doc.toUtf8());
		file.close();

		QProcess prc;
		prc.setWorkingDirectory(dir);
		prc.start("pdftex -interaction nonstopmode -halt-on-error -file-line-error" + fileName);
		if (prc.waitForFinished(-1))
		{
			qDebug() << prc.errorString();
			res.first = CODE_FAILURE;
			return res;
		}
		QProcess::execute("bibtex " + fileName);
		prc.start("pdftex -interaction nonstopmode -halt-on-error -file-line-error" + fileName);
		if (!prc.waitForFinished(-1))
		{
			qDebug() << prc.errorString();
			res.first = CODE_FAILURE;
			return res;
		}
		prc.start("pdftex -interaction nonstopmode -halt-on-error -file-line-error" + fileName);
		if (!prc.waitForFinished(-1))
		{
			qDebug() << prc.errorString();
			res.first = CODE_FAILURE;
			return res;
		}

		if (!QFileInfo(dir + fileName + ".pdf").exists())
		{
			res.first = CODE_FAILURE;
			prc.start("pandoc " + fileName + ".log -o " + fileName + ".pdf");
			if (!prc.waitForFinished(-1))
			{
				qDebug() << prc.errorString();
				res.first = CODE_FAILURE;
				return res;
			}
		}
		QFile out(dir + "/" + fileName + ".pdf");
		if (!out.open(QIODevice::ReadOnly))
		{
			qDebug() << out.errorString();
			res.first = CODE_FAILURE;
			return res;
		}
		QTextStream in(&out);
		r = in.readAll().toUtf8();
		res.second = r;
	}
	else {
		qDebug() << "unbale to open";
		res.first = CODE_FAILURE;
		return res;
	}
    QProcess::execute("del *.dvi \\ && "
        "del* .out \\ "
        "del* .log \\ "
        "del* .aux \\ "
        "del* .bbl \\ "
        "del* .blg \\ "
        "del* .brf \\ "
		);
	file.remove();

	return res;
}

void syncServer::onSslErrors(const QList<QSslError>& errors)
{
	foreach(const QSslError & e, errors)
	{
		qDebug() << "Ssl errors occurred" << e;
	}
}
