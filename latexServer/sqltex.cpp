#include "sqltex.h"
#include <QDebug>

//note: QTime::currentTime().toString("HH:mm:ss")

sqlTex::sqlTex(QObject *parent)
	: QObject(parent)
{
	m_db = QSqlDatabase::addDatabase("QMYSQL");
	m_db.setHostName("localhost");
	m_db.setPort(6033);
	m_db.setUserName("");
	m_db.setPassword("");

	if (!m_db.isValid()) {
		qDebug() << m_db.lastError().text();
		return;
	}

	m_db.open();
	QSqlQuery q;
	if(!q.exec("SELECT SCHEMA_NAME FROM INFORMATION_SCHEMA.SCHEMATA WHERE SCHEMA_NAME = 'latexdb'"))
		qDebug() << q.lastError();

	if (q.size()==0) {
		m_db.exec("CREATE DATABASE IF NOT EXISTS latexdb");
		qDebug() << m_db.lastError().text();
	}
	
	m_db.setDatabaseName("latexdb");
	m_db.close();
	m_db.open();

	createTable();

	//qDebug() << q.lastError();
}


//TODO create index
void sqlTex::createTable()
{
	QSqlQuery q;
	if(!q.exec("SELECT table_name FROM information_schema.tables where table_schema='latexdb';"))
		qDebug() << q.lastError();
	QStringList tables;
	while (q.next()) {
		tables.append(q.record().value("table_name").toString());
	}
	if (!tables.contains("login"))
	{
		if (!q.exec("create table IF NOT EXISTS login"
			" (login_id bigint auto_increment primary key, "
			" user_id int not null,"
			" login_time timestamp , "
			" logout_time timestamp , "
			" ip varchar(100) "
			" );"))
		{
			qDebug() << q.lastError();
		}

	}
	if (!tables.contains("user"))
	{
		if (!q.exec("create table IF NOT EXISTS user"
			"(user_id int auto_increment primary key,"
            " username varchar(32) unique, "
			" password varchar(64), "
			" status int(1),"
			" login_id bigint unique,"
			" foreign key (login_id) references login (login_id) on delete set null on update cascade"
			" );"))
		{
			qDebug() << q.lastError();
		}
	}
	if (!tables.contains("doc"))
	{
		if (!q.exec("create table IF NOT EXISTS doc"
			"(doc_id int auto_increment primary key,"
            " user_id int,"
			" foreign key (user_id) references user (user_id) on delete cascade on update cascade, "
            " create_time timestamp, "
            " update_time timestamp, "
			" latest_id bigint"
			" );"))
		{
			qDebug() << q.lastError();
		}
	}

	if (!tables.contains("doclink"))
	{
		if (!q.exec("create table IF NOT EXISTS doclink"
			" (link_id bigint auto_increment primary key,"
			" doc_id int not null,"
			" user_id int not null, "
			" foreign key (doc_id) references doc(doc_id) on delete cascade on update cascade, "
			" foreign key (user_id) references user (user_id) on delete cascade on update cascade"
			" ); "))
		{
			qDebug() << q.lastError();
		}

	}


	if (!tables.contains("alldoc"))
	{
		if (!q.exec("create table IF NOT EXISTS alldoc"
			"(version_id bigint auto_increment primary key,"
			" doc longtext, "
			" doc_id int not null, "
			" user_id int not null, "
			" foreign key (doc_id) references doc (doc_id) on delete cascade on update cascade, "
			" foreign key (user_id) references user (user_id) on delete cascade on update cascade,"
			" time timestamp not null,"
			" commit tinyint(1), "
			" latest tinyint(1), "
			" locked tinyint(1), "
			" origin bigint,"
			" foreign key (origin) references alldoc (version_id) on delete set null on update cascade"
			" );"))
		{
			qDebug() << q.lastError();
		}

		if (!q.exec("alter table doc "
			"add foreign key (latest_id) references alldoc (version_id) on delete set null on update cascade"
			" ;"))
		{
			qDebug() << q.lastError();
		}
	}

	if (!tables.contains("connection"))
	{	//this table clears every time
		if (!q.exec("create table IF NOT EXISTS connection"
			"(on_id int auto_increment primary key,"
			" user_id int not null, "
			" doc_id int, "
			" foreign key (user_id) references user (user_id) on delete cascade on update cascade, "
			" foreign key (doc_id) references doc (doc_id) on delete set null on update cascade"
			" ); "))
		{
			qDebug() << q.lastError();
		}
	}
}

//retrun CODE_DATABASE_ERROR,CODE_SUCCESS,CODE_NEW
errorCode sqlTex::login(
	QString &userId, 
	QString pw,
	QString ip
)
{
	if (!pw.isEmpty())
	{
		//TODO pw
	}

	QSqlQuery q,q2;
    QSqlRecord rec;
	q.prepare("select login_id , status from user where user_id = ? ;");
	q.addBindValue(userId);
	if (!q.exec())
	{
		qDebug() << q.lastError();
		return CODE_DATABASE_ERROR;
	}
	bool exist = false;
	if (q.next())
	{
		rec = q.record();
		exist = true;
		//push someone out
	}
	else
	{
		q.prepare("select user_id , login_id , status from user where username = ? ;");
		q.addBindValue(userId);
		if (!q.exec())
		{
			qDebug() << q.lastError();
			return CODE_DATABASE_ERROR;
		}
		if (q.next())
		{
			exist = true;
			rec = q.record();
			userId = QString::number(rec.value("user_id").toInt());
			//push someone out
		}
		else {
			QSqlQuery q3;
			q3.prepare("insert into user (username, password)"
				"values (?,?);");
			q3.addBindValue(userId);
			q3.addBindValue(pw);

			if (!q3.exec())
			{
				qDebug() << q3.lastError();
				return CODE_DATABASE_ERROR;
			}
            //TODO error here?
            userId=QString::number(q3.lastInsertId().toInt());
//			if (q3.next())
//			{
//				rec = q3.record();
//				userId = QString::number(rec.value("user_id").toInt());
//				//push someone out
//			}
		}
	}
	
	q2.prepare("insert into login (user_id, login_time, ip)"
		"values (?,?,?);");
    q2.addBindValue(userId);
	q2.addBindValue(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
	q2.addBindValue(ip);
	if (!q2.exec()) {
		qDebug() << q2.lastError();
		return CODE_DATABASE_ERROR;
	}

	QString loginId = QString::number(q2.lastInsertId().toInt());

    q.prepare("update user "
            "set login_id=? , status=? "
            "where user_id = ? ;");
    q.addBindValue(loginId);
    q.addBindValue(1);
    q.addBindValue(userId);
    if (!q.exec())
    {
        qDebug() << q.lastError();
        return CODE_DATABASE_ERROR;
    }


	if (exist)
	{
		return CODE_SUCCESS;
	}
	else
	{
		return CODE_NEW;
	}
}

// @return CODE_INVALID,CODE_DATABASE_ERROR,CODE_NOT_LOGIN,CODE_SUCCESS
errorCode sqlTex::logout(QString userId)
{
	QSqlQuery q;
    q.prepare("select status,login_id from user where user_id= ? ;");
	q.addBindValue(userId);
	if (!q.exec())
	{
		qDebug() << q.lastError();
		return CODE_DATABASE_ERROR;
	}
	if (!q.next())
	{
		qDebug() << "userId not found";
		return CODE_INVALID;
	}

	QSqlRecord rec = q.record();
    QString login_id = QString::number(
                rec.value("login_id").toInt());
    if (!rec.value("status").toInt()){
        removeConnection(userId);
		return CODE_NOT_LOGIN;
    }
    else{
        q.prepare("update user "
                "set status=? "
                "where user_id = ? ;");
        q.addBindValue(0);
        q.addBindValue(userId);
        if (!q.exec())
        {
            qDebug() << q.lastError();
            return CODE_DATABASE_ERROR;
        }
    }

    q.prepare("update login "
            "set logout_time=? "
            "where user_id = ? ;");
    q.addBindValue(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
	q.addBindValue(login_id);
	if (!q.exec())
	{
		qDebug() << q.lastError();
		return CODE_DATABASE_ERROR;
	}
	return removeConnection(userId);
}

//remove connection table, but not logout
// @return CODE_INVALID,CODE_DATABASE_ERROR,CODE_SUCCESS
errorCode sqlTex::removeConnection(QString userId, QString docId)
{
	QSqlQuery q;
	if (docId.isEmpty()) {
        //TODO debug here!
        qDebug()<<userId.toInt();
		if (userId.isEmpty())
			return CODE_INVALID;
		else {
            q.prepare("delete from connection where user_id = ?");
            q.addBindValue(userId);
			if (!q.exec())
			{//TODO compare difference between errors
				qDebug() << q.lastError().databaseText();
				return CODE_DATABASE_ERROR;
			}
			if (!q.next())
			{
                qDebug() << "logout: userId and docId not found in table connection" << userId << " "<< docId;
                //return CODE_INVALID;
                return CODE_SUCCESS;
			}
		}
	}
	else if(userId.isEmpty())
	{
		q.prepare("delete from connection where doc_id = ?");
        q.addBindValue(docId);
		if (!q.exec())
		{
			qDebug() << q.lastError();
			return CODE_DATABASE_ERROR;
		}
		if (!q.next())
		{
            qDebug() << "logout: docId not found" << userId << " "<< docId;
			return CODE_INVALID;
		}
	}
	else
	{
		q.prepare("delete from connection " \
			"where user_id = ? and doc_id = ?");
        q.addBindValue(userId);
        q.addBindValue(docId);
		if (!q.exec())
		{
			qDebug() << q.lastError();
			return CODE_DATABASE_ERROR;
		}
		if (!q.next())
		{
            qDebug() << "logout: userId and docId not found in table connection" << userId << " " << docId;
            return CODE_SUCCESS;
		}
	}
	return CODE_SUCCESS;
}	

//return >=0 doc_id;<0 db error
qint64 sqlTex::newDocId()
{
	QSqlQuery q;
    q.prepare("insert into doc (create_time) values (?);");
    q.addBindValue(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
    if (!q.exec())
	{
		qDebug() << q.lastError();
		return -1;
	}
	else 
	{
        return q.lastInsertId().toInt();
//		if (q.next()) {
//			qint64 num = q.record().field("doc_id").value().toInt();
//			return num;
//		}
//		else {
//			return -1;
//		}
	}
}

//version 1:if local update, the originalId is provided by client
//version 2:originId should be nullptr
QPair<errorCode, QString> sqlTex::uorhDoc(QString doc,
                                          QString userId,
                                          QString docId,
                                          bool commit,
                                          bool insist,
                                          QString originId)
{
    if(userId.isEmpty()||docId.isEmpty())
        return qMakePair(CODE_INVALID, QString());

	if (originId.isEmpty())
		originId = getOrigin(userId, docId, commit).value("version_id").toString();
    bool change;
	if (originId.isEmpty())
    {
        change=true;
        //return qMakePair(CODE_DATABASE_ERROR,QString());
    }
    else
        change = lockDoc(originId, commit, insist);
	if (insist && !change)
		return qMakePair(CODE_DATABASE_ERROR, QString());

	QString newId;
	//we need two versions here
	if(change&&!commit)
        //commit a version and get id
		newId=updateDoc(doc, userId, docId, true, change, originId).second;
    //use newid to update
    return updateDoc(doc, userId, docId, commit, change, (change && !commit) ? newId : originId);
}

QStringList sqlTex::getPeers(QString userId, QString docId)
{
	QSqlQuery q;
	QStringList res;
	QSqlRecord rec;
	if (!docId.isEmpty()&&userId.isEmpty())
	{
        q.prepare("select * from connection where doc_id = ?");
		q.addBindValue(docId);
	}
	else if(docId.isEmpty()&&!userId.isEmpty())
	{
        q.prepare("select * from connection where user_id = ?");
		q.addBindValue(userId);
		if (!q.exec())
		{
			qDebug() << q.lastError();
			return res;
		}
		if (!q.next())
		{
			qDebug() << "not found";
			return res;
		}

		do{
			rec = q.record();
			res.append(getPeers(userId, rec.value("doc_id").toString()));
		} while (q.next());

	    return res;
	}
	else if (!docId.isEmpty() & !userId.isEmpty())
	{
        q.prepare("select * from connection "
            "where user_id != ? and doc_id = ?");
		q.addBindValue(userId);
		q.addBindValue(docId);
	}
	else
	{
		qDebug() << "invalid";
		return QStringList();
	}

	if (!q.exec())
	{
		qDebug() << q.lastError();
		return QStringList();
	}
	if (!q.next())
	{
        qDebug() << "peers not found";
		return res;
	}

	do {
		rec = q.record();
		res.append(rec.value("user_Id").toString());
	} while (q.next());
	return res;
}


errorCode sqlTex::toconnect(QString userId, QString &docId)
{
	if (userId.isEmpty())
		return CODE_INVALID;
	if (docId.isEmpty())
		docId = QString::number(newDocId());

    QSqlQuery q;
    q.prepare("select * from user where user_id = ?");
    q.addBindValue(userId);
    if (!q.exec())
    {
        qDebug() << q.lastError();
        return CODE_DATABASE_ERROR;
    }
    if (!q.next())
    {
        qDebug() << q.lastError();
        return CODE_NOT_LOGIN;
    }

    QSqlRecord rec=q.record();
    if(!rec.value("status").toInt())
        return CODE_NOT_LOGIN;

    q.prepare("select * from connection where user_id = ? and doc_id = ?");
	q.addBindValue(userId);
	q.addBindValue(docId);
	if (!q.exec())
	{
		qDebug() << q.lastError();
		return CODE_DATABASE_ERROR;
	}
	if (!q.next())
	{
        q.prepare("insert into connection (user_id, doc_id)"
            "values (?, ?);");
		q.addBindValue(userId);
		q.addBindValue(docId);
		if (!q.exec())
		{
            qDebug() << q.lastError();
			return CODE_DATABASE_ERROR;
		}
//		if (!q.next())
//		{
//			qDebug() << "not created";
//			return CODE_DATABASE_ERROR;
//		}
	}

    q.prepare("select * from doclink where user_id = ? and doc_id = ?");
	q.addBindValue(userId);
	q.addBindValue(docId);
	if (!q.exec())
	{
		qDebug() << q.lastError();
		return CODE_DATABASE_ERROR;
	}
	if (!q.next())
	{
        q.prepare("insert into doclink (user_id, doc_id)"
			"values (?, ?);");
		q.addBindValue(userId);
		q.addBindValue(docId);
		if (!q.exec())
		{
			qDebug() << q.lastError();
			return CODE_DATABASE_ERROR;
		}
//		if (!q.next())
//		{
//			qDebug() << "not created";
//			return CODE_DATABASE_ERROR;
//		}
	}
    return CODE_SUCCESS;
}

QString sqlTex::getCommit(QString version_id)
{
	if (version_id.isEmpty())
		return nullptr;
	QSqlQuery q;
	QSqlRecord rec;
	QString id = version_id;
	do
	{
		q.prepare("select commit,origin from alldoc "
			"wherer version_id = ? ;");
		q.addBindValue(id);
		if (!q.exec())
		{
			qDebug() << q.lastError();
			return nullptr;
		}
		if (!q.next())
		{
			qDebug() << "not found";
			return nullptr;
		}
		rec = q.record();

		if (rec.value("commit").toInt() == 1)
			return id;
		else if (rec.value("origin").toString().isEmpty())
			return nullptr;
		id = rec.value("origin").toString();
	} while (1);
	return nullptr;
}

//if local update, the originalId is provided by client
//if commited, new version_id, instead of patch is returned
//update doc, if commit, add new id to doc
//  and get a patch
//  if origin is empty, create must be marked,

QPair<errorCode,QString> sqlTex::updateDoc(QString doc,
                                   QString userId,
                                   QString docId,
                                   bool commit,
                                   bool create,
                                   QString originId)
{
    QSqlQuery q;
    QSqlRecord reco;
    QString recn;
    QPair<errorCode, QString> res;

    //get origin record
    if (originId.isEmpty()){
        //attempt to get origin
        reco = getOrigin(userId, docId, commit);
        if(!reco.isEmpty())
            originId=QString::number(reco.value("version_id").toInt());
    }
    else
    {
        q.prepare("select * from alldoc " \
            "where version_id =?;"
        );
        q.addBindValue(originId);
        if (!q.exec())
        {
            qDebug() << q.lastError();
            res.first = CODE_DATABASE_ERROR;
            return res;
        }
        if (!q.next())
        {
            qDebug() << "version not found";
            res.first = CODE_INVALID;
            return res;
        }
        else
        {
            reco = q.record();
        }
    }

    //create new record
	if (create)
	{
        if (!originId.isEmpty()){
            q.prepare("update alldoc "
                        "set latest=? "
                        "where version_id = ? ;");
            q.addBindValue(0);
            q.addBindValue(originId);
            if (!q.exec())
            {
                qDebug() << q.lastError();
                return qMakePair(CODE_DATABASE_ERROR,QString());
            }

            q.prepare(
                "insert into alldoc "
                "(doc,doc_id,user_id,time,commit,latest,locked,origin)"
                "values (?, ?, ?, ?, ?, ?, ?, ?)"
            );
            q.addBindValue(doc);
            q.addBindValue(docId);//.toInt());
            q.addBindValue(userId);//.toInt());
            q.addBindValue(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
            q.addBindValue(commit ? 1 : 0);
            q.addBindValue(1);
            q.addBindValue(0);
            q.addBindValue(originId);//.toInt());
        }
        else {
            q.prepare(
                "insert into alldoc "
                "(doc,doc_id,user_id,time,commit,latest,locked)"
                "values (?, ?, ?, ?, ?, ?, ?)"
            );
            q.addBindValue(doc);
            q.addBindValue(docId);//.toInt());
            q.addBindValue(userId);//.toInt());
            q.addBindValue(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
            q.addBindValue(commit ? 1 : 0);
            q.addBindValue(1);
            q.addBindValue(0);
        }

		if (!q.exec())
		{
			qDebug() << q.lastError();
			res.first = CODE_DATABASE_ERROR;
			return res;
		}
//		if (!q.next())
//		{
//			qDebug() << "insert error";
//			res.first = CODE_DATABASE_ERROR;
//			return res;
//		}

        //new record
        recn = QString::number(q.lastInsertId().toInt());
	}
	else {
        //invalid
        if (originId.isEmpty()) {
			res.first = CODE_INVALID;
			return res;
		};

        q.prepare("update alldoc "
                    "set latest=? , time=? "
                    "where version_id = ? ;");
        q.addBindValue(1);
        q.addBindValue(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
        q.addBindValue(originId);
        if (!q.exec())
        {
            qDebug() << q.lastError();
            return qMakePair(CODE_DATABASE_ERROR,QString());
        }
    }
		
	if (commit) {
        if(!updateLatest(docId, create ? recn : originId))
            return QPair<errorCode,QString>(CODE_DATABASE_ERROR,"");
        res.second = create ? recn: originId;
	}
	else
    {
        QList<Patch> patcho;
        if(!originId.isEmpty())
        {
            patcho = m_dmp.patch_make(reco.value("doc").toString(), doc);
            QString out
                = m_dmp.patch_apply(patcho, reco.value("doc").toString()).first;
            if(!create){
                q.prepare("update alldoc "
                       "set doc=? "
                       "where version_id = ? ;");
               q.addBindValue(out);
               q.addBindValue(originId);
               if (!q.exec())
               {
                   qDebug() << q.lastError();
                   return qMakePair(CODE_DATABASE_ERROR,QString());
               }
            }
        }
        else{
            patcho = m_dmp.patch_make(doc, QString());
        }
        QString patch = m_dmp.patch_toText(patcho);
        res.second = patch;
	}

	res.first = CODE_SUCCESS;
	return res;
}

// return null if error
// is commited
QSqlRecord sqlTex::getOrigin(QString userId, QString docId,bool commit)
{
	QSqlQuery q;
	if(commit)
		q.prepare("select * from alldoc " \
            "where user_id = ? and doc_id = ? and commit = 1 " \
			"order by version_id desc limit 1;"
		);
	else 
		q.prepare("select * from alldoc " \
            "where user_id = ? and doc_id = ? and commit = 0 " \
			"order by version_id desc limit 1;"
		);
	q.addBindValue(userId);
	q.addBindValue(docId);
	if (!q.exec())
	{
		qDebug() << q.lastError();
		return QSqlRecord();
	}
	if (!q.next())
	{
		qDebug() << "version not found";
		return QSqlRecord();
	}
    QSqlRecord rec=q.record();
    if(rec.value("origin").toInt()<=0)
        return QSqlRecord();
	return q.record();
}

//if timeout or user logout, lock commit
//if time within, return false
//if no such, return false
bool sqlTex::lockDoc(QString version_id, bool commit, bool insist)
{
	QSqlQuery q;
	if(commit)
		q.prepare("select doc_id,time,commit,latest,locked from alldoc where version_id = ?  and commit = 1");
	else
		q.prepare("select doc_id,time,commit,latest,locked from alldoc where version_id = ? and commit = 0");
	q.addBindValue(version_id);
	if (!q.exec())
	{
		qDebug() << q.lastError();
		return false;
	}
	if (!q.next())
	{
		qDebug() << "version not found";
		return false;
	}

	QSqlRecord rec = q.record();
	if (rec.value("locked").toInt())
		return true;

	if (!insist)
	{
		QDateTime time = rec.value("time").toDateTime();
		if (QDateTime::currentDateTime() < time.addSecs(m_interval * 60))
			return false;
	}
	else {
        if(commit)
            q.prepare("update alldoc "
                   "set locked=? "
                   "where version_id = ?  and commit = 1");
        else
            q.prepare("update alldoc "
                   "set locked=? "
                   "where version_id = ?  and commit = 1");
       q.addBindValue(1);
       q.addBindValue(version_id);
       if (!q.exec())
       {
           qDebug() << q.lastError();
       }
	}
	return true;
}

bool sqlTex::updateLatest(QString docId, QString nversion_id)
{
    QSqlQuery q;
    q.prepare("update doc "
               "set latest_id=? ,update_time=? "
               "where doc_id = ? ;");
    q.addBindValue(nversion_id);
    q.addBindValue(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
    q.addBindValue(docId);
	if (!q.exec())
	{
		qDebug() << q.lastError();
		return false;
	}
	return true;
}

sqlTex::~sqlTex()
{
	//clear connection table
}
