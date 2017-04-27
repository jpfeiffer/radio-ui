#include "LogoDownloader.h"

#include <QPixmap>

LogoDownloader::LogoDownloader()
	: QObject(nullptr)
	, m_Manager()
{
	connect(&m_Manager, &QNetworkAccessManager::finished, this, &LogoDownloader::onDownloadFinished);
}
//----------------------------------------------------------------------------------------------------------------------

void LogoDownloader::downloadLogo(const QUrl &url, const LogoReceiver &receiver)
{
	m_Jobs.insert(url.toString(), receiver);

	//issue a new request
	QNetworkRequest request(url);
	m_Manager.get(request);
}
//----------------------------------------------------------------------------------------------------------------------

void LogoDownloader::onDownloadFinished(QNetworkReply* reply)
{
	if(false == m_Jobs.contains(reply->url().toString())) return;

	QPixmap logo;

	if(QNetworkReply::NoError == reply->error())
	{
		logo.loadFromData(reply->readAll());
	}

	reply->deleteLater();

	auto receiver = m_Jobs.value(reply->url().toString());
	if(nullptr != receiver) receiver(logo);

	m_Jobs.remove(reply->url().toString());
}
//----------------------------------------------------------------------------------------------------------------------
