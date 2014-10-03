/****************************************************************************
**
**
****************************************************************************/

#ifndef DEVICEMODEL_H
#define DEVICEMODEL_H
#define QT_SHAREDPOINTER_TRACK_POINTERS 1
#include <QtCore/QAbstractListModel>
#include <QtCore/QStringList>
#include <QtCore/QDebug>
#include "iHanClient/varcontainer.hpp"
#include "iHanClient/MessageBus.hpp"
#include "iHanClient/MsgTypes.hpp"

class DeviceItem
{
public:
        DeviceItem(VarStorage data, DeviceItem *parent = 0);
        ~DeviceItem();
        void setData(VarStorage data);
        void appendChild(DeviceItem *child);
        DeviceItem *child(int row);
        int childCount() const;
        int columnCount() const;
        VarStorage data() const;
        int row() const;
        DeviceItem *parent();
        bool removeChild(DeviceItem *item);
        bool remove();
private:
        QList<DeviceItem *> childItems;
        VarStorage itemData;
        DeviceItem *parentItem;
};


class DeviceModel_t : public QAbstractItemModel {
    Q_OBJECT
public:
    enum DeviceRoles {
        NameRole = Qt::UserRole + 1,
        SerialRole,
        TypeRole,
        ParentSerialRole,
        ConfigRole,
        ConfigDescRole,
        VariablesRole,
        VariablesDescRole,
        VariableHelper,
        ConfigHelper
    };
    enum DeviceColumns {
    	DeviceName = 0,
    	DeviceSerial,
    	DeviceType,
    	DeviceParentSerial,
    	DeviceConfig,
    	DeviceConfigDescriptor,
    	DeviceVariable,
    	DeviceVariableDescriptor,
    	DeviceVarHelper,
    	DeviceConfigHelper,
    	DEVICEMODEL_T_COLUMNS_MAX = DeviceConfigHelper + 1
    };

    DeviceModel_t(QObject *parent = 0);
    ~DeviceModel_t();
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    int columnCount(const QModelIndex & /*parent*/) const;
    void addDevice(const VarStorage &device);
    void delDevice(const std::string Device);
    QModelIndex index(int, int, const QModelIndex&) const;
    QModelIndex parent(const QModelIndex&) const;
    QModelIndex FindDevice(QString serial);
    Q_INVOKABLE int rowCount(const QModelIndex & parent = QModelIndex()) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
    bool setData (const QModelIndex & index, const QVariant &value, int role = Qt::EditRole);
    void setData (QString serial, QString name, QVariant value, bool sync = true);
    void updateDevice(const VarStorage &device);
    void updateDeviceConfig(const VarStorage &device);
signals:
	void sendMsg(MessageBus);

private:
    QMap <std::string, DeviceItem*> m_devices;
    DeviceItem *rootItem;
};

typedef boost::shared_ptr<DeviceModel_t> DeviceModel;

extern QMap<std::string, VarStorage> GlobalDevices;

typedef QMap<int, QString> VarList_t;

Q_DECLARE_METATYPE(DeviceModel_t*)
Q_DECLARE_METATYPE(VarList_t);

class VarStorageElement: public QObject {
	Q_OBJECT
public:
	VarStorageElement() {};
	VarStorageElement(VarStorage item) { this->var = item; };
	void SetVarStorageElement(VarStorage item) { this->var = item; };
	VarStorage GetItem() { return this->var;};
	Q_INVOKABLE QStringList getFields(VarStorage Var) {
		if (!Var) {
			qDebug() << "Empty Var for getFields";
			return QStringList();
		}
		std::vector<std::string> *fields;
		QStringList fieldnames;
		fields = Var->getFields();
		std::vector<std::string>::iterator it;
		for (it = fields->begin(); it != fields->end(); ++it) {
			fieldnames.append(static_cast<std::string>((*it)).c_str());
		}
		return fieldnames;
	}
	Q_INVOKABLE QString getString(VarStorage Var, QString name, int pos = 0) {
		if (!Var) {
			qDebug() << "Empty Var for " << name;
			return QString();
		}
		if ((unsigned int)pos > Var->getSize(name.toStdString())) {
			qWarning() << "pos is greater than size for " << name;
			return QString();
		}
		std::string result;
		if (Var->getStringValue(name.toStdString(), result, pos) == true)
			return result.c_str();
		else {
			qDebug() << "No Variable Named " << name;
			return QString();
		}
	}
	Q_INVOKABLE int getInt(VarStorage Var, QString name, int pos = 0) {
		if (!Var) return 0;
		int result;
		if ((unsigned int)pos > Var->getSize(name.toStdString())) {
			qWarning() << "pos is greater than size for " << name;
			return 0;
		}

		if (Var->getIntValue(name.toStdString(), result, pos) == true)
			return result;
		else
			return 0;
	}
	Q_INVOKABLE qlonglong getLong(VarStorage Var, QString name, int pos = 0) {
		if (!Var) return 0;
		long result;
		if ((unsigned int)pos > Var->getSize(name.toStdString())) {
			qWarning() << "pos is greater than size for " << name;
			return 0;
		}

		if (Var->getLongValue(name.toStdString(), result, pos) == true)
			return result;
		else
			return 0;
	}
	Q_INVOKABLE qlonglong getLongLong(VarStorage Var, QString name, int pos = 0) {
		if (!Var) return 0;
		long long result;
		if ((unsigned int)pos > Var->getSize(name.toStdString())) {
			qWarning() << "pos is greater than size for " << name;
			return 0;
		}

		if (Var->getLongLongValue(name.toStdString(), result, pos) == true)
			return result;
		else
			return 0;
	}
	Q_INVOKABLE float getFloat(VarStorage Var, QString name, int pos = 0) {
		if (!Var) return 0;
		if ((unsigned int)pos > Var->getSize(name.toStdString())) {
			qWarning() << "pos is greater than size for " << name;
			return 0;
		}

		float result;
		if (Var->getFloatValue(name.toStdString(), result, pos) == true)
			return result;
		else
			return 0;
	}
	Q_INVOKABLE bool getBool(VarStorage Var, QString name, int pos = 0) {
		if (!Var) return false;
		if ((unsigned int)pos > Var->getSize(name.toStdString())) {
			qWarning() << "pos is greater than size for " << name;
			return false;
		}

		bool result;
		if (Var->getBoolValue(name.toStdString(), result, pos) == true)
			return result;
		else
			return false;
	}
	Q_INVOKABLE QString getTime(VarStorage Var, QString name, int pos = 0) {
		if (!Var) return QString();
		if ((unsigned int)pos > Var->getSize(name.toStdString())) {
			qWarning() << "pos is greater than size for " << name;
			return QString();
		}

		boost::posix_time::ptime result;
		if (Var->getTimeValue(name.toStdString(), result, pos) == true)
			return boost::posix_time::to_iso_extended_string(result).c_str();
		else
			return boost::posix_time::to_iso_extended_string(boost::posix_time::ptime(boost::date_time::not_a_date_time)).c_str();
	}
	Q_INVOKABLE QVariant getHash(VarStorage Var, QString name, QString element, int pos = 0) {
		if (!Var) return QVariant();
		if ((unsigned int)pos > Var->getSize(name.toStdString())) {
			qWarning() << "pos is greater than size for " << name;
			return QVariant();
		}

		HashVals result;
		if (Var->getHashValue(name.toStdString(), result, pos) == true) {
			if (result[element.toStdString()].type() == typeid(std::string)) {
				return QString(boost::get<std::string>(result[element.toStdString()]).c_str());
			} else if (result[element.toStdString()].type() == typeid(int)) {
				return (int)boost::get<int>(result[element.toStdString()]);
			} else if (result[element.toStdString()].type() == typeid(long)) {
				return (qlonglong)boost::get<long>(result[element.toStdString()]);
			} else if (result[element.toStdString()].type() == typeid(long long)) {
				return (qlonglong)boost::get<long long>(result[element.toStdString()]);
			} else if (result[element.toStdString()].type() == typeid(float)) {
				return (float)boost::get<float>(result[element.toStdString()]);
			} else if (result[element.toStdString()].type() == typeid(boost::posix_time::ptime)) {
				return QString(boost::posix_time::to_iso_string(boost::get<boost::posix_time::ptime>(result[element.toStdString()])).c_str());
			} else if (result[element.toStdString()].type() == typeid(ListOptions)) {
				ListOptions lv = boost::get<ListOptions>(result[element.toStdString()]);
				int i = 0;
				VarList_t map;
				while (strlen(lv[i].desc) > 0) {
					map.insert(lv[i].index, lv[i].desc);
					i++;
				}
				return QVariant::fromValue<VarList_t>(map);
			} else {
				qWarning() << "Can't Handle Hash Type ";
				return QVariant();
			}
		} else {
			return QVariant();
		}
	}
	Q_INVOKABLE QVariant getHash(VarStorage Var, QString name, int pos = 0) {
		if (!Var) return QVariant();
		if ((unsigned int)pos > Var->getSize(name.toStdString())) {
			qWarning() << "pos is greater than size for " << name;
			return QVariant();
		}

		HashVals result;
		if (Var->getHashValue(name.toStdString(), result, pos) == true) {
			return QVariant::fromValue<HashVals>(result);
		} else {
			return QVariant();
		}
	}
	Q_INVOKABLE QVariant getVarStorage(VarStorage Var, QString name, int pos = 0) {
		if (!Var) return QVariant();
		if ((unsigned int)pos > Var->getSize(name.toStdString())) {
			qWarning() << "pos is greater than size for " << name;
			return QVariant();
		}

		VarStorage result;
		if (Var->getVarStorageValue(name.toStdString(), result, pos) == true) {
			/* we should make a copy, not just pass the shared_ptr out. This way
			 * any updates to the VarStorage will not be automatically reflected here
			 */
			VarContainerCopy(returnval, result);
			return QVariant::fromValue<VarStorage>(returnval);
		} else {
			return QVariant();
		}
	}
	Q_INVOKABLE QVariant getList(VarStorage Var, QString name, int pos = 0) {
		if (!Var) return QVariant();
		if ((unsigned int)pos > Var->getSize(name.toStdString())) {
			qWarning() << "pos is greater than size for " << name;
			return QVariant();
		}

		list_const_iterator iter;
		VarList_t map;
		std::cout << Var << std::endl;
		std::cout << name.toStdString() << std::endl;
		for (iter = Var->getListIterBegin(name.toStdString(), pos); iter != Var->getListIterEnd(name.toStdString(), pos); ++iter) {
			std::cout << (*iter).first << " " << (*iter).second.c_str() << std::endl;
			map.insert((*iter).first, (*iter).second.c_str());
			std::cout << "done" << std::endl;
		}
		return QVariant::fromValue<VarList_t>(map);
	}
	Q_INVOKABLE QVariant getListSelection(VarStorage Var, QString name, int pos = 0) {
		if (!Var) return QVariant();
		if ((unsigned int)pos > Var->getSize(name.toStdString())) {
			qWarning() << "pos is greater than size for " << name;
			return QVariant();
		}

		unsigned int result;
		if (Var->getListSelectedValue(name.toStdString(), result, pos)) {
				return result;
		} else {
			return QVariant();
		}
	}

	Q_INVOKABLE void setData(QObject *model, QString serial, QString name, QVariant value, int pos = 0) {
		if (pos > 0)
			qWarning() << "setData for Arrays not implemented yet";
		return;
		dynamic_cast<DeviceModel_t*>(model)->setData(serial, name, value);

	}


	Q_INVOKABLE bool getReadOnly(VarStorage Var, QString name) {
		if (!Var) {
			qWarning() << "Var Doesn't exist in getReadOnly?!?!? (looking for " << name << ")";
			return true;
		}
		HashVals fieldconfig;
		qWarning() << "Looking For " << name << " as Read Only\n";
		if (Var->getHashValue(name.toStdString(), fieldconfig)) {
			if ((int)boost::get<int>(fieldconfig["flags"]) && TCF_READONLY) {
				return true;
			} else {
				return false;
			}
		}
		qWarning() << "Doesn exist\n";
		return false;
	}

	Q_INVOKABLE QString getString(QString name, int pos = 0) { return this->getString(this->var, name, pos);}
	Q_INVOKABLE int getInt(QString name, int pos = 0) { return this->getInt(this->var, name, pos);}
	Q_INVOKABLE qlonglong getLong(QString name, int pos = 0) { return this->getLong(this->var, name, pos);}
	Q_INVOKABLE qlonglong getLongLong(QString name, int pos = 0) { return this->getLongLong(this->var, name, pos); }
	Q_INVOKABLE float getFloat(QString name, int pos = 0) { return this->getFloat(this->var, name, pos);}
	Q_INVOKABLE bool getBool(QString name, int pos = 0) { return this->getBool(this->var, name, pos);}
	Q_INVOKABLE QString getTime(QString name, int pos = 0) { return this->getTime(this->var, name, pos);}
	Q_INVOKABLE QVariant getHash(QString name, QString element, int pos = 0) { return this->getHash(this->var, name, element, pos);}
	Q_INVOKABLE QVariant getList(QString name, int pos = 0) { return this->getList(this->var, name, pos);}
	Q_INVOKABLE QVariant getListSelection(QString name, int pos = 0) { return this->getListSelection(this->var, name, pos);}
	Q_INVOKABLE QStringList getFields() { return this->getFields(this->var); }
	private:
		VarStorage var;
};

class VarStorageHelper_t {
public:
	VarStorageHelper_t(VarStorage value, VarStorage descriptor, QString Serial);
	~VarStorageHelper_t();
	QVariant getValue(QString name, int pos = 0);
	VarStorage setValue(VarStorage, QString, QVariant, int pos = 0);
	QVariant getListOptions(QString name, int pos = 0);
	int getRealType(QString name);
	int getType(QString name);
	int getType(QString name, QString element);
	QString getName(QString name);
	QString getDescription(QString name);
	qlonglong getMin(QString name);
	qlonglong getMax(QString name);
	QVariant getDefault(QString name);
	int getFlags(QString name);
    bool getReadOnly(QString name);
	int getGroup(QString name);
	QStringList getItems();
	int getArrayMaxSize(QString name);
	int getSize(QString name);
	VarStorage delValue(VarStorage UpdateVals, QString name, int pos = 0);
	QString getSerial();
	int getVarType(QString name);
private:
	VarStorage values;
	VarStorage descriptor;
	VarStorageElement VSE;
	QString Serial;
};

typedef QSharedPointer<VarStorageHelper_t> VarStorageHelper;

Q_DECLARE_METATYPE(VarStorageHelper)

#endif
