/**
 * \file importselector.h
 * Import selector widget.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 17 Sep 2003
 */

#ifndef IMPORTSELECTOR_H
#define IMPORTSELECTOR_H

#include "importtrackdata.h"
#include <qvbox.h>
#include <qstring.h>
#include <qstringlist.h>

class QPushButton;
class QComboBox;
class QLineEdit;
class QCheckBox;
class QSpinBox;
class ImportParser;
class StandardTags;
class FreedbDialog;
class FreedbConfig;
class MusicBrainzDialog;
class MusicBrainzConfig;
class ImportTable;

/**
 * Import selector widget.
 */
class ImportSelector : public QVBox
{
Q_OBJECT

public:
	/** Import destinations */
	enum Destination { DestV1, DestV2 };

	/**
	 * Constructor.
	 *
	 * @param parent        parent widget
	 * @param trackDataList track data to be filled with imported values,
	 *                      is passed with durations of files set
	 * @param name          Qt name
	 * @param f             window flags
	 */
	ImportSelector(QWidget *parent, ImportTrackDataVector& trackDataList,
								 const char *name = 0, WFlags f = 0);
	/**
	 * Destructor.
	 */
	~ImportSelector();
	/**
	 * Look for album specific information (artist, album, year, genre) in
	 * a header (e.g. in a freedb header).
	 *
	 * @param st standard tags to put resulting values in,
	 *           fields which are not found are not touched.
	 *
	 * @return true if one or more field were found.
	 */
	bool parseHeader(StandardTags &st);
	/**
	 * Get next line as standardtags from imported file or clipboard.
	 *
	 * @param st standard tags
	 * @param start true to start with the first line, false for all
	 *              other lines
	 *
	 * @return true if ok (result in st),
	 *         false if end of file reached.
	 */
	bool getNextTags(StandardTags &st, bool start);
	/**
	 * Set ID3v1 or ID3v2 tags as import destination.
	 *
	 * @param dest DestV1 or DestV2 for ID3v1 or ID3v2
	 */
	void setDestination(ImportSelector::Destination dest);
	/**
	 * Get import destination.
	 *
	 * @return DestV1 or DestV2 for ID3v1 or ID3v2.
	 */
	Destination getDestination();
	/**
	 * Set import format regexp.
	 *
	 * @param names   import format names list
	 * @param headers import format header regexps
	 * @param tracks  import format track regexps
	 * @param idx     selected index
	 */
	void setImportFormat(const QStringList &names,
						 const QStringList &headers,
						 const QStringList &tracks,
						 int idx);
	/**
	 * Get import format regexp.
	 *
	 * @param names   import format names
	 * @param headers import format header regexp
	 * @param tracks  import format track regexp
	 *
	 * @return index of current selection.
	 */
	int getImportFormat(QString &names,
						QString &headers,
						QString &tracks) const;
	/**
	 * Set freedb.org configuration.
	 *
	 * @param cfg freedb configuration.
	 */
	void setFreedbConfig(const FreedbConfig *cfg);
	/**
	 * Get freedb.org configuration.
	 *
	 * @param cfg freedb configuration.
	 */
	void getFreedbConfig(FreedbConfig *cfg) const;
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
	/**
	 * Get list with track durations.
	 *
	 * @return list with track durations,
	 *         0 if no track durations found.
	 */
	QValueList<int>* getTrackDurations();
	/**
	 * Set time difference check configuration.
	 *
	 * @param enable  true to enable check
	 * @param maxDiff maximum allowable time difference
	 */ 
	void setTimeDifferenceCheck(bool enable, int maxDiff);
	/**
	 * Get time difference check configuration.
	 *
	 * @param enable  true if check is enabled
	 * @param maxDiff maximum allowed time difference
	 */ 
	void getTimeDifferenceCheck(bool& enable, int& maxDiff) const;
	/**
	 * List with line formats.
	 * The following codes are used before the () expressions.
	 * %s title (song)
	 * %l album
	 * %a artist
	 * %c comment
	 * %y year
	 * %t track
	 * %g genre
	 */
	static const char **lineFmtList;

public slots:
	/**
	 * Called when the maximum time difference value is changed.
	 */
	void maxDiffChanged();
	/**
	 * Move a table row.
	 *
	 * @param section not used
	 * @param fromIndex index of position moved from
	 * @param fromIndex index of position moved to
	 */
	void moveTableRow(int, int fromIndex, int toIndex);
	/**
	 * Let user select file, assign file contents to text and preview in
	 * table.
	 */
	void fromFile();
	/**
	 * Assign clipboard contents to text and preview in table.
	 */
	void fromClipboard();
	/**
	 * Import from freedb.org and preview in table.
	 */
	void fromFreedb();
	/**
	 * Import from MusicBrainz and preview in table.
	 */
	void fromMusicBrainz();
	/**
	 * Set the format lineedits to the format selected in the combo box.
	 *
	 * @param index current index of the combo box
	 */
	void setFormatLineEdit(int index);
	/**
	 * Called when freedb.org album data is received.
	 *
	 * @param txt text containing album data from freedb.org
	 */
	void freedbAlbumDataReceived(QString txt);
	/**
	 * Show fields to import in text as preview in table.
	 */
	void showPreview();
private:
	enum TabColumn {
		LengthColumn, TrackColumn, TitleColumn, ArtistColumn,
		AlbumColumn, YearColumn, GenreColumn, CommentColumn, NumColumns
	};
	enum ImportSource {
		None, File, Clipboard, Freedb, MusicBrainz
	};
	/**
	 * Update track data list with imported tags.
	 *
	 * @param impSrc import source
	 *
	 * @return true if tags were found.
	 */
	bool updateTrackData(ImportSource impSrc);
	/** From File button */
	QPushButton *fileButton;
	/** From Clipboard button */
	QPushButton *clipButton;
	/** From freedb.org button */
	QPushButton *freedbButton;
	/** From MusicBrainz button */
	QPushButton *m_musicBrainzButton;
	/** Preview table */
	ImportTable *tab;
	/** contents of imported file/clipboard */
	QString text;
	/** combobox with import destinations */
	QComboBox *destComboBox;
	/** combobox with import formats */
	QComboBox *formatComboBox;
	/** LineEdit for header regexp */
	QLineEdit *headerLineEdit;
	/** LineEdit for track regexp */
	QLineEdit *trackLineEdit;
	QCheckBox* mismatchCheckBox;
	QSpinBox* maxDiffSpinBox;
	/** header parser object */
	ImportParser *header_parser;
	/** track parser object */
	ImportParser *track_parser;
	/** header format regexps */
	QStringList formatHeaders;
	/** track format regexps */
	QStringList formatTracks;
	/** freedb.org import dialog */
	FreedbDialog *freedbDialog;
	/** freedb.org import dialog */
	MusicBrainzDialog* m_musicBrainzDialog;
	/** import source */
	ImportSource importSource;
	/** freedb config */
	const FreedbConfig *freedbCfg;
	/** MusicBrainz config */
	const MusicBrainzConfig *m_musicBrainzCfg;
	/** track data */
	ImportTrackDataVector& m_trackDataVector;
};

#endif
