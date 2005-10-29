/**
 * \file musicbrainzdialog.h
 * MusicBrainz import dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 13 Sep 2005
 */

#ifndef MUSICBRAINZDIALOG_H
#define MUSICBRAINZDIALOG_H

#include "config.h"

#include "importtrackdata.h"
#include <qdialog.h>
#include <qstring.h>
#if QT_VERSION >= 300
#include <qvaluevector.h>
#else
#include <qvaluelist.h>
#endif

class QLineEdit;
class QComboBox;
class QPushButton;
class QCheckBox;
class QTable;
class QTimer;
class MusicBrainzConfig;
class MusicBrainzClient;

/**
 * musicBrainz.org import dialog.
 */
class MusicBrainzDialog : public QDialog {
Q_OBJECT

public:
	/**
	 * Constructor.
	 *
	 * @param parent          parent widget
	 * @param trackDataVector track data to be filled with imported values,
	 *                        is passed with filenames set
	 */
	MusicBrainzDialog(QWidget* parent,
										ImportTrackDataVector& trackDataVector);
	/**
	 * Destructor.
	 */
	~MusicBrainzDialog();

#ifdef HAVE_TUNEPIMP
	/**
	 * Get string with server and port.
	 *
	 * @return "servername:port".
	 */
	QString getServer() const;

	/**
	 * Set string with server and port.
	 *
	 * @param srv "servername:port"
	 */
	void setServer(const QString& srv);

	/**
	 * Get proxy.
	 *
	 * @param used is set to true if proxy is used
	 *
	 * @return proxy, e.g. "myproxy:8080".
	 */
	QString getProxy(bool* used) const;

	/**
	 * Set proxy.
	 *
	 * @param proxy proxy, e.g. "myproxy:8080"
	 * @param used is set to true if proxy is used
	 */
	void setProxy(const QString& proxy, bool used);

	/**
	 * Set MusicBrainz configuration.
	 *
	 * @param cfg MusicBrainz configuration.
	 */
	void setMusicBrainzConfig(const MusicBrainzConfig* cfg);

	/**
	 * Get MusicBrainz configuration.
	 *
	 * @param cfg MusicBrainz configuration.
	 */
	void getMusicBrainzConfig(MusicBrainzConfig* cfg) const;
#endif // HAVE_TUNEPIMP

signals:
	/**
	 * Emitted when the m_trackDataVector was updated with new imported data.
	 */
	void trackDataUpdated();

public slots:
	/**
	 * Shows the dialog as a modal dialog.
	 */
	int exec();

protected slots:
	/**
	 * Hides the dialog and sets the result to QDialog::Accepted.
	 */
	virtual void accept();

	/**
	 * Hides the dialog and sets the result to QDialog::Rejected.
	 */
	virtual void reject();

private slots:
	/**
	 * Set the configuration in the client.
	 */
	void setClientConfig();

	/**
	 * Called when the periodic timer times out.
	 * Used to poll the MusicBrainz client.
	 */
	void timerDone();

	/**
	 * Apply imported data.
	 */
	void apply();

	/**
	 * Set the status of a file.
	 *
	 * @param index  index of file
	 * @param status status string
	 */
	void setFileStatus(int index, QString status);

	/**
	 * Update the track data combo box of a file.
	 *
	 * @param index  index of file
	 */
	void updateFileTrackData(int index);

	/**
	 * Set meta data for a file.
	 *
	 * @param index     index of file
	 * @param trackData meta data
	 */
	void setMetaData(int index, ImportTrackData& trackData);

	/**
	 * Set result list for a file.
	 *
	 * @param index           index of file
	 * @param trackDataVector result list
	 */
	void setResults(int index, ImportTrackDataVector& trackDataVector);

#ifdef HAVE_TUNEPIMP
private:
	/**
	 * Clear all results.
	 */
	void clearResults();

 /**
	* Create and start the MusicBrainz client.
	*/
	void startClient();

	/**
	 * Stop and destroy the MusicBrainz client.
	 */
	void stopClient();

	QComboBox* m_serverComboBox;
	QCheckBox* m_proxyCheckBox;
	QLineEdit* m_proxyLineEdit;
	QTable* m_albumTable;
	QTimer* m_timer;
	MusicBrainzClient* m_client;
	ImportTrackDataVector& m_trackDataVector;
#if QT_VERSION >= 300
	QValueVector<ImportTrackDataVector> m_trackResults;
#else
	QValueList<ImportTrackDataVector> m_trackResults;
#endif
#endif // HAVE_TUNEPIMP
};

#endif // MUSICBRAINZDIALOG_H
