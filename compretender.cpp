#include "compretender.h"
#include "ui_compretender.h"
#include <QMessageBox>
#include <QtGui/QComboBox>
#include <QtDebug>
#include <QDir>

#include "Emulator.h"

//-----------------------------------------------------------------------------------------
ComPretender::ComPretender(QWidget *parent, Qt::WFlags flags)
	: QMainWindow(parent, flags)
{
	ui.setupUi(this);
//
	virtualDevice= new Emulator();
	virtualDevice->assignMessageBus(ui.textEdit);
	getSerialPorts();

	//ui.DevicesCombo->clear();
	//ui.DevicesCombo->addItems(virtualDevice->getDevicesList());

	ui.DevicesCombo->setVisible(false);
	ui.label_2->setVisible(false);
	ui.pushButton->setVisible(false);
	ui.startComunicate->setVisible(false);
	ui.seeChars->setVisible(false);
	ui.pushButton_2->setVisible(false);


}

//-----------------------------------------------------------------------------------------
ComPretender::~ComPretender()
{

}

//-----------------------------------------------------------------------------------------
void ComPretender::addLogMessage( QString msg)
{
	ui.textEdit->moveCursor(QTextCursor::End, QTextCursor::MoveAnchor);
	ui.textEdit->textCursor().insertText(msg+"\r");
}

//-----------------------------------------------------------------------------------------
void ComPretender::getSerialPorts()
{
	QList <QextPortInfo> portsList=virtualDevice->getPortsList();
	ui.comPortSelect->clear();
	for (int i=0;i<portsList.count();i++)
	{
		ui.comPortSelect->addItem(portsList.at(i).friendName);
	}

}

//-----------------------------------------------------------------------------------------
void ComPretender::on_comConnect_clicked()
{
	
/*
	The port must be configured for 19200 bps (or 115200 bps for MSM5000 phones), 8-bit 
	characters, 1 stop bit, and no parity.*/
	//QueryMode mode = EventDriven;
	ui.textEdit->clear();
	if (!virtualDevice->connectDevice(ui.DevicesCombo->currentIndex(),ui.comPortSelect->currentIndex()))
	{
		addLogMessage("Can not open port. Try to close modem software or check port!!!");
	} else 	{addLogMessage("Port successfully opened");virtualDevice->writeDevideMessage();}

}

//-----------------------------------------------------------------------------------------
void ComPretender::on_comRefresh_clicked()
{
	getSerialPorts();

	ui.DevicesCombo->clear();
	ui.DevicesCombo->addItems(virtualDevice->getDevicesList());

}

//-----------------------------------------------------------------------------------------
void ComPretender::on_comDisconnect_clicked()
{
	virtualDevice->disconnectDevice();
	//addLogMessage("Virtual device disconnected");
}


//-----------------------------------------------------------------------------------------
void ComPretender::onReadyRead()
{
	/*
	QByteArray bytes;
    int a = port->bytesAvailable();
    bytes.resize(a);
    port->read(bytes.data(), a);
    qDebug() << "bytes read:" << bytes.size();
    qDebug() << "bytes:" << bytes;

	QString request=bytes.toHex().toUpper();
	if (ManualMode)
	{
		addLogMessage(request.insert(0,"<"));	
		bytes.insert(0,"<<|");
		for (int i=0;i<bytes.length();i++)
		{
			if (bytes.at(i)<0x20) bytes.replace(i,1,".");
			//if (bytes.at(i)<0x20) bytes.remove(i,1);
		}
		addLogMessage(QString(bytes)+"|>>");	
	}else if (inServerMode){
	QString my_request=GetNextRequest();
	QByteArray cmd("");
	cmd.append(my_request);
	cmd=QByteArray::fromHex(cmd);
	if (port) port->write(cmd);
	addLogMessage(request.insert(0,"<"));	
	addLogMessage(my_request.insert(0,">"));	
	}else{
			//Отримання відповіді з файлу та конвертування в байти
	int stat=0;
	QString answer=GetDeviceAnswer(request,stat);
	QByteArray cmd("");
	cmd.append(answer);
	cmd=QByteArray::fromHex(cmd);
	if (port) port->write(cmd);

	//if (stat==11) ui.textEdit->clear();
	addLogMessage(request.insert(0,">"));	
	addLogMessage(answer.insert(0,"<"));	
	if (stat==10) {
		addLogMessage("Device type has been requested. Now in StartUnlockMode");
		//QMessageBox::about(0,"End init","Device type has been requested");
	}
	}
*/
}


//-----------------------------------------------------------------------------------------
void ComPretender::onDsrChanged(bool status)
{
    if (status)
        qDebug() << "device was turned on";
    else
        qDebug() << "device was turned off";
}

//-----------------------------------------------------------------------------------------
void ComPretender::on_pushButton_clicked()
{
	if (!virtualDevice->connected())
	{
		addLogMessage("Virtual device is not active!!!");
		return;
	}

	ui.textEdit->clear();
	QByteArray write_buf=QByteArray::fromHex(ui.lineATcmd->text().toAscii());
		//addLogMessage(">"+ui.lineATcmd->text());
		 
	QByteArray bytes=QByteArray::fromHex(ui.lineATcmd->text().toAscii());
	bytes.insert(0,"!<<|");
	for (int i=0;i<bytes.length();i++)
	{
		if (bytes.at(i)<0x20) bytes.replace(i,1,".");
		//if (bytes.at(i)<0x20) bytes.remove(i,1);
	}
		//addLogMessage(QString(bytes)+"|>>");	

	virtualDevice->writeToPort(write_buf);

}

//-----------------------------------------------------------------------------------------
/* just send some commands to com port in AT mode*/
void ComPretender::on_sendATcmd_clicked()
{
	if (!virtualDevice->connected())
	{
		addLogMessage("you've fogot to open port!!!");
		return;
	}
	QByteArray cmd=QString(ui.lineATcmd->text()+"\r").toAscii();
	virtualDevice->writeToPort(cmd);
}

//-----------------------------------------------------------------------------------------
void ComPretender::on_pushButton_2_clicked()
{
addLogMessage("nothing happend...");
}

//-----------------------------------------------------------------------------------------
void ComPretender::on_groupBox_toggled(bool)
{

}

//-----------------------------------------------------------------------------------------
void ComPretender::on_startComunicate_clicked()
{///~~~~~~~~~~~~
/*
	if (port&&!ManualMode) {
		SetIntoStartUnlockMode();
		inServerMode=true;
		QString req=GetNextRequest();
		QByteArray cmd=QByteArray::fromHex(req.toAscii());
		port->write(cmd);
	addLogMessage(req.insert(0,">"));	
	}
	*/
}
//-----------------------------------------------------------------------------------------
void ComPretender::on_seeChars_clicked()
{
ui.textEdit->clear();
	//QByteArray cmd;
	//cmd.append(ui.lineATcmd->text());
	//cmd=QByteArray::fromHex(cmd);
	//addLogMessage(""""+ui.lineATcmd->text()+"""");
		 
		QByteArray bytes=QByteArray::fromHex(ui.lineATcmd->text().toAscii());
		bytes.insert(0,"""2<<|");
		for (int i=0;i<bytes.length();i++)
		{
			//if ((bytes.at(i)==0x0D) || bytes.at(i)==0x0A) bytes.replace(i,1,"n");
			if (bytes.at(i)<0x20) bytes.replace(i,1,".");
			//if (bytes.at(i)<0x20) bytes.remove(i,1);
		}
		addLogMessage(QString(bytes)+"|>>""");	

	}