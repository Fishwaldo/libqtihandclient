/*
 * MessageHandler.cpp
 *
 *  Created on: Sep 1, 2009
 *      Author: fish
 */
#include <QtGui/QScreen>
#include <QtNetwork/QAbstractSocket>
#include "QtiHanClient/MessageHandler.h"
#include "QtiHanClient/DeviceModel.h"
#include "iHanClient/MsgTypes.hpp"
#include "iHanClient/VariableTypes.hpp"
#include "iHanClient/MessageBus.hpp"




MessageHandler::MessageHandler(QObject *parent) : CurState(S_DISCONNECTED), flags(0) {
	setParent(parent);
    tcpSocket = new QTcpSocket(this);
    tcpDataIO =  new QTcpDataIO(this->tcpSocket);
    QObject::connect(tcpSocket, SIGNAL(readyRead()),
            this, SLOT(HandleSockPackets()));
    QObject::connect(tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)),
			this, SLOT(HandleSockError(QAbstractSocket::SocketError)));
    QObject::connect(tcpSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)),
			this, SLOT(HandleSockUpdate(QAbstractSocket::SocketState)));
    QObject::connect(tcpSocket, SIGNAL(connected()),
			this, SLOT(HandleConnected()));
    gw.SetDataIO(muscle::DataIORef(this->tcpDataIO));
    this->hostid = qrand();
    this->port = 1234;
    this->type = 0;
}

MessageHandler::~MessageHandler() {
	this->gw.Shutdown();
	iHanClient::VarTypeHelper::Destroy();
}

bool MessageHandler::connect() {
	qDebug() << this->hostname;
	if (!this->hostname.length()) {
		emit error("Hostname Not Set", QAbstractSocket::UnknownSocketError);
		return false;
	}
	if (this->port == 0) {
		emit error("Port Not Set", QAbstractSocket::UnknownSocketError);
		return false;
	}
	if (!this->username.length()) {
		emit error("Username Not Set", QAbstractSocket::UnknownSocketError);
		return false;
	}
	if (!this->password.length()) {
		emit error("Password Not Set", QAbstractSocket::UnknownSocketError);
		return false;
	}
	if (this->type == 0) {
		emit error("Client Type Not Set", QAbstractSocket::UnknownSocketError);
		return false;
	}
	qDebug() << "Trying to connect to " << this->hostname << ":" << this->port;
	tcpSocket->connectToHost(this->hostname, this->port);
    return true;
}

void MessageHandler::HandleSockPackets()
{
	this->tcpDataIO->RealRead();
	this->gw.DoInput(*this);
}
void MessageHandler::HandleSockError(QAbstractSocket::SocketError errorcode) {
	qDebug() << "SockError " <<  qPrintable(tcpSocket->errorString());
	emit error(tcpSocket->errorString(), errorcode);
	emit disconnected();
}
void MessageHandler::HandleSockUpdate(QAbstractSocket::SocketState state) {
	if (state == QAbstractSocket::ClosingState) {
		qDebug() << "Socket Closed";
		emit disconnected();
	}
}
void MessageHandler::HandleConnected() {
	VarContainerFactory(clncap);
	clncap->addStringValue(MSGB_CLNCAP_AUTHUSER, this->username.toStdString());
	clncap->addStringValue(MSGB_CLNCAP_AUTHKEY, this->password.toStdString());
	clncap->addLongValue(MSGB_CLNCAP_CAPDEVICE, this->type);
	clncap->addLongValue(MSGB_CLNCAP_HOSTID, this->hostid);
	clncap->addLongLongValue(MSGB_CLNCAP_FLAGS, this->flags);
	MessageBusFactory(mb);
	if (!mb->createClientCap(clncap, "")) {
		qWarning() << "Failed to create ClientCap Message";
		return;
	}

	this->sendMessage(mb);
	emit connected();
	this->CurState = S_CONNECTED;
	emit StateChange(this->CurState);
}
void MessageHandler::MessageReceivedFromGateway(const muscle::MessageRef & msg, void * /*userData*/)
{
	VarContainerFactory(rawmsg);
	rawmsg->importMuscleMsg(msg);
	MessageBusFactory(mb);
	if (!mb->importTransportVarStorage(rawmsg)) {
		qWarning() << "Failed to importTransportVarStorage Message";
		msg()->PrintToStream();
		return;
	}
	switch (mb->getType()) {
		case MSB_SERVER_CAP:
			/* w00p, we are logged in. Send our CAPS */
			processServerCaps(mb);
			qDebug() << "Connected!";
			this->CurState = S_READY;
			emit StateChange(this->CurState);
			break;
		case MSB_NEW_DEVICE:
			processNewEndPt(mb);
			break;
		case MSB_DEL_DEVICE:
			processDelEndPt(mb);
			break;
		case MSB_REPORT_VAR:
			processSensorUpdate(mb);
			break;
		case MSB_REPORT_CONFIG:
			processConfigUpdate(mb);
			break;
		case MSB_SETUP:
			processSetup(mb);
			break;
		default:
			qWarning() << "Got Unknown What Message: " <<  mb->getTypeAsString().c_str();
			//qWarning() << mb;
			break;
	}
}
void MessageHandler::processNewEndPt(MessageBus msg) {
	if (msg->getType() != MSB_NEW_DEVICE) {
		qWarning() << "Invalid MessageBus Type recieved in processNewEndPt" << msg->getTypeAsString().c_str();
		return;
	}
	VarStorage newdev = msg->getNewDevice();
	if (newdev->getSize() == 0) {
		qWarning() << "Empty newDev Client from MessageBus";
		return;
	}
	std::string DeviceSerial;
	newdev->getStringValue(SRVCAP_ENDPT_SERIAL, DeviceSerial);
	std::string DeviceName;
	newdev->getStringValue(SRVCAP_ENDPT_NAME, DeviceName);
	qDebug() << "Adding New Endpoint: " << DeviceName.c_str() << " (" << DeviceSerial.c_str() << ")";
	emit newEndPt(msg);
}

void MessageHandler::processDelEndPt(MessageBus msg) {
	if (msg->getType() != MSB_DEL_DEVICE) {
		qWarning() << "Invalid MessageBus Type recieved in processDelEndPt" << msg->getTypeAsString().c_str();
		return;
	}
	VarStorage newdev = msg->getNewDevice();
	if (newdev->getSize() == 0) {
		qWarning() << "Empty newDev Client from MessageBus";
		return;
	}
	std::string DeviceSerial;
	newdev->getStringValue(SRVCAP_ENDPT_SERIAL, DeviceSerial);
	std::string DeviceName;
	newdev->getStringValue(SRVCAP_ENDPT_NAME, DeviceName);
	qDebug() << "Deleting Endpoint: " << DeviceName.c_str() << " (" << DeviceSerial.c_str() << ")";
	emit delEndPt(msg);
}


void MessageHandler::processServerCaps(MessageBus msg) {
	Q_UNUSED(msg);
//	msg()->PrintToStream();
}



void MessageHandler::sendMessage(MessageBus msg) {
	VarStorage rawmsg = msg->getTransportVarStorage();
	if (rawmsg) {
		muscle::MessageRef MMsg = rawmsg->toMuscle();
		MMsg()->what = rawmsg->getWhat();
		this->gw.AddOutgoingMessage(MMsg);
		this->gw.DoOutput();
	} else {
		qWarning() << "Failed to get TransportVarStorage Message";
		return;
	}

}

void MessageHandler::processSensorUpdate(MessageBus msg) {
	if (msg->getType() != MSB_REPORT_VAR) {
		qWarning() << "Invalid MessageBus Type recieved in processSensorUpdate" << msg->getTypeAsString().c_str();
		return;
	}
	VarStorage var = msg->getReportVar();
	if (var->getSize() == 0) {
		qWarning() << "Empty var message from MessageBus";
		std::cout << msg << std::endl;
		return;
	}
	std::cout << msg << std::endl;
	emit updateValues(msg);
}
void MessageHandler::processConfigUpdate(MessageBus msg) {
	if (msg->getType() != MSB_REPORT_CONFIG) {
		qWarning() << "Invalid MessageBus Type recieved in processConfigUpdate" << msg->getTypeAsString().c_str();
		return;
	}
	VarStorage config = msg->getReportConfig();
	if (config->getSize() == 0) {
		qWarning() << "Empty config message from MessageBus";
		std::cout << msg << std::endl;
		return;
	}
	emit updateConfig(msg);
}

void MessageHandler::setType(int Type) {
	this->type = Type;
}

int MessageHandler::getType() {
	return this->type;
}

void MessageHandler::setUserName(QString name) {
	this->username = name;
}
QString MessageHandler::getUserName() {
	return this->username;
}
void MessageHandler::setPassword(QString password) {
	this->password = password;
}
QString MessageHandler::getPassword() {
	return this->password;
}
void MessageHandler::setHostName(QString hostname) {
	this->hostname = hostname;
}
QString MessageHandler::getHostName() {
	return this->hostname;
}
void MessageHandler::setPort(quint32 port) {
	this->port = port;
}
quint32 MessageHandler::getPort() {
	return this->port;
}

State_e MessageHandler::getState() {
	return this->CurState;
}

void MessageHandler::processSetup(MessageBus msg) {
	if (msg->getType() != MSB_SETUP) {
		qWarning() << "Invalid MessageBus Type recieved in processSetup" << msg->getTypeAsString().c_str();
		return;
	}
	VarStorage setup = msg->getSetup();
	if (setup->getSize() == 0) {
		qWarning() << "Empty Setup message from MessageBus";
		return;
	}
	if (setup->getSize(MSGB_SETUP_CLIENTINFORM) > 0) {
		qDebug() << "Got Client Inform Message";
		emit gotMyInfo(msg);
	}

	if ((this->flags & CLNTCAP_FLAG_VARTYPE) & setup->getSize("VarTypes")) {
		qDebug() << "Got VarTypes Setup Message";
		HashVals VarTypes;
		setup->getHashValue("VarTypes", VarTypes);
		if (VarTypes.size() == 0) {
			qWarning() << "VarTypes Setup Message is Empty";
			return;
		}
		iHanClient::VarTypeHelper::Create(VarTypes);
	}
	if ((this->flags & CLNTCAP_FLAG_TERMS) & setup->getSize("TermTypes")) {
		qDebug() << "Got TermTypeMappings Setup Message";
		emit gotTermTypeMapping(msg);
	}

}




QTcpDataIO::QTcpDataIO(QTcpSocket *thetcpsocket) {
        this->tcpsocket = thetcpsocket;
}
int32 QTcpDataIO::Write(const void * buffer, uint32 size) {
        return this->tcpsocket->write((const char *)buffer,size);
}
int32 QTcpDataIO::Read(void * buffer, uint32 size) {
        int32 actualsize;
        if ((int)size > this->indata.size())
                actualsize = this->indata.size();
        else
                actualsize = size;
        memcpy(buffer, this->indata.data(), actualsize);
        this->indata.remove(0, actualsize);
        return actualsize;
}
void QTcpDataIO::RealRead() {
        this->indata.append(this->tcpsocket->readAll());
}
