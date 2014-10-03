/* controlpanel - QtiHanClient.h
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

/** @file QtiHanClient.h
 *  @brief
 */



#ifndef QTIHANCLIENT_H_
#define QTIHANCLIENT_H_

#include <QtCore/QtCore>
#include <QtiHanClient/MessageHandler.h>
#include <QtiHanClient/DeviceModel.h>
#include <QtiHanClient/kdescendantsproxymodel.h>
#include <QtiHanClient/modeltest.h>

class QtiHanClient : public QObject {
    Q_OBJECT
public:

    /*           VarType          TermName        VarID    VarValue */
    typedef QMap< QString, QMap < QString, QMap < QString, QVariant > > > Term_Map_t;


	static QtiHanClient* Create(QObject *parent=0);
	static QtiHanClient* Get(){ return s_instance; }

	static void Destroy();

    Q_PROPERTY(int type READ getType WRITE setType);
    Q_PROPERTY(QString username READ getUserName WRITE setUserName);
    Q_PROPERTY(QString password READ getPassword WRITE setPassword);
    Q_PROPERTY(QString hostname READ getHostName WRITE setHostName);
    Q_PROPERTY(quint32 port READ getPort WRITE setPort);
    Q_PROPERTY(int hostid READ getHostID WRITE setHostID);
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

    int getHostID();
    void setHostID(int);

    bool connect();
    DeviceModel_t *getDeviceModel() { return this->tdm; };
    KDescendantsProxyModel *getFlatDeviceModel() { return this->fdm; };
    Term_Map_t getTermTypeMappings() { return this->TermTypeMappings; };

    QString getMyDeviceID();


Q_SIGNALS:
	void connected();
	void disconnected();
	void error(QString, QAbstractSocket::SocketError);
	void newEndPt(VarStorage item);
	void delEndPt(std::string item);
	void updateValues(QString, QVector<QString>);
	void updateConfig(QString, QVector<QString>);
	void StateChange(State_e);
public Q_SLOTS:
	void sendMessage(MessageBus);
private Q_SLOTS:
	void HandleConnected();
	void HandleDisconnected();
	void HandleError(QString, QAbstractSocket::SocketError);
	void HandleNewDevice(MessageBus item);
	void HandleDelDevice(MessageBus item);
	void HandleDeviceUpdate(MessageBus item);
	void HandleDeviceConfigUpdate(MessageBus item);
	void HandleStateChange(State_e state);
	void HandleTermTypeMappings(MessageBus vals);
	void HandleClientInform(MessageBus vals);


private:
    QtiHanClient(QObject *parent = 0);
    virtual ~QtiHanClient();




	int type;
	MessageHandler *mh;
	DeviceModel_t *tdm;
	KDescendantsProxyModel *fdm;
	ModelTest *mt;
	Term_Map_t TermTypeMappings;
	static QtiHanClient*		s_instance;
	VarStorage myinfo;

};




#endif /* QTIHANCLIENT_H_ */
