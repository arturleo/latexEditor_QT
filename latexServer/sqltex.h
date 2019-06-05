#ifndef SQLTEX_H
#define SQLTEX_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QVariant>
#include <QSqlRecord>
#include <QSqlField>
#include <QDateTime>
#include <QSqlError>

#include "diff_match_patch.h"
#include "datatypes.h"
//now: logout time, connection (cd)
/*
 * user: user_id username password(salted) status login_id()
 * doc: doc_id user_id create_time update_time latest_id(version_id)
 * doclink: link_id doc_id user_id
 * login: login_id user_id login_time logout_time ip
 * alldoc: version_id doc doc_id user_id time commit(bool) latest(bool) locked(bool) origin(version_id)
 * connection: on_id user_id doc_id
 *	//TODO maybe in version 3, this table is omitted, to doclink "online" field
 */

class sqlTex : public QObject
{
	Q_OBJECT

public:
	sqlTex(QObject* parent = nullptr);
	~sqlTex();

	//TODO username
	errorCode login(QString &userId, QString pw = nullptr
		, QString ip = nullptr);
	errorCode logout(QString userId);
	errorCode removeConnection(QString userId, QString docId=nullptr);
	qint64 newDocId();
	QPair<errorCode, QString> uorhDoc(QString doc, QString userId, QString docId, bool commit=false, bool insist=false, QString originId=nullptr);

	QStringList getPeers(QString userId, QString docId=nullptr);
    errorCode toconnect(QString userId, QString &docId);

private:
	void createTable();

	QString getCommit(QString version_id);
	QPair<errorCode, QString> updateDoc(QString doc, QString userId, QString docId, bool commit=false, bool create=false, QString originId = nullptr);
	QSqlRecord getOrigin(QString userId, QString docId, bool commit);
	bool lockDoc(QString version_id, bool commit=false, bool insist = false);
    bool updateLatest(QString docId, QString nversion_id);

	diff_match_patch m_dmp;
	QSqlDatabase m_db;
	qint64 m_interval = 10;//mins of new commit

};


#endif // !SQLTEX_H
