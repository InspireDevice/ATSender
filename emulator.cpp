#include "emulator.h"

Emulator::Emulator()
:QObject()
{
	devicePort=0;
	msgBus=0;

	exePath=QDir::currentPath()+"/";
	devicesPath=exePath+"Devices/";

	setEmulationMode(EMUL_OFFLINE_MODE);

	QList <QString> a=getDevicesList();
	QList <QextPortInfo> b=getPortsList();

}

Emulator::~Emulator()
{
}


//==========================================================
//==========================================================
//==========================================================
//==========================================================
//Devices
QList <QString> Emulator::getDevicesList()
{
	QDir myDir ("");
	myDir.setPath(devicesPath);
	devicesList=myDir.entryList(QDir::Files);
	devicesList.insert(0,"===**NULL DEVICE**===");
	return devicesList;
}

//==========================================================
//==========================================================
//Serial ports
QList <QextPortInfo> Emulator::getPortsList()
{
	QextPortInfo temp;

	portsList = QextSerialEnumerator::getPorts();
/*
	//this is special case for com0com ports...
	temp.friendName = "COM15";
	temp.portName = "COM15";
	portsList.insert(0,temp);

	temp.friendName = "COM16";
	temp.portName = "COM16";
	portsList.insert(0,temp);
*/

	return portsList;
}

//==========================================================
//==========================================================
//Emulation Start/Stop
bool Emulator::connectDevice(const int deviceIndex,const int portIndex)
{
	clearMessageBus();
	disconnectDevice();

	//writeMessage("Connecting device");
	
	writeMessage("   Trying to open port...");
	if (!openPort(portIndex)) {
		//writeDevideMessage();
		//writeMessage("   Device is disconnected, becouse of open port failure. ");
		writeDevideMessage();
		return false;
	}

	if (loadDeviceEmulationData(0)!=RET_OK){
		return false;
	}
	return true;
}
//---------------------------------------------------------
void Emulator::disconnectDevice()
{
	if (devicePort){
		writeMessage("Disconnecting from port...");
		delete devicePort;
		devicePort=0;
		writeMessage("Port closed.");
	}
	FreeDeviceData();
	setEmulationMode(EMUL_OFFLINE_MODE);
}
//---------------------------------------------------------
bool Emulator::connected()
{
	if (devicePort) return true;
	else return false;
}
//==========================================================
//==========================================================
void Emulator::setEmulationMode(int emulMode)
{
	QString msg="";
	emulationMode=emulMode;
	switch (emulMode){
		case EMUL_FULL_EMUL_MODE:
			msg="EMULATION: ***** FULL EMULATION MODE *****";
			break;
		case EMUL_MANUAL_MODE:
			msg="EMULATION: ***** MANUAL MODE *****";
			break;
		case EMUL_OFFLINE_MODE:
			if (connected()) disconnectDevice();
			msg="EMULATION: ***** DEVICE DISCONNECTED *****";
			break;
		case EMUL_SMART_MODE:
			smartModeErrorsCount=0;
			msg="EMULATION: ***** SMART MODE *****";
			break;

		case EMUL_ERROR_STOPPED:
			msg="EMULATION: ***** ERROR. EULATION STOPPED *****";
			break;
		case EMUL_SYNC_MODE:
			msg="EMULATION: ***** SYNC MODE *****";
			break;
		default:
			{
				msg="EMULATION: !!!! INCORRECT EMULATION MODE !!!!";
				disconnectDevice();
				break;
			}

	}
	if (connected()) msg="//"+msg;
	//writeMessage(msg);
}
//==========================================================
//==========================================================
void Emulator::writeMessage(const QString message)
{
	if (msgBus!=0){

		QRegExp rx("\"(\\w+)\"");
		QString str = message;
		QStringList list;
		int pos = 0;

		while ((pos = rx.indexIn(str, pos)) != -1) {
			list << rx.cap(1);
			pos += rx.matchedLength();
		}
		msgBus->moveCursor(QTextCursor::End, QTextCursor::MoveAnchor);
		msgBus->textCursor().insertText(message+"\r");
		//msgBus->textCursor().insertText(list.join("\r")+"\r");

		for (int i = 0; i < list.size(); ++i){
			QByteArray myData =QByteArray::fromHex(list.at(i).toLatin1());
			QChar ch = myData.at(0);
			//qDebug ()<< (ch).unicode();
			if (ch ==128) {
				//convert
				//myData.remove(0,1);
				msgBus->textCursor().insertText(QString::fromUtf16(reinterpret_cast<const ushort*>(myData.constData())) +"\r");
			}else
			{
				msgBus->textCursor().insertText(myData +"\r");
			}
		}
         
		
	}
}

void Emulator::writeDevideMessage()
{
	if (msgBus!=0){
	msgBus->moveCursor(QTextCursor::End, QTextCursor::MoveAnchor);
	msgBus->textCursor().insertText("---------------------------------------------------\r");
	}
}

void Emulator::clearMessageBus()
{
	msgBus->clear();
	writeDevideMessage();
	writeMessage("Visit our site : http://inspire-device.com");
	writeDevideMessage();
}

//==========================================================
//==========================================================
void Emulator::assignMessageBus(QTextEdit *messageBus)
{
msgBus=messageBus;
clearMessageBus();
}
//==========================================================
//==========================================================
int Emulator::loadDeviceEmulationData(int deviceIndex)
{
	virtualDeviceData.clear();

	if (deviceIndex<0 || deviceIndex>=devicesList.count()) {
		writeDevideMessage();
		writeMessage("**incorrect device file index");
		return -1;
	}

	if (deviceIndex==0) {
//		writeMessage("No any file loaded. Manual command sender mode.");
		setEmulationMode(EMUL_MANUAL_MODE);
		return RET_OK;
	}

	QString fileName=devicesPath+devicesList.at(deviceIndex);
	writeMessage("Loading device file: <"+fileName+">");

	QFile file(fileName);
	QString line;
	QList <QString>  fileConsist,list;
	//Завантаження всіх не пустих рядків з файлу
	//в список FileConsist
	if (file.open(QFile::ReadOnly)) {
		QTextStream input(&file);
		do {
			line=input.readLine().trimmed();
			if (line.length()>1)
		 {
			 //Skipping comments
			 if (!((line.left(2).contains("//")) || (line.left(1).contains("#"))))
				 fileConsist.append(line);	   
		 }
		}while (!line.isNull());
	}else {
		writeMessage("failed to open file");
		return -1;
	}
	file.close();

	//Перевірка типу файлу
	int logtype=TestFileType(fileConsist);

	//Перенесення даних в масив у потрібному форматі
	switch (logtype) {
		case 1:{
			writeMessage("file type: normal");
			list = fileConsist;
			break;
			   }
		case 2:{
			// IRP_MJ_WRITE/IRP_MJ_READ
			writeMessage("file type: IRP_MJ_WRITE/IRP_MJ_READ");
			for (int i=0;i<fileConsist.count();i++){             ////////Can be incorrect
				list.append(parse_line(fileConsist.at(i)));
			}   //For end
			break;
			   }
		default:
			{
				writeDevideMessage();
				writeMessage("** unknown file structure.");
				return -1;

			   }    //case 2 end
	}//switch end

//Normalize log
	virtualDeviceData.clear();
	int prevCmdIsAnsw=true;
	for (int i=0;i<list.count();i++)
	{
		if (list.at(i).contains(">"))
		{
			if (!prevCmdIsAnsw) virtualDeviceData.append("<");
			virtualDeviceData.append(list.at(i));
			prevCmdIsAnsw=false;
		}else
		if (list.at(i).contains("<"))
		{
			if (prevCmdIsAnsw) {
				writeDevideMessage();
				writeMessage("** bad device file structure. Two answers in a row.");
				return -1;
			}
			else {
				virtualDeviceData.append(list.at(i));
				prevCmdIsAnsw=true;
			}
		}else {
			writeDevideMessage();
			writeMessage(list.at(i));
			writeMessage("**bad symbols detected.");
			return -1;
		}

	}

	lastReqIndex=-1;
	writeMessage("Data lines count: "+QString::number(virtualDeviceData.count()));

	
	return RET_OK;
}
//---------------------------------------------------------------
bool Emulator::openPort(int portIndex)
{
	if (devicePort) delete devicePort;
	if ((portIndex<0)||(portIndex>=portsList.count())){
		writeDevideMessage();
		writeMessage("***incorrect port index!!!");
		return false;
	}

	QString portName = portsList.at(portIndex).portName;

	devicePort = new QextSerialPort(portsList.at(portIndex).portName, QextSerialPort::EventDriven);
	devicePort->setBaudRate(BAUD115200);
	//devicePort->waitForBytesWritten(200);
	//devicePort->waitForReadyRead(200);

	//port->setFlowControl(FLOW_OFF);
	devicePort->setParity(PAR_NONE);
	devicePort->setDataBits(DATA_8);
	devicePort->setStopBits(STOP_1);
	devicePort->setTimeout(500);

	writeMessage("Opening port: "+portName);
	if (devicePort->open(QIODevice::ReadWrite | QIODevice::Unbuffered))
	{
		//QObject::connect(devicePort, SIGNAL(readyRead()), this, SLOT(onReadyRead()),Qt::AutoConnection);
		if (Emulator::connect(devicePort, SIGNAL(readyRead()), this, SLOT(onReadyRead()),Qt::AutoConnection) 
			&&connect(devicePort, SIGNAL(dsrChanged(bool)), this, SLOT(onDsrChanged(bool)),Qt::AutoConnection)
			)
		{
			qDebug() <<"port opened";
			//writeMessage("Port opened.");
			return true;
		}else {
			delete devicePort;
			devicePort=0;
			writeDevideMessage();
			writeMessage("**Failed to connect port signals.");
			return false;
		}
		
	}
	else{
		writeMessage("Failed to open port.");
		return false;
	}
	/*
	if (!(port->lineStatus() & LS_DSR)) {
	qDebug() <<"warning: maybe device is not turned on";
	addLogMessage("warning: maybe device is not turned on");
	}
	*/
}
//==========================================================
//==========================================================
void Emulator::FreeDeviceData()
{
	virtualDeviceData.clear();
}

//==========================================================
//==========================================================
/*
QString Emulator::GetNextRequest()//const bool ignorAnswer=true,const QString deviceAnsw="")
{
QString rslt;
if (nextReqIndex<0)
{
//Шукаємо індекс першого запиту
nextReqIndex=0;
while ((virtualDeviceData.at(nextReqIndex).left(1)!=">")&&(nextReqIndex<virtualDeviceData.count()))
{
nextReqIndex++;
}
}
//Отримуємо значення запиту
if (nextReqIndex>=virtualDeviceData.count()){
return "";}
else {rslt=virtualDeviceData.at(nextReqIndex);rslt.remove(0,1);}

//Шукаємо індекс наступного запиту
nextReqIndex++;
while ((nextReqIndex<virtualDeviceData.count())&&(virtualDeviceData.at(nextReqIndex).left(1)!=">"))
{
nextReqIndex++;
}
return rslt;
}
//==========================================================
//==========================================================

QString Emulator::GetDeviceAnswer(QString reqst,int &status)
{
status=0;
if (!EmulOn) return "";

reqst.insert(0,">");

if (!initDevice && (nextReqIndex==-1)) status=11;

//Finding next index if this is first request
if (nextReqIndex<0)
{
QString temp;
for (int i=0;i<virtualDeviceData.count();i++)
{
temp=virtualDeviceData.at(i);
if (temp.trimmed().toLower()==reqst.trimmed().toLower())
{
nextReqIndex=i;
break;
}
}//For
}//if

if ((nextReqIndex<0)||(virtualDeviceData.count()<=nextReqIndex)) return "";

QString rslt="";
//Getting the answer
if (virtualDeviceData.at(nextReqIndex)==reqst)
{
rslt=virtualDeviceData.at(nextReqIndex+1);
if (rslt.at(0)=='<') 
{
rslt=rslt.remove(0,1);
nextReqIndex=nextReqIndex+2;
}
else 
{
rslt="";
nextReqIndex=nextReqIndex+1;
}
}
else
{
nextReqIndex=virtualDeviceData.count();
initDevice=false;
status=-10;
return "";
}

if ((initDevice)&&(nextReqIndex>=virtualDeviceData.count()))
{
virtualDeviceData=&devFile;
nextReqIndex=-1;
initDevice=false;
status=10;

}
return rslt;
}
*/
//==========================================================
//==========================================================
int Emulator::TestFileType(const QList <QString>  &fileLines)
{
	int ftype=0;
	QString line("");

	for (int i=0;((i<fileLines.count())&(!ftype));i++){
		line=fileLines.at(i);
		if (line.indexOf(">",0,Qt::CaseInsensitive)==0) {
			ftype=1;
		}
		else if (line.indexOf("Data",0,Qt::CaseInsensitive)) {
			ftype=2;
		}
	}
	return ftype;
}
//==========================================================
//==========================================================
QString Emulator::parse_line(const QString line)
{

	QString dataStr("");

	if (line.length())
	{
		QString funcStr("");

		int pos=0;
		pos = line.indexOf("Data: ") + 6;
		dataStr =  line.mid(pos);
		dataStr.remove(" ",Qt::CaseInsensitive);

		if ((dataStr.length()==2)&(dataStr.indexOf("7E",0,Qt::CaseInsensitive)>=0)) 
		{
			dataStr.clear();
		} else
		{
			if (line.indexOf("IRP_MJ_WRITE",0,Qt::CaseInsensitive)>0) dataStr.insert(0,">");
			if (line.indexOf("IRP_MJ_READ",0,Qt::CaseInsensitive)>0) dataStr.insert(0,"<");
		}//Else
	}
	return dataStr;
}
//==========================================================
//==========================================================
bool Emulator::writeToPort(const QByteArray &buff)
{
	QByteArray tmp=buff;
	//qint64 writed=0;
	if (connected())
	{
		devicePort->write(buff);
		Sleep(50);
		
		writeMessage("      Request:");
		writeMessage(tmp.toHex().toUpper().insert(0,">"));	
		tmp.insert(0,QString("<<|"));
		for (int i=0;i<tmp.length();i++)
		{
			if (tmp.at(i)<0x20) tmp.replace(i,1,".");
			//if (bytes.at(i)<0x20) bytes.remove(i,1);
		}
		writeMessage(QString(tmp)+"|>>");	
		writeDevideMessage();
		return true;
	}else {
		writeMessage("Device is not connected.");
		return false;
	}
}
//==========================================================
//==========================================================
QString Emulator::GetAnyLine(int index)
{
	if (index<virtualDeviceData.count())
	{
		return virtualDeviceData.at(index);
	} else return "";
}

//==========================================================
//==========================================================
bool Emulator::getAnswer(const QString &request,QString &answer)
{
	
	answer.clear();
	int positiveOffsetIndex=-1;
	int negativeOffsetIndex=-1;
	if (lastReqIndex<0) lastReqIndex=-2;

	QString request_=dropLeading7E(request);
	request_.insert(0,'>');

	//QString tmp;
	//positive
	for (int i=lastReqIndex+2;i<virtualDeviceData.count();i=i+2)
	{
		if (request_.compare(dropLeading7E(virtualDeviceData.at(i)))==0)
		{
			positiveOffsetIndex=i;
			i=virtualDeviceData.count();
		}
	}
	//zero
	if (positiveOffsetIndex==-1)
	{
		if (lastReqIndex>=0)
			if (request_.compare(dropLeading7E(virtualDeviceData.at(lastReqIndex)))==0) positiveOffsetIndex=lastReqIndex;
	}
	//negative
	for (int i=lastReqIndex-2;i>=0;i=i-2)
	{
		if (request_.compare(dropLeading7E(virtualDeviceData.at(i)))==0)
		{
			negativeOffsetIndex=i;
		//i=-1;
			break;
		}
	}

	switch (emulationMode){
		case EMUL_FULL_EMUL_MODE:
			if (positiveOffsetIndex==-1 && negativeOffsetIndex!=-1){
				lastReqIndex=negativeOffsetIndex;
				answer=virtualDeviceData.at(lastReqIndex+1);
				answer.remove(0,1);
			}else 
				if(positiveOffsetIndex!=-1 && negativeOffsetIndex==-1){
				lastReqIndex=positiveOffsetIndex;
				answer=virtualDeviceData.at(lastReqIndex+1);
				answer.remove(0,1);
			}else 
				if(positiveOffsetIndex==-1 && negativeOffsetIndex==-1){
					if (!(request_==">7C93497E" && getAnswer("7C0023567E",answer))) 
						answer=request;
			}else{
				if (positiveOffsetIndex-lastReqIndex<lastReqIndex-negativeOffsetIndex)
				{
					lastReqIndex=positiveOffsetIndex;
					answer=virtualDeviceData.at(lastReqIndex+1);
					answer.remove(0,1);
				}else{
					lastReqIndex=negativeOffsetIndex;
					answer=virtualDeviceData.at(lastReqIndex+1);
					answer.remove(0,1);
				}
				}
			return true;
			break;

		case EMUL_MANUAL_MODE://**done
			return true;
			break;

		case EMUL_ERROR_STOPPED:
			return false;
			break;
		case EMUL_OFFLINE_MODE:
			return false;
			break;
		case EMUL_SMART_MODE:
			if (positiveOffsetIndex!=-1 && positiveOffsetIndex-lastReqIndex>2*EMUL_MAX_CMD_OFFSET) {
				positiveOffsetIndex=-1;
				writeMessage("//Cmd Offset Exceeded");
			}
			if (negativeOffsetIndex!=-1 && lastReqIndex-negativeOffsetIndex>2*EMUL_MAX_CMD_OFFSET){
				negativeOffsetIndex=-1;
				writeMessage("//Cmd Offset Exceeded");
			}
	
				if (positiveOffsetIndex==-1 && negativeOffsetIndex!=-1){
					lastReqIndex=negativeOffsetIndex;
				}else 
				if(positiveOffsetIndex!=-1 && negativeOffsetIndex==-1){
					lastReqIndex=positiveOffsetIndex;
				}else 
				if(positiveOffsetIndex==-1 && negativeOffsetIndex==-1){
					if (request_==">7C93497E" && getAnswer("7C0023567E",answer)) {
						writeMessage("//>7C93497E replaced by 7C0023567E");
						smartModeErrorsCount=0;
						return true;
					}else{
						smartModeErrorsCount++;
						if (smartModeErrorsCount>EMUL_SMART_NOANSW_FOR_ERR) setEmulationMode(EMUL_ERROR_STOPPED);
						return false;
					}
				}else{lastReqIndex=positiveOffsetIndex;
				/*if (positiveOffsetIndex-lastReqIndex<lastReqIndex-negativeOffsetIndex)// Скаче куди бдижче.. може змінити ?
				{
					lastReqIndex=positiveOffsetIndex;
				}else{
					lastReqIndex=negativeOffsetIndex;
				}*/
				}
					answer=virtualDeviceData.at(lastReqIndex+1);
					answer.remove(0,1);
					smartModeErrorsCount=0;
					return true;
			break;
		case EMUL_SYNC_MODE:
			if ((positiveOffsetIndex-lastReqIndex)==2)
			{
				lastReqIndex=lastReqIndex;
				answer=virtualDeviceData.at(positiveOffsetIndex+1);
				answer.remove(0,1);
				return true;
			}else{
				setEmulationMode(EMUL_ERROR_STOPPED);
				return false;
			}

			break;
	}//switch
}
//==========================================================
//==========================================================
QString Emulator::dropLeading7E(const QString cmd)
{
	QString tmp=cmd;
	//if 
	while	(tmp.count()>=2 && tmp.left(2).contains(tr("7E"),Qt::CaseInsensitive)){
		tmp.remove(0,2);
	}
	
	
	while (tmp.count()>=3 && tmp.mid(1,2).contains(tr("7E"),Qt::CaseInsensitive) && (tmp.at(0)=='>'||tmp.at(0)=='<')) {
		tmp.remove(1,2);
	}
	return tmp;
}

//==========================================================
//==========================================================
//**********************************************************
/////////////////SLOTS/////////////////////
void Emulator::onReadyRead()
{
//Read data
	QByteArray buff;
	int cnt;

	if (!devicePort){
		writeMessage("port allready closed");
		return;
	}else{
		cnt = devicePort->bytesAvailable();
		if (!cnt) return;
	}
	
    buff.resize(cnt);//*
	cnt=devicePort->read(buff.data(), cnt);//*
//	buff.resize(cnt);//*
    qDebug() << "bytes read:" << buff.size();
    qDebug() << "bytes:" << buff;

	QString request=buff.toHex().toUpper();
	QString answer;

	switch (emulationMode){
		case EMUL_FULL_EMUL_MODE:
		case EMUL_SMART_MODE:
		case EMUL_SYNC_MODE:

			writeMessage(">"+request);	
			if (getAnswer(request,answer))
			{
				devicePort->write(QByteArray::fromHex(answer.toAscii()));
				writeMessage(answer.insert(0,"<"));	

			}
			break;

		case EMUL_MANUAL_MODE://**done
			writeMessage("      Answer:");
			writeMessage(request.insert(0,"<"));	
			buff.insert(0,"$<<|");
			for (int i=0;i<buff.length();i++)
			{
				if (buff.at(i)==0x0A) 
					buff.replace(i,1,"\r");
				else if (buff.at(i)<0x20) buff.replace(i,1,".");
			//if (bytes.at(i)<0x20) bytes.remove(i,1);
			}

			writeMessage(QString(buff)+"|>>");	
			writeDevideMessage();
			break;

		case EMUL_ERROR_STOPPED:
			writeMessage(request.insert(0,">"));	
/*			buff.insert(0,"<<|");
			for (int i=0;i<buff.length();i++)
			{
				if (buff.at(i)<0x20) buff.replace(i,1,".");
			}
			writeMessage(QString(buff)+"|>>");	
			writeDevideMessage();//*/

			break;
		case EMUL_OFFLINE_MODE:
			break;
	}//switch
}
//-----------------------------------------------------------
void Emulator::onDsrChanged(bool status)
{
	if (status)
		qDebug() << "device was turned on";
	else
		qDebug() << "device was turned off";
}
