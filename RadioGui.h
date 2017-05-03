#pragma once

#include <memory>

#include <QMainWindow>
#include <QMediaPlayer>

#include "LogoDownloader.h"

namespace Ui
{

class RadioGui;

}

/**
 * @brief The StationInformation struct contains all information to describe a single radtio station stream
 */
struct StationInformation
{
	/**
	 * @brief m_strDefaultPublisher The station publisher information, this is usually the station name
	 *
	 * @note This string is used as the default title when playing this station. If the first metadata key returns any
	 * useful string data, that string is displayed instead.
	 */
	QString m_strDefaultPublisher;

	//! From where to load the audio, must point to a format understandable by QMediaPlayer
	QString m_strMediaUrl;

	/**
	 * @brief m_uLogoUrl The RadioGui tries to download the station logo from this url if provided
	 *
	 * @note Must be in a format suitable for QPixmap
	 */
	QUrl m_uLogoUrl;

	/**
	 * @brief m_pStationLogo The station logo to display.
	 *
	 * @note If m_uLogoUrl is set, this pixmap is set to the image data downloaded from m_uLogoUrl
	 */
	QPixmap m_pStationLogo;

	/**
	 * @brief m_strFirstMetadataKey The data to display as the title for this stream, if this key can be found in the
	 * meta data returned from the stream, that string is displayed
	 */
	QString m_strFirstMetadataKey;

	/**
	 * @brief m_strSecondMetadataKey The secondary information to be displayed below the main metadata string
	 */
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

	/**
	 * @brief loadStations Called during construction to load the stations from the stations.json file
	 */
	void loadStations();

	/**
	 * @brief on_btnStartStop_clicked Stop playing the current station. Does nothing if no station is playing
	 */
	void on_btnStartStop_clicked();

	/**
	 * @brief on_btnNavigation_clicked Switch between the main gui pages
	 */
	void on_btnNavigation_clicked();

	/**
	 * @brief on_btnSource_clicked Switch to another station for playback
	 */
	void on_btnSource_clicked();

	/**
	 * @brief onMediaChanged Used to detect when new media is available. This method is used to extract any available
	 * meta data from the stream and tries to read the meta data keys contained in the station information.
	 */
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

//we want to store StationInformation values as properties in QObject instances
Q_DECLARE_METATYPE(StationInformation)
