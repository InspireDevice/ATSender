#ifndef EMULATOR_H
#define EMULATOR_H

#include <QObject>
#include <QTextStream>
#include <QtGui/QTextEdit>
#include <QFile>
#include <QDIR>
#include <QtDebug>
#include <QList>

#include "qcdiag\qextserialenumerator.h"
#include "qcdiag\qextserialport.h"


//Emulation Modes
#define EMUL_SMART_MODE				0
#define EMUL_SYNC_MODE				1
#define EMUL_FULL_EMUL_MODE			2
#define EMUL_MANUAL_MODE			3

#define EMUL_ERROR_STOPPED			50
#define EMUL_OFFLINE_MODE			100

#define EMUL_MAX_CMD_OFFSET			15
#define EMUL_SWITCH_TO_SYNC_COUNT   10
#define EMUL_SMART_NOANSW_FOR_ERR   4
//Return
#define RET_OK						0

#define  MAX_PACKET_LEN    0x5500

class Emulator : public QObject
{
	Q_OBJECT
public:
	Emulator();
	virtual ~Emulator();

	//Devices
	QList <QString> getDevicesList();

	//Serial ports
	QList <QextPortInfo> getPortsList();

	//Emulation
	bool connectDevice(const int deviceIndex=0,const int portIndex=0);
	void disconnectDevice();
	bool connected();
	void setEmulationMode(int emulMode);

	void writeMessage(const QString message);
	void writeDevideMessage();
	void clearMessageBus();
	void assignMessageBus(QTextEdit *messageBus);


//*************


	int TestFileType(const QList <QString>  &fileLines);
	QString parse_line(const QString line);
	QString GetAnyLine(int index);
	bool writeToPort(const QByteArray &buff);

private:
/////Vars
	QList <QString> devicesList;
	QList<QextPortInfo> portsList;
	QextSerialPort * devicePort;
	QString exePath,devicesPath;

	QList <QString>  virtualDeviceData;
	int emulationMode;

	QTextEdit *msgBus;

	int lastReqIndex,smartModeErrorsCount;

	
//Funcs
	int loadDeviceEmulationData(int deviceIndex);
	void FreeDeviceData();
	bool getAnswer(const QString &request,QString &answer);

	bool openPort(int portIndex);
	QString dropLeading7E(const QString cmd);
	

//protected:
private slots:
    
	void onReadyRead();
    void onDsrChanged(bool status);


};

#endif // EMULATOR_H
