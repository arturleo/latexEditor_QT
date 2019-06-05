#ifndef DATATYPES_H
#define DATATYPES_H

#include <QObject>
#include <QtCore/QDataStream>

enum errorCode
{
    CODE_SUCCESS = 0x00,//
	CODE_INVALID = 0x01,
	CODE_PASS_INCORRECT = 0x02,
    CODE_NEW = 0x03,//
    CODE_PEERS = 0x04,//
	CODE_NOT_LOGIN = 0x05,
	CODE_NO_DOC = 0x06,
	CODE_NOT_EXIST = 0x07,
	CODE_CONFLICT = 0x08,
    CODE_DATABASE_ERROR = 0x09,
	CODE_FAILURE = 0x0A,
};

enum displayMode{
    DISPLAY_VIEW=0,
    DISPLAY_EDIT=1,
    DISPLAY_SPLIT=2
};


#endif // !DATATYPES_H


