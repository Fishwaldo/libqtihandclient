/*
 * MessageHandler.h
 *
 *  Created on: Sep 1, 2009
 *      Author: fish
 */

#ifndef MESSAGEHANDLER_H_
#define MESSAGEHANDLER_H_
#include <QtNetwork/QTcpSocket>
#include <QtNetwork/QtNetwork>
#include "support/MuscleSupport.h"
#include "message/Message.h"
#include "iogateway/MessageIOGateway.h"
#include "iogateway/AbstractMessageIOGateway.h"
#include "util/RefCount.h"
#include "util/Hashtable.h"
//#include "qtsupport/QSocketDataIO.h"

#include "iHanClient/varcontainer.hpp"
#include "iHanClient/MessageBus.hpp"
#include "iHanClient/MsgTypes.hpp"
class QTcpDataIO;

typedef enum State_e {S_DISCONNECTED, S_CONNECTED, S_READY} State_e;

class MessageHandler : public QObject, public muscle::AbstractGatewayMessageReceiver
{
	Q_OBJECT
public:
	MessageHandler(QObject *parent = 0);
	virtual ~MessageHandler();
    Q_PROPERTY(int type READ getType WRITE setType);
    Q_PROPERTY(QString username READ getUserName WRITE setUserName);
    Q_PROPERTY(QString password WRITE setPassword);
    Q_PROPERTY(QString hostname READ getHostName WRITE setHostName);
    Q_PROPERTY(quint32 port READ getPort WRITE setPort);
    Q_PROPERTY(quint32 flags READ getFlags WRITE setFlags);

    void setType(int Type);
    int getType();
    void setUserName(QString name);
    QString getUserName();
    void setPassword(QString password);
    QString getPassword();
    void setHostName(QString hostname);
    QString getHostName();
    void setPort(quint32 port);
    quint32 getPort();
    quint32 getFlags() { return this->flags; };
    void setFlags(quint32 newflags) { this->flags = newflags; };


    int getHostID() {
    	return this->hostid;
    }
    void setHostID(int val) {
    	this->hostid = val;
    }
    bool connect();
    State_e getState();

Q_SIGNALS:
	void connected();
	void disconnected();
	void error(QString, QAbstractSocket::SocketError);
	void StateChange(State_e);
	void newEndPt(MessageBus item);
	void delEndPt(MessageBus item);
	void updateValues(MessageBus vals);
	void updateConfig(MessageBus vals);
	void gotTermTypeMapping(MessageBus vals);
	void gotMyInfo(MessageBus vals);

public Q_SLOTS:
	void sendMessage(MessageBus);
private slots:
    void HandleSockPackets();
    void HandleSockError(QAbstractSocket::SocketError);
    void HandleSockUpdate(QAbstractSocket::SocketState);
    void HandleConnected();

private:
	void MessageReceivedFromGateway(const muscle::MessageRef & msg, void * /*userData*/);
	void processServerCaps(MessageBus msg);
	void processNewEndPt(MessageBus msg);
	void processDelEndPt(MessageBus  msg);
	void processSensorUpdate(MessageBus msg);
	void processConfigUpdate(MessageBus msg);
	void processSetup(MessageBus msg);
	int type;
	QString username;
	QString password;
	QString hostname;
	quint32 port;
	muscle::MessageIOGateway gw;
	QTcpSocket *tcpSocket;
	QTcpDataIO *tcpDataIO;
	int hostid;
	State_e CurState;
	quint32 flags;
};

class QTcpDataIO : public muscle::DataIO, public muscle::MessageIOGateway
{
public:
	QTcpDataIO(QTcpSocket *thetcpsocket);
	virtual ~QTcpDataIO () { }
	int32 Write(const void * buffer, uint32 size);
	int32 Read(void * buffer, uint32 size);
	virtual status_t Seek(int64 /*offset*/, int /*whence*/) {return B_ERROR;};
	virtual int64 GetPosition() const {return B_ERROR;};
	virtual void FlushOutput() {return;};
	virtual void Shutdown() {if (tcpsocket->isOpen()) tcpsocket->close();};
	virtual const muscle::ConstSocketRef & GetSelectSocket() const { return muscle::GetNullSocket(); };
	virtual const muscle::ConstSocketRef & GetReadSelectSocket() const { return muscle::GetNullSocket(); };
	virtual const muscle::ConstSocketRef & GetWriteSelectSocket() const { return muscle::GetNullSocket(); };
	void ReleaseSocket() {return;};
	void setParent(QTcpSocket *myparent) { this->tcpsocket = myparent;}
	void RealRead();
private:
	QTcpSocket *tcpsocket;
	QByteArray indata;
};

#endif /* MESSAGEHANDLER_H_ */
