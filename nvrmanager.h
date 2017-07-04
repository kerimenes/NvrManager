#ifndef NVRMANAGER_H
#define NVRMANAGER_H

#include <QObject>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QNetworkAccessManager>

struct LoginInfo {
	QString uuid;
	QString sm_ip;
	QString status;
	QString username;
	QString login_date;
	QString error_code;
};

struct NvrInfo {
	QString nvr_name;
	QString nvr_uuid;
	QString nvr_address;
	QString nvr_version;
	QString nvr_block_path;
	qint16 total_number_of_encoders;
	qint16 total_number_of_encoders_per_nvr;
};

struct EncInfo {
	QString cam_name;
	QString cam_uuid;
	QString cam_model;
//	QString cam_location;
	QString cam_record_ip;
	QString cam_discovery_type;
	QString cam_network_status;
	QString cam_recording_status;
	QString cam_recording_address;
	QString cam_low_resolution_view_address;
	QString cam_high_resolution_view_address;
	QString nvr_ip;
	QString nvr_address;
};

class NvrManager : public QObject
{
	Q_OBJECT
public:
	explicit NvrManager(QObject *parent = 0);

	QList<NvrInfo> getNvrInfo(const QStringList &smIps, int timeout = 10000);
	QList<EncInfo> getEncInfo(const QStringList &smIps, int timeout = 10000);
	QStringList getRecordInfo(const QString &smIp, const QString &ip, qint64 days, int timeout = 10000);
signals:

public slots:
protected slots:
	void readyRead();
	void uploadProgres(qint64 bytesSent, qint64 bytesTotal);
	void downloadProgress(qint64 bytesReceiver, qint64 bytesTotal);
protected:
	QByteArray xmlCreateLogin();
	QByteArray xmlCreateNvrInfo(const QString &uuid);
	QByteArray xmlCreateEncInfo(const QString &uuid);
	QByteArray xmlCreateRecordInterval(const QString &uuid, const QString &encuuid, const QString &startdate, const QString &enddate);

	LoginInfo nvrLogin(const QString &smIp, int timeout);
	LoginInfo parseLogin(QJsonObject obj, QString smIp);
	QList<NvrInfo> nvrInfo(const QString &smIp, const QString &uuid, int timeout);
	QList<NvrInfo> parseNvrInfo(QJsonObject obj, const QString &smIp);
	QList<EncInfo> encInfo(const QString &smIp, const QString &uuid, int timeout);
	QList<EncInfo> parseEncInfo(QJsonObject obj, const QString &smIp);
	QStringList recordInfo(const QString &smIp, const QString &ip, qint64 days, int timeout);
	QStringList parseRecordInfo(QJsonObject obj);

	QJsonObject xmlToJson(QString data);
	void jsonInsert(QJsonObject &obj, QString key, QString value);
	QPair<QString, QString> postReplyData(QNetworkRequest request, QByteArray ba, int timeout);
	QNetworkRequest networkRequest(const QString &url, const QString &soapUrl, const QByteArray &ba);

private:
	QNetworkAccessManager* manager;
	QString login_uuid;
};

#endif // NVRMANAGER_H
