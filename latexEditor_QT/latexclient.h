#ifndef LATEXCLIENT_H
#define LATEXCLIENT_H

#include <QObject>
#include <QtWebSockets/QWebSocket>
#include <QtNetwork/QSslError>
#include <QList>
#include <QString>
#include <QUrl>
#include <QSettings>
#include <QCryptographicHash>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>

#include "texsetting.h"
#include "datatypes.h"

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
 *	3: connect { req:"connect", data:{docId:""/null, userId:"",innerId,peerId:[,,,]}}
 *  3-1: disconnect { req:"disconnect, data:{userId,docId,user}"}<=delete user?doc?peer?
 *=>4: update { req:"update", data:{userId:"",docId:"", doc:""}}//check user and time
 *=>5: commit
 *
 * TODO get and compare branch
 * 2,4,5 together:
 *      { req:"document", data:{userid, docid, doc:"", update:bool, commit:bool, convert:bool}}
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
 *                        userID, docId, innerId,users:[userID1,userID2....][who connected]}
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


//TODO inner id
QT_FORWARD_DECLARE_CLASS(QWebSocket)

class latexClient : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool valid READ isValid WRITE setValid NOTIFY validChanged )
    Q_PROPERTY(bool login READ isLogin WRITE setLogin NOTIFY loginChanged )
    Q_PROPERTY(bool connected READ isConnected WRITE setConnected NOTIFY connectedChanged )
    Q_PROPERTY(bool update READ isUpdate WRITE setUpdate NOTIFY updateChanged )

public:
    explicit latexClient(texSetting &setting, QObject *parent = nullptr);
	virtual ~latexClient();

    Q_INVOKABLE void toConnect();
    Q_INVOKABLE void toDisconnect();
    //check valid before doing this
    Q_INVOKABLE void sendLogin();//QString userId,QString password=nullptr);//TODO add salt
    Q_INVOKABLE void sendLogout();
    Q_INVOKABLE void sendDoc(QString doc,bool update=true, bool convert=false, bool commit=false );
    Q_INVOKABLE void sendConnect(QList<QString> userList=QList<QString>());//TODO connect doc, docId
    Q_INVOKABLE void sendDisconnect(QList<QString> userList=QList<QString>());//TODO connect doc, docId

	bool isValid();
    bool isLogin();
    bool isConnected();
    bool isUpdate();

    void setValid(bool state);
    void setLogin(bool state);
    void setConnected(bool state);
    void setUpdate(bool state);
signals:
    void logout();
    void convertResult(QByteArray pdf);
    void convertError(QByteArray error);
    void connectSuccess();
    void connectFailed();
    void updateDoc(QString patch);

    void validChanged();
    void loginChanged();
    void connectedChanged();
    void updateChanged();
private slots:
    void onConnected();
    void onByteMessageReceived(QByteArray message);
    void onSslErrors(const QList<QSslError> &errors);
    void onSettingChanged(QString key);
    void socketDisconnected();
private:
    void loginMessageReceived(QJsonObject data);
    void convertMessageReceived(QJsonObject data);
    void connectMessageReceived(QJsonObject data);
    void updateMessageReceived(QJsonObject data);
    void commitMessageReceived(QJsonObject data);
    void rupdateMessageReceived(QJsonObject data);
    void rcommitMessageReceived(QJsonObject data);
    void rconnectMessageReceived(QJsonObject data);
    void rdisconnectMessageReceived(QJsonObject data);
    void rdisconnectedMessageReceived(QJsonObject data);
    //void rloginMessageReceived(QJsonArray data);

    //TODO so far not know how to handle innerId
    qint16 m_innerId;
    QUrl m_url;
    int m_port;
    QString m_docId,m_userId;//connect
    texSetting &m_setting;
    QWebSocket m_webSocket;
    bool m_valid, m_connected,
        m_docConnected, m_update;
};


#endif // LATEXCLIENT_H
