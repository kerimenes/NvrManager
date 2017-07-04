#include "nvrmanager.h"
#include <QXmlStreamWriter>

QStringList stringSorting(QStringList strList)
{
	QList<int> intList;
	foreach (QString str, strList) {
		intList << str.toInt();
	}
	qSort(intList);
	QStringList outStrList;
	for (int i = 0; i < intList.size(); i++)
	{
		outStrList << QString::number(intList.at(i));
	}
	return outStrList;
}


NvrManager::NvrManager(QObject *parent)
	: QObject(parent)
{
	manager = new QNetworkAccessManager();
}

void NvrManager::readyRead()
{
//	qDebug() << "Upload finished";
//	qDebug() << "Download started";
}

void NvrManager::uploadProgres(qint64 bytesSent, qint64 bytesTotal)
{
	QNetworkReply *replyUp = qobject_cast<QNetworkReply*>(sender());
	qDebug() << replyUp->url().host();
	qDebug() << "bytesSent" << bytesSent << "bytesTotal" << bytesTotal;
}

void NvrManager::downloadProgress(qint64 bytesReceiver, qint64 bytesTotal)
{
	qDebug() << "bytesReceiver" << bytesReceiver << "bytesTotal" << bytesTotal;
}

QByteArray NvrManager::xmlCreateLogin()
{
	QByteArray ba;
	QXmlStreamWriter xml(&ba);

	xml.setCodec("UTF-8");
	xml.writeStartDocument();
	xml.setAutoFormatting(true);
	xml.setAutoFormattingIndent(-3);

	xml.writeStartElement("s:Envelope");
	xml.writeAttribute("xmlns:s", "http://schemas.xmlsoap.org/soap/envelope/");
	xml.writeStartElement("s:Body");
	xml.writeStartElement("Login");
	xml.writeAttribute("xmlns", "http://tempuri.org/");
	xml.writeTextElement("userName", "administrator");
	xml.writeTextElement("password", "administrator");
	xml.writeEndElement(); // login
	xml.writeEndElement(); // Body
	xml.writeEndElement(); // Envelope
	xml.writeEndDocument();
	return ba;
}

QByteArray NvrManager::xmlCreateNvrInfo(const QString &uuid)
{
	QByteArray ba;
	QXmlStreamWriter xml(&ba);

	xml.setCodec("UTF-8");
	xml.writeStartDocument();
	xml.setAutoFormatting(true);
	xml.setAutoFormattingIndent(-3);

	xml.writeStartElement("s:Envelope");
	xml.writeAttribute("xmlns:s", "http://schemas.xmlsoap.org/soap/envelope/");
	xml.writeStartElement("s:Body");
	xml.writeStartElement("GetAllNVRs");
	xml.writeAttribute("xmlns", "http://tempuri.org/");
	xml.writeTextElement("sessionUuid", uuid);
	xml.writeEndElement(); // GetAllNVRs
	xml.writeEndElement(); // Body
	xml.writeEndElement(); // Envelope
	xml.writeEndDocument();
	return ba;
}

QByteArray NvrManager::xmlCreateEncInfo(const QString &uuid)
{
	QByteArray ba;
	QXmlStreamWriter xml(&ba);

	xml.setCodec("UTF-8");
	xml.writeStartDocument();
	xml.setAutoFormatting(true);
	xml.setAutoFormattingIndent(-3);

	xml.writeStartElement("s:Envelope");
	xml.writeAttribute("xmlns:s", "http://schemas.xmlsoap.org/soap/envelope/");
	xml.writeStartElement("s:Body");
	xml.writeStartElement("GetAllEncoders");
	xml.writeAttribute("xmlns", "http://tempuri.org/");
	xml.writeTextElement("sessionUuid", uuid);
	xml.writeEndElement(); // GetAllEncoders
	xml.writeEndElement(); // Body
	xml.writeEndElement(); // Envelope
	xml.writeEndDocument();
	return ba;
}

QByteArray NvrManager::xmlCreateRecordInterval(const QString &uuid, const QString &encuuid, const QString &startdate, const QString &enddate)
{
	QByteArray ba;
	QXmlStreamWriter xml(&ba);

	xml.setCodec("UTF-8");
	xml.writeStartDocument();
	xml.setAutoFormatting(true);
	xml.setAutoFormattingIndent(-3);

	xml.writeStartElement("s:Envelope");
	xml.writeAttribute("xmlns:s", "http://schemas.xmlsoap.org/soap/envelope/");
	xml.writeStartElement("s:Body");
	xml.writeStartElement("GetRecordedIntervalInformations");
	xml.writeAttribute("xmlns", "http://tempuri.org/");
	xml.writeTextElement("encoderUuid", encuuid);
	xml.writeTextElement("startDate", startdate);
	xml.writeTextElement("endDate", enddate);
	xml.writeTextElement("sessionUuid", uuid);
	xml.writeEndElement(); // GetRecordedIntervalInformations
	xml.writeEndElement(); // Body
	xml.writeEndElement(); // Envelope
	xml.writeEndDocument();
	return ba;
}

QJsonObject NvrManager::xmlToJson(QString data)
{
	int index = 0;
	QStringList stack;
	QJsonObject obj;
	QXmlStreamReader* xmlReader = new QXmlStreamReader(data);
	while(!xmlReader->atEnd() && !xmlReader->hasError()) {
		QXmlStreamReader::TokenType token = xmlReader->readNext();
		if(token == QXmlStreamReader::StartDocument) {
			continue;
		}

		if(token == QXmlStreamReader::StartElement) {
			stack.push_back(xmlReader->name().toString());
			if (xmlReader->name() == "DTOEncoder") {
				index++;
			}
			if (xmlReader->name() == "IntervalInformation") {
				index++;
			}
		}

		if(token == QXmlStreamReader::Characters) {
			if (index)
				jsonInsert(obj, QString("%1.%2").arg(index).arg(stack.last()), xmlReader->text().toString());
			else {
				jsonInsert(obj, stack.last(), xmlReader->text().toString());
			}
		}

		if(token == QXmlStreamReader::EndElement) {
			stack.pop_back();
		}

		if (token == QXmlStreamReader::EndDocument) {

		}
	}
	xmlReader->clear();
	return obj;
}

QList<NvrInfo> NvrManager::getNvrInfo(const QStringList &smIps, int timeout)
{
	QList<NvrInfo> nvrInfoList;
	foreach (QString smIp, smIps) {
		LoginInfo i = nvrLogin(smIp, timeout);
		if(i.uuid.isEmpty())
			continue;
		nvrInfoList = nvrInfo(smIp, i.uuid, timeout);
	}
#if 0
	foreach (NvrInfo nvrinfo, nvrInfoList) {
			qDebug() << nvrinfo.nvr_address;
			qDebug() << nvrinfo.nvr_block_path;
			qDebug() << nvrinfo.nvr_uuid;
			qDebug() << nvrinfo.total_number_of_encoders_per_nvr;
			qDebug() << nvrinfo.nvr_version;
		}
#endif
	return nvrInfoList;
}

QList<EncInfo> NvrManager::getEncInfo(const QStringList &smIps, int timeout)
{
	QList<EncInfo> encInfoList;
	foreach (QString smIp, smIps) {
		LoginInfo i = nvrLogin(smIp, timeout);
		if(i.uuid.isEmpty())
			continue;
		encInfoList = encInfo(smIp, i.uuid, timeout);
	}
#if 0
	foreach (EncInfo encinfo, encInfoList) {
//		qDebug() << encinfo.cam_name;
//		qDebug() << encinfo.cam_model;
//		qDebug() << encinfo.cam_high_resolution_view_address;
//		qDebug() << encinfo.cam_uuid;
		if (encinfo.cam_record_ip.contains("244"))
			qDebug() << encinfo.nvr_address << "getencinfo";

	}
#endif
	return encInfoList;
}

QStringList NvrManager::getRecordInfo(const QString &smIp, const QString &ip, qint64 days, int timeout)
{
	return recordInfo(smIp, ip, days, timeout);
}

QStringList NvrManager::recordInfo(const QString &smIp, const QString &ip, qint64 days, int timeout)
{
	if(days > 0)
		days = days * (-1);
	QStringList recordInfoList;
	QList<EncInfo> encInfoList;
	encInfoList = getEncInfo(QStringList() << smIp);
	QString endtime = QDateTime::currentDateTimeUtc().toString(Qt::ISODate); /* Today */
	QString starttime = QDateTime::currentDateTimeUtc().addDays(days).toString(Qt::ISODate);
	foreach (EncInfo encinfo, encInfoList) {
		if(encinfo.cam_record_ip.contains(ip)) {
			QByteArray ba = xmlCreateRecordInterval(login_uuid, encinfo.cam_uuid, starttime, endtime);
			QString url = encinfo.nvr_address;
			if(url.isEmpty())
				continue;
			QByteArray soapUrl = "http://tempuri.org/INvrService/GetRecordedIntervalInformations";
			QNetworkRequest request = networkRequest(url, soapUrl, ba);
			QPair<QString, QString> p = postReplyData(request, ba, timeout);
			if(!p.second.size()) {
				qDebug() << "Record Interval data is empty. error : " << p.first;
				continue;
			}
			QJsonObject obj = xmlToJson(p.second);
			recordInfoList = parseRecordInfo(obj);
		}
		else continue;
	}
	return recordInfoList;
}

QStringList NvrManager::parseRecordInfo(QJsonObject obj)
{
	QStringList keys = obj.keys();
	keys = stringSorting(keys);
	QStringList endTimeList;
	QStringList startTimeList;
	foreach (QString key, keys) {
		QJsonObject my_obj = obj.value(key).toObject();
		if(my_obj.value("EndDateTime").toString().isEmpty())
			continue;
		endTimeList << my_obj.value("EndDateTime").toString();
		startTimeList << my_obj.value("StartDateTime").toString();
	}
	QStringList recordIntervals;
	for (int i = 1; i < startTimeList.size(); i++) {
		QString tmp = QString("%1 - %2").arg(endTimeList.at(i-1)).arg(startTimeList.at(i));
		recordIntervals << tmp;
	}
	return recordIntervals;
}

QList<EncInfo> NvrManager::encInfo(const QString &smIp, const QString &uuid, int timeout)
{
	QByteArray ba = xmlCreateEncInfo(uuid);
	QString url = QString("http://%1:8733/IEncoderDeviceManager").arg(smIp);
	QByteArray soapUrl = "http://tempuri.org/IEncoderDeviceManager/GetAllEncoders";
	QNetworkRequest request = networkRequest(url, soapUrl, ba);
	QPair<QString, QString> p;
	p = postReplyData(request, ba, timeout);
	if(!p.second.size()) {
		qDebug() << "Return value is empty" << p.first;
		return QList<EncInfo>();
	}
	QList<EncInfo> encInfoList;
	QJsonObject obj = xmlToJson(p.second);
	encInfoList = parseEncInfo(obj, smIp);
	if(!encInfoList.size()) {
		qDebug() << "Parsing error Maybe data is empty";
		return QList<EncInfo>();
	}
	return encInfoList;
}

QList<EncInfo> NvrManager::parseEncInfo(QJsonObject obj, const QString &smIp)
{
	QJsonObject my_obj;
	QString serviceAddress;
	QList<EncInfo> encInfoList;
	QStringList keys = obj.keys();
	foreach (QString key, stringSorting(keys)) {
		EncInfo i;
		my_obj = obj.value(key).toObject();
		i.cam_uuid = my_obj.value("Uuid").toString();
		i.cam_model = my_obj.value("Model").toString();
//		i.cam_location = my_obj.value("Location").toString();
		i.cam_discovery_type = my_obj.value("DiscoveryType").toString();
		i.cam_network_status = my_obj.value("NetworkStatus").toString();
		i.cam_recording_status = my_obj.value("RecordingStatus").toString();
		if (!my_obj.value("ServiceAddress").toString().isEmpty()) {
			serviceAddress = my_obj.value("ServiceAddress").toString();
			i.nvr_address = serviceAddress;
			serviceAddress = serviceAddress.split("//").last();
			serviceAddress = serviceAddress.split(":").first();
			if(serviceAddress == "127.0.0.1")
				serviceAddress = smIp;
			i.nvr_address.replace("127.0.0.1", serviceAddress);
		}
		i.nvr_ip = serviceAddress;
		QString recordAddress = my_obj.value("RecordingAddress").toString();
		i.cam_recording_address = recordAddress;
		QString recordIp = recordAddress.split("//").last();
		recordIp = recordIp.split("/").first();
		i.cam_record_ip = recordIp;
		encInfoList << i;
	}
	return encInfoList;
}

QList<NvrInfo> NvrManager::nvrInfo(const QString &smIp, const QString &uuid, int timeout)
{
	QByteArray ba = xmlCreateNvrInfo(uuid);
	QString url = QString("http://%1:8733/INVRDeviceManager").arg(smIp);
	QString soapUrl = "http://tempuri.org/INVRDeviceManager/GetAllNVRs";
	QNetworkRequest request = networkRequest(url, soapUrl, ba);
	QPair<QString, QString> p;
	p = postReplyData(request, ba, timeout);
	if(!p.second.size()) {
		qDebug() << "Not return value from nvr info " << p.first;
		return QList<NvrInfo>();
	}

	QList<NvrInfo> nvrInfoList;
	QJsonObject obj = xmlToJson(p.second);
	nvrInfoList = parseNvrInfo(obj, smIp);

	if(!nvrInfoList.size()) {
		qDebug() << "Nvr info  list is empty";
		return QList<NvrInfo>();
	}
	return nvrInfoList;
}

QList<NvrInfo> NvrManager::parseNvrInfo(QJsonObject obj, const QString &smIp)
{
	int count = 0;
	QJsonObject my_obj;
	QStringList nvrlist;
	QString serviceAddress;
	QList <NvrInfo> nvrInfoList;
	QStringList keys = obj.keys();
	foreach (QString key, stringSorting(keys)) {
		NvrInfo i;
		my_obj = obj.value(key).toObject();
		if (!my_obj.value("ServiceAddress").toString().isEmpty()) {
			serviceAddress = my_obj.value("ServiceAddress").toString();
			serviceAddress = serviceAddress.split("//").last();
			serviceAddress = serviceAddress.split(":").first();
			if(serviceAddress == "127.0.0.1")
				serviceAddress = smIp;
		}
		if(nvrlist.contains(serviceAddress)) {
			count++;
			continue;
		}
		if(!serviceAddress.isEmpty()) {
			i.nvr_name = my_obj.value("Name").toString();
			i.nvr_uuid = my_obj.value("Uuid").toString();
			i.nvr_version = my_obj.value("Version").toString();
			i.nvr_block_path = my_obj.value("BlockPath").toString();

			i.nvr_address = serviceAddress;
			i.total_number_of_encoders_per_nvr = count;
			i.total_number_of_encoders = keys.size() - 3;
			nvrInfoList << i;
			count = 0;
		}
		nvrlist << serviceAddress;
	}
	return nvrInfoList;
}

LoginInfo NvrManager::nvrLogin(const QString &smIp, int timeout)
{
	QByteArray ba = xmlCreateLogin();
	QString url = QString("http://%1:8733/ISessionManager").arg(smIp);
	QString	soapUrl = "http://tempuri.org/ISessionManager/Login";
	QNetworkRequest request = networkRequest(url, soapUrl, ba);
	QPair<QString, QString> p;
	p = postReplyData(request, ba, timeout);
	if (!p.second.size()) {
		qDebug() << "return value is empty : nvrLogin: postReplyData";
		return LoginInfo();
	}
	LoginInfo i;
	if(!p.second.isEmpty()) {
		QJsonObject obj = xmlToJson(p.second);
		i = parseLogin(obj, smIp);
	}

	if(i.uuid.isEmpty()) {
		qDebug() << "uuid return value is empty";
		return LoginInfo();
	}
	login_uuid = i.uuid;
	return i;
}

LoginInfo NvrManager::parseLogin(QJsonObject obj, QString smIp)
{
	LoginInfo i;
	i.sm_ip = smIp;
	i.uuid = obj.value("Uuid").toString();
	i.status = obj.value("Status").toString();
	i.username = obj.value("UserName").toString();
	i.login_date = obj.value("LoginDate").toString();
	i.error_code = obj.value("ErrorCode").toString();
	return i;
}

void NvrManager::jsonInsert(QJsonObject &obj, QString key, QString value)
{
	if (key.contains(".")) {
		QStringList flds = key.split(".");
		QJsonObject stats_obj;
		stats_obj = obj.value(flds.at(0)).toObject();
		stats_obj[flds.at(1)] = value;
		obj.insert(flds.at(0), stats_obj);
	} else {
		obj.insert(key, value);
	}
}

QNetworkRequest NvrManager::networkRequest(const QString &url, const QString &soapUrl, const QByteArray &ba)
{
	QNetworkRequest request;
	request.setUrl(QUrl(url));

	request.setHeader(QNetworkRequest::ContentTypeHeader,
					  QVariant(QString("text/xml; charset=utf-8")));
	request.setHeader(QNetworkRequest::ContentLengthHeader,
					  QVariant(qulonglong(ba.size())));
	request.setAttribute(QNetworkRequest::CacheLoadControlAttribute,
						 QVariant(int(QNetworkRequest::AlwaysNetwork)));
	request.setRawHeader("SOAPAction", soapUrl.toLocal8Bit());
	return request;
}

#include <QTimer>
#include <QEventLoop>

QPair<QString, QString> NvrManager::postReplyData(QNetworkRequest request, QByteArray ba, int timeout)
{
	qDebug() << "Start post reply data";
	QNetworkReply *reply = manager->post(request, ba);
	connect(reply, SIGNAL(readyRead()), SLOT(readyRead()));
	connect(reply, SIGNAL(uploadProgress(qint64,qint64)), SLOT(uploadProgres(qint64,qint64)));
	connect(reply, SIGNAL(downloadProgress(qint64,qint64)), SLOT(downloadProgress(qint64,qint64)));
	QEventLoop el;
	QTimer t;
	t.setSingleShot(true);
	QObject::connect(reply, SIGNAL(finished()), &el, SLOT(quit()));
	QObject::connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), &el, SLOT(quit()));
	QObject::connect(&t, SIGNAL(timeout()), &el, SLOT(quit()));
	if (timeout)
		t.start(timeout);
	if (reply->isRunning())
		el.exec();
	QString err;
	QString data;
	if (!timeout || t.isActive()) {
		if (reply->error() != QNetworkReply::NoError)
			err = reply->errorString();
		else
			data = reply->readAll();
	}
	return QPair<QString, QString>(err, data);
}

