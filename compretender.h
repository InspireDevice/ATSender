#ifndef COMPRETENDER_H
#define COMPRETENDER_H

#include <QtGui/QMainWindow>
#include "ui_compretender.h"
#include "qcdiag\qextserialenumerator.h"
#include "qcdiag\qextserialport.h"
#include "emulator.h"
//#include "test.h"
//#include "msgs.h"

class ComPretender : public QMainWindow
{
	Q_OBJECT

public:
	ComPretender(QWidget *parent = 0, Qt::WFlags flags = 0);
	~ComPretender();

	Emulator *virtualDevice;
	void addLogMessage(QString msg);


private:
	Ui::ComPretenderClass ui;

	QextSerialPort * port;
	QList<QextPortInfo> portNames;
	void getSerialPorts();

private slots:
	void on_seeChars_clicked();
	void on_groupBox_toggled(bool);
	void on_pushButton_2_clicked();
	void on_startComunicate_clicked();
	void on_sendATcmd_clicked();
	void on_pushButton_clicked();
	void on_comDisconnect_clicked();
	void on_comRefresh_clicked();
	void on_comConnect_clicked();
    void onReadyRead();
    void onDsrChanged(bool status);
};

#endif // COMPRETENDER_H
