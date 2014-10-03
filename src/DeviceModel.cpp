/****************************************************************************
 **
 **
 ****************************************************************************/
#include <QtCore/QFileInfo>
#include <QtCore/QSharedPointer>
#include <boost/algorithm/string.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "QtiHanClient/DeviceModel.h"
#include "QtiHanClient/QtiHanClient.h"
#include "iHanClient/MsgTypes.hpp"

using namespace boost;

DeviceItem::DeviceItem(VarStorage data, DeviceItem *parent) {
	parentItem = parent;
	itemData = data;
//        cout << "new DeviceItem: Data:" << data << " parent: "  << parent << " this: " << this << std::endl;
}
DeviceItem::~DeviceItem() {
	qDeleteAll(childItems);
}
void DeviceItem::setData(VarStorage data) {
	itemData = data;
}
void DeviceItem::appendChild(DeviceItem *item) {
//cout << "appendChild" << std::endl;
	childItems.append(item);
}
DeviceItem *DeviceItem::child(int row) {
//cout << "row: " << row << std::endl;
	return childItems.value(row);
}
int DeviceItem::childCount() const {
	//cout << "count: " << childItems.count() << std::endl;
	return childItems.count();
}
int DeviceItem::row() const {
//cout << "getrow()" << std::endl;
	if (parentItem)
		return parentItem->childItems.indexOf(const_cast<DeviceItem*>(this));

	return 0;
}
int DeviceItem::columnCount() const {
	//cout << "getcolumncount" << DeviceModel_t::DEVICEMODEL_T_COLUMNS_MAX << std::endl;
	return DeviceModel_t::DEVICEMODEL_T_COLUMNS_MAX;
}
VarStorage DeviceItem::data() const {
//cout << "getdata" << std::endl;
	return itemData;
}
DeviceItem *DeviceItem::parent() {
//cout << "getparent" << std::endl;
	return parentItem;
}
bool DeviceItem::removeChild(DeviceItem *item) {
	int i = this->childItems.indexOf(item);
	this->childItems.removeAt(i);
	return true;
}

bool DeviceItem::remove() {
	return this->parentItem->removeChild(this);
}


DeviceModel_t::DeviceModel_t(QObject *parent) :
		QAbstractItemModel(parent) {
	setParent(parent);
#if QT_VERSION < 0x050000
	QHash<int, QByteArray> roles;
	roles[NameRole] = "name";
	roles[SerialRole] = "serial";
	roles[TypeRole] = "type";
	roles[ParentSerialRole] = "parentserial";
	roles[ConfigRole] = "config";
	roles[ConfigDescRole] = "configdesc";
	roles[VariablesRole] = "variables";
	roles[VariablesDescRole] = "variablesdesc";
	setRoleNames(roles);
#endif
	//connect(this, SIGNAL(sendMsg(MsgType, VarStorage)), parent, SLOT(sendMessage(MsgType, VarStorage)));
	VarContainerFactory(emptydev);
	this->rootItem = new DeviceItem(emptydev);
}

DeviceModel_t::~DeviceModel_t() {
	delete rootItem;
}

QModelIndex DeviceModel_t::index(int row, int column, const QModelIndex &parent) const {
	if (!hasIndex(row, column, parent)) {
		//qDebug() << "DeviceModel_t::Index " << row << " " << column << " not ok";
		return QModelIndex();
	}
	DeviceItem *parentItem;
	if (!parent.isValid())
		parentItem = rootItem;
	else
		parentItem = static_cast<DeviceItem*>(parent.internalPointer());

	DeviceItem *childItem = parentItem->child(row);
	if (childItem)
		return createIndex(row, column, childItem);
	else

		return QModelIndex();
}
QModelIndex DeviceModel_t::parent(const QModelIndex &index) const {
	if (!index.isValid())
		return QModelIndex();

	DeviceItem *childItem = static_cast<DeviceItem*>(index.internalPointer());
	DeviceItem *parentItem = childItem->parent();

	if (parentItem == rootItem)
		return QModelIndex();

	return createIndex(parentItem->row(), 0, parentItem);
}

QVariant DeviceModel_t::headerData(int section, Qt::Orientation orientation, int role) const {
	//Q_UNUSED(orientation);
	if (role == Qt::DisplayRole) {
		if (orientation == Qt::Horizontal) {
			switch (section) {
				case DeviceName:
					return "Name";
				case DeviceSerial:
					return "Serial";
				case DeviceType:
					return "Type";
				case DeviceParentSerial:
					return "Parent Serial";
				case DeviceConfig:
					return "Config";
				case DeviceConfigDescriptor:
					return "Config Descriptors";
				case DeviceVariable:
					return "Variables";
				case DeviceVariableDescriptor:
					return "Variable Descriptors";
				case DeviceVarHelper:
					return "Device Variable Helper";
				case DeviceConfigHelper:
					return "Device Config Helper";
				default:
					return "unknown";
			}
		} else if (orientation == Qt::Vertical) {
			return QVariant();
		}
	} else if (role == Qt::SizeHintRole) {
		return QVariant();
	}
	return QVariant();
}
int DeviceModel_t::columnCount(const QModelIndex & /*parent*/) const {
	return DEVICEMODEL_T_COLUMNS_MAX;
}

QModelIndex DeviceModel_t::FindDevice(QString serial) {
	QModelIndexList parentnames = this->match(this->index(0, 0, QModelIndex()), (int)DeviceModel_t::SerialRole, serial,  DeviceModel_t::DeviceSerial, Qt::MatchRecursive);
	if (parentnames.count() > 0)
		return parentnames[0];
	else {
		qWarning() << "Can't find Serial in DeviceModel: " << serial;
		return QModelIndex();
	}
}

void DeviceModel_t::addDevice(const VarStorage &Device) {
	std::string deviceid;
	std::string deviceName;
	std::string parentid;
	long type;
	Device->getStringValue("EndPtName", deviceName);
	Device->getLongValue(SRVCAP_ENDPT_TYPE, type);
//         qDebug() << QString("Adding Device ").append(deviceName.c_str()) << " to DeviceModel of type " << type;
	if (Device->getStringValue(SRVCAP_ENDPT_SERIAL, deviceid) == true) {
		if (!m_devices.contains(deviceid)) {
			/* does it have a parent? */
			Device->getStringValue(SRVCAP_PARENT_SERIAL, parentid);
			DeviceItem *item;
			beginInsertRows(this->FindDevice(QString(parentid.c_str())), rowCount(this->FindDevice(QString(parentid.c_str()))), rowCount(this->FindDevice(QString(parentid.c_str()))));
			if (m_devices.contains(parentid)) {
				DeviceItem *parent = m_devices.value(parentid);
				item = new DeviceItem(Device, m_devices.value(parentid));
				parent->appendChild(item);
			} else {
				item = new DeviceItem(Device, this->rootItem);
				//cout << "lost device: " << item << std::endl;
				this->rootItem->appendChild(item);
			}
			m_devices.insert(deviceid, item);
//			 qDebug() << QString("Added Device ").append(deviceName.c_str()) << " to DeviceModel of type " << type;
			endInsertRows();
		} else {
			qWarning() << QString("Device ").append(deviceName.c_str()) << " already present in DeviceModel";
		}
	} else {
		qWarning() << "Can't Find Device Serial Number in DeviceModel";
	}
}
void DeviceModel_t::delDevice(const std::string deviceid) {
	QModelIndexList Items = this->match(this->index(0, 0, QModelIndex()), (int) SerialRole, QString::fromStdString(deviceid), 2, Qt::MatchRecursive);
	if (Items.count() <= 0) {
		qWarning() << "Can't find Device in match list for Model";
		return;
	}
	if (m_devices.contains(deviceid)) {
		DeviceItem *delitem = m_devices.value(deviceid);
		if (delitem->childCount() > 0) {
			qWarning() << "Can't Remove Device as it still has Children";
			return;
		}
		beginRemoveRows(Items[0].parent(), Items[0].row(), Items[0].row());
		/* XXX Confirm the sharedpointer does actually delete here */
		delitem->remove();
		m_devices.remove(deviceid);
		qDebug() << QString("Deleting Device ").append(deviceid.c_str()) << " to DeviceModel";
		endRemoveRows();
	} else {
		qWarning() << "Can't Find Device Serial Number in DeviceModel";
	}
}

void DeviceModel_t::updateDevice(const VarStorage &device) {
	std::string from;
	if (!device->getStringValue(SRVCAP_ENDPT_SERIAL, from)) {
		//cout << "Cant Find Device" << std::endl;
		return;
	}
	QModelIndexList Items = this->match(this->index(0, 0, QModelIndex()), (int) SerialRole, QString::fromStdString(from), 2, Qt::MatchRecursive);
	if (Items.count() <= 0) {
		qWarning() << "Can't find Device in match list for Model";
		return;
	}
	if (this->m_devices.contains(from)) {
		emit dataChanged(Items[0], Items[0]);
	} else {
		qWarning() << "Can't find Device in m_devices list for Update";
	}
}

void DeviceModel_t::updateDeviceConfig(const VarStorage &device) {
	std::string from;
	if (!device->getStringValue(SRVCAP_ENDPT_SERIAL, from)) {
		//cout << "Cant Find Device" << std::endl;
		return;
	}
	QModelIndexList Items = this->match(this->index(0, 0, QModelIndex()), (int) SerialRole, QString::fromStdString(from), 2, Qt::MatchRecursive);
	if (Items.count() <= 0) {
		qWarning() << "Can't find Device in match list for Model";
		return;
	}
	if (this->m_devices.contains(from)) {
		emit dataChanged(Items[0], Items[0]);
	} else {
		qWarning() << "Can't find Device in m_devices list for Update";
	}
}
int DeviceModel_t::rowCount(const QModelIndex & parent) const {
	DeviceItem *parentItem;
	if (parent.column() > 0)
		return 0;
	if (!parent.isValid())
		parentItem = rootItem;
	else
		parentItem = static_cast<DeviceItem*>(parent.internalPointer());
	return parentItem->childCount();
}

Qt::ItemFlags DeviceModel_t::flags(const QModelIndex &index) const {
	if (!index.isValid())
		return 0;
	switch (index.column()) {
		case DeviceConfig:
		case DeviceVariable:
			return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
			break;
		default:
			return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
			break;
	}
	return 0;
}

QVariant DeviceModel_t::data(const QModelIndex & index, int role) const {
	if (index.row() < 0 || index.row() > m_devices.count()) {
		//qWarning() << "Index (Row) out of Range" << index.row();
		return QVariant();
	}
	if (index.column() < 0 || index.column() > DEVICEMODEL_T_COLUMNS_MAX) {
		//qWarning() << "Index (Column) out of Range" << index.column();
		return QVariant();
	}
	if (!index.isValid()) {
		//qWarning() << "Invalid Index";
		return QVariant();
	}
	/*     std::string deviceID = m_devices[index.row()];
	 if (!GlobalDevices.contains(deviceID)) {
	 qWarning() << QString("Device ").append(deviceID.c_str()) << " Doesn't Existing in GlobalDeviceMap";
	 return QVariant();
	 }
	 const VarStorage Device = GlobalDevices.value(deviceID);
	 */
	const DeviceItem *item = static_cast<DeviceItem*>(index.internalPointer());
	const VarStorage Device = item->data();
//cout << "want " << item << " stored in " << Device << std::endl;
	if (Device.get() == NULL) {
		qCritical() << "Device Is Empty?";
		return QVariant();
	}
//     qDebug() << "get Role" << role;
//     Device->printToStream();
	if (role == Qt::DisplayRole) {
		switch (index.column()) {
			case DeviceName: {
				VarStorage Config;
				if (Device->getVarStorageValue(SRVCAP_ENDPT_CONFIG, Config)) {
					std::string name;
					if (Config->getStringValue("Name", name)) {
						//qDebug() << QString("Wanted Name of Device ").append(deviceID.c_str()) << name.c_str();
						return QString(name.c_str());
					} else
						return QString();
				} else {
					qWarning("Can't get Config from Device");
					return QString();
				}
			}
			break;
			case DeviceSerial: {
				//qDebug() << QString("Wanted Serial of Device ").append(deviceID.c_str());
				std::string name;
				Device->getStringValue(SRVCAP_ENDPT_SERIAL, name);
				return QString(name.c_str());
			}
			break;
			case DeviceType: {
				long type;
				Device->getLongValue(SRVCAP_ENDPT_TYPE, type);
				//qDebug() << QString("Wanted Type of Device ").append(deviceID.c_str()) << type;
				return (qlonglong) type;
			}
			break;
			case DeviceParentSerial: {
				std::string name;
				Device->getStringValue(SRVCAP_PARENT_SERIAL, name);
				//qDebug() << QString("Wanted ParentSerial of Device ").append(deviceID.c_str()) << name.c_str();
				return QString(name.c_str());
			}
			break;
			case DeviceConfig: {
				//qDebug() << QString("Wanted Config of Device ").append(deviceID.c_str());
				VarStorage var;
				Device->getVarStorageValue(SRVCAP_ENDPT_CONFIG, var);
				return QVariant::fromValue<VarStorage>(var);
			}
			break;
			case DeviceConfigDescriptor: {
				//qDebug() << QString("Wanted ConfigDesc of Device ").append(deviceID.c_str());
				VarStorage var;
				Device->getVarStorageValue(SRVCAP_ENDPT_CONFIG_DESC, var);
				return QVariant::fromValue<VarStorage>(var);
			}
			break;
			case DeviceVariable: {
				//qDebug() << QString("Wanted Variables of Device ").append(deviceID.c_str());
				VarStorage var;
				Device->getVarStorageValue(SRVCAP_ENDPT_VARS, var);
				return QVariant::fromValue<VarStorage>(var);
			}
			break;
			case DeviceVariableDescriptor: {
				//qDebug() << QString("Wanted VariablesDesc of Device ").append(deviceID.c_str());
				VarStorage var;
				Device->getVarStorageValue(SRVCAP_ENDPT_VARS_DESC, var);
				//var->printToStream();
				return QVariant::fromValue<VarStorage>(var);
			}
			break;
			case DeviceVarHelper: {
				VarStorage var, vardesc;
				std::string Serial;
				Device->getVarStorageValue(SRVCAP_ENDPT_VARS_DESC, vardesc);
				Device->getVarStorageValue(SRVCAP_ENDPT_VARS, var);
				Device->getStringValue(SRVCAP_ENDPT_SERIAL, Serial);
				VarStorageHelper vse(new VarStorageHelper_t(var, vardesc, QString::fromStdString(Serial)));
				return QVariant::fromValue<VarStorageHelper>(vse);
			}
			break;
			case DeviceConfigHelper: {
				VarStorage var, vardesc;
				std::string Serial;
				Device->getVarStorageValue(SRVCAP_ENDPT_CONFIG_DESC, vardesc);
				Device->getVarStorageValue(SRVCAP_ENDPT_CONFIG, var);
				Device->getStringValue(SRVCAP_ENDPT_SERIAL, Serial);
				VarStorageHelper vse(new VarStorageHelper_t(var, vardesc, QString::fromStdString(Serial)));
				return QVariant::fromValue<VarStorageHelper>(vse);
			}
			break;
		}
	} else if (role == SerialRole) {
		std::string name;
		Device->getStringValue(SRVCAP_ENDPT_SERIAL, name);
		return QString(name.c_str());
	} else {
		/* Role */
		return QVariant();
	}
	return QVariant();
}

bool DeviceModel_t::setData (const QModelIndex & index, const QVariant &value, int role)  {
	if (index.row() < 0 || index.row() > m_devices.count()) {
		//qWarning() << "Index (Row) out of Range";
		return false;
	}
	if (index.column() < 0 || index.column() > 8) {
		//qWarning() << "Index (Column) out of Range";
		return false;
	}
	if (!index.isValid()) {
		//qWarning() << "Invalid Index";
		return false;
	}

	const DeviceItem *item = static_cast<DeviceItem*>(index.internalPointer());
	const VarStorage Device = item->data();
	if (Device.get() == NULL) {
		qCritical() << "Device Is Empty?";
		return false;
	}
	std::string DeviceSerialNo;
	Device->getStringValue(SRVCAP_ENDPT_SERIAL, DeviceSerialNo);
	qDebug() << "Called SetData on " << DeviceSerialNo.c_str() << " with " << value << " for " << index.column();
	VarStorage var2 = value.value<VarStorage>();
	//var2->printToStream();
	if (role == Qt::EditRole) {
		switch (index.column()) {
			case DeviceName: {
				qWarning() << "setData on DeviceName - Not Editable";
				return false;
			}
			break;
			case DeviceSerial: {
				qWarning() << "setData on DeviceSerial - Not Editable";
				return false;
			}
			break;
			case DeviceType: {
				qWarning() << "setData on DeviceType - Not Editable";
				return false;
			}
			break;
			case DeviceParentSerial: {
				qWarning() << "setData on DeviceParentSerial - Not Editable";
				return false;
			}
			break;
			case DeviceConfig: {
				qDebug() << "setData on DeviceConfig - Processing";
				VarStorage var = value.value<VarStorage>();
				MessageBusFactory(Config);
				if (Config->createSetConfig(var, QtiHanClient::Get()->getMyDeviceID().toStdString())) {
					Config->setDestination(DeviceSerialNo);
					emit sendMsg(Config);
				} else {
					qWarning() << "Couldn't Create a SetConfig MesssageBus";
					return false;
				}
				return true;
#if 0
				//qDebug() << QString("Wanted Config of Device ").append(deviceID.c_str());
				VarStorage var;
				Device->getVarStorageValue(SRVCAP_ENDPT_CONFIG, var);
				return QVariant::fromValue<VarStorage>(var);
#endif
			}
			break;
			case DeviceConfigDescriptor: {
				qWarning() << "setData on DeviceConfigDescriptor - Not Editable";
				return false;
			}
			break;
			case DeviceVariable: {
				qDebug() << "setData on DeviceVariable - Processing";
				VarStorage var = value.value<VarStorage>();
				MessageBusFactory(Var);
				if (Var->createSetVar(var, QtiHanClient::Get()->getMyDeviceID().toStdString())) {
					Var->setDestination(DeviceSerialNo);
					emit sendMsg(Var);
				} else {
					qWarning() << "Couldn't Create a SetVar Message";
					return false;
				}
				return true;
#if 0
				//qDebug() << QString("Wanted Variables of Device ").append(deviceID.c_str());
				VarStorage var;
				Device->getVarStorageValue(SRVCAP_ENDPT_VARS, var);
				return QVariant::fromValue<VarStorage>(var);
#endif
			}
			break;
			case DeviceVariableDescriptor: {
				qWarning() << "setData on DeviceVariableDescriptor - Not Editable";
				return false;
			}
			break;
		}
	}
	return false;
}

void DeviceModel_t::setData(QString serial, QString name, QVariant value, bool sync) {
	Q_UNUSED(sync);
	if (this == NULL) {
		/* WTF? */
		qWarning() << QString("Trying to set " + serial + " device (name) on a NULL Pointer?");
		return;
	}
	qDebug() << QString("Setting " + name + "(" + serial + ") to ") << value;
#if 0
	/* first, find the device */
	if (m_devices.contains(serial.toStdString())) {
		VarContainerFactory(device);
		device = GlobalDevices.value(serial.toStdString());
		/* then, find the role */
		QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();
		QHashIterator<int, QByteArray> i(roles);
		while (i.hasNext()) {
			i.next();
			//qDebug() << i.key() << ": " << QString(i.value()).toStdString() << endl;
			if (i.value() == name.data()) {
				//this->setData(this->index(m_devices.indexOf(serial.toStdString())), value, i.key);
				/* we are setting a exported variable... */
				return;
			}
		}
		/* if we get here, its either in the config or variables export */
		/* try variables first */
		VarContainerFactory(variable);
		if (device->getVarStorageValue(SRVCAP_ENDPT_VARS, variable) == true) {
			if (variable->getType(name.toStdString()) != ST_INVALID) {
				VarContainerFactory(newvariable);
				switch (variable->getType(name.toStdString())) {
					case ST_STRING:
					newvariable->addStringValue(name.toStdString(), value.toString().toStdString());
					break;
					case ST_INT:
					newvariable->addIntValue(name.toStdString(), value.toInt());
					break;
					case ST_LONG:
					newvariable->addLongValue(name.toStdString(), value.toLongLong());
					break;
					case ST_LONGLONG:
					newvariable->addLongLongValue(name.toStdString(), value.toLongLong());
					break;
					case ST_FLOAT:
					newvariable->addFloatValue(name.toStdString(), value.toFloat());
					break;
					case ST_HASH:
					qWarning() << "Hash Not implemented in setData";
					break;
					case ST_BOOL:
					newvariable->addBoolValue(name.toStdString(), value.toBool());
					break;
					case ST_DATETIME:
					qWarning() << "DateTime not implemented in setData";
					break;
					case ST_VARSTORAGE:
					qWarning() << "VarStorage not implemented in setData";
					break;
					case ST_INVALID:
					qWarning() << "setData for a INVALID???";
					break;
				}
				if (sync) {
					/* send it out straight away using MSG_WHAT_ENDPNT */
					VarContainerFactory(output);
					output->addStringValue(SRVCAP_ENDPT_SERIAL, serial.toStdString());
					output->addVarStorageValue(SRVCAP_ENDPT_VARS, newvariable);
					this->sendMsg(output);
				}
			}
		}
	}
#endif
}

VarStorageHelper_t::VarStorageHelper_t(VarStorage val, VarStorage desc, QString Serial) {
	this->values = val;
	this->descriptor = desc;
	this->Serial = Serial;
}
VarStorageHelper_t::~VarStorageHelper_t() {

}

QString VarStorageHelper_t::getSerial() {
	return this->Serial;
}
int VarStorageHelper_t::getRealType(QString name) {
	HashVals hv;
	//std::cout << "GetType" << this->descriptor << std::endl;
	if (!this->descriptor->getHashValue(name.toStdString(), hv)) {
		qWarning() << "Couldn't get ConfigDescriptor for getType:" << name;
		return ST_INVALID;
	}
	return boost::get<int>(hv["Type"]);
}

int VarStorageHelper_t::getType(QString name) {
	HashVals hv;
	//std::cout << "GetType" << this->descriptor << std::endl;
	if (!this->descriptor->getHashValue(name.toStdString(), hv)) {
		qWarning() << "Couldn't get ConfigDescriptor for getType:" << name;
		return QVariant::Invalid;
	}

	//std::cout << "GetType: " << hv << std::endl;
	switch (boost::get<int>(hv["Type"])) {
		case ST_STRING:
			return QVariant::String;
		case ST_INT:
			return QVariant::Int;
		case ST_LONG:
			return QVariant::LongLong;
		case ST_LONGLONG:
			return QVariant::LongLong;
		case ST_FLOAT:
			return QVariant::Double;
		case ST_HASH:
			return qMetaTypeId<HashVals>();
		case ST_BOOL:
			return QVariant::Bool;
		case ST_DATETIME:
			return QVariant::DateTime;
		case ST_VARSTORAGE:
			return QMetaType::type("VarStorage");
		case ST_LIST:
			return QMetaType::type("VarList_t");
		case ST_INVALID:
			return QVariant::Invalid;
	}
	qWarning() << "Unhandled type in getType: " << boost::get<int>(hv["Type"]);
	return QVariant::Invalid;
}

int VarStorageHelper_t::getType(QString name, QString element) {
	if (this->getType(name) == ST_HASH) {
		HashVals hash;
		if (!this->values->getHashValue(name.toStdString(), hash)) {
			qWarning() << "Couldn't get HashVal for " << name;
			return QVariant::Invalid;
		}
		if (hash[element.toStdString()].type() == typeid(std::string)) {
			return QVariant::String;
		} else if (hash[element.toStdString()].type() == typeid(int)) {
			return QVariant::Int;
		} else if (hash[element.toStdString()].type() == typeid(long)) {
			return QVariant::LongLong;
		} else if (hash[element.toStdString()].type() == typeid(long long)) {
			return QVariant::LongLong;
		} else if (hash[element.toStdString()].type() == typeid(float)) {
			return QVariant::Double;
		} else if (hash[element.toStdString()].type() == typeid(HashVals)) {
			return QMetaType::type("HashVals");
		} else if (hash[element.toStdString()].type() == typeid(bool)) {
			return QVariant::Bool;
		} else if (hash[element.toStdString()].type() == typeid(boost::posix_time::ptime)) {
			return QVariant::DateTime;
		} else if (hash[element.toStdString()].type() == typeid(VarStorage)) {
			return QMetaType::type("VarStorage");
		} else if (hash[element.toStdString()].type() == typeid(ListOptions)) {
			return QMetaType::type("VarList_t");
		} else {
			qWarning() << "Unknown Hash Type";
			return QVariant::Invalid;
		}
	} else if (this->getType(name) == ST_VARSTORAGE) {
		VarContainerFactory(var);
		if (!this->values->getVarStorageValue(name.toStdString(), var)) {
			qWarning() << "Couldn't get VarStorage for " << name;
			return QVariant::Invalid;
		}
		switch (var->getType(name.toStdString())) {
			case ST_STRING:
				return QVariant::String;
			case ST_INT:
				return QVariant::Int;
			case ST_LONG:
				return QVariant::LongLong;
			case ST_LONGLONG:
				return QVariant::LongLong;
			case ST_FLOAT:
				return QVariant::Double;
			case ST_HASH:
				return QMetaType::type("HashVals");
			case ST_BOOL:
				return QVariant::Bool;
			case ST_DATETIME:
				return QVariant::DateTime;
			case ST_VARSTORAGE:
				return QMetaType::type("VarStorage");
			case ST_LIST:
				return QVariant::Int;
			case ST_INVALID:
				return QVariant::Invalid;
		}
	}
	qWarning() << name << " is not a HashVals or VarStorage Container";
	return QVariant::Invalid;
}
QVariant VarStorageHelper_t::getValue(QString name, int pos) {
	/* we support : as a field seperator. So if there is a : in the name,
	 * check if the value type is either a hash or a varstorage
	 */
	VarContainerFactory(val);
	val = this->values;
	QString fieldName = name;
	QString containerName;
	if (name.contains(':')) {
		QStringList names = name.split(':');
		for (int i = 0; i < names.count(); i++) {
			if (val->getType(names[i].toStdString()) == ST_HASH) {
				/* if its a hash, it has to to be the 2nd last entry
				 * in the chain
				 */
				if (i == names.count() - 2) {
					fieldName = names[i];
					containerName = names[i + 1];
					break;
				} else {
					qWarning() << names[i] << " Not 2nd Last Entry for HashVals Type";
					return QVariant();
				}
			} else if (this->values->getType(names[i].toStdString()) == ST_VARSTORAGE) {
				val->getVarStorageValue(names[i].toStdString(), val);
			} else {
				cout << "its something else " << std::endl;
				if (i == names.count() - 1) {
					/* the last entry should always be a normal value,
					 * not a container
					 */
					if (val->getSize(names[i].toStdString()) > 0) {
						fieldName = names[i];
						break;
					} else {
						qWarning() << names[i] << " does not exist";
						return QVariant();
					}
				} else {
					qWarning() << names[i] << " is not a Non-Container Value";
					return QVariant();
				}
			}
		}

	}
	if ((this->getArrayMaxSize(fieldName) == 0) && (pos > 0)) {
		qWarning() << "Requested a ArrayVariable on a Non-Array: " << fieldName;
		return QVariant();
	} else if (this->getSize(fieldName) < pos) {
		qWarning() << "Out of Bounds Request for a Array Field: " << fieldName;
		return QVariant();
	}

	switch (val->getType(fieldName.toStdString())) {
		case ST_STRING:
			return QVariant(VSE.getString(val, fieldName, pos));
		case ST_INT:
			return QVariant(VSE.getInt(val, fieldName, pos));
		case ST_LONG:
			return QVariant(VSE.getLong(val, fieldName, pos));
		case ST_LONGLONG:
			return QVariant(VSE.getLongLong(val, fieldName, pos));
		case ST_FLOAT:
			return QVariant(VSE.getFloat(val, fieldName, pos));
		case ST_HASH:
			/* HASH's have to have : as a field Seperator
			 */
			if (name.contains(':')) {
				return QVariant(VSE.getHash(val, fieldName, containerName, pos));
			} else {
				//qWarning() << "Invalid fieldName for Hash Container: " << name;
				return QVariant(VSE.getHash(val, fieldName, pos));
			}
		case ST_BOOL:
			return QVariant(VSE.getBool(val, fieldName, pos));
		case ST_DATETIME:
			return QVariant(QDateTime::fromString(VSE.getTime(val, fieldName, pos), Qt::ISODate));
		case ST_VARSTORAGE:
			return QVariant(VSE.getVarStorage(val, fieldName, pos));
		case ST_LIST:
			return QVariant(VSE.getListSelection(val, fieldName, pos));
		case ST_INVALID:
			qWarning() << "Unknown Type Requested from getValue(name): " << name << " Type: " << val->getType(fieldName.toStdString());
			qWarning() << "fieldName: " << fieldName << ", Container: " << containerName << ", VarStorage: ";
			val->printToStream();
			;
			return QVariant();
	}
	qWarning() << "Invalid Type in getValue: " << val->getType(fieldName.toStdString());
	return QVariant();
}

QString VarStorageHelper_t::getName(QString name) {
	HashVals vals;
	if (this->descriptor->getHashValue(name.toStdString(), vals)) {
		return QString(boost::get<std::string>(vals["FriendlyName"]).c_str());
	} else {
		qWarning() << name << " does not exist in Descriptors (getName())";
		return QString();
	}
}

QString VarStorageHelper_t::getDescription(QString name) {
	HashVals vals;
	if (this->descriptor->getHashValue(name.toStdString(), vals)) {
		return QString(boost::get<std::string>(vals["Description"]).c_str());
	} else {
		qWarning() << name << " does not exist in Descriptors (getDescription())";
		return QString();
	}
}

qlonglong VarStorageHelper_t::getMin(QString name) {
	HashVals vals;
	if (this->descriptor->getHashValue(name.toStdString(), vals)) {
		return boost::get<long long>(vals["min"]);
	} else {
		qWarning() << name << " does not exist in Descriptors (getMin())";
		return 0;
	}
}

qlonglong VarStorageHelper_t::getMax(QString name) {
	HashVals vals;
	if (this->descriptor->getHashValue(name.toStdString(), vals)) {
		return boost::get<long long>(vals["max"]);
	} else {
		qWarning() << name << " does not exist in Descriptors (getMax())";
		return -1;
	}
}

QVariant VarStorageHelper_t::getDefault(QString name) {
	HashVals vals;
	if (this->descriptor->getHashValue(name.toStdString(), vals)) {
		switch (boost::get<int>(vals["type"])) {
			case ST_STRING:
			case ST_DATETIME:
				return QVariant((char *) boost::get<std::string>(vals["defaultstr"]).c_str());
			case ST_INT:
			case ST_LONG:
			case ST_LONGLONG:
			case ST_FLOAT:
			case ST_BOOL:
			case ST_LIST:
				return QVariant((long long) boost::get<long long>(vals["defaultnum"]));
			case ST_HASH:
			case ST_VARSTORAGE:
			case ST_INVALID:
				qWarning() << "Unsupported Type Requested from getDefault(name): " << name;
				return QVariant();
		}
	} else {
		qWarning() << name << " does not exist in Descriptors (getDescription())";
		return QVariant();
	}
	return QVariant();
}

int VarStorageHelper_t::getFlags(QString name) {
	HashVals vals;
	if (this->descriptor->getHashValue(name.toStdString(), vals)) {
		return boost::get<int>(vals["flags"]);
	} else {
		qWarning() << name << " does not exist in Descriptors (getFlags())";
		return -1;
	}
}

bool VarStorageHelper_t::getReadOnly(QString name) {
	HashVals vals;
	if (this->descriptor->getHashValue(name.toStdString(), vals)) {
		return (boost::get<int>(vals["flags"]) && TCF_READONLY);
	} else {
		qWarning() << name << " does not exist in Descriptors (getReadOnly())";
		return false;
	}
}

int VarStorageHelper_t::getGroup(QString name) {
	HashVals vals;
	if (this->descriptor->getHashValue(name.toStdString(), vals)) {
		return boost::get<long long>(vals["group"]);
	} else {
		qWarning() << name << " does not exist in Descriptors (getGroup())";
		return -1;
	}
}

QStringList VarStorageHelper_t::getItems() {
	return VSE.getFields(this->descriptor);
}

VarStorage VarStorageHelper_t::setValue(VarStorage updateVals, QString field, QVariant data, int pos) {
	if (!data.canConvert(static_cast<QVariant::Type>(this->getType(field)))) {
		qDebug() << "Cannot Convert " << data << " to Field " << field << " Type: " << this->getType(field);
		return updateVals;
	}
	if (pos > this->getArrayMaxSize(field)) {
		qWarning() << "trying to setValue on Array Variable greater than arraysize";
		return updateVals;
	}
	if ((this->getArrayMaxSize(field) > 0) && (pos > this->getSize(field)+ 1)) {
		qWarning() << "Cannot have a hole in a Array";
		return updateVals;
	}

	switch (this->getRealType(field)) {
		case ST_STRING:
			updateVals->replaceStringValue(field.toStdString(), data.toString().toStdString(), pos);
			break;
		case ST_INT:
			updateVals->replaceIntValue(field.toStdString(), data.toInt(), pos);
			break;
		case ST_LONG:
			updateVals->replaceLongValue(field.toStdString(), data.toLongLong(), pos);
			break;
		case ST_LONGLONG:
			updateVals->replaceLongValue(field.toStdString(), data.toLongLong(), pos);
			break;
		case ST_FLOAT:
			updateVals->replaceFloatValue(field.toStdString(), data.toFloat(), pos);
			break;
		case ST_HASH:
			updateVals->replaceHashValue(field.toStdString(), data.value<HashVals>(), pos);
			break;
		case ST_BOOL:
			updateVals->replaceBoolValue(field.toStdString(), data.toBool(), pos);
			break;
		case ST_DATETIME: {
			boost::posix_time::ptime dt;
			dt = boost::posix_time::from_iso_string(data.toDateTime().toString(Qt::ISODate).toStdString());
			updateVals->replaceTimeValue(field.toStdString(), dt, pos);
			break;
		}
		case ST_VARSTORAGE: {
			VarStorage vars = data.value<VarStorage>();
			updateVals->replaceVarStorageValue(field.toStdString(), vars, pos);
			break;
		}
		case ST_LIST: {
			if (data.type() == QVariant::Int) {
				updateVals->setListSelectedValue(field.toStdString(), data.toUInt(), pos);
			} else if (data.userType() == QMetaType::type("VarList_t")) {
				qWarning() << "Can't Update ValueList, Only Selection";
			}
			break;
		}
		case ST_INVALID:
			qDebug() << " SetValue Called on a ST_INVALID Field";
			break;
	}
	return updateVals;
}
QVariant VarStorageHelper_t::getListOptions(QString fieldName, int pos) {
	if ((this->getArrayMaxSize(fieldName) == 0) && (pos > 0)) {
		qWarning() << "Requested a ArrayVariable on a Non-Array: " << fieldName;
		return QVariant();
	} else if (this->getSize(fieldName) < pos) {
		qWarning() << "Out of Bounds Request for a Array Field: " << fieldName;
		return QVariant();
	}
	if (this->getType(fieldName) != ST_LIST) {
		qWarning() << "Field is not a List, so cant get ListOptions " << fieldName;
		return QVariant();
	}
	return QVariant(VSE.getList(this->values, fieldName, pos));
}




int VarStorageHelper_t::getArrayMaxSize(QString name) {
	HashVals vals;
	if (this->descriptor->getHashValue(name.toStdString(), vals)) {
		return boost::get<int>(vals["MaxSize"]);
	} else {
		qWarning() << name << " does not exist in Descriptors (getArraySize())";
		return -1;
	}
}
int VarStorageHelper_t::getSize(QString name) {
	return this->values->getSize(name.toStdString());
}
VarStorage VarStorageHelper_t::delValue(VarStorage UpdateVals, QString name, int pos) {
	if (this->getArrayMaxSize(name) == 0) {
		qDebug() << "Trying to delete a field on a Array: " << name;
		return UpdateVals;
	}
	if (this->getFlags(name) & TCF_FIXEDSIZE) {
		qDebug() << "Trying to delete a field on a Array with FixedSize Flag Set: " << name;
		return UpdateVals;
	}
	if (this->getSize(name) < pos) {
		qDebug() << "Trying to delete a field on a Array thats out of range: " << this->getSize(name) << " < " << pos;
		return UpdateVals;
	}
	/* ok - We are clear to delete it, add the delete Variable... */
	HashVals vals;
	vals["NAME"] = name.toStdString();
	vals["POS"] = pos;
	UpdateVals->addHashValue(SRVCAP_ENDPT_DELETEARRAYENTRY, vals);
	this->values->delValue(name.toStdString(), pos);
	return UpdateVals;
}

int VarStorageHelper_t::getVarType(QString name) {
	HashVals vals;
	if (this->descriptor->getHashValue(name.toStdString(), vals)) {
		return boost::get<int>(vals["VarType"]);
	} else {
		qWarning() << name << " does not exist in Descriptors (getVarType())";
		return -1;
	}
}

