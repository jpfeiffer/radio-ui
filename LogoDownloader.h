#pragma once

#include <functional>

#include <QObject>

#include <QUrl>

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

/**
 * @brief The LogoDownloader class is a small helper for logo downloads
 */
class LogoDownloader : public QObject
{
	Q_OBJECT

public:

	typedef std::function<void(QPixmap)> LogoReceiver;

	/**
	 * @brief LogoDownloader Default constructor
	 * @param parent
	 */
	explicit LogoDownloader();

	/**
	 * @brief downloadLogo Request a logo for download
	 * @param url The logo to download
	 */
	void downloadLogo(const QUrl &url, const LogoReceiver &receiver);

private slots:

	/**
	 * @brief onDownloadFinished Called when the request is finished
	 * @param reply The reply
	 */
	void onDownloadFinished(QNetworkReply* reply);

private:

	/**
	 * @brief m_Manager The instance for the download request
	 */
	QNetworkAccessManager m_Manager;

	/**
	 * @brief m_Jobs All jobs
	 */
	QMap<QString, LogoReceiver> m_Jobs;
};
