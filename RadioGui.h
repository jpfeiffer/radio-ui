#pragma once

#include <memory>

#include <QMainWindow>
#include <QMediaPlayer>

#include "LogoDownloader.h"

namespace Ui
{

class RadioGui;

}

struct StationInformation
{
	QString m_strDefaultPublisher;

	QString m_strMediaUrl;

	QUrl m_uLogoUrl;
	QPixmap m_pStationLogo;

	QString m_strFirstMetadataKey;
	QString m_strSecondMetadataKey;
};

/**
 * @brief The RadioGui class provides the main window for the radio gui
 */
class RadioGui : public QMainWindow
{
	Q_OBJECT

 public:

	/**
	 * @brief RadioGui Default constructor
	 * @param parent
	 */
	explicit RadioGui(QWidget *parent = nullptr);

	/**
	 * @brief ~RadioGui Default destructor
	 */
	virtual ~RadioGui();

private slots:

	void loadStations();

	void on_btnStartStop_clicked();

	void on_btnNavigation_clicked();

	void on_btnSource_clicked();

	void onMediaChanged();

private:

	/**
	 * @brief ui The user interface
	 */
	Ui::RadioGui* m_ui;

	/**
	 * @brief m_CurrentMedia The currently selected media
	 */
	StationInformation m_CurrentStation;

	/**
	 * @brief m_Player The instance used to play the media urls
	 */
	std::shared_ptr<QMediaPlayer> m_Player;

	/**
	 * @brief m_LogoDownLoader Used when station logos need to be downloaded
	 */
	std::shared_ptr<LogoDownloader> m_LogoDownLoader;
};

Q_DECLARE_METATYPE(StationInformation)
