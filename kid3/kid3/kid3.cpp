/**
 * \file kid3.cpp
 * Kid3 application.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 9 Jan 2003
 */

#include "config.h"
#include <qdir.h>
#include <qprinter.h>
#include <qpainter.h>
#include <qurl.h>
#include <qtextstream.h>
#include <qcursor.h>
#include <qmessagebox.h>
#include <qpushbutton.h>
#include "qtcompatmac.h"
#if QT_VERSION >= 0x040000
#include <Q3ProgressBar>
#include <Q3GroupBox>
#include <QCloseEvent>
#include <Q3ValueList>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <Q3PopupMenu>
#else
#include <qprogressbar.h>
#include <qgroupbox.h>
#endif

#ifdef CONFIG_USE_KDE
#include <kapp.h>
#include <kurl.h>
#include <kmenubar.h>
#include <kstatusbar.h>
#include <kconfig.h>
#include <kstdaction.h>
#include <kaction.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <kkeydialog.h>
#else
#include <qapplication.h>
#include <qmenubar.h>
#include <qaction.h>
#include <qstatusbar.h>
#include <qmessagebox.h>
#include <qfiledialog.h>
#endif

#include "kid3.h"
#include "id3form.h"
#include "genres.h"
#include "framelist.h"
#include "configdialog.h"
#include "importdialog.h"
#include "exportdialog.h"
#include "numbertracksdialog.h"
#include "standardtags.h"
#include "rendirdialog.h"
#include "filelistitem.h"
#include "dirlist.h"
#ifdef HAVE_ID3LIB
#include "mp3file.h"
#endif
#ifdef HAVE_VORBIS
#include "oggfile.h"
#endif
#ifdef HAVE_FLAC
#include "flacfile.h"
#endif
#ifdef HAVE_TAGLIB
#include "taglibfile.h"
#endif

#ifdef KID3_USE_KCONFIGDIALOG
#include <kconfigskeleton.h>
#endif

#ifndef CONFIG_USE_KDE
#include <qdialog.h>
#include <qtextbrowser.h>
#include <qtextcodec.h>

/**
 * Help browser.
 */
class BrowserDialog : public QDialog {
public:
	/**
	 * Constructor.
	 */
	BrowserDialog(QWidget* parent, QString& caption);

	/**
	 * Destructor.
	 */
	~BrowserDialog();

	/**
	 * Show context help.
	 * @param anchor name of anchor
	 */
	void goToAnchor(const QString& anchor);

private:
	QTextBrowser* m_textBrowser;
	QString m_filename;
};

BrowserDialog::BrowserDialog(QWidget* parent, QString& caption)
	: QDialog(parent, "browser")
{
	setCaption(caption);
	QVBoxLayout* vlayout = new QVBoxLayout(this);
	if (!vlayout) {
		return ;
	}
	vlayout->setSpacing(6);
	vlayout->setMargin(6);

	m_textBrowser = new QTextBrowser(this, "textBrowser");
	m_filename = QDir::currentDirPath() + QDir::separator() + "kid3_";
	QString lang((QString(QTextCodec::locale())).left(2));
	if (!QFile::exists(m_filename + lang + ".html")) {
		lang = "en";
	}
	m_filename += lang + ".html";
#if QT_VERSION >= 0x040000
	m_textBrowser->setSource(QUrl::fromLocalFile(m_filename));
#else
	m_textBrowser->setSource(m_filename);
#endif
	vlayout->addWidget(m_textBrowser);

	QHBoxLayout* hlayout = new QHBoxLayout(vlayout);
	QSpacerItem* hspacer = new QSpacerItem(16, 0, QSizePolicy::Expanding,
	                                       QSizePolicy::Minimum);
	QPushButton* backButton = new QPushButton(i18n("&Back"), this);
	QPushButton* forwardButton = new QPushButton(i18n("&Forward"), this);
	QPushButton* closeButton = new QPushButton(i18n("&Close"), this);
	if (hlayout && backButton && forwardButton && closeButton) {
		hlayout->addWidget(backButton);
		hlayout->addWidget(forwardButton);
		hlayout->addItem(hspacer);
		hlayout->addWidget(closeButton);
		closeButton->setDefault(true);
		backButton->setEnabled(false);
		forwardButton->setEnabled(false);
		connect(backButton, SIGNAL(clicked()), m_textBrowser, SLOT(backward()));
		connect(forwardButton, SIGNAL(clicked()), m_textBrowser, SLOT(forward()));
		connect(m_textBrowser, SIGNAL(backwardAvailable(bool)), backButton, SLOT(setEnabled(bool)));
		connect(m_textBrowser, SIGNAL(forwardAvailable(bool)), forwardButton, SLOT(setEnabled(bool)));
		connect(closeButton, SIGNAL(clicked()), this, SLOT(accept()));
	}
	resize(500, 500);
}

BrowserDialog::~BrowserDialog()
{}

void BrowserDialog::goToAnchor(const QString& anchor)
{
#if QT_VERSION >= 0x040000
	QUrl url = QUrl::fromLocalFile(m_filename);
	url.setFragment(anchor);
	m_textBrowser->setSource(url);
#else
	if (!anchor.isEmpty()) {
		m_textBrowser->setSource(m_filename + '#' + anchor);
	} else {
		m_textBrowser->setSource(m_filename);
	}
#endif
}

BrowserDialog* Kid3App::s_helpBrowser = 0;
#endif


MiscConfig Kid3App::s_miscCfg("General Options");
ImportConfig Kid3App::s_genCfg("General Options");
FormatConfig Kid3App::s_fnFormatCfg("FilenameFormat");
FormatConfig Kid3App::s_id3FormatCfg("Id3Format");
FreedbConfig Kid3App::s_freedbCfg("Freedb");
DiscogsConfig Kid3App::s_discogsCfg("Discogs");
MusicBrainzConfig Kid3App::s_musicBrainzCfg("MusicBrainz");

/**
 * Constructor.
 *
 * @param name name
 */
Kid3App::Kid3App() :
	m_importDialog(0), m_exportDialog(0), m_numberTracksDialog(0)
{
	m_copyTags = new StandardTags();
	initStatusBar();
	setModified(false);
	m_dirName = QString::null;
	initView();
	initActions();
	s_fnFormatCfg.setAsFilenameFormatter();

	resize(sizeHint());
#ifdef CONFIG_USE_KDE
	m_config=kapp->config();
#else
	m_config = new Kid3Settings();
	m_config->setPath("kid3.sourceforge.net", "Kid3", Kid3Settings::User);
	m_config->beginGroup("/kid3");
#endif
	readOptions();
}

/**
 * Destructor.
 */
Kid3App::~Kid3App()
{
	delete m_importDialog;
	delete m_numberTracksDialog;
#ifndef CONFIG_USE_KDE
	delete s_helpBrowser;
	s_helpBrowser = 0;
#endif
}

/**
 * Init menu and toolbar actions.
 */
void Kid3App::initActions()
{
#ifdef CONFIG_USE_KDE
	m_fileOpen = KStdAction::open(
	    this, SLOT(slotFileOpen()), actionCollection());
	m_fileOpenRecent = KStdAction::openRecent(
	    this, SLOT(slotFileOpenRecent(const KURL&)), actionCollection());
	m_fileRevert = KStdAction::revert(
	    this, SLOT(slotFileRevert()), actionCollection());
	m_fileSave = KStdAction::save(
	    this, SLOT(slotFileSave()), actionCollection());
	m_fileQuit = KStdAction::quit(
	    this, SLOT(slotFileQuit()), actionCollection());
#if KDE_VERSION < 0x30200
	m_viewToolBar = KStdAction::showToolbar(
	    this, SLOT(slotViewToolBar()), actionCollection());
	m_viewStatusBar = KStdAction::showStatusbar(
	    this, SLOT(slotViewStatusBar()), actionCollection());
	m_viewToolBar->setStatusText(i18n("Enables/disables the toolbar"));
	m_viewStatusBar->setStatusText(i18n("Enables/disables the statusbar"));
#else
	setStandardToolBarMenuEnabled(true);
	createStandardStatusBarAction();
#endif
	settingsShortcuts = KStdAction::keyBindings(
		this, SLOT(slotSettingsShortcuts()), actionCollection());
	m_settingsConfigure = KStdAction::preferences(
	    this, SLOT(slotSettingsConfigure()), actionCollection());

	m_fileOpen->setStatusText(i18n("Opens a directory"));
	m_fileOpenRecent->setStatusText(i18n("Opens a recently used directory"));
	m_fileRevert->setStatusText(
	    i18n("Reverts the changes of all or the selected files"));
	m_fileSave->setStatusText(i18n("Saves the changed files"));
	m_fileQuit->setStatusText(i18n("Quits the application"));
	settingsShortcuts->setStatusText(i18n("Configure Shortcuts"));
	m_settingsConfigure->setStatusText(i18n("Preferences dialog"));

	new KAction(i18n("&Import..."), 0, this,
		    SLOT(slotImport()), actionCollection(),
		    "import");
	new KAction(i18n("Import from &freedb.org..."), 0, this,
		    SLOT(slotImportFreedb()), actionCollection(),
		    "import_freedb");
	new KAction(i18n("Import from &Discogs..."), 0, this,
		    SLOT(slotImportDiscogs()), actionCollection(),
		    "import_discogs");
	new KAction(i18n("Import from MusicBrainz &release..."), 0, this,
		    SLOT(slotImportMusicBrainzRelease()), actionCollection(),
		    "import_musicbrainzrelease");
#ifdef HAVE_TUNEPIMP
	new KAction(i18n("Import from &MusicBrainz fingerprint..."), 0, this,
		    SLOT(slotImportMusicBrainz()), actionCollection(),
		    "import_musicbrainz");
#endif
	new KAction(i18n("&Export..."), 0, this,
		    SLOT(slotExport()), actionCollection(),
		    "export");
	new KAction(i18n("&Create Playlist"), 0, this,
		    SLOT(slotCreatePlaylist()), actionCollection(),
		    "create_playlist");
	new KAction(i18n("Apply &Filename Format"), 0, this,
		    SLOT(slotApplyFilenameFormat()), actionCollection(),
		    "apply_filename_format");
	new KAction(i18n("Apply &Tag Format"), 0, this,
		    SLOT(slotApplyId3Format()), actionCollection(),
		    "apply_id3_format");
	new KAction(i18n("&Rename Directory..."), 0, this,
		    SLOT(slotRenameDirectory()), actionCollection(),
		    "rename_directory");
	new KAction(i18n("&Number Tracks..."), 0, this,
		    SLOT(slotNumberTracks()), actionCollection(),
		    "number_tracks");
	m_settingsShowHideV1 =
		new KAction(i18n("Hide Tag &1"), 0, this,
								SLOT(slotSettingsShowHideV1()), actionCollection(),
								"hide_v1");
	m_settingsShowHideV2 =
		new KAction(i18n("Hide Tag &2"), 0, this,
								SLOT(slotSettingsShowHideV2()), actionCollection(),
								"hide_v2");

	new KAction(i18n("Select &All"), KShortcut("Alt+A"), m_view,
		    SLOT(selectAllFiles()), actionCollection(),
		    "select_all");
	new KAction(i18n("&Next File"), KShortcut("Alt+Down"), m_view,
		    SLOT(selectNextFile()), actionCollection(),
		    "next_file");
	new KAction(i18n("&Previous File"), KShortcut("Alt+Up"), m_view,
		    SLOT(selectPreviousFile()), actionCollection(),
		    "previous_file");
	new KAction(i18n("Tag 1") + ": " + i18n("From Filename"), 0, m_view, SLOT(fromFilenameV1()),
				actionCollection(), "v1_from_filename");
	new KAction(i18n("Tag 1") + ": " + i18n("From Tag 2"), 0, m_view, SLOT(fromID3V1()),
				actionCollection(), "v1_from_v2");
	new KAction(i18n("Tag 1") + ": " + i18n("Copy"), 0, m_view, SLOT(copyV1()),
				actionCollection(), "v1_copy");
	new KAction(i18n("Tag 1") + ": " + i18n("Paste"), 0, m_view, SLOT(pasteV1()),
				actionCollection(), "v1_paste");
	new KAction(i18n("Tag 1") + ": " + i18n("Remove"), 0, m_view, SLOT(removeV1()),
				actionCollection(), "v1_remove");
	new KAction(i18n("Tag 2") + ": " + i18n("From Filename"), 0, m_view, SLOT(fromFilenameV2()),
				actionCollection(), "v2_from_filename");
	new KAction(i18n("Tag 2") + ": " + i18n("From Tag 1"), 0, m_view, SLOT(fromID3V2()),
				actionCollection(), "v2_from_v1");
	new KAction(i18n("Tag 2") + ": " + i18n("Copy"), 0, m_view, SLOT(copyV2()),
				actionCollection(), "v2_copy");
	new KAction(i18n("Tag 2") + ": " + i18n("Paste"), 0, m_view, SLOT(pasteV2()),
				actionCollection(), "v2_paste");
	new KAction(i18n("Tag 2") + ": " + i18n("Remove"), 0, m_view, SLOT(removeV2()),
				actionCollection(), "v2_remove");
	new KAction(i18n("Frames:") + " " + i18n("Edit"), 0, m_view, SLOT(editFrame()),
				actionCollection(), "frames_edit");
	new KAction(i18n("Frames:") + " " + i18n("Add"), 0, m_view, SLOT(addFrame()),
				actionCollection(), "frames_add");
	new KAction(i18n("Frames:") + " " + i18n("Delete"), 0, m_view, SLOT(deleteFrame()),
				actionCollection(), "frames_delete");
	new KAction(i18n("Filename") + ": " + i18n("From Tag 1"), 0, m_view, SLOT(fnFromID3V1()),
				actionCollection(), "filename_from_v1");
	new KAction(i18n("Filename") + ": " + i18n("From Tag 2"), 0, m_view, SLOT(fnFromID3V2()),
				actionCollection(), "filename_from_v2");
	new KAction(i18n("Filename") + ": " + i18n("Focus"), 0, m_view, SLOT(setFocusFilename()),
				actionCollection(), "filename_focus");
	new KAction(i18n("Tag 1") + ": " + i18n("Focus"), 0, m_view, SLOT(setFocusV1()),
				actionCollection(), "v1_focus");
	new KAction(i18n("Tag 2") + ": " + i18n("Focus"), 0, m_view, SLOT(setFocusV2()),
				actionCollection(), "v2_focus");

	createGUI();

#else
	m_fileOpen = new QAction(this);
	if (m_fileOpen) {
		m_fileOpen->setText(i18n("Opens a directory"));
		m_fileOpen->setMenuText(i18n("&Open..."));
		m_fileOpen->setAccel(Qt::CTRL + Qt::Key_O);
		connect(m_fileOpen, SIGNAL(activated()),
			this, SLOT(slotFileOpen()));
	}
	m_fileSave = new QAction(this);
	if (m_fileSave) {
		m_fileSave->setText(i18n("Saves the changed files"));
		m_fileSave->setMenuText(i18n("&Save"));
		m_fileSave->setAccel(Qt::CTRL + Qt::Key_S);
		connect(m_fileSave, SIGNAL(activated()),
			this, SLOT(slotFileSave()));
	}
	m_fileRevert = new QAction(this);
	if (m_fileRevert) {
		m_fileRevert->setText(
		    i18n("Reverts the changes of all or the selected files"));
		m_fileRevert->setMenuText(i18n("Re&vert"));
		connect(m_fileRevert, SIGNAL(activated()),
			this, SLOT(slotFileRevert()));
	}
	m_fileImport = new QAction(this);
	if (m_fileImport) {
		m_fileImport->setText(i18n("Import from file or clipboard"));
		m_fileImport->setMenuText(i18n("&Import..."));
		connect(m_fileImport, SIGNAL(activated()),
			this, SLOT(slotImport()));
	}
	m_fileImportFreedb = new QAction(this);
	if (m_fileImportFreedb) {
		m_fileImportFreedb->setText(i18n("Import from freedb.org"));
		m_fileImportFreedb->setMenuText(i18n("Import from &freedb.org..."));
		connect(m_fileImportFreedb, SIGNAL(activated()),
			this, SLOT(slotImportFreedb()));
	}
	m_fileImportDiscogs = new QAction(this);
	if (m_fileImportDiscogs) {
		m_fileImportDiscogs->setText(i18n("Import from Discogs"));
		m_fileImportDiscogs->setMenuText(i18n("Import from &Discogs..."));
		connect(m_fileImportDiscogs, SIGNAL(activated()),
			this, SLOT(slotImportDiscogs()));
	}
	m_fileImportMusicBrainzRelease = new QAction(this);
	if (m_fileImportMusicBrainzRelease) {
		m_fileImportMusicBrainzRelease->setText(i18n("Import from MusicBrainz release"));
		m_fileImportMusicBrainzRelease->setMenuText(i18n("Import from MusicBrainz &release..."));
		connect(m_fileImportMusicBrainzRelease, SIGNAL(activated()),
			this, SLOT(slotImportMusicBrainzRelease()));
	}
#ifdef HAVE_TUNEPIMP
	m_fileImportMusicBrainz = new QAction(this);
	if (m_fileImportMusicBrainz) {
		m_fileImportMusicBrainz->setText(i18n("Import from MusicBrainz fingerprint"));
		m_fileImportMusicBrainz->setMenuText(i18n("Import from &MusicBrainz fingerprint..."));
		connect(m_fileImportMusicBrainz, SIGNAL(activated()),
			this, SLOT(slotImportMusicBrainz()));
	}
#endif
	m_fileExport = new QAction(this);
	if (m_fileExport) {
		m_fileExport->setText(i18n("Export to file or clipboard"));
		m_fileExport->setMenuText(i18n("&Export..."));
		connect(m_fileExport, SIGNAL(activated()),
			this, SLOT(slotExport()));
	}
	m_fileCreatePlaylist = new QAction(this);
	if (m_fileCreatePlaylist) {
		m_fileCreatePlaylist->setText(i18n("Create M3U Playlist"));
		m_fileCreatePlaylist->setMenuText(i18n("&Create Playlist"));
		connect(m_fileCreatePlaylist, SIGNAL(activated()),
			this, SLOT(slotCreatePlaylist()));
	}
	m_fileQuit = new QAction(this);
	if (m_fileQuit) {
		m_fileQuit->setText(i18n("Quits the application"));
		m_fileQuit->setMenuText(i18n("&Quit"));
		m_fileQuit->setAccel(Qt::CTRL + Qt::Key_Q);
		connect(m_fileQuit, SIGNAL(activated()),
			this, SLOT(slotFileQuit()));
	}
	m_helpHandbook = new QAction(this);
	if (m_helpHandbook) {
		m_helpHandbook->setText(i18n("Kid3 Handbook"));
		m_helpHandbook->setMenuText(i18n("Kid3 &Handbook"));
		connect(m_helpHandbook, SIGNAL(activated()),
			this, SLOT(slotHelpHandbook()));
	}
	m_helpAbout = new QAction(this);
	if (m_helpAbout) {
		m_helpAbout->setText(i18n("About Kid3"));
		m_helpAbout->setMenuText(i18n("&About Kid3"));
		connect(m_helpAbout, SIGNAL(activated()),
			this, SLOT(slotHelpAbout()));
	}
	m_helpAboutQt = new QAction(this);
	if (m_helpAboutQt) {
		m_helpAboutQt->setText(i18n("About Qt"));
		m_helpAboutQt->setMenuText(i18n("About &Qt"));
		connect(m_helpAboutQt, SIGNAL(activated()),
			this, SLOT(slotHelpAboutQt()));
	}
	m_toolsApplyFilenameFormat = new QAction(this);
	if (m_toolsApplyFilenameFormat) {
		m_toolsApplyFilenameFormat->setText(i18n("Apply Filename Format"));
		m_toolsApplyFilenameFormat->setMenuText(i18n("Apply &Filename Format"));
		connect(m_toolsApplyFilenameFormat, SIGNAL(activated()),
			this, SLOT(slotApplyFilenameFormat()));
	}
	m_toolsApplyId3Format = new QAction(this);
	if (m_toolsApplyId3Format) {
		m_toolsApplyId3Format->setText(i18n("Apply Tag Format"));
		m_toolsApplyId3Format->setMenuText(i18n("Apply &Tag Format"));
		connect(m_toolsApplyId3Format, SIGNAL(activated()),
			this, SLOT(slotApplyId3Format()));
	}
	m_toolsRenameDirectory = new QAction(this);
	if (m_toolsRenameDirectory) {
		m_toolsRenameDirectory->setText(i18n("Rename Directory"));
		m_toolsRenameDirectory->setMenuText(i18n("&Rename Directory..."));
		connect(m_toolsRenameDirectory, SIGNAL(activated()),
			this, SLOT(slotRenameDirectory()));
	}
	m_toolsNumberTracks = new QAction(this);
	if (m_toolsNumberTracks) {
		m_toolsNumberTracks->setText(i18n("Number Tracks"));
		m_toolsNumberTracks->setMenuText(i18n("&Number Tracks..."));
		connect(m_toolsNumberTracks, SIGNAL(activated()),
			this, SLOT(slotNumberTracks()));
	}
	m_settingsShowHideV1 = new QAction(this);
	if (m_settingsShowHideV1) {
		m_settingsShowHideV1->setText(i18n("Hide Tag 1"));
		m_settingsShowHideV1->setMenuText(i18n("Hide Tag &1"));
		connect(m_settingsShowHideV1, SIGNAL(activated()),
			this, SLOT(slotSettingsShowHideV1()));
	}
	m_settingsShowHideV2 = new QAction(this);
	if (m_settingsShowHideV2) {
		m_settingsShowHideV2->setText(i18n("Hide Tag 2"));
		m_settingsShowHideV2->setMenuText(i18n("Hide Tag &2"));
		connect(m_settingsShowHideV2, SIGNAL(activated()),
			this, SLOT(slotSettingsShowHideV2()));
	}
	m_settingsConfigure = new QAction(this);
	if (m_settingsConfigure) {
		m_settingsConfigure->setText(i18n("Configure Kid3"));
		m_settingsConfigure->setMenuText(i18n("&Configure Kid3..."));
		connect(m_settingsConfigure, SIGNAL(activated()),
			this, SLOT(slotSettingsConfigure()));
	}
	m_menubar = new QMenuBar(this);
	m_fileMenu = new Q3PopupMenu(this);
	m_toolsMenu = new Q3PopupMenu(this);
	m_settingsMenu = new Q3PopupMenu(this);
	m_helpMenu = new Q3PopupMenu(this);
	if (m_menubar && m_fileMenu && m_toolsMenu && m_settingsMenu && m_helpMenu) {
		m_fileOpen->addTo(m_fileMenu);
		m_fileMenu->insertSeparator();
		m_fileSave->addTo(m_fileMenu);
		m_fileRevert->addTo(m_fileMenu);
		m_fileMenu->insertSeparator();
		m_fileImport->addTo(m_fileMenu);
		m_fileImportFreedb->addTo(m_fileMenu);
		m_fileImportDiscogs->addTo(m_fileMenu);
		m_fileImportMusicBrainzRelease->addTo(m_fileMenu);
#ifdef HAVE_TUNEPIMP
		m_fileImportMusicBrainz->addTo(m_fileMenu);
#endif
		m_fileExport->addTo(m_fileMenu);
		m_fileCreatePlaylist->addTo(m_fileMenu);
		m_fileMenu->insertSeparator();
		m_fileQuit->addTo(m_fileMenu);
		m_menubar->insertItem((i18n("&File")), m_fileMenu);

		m_toolsApplyFilenameFormat->addTo(m_toolsMenu);
		m_toolsApplyId3Format->addTo(m_toolsMenu);
		m_toolsRenameDirectory->addTo(m_toolsMenu);
		m_toolsNumberTracks->addTo(m_toolsMenu);
		m_menubar->insertItem((i18n("&Tools")), m_toolsMenu);

		m_settingsShowHideV1->addTo(m_settingsMenu);
		m_settingsShowHideV2->addTo(m_settingsMenu);
		m_settingsMenu->insertSeparator();
		m_settingsConfigure->addTo(m_settingsMenu);
		m_menubar->insertItem(i18n("&Settings"), m_settingsMenu);

		m_helpHandbook->addTo(m_helpMenu);
		m_helpAbout->addTo(m_helpMenu);
		m_helpAboutQt->addTo(m_helpMenu);
		m_menubar->insertItem(i18n("&Help"), m_helpMenu);
	}
	setCaption("Kid3");
#endif
}

/**
 * Init status bar.
 */
void Kid3App::initStatusBar()
{
	statusBar()->message(i18n("Ready."));
}

/**
 * Init GUI.
 */
void Kid3App::initView()
{ 
	m_view = new Id3Form(this, "id3Form");
	if (m_view) {
		setCentralWidget(m_view);	
		m_view->initView();
	}
}

/**
 * Open directory.
 *
 * @param dir     directory or file path
 * @param confirm if true ask if there are unsaved changes
 */
void Kid3App::openDirectory(QString dir, bool confirm)
{
	if (confirm && !saveModified()) {
		return;
	}
	if (dir.isNull() || dir.isEmpty()) {
		return;
	}
	QFileInfo file(dir);
	if (!file.isDir()) {
		dir = file.dirPath(true);
	}

	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	slotStatusMsg(i18n("Opening directory..."));
	if (m_view->readFileList(dir)) {
		m_view->readDirectoryList(dir);
		setModified(false);
#ifdef CONFIG_USE_KDE
		KURL url;
		url.setPath(dir);
		m_fileOpenRecent->addURL(url);
		setCaption(dir, false);
#else
		setCaption(dir + " - Kid3");
#endif
		m_dirName = dir;
	}
	slotStatusMsg(i18n("Ready."));
	QApplication::restoreOverrideCursor();
}

/**
 * Save application options.
 */
void Kid3App::saveOptions()
{
#ifdef CONFIG_USE_KDE
	m_fileOpenRecent->saveEntries(m_config, "Recent Files");
#else
	s_miscCfg.m_windowWidth = size().width();
	s_miscCfg.m_windowHeight = size().height();
#endif
	m_view->saveConfig();

	s_miscCfg.writeToConfig(m_config);
	s_fnFormatCfg.writeToConfig(m_config);
	s_id3FormatCfg.writeToConfig(m_config);
	s_genCfg.writeToConfig(m_config);
	s_freedbCfg.writeToConfig(m_config);
	s_discogsCfg.writeToConfig(m_config);
#ifdef HAVE_TUNEPIMP
	s_musicBrainzCfg.writeToConfig(m_config);
#endif
}

/**
 * Load application options.
 */
void Kid3App::readOptions()
{
	s_miscCfg.readFromConfig(m_config);
	s_fnFormatCfg.readFromConfig(m_config);
	s_id3FormatCfg.readFromConfig(m_config);
	s_genCfg.readFromConfig(m_config);
	s_freedbCfg.readFromConfig(m_config);
	s_discogsCfg.readFromConfig(m_config);
#ifdef HAVE_TUNEPIMP
	s_musicBrainzCfg.readFromConfig(m_config);
#endif
	updateHideV1();
	updateHideV2();
#ifdef CONFIG_USE_KDE
	setAutoSaveSettings();
	m_fileOpenRecent->loadEntries(m_config,"Recent Files");
#if KDE_VERSION < 0x30200
	m_viewToolBar->setChecked(!toolBar("mainToolBar")->isHidden());
	m_viewStatusBar->setChecked(!statusBar()->isHidden());
#endif
#else
	if (s_miscCfg.m_windowWidth != -1 && s_miscCfg.m_windowHeight != -1) {
		resize(s_miscCfg.m_windowWidth, s_miscCfg.m_windowHeight);
	}
#endif
	m_view->readConfig();
}

#ifdef CONFIG_USE_KDE
/**
 * Saves the window properties to the session config file.
 *
 * @param cfg application configuration
 */
void Kid3App::saveProperties(KConfig* cfg)
{
	if (cfg) { // otherwise KDE 3.0 compiled program crashes with KDE 3.1
		cfg->writeEntry("dirname", m_dirName);
	}
}

/**
 * Reads the session config file and restores the application's state.
 *
 * @param cfg application configuration
 */
void Kid3App::readProperties(KConfig* cfg)
{
	openDirectory(cfg->readEntry("dirname", ""));
}

#else /* CONFIG_USE_KDE */

/**
 * Window is closed.
 *
 * @param ce close event
 */
void Kid3App::closeEvent(QCloseEvent* ce)
{
	if (queryClose()) {
		ce->accept();
	}
	else {
		ce->ignore();
	}
}
#endif /* CONFIG_USE_KDE */

/**
 * Save all changed files.
 *
 * @return true
 */
bool Kid3App::saveDirectory()
{
	QString errorFiles;
	int numFiles = 0, totalFiles = 0;
	FileListItem* mp3file = m_view->firstFile();
	// Get number of files to be saved to display correct progressbar
	while (mp3file != 0) {
		if (mp3file->getFile()->isChanged()) {
			++totalFiles;
		}
		mp3file = m_view->nextFile();
	}
	Q3ProgressBar* progress = new Q3ProgressBar();
	statusBar()->addWidget(progress, 0, true);
	progress->setTotalSteps(totalFiles);
	progress->setProgress(numFiles);
#ifdef CONFIG_USE_KDE
	kapp->processEvents();
#else
	qApp->processEvents();
#endif
	mp3file = m_view->firstFile();
	while (mp3file != 0) {
		bool renamed = false;
		if (!mp3file->getFile()->writeTags(false, &renamed, s_miscCfg.m_preserveTime)) {
			errorFiles.append(mp3file->getFile()->getFilename());
			errorFiles.append('\n');
		}
		if (renamed) {
			mp3file->updateText();
		}
		mp3file = m_view->nextFile();
		++numFiles;
		progress->setProgress(numFiles);
	}
	statusBar()->removeWidget(progress);
	delete progress;
	updateModificationState();
	if (!errorFiles.isEmpty()) {
		QMessageBox::warning(0, i18n("File Error"),
							 i18n("Error while writing file:\n") +
							 errorFiles,
							 QMessageBox::Ok, QCM_NoButton);
	}
	return true;
}

/**
 * If anything was modified, save after asking user.
 *
 * @return false if user canceled.
 */
bool Kid3App::saveModified()
{
	bool completed=true;

	if(isModified() && !m_dirName.isEmpty())
	{
		Kid3App* win=(Kid3App *) parent();
#ifdef CONFIG_USE_KDE
		const int Yes = KMessageBox::Yes;
		const int No = KMessageBox::No;
		const int Cancel = KMessageBox::Cancel;
		int want_save = KMessageBox::warningYesNoCancel(
		    win,
		    i18n("The current directory has been modified.\n"
			 "Do you want to save it?"),
		    i18n("Warning"));
#else
		const int Yes = QMessageBox::Yes;
		const int No = QMessageBox::No;
		const int Cancel = QMessageBox::Cancel;
		int want_save = QMessageBox::warning(
			win,
			i18n("Warning - Kid3"),
			i18n("The current directory has been modified.\n"
			     "Do you want to save it?"),
			QMessageBox::Yes | QMessageBox::Default,
			QMessageBox::No,
			QMessageBox::Cancel | QMessageBox::Escape);
#endif
		switch(want_save)
		{
		case Yes:
			saveDirectory();
			completed=true;
			break;

		case No:
			setModified(false);
			completed=true;
			break;

		case Cancel:
			completed=false;
			break;

		default:
			completed=false;
			break;
		}
	}

	return completed;
}

/**
 * Free allocated resources.
 * Our destructor may not be called, so cleanup is done here.
 */
void Kid3App::cleanup()
{
#ifndef CONFIG_USE_KDE
#ifdef _MSC_VER
	// A _BLOCK_TYPE_IS_VALID assertion pops up if config is deleted
	// on Windows, MSVC 2005, Qt 4.1.2
	m_config->sync();
#else
	delete m_config;
#endif
#endif
		delete m_copyTags;
#ifdef HAVE_ID3LIB
		Mp3File::staticCleanup();
#endif
#ifdef HAVE_VORBIS
		OggFile::staticCleanup();
#endif
#ifdef HAVE_FLAC
		FlacFile::staticCleanup();
#endif
#ifdef HAVE_TAGLIB
		TagLibFile::staticCleanup();
#endif
}

/**
 * Update modification state before closing.
 * Called on closeEvent() of window.
 * If anything was modified, save after asking user.
 * Save options before closing.
 * This method is called by closeEvent(), which occurs when the
 * window is closed or slotFileQuit() (Quit menu) is selected.
 *
 * @return false if user canceled,
 *         true will quit the application.
 */
bool Kid3App::queryClose()
{
	updateCurrentSelection();
	if (saveModified()) {
		saveOptions();
		cleanup();
		return true;
	}
	return false;
}

/**
 * Request new directory and open it.
 */
void Kid3App::slotFileOpen()
{
	updateCurrentSelection();
	if(saveModified()) {
		QString dir, filter, flt;
#ifdef CONFIG_USE_KDE
		flt = "*.mp3 *.ogg *.flac *.mpc *.MP3 *.OGG *.FLAC *.MPC *.Mp3 *.Ogg *.Flac *.Mpc *.mP3 *.ogG *.oGg *.oGG *.OgG *.OGg *.flaC *.flAc *.flAC *.FlaC *.FlAc *.mpC *.mPc *.mPC *.MpC *.MPc|MP3, OGG, FLAC, MPC (*.mp3, *.ogg, *.flac *.mpc)\n";
#if defined HAVE_ID3LIB || defined HAVE_TAGLIB
		flt += "*.mp3 *.MP3 *.Mp3 *.mP3|MP3 (*.mp3)\n";
#endif
#if defined HAVE_VORBIS || defined HAVE_TAGLIB
		flt += "*.ogg *.OGG *.Ogg *.ogG *.oGg *.oGG *.OgG *.OGg|OGG (*.ogg)\n";
#endif
#if defined HAVE_FLAC || defined HAVE_TAGLIB
		flt += "*.flac *.FLAC *.Flac *.flaC *.flAc *.flAC *.FlaC *.FlAc|FLAC (*.flac)\n";
#endif
#ifdef HAVE_TAGLIB
		flt += "*.mpc *.MPC *.Mpc *.mpC *.mPc *.mPC *.MpC *.MPc|MPC (*.mpc)\n";
#endif
		flt += ("*|All Files (*)");
		KFileDialog diag(
		    QString::null,
		    flt,
		    this, "filedialog", true);
		diag.setCaption(i18n("Open"));
		if (diag.exec() == QDialog::Accepted) {
			dir = diag.selectedFile();
			filter = diag.currentFilter();
		}
#else
		flt = "MP3, OGG, FLAC, MPC (*.mp3 *.ogg *.flac *.mpc *.MP3 *.OGG *.FLAC *.MPC *.Mp3 *.Ogg *.Flac *.Mpc *.mP3 *.ogG *.oGg *.oGG *.OgG *.OGg *.flaC *.flAc *.flAC *.FlaC *.FlAc *.mpC *.mPc *.mPC *.MpC *.MPc);;";
#if defined HAVE_ID3LIB || defined HAVE_TAGLIB
		flt += "MP3 (*.mp3 *.MP3 *.Mp3 *.mP3);;";
#endif
#if defined HAVE_VORBIS || defined HAVE_TAGLIB
		flt += "OGG (*.ogg *.OGG *.Ogg *.ogG *.oGg *.oGG *.OgG *.OGg);;";
#endif
#if defined HAVE_FLAC || defined HAVE_TAGLIB
		flt += "FLAC (*.flac *.FLAC *.Flac *.flaC *.flAc *.flAC *.FlaC *.FlAc);;";
#endif
#ifdef HAVE_TAGLIB
		flt += "MPC (*.mpc *.MPC *.Mpc *.mpC *.mPc *.mPC *.MpC *.MPc);;";
#endif
		flt += i18n("All Files (*)");
		dir = QFileDialog::getOpenFileName(
		    QString::null, flt,
		    this, 0, QString::null, &filter);
#endif
		if (!dir.isEmpty()) {
			int start = filter.find('('), end = filter.find(')');
			if (start != -1 && end != -1 && end > start) {
				filter = filter.mid(start + 1, end - start - 1);
			}
			s_miscCfg.m_nameFilter = filter;
			openDirectory(dir);
		}
	}
}

#ifdef CONFIG_USE_KDE
/**
 * Open recent directory.
 *
 * @param url URL of directory to open
 */
void Kid3App::slotFileOpenRecent(const KURL& url)
{
	updateCurrentSelection();
	QString dir = url.path();
	openDirectory(dir, true);
}
#else /* CONFIG_USE_KDE */
void Kid3App::slotFileOpenRecent(const KURL&) {}
#endif /* CONFIG_USE_KDE */

/**
 * Revert file modifications.
 * Acts on selected files or all files if no file is selected.
 */
void Kid3App::slotFileRevert()
{
	FileListItem* mp3file = m_view->firstFile();
	bool no_selection = m_view->numFilesSelected() == 0;
	while (mp3file != 0) {
		if (no_selection || mp3file->isInSelection()) {
			mp3file->getFile()->readTags(true);
		}
		mp3file = m_view->nextFile();
	}
	if (!no_selection) {
		StandardTags st; // empty
		m_view->setStandardTagsV1(&st);
		m_view->setStandardTagsV2(&st);
		m_view->setFilenameEditEnabled(false);
		fileSelected();
	}
	else {
		updateModificationState();
	}
}

/**
 * Save modified files.
 */
void Kid3App::slotFileSave()
{
	updateCurrentSelection();
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	slotStatusMsg(i18n("Saving directory..."));

	saveDirectory();
	slotStatusMsg(i18n("Ready."));
	QApplication::restoreOverrideCursor();
	updateGuiControls();
}

/**
 * Quit application.
 */
void Kid3App::slotFileQuit()
{
	slotStatusMsg(i18n("Exiting..."));
	close(); /* this will lead to call of closeEvent(), queryClose() */
}

#ifdef CONFIG_USE_KDE
#if KDE_VERSION < 0x30200
/**
 * Turn tool bar on or off.
 */
void Kid3App::slotViewToolBar()
{
	slotStatusMsg(i18n("Toggling toolbar..."));
	if(!m_viewToolBar->isChecked()) {
		toolBar("mainToolBar")->hide();
	}
	else {
		toolBar("mainToolBar")->show();
	}		
	slotStatusMsg(i18n("Ready."));
}

/**
 * Turn status bar on or off.
 */
void Kid3App::slotViewStatusBar()
{
	slotStatusMsg(i18n("Toggle the statusbar..."));
	if(!m_viewStatusBar->isChecked()) {
		statusBar()->hide();
	}
	else {
		statusBar()->show();
	}
	slotStatusMsg(i18n("Ready."));
}
#else

void Kid3App::slotViewToolBar() {}
void Kid3App::slotViewStatusBar() {}

#endif

/**
 * Shortcuts configuration.
 */
void Kid3App::slotSettingsShortcuts()
{
	KKeyDialog::configure(actionCollection(), this);
}

/**
 * Display help for a topic.
 *
 * @param anchor anchor in help document
 */
void Kid3App::displayHelp(const QString& anchor)
{
	kapp->invokeHelp(anchor, QString::null, "");
}

void Kid3App::slotHelpHandbook() {}
void Kid3App::slotHelpAbout() {}
void Kid3App::slotHelpAboutQt() {}

#else /* CONFIG_USE_KDE */

void Kid3App::slotViewToolBar() {}
void Kid3App::slotViewStatusBar() {}
void Kid3App::slotSettingsShortcuts() {}

/**
 * Display help for a topic.
 *
 * @param anchor anchor in help document
 */
void Kid3App::displayHelp(const QString& anchor)
{
	if (!s_helpBrowser) {
		QString caption(i18n("Kid3 Handbook"));
		s_helpBrowser =
			new BrowserDialog(NULL, caption);
	}
	if (s_helpBrowser) { 
		s_helpBrowser->goToAnchor(anchor);
		s_helpBrowser->setModal(!anchor.isEmpty());
		if (!s_helpBrowser->isShown()) {
			s_helpBrowser->show();
		}
	}
}

/**
 * Display handbook.
 */
void Kid3App::slotHelpHandbook()
{
	displayHelp();
}

/**
 * Display "About" dialog.
 */
void Kid3App::slotHelpAbout()
{
	QMessageBox::about(
		(Kid3App*)parent(), "Kid3",
		"Kid3 " VERSION
		"\n(c) 2003-2006 Urs Fleisch\nufleisch@users.sourceforge.net");
}

/**
 * Display "About Qt" dialog.
 */
void Kid3App::slotHelpAboutQt()
{
	QMessageBox::aboutQt((Kid3App*)parent(), "Kid3");
}
#endif /* CONFIG_USE_KDE */

/**
 * Change status message.
 *
 * @param text message
 */
void Kid3App::slotStatusMsg(const QString& text)
{
	statusBar()->message(text);
	// processEvents() is necessary to make the change of the status bar
	// visible when it is changed back again in the same function,
	// i.e. in the same call from the Qt main event loop.
#ifdef CONFIG_USE_KDE
	kapp->processEvents();
#else
	qApp->processEvents();
#endif
}

/**
 * Create playlist.
 */
void Kid3App::slotCreatePlaylist()
{
	FileListItem* mp3file = m_view->firstFileInDir();
	if (!(mp3file && mp3file->getFile())) return;
	QDir dir(mp3file->getFile()->getDirname());
	QString dirname = dir.absPath();
	QString fn = dirname + QDir::separator() + dir.dirName() + ".m3u";
	QFile file(fn);
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	slotStatusMsg(i18n("Creating playlist..."));
	if (file.open(QCM_WriteOnly)) {
		QTextStream stream(&file);
		while (mp3file != 0) {
			stream << mp3file->getFile()->getFilename() << "\n";
			mp3file = m_view->nextFileInDir();
		}
		file.close();
	}
	slotStatusMsg(i18n("Ready."));
	QApplication::restoreOverrideCursor();
}

/**
 * Update track data and create import dialog.
 */
void Kid3App::setupImportDialog()
{
	m_trackDataList.clear();
	FileListItem* mp3file = m_view->firstFileInDir();
	bool firstTrack = true;
	while (mp3file != 0) {
		mp3file->getFile()->readTags(false);
		if (firstTrack) {
			StandardTags st;
			mp3file->getFile()->getStandardTagsV2(&st);
			if (st.artist.isEmpty() && st.album.isEmpty()) {
				mp3file->getFile()->getStandardTagsV1(&st);
			}
			m_trackDataList.m_artist = st.artist;
			m_trackDataList.m_album = st.album;
			firstTrack = false;
		}
		m_trackDataList.push_back(ImportTrackData(mp3file->getFile()->getAbsFilename(),
                                            mp3file->getFile()->getDuration()));
		mp3file = m_view->nextFileInDir();
	}

	if (!m_importDialog) {
		QString caption(i18n("Import"));
		m_importDialog =
			new ImportDialog(NULL, caption, m_trackDataList);
	}
	if (m_importDialog) {
		m_importDialog->clear();
	}
}

/**
 * Execute the import dialog.
 */
void Kid3App::execImportDialog()
{
	if (m_importDialog &&
			m_importDialog->exec() == QDialog::Accepted) {
		slotStatusMsg(i18n("Import..."));
		ImportTrackDataVector::const_iterator it = m_trackDataList.begin();
		StandardTags st;
		bool destV1 = m_importDialog->getDestV1();
		StandardTagsFilter flt(destV1 ?
													 m_view->getFilterFromID3V1() :
													 m_view->getFilterFromID3V2());
		bool no_selection = m_view->numFilesSelected() == 0;
		FileListItem* mp3file = m_view->firstFileInDir();
		while (mp3file != 0) {
			mp3file->getFile()->readTags(false);
			if (destV1) {
				mp3file->getFile()->getStandardTagsV1(&st);
			} else {
				mp3file->getFile()->getStandardTagsV2(&st);
			}
			if (it != m_trackDataList.end()) {
				(*it).copyActiveTags(st);
				++it;
			} else {
				break;
			}
			formatStandardTagsIfEnabled(&st);
			if (destV1) {
				mp3file->getFile()->setStandardTagsV1(&st, flt);
			} else {
				mp3file->getFile()->setStandardTagsV2(&st, flt);
			}
			mp3file = m_view->nextFileInDir();
		}
		if (!no_selection) {
			StandardTags st; // empty
			m_view->setStandardTagsV1(&st);
			m_view->setStandardTagsV2(&st);
			m_view->setFilenameEditEnabled(false);
			fileSelected();
		}
		else {
			updateModificationState();
		}
		slotStatusMsg(i18n("Ready."));
		QApplication::restoreOverrideCursor();
	}
}

/**
 * Import.
 */
void Kid3App::slotImport()
{
	setupImportDialog();
	if (m_importDialog) {
		m_importDialog->setAutoStartSubDialog(ImportDialog::ASD_None);
		execImportDialog();
	}
}

/**
 * Import from freedb.org.
 */
void Kid3App::slotImportFreedb()
{
	setupImportDialog();
	if (m_importDialog) {
		m_importDialog->setAutoStartSubDialog(ImportDialog::ASD_Freedb);
		execImportDialog();
	}
}

/**
 * Import from Discogs.
 */
void Kid3App::slotImportDiscogs()
{
	setupImportDialog();
	if (m_importDialog) {
		m_importDialog->setAutoStartSubDialog(ImportDialog::ASD_Discogs);
		execImportDialog();
	}
}

/**
 * Import from MusicBrainz release database.
 */
void Kid3App::slotImportMusicBrainzRelease()
{
	setupImportDialog();
	if (m_importDialog) {
		m_importDialog->setAutoStartSubDialog(ImportDialog::ASD_MusicBrainzRelease);
		execImportDialog();
	}
}

/**
 * Import from MusicBrainz.
 */
void Kid3App::slotImportMusicBrainz()
{
#ifdef HAVE_TUNEPIMP
	setupImportDialog();
	if (m_importDialog) {
		m_importDialog->setAutoStartSubDialog(ImportDialog::ASD_MusicBrainz);
		execImportDialog();
	}
#endif
}

/**
 * Set data to be exported.
 *
 * @param src ExportDialog::SrcV1 to export ID3v1,
 *            ExportDialog::SrcV2 to export ID3v2
 */
void Kid3App::setExportData(int src)
{
	if (m_exportDialog) {
		ImportTrackDataVector trackDataVector;
		FileListItem* mp3file = m_view->firstFileInDir();
		bool firstTrack = true;
		while (mp3file != 0) {
			mp3file->getFile()->readTags(false);
			ImportTrackData trackData(mp3file->getFile()->getAbsFilename(),
																mp3file->getFile()->getDuration());
			if (src == ExportDialog::SrcV1) {
				mp3file->getFile()->getStandardTagsV1(&trackData);
			} else {
				mp3file->getFile()->getStandardTagsV2(&trackData);
			}
			if (firstTrack) {
				trackDataVector.m_artist = trackData.artist;
				trackDataVector.m_album = trackData.album;
				firstTrack = false;
			}
			trackDataVector.push_back(trackData);
			mp3file = m_view->nextFileInDir();
		}
		m_exportDialog->setExportData(trackDataVector);
	}
}

/**
 * Export.
 */
void Kid3App::slotExport()
{
	m_exportDialog = new ExportDialog(0);
	if (m_exportDialog) {
		m_exportDialog->readConfig();
		setExportData(s_genCfg.m_exportSrcV1 ?
									ExportDialog::SrcV1 : ExportDialog::SrcV2);
		connect(m_exportDialog, SIGNAL(exportDataRequested(int)),
						this, SLOT(setExportData(int)));
		m_exportDialog->exec();
		delete m_exportDialog;
		m_exportDialog = 0;
	}
}

/**
 * Show or hide the ID3V1.1 controls according to the settings and
 * set the menu entries appropriately.
 */
void Kid3App::updateHideV1()
{
	m_view->hideV1(s_miscCfg.m_hideV1);
	if (s_miscCfg.m_hideV1) {
#ifdef CONFIG_USE_KDE
		m_settingsShowHideV1->setText(i18n("Show Tag &1"));
#else
		m_settingsShowHideV1->setText(i18n("Show Tag 1"));
		m_settingsShowHideV1->setMenuText(i18n("Show Tag &1"));
#endif

	} else {
#ifdef CONFIG_USE_KDE
		m_settingsShowHideV1->setText(i18n("Hide Tag &1"));
#else
		m_settingsShowHideV1->setText(i18n("Hide Tag 1"));
		m_settingsShowHideV1->setMenuText(i18n("Hide Tag &1"));
#endif
	}
#if QT_VERSION >= 0x040000
	m_view->adjustRightHalfBoxSize();
#endif
}

/**
 * Show or hide the ID3V2.3 controls according to the settings and
 * set the menu entries appropriately.
 */
void Kid3App::updateHideV2()
{
	m_view->hideV2(s_miscCfg.m_hideV2);
	if (s_miscCfg.m_hideV2) {
#ifdef CONFIG_USE_KDE
		m_settingsShowHideV2->setText(i18n("Show Tag &2"));
#else
		m_settingsShowHideV2->setText(i18n("Show Tag 2"));
		m_settingsShowHideV2->setMenuText(i18n("Show Tag &2"));
#endif

	} else {
#ifdef CONFIG_USE_KDE
		m_settingsShowHideV2->setText(i18n("Hide Tag &2"));
#else
		m_settingsShowHideV2->setText(i18n("Hide Tag 2"));
		m_settingsShowHideV2->setMenuText(i18n("Hide Tag &2"));
#endif
	}
#if QT_VERSION >= 0x040000
	m_view->adjustRightHalfBoxSize();
#endif
}

/**
 * Show or hide ID3v1.1 controls.
 */
void Kid3App::slotSettingsShowHideV1()
{
	s_miscCfg.m_hideV1 = !s_miscCfg.m_hideV1;
	updateHideV1();
}

/**
 * Show or hide ID3v2.3 controls.
 */
void Kid3App::slotSettingsShowHideV2()
{
	s_miscCfg.m_hideV2 = !s_miscCfg.m_hideV2;
	updateHideV2();
}

/**
 * Preferences.
 */
void Kid3App::slotSettingsConfigure()
{
	QString caption(i18n("Configure - Kid3"));
#ifdef KID3_USE_KCONFIGDIALOG
	KConfigSkeleton* configSkeleton = new KConfigSkeleton;
	ConfigDialog* dialog =
		new ConfigDialog(NULL, caption, configSkeleton);
#else
	ConfigDialog* dialog =
		new ConfigDialog(NULL, caption);
#endif
	if (dialog) {
		dialog->setConfig(&s_fnFormatCfg, &s_id3FormatCfg, &s_miscCfg);
		if (dialog->exec() == QDialog::Accepted) {
			dialog->getConfig(&s_fnFormatCfg, &s_id3FormatCfg, &s_miscCfg);
			s_fnFormatCfg.writeToConfig(m_config);
			s_id3FormatCfg.writeToConfig(m_config);
			s_miscCfg.writeToConfig(m_config);
#ifdef CONFIG_USE_KDE
			m_config->sync();
#endif
		}
	}
#ifdef KID3_USE_KCONFIGDIALOG
	delete configSkeleton;
#endif
}

/**
 * Apply filename format.
 */
void Kid3App::slotApplyFilenameFormat()
{
	StandardTags st;
	if (m_view->numFilesSelected() == 1) {
		updateCurrentSelection();
	}
	FileListItem* mp3file = m_view->firstFile();
	bool no_selection = m_view->numFilesSelected() == 0;
	while (mp3file != 0) {
		if (no_selection || mp3file->isInSelection()) {
			mp3file->getFile()->readTags(false);
			QString str;
			str = mp3file->getFile()->getFilename();
			s_fnFormatCfg.formatString(str);
			mp3file->getFile()->setFilename(str);
		}
		mp3file = m_view->nextFile();
	}
	updateGuiControls();
}

/**
 * Apply ID3 format.
 */
void Kid3App::slotApplyId3Format()
{
	StandardTags st;
	if (m_view->numFilesSelected() == 1) {
		updateCurrentSelection();
	}
	StandardTagsFilter fltV1(m_view->getFilterFromID3V1());
	StandardTagsFilter fltV2(m_view->getFilterFromID3V2());
	FileListItem* mp3file = m_view->firstFile();
	bool no_selection = m_view->numFilesSelected() == 0;
	while (mp3file != 0) {
		if (no_selection || mp3file->isInSelection()) {
			mp3file->getFile()->readTags(false);
			mp3file->getFile()->getStandardTagsV1(&st);
			s_id3FormatCfg.formatStandardTags(st);
			mp3file->getFile()->setStandardTagsV1(&st, fltV1);
			mp3file->getFile()->getStandardTagsV2(&st);
			s_id3FormatCfg.formatStandardTags(st);
			mp3file->getFile()->setStandardTagsV2(&st, fltV2);
		}
		mp3file = m_view->nextFile();
	}
	updateGuiControls();
}

/**
 * Rename directory.
 */
void Kid3App::slotRenameDirectory()
{
	if (saveModified() && m_view->firstFileInDir()) {
		QString caption(i18n("Rename Directory"));
		RenDirDialog* dialog =
			new RenDirDialog(NULL, caption, m_view->firstFileInDir()->getFile());
		if (dialog) {
			if (dialog->exec() == QDialog::Accepted) {
				FileListItem* mp3file = m_view->firstFileInDir();
				QString errorMsg;
				bool again = false;
				while (mp3file &&
					   dialog->performAction(mp3file->getFile(), again, &errorMsg)) {
					mp3file = m_view->nextFileInDir();
				}
				openDirectory(dialog->getNewDirname());
				if (again) {
					mp3file = m_view->firstFileInDir();
					while (mp3file &&
						   dialog->performAction(mp3file->getFile(), again, &errorMsg)) {
						mp3file = m_view->nextFileInDir();
					}
					openDirectory(dialog->getNewDirname());
				}
				if (!errorMsg.isEmpty()) {
					QMessageBox::warning(0, i18n("File Error"),
										 i18n("Error while renaming:\n") +
										 errorMsg,
										 QMessageBox::Ok, QCM_NoButton);
				}
			}
		}
	}
}

/**
 * Number tracks.
 */
void Kid3App::slotNumberTracks()
{
	if (!m_numberTracksDialog) {
		m_numberTracksDialog = new NumberTracksDialog(0);
	}
	if (m_numberTracksDialog) {
		if (m_numberTracksDialog->exec() == QDialog::Accepted) {
			int nr = m_numberTracksDialog->getStartNumber();
			bool destV1 = m_numberTracksDialog->getDestV1();

			updateCurrentSelection();
			FileListItem* mp3file = m_view->firstFileInDir();
			bool no_selection = m_view->numFilesSelected() == 0;
			while (mp3file != 0) {
				if (no_selection || mp3file->isInSelection()) {
					mp3file->getFile()->readTags(false);
					if (destV1) {
						int oldnr = mp3file->getFile()->getTrackNumV1();
						if (nr != oldnr) {
							mp3file->getFile()->setTrackNumV1(nr);
						}
					} else {
						int oldnr = mp3file->getFile()->getTrackNumV2();
						if (nr != oldnr) {
							mp3file->getFile()->setTrackNumV2(nr);
						}
					}
					++nr;
				}
				mp3file = m_view->nextFileInDir();
			}
			updateGuiControls();
		}
	}
}

/**
 * Open directory on drop.
 *
 * @param txt URL of directory or file in directory
 */
void Kid3App::openDrop(QString txt)
{
	int lfPos = txt.find('\n');
	if (lfPos > 0 && lfPos < (int)txt.length() - 1) {
		txt.truncate(lfPos + 1);
	}
	QUrl url(txt);
	if (url.hasPath()) {
		QString dir = url.path().stripWhiteSpace();
#if defined _WIN32 || defined WIN32
		// There seems to be problems with filenames on Win32,
		// so correct
		if (dir[0] == '/' && dir[1] == '/' && dir[3] == '|') {
			dir[3] = ':';
			dir.remove(0, 2);
		} else if (dir[0] == '/' && dir[2] == ':') {
			dir.remove(0, 1);
		}
#endif
		updateCurrentSelection();
		openDirectory(dir, true);
	}
}

/**
 * Set tags in file to tags in GUI controls.
 *
 * @param mp3file file
 */
void Kid3App::updateTags(TaggedFile* mp3file)
{
	StandardTags st;
	StandardTagsFilter flt;
	flt.setAllTrue();
	m_view->getStandardTagsV1(&st);
	mp3file->setStandardTagsV1(&st, flt);
	m_view->getStandardTagsV2(&st);
	mp3file->setStandardTagsV2(&st, flt);
	if (m_view->isFilenameEditEnabled()) {
		mp3file->setFilename(m_view->getFilename());
	}
}

/**
 * Update modification state, caption and listbox entries.
 */
void Kid3App::updateModificationState()
{
	setModified(m_view->updateModificationState());
#ifdef CONFIG_USE_KDE
	setCaption(m_dirName, isModified());
#else
	QString cap(m_dirName);
	if (isModified()) {
		cap += i18n(" [modified]");
	}
	if (!cap.isEmpty()) {
		cap += " - ";
	}
	cap += "Kid3";
	setCaption(cap);
#endif
}

/**
 * Update files of current selection.
 */
void Kid3App::updateCurrentSelection()
{
	FileListItem* mp3file = m_view->firstFile();
	while (mp3file != 0) {
		if (mp3file->isInSelection()) {
			updateTags(mp3file->getFile());
		}
		mp3file = m_view->nextFile();
	}
	updateModificationState();
}

/**
 * Update GUI controls from the tags in the files.
 * The new selection is stored and the GUI controls and frame list
 * updated accordingly (filtered for multiple selection).
 */
void Kid3App::updateGuiControls()
{
	StandardTags tags_v1, tags_v2;
	FileListItem* mp3file = m_view->firstFile();
	FileListItem* singleItem = 0;
	TaggedFile* single_v2_file = 0;
	TaggedFile* firstMp3File = 0;
	int num_files_selected = 0;
	bool tagV1Supported = false;

	while (mp3file != 0) {
		if (mp3file->isSelected()) {
			StandardTags filetags;
			mp3file->setInSelection(true);
			mp3file->getFile()->readTags(false);
			mp3file->getFile()->getStandardTagsV1(&filetags);
			if (num_files_selected == 0) {
				tags_v1 = filetags;
			}
			else {
				tags_v1.filterDifferent(filetags);
			}
			mp3file->getFile()->getStandardTagsV2(&filetags);
			if (num_files_selected == 0) {
				tags_v2 = filetags;
				single_v2_file = mp3file->getFile();
				singleItem = mp3file;
				firstMp3File = mp3file->getFile();
			}
			else {
				tags_v2.filterDifferent(filetags);
				single_v2_file = 0;
				singleItem = 0;
			}
			++num_files_selected;

			if (mp3file->getFile()->isTagV1Supported()) {
				tagV1Supported = true;
			}
		}
		else {
			mp3file->setInSelection(false);
		}
		mp3file = m_view->nextFile();
	}

#if defined HAVE_ID3LIB && defined HAVE_TAGLIB
	if (single_v2_file) {
		if (dynamic_cast<Mp3File*>(single_v2_file) != 0 &&
				!single_v2_file->isChanged() &&
				single_v2_file->isTagInformationRead() && single_v2_file->hasTagV2() &&
				single_v2_file->getTagFormatV2() == QString::null) {
			TagLibFile* tagLibFile;
			if (singleItem &&
					(tagLibFile = new TagLibFile(
						single_v2_file->getDirInfo(),
						single_v2_file->getFilename())) != 0) {
				singleItem->setFile(tagLibFile);
				single_v2_file = tagLibFile;
				single_v2_file->readTags(false);
				single_v2_file->getStandardTagsV1(&tags_v1);
				single_v2_file->getStandardTagsV2(&tags_v2);
			}
		}
	}
#endif

	m_view->setStandardTagsV1(&tags_v1);
	m_view->setStandardTagsV2(&tags_v2);
	m_view->setAllCheckBoxes(num_files_selected == 1);
	updateModificationState();
	if (single_v2_file) {
		FrameList* framelist = single_v2_file->getFrameList();
		if (framelist) {
			framelist->setTags(single_v2_file);
		}
		m_view->setFilenameEditEnabled(true);
		m_view->setFilename(single_v2_file->getFilename());
		m_view->setDetailInfo(single_v2_file->getDetailInfo());
		m_view->setTagFormatV1(single_v2_file->getTagFormatV1());
		m_view->setTagFormatV2(single_v2_file->getTagFormatV2());
	}
	else {
		FrameList* framelist;
		if (firstMp3File && (framelist = firstMp3File->getFrameList()) != 0) {
			framelist->clear();
		}
		m_view->setFilenameEditEnabled(false);
		m_view->setDetailInfo("");
		m_view->setTagFormatV1(QString::null);
		m_view->setTagFormatV2(QString::null);
	}

	if (num_files_selected == 0) {
		tagV1Supported = true;
	}
	m_view->enableControlsV1(tagV1Supported);
}

/**
 * Process change of selection.
 * The files of the current selection are updated.
 * The new selection is stored and the GUI controls and frame list
 * updated accordingly (filtered for multiple selection).
 */
void Kid3App::fileSelected()
{
	updateCurrentSelection();
	updateGuiControls();
}

/**
 * Copy a set of standard tags into copy buffer.
 *
 * @param st tags to copy
 */
void Kid3App::copyTags(const StandardTags* st)
{
	*m_copyTags = *st;
}

/**
 * Paste from copy buffer to standard tags.
 *
 * @param st tags to fill from data in copy buffer.
 */
void Kid3App::pasteTags(StandardTags* st)
{
	if (!m_copyTags->title.isNull())
		st->title = m_copyTags->title;
	if (!m_copyTags->artist.isNull())
		st->artist = m_copyTags->artist;
	if (!m_copyTags->album.isNull())
		st->album = m_copyTags->album;
	if (!m_copyTags->comment.isNull())
		st->comment = m_copyTags->comment;
	if (m_copyTags->year >= 0)
		st->year = m_copyTags->year;
	if (m_copyTags->track >= 0)
		st->track = m_copyTags->track;
	if (m_copyTags->genre >= 0) {
		st->genre = m_copyTags->genre;
		st->genreStr = m_copyTags->genreStr;
	}
}

/**
 * Paste from copy buffer to ID3v1 tags.
 */
void Kid3App::pasteTagsV1()
{
	updateCurrentSelection();
	StandardTags st;
	StandardTagsFilter flt(m_view->getFilterFromID3V1());
	FileListItem* mp3file = m_view->firstFile();
	while (mp3file != 0) {
		if (mp3file->isInSelection()) {
			mp3file->getFile()->getStandardTagsV1(&st);
			pasteTags(&st);
			formatStandardTagsIfEnabled(&st);
			mp3file->getFile()->setStandardTagsV1(&st, flt);
		}
		mp3file = m_view->nextFile();
	}
	// update controls with filtered data
	updateGuiControls();
}

/**
 * Paste from copy buffer to ID3v2 tags.
 */
void Kid3App::pasteTagsV2()
{
	updateCurrentSelection();
	StandardTags st;
	StandardTagsFilter flt(m_view->getFilterFromID3V2());
	FileListItem* mp3file = m_view->firstFile();
	while (mp3file != 0) {
		if (mp3file->isInSelection()) {
			mp3file->getFile()->getStandardTagsV2(&st);
			pasteTags(&st);
			formatStandardTagsIfEnabled(&st);
			mp3file->getFile()->setStandardTagsV2(&st, flt);
		}
		mp3file = m_view->nextFile();
	}
	// update controls with filtered data
	updateGuiControls();
}

/**
 * Set ID3v1 tags according to filename.
 * If a single file is selected the tags in the GUI controls
 * are set, else the tags in the multiple selected files.
 */
void Kid3App::getTagsFromFilenameV1()
{
	updateCurrentSelection();
	StandardTags st;
	FileListItem* mp3file = m_view->firstFile();
	bool multiselect = m_view->numFilesSelected() > 1;
	StandardTagsFilter flt(m_view->getFilterFromID3V1());
	while (mp3file != 0) {
		if (mp3file->isInSelection()) {
			if (!multiselect && m_view->isFilenameEditEnabled()) {
				mp3file->getFile()->setFilename(
					m_view->getFilename());
			}
			mp3file->getFile()->getStandardTagsV1(&st);
			mp3file->getFile()->getTagsFromFilename(&st, m_view->getFilenameFormat());
			formatStandardTagsIfEnabled(&st);
			mp3file->getFile()->setStandardTagsV1(&st, flt);
		}
		mp3file = m_view->nextFile();
	}
	// update controls with filtered data
	updateGuiControls();
}

/**
 * Set ID3v2 tags according to filename.
 * If a single file is selected the tags in the GUI controls
 * are set, else the tags in the multiple selected files.
 */
void Kid3App::getTagsFromFilenameV2()
{
	updateCurrentSelection();
	StandardTags st;
	FileListItem* mp3file = m_view->firstFile();
	bool multiselect = m_view->numFilesSelected() > 1;
	StandardTagsFilter flt(m_view->getFilterFromID3V2());
	while (mp3file != 0) {
		if (mp3file->isInSelection()) {
			if (!multiselect && m_view->isFilenameEditEnabled()) {
				mp3file->getFile()->setFilename(
					m_view->getFilename());
			}
			mp3file->getFile()->getStandardTagsV2(&st);
			mp3file->getFile()->getTagsFromFilename(&st, m_view->getFilenameFormat());
			formatStandardTagsIfEnabled(&st);
			mp3file->getFile()->setStandardTagsV2(&st, flt);
		}
		mp3file = m_view->nextFile();
	}
	// update controls with filtered data
	updateGuiControls();
}

/**
 * Set filename according to tags.
 * If a single file is selected the tags in the GUI controls
 * are used, else the tags in the multiple selected files.
 *
 * @param tag_version 1=ID3v1, 2=ID3v2
 */
void Kid3App::getFilenameFromTags(int tag_version)
{
	updateCurrentSelection();
	StandardTags st;
	FileListItem* mp3file = m_view->firstFile();
	bool multiselect = m_view->numFilesSelected() > 1;
	while (mp3file != 0) {
		if (mp3file->isInSelection()) {
			if (tag_version == 2) {
				mp3file->getFile()->getStandardTagsV2(&st);
			}
			else {
				mp3file->getFile()->getStandardTagsV1(&st);
			}
			mp3file->getFile()->getFilenameFromTags(
				&st, m_view->getFilenameFormat());
			formatFileNameIfEnabled(mp3file->getFile());
			if (!multiselect) {
				m_view->setFilename(
					mp3file->getFile()->getFilename());
			}
		}
		mp3file = m_view->nextFile();
	}
	// update controls with filtered data
	updateGuiControls();
}

/**
 * Copy ID3v1 tags to ID3v2 tags of selected files.
 */
void Kid3App::copyV1ToV2()
{
	updateCurrentSelection();
	StandardTags st;
	StandardTagsFilter flt(m_view->getFilterFromID3V2());
	FileListItem* mp3file = m_view->firstFile();
	while (mp3file != 0) {
		if (mp3file->isInSelection()) {
			mp3file->getFile()->getStandardTagsV1(&st);
			formatStandardTagsIfEnabled(&st);
			mp3file->getFile()->setStandardTagsV2(&st, flt);
		}
		mp3file = m_view->nextFile();
	}
	// update controls with filtered data
	updateGuiControls();
}

/**
 * Copy ID3v2 tags to ID3v1 tags of selected files.
 */
void Kid3App::copyV2ToV1()
{
	updateCurrentSelection();
	StandardTags st;
	StandardTagsFilter flt(m_view->getFilterFromID3V1());
	FileListItem* mp3file = m_view->firstFile();
	while (mp3file != 0) {
		if (mp3file->isInSelection()) {
			mp3file->getFile()->getStandardTagsV2(&st);
			formatStandardTagsIfEnabled(&st);
			mp3file->getFile()->setStandardTagsV1(&st, flt);
		}
		mp3file = m_view->nextFile();
	}
	// update controls with filtered data
	updateGuiControls();
}

/**
 * Remove ID3v1 tags in selected files.
 */
void Kid3App::removeTagsV1()
{
	updateCurrentSelection();
	StandardTagsFilter flt(m_view->getFilterFromID3V1());
	FileListItem* mp3file = m_view->firstFile();
	while (mp3file != 0) {
		if (mp3file->isInSelection()) {
			mp3file->getFile()->removeTagsV1(flt);
		}
		mp3file = m_view->nextFile();
	}
	updateGuiControls();
}

/**
 * Remove ID3v2 tags in selected files.
 */
void Kid3App::removeTagsV2()
{
	updateCurrentSelection();
	StandardTagsFilter flt(m_view->getFilterFromID3V2());
	FileListItem* mp3file = m_view->firstFile();
	while (mp3file != 0) {
		if (mp3file->isInSelection()) {
			mp3file->getFile()->removeTagsV2(flt);
		}
		mp3file = m_view->nextFile();
	}
	updateGuiControls();
}

/**
 * Update ID3v2 tags in GUI controls from file displayed in frame list.
 *
 * @param taggedFile the selected file
 */
void Kid3App::updateAfterFrameModification(TaggedFile* taggedFile)
{
	if (taggedFile) {
		StandardTags st;
		taggedFile->getStandardTagsV2(&st);
		m_view->setStandardTagsV2(&st);
		updateModificationState();
	}
}

/**
 * Get the selected file together with its frame list.
 * If multiple files are selected, 0 is returned for both parameters.
 *
 * @param taggedFile the file is returned here,
 *                   0 if not exactly one file is selected
 * @param framelist  the frame list is returned here,
 *                   0 if not exactly one file is selected
 */
void Kid3App::getSelectedFileWithFrameList(
	TaggedFile*& taggedFile, FrameList*& framelist)
{
	taggedFile = 0;
	framelist = 0;
	if (m_view->numFilesSelected() != 1) {
		return;
	}
	FileListItem* mp3file = m_view->firstFile();
	while (mp3file != 0) {
		if (mp3file->isInSelection()) {
			taggedFile = mp3file->getFile();
			framelist = mp3file->getFile()->getFrameList();
			return;
		}
		mp3file = m_view->nextFile();
	}
}

/**
 * Edit selected frame.
 */
void Kid3App::editFrame()
{
	FrameList* framelist;
	TaggedFile* taggedFile;
	updateCurrentSelection();
	getSelectedFileWithFrameList(taggedFile, framelist);
	if (taggedFile && framelist && framelist->editFrame()) {
		updateAfterFrameModification(taggedFile);
	}
}

/**
 * Delete selected frame.
 */
void Kid3App::deleteFrame()
{
	FrameList* framelist;
	TaggedFile* taggedFile;
	updateCurrentSelection();
	getSelectedFileWithFrameList(taggedFile, framelist);
	if (taggedFile && framelist && framelist->deleteFrame()) {
		updateAfterFrameModification(taggedFile);
	}
}

/**
 * Select a frame type and add such a frame to frame list.
 */
void Kid3App::addFrame()
{
	FrameList* framelist;
	TaggedFile* taggedFile;
	int id;
	updateCurrentSelection();
	getSelectedFileWithFrameList(taggedFile, framelist);
	if (taggedFile && framelist &&
			(id = framelist->selectFrameId()) != -1 &&
			framelist->addFrame(id, true)) {
		updateAfterFrameModification(taggedFile);
	} else if (!taggedFile && !framelist) {
		// multiple files selected
		FileListItem* mp3file = m_view->firstFile();
		bool firstFile = true;
		while (mp3file != 0) {
			if (mp3file->isInSelection()) {
				if (firstFile) {
					firstFile = false;
					taggedFile = mp3file->getFile();
					framelist = mp3file->getFile()->getFrameList();
					framelist->setTags(taggedFile);
					if ((id = framelist->selectFrameId()) != -1 &&
							framelist->addFrame(id, true)) {
						framelist->copyFrame();
					} else {
						break;
					}
				} else {
					if (mp3file->getFile()->getFrameList() == framelist) {
						framelist->setTags(mp3file->getFile());
						framelist->pasteFrame();
					}
				}
			}
			mp3file = m_view->nextFile();
		}
		updateModificationState();
	}
}

/**
 * Format a filename if format while editing is switched on.
 *
 * @param taggedFile file to modify
 */
void Kid3App::formatFileNameIfEnabled(TaggedFile* taggedFile) const
{
	if (s_fnFormatCfg.m_formatWhileEditing) {
		QString fn(taggedFile->getFilename());
		s_fnFormatCfg.formatString(fn);
		taggedFile->setFilename(fn);
	}
}

/**
 * Format tags if format while editing is switched on.
 *
 * @param st standard tags
 */
void Kid3App::formatStandardTagsIfEnabled(StandardTags* st) const
{
	if (s_id3FormatCfg.m_formatWhileEditing) {
		s_id3FormatCfg.formatStandardTags(*st);
	}
}
