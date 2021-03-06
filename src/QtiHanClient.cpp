/* controlpanel - QtiHanClient.cpp
 ** Copyright (c) 2010 Justin Hammond
 **
 **  This program is free software; you can redistribute it and/or modify
 **  it under the terms of the GNU General Public License as published by
 **  the Free Software Foundation; either version 2 of the License, or
 **  (at your option) any later version.
 **
 **  This program is distributed in the hope that it will be useful,
 **  but WITHOUT ANY WARRANTY; without even the implied warranty of
 **  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 **  GNU General Public License for more details.
 **
 **  You should have received a copy of the GNU General Public License
 **  along with this program; if not, write to the Free Software
 **  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
 **  USA
 **
 ** controlpanel SVN Identification:
 ** $Rev$
 */

/** @file QtiHanClient.cpp
 *  @brief
 */

#include "QtiHanClient/QtiHanClient.h"
#include <iostream>
#include <fstream>
#include <string>
#include <cmath>
#include <vector>
#include <sstream>
#include <cstdio>
#include <QTableView>

QMap<std::string, VarStorage> GlobalDevices;
QtiHanClient* QtiHanClient::s_instance = NULL;
QtiHanClient* QtiHanClient::Create(QObject *parent)
{
	if( NULL == s_instance )
	{
		s_instance = new QtiHanClient(parent);
	}
	return s_instance;
}
void QtiHanClient::Destroy()
{
	delete s_instance;
	s_instance = NULL;
}



QtiHanClient::QtiHanClient(QObject *parent) {
	setParent(parent);
	this->mh = new MessageHandler(this);
	/* XXX Do this Dynamically when reqested */
	this->tdm = new DeviceModel_t(this);
	this->fdm = new KDescendantsProxyModel(this);
	this->fdm->setSourceModel(this->tdm);
	this->mt = new ModelTest(this->tdm, this);

	QObject::connect(this->mh, SIGNAL(connected()),
			this, SLOT(HandleConnected()));
	QObject::connect(this->mh, SIGNAL(disconnected()),
			this, SLOT(HandleDisconnected()));
	QObject::connect(this->mh, SIGNAL(error(QString, QAbstractSocket::SocketError)),
			this, SLOT(HandleError(QString, QAbstractSocket::SocketError)));
	QObject::connect(this->mh, SIGNAL(newEndPt(MessageBus)),
			this, SLOT(HandleNewDevice(MessageBus)));
	QObject::connect(this->mh, SIGNAL(delEndPt(MessageBus)),
			this, SLOT(HandleDelDevice(MessageBus)));
	QObject::connect(this->mh, SIGNAL(updateValues(MessageBus)),
			this, SLOT(HandleDeviceUpdate(MessageBus)));
	QObject::connect(this->mh, SIGNAL(updateConfig(MessageBus)),
			this, SLOT(HandleDeviceConfigUpdate(MessageBus)));
	QObject::connect(this->mh, SIGNAL(addConfig(MessageBus)),
			this, SLOT(HandleAddConfig(MessageBus)));
	QObject::connect(this->mh, SIGNAL(addVar(MessageBus)),
			this, SLOT(HandleAddVar(MessageBus)));
	QObject::connect(this->mh, SIGNAL(delConfig(MessageBus)),
			this, SLOT(HandleDelConfig(MessageBus)));
	QObject::connect(this->mh, SIGNAL(delVar(MessageBus)),
			this, SLOT(HandleDelVar(MessageBus)));

	QObject::connect(this->mh, SIGNAL(StateChange(State_e)),
			this, SLOT(HandleStateChange(State_e)));
	QObject::connect(this->mh, SIGNAL(gotTermTypeMapping(MessageBus)),
			this, SLOT(HandleTermTypeMappings(MessageBus)));
	QObject::connect(this->tdm, SIGNAL(sendMsg(MessageBus)),
			this->mh, SLOT(sendMessage(MessageBus)));
	QObject::connect(this->mh, SIGNAL(gotMyInfo(MessageBus)),
			this, SLOT(HandleClientInform(MessageBus)));
	this->type = 0;
	this->mh->setFlags(CLNTCAP_FLAG_VARTYPE | CLNTCAP_FLAG_TERMS);

	QTableView *tv = new QTableView();
	tv->setModel(this->fdm);
	tv->show();

}
QtiHanClient::~QtiHanClient() {
	delete this->mh;
}

void QtiHanClient::setType(int Type) {
	this->type = Type;
	this->mh->setType(this->type);
}

int QtiHanClient::getType() {
	return this->type;
}

void QtiHanClient::setUserName(QString name) {
	this->mh->setUserName(name);
}
QString QtiHanClient::getUserName() {
	return this->mh->getUserName();
}
void QtiHanClient::setPassword(QString password) {
	this->mh->setPassword(password);
}
QString QtiHanClient::getPassword() {
	return this->mh->getPassword();
}
void QtiHanClient::setHostName(QString hostname) {
	this->mh->setHostName(hostname);
}
QString QtiHanClient::getHostName() {
	return this->mh->getHostName();
}
void QtiHanClient::setPort(quint32 port) {
	this->mh->setPort(port);
}
quint32 QtiHanClient::getPort() {
	return this->mh->getPort();
}

int QtiHanClient::getHostID() {
	return this->mh->getHostID();
}
void QtiHanClient::setHostID(int val) {
	this->mh->setHostID(val);
}

bool QtiHanClient::connect() {
	return this->mh->connect();
}

void QtiHanClient::HandleConnected() {
	emit connected();
}
void QtiHanClient::HandleDisconnected() {
	emit disconnected();
}
void QtiHanClient::HandleError(QString errstr, QAbstractSocket::SocketError errtype) {
	emit error(errstr, errtype);
}
void QtiHanClient::HandleStateChange(State_e state) {
	emit StateChange(state);
}
void QtiHanClient::HandleNewDevice(MessageBus msg) {
	VarStorage item = msg->getNewDevice();
	std::string deviceID;
	if (!item->getStringValue(SRVCAP_ENDPT_SERIAL, deviceID)) {
		qWarning() << "Can't get End Point Serial in newEndPt";
		return;
	}
	/* Regardless of whats going on, add this new End Point to the Global DeviceMap */
	if (GlobalDevices.contains(deviceID)) {
		qWarning() << "Device Already Exists in Global DeviceMap in newEndPt";
		return;
	}
	GlobalDevices[deviceID] = item;
	/* Create a New DeviceModel based on this device */
	this->tdm->addDevice(item);
	emit newEndPt(item);
}
void QtiHanClient::HandleDelDevice(MessageBus msg) {

	std::string deviceID = msg->getDelDevice();
#if 0
	if (!item->getStringValue(SRVCAP_ENDPT_SERIAL, deviceID)) {
		qWarning() << "Can't get End Point Serial in delEndPt";
		return;
	}
#endif
	this->tdm->delDevice(deviceID);
	/* Regardless of whats going on, del this End Point in the Global DeviceMap */
	if (GlobalDevices.contains(deviceID)) {
		GlobalDevices.remove(deviceID);
		return;
	} else {
		qWarning() << "Device Does Not Exists in Global DeviceMap";
	}
	emit delEndPt(deviceID);
}
void QtiHanClient::HandleDeviceUpdate(MessageBus msg) {
	VarStorage newvals = msg->getReportVar();
	std::string deviceID;
	QVector<QString> updatedfields;
	VarStorage vals;
	VarStorage valsdescriptor;
	bool compare = true;
	deviceID = msg->getSource();
#if 0
	if (!item->getStringValue(SRVCAP_ENDPT_SERIAL, deviceID)) {
		qWarning() << "Can't get End Point Serial in HandleDeviceUpdate";
		return;
	}
	VarStorage newvals;
	if (item->getVarStorageValue(SRVCAP_ENDPT_VARS, newvals) == false) {
		qWarning("Can't get End Point Vars from Packet");
		std::cout << *item << std::endl;
		return;
	}

#endif
	if (!GlobalDevices.contains(deviceID)) {
		qWarning() << "Can't find Device in DeviceMap for HandleDeviceUpdate";
		return;
	}
	qDebug() << "Got Updated Values from Device";
	if (GlobalDevices[deviceID]->getVarStorageValue(SRVCAP_ENDPT_VARS, vals) == false) {
		qWarning("Can't get End Point Vars from GloablDevices");
		return;
	}
	if (GlobalDevices[deviceID]->getVarStorageValue(SRVCAP_ENDPT_VARS_DESC, valsdescriptor) == false) {
		qWarning("Can't get End Point Var Descriptors from GloablDevices");
		return;
	}

	//cout << "New: " << std::endl;
	//newvals->printToStream();
	//cout << "Old: " << std::endl;
	//vals->printToStream();
	std::vector<std::string> *newfields = newvals->getFields();
	//std::vector<std::string> *oldfields = vals->getFields();
	//qDebug() << "New Fields:";
	for (std::size_t i = 0; i < newfields->size(); i++) {
		//std::cout << newfields->at(i) << std::endl;

		/* check if the NewField is a Callback */
		HashVals desc;
		if (valsdescriptor->getHashValue(newfields->at(i), desc)) {
			try {
				if ((t_ConfigType)boost::get<int>(desc["Type"]) == TC_CALLBACK) {
					VarStorage cb;
					if (newvals->getVarStorageValue(newfields->at(i), cb)) {
						qDebug() << "Got Config Callback for Device " << deviceID.c_str() << " on Field " << QString::fromStdString(newfields->at(i));
						emit configCallback(deviceID.c_str(), QString::fromStdString(newfields->at(i)), cb);
					} else {
						qWarning() << "Couldn't retrieve Callback Value from Update Message";
					}
					continue;
				}
			} catch (std::exception &e) {
				qWarning() << "Exception in HandleDeviceUpdate: " << e.what();
				continue;
			}
		} else {
			qWarning() << "Couldn't Retrieve VarValDescriptor for Field " << QString::fromStdString(newfields->at(i));
			continue;
		}

		/* confirm exists in oldfields */
		if (vals->getSize(newfields->at(i)) <= 0) {
			qDebug() << "New Field in Update Message";
			compare = false;
		}
		StoredType_t newtype = newvals->getType(newfields->at(i));
		if ((compare) && (newtype != vals->getType(newfields->at(i)))) {
			/* values do not match type */
			qWarning() << "Updated Field " << QString::fromStdString(newfields->at(i)) <<  " is not the same type as existing field";
			qWarning() << "NewType: " << newtype << " Existing Type: " << vals->getType(newfields->at(i));
			continue;
		}
		for (unsigned int j = 0; j < newvals->getSize(newfields->at(i)); j++) {
			switch (newtype) {
				case ST_STRING: {
					std::string newval;
					std::string oldval;
					if (!newvals->getStringValue(newfields->at(i), newval, j)) {
						qWarning() << "Couldn't get updated String Value";
						continue;
					}
					if ((compare) && (!vals->getStringValue(newfields->at(i), oldval, j))) {
						qWarning() << "Couldn't get Old String Value";
						continue;
					}
					if ((!compare) || (newval != oldval)) {
						vals->replaceStringValue(newfields->at(i), newval, j);
						if (!updatedfields.contains(newfields->at(i).c_str()))
							updatedfields.push_back(newfields->at(i).c_str());
					}
				}
				break;
				case ST_INT: {
					int newval;
					int oldval;
					if (!newvals->getIntValue(newfields->at(i), newval, j)) {
						qWarning() << "Couldn't get updated Int Value";
						continue;
					}
					if ((compare) && (!vals->getIntValue(newfields->at(i), oldval, j))) {
						qWarning() << "Couldn't get Old Int Value";
						continue;
					}
					if ((!compare) || (newval != oldval)) {
						vals->replaceIntValue(newfields->at(i), newval, j);
						if (!updatedfields.contains(newfields->at(i).c_str()))
							updatedfields.push_back(newfields->at(i).c_str());
					}
				}
				break;
				case ST_LONG: {
					long newval;
					long oldval;
					if (!newvals->getLongValue(newfields->at(i), newval, j)) {
						qWarning() << "Couldn't get updated Long Value";
						continue;
					}
					if ((compare) && (!vals->getLongValue(newfields->at(i), oldval, j))) {
						qWarning() << "Couldn't get Old Long Value";
						continue;
					}
					if ((!compare) || (newval != oldval)) {
						vals->replaceLongValue(newfields->at(i), newval, j);
						if (!updatedfields.contains(newfields->at(i).c_str()))
							updatedfields.push_back(newfields->at(i).c_str());
					}
				}
				break;
				case ST_LONGLONG:{
					long long newval;
					long long oldval;
					if (!newvals->getLongLongValue(newfields->at(i), newval, j)) {
						qWarning() << "Couldn't get updated Long Long Value";
						continue;
					}
					if ((compare) && (!vals->getLongLongValue(newfields->at(i), oldval, j))) {
						qWarning() << "Couldn't get Old Long Long Value";
						continue;
					}
					if ((!compare) || (newval != oldval)) {
						vals->replaceLongLongValue(newfields->at(i), newval, j);
						if (!updatedfields.contains(newfields->at(i).c_str()))
							updatedfields.push_back(newfields->at(i).c_str());
					}
				}
				break;
				case ST_FLOAT:{
					float newval;
					float oldval;
					if (!newvals->getFloatValue(newfields->at(i), newval, j)) {
						qWarning() << "Couldn't get updated Float Value";
						continue;
					}
					if ((compare) && (!vals->getFloatValue(newfields->at(i), oldval, j))) {
						qWarning() << "Couldn't get Old Float Value";
						continue;
					}
					if ((!compare) || (newval != oldval)) {
						vals->replaceFloatValue(newfields->at(i), newval, j);
						if (!updatedfields.contains(newfields->at(i).c_str()))
							updatedfields.push_back(newfields->at(i).c_str());
					}
				}
				break;
				case ST_BOOL:{
					bool newval;
					bool oldval;
					if (!newvals->getBoolValue(newfields->at(i), newval, j)) {
						qWarning() << "Couldn't get updated Bool Value";
						continue;
					}
					if ((compare) && (!vals->getBoolValue(newfields->at(i), oldval, j))) {
						qWarning() << "Couldn't get Old Bool Value";
						continue;
					}
					if ((!compare) || (newval != oldval)) {
						vals->replaceBoolValue(newfields->at(i), newval, j);
						if (!updatedfields.contains(newfields->at(i).c_str()))
							updatedfields.push_back(newfields->at(i).c_str());
					}
				}
				break;
				case ST_DATETIME:{
					boost::posix_time::ptime newval;
					boost::posix_time::ptime oldval;
					if (!newvals->getTimeValue(newfields->at(i), newval, j)) {
						qWarning() << "Couldn't get updated Bool Value";
						continue;
					}
					if ((compare) && (!vals->getTimeValue(newfields->at(i), oldval, j))) {
						qWarning() << "Couldn't get Old Bool Value";
						continue;
					}
					if ((!compare) || (newval != oldval)) {
						vals->replaceTimeValue(newfields->at(i), newval, j);
						if (!updatedfields.contains(newfields->at(i).c_str()))
							updatedfields.push_back(newfields->at(i).c_str());
					}
				}
				break;
				case ST_VARSTORAGE: {
					/* For now, just replace the entire thing */
					VarStorage newval;
					if (!newvals->getVarStorageValue(newfields->at(i), newval, j)) {
						qWarning() << "Couldn't get updated VarStorage Value";
						continue;
					}
					vals->replaceVarStorageValue(newfields->at(i), newval, j);
					if (!updatedfields.contains(newfields->at(i).c_str()))
						updatedfields.push_back(newfields->at(i).c_str());

				}
				break;
				case ST_HASH: {
					/* For now, just replace the entire thing */
					HashVals newval;
					if (!newvals->getHashValue(newfields->at(i), newval, j)) {
						qWarning() << "Couldn't get updated Hash Value";
						continue;
					}
					vals->replaceHashValue(newfields->at(i), newval, j);
					if (!updatedfields.contains(newfields->at(i).c_str()))
						updatedfields.push_back(newfields->at(i).c_str());
				}
				break;
				case ST_LIST: {
					ListVals value;
					if (!newvals->getListValue_p(newfields->at(i), value, j)) {
						qWarning() << "Couldn't get Updated List Value";
						continue;
					}
					vals->replaceListValue_p(newfields->at(i), value, j);
					if (!updatedfields.contains(newfields->at(i).c_str()))
						updatedfields.push_back(newfields->at(i).c_str());
				}
				break;
				case ST_INVALID: {
					qWarning() << "Recieved a Field with Type 'Invalid'";
					continue;
				}
				break;
			};
		}
	}

	if (updatedfields.size() == 0)
		return;

	qDebug() << "Updated Var Fields:";
	for (int i = 0; i < updatedfields.size(); i++) {
		qDebug() << qPrintable(updatedfields.at(i));
	}

	this->tdm->updateDeviceVars(newvals);
	emit updateValues(deviceID.c_str(), updatedfields);
}

void QtiHanClient::HandleDeviceConfigUpdate(MessageBus msg) {
	VarStorage newvals = msg->getReportConfig();
	std::string deviceID = msg->getSource();
	QVector<QString> updatedfields;
	VarStorage vals;
	VarStorage valsdescriptor;
	bool compare = true;

	if (!GlobalDevices.contains(deviceID)) {
		qWarning() << "Can't find Device in DeviceMap for HandleDeviceConfigUpdate";
		return;
	}
	qDebug() << "Got Updated Config from Device";

#if 0
	if (!item->getStringValue(SRVCAP_ENDPT_SERIAL, deviceID)) {
		qWarning() << "Can't get End Point Serial in HandleDeviceConfigUpdate";
		return;
	}
	VarStorage newvals;

	if (item->getVarStorageValue(SRVCAP_ENDPT_CONFIG, newvals) == false) {
		qWarning("Can't get End Point Config from Packet");
		return;
	}
#endif
	if (GlobalDevices[deviceID]->getVarStorageValue(SRVCAP_ENDPT_CONFIG, vals) == false) {
		qWarning("Can't get End Point Config from GloablDevices");
		return;
	}
	if (GlobalDevices[deviceID]->getVarStorageValue(SRVCAP_ENDPT_CONFIG_DESC, valsdescriptor) == false) {
		qWarning("Can't get End Point Var Descriptors from GloablDevices");
		return;
	}
	//cout << "New: " << std::endl;
	//newvals->printToStream();
	//cout << "Old: " << std::endl;
	//vals->printToStream();
	std::vector<std::string> *newfields = newvals->getFields();
	//std::vector<std::string> *oldfields = vals->getFields();
	//qDebug() << "New Fields:";
	for (std::size_t i = 0; i < newfields->size(); i++) {
		//std::cout << newfields->at(i) << std::endl;
		/* confirm exists in oldfields */

		/* check if the NewField is a Callback */
		HashVals desc;
		if (valsdescriptor->getHashValue(newfields->at(i), desc)) {
			try {
				if ((t_ConfigType)boost::get<int>(desc["Type"]) == TC_CALLBACK) {
					VarStorage cb;
					if (newvals->getVarStorageValue(newfields->at(i), cb)) {
						qDebug() << "Got Config Callback for Device " << deviceID.c_str() << " on Field " << QString::fromStdString(newfields->at(i));
						emit configCallback(deviceID.c_str(), QString::fromStdString(newfields->at(i)), cb);
					} else {
						qWarning() << "Couldn't retrieve Callback Value from Update Message";
					}
					continue;
				}
			} catch (std::exception &e) {
				qWarning() << "Exception in HandleDeviceUpdate: " << e.what();
				continue;
			}
		} else {
			qWarning() << "Couldn't Retrieve ConfigVarDescriptor for Field " << QString::fromStdString(newfields->at(i));
			continue;
		}



		if (vals->getSize(newfields->at(i)) <= 0) {
			qDebug() << "New Field in HandleDeviceConfigUpdate";
			/* just add it to the updates */
			compare = false;
		}
		StoredType_t newtype = newvals->getType(newfields->at(i));
		if ((compare) && (newtype != vals->getType(newfields->at(i)))) {
			/* values do not match type */
			qWarning() << "Updated Field " << QString::fromStdString(newfields->at(i)) << " is not the same type as existing field";
			qWarning() << "NewType: " << newtype << " Existing Type: " << vals->getType(newfields->at(i));
			continue;
		}
		for (unsigned int j = 0 ; j < newvals->getSize(newfields->at(i)); j++ ) {
			switch (newtype) {
				case ST_STRING: {
					std::string newval;
					std::string oldval;
					if (!newvals->getStringValue(newfields->at(i), newval, j)) {
						qWarning() << "Couldn't get updated String Value";
						continue;
					}
					if ((compare) && (!vals->getStringValue(newfields->at(i), oldval, j))) {
						qWarning() << "Couldn't get Old String Value";
						continue;
					}
					if ((!compare) || (newval != oldval)) {
						vals->replaceStringValue(newfields->at(i), newval, j);
						if (!updatedfields.contains(newfields->at(i).c_str()))
							updatedfields.push_back(newfields->at(i).c_str());
					}
				}
				break;
				case ST_INT: {
					int newval;
					int oldval;
					if (!newvals->getIntValue(newfields->at(i), newval, j)) {
						qWarning() << "Couldn't get updated Int Value";
						continue;
					}
					if ((compare) && (!vals->getIntValue(newfields->at(i), oldval, j))) {
						qWarning() << "Couldn't get Old Int Value";
						continue;
					}
					if ((!compare) || (newval != oldval)) {
						vals->replaceIntValue(newfields->at(i), newval, j);
						if (!updatedfields.contains(newfields->at(i).c_str()))
							updatedfields.push_back(newfields->at(i).c_str());
					}
				}
				break;
				case ST_LONG: {
					long newval;
					long oldval;
					if (!newvals->getLongValue(newfields->at(i), newval, j)) {
						qWarning() << "Couldn't get updated Long Value";
						continue;
					}
					if ((compare) && (!vals->getLongValue(newfields->at(i), oldval, j))) {
						qWarning() << "Couldn't get Old Long Value";
						continue;
					}
					if ((!compare) || (newval != oldval)) {
						vals->replaceLongValue(newfields->at(i), newval, j);
						if (!updatedfields.contains(newfields->at(i).c_str()))
							updatedfields.push_back(newfields->at(i).c_str());
					}
				}
				break;
				case ST_LONGLONG:{
					long long newval;
					long long oldval;
					if (!newvals->getLongLongValue(newfields->at(i), newval,  j)) {
						qWarning() << "Couldn't get updated Long Long Value";
						continue;
					}
					if ((compare) && (!vals->getLongLongValue(newfields->at(i), oldval, j))) {
						qWarning() << "Couldn't get Old Long Long Value";
						continue;
					}
					if ((!compare) || (newval != oldval)) {
						vals->replaceLongLongValue(newfields->at(i), newval, j);
						if (!updatedfields.contains(newfields->at(i).c_str()))
							updatedfields.push_back(newfields->at(i).c_str());
					}
				}
				break;
				case ST_FLOAT:{
					float newval;
					float oldval;
					if (!newvals->getFloatValue(newfields->at(i), newval, j)) {
						qWarning() << "Couldn't get updated Float Value";
						continue;
					}
					if ((compare) && (!vals->getFloatValue(newfields->at(i), oldval, j))) {
						qWarning() << "Couldn't get Old Float Value";
						continue;
					}
					if ((!compare) || (newval != oldval)) {
						vals->replaceFloatValue(newfields->at(i), newval, j);
						if (!updatedfields.contains(newfields->at(i).c_str()))
							updatedfields.push_back(newfields->at(i).c_str());
					}
				}
				break;
				case ST_BOOL:{
					bool newval;
					bool oldval;
					if (!newvals->getBoolValue(newfields->at(i), newval, j)) {
						qWarning() << "Couldn't get updated Bool Value";
						continue;
					}
					if ((compare) && (!vals->getBoolValue(newfields->at(i), oldval, j))) {
						qWarning() << "Couldn't get Old Bool Value";
						continue;
					}
					if ((!compare) || (newval != oldval)) {
						vals->replaceBoolValue(newfields->at(i), newval, j);
						if (!updatedfields.contains(newfields->at(i).c_str()))
							updatedfields.push_back(newfields->at(i).c_str());
					}
				}
				break;
				case ST_DATETIME:{
					boost::posix_time::ptime newval;
					boost::posix_time::ptime oldval;
					if (!newvals->getTimeValue(newfields->at(i), newval, j)) {
						qWarning() << "Couldn't get updated Bool Value";
						continue;
					}
					if ((compare) && (!vals->getTimeValue(newfields->at(i), oldval, j))) {
						qWarning() << "Couldn't get Old Bool Value";
						continue;
					}
					if ((!compare) || (newval != oldval)) {
						vals->replaceTimeValue(newfields->at(i), newval, j);
						if (!updatedfields.contains(newfields->at(i).c_str()))
							updatedfields.push_back(newfields->at(i).c_str());
					}
				}
				break;
				case ST_VARSTORAGE: {
					/* For now, just replace the entire thing */
					VarStorage newval;
					if (!newvals->getVarStorageValue(newfields->at(i), newval, j)) {
						qWarning() << "Couldn't get updated VarStorage Value";
						continue;
					}
					vals->replaceVarStorageValue(newfields->at(i), newval, j);
					if (!updatedfields.contains(newfields->at(i).c_str()))
						updatedfields.push_back(newfields->at(i).c_str());
				}
				break;
				case ST_HASH: {
					/* For now, just replace the entire thing */
					HashVals newval;
					if (!newvals->getHashValue(newfields->at(i), newval, j)) {
						qWarning() << "Couldn't get updated Hash Value";
						continue;
					}
					vals->replaceHashValue(newfields->at(i), newval, j);
					if (!updatedfields.contains(newfields->at(i).c_str()))
						updatedfields.push_back(newfields->at(i).c_str());
				}
				break;
				case ST_LIST: {
					ListVals value;
					if (!newvals->getListValue_p(newfields->at(i), value, j)) {
						qWarning() << "Couldn't get Updated List Value";
						continue;
					}
					vals->replaceListValue_p(newfields->at(i), value, j);
					if (!updatedfields.contains(newfields->at(i).c_str()))
						updatedfields.push_back(newfields->at(i).c_str());
				}
				break;
				case ST_INVALID: {
					qWarning() << "Recieved a Field with Type 'Invalid'";
					continue;
				}
				break;
			};
		}
	}
	if (updatedfields.size() == 0)
		return;

	qDebug() << "Updated Config Fields:";
	for (int i = 0; i < updatedfields.size(); i++) {
		qDebug() << qPrintable(updatedfields.at(i));
	}

	//newvals->addStringValue(SRVCAP_ENDPT_SERIAL, deviceID);
	//GlobalDevices[deviceID]->replaceVarStorageValue(SRVCAP_ENDPT_VARS, newvals);
	//this->tdm->updateDevice(item);
	this->tdm->updateDeviceConfig(newvals);
	emit updateConfig(deviceID.c_str(), updatedfields);
}

void QtiHanClient::HandleAddConfig(MessageBus item) {
	VarStorage msg = item->getNewConfig();
	HashVals hv;
	if (!msg->getHashValue("ConfigDescriptor", hv)) {
		qWarning() << "Can't get ConfigDescriptor for HandleAddConfig";
		return;
	}
	std::string deviceID = item->getSource();
	/* Regardless of whats going on, add this new End Point to the Global DeviceMap */
	if (!GlobalDevices.contains(deviceID)) {
		qWarning() << "Cannot Find device " << QString::fromStdString(deviceID) << " in GlobalDevices List for HandleAddConfig";
		return;
	}
	VarStorage config;
	if (GlobalDevices[deviceID]->getVarStorageValue(SRVCAP_ENDPT_CONFIG_DESC, config) == false) {
		qWarning("Can't get End Point Config from GloablDevices");
		return;
	}
	if (config->getSize(boost::get<std::string>(hv["Name"])) > 0) {
		qWarning() << "ConfigDescriptor for Field " << QString::fromStdString(boost::get<std::string>(hv["Name"])) << " Already Exists";
		return;
	}
	if (!config->addHashValue(boost::get<std::string>(hv["Name"]), hv)) {
		qWarning() << "Failed to add ConfigDescriptor for field " << QString::fromStdString(boost::get<std::string>(hv["Name"]));
		return;
	}
	std::cout << "HandleAddConfig: " << deviceID << " Field:" << boost::get<std::string>(hv["Name"]) << std::endl;
	this->tdm->addDeviceConfigDescriptors(deviceID, msg);
	emit newDeviceConfig(QString::fromStdString(deviceID), QString::fromStdString(boost::get<std::string>(hv["Name"])));
}
void QtiHanClient::HandleAddVar(MessageBus item) {
	VarStorage msg = item->getNewVar();
	HashVals hv;
	if (!msg->getHashValue("VarDescriptor", hv)) {
		qWarning() << "Can't get ConfigDescriptor for HandleAddVar";
		return;
	}
	std::string deviceID = item->getSource();
	/* Regardless of whats going on, add this new End Point to the Global DeviceMap */
	if (!GlobalDevices.contains(deviceID)) {
		qWarning() << "Cannot Find device " << QString::fromStdString(deviceID) << " in GlobalDevices List for HandleAddVar";
		return;
	}
	VarStorage config;
	if (GlobalDevices[deviceID]->getVarStorageValue(SRVCAP_ENDPT_VARS_DESC, config) == false) {
		qWarning("Can't get End Point Config from GloablDevices");
		return;
	}
	if (config->getSize(boost::get<std::string>(hv["Name"])) > 0) {
		qWarning() << "ConfigDescriptor for Field " << QString::fromStdString(boost::get<std::string>(hv["Name"])) << " Already Exists";
		return;
	}
	if (!config->addHashValue(boost::get<std::string>(hv["Name"]), hv)) {
		qWarning() << "Failed to add ConfigDescriptor for field " << QString::fromStdString(boost::get<std::string>(hv["Name"]));
		return;
	}
	std::cout << "HandleAddVar: " << deviceID << " Field:" << boost::get<std::string>(hv["Name"]) << std::endl;
	this->tdm->addDeviceVarDescriptors(deviceID, msg);
	emit newDeviceVar(QString::fromStdString(deviceID), QString::fromStdString(boost::get<std::string>(hv["Name"])));
}
void QtiHanClient::HandleDelConfig(MessageBus item) {
	VarStorage msg = item->getDelConfig();
	std::string field;
	if (!msg->getStringValue("DelConfig", field)) {
		qWarning() << "Can't get DelConfig Field for HandleDelConfig";
		return;
	}
	std::string deviceID = item->getSource();
	/* Regardless of whats going on, add this new End Point to the Global DeviceMap */
	if (!GlobalDevices.contains(deviceID)) {
		qWarning() << "Cannot Find device " << QString::fromStdString(deviceID) << " in GlobalDevices List for HandleDelConfig";
		return;
	}
	VarStorage config;
	if (GlobalDevices[deviceID]->getVarStorageValue(SRVCAP_ENDPT_CONFIG_DESC, config) == false) {
		qWarning("Can't get End Point Config from GloablDevices");
		return;
	}
	if (config->getSize(field) <= 0) {
		qWarning() << "ConfigDescriptor for Field " << QString::fromStdString(field) << " does not exist";
		return;
	}
	if (!config->delValue(field)) {
		qWarning() << "Can't Delete ConfigDescriptor for Field " << QString::fromStdString(field);
		return;
	}
	std::cout << "HandleDelConfig: " << deviceID << " Field:" << field << std::endl;
	this->tdm->delDeviceConfigDescriptors(deviceID, field);
	emit delDeviceConfig(QString::fromStdString(deviceID), QString::fromStdString(field));
}
void QtiHanClient::HandleDelVar(MessageBus item) {
	VarStorage msg = item->getDelVar();
	std::string field;
	if (!msg->getStringValue("delVar", field)) {
		qWarning() << "Can't get delVar Field for HandleDelVar";
		return;
	}
	std::string deviceID = item->getSource();
	/* Regardless of whats going on, add this new End Point to the Global DeviceMap */
	if (!GlobalDevices.contains(deviceID)) {
		qWarning() << "Cannot Find device " << QString::fromStdString(deviceID) << " in GlobalDevices List for HandleDelVar";
		return;
	}
	VarStorage config;
	if (GlobalDevices[deviceID]->getVarStorageValue(SRVCAP_ENDPT_VARS_DESC, config) == false) {
		qWarning("Can't get End Point Config from GloablDevices");
		return;
	}
	if (config->getSize(field) <= 0) {
		qWarning() << "ConfigDescriptor for Field " << QString::fromStdString(field) << " does not exist";
		return;
	}
	if (!config->delValue(field)) {
		qWarning() << "Can't Delete ConfigDescriptor for Field " << QString::fromStdString(field);
		return;
	}

	std::cout << "HandleDelVar: " << deviceID << " Field:" << field << std::endl;
	this->tdm->delDeviceVarDescriptors(deviceID, field);
	emit delDeviceConfig(QString::fromStdString(deviceID), QString::fromStdString(field));
}



void QtiHanClient::sendMessage(MessageBus msg) {
	this->mh->sendMessage(msg);
}

void QtiHanClient::HandleTermTypeMappings(MessageBus msg) {
	VarStorage val1 = msg->getSetup();
	VarStorage vals;
	if (!val1->getVarStorageValue("TermTypes", vals)) {
		qWarning() << "Can't get TermTypes from Setup Message";
		return;
	}
	/* Convert to QMap */
	/*       VarType         TermName        VarID    VarValue */
	/* QMap< QString, QMap < QString, QMap < QString, QVariant > > > Term_Map_t; */
	std::vector<std::string> *VarTypes = vals->getFields();
	for (std::vector<std::string>::iterator iter = VarTypes->begin(); iter != VarTypes->end(); ++iter) {
		/* (*iter) = VarType */
		QString VarType((*iter).c_str());
		VarContainerFactory(Vars);
		if (!vals->getVarStorageValue((*iter), Vars)) {
			qWarning() << "Could not Get TermName Variable: " << (*iter).c_str();
			//qWarning() << vals;
			continue;
		}
		QMap<QString, QMap < QString, QVariant > > QTermVals;
		std::vector<std::string> *TermNames = Vars->getFields();
		for (std::vector<std::string>::iterator TermIter = TermNames->begin(); TermIter != TermNames->end(); TermIter++) {
			HashVals VarValues;
			//std::cout << (*TermIter) << std::endl;
			if (!Vars->getHashValue((*TermIter), VarValues)) {
				qWarning() << "Could not get Term Values for Term " << (*TermIter).c_str();
				//qWarning() << Vars;
				continue;
			}
			QMap<QString, QVariant> QTermVars;
			std::map<std::string, HashValsVariant_t>::const_iterator hvit;
			for (hvit = VarValues.begin(); hvit != VarValues.end(); hvit++) {
				QTermVars.insert((*hvit).first.c_str(), QVariant(boost::get<std::string>((*hvit).second).c_str()));
				//cout << "\t\t" << (*hvit).first.c_str() << std::endl;
			}
			QTermVals.insert((*TermIter).c_str(), QTermVars);
		}
		this->TermTypeMappings.insert((*iter).c_str(), QTermVals);

	}
}

void QtiHanClient::HandleClientInform(MessageBus msg) {
	VarStorage clninfo;
	msg->getSetup()->getVarStorageValue(MSGB_SETUP_CLIENTINFORM, clninfo);
	if (clninfo->getSize() == 0) {
		qWarning() << "ClientInfo Structure is empty";
		exit(-1);
	}
	this->myinfo = clninfo;
}
QString QtiHanClient::getMyDeviceID() {
	if (this->myinfo->getSize(SRVCAP_ENDPT_SERIAL) == 0) {
		qDebug() << "Missing Serial Numer";
		return QString();
	}
	std::string id;
	this->myinfo->getStringValue(SRVCAP_ENDPT_SERIAL, id);
	return QString(id.c_str());
}
