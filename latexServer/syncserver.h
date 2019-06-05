#ifndef SYNCSERVER_H
#define SYNCSERVER_H

#include <QObject>
#include <QList>
#include <QByteArray>
#include <QStringList>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QProcess> 
#include <QTemporaryFile> 
#include "QtWebSockets/QWebSocketServer"
#include "QtWebSockets/QWebSocket"
#include <QtNetwork/QSslCertificate>
#include <QtNetwork/QSslKey>
#include <QtNetwork/QSslError>
#include <QCryptographicHash>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>

#include "sqltex.h"
#include "datatypes.h"

QT_FORWARD_DECLARE_CLASS(QWebSocketServer)
QT_FORWARD_DECLARE_CLASS(QWebSocket)

/*
 *document changed if set server=> send message to compile
 *          if login=> save
 *          if splitview=> send pdf;
 *      if has cooperator=> set document bound
 *                  then push and patch
 *              commit system: branch, master
 *
 *	1: login{ req:"login",data:{user:"",password:""} }
 *  1-1: logout{req,data{userId}}
 *=>2: convert { req:"convert", data:{docId, doc:"",innerId}}//check user and time
 *	3: connect { req:"connect", data:{docId:""/null, userId:"",innerId}}
 *  3-1: disconnect { req:"disconnect, data:{userId,docId,user}"}<=delete user?doc?peer?
 *=>4: update { req:"update", data:{userid:"",docId:"", doc:""}}//check user and time
 *=>5: commit
 *
 * TODO get and compare branch
 * 2,4,5 together:
 *      { req:"document", data:{userId, docId, doc:"", update:bool, commit:bool, convert:bool}}
 *
 *
 *reply:
 *
 *TODO: remove cooperate user
 *=> 1: {res:"login",data:{state:[success==0|invalid|pass incorrect==2|new user==4],userId}}
    1-1: logout
 *both added to sql
 *TODO check ownership
 *  2: {res:"convert",data:{state[success==0|invalid|failed!=0],docId,innerid,result:{pdf/debug}}}
 *  3: {res:"connect",data:{state[success|invalid|notLogIn|docexisted|usernotconnected],//TODO why failed
 *                        userId, docId, innerId,users:[userID1,userID2....][who connected]}
 *  3-1: {res:"disconnect", state:[success|invalid|user|doc|peer],userId,docId,[users]}
 *=>4: {res:"update",state:[success|invalid|notLogIn|idNotExist|sentfailed],userId,docId}
 *=>5: commit {res,[success|invalid|notLogIn|idNotExist|conflict],user,doc}//TODO why conflict?
 *
 *
 *redirect:
 *  1: be connected{req:"rconnect",data:{userId:"",docId:"",userFrom}}
 *		rdisconnect: ask to disconnect
 *		rdisconnected: someone disconnected
 *		rlogout: ask to logout
 *  2: sent Doc: {req:"rupdate",data:{userId:"",docId:"",doc:""<=patch[,patch:""],time:""}}
 *  3: sent Commit: {req:"rcommit",
 *              data:{docId:""
 *					,doc:""[/patch:""]
 *					,time:""
 *					,userId:""}}
 *
 *
 */

/*
 * describe commit here:
    version 1:
    the rule is: "only syncronize the latest commit branch with origin branch"
    online: userA create doc, commit it online, userB open commit,
        add patch to commit, send patch to A.
    offline: B open doc with modified, then it will either update or commit,
        otherwise use another docid
    branch mode: A, B modify it without sync, then commit it

    version 2:
    online edit:
        users overwrite their own branch to get patch
            patch then sent to all users online
                after 10 min, new branch is set, the master is auto saved
        the master commit is made when user save or commit it,
        if commited, origin is changed

    offline:
        check if last master origin equals matser commit
        user doc is patched with its last master origin on matser commit
        then show it and ask user's agreement
        choose another docid, modify the branch or commit

    branch mode:
        like online, the only difference is not syncronize
        the master branch is not auto committed too
*/



//TODO user logout or disconnect, send version_id to logout
//TODO only send patch
//TODO send docid with mode: online/branch , here only online
//TODO add ids and merge func
//TODO updatecommit func to accept 
class syncServer : public QObject
{
    Q_OBJECT

public:
    explicit syncServer(quint16 port, QObject *parent = nullptr);
    virtual ~syncServer() override;

	void reply(QWebSocket* client, QString res, errorCode code, QString userId = nullptr, QString innerId = nullptr, QString docId = nullptr, QString result = nullptr);//login,disconnect,update,commit
    void replyConnect(QWebSocket* client, QString res, errorCode code, QString userId=nullptr, QString innerId=nullptr, QString docId=nullptr, QSet<QString> users = QSet<QString>());

	void send(QWebSocket* client, QString req, QString userId, QString docId=nullptr, QString userFrom = nullptr);
	void sendDoc(QWebSocket* client, QString req, QString docId, QString userId, QString doc, QString time);//rupdate rcommit
    //void sendUpdate();

protected slots:
    void onNewConnection();
    void binaryMessageReceived(QByteArray byte);
    void socketDisconnected();
    void onSslErrors(const QList<QSslError> &errors);

signals:

private:
    void loginMessageReceived(QWebSocket* client, const QString &userName, const QString password=nullptr);//TODO send sql user docs list
    void documentMessageReceived(QWebSocket* client, QJsonObject &data);
    void connectMessageReceived(QWebSocket* client,	QJsonObject &data);
	void disconnectMessageReceived(QWebSocket* client, QJsonObject &data);

	errorCode disconnect(QString &userId, QString docId = nullptr);

	QPair<errorCode, QByteArray> convert(QString doc);

	QSet<QWebSocket *> m_clients;//no use
	QHash<QString, QWebSocket*> m_userIdTb;//userId=>socket
	QHash<QWebSocket*, QString> m_socketTb;//socket=>userId//for faster speed
	//QHash<QString, QSet<QString>*> m_openDocuments;//docId=>userIds
	//check user validability
    //QHash<QString, QSet<QString>*> m_userInfos;//userId=>docIds    

	sqlTex m_sql;
    QWebSocketServer *m_pWebSocketServer;
	//TODO delete it
	//TODO add path to 
	QString PATH = "";
};

#endif // SYNCSERVER_H
