#include "RadioGui.h"
#include "ui_RadioGui.h"

#include <QThread>

#include <QFile>
#include <QFileInfo>
#include <QTextStream>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

#include <QMediaService>
#include <QMediaMetaData>
#include <QMetaDataReaderControl>

namespace
{

//full volume when starting and switching stations
const int ciDefaultVolume = 100;

//the stylesheet to use for the station label
const QString cstrDefaultLabelStyleSheet = QStringLiteral("QLabel { background-color: %1; }");

//the stylesheet to use for each station button
const QString cstrDefaultButtonStyleSheet = QStringLiteral("QPushButton {" \
																													 " border-radius: 15px;" \
																													 " border: 3px solid rgb(72, 126, 176);" \
																													 " color: rgb(72, 126, 176);" \
																													 " background-color: %1;" \
																													 " padding: 5px 5px 5px 5px;" \
																													 " outline: none;" \
																													 "}" \
																													 "QPushButton:checked {" \
																													 " border-radius: 15px;" \
																													 " border: 3px solid rgb(255,255,255);" \
																													 " color: rgb(255,255,255);" \
																													 " background-color: %2;" \
																													 " padding: 5px 5px 5px 5px;" \
																													 " outline: none;" \
																													 "}");

QPixmap ScaleLogoToRectangle(const QPixmap &logo, const QRect &rect, const double &factor = 1.0)
{
	auto scaledLogo = logo;

	double heightRatio = (1.0 * scaledLogo.height()) / (1.0 * rect.height());
	double widthRatio = (1.0 * scaledLogo.width()) / (1.0 * rect.width());

	if((1.0 < widthRatio) && (widthRatio > heightRatio))
	{
		scaledLogo = scaledLogo.scaledToWidth(rect.width() * factor, Qt::SmoothTransformation);

	}
	else if((1.0 < heightRatio) && (heightRatio > widthRatio))
	{
		scaledLogo = scaledLogo.scaledToHeight(rect.height() * factor, Qt::SmoothTransformation);
	}

	return scaledLogo;
}
//----------------------------------------------------------------------------------------------------------------------

template<typename T = QLabel>
void SetMaximumFontForTextContainer(T* l, const double &fraction = 0.9)
{
	//the available size
	auto newSize = l->contentsRect().size();

	QFontMetrics fm = l->fontMetrics();

	//we use only a fraction of the available width and height
	auto availableWidth = fraction * newSize.width();
	auto availableHeight = fraction * newSize.height();

	//detect the currently longest string
	auto cls = l->text();

	auto maxWidth = 1.0 * fm.width(cls);
	auto maxHeight = 1.0 * fm.height();

	QFont font(l->font());

	auto widthScaling = availableWidth / maxWidth;
	auto heightScaling = availableHeight / maxHeight;

	//scale size according to size change but only up to the minimum value possible
	font.setPointSizeF(font.pointSizeF() * qMin(widthScaling, heightScaling));
	l->setFont(font);
}
//----------------------------------------------------------------------------------------------------------------------

/**
 * @brief ReadStationsFromFile Import station description from a UTF-8 encoded json file
 * @param file The file to import
 * @return The found stations
 */
QList<StationInformation> ReadStationsFromFile(const QString &file)
{
	QList<StationInformation> stations;

	QFileInfo fi(file);
	if(false == fi.exists()) return stations;

	QFile f(fi.absoluteFilePath());
	if(true == f.open(QFile::ReadOnly))
	{
		QTextStream s(&f);
		QJsonDocument jsonDocument = QJsonDocument::fromJson(s.readAll().toUtf8());

		const auto jsonObj = jsonDocument.object();

		//all top-level keys are used as  the station names (which may get overwritten when meta-data is found)
		for(const auto &stationName : jsonObj.keys())
		{
			const auto stationObject = jsonObj.value(stationName).toObject();

			bool validLogo = (stationObject.contains("logo") ||
												stationObject.contains("logo-file") ||
												stationObject.contains("logo-url"));

			bool valid = stationObject.contains("url") &&
									 validLogo &&
									 stationObject.contains("meta_key_1") &&
									 stationObject.contains("meta_key_2");

			if(false == valid) continue;

			StationInformation station;
			station.m_strDefaultPublisher = stationName;
			station.m_strMediaUrl = stationObject.value("url").toString();
			station.m_strFirstMetadataKey = stationObject.value("meta_key_1").toString();
			station.m_strSecondMetadataKey = stationObject.value("meta_key_2").toString();

			//we expect the logo data as base64 data
			if(true == stationObject.contains("logo"))
			{
				auto pixmapData = stationObject.value(QString("logo")).toString().toLatin1();
				station.m_pStationLogo.loadFromData(QByteArray::fromBase64(pixmapData));
			}
			//alternatively we allow the logo-file point to a valid image file
			else if(true == stationObject.contains("logo-file"))
			{
				auto pixmapPath = stationObject.value(QString("logo-file")).toString();
				station.m_pStationLogo.load(pixmapPath);
			}
			else if(true == stationObject.contains("logo-url"))
			{
				//only store the url, download needs to be performed when needed
				station.m_uLogoUrl = stationObject.value(QString("logo-url")).toString();
			}

			{
				station.m_cBackgroundColorNormal = Qt::black;

				if(true == stationObject.contains("background-color-normal"))
				{
					auto backgroundColor = stationObject.value(QString("background-color-normal")).toString();

					QColor c(backgroundColor);
					if(true == c.isValid())
					{
						station.m_cBackgroundColorNormal = c;
					}
				}
			}

			{
				station.m_cBackgroundColorChecked = QColor(72, 126, 176);

				if(true == stationObject.contains("background-color-checked"))
				{
					auto backgroundColor = stationObject.value(QString("background-color-checked")).toString();

					QColor c(backgroundColor);
					if(true == c.isValid())
					{
						station.m_cBackgroundColorChecked = c;
					}
				}
			}

			stations.append(station);
		}
	}

	return stations;
}
//----------------------------------------------------------------------------------------------------------------------

}

RadioGui::RadioGui(QWidget *parent)
	: QMainWindow(parent)
	, m_ui(new Ui::RadioGui)
	, m_Player(new QMediaPlayer(), [](QMediaPlayer* p) { p->deleteLater(); })
	, m_LogoDownLoader(new LogoDownloader(), [](LogoDownloader* d) { d->deleteLater(); })
{
	m_ui->setupUi(this);

	qRegisterMetaType<StationInformation>();

	m_ui->btnPlayingPage->setChecked(true);
	m_ui->stackedWidget->setCurrentWidget(m_ui->pagePlaying);
	m_ui->sliderVolume->setValue(ciDefaultVolume);

	connect(m_ui->btnPlayingPage, &QPushButton::clicked, this, &RadioGui::on_btnNavigation_clicked);
	connect(m_ui->btnSelectSourcePage, &QPushButton::clicked, this, &RadioGui::on_btnNavigation_clicked);
	connect(m_ui->btnSettingsPage, &QPushButton::clicked, this, &RadioGui::on_btnNavigation_clicked);

	connect(m_Player.get(), &QMediaPlayer::volumeChanged, m_ui->sliderVolume, &QSlider::setValue);
	connect(m_Player.get(), &QMediaPlayer::currentMediaChanged, this, &RadioGui::onMediaChanged);
	connect(m_Player.get(), &QMediaPlayer::metaDataAvailableChanged, this, &RadioGui::onMediaChanged);

	connect(m_ui->sliderVolume, &QSlider::valueChanged, m_Player.get(), &QMediaPlayer::setVolume);

	//load available stations
	loadStations();

	//play first station
	if(true == m_ui->btn1->isEnabled())
	{
		emit m_ui->btn1->clicked();
	}
}
//----------------------------------------------------------------------------------------------------------------------

RadioGui::~RadioGui()
{
	delete m_ui;
}
//----------------------------------------------------------------------------------------------------------------------

void RadioGui::loadStations()
{
	QList<StationInformation> cStations = ReadStationsFromFile(QString("stations.json"));

	auto buttons = m_ui->pageSelectSource->findChildren<QPushButton*>();

	//disable all per default
	for(auto button : buttons) { button->setEnabled(false); }

	for(auto i = 0; i < qMin(cStations.size(), buttons.size()); ++i)
	{
		auto button = m_ui->pageSelectSource->findChild<QPushButton*>(QString("btn%1").arg(i+1));

		Q_ASSERT(nullptr != button);
		if(nullptr == button) button = buttons.at(i);

		auto station = cStations[i];
		button->setProperty("station", QVariant::fromValue(station));

		//if a valid url is given, we need to download the logo now
		if(true == station.m_uLogoUrl.isValid())
		{
			m_LogoDownLoader->downloadLogo(station.m_uLogoUrl,
																		 [=](QPixmap logo)
																		 {
																				auto s = button->property("station").value<StationInformation>();
																				button->setIcon(QIcon(logo));
																				s.m_pStationLogo = logo;
																				button->setProperty("station", QVariant::fromValue(s));

																				if(true == button->icon().isNull())
																				{
																					button->setText(s.m_strDefaultPublisher);
																					SetMaximumFontForTextContainer<QAbstractButton>(button);
																				}
																		 });
		}
		else
		{
			button->setIcon(QIcon(station.m_pStationLogo));

			if(true == button->icon().isNull())
			{
				button->setText(station.m_strDefaultPublisher);
				SetMaximumFontForTextContainer<QAbstractButton>(button);
			}
		}

		//we replace the stylesheet with a modified one using the specified colors for the station
		auto styleSheet = cstrDefaultButtonStyleSheet;
		styleSheet = styleSheet.arg(station.m_cBackgroundColorNormal.name(QColor::HexArgb));
		styleSheet = styleSheet.arg(station.m_cBackgroundColorChecked.name(QColor::HexArgb));

		button->setStyleSheet(styleSheet);

		button->setChecked(button == m_ui->btn1);
		button->setEnabled(true);

		connect(button, &QPushButton::clicked, this, &RadioGui::on_btnSource_clicked);
	}
}
//----------------------------------------------------------------------------------------------------------------------

void RadioGui::on_btnStartStop_clicked()
{
	m_ui->btnStartStop->setIcon(m_ui->btnStartStop->isChecked() ? QIcon(":/Resources/Resources/pause.png") :
																																QIcon(":/Resources/Resources/play-button.png"));

	if(true == m_ui->btnStartStop->isChecked())
	{
		if(QMediaPlayer::StoppedState == m_Player->state())
		{
			m_Player->setMedia(QMediaContent(m_CurrentStation.m_strMediaUrl));
			m_Player->setVolume(ciDefaultVolume);
		}
		m_Player->play();
	}
	else
	{
		if(QMediaPlayer::PlayingState == m_Player->state())
		{
			m_Player->stop();
		}
	}
}
//----------------------------------------------------------------------------------------------------------------------

void RadioGui::on_btnNavigation_clicked()
{
	QPushButton* pressedButton{};

	for(auto button : m_ui->navigationButtonsWidget->findChildren<QPushButton*>())
	{
		button->setChecked(button == sender());

		if(button == sender()) pressedButton = button;
	}

	if(m_ui->btnPlayingPage == pressedButton)
	{
		 m_ui->stackedWidget->setCurrentWidget(m_ui->pagePlaying);
	}
	else if(m_ui->btnSelectSourcePage == pressedButton)
	{
		 m_ui->stackedWidget->setCurrentWidget(m_ui->pageSelectSource);
	}
	else if(m_ui->btnSettingsPage == pressedButton)
	{
		 m_ui->stackedWidget->setCurrentWidget(m_ui->pageSettings);
	}
}
//----------------------------------------------------------------------------------------------------------------------

void RadioGui::on_btnSource_clicked()
{
	QPushButton* clickedButton{};

	for(auto button : m_ui->pageSelectSource->findChildren<QPushButton*>())
	{
		button->setChecked(button == sender());
		if(button == sender()) clickedButton = button;
	}

	if(nullptr != clickedButton)
	{
		m_CurrentStation = clickedButton->property("station").value<StationInformation>();

		//the station logo may be too small or too big, scale for consistent look
		auto logo = ScaleLogoToRectangle(QPixmap(m_CurrentStation.m_pStationLogo), m_ui->lblStation->contentsRect(), 0.8);

		const auto style = cstrDefaultLabelStyleSheet.arg(m_CurrentStation.m_cBackgroundColorNormal.name(QColor::HexArgb));
		m_ui->lblStation->setStyleSheet(style);
		m_ui->lblStation->setPixmap(logo);


		m_ui->labelInfo1->setText(m_CurrentStation.m_strDefaultPublisher);
		m_ui->labelInfo2->setText(QString());

		SetMaximumFontForTextContainer(m_ui->labelInfo1);
		SetMaximumFontForTextContainer(m_ui->labelInfo2);

		if(QMediaPlayer::PlayingState == m_Player->state())
		{
			m_Player->stop();
		}

		m_Player->setMedia(QMediaContent(m_CurrentStation.m_strMediaUrl));
		m_Player->setVolume(ciDefaultVolume);
		m_Player->play();

		m_ui->btnStartStop->setChecked(true);
		m_ui->btnStartStop->setIcon(QIcon(":/Resources/Resources/pause.png"));

		emit m_ui->btnPlayingPage->clicked();
	}
}
//----------------------------------------------------------------------------------------------------------------------

void RadioGui::onMediaChanged()
{
	for(auto i : m_Player->availableMetaData())
	{
		if(m_CurrentStation.m_strFirstMetadataKey.toLower() == i.toLower())
		{
			m_ui->labelInfo1->setText(m_Player->metaData(i).toString());
			SetMaximumFontForTextContainer(m_ui->labelInfo1);
		}

		if(m_CurrentStation.m_strSecondMetadataKey.toLower() == i.toLower())
		{
			m_ui->labelInfo2->setText(m_Player->metaData(i).toString());
			SetMaximumFontForTextContainer(m_ui->labelInfo2);
		}
	}
}
//----------------------------------------------------------------------------------------------------------------------
