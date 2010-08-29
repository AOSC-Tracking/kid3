/**
 * \file id3form.cpp
 * GUI for kid3, originally generated by QT Designer.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 8 Apr 2003
 *
 * Copyright (C) 2003-2009  Urs Fleisch
 *
 * This file is part of Kid3.
 *
 * Kid3 is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Kid3 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qsplitter.h>
#include <qdir.h>
#include <qframe.h>

#if QT_VERSION >= 0x040000
#include <QListWidget>
#include <QVBoxLayout>
#include <QDropEvent>
#include <QDragEnterEvent>
#include <QLabel>
#include <QGridLayout>
#include <QGroupBox>
#include <QScrollArea>
#include <QUrl>
#include <QApplication>
#else
#include <qdragobject.h>
#include <qlistbox.h>
#include <qgroupbox.h>
#include <qvbox.h>
#endif

/** Shortcut for pointer to parent (application) widget. */
#define theApp ((Kid3App *)parentWidget())

#include "filelist.h"
#include "framelist.h"
#include "frametable.h"
#include "genres.h"
#include "kid3.h"
#include "miscconfig.h"
#include "formatconfig.h"
#include "id3form.h"

#if QT_VERSION < 0x040000
/**
 * A QScrollView which returns the sizeHint of its child.
 * This is necessary to get a reasonable default size of the window.
 */
class Kid3ScrollView: public QScrollView {
public:
	/**
	 * Constructor.
	 * @param parent parent widget
	 * @param name   name
	 * @param f      window flags
	 */
	Kid3ScrollView(QWidget* parent = 0, const char* name = 0, Qt::WFlags f = 0);
	/**
	 * Constructor.
	 * @param parent  parent widget
	 * @param _client client widget
	 */
	Kid3ScrollView(QWidget* parent, QWidget *_client);
	/**
	 * Get size hint.
	 * @return size hint.
	 */
	virtual QSize sizeHint() const;
	/**
	 * Add child widget.
	 * @param child child widget
	 * @param x     x-coordinate
	 * @param y     y-coordinate
	 */
	virtual void addChild(QWidget* child, int x = 0, int y = 0);
private:
	QWidget* client;
};

/**
 * Constructor.
 *
 * @param parent parent widget
 * @see QScrollView
 */
Kid3ScrollView::Kid3ScrollView(QWidget* parent, const char* name, Qt::WFlags f)
	: QScrollView(parent, name, f), client(0) {}

/**
 * Returns the recommended size for the widget by using the sizeHint of
 * the child.
 *
 * @return recommended size.
 */
QSize Kid3ScrollView::sizeHint() const
{
	return client ? client->sizeHint() : QScrollView::sizeHint();
}

/**
 * Add a single widget to the ScrollView.
 * The widget's parent should be the ScrollView's viewport.
 *
 * @param child child widget
 */
void Kid3ScrollView::addChild(QWidget* child, int x, int y)
{
	client = child;
	QScrollView::addChild(child, x, y);
}
#endif

/** 
 * Constructs an Id3Form as a child of 'parent', with the 
 * name 'name' and widget flags set to 'f'.
 * @param parent parent widget
 */
Id3Form::Id3Form(QWidget* parent)
	: QSplitter(parent)
{
#if QT_VERSION >= 0x040200
	const int margin = 6;
	const int spacing = 2;
#elif QT_VERSION >= 0x040000
	const int margin = 12;
	const int spacing = 2;
#else
	const int margin = 16;
	const int spacing = 6;
#endif

	setAcceptDrops(true);
	QCM_setWindowTitle(i18n("Kid3"));

	m_vSplitter = new QSplitter(Qt::Vertical, this);
	m_fileListBox = new FileList(m_vSplitter, theApp);
	m_dirListBox = new DirList(m_vSplitter);

#if QT_VERSION >= 0x040000
	m_rightHalfVBox = new QWidget;
	QScrollArea* scrollView = new QScrollArea(this);
	scrollView->setWidget(m_rightHalfVBox);
	scrollView->setWidgetResizable(true);
	QVBoxLayout* rightHalfLayout = new QVBoxLayout(m_rightHalfVBox);
	rightHalfLayout->setSpacing(2);
	rightHalfLayout->setMargin(2);

	QGroupBox* filenameGroupBox =
		new QGroupBox(i18n("File&name"), m_rightHalfVBox);
	filenameGroupBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	rightHalfLayout->addWidget(filenameGroupBox);
	QGridLayout* filenameGroupBoxLayout = new QGridLayout(filenameGroupBox);
	filenameGroupBoxLayout->setMargin(margin);
	filenameGroupBoxLayout->setSpacing(spacing);

	m_nameLabel = new QLabel(i18n("Name:"), filenameGroupBox);
	filenameGroupBoxLayout->addWidget(m_nameLabel, 0, 0);

	m_nameLineEdit = new QLineEdit(filenameGroupBox);
	filenameGroupBoxLayout->addWidget(m_nameLineEdit, 0, 1, 1, 2);

	QLabel* formatLabel = new QLabel(i18n("Format:") + QChar(0x2191),
	                                 filenameGroupBox);
	filenameGroupBoxLayout->addWidget(formatLabel, 1, 0);

	m_formatComboBox = new QComboBox(filenameGroupBox);
	m_formatComboBox->setEditable(true);
	m_formatComboBox->setToolTip(FrameFormatReplacer::getToolTip());
	filenameGroupBoxLayout->addWidget(m_formatComboBox, 1, 1);

	m_fnV1Button = new QPushButton(i18n("From Tag 1"), filenameGroupBox);
	filenameGroupBoxLayout->addWidget(m_fnV1Button, 1, 2);

	QLabel* formatFromFilenameLabel = new QLabel(i18n("Format:") + QChar(0x2193),
	                                             filenameGroupBox);
	filenameGroupBoxLayout->addWidget(formatFromFilenameLabel, 2, 0);

	m_formatFromFilenameComboBox = new QComboBox(filenameGroupBox);
	m_formatFromFilenameComboBox->setEditable(true);
	m_formatFromFilenameComboBox->setToolTip(FrameFormatReplacer::getToolTip());
	filenameGroupBoxLayout->addWidget(m_formatFromFilenameComboBox, 2, 1);

	QPushButton* fnV2Button =
		new QPushButton(i18n("From Tag 2"), filenameGroupBox);
	filenameGroupBoxLayout->addWidget(fnV2Button, 2, 2);

	QLabel* infoLabel = new QLabel(i18n("Info:"), filenameGroupBox);
	filenameGroupBoxLayout->addWidget(infoLabel, 3, 0);

	m_detailsLabel = new QLabel(filenameGroupBox);
	filenameGroupBoxLayout->addWidget(m_detailsLabel, 3, 1, 1, 2);

	m_idV1GroupBox = new QGroupBox(i18n("Tag &1"), m_rightHalfVBox);
	m_idV1GroupBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	rightHalfLayout->addWidget(m_idV1GroupBox, 100);

	QHBoxLayout* idV1HBoxLayout = new QHBoxLayout(m_idV1GroupBox);
	idV1HBoxLayout->setMargin(margin);
	idV1HBoxLayout->setSpacing(spacing);
	m_framesV1Table = new FrameTable(m_idV1GroupBox, true);
	idV1HBoxLayout->addWidget(m_framesV1Table, 100);

	QVBoxLayout* buttonsV1VBoxLayout = new QVBoxLayout;
	idV1HBoxLayout->addLayout(buttonsV1VBoxLayout);

	QPushButton* filenameV1PushButton =
		new QPushButton(i18n("From Filename"), m_idV1GroupBox);
	buttonsV1VBoxLayout->addWidget(filenameV1PushButton);

	QPushButton* id3V1PushButton =
		new QPushButton(i18n("From Tag 2"), m_idV1GroupBox);
	buttonsV1VBoxLayout->addWidget(id3V1PushButton);

	QPushButton* copyV1PushButton = new QPushButton(i18n("Copy"), m_idV1GroupBox);
	buttonsV1VBoxLayout->addWidget(copyV1PushButton);

	QPushButton* pasteV1PushButton =
		new QPushButton(i18n("Paste"), m_idV1GroupBox);
	buttonsV1VBoxLayout->addWidget(pasteV1PushButton);

	QPushButton* removeV1PushButton =
		new QPushButton(i18n("Remove"), m_idV1GroupBox);
	buttonsV1VBoxLayout->addWidget(removeV1PushButton);

	buttonsV1VBoxLayout->addItem(
		new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));

	m_idV2GroupBox = new QGroupBox(i18n("Tag &2"), m_rightHalfVBox);
	rightHalfLayout->addWidget(m_idV2GroupBox, 100);

	QHBoxLayout* idV2HBoxLayout = new QHBoxLayout(m_idV2GroupBox);
	idV2HBoxLayout->setMargin(margin);
	idV2HBoxLayout->setSpacing(spacing);
	m_framesV2Table = new FrameTable(m_idV2GroupBox, false);
	m_framelist = new FrameList(m_framesV2Table);
	idV2HBoxLayout->addWidget(m_framesV2Table);

	QVBoxLayout* buttonsV2VBoxLayout = new QVBoxLayout;
	idV2HBoxLayout->addLayout(buttonsV2VBoxLayout);

	QPushButton* filenameV2PushButton =
		new QPushButton(i18n("From Filename"), m_idV2GroupBox);
	buttonsV2VBoxLayout->addWidget(filenameV2PushButton);

	m_id3V2PushButton = new QPushButton(i18n("From Tag 1"), m_idV2GroupBox);
	buttonsV2VBoxLayout->addWidget(m_id3V2PushButton);

	QPushButton* copyV2PushButton =
		new QPushButton(i18n("Copy"), m_idV2GroupBox);
	buttonsV2VBoxLayout->addWidget(copyV2PushButton);

	QPushButton* pasteV2PushButton =
		new QPushButton(i18n("Paste"), m_idV2GroupBox);
	buttonsV2VBoxLayout->addWidget(pasteV2PushButton);

	QPushButton* removeV2PushButton =
		new QPushButton(i18n("Remove"), m_idV2GroupBox);
	buttonsV2VBoxLayout->addWidget(removeV2PushButton);

	buttonsV2VBoxLayout->insertSpacing(-1, 8);

	QPushButton* editFramesPushButton =
		new QPushButton(i18n("Edit"), m_idV2GroupBox);
	buttonsV2VBoxLayout->addWidget(editFramesPushButton);
	QPushButton* framesAddPushButton =
		new QPushButton(i18n("Add"), m_idV2GroupBox);
	buttonsV2VBoxLayout->addWidget(framesAddPushButton);
	QPushButton* deleteFramesPushButton =
		new QPushButton(i18n("Delete"), m_idV2GroupBox);
	buttonsV2VBoxLayout->addWidget(deleteFramesPushButton);

	m_pictureLabel = new PictureLabel(this);
	buttonsV2VBoxLayout->addWidget(m_pictureLabel);

	buttonsV2VBoxLayout->addItem(
		new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));

	rightHalfLayout->insertStretch(-1);
#else
	Kid3ScrollView* scrollView = new Kid3ScrollView(this);
	scrollView->setResizePolicy(QScrollView::AutoOneFit);
	scrollView->setFrameStyle(QFrame::NoFrame);
	m_rightHalfVBox = new QWidget(scrollView->viewport());
	QVBoxLayout* rightHalfLayout = new QVBoxLayout(m_rightHalfVBox);
	rightHalfLayout->setSpacing(2);
	rightHalfLayout->setMargin(2);

	QGroupBox* filenameGroupBox = new QGroupBox(i18n("File&name"), m_rightHalfVBox);
	filenameGroupBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	rightHalfLayout->addWidget(filenameGroupBox);
	QGridLayout* filenameGroupBoxLayout =
		new QGridLayout(filenameGroupBox, 5, 3, margin, spacing);
	int fontHeight = filenameGroupBox->fontMetrics().height();
	if (fontHeight > 0 && fontHeight < 100) {
		filenameGroupBoxLayout->addMultiCell(
			new QSpacerItem(1, fontHeight / 2, QSizePolicy::Fixed, QSizePolicy::Fixed),
			0, 0, 0, 2);
	}

	m_nameLabel = new QLabel(i18n("Name:"), filenameGroupBox);
	filenameGroupBoxLayout->addWidget(m_nameLabel, 1, 0);

	m_nameLineEdit = new QLineEdit(filenameGroupBox);
	filenameGroupBoxLayout->addWidget(m_nameLineEdit, 1, 1);
	// using "filenameGroupBoxLayout->addMultiCellWidget(m_nameLineEdit, 1, 1, 1, 2)"
	// makes the first column wide?!

	QLabel* formatLabel = new QLabel(i18n("Format:") + QChar(0x2191),
	                                 filenameGroupBox);
	filenameGroupBoxLayout->addWidget(formatLabel, 2, 0);

	m_formatComboBox = new QComboBox(filenameGroupBox);
	m_formatComboBox->setEditable(true);
	m_formatComboBox->setDuplicatesEnabled(false);
	QToolTip::add(m_formatComboBox, FrameFormatReplacer::getToolTip());
	filenameGroupBoxLayout->addWidget(m_formatComboBox, 2, 1);

	m_fnV1Button = new QPushButton(i18n("From Tag 1"), filenameGroupBox);
	filenameGroupBoxLayout->addWidget(m_fnV1Button, 2, 2);

	QLabel* formatFromFilenameLabel = new QLabel(i18n("Format:") + QChar(0x2193),
	                                             filenameGroupBox);
	filenameGroupBoxLayout->addWidget(formatFromFilenameLabel, 3, 0);

	m_formatFromFilenameComboBox = new QComboBox(filenameGroupBox);
	m_formatFromFilenameComboBox->setEditable(true);
	m_formatFromFilenameComboBox->setDuplicatesEnabled(false);
	QToolTip::add(m_formatFromFilenameComboBox, FrameFormatReplacer::getToolTip());
	filenameGroupBoxLayout->addWidget(m_formatFromFilenameComboBox, 3, 1);

	QPushButton* fnV2Button =
		new QPushButton(i18n("From Tag 2"), filenameGroupBox);
	filenameGroupBoxLayout->addWidget(fnV2Button, 3, 2);

	QLabel* infoLabel = new QLabel(i18n("Info:"), filenameGroupBox);
	filenameGroupBoxLayout->addWidget(infoLabel, 4, 0);

	m_detailsLabel = new QLabel(filenameGroupBox);
	filenameGroupBoxLayout->addMultiCellWidget(m_detailsLabel, 4, 4, 1, 2);

	m_idV1GroupBox =
		new QGroupBox(2, Qt::Horizontal, i18n("Tag &1"), m_rightHalfVBox);
	m_idV1GroupBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	m_idV1GroupBox->setInsideMargin(margin);
	m_idV1GroupBox->setInsideSpacing(spacing);
	rightHalfLayout->addWidget(m_idV1GroupBox, 100);

	m_framesV1Table = new FrameTable(m_idV1GroupBox, true);

	QVBox* buttonsV1VBox = new QVBox(m_idV1GroupBox);
	buttonsV1VBox->setSpacing(spacing);
	QPushButton* filenameV1PushButton =
		new QPushButton(i18n("From Filename"), buttonsV1VBox);
	QPushButton* id3V1PushButton =
		new QPushButton(i18n("From Tag 2"), buttonsV1VBox);
	QPushButton* copyV1PushButton =
		new QPushButton(i18n("Copy"), buttonsV1VBox);
	QPushButton* pasteV1PushButton =
		new QPushButton(i18n("Paste"), buttonsV1VBox);
	QPushButton* removeV1PushButton =
		new QPushButton(i18n("Remove"), buttonsV1VBox);
	new QWidget(buttonsV1VBox);

	m_idV2GroupBox =
		new QGroupBox(2, Qt::Horizontal, i18n("Tag &2"), m_rightHalfVBox);
	m_idV2GroupBox->setInsideMargin(margin);
	m_idV2GroupBox->setInsideSpacing(spacing);
	rightHalfLayout->addWidget(m_idV2GroupBox, 100);

	m_framesV2Table = new FrameTable(m_idV2GroupBox, false);
	m_framelist = new FrameList(m_framesV2Table);

	QVBox* buttonsV2VBox = new QVBox(m_idV2GroupBox);
	buttonsV2VBox->setSpacing(spacing);
	QPushButton* filenameV2PushButton =
		new QPushButton(i18n("From Filename"), buttonsV2VBox);
	m_id3V2PushButton = new QPushButton(i18n("From Tag 1"), buttonsV2VBox);
	QPushButton* copyV2PushButton =
		new QPushButton(i18n("Copy"), buttonsV2VBox);
	QPushButton* pasteV2PushButton =
		new QPushButton(i18n("Paste"), buttonsV2VBox);
	QPushButton* removeV2PushButton =
		new QPushButton(i18n("Remove"), buttonsV2VBox);

	QWidget* spacer = new QWidget(buttonsV2VBox);
	spacer->setMaximumHeight(spacing);

	QPushButton* editFramesPushButton =
		new QPushButton(i18n("Edit"), buttonsV2VBox);
	QPushButton* framesAddPushButton =
		new QPushButton(i18n("Add"), buttonsV2VBox);
	QPushButton* deleteFramesPushButton =
		new QPushButton(i18n("Delete"), buttonsV2VBox);

	m_pictureLabel = new PictureLabel(buttonsV2VBox);

	QWidget* expandWidget = new QWidget(buttonsV2VBox);
	expandWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

	rightHalfLayout->insertStretch(-1);
	scrollView->addChild(m_rightHalfVBox);
#endif

	// signals and slots connections
	connect(filenameV1PushButton, SIGNAL(clicked()), this,
			SLOT(fromFilenameV1()));
	connect(id3V1PushButton, SIGNAL(clicked()), this, SLOT(fromID3V1()));
	connect(copyV1PushButton, SIGNAL(clicked()), this, SLOT(copyV1()));
	connect(pasteV1PushButton, SIGNAL(clicked()), this, SLOT(pasteV1()));
	connect(removeV1PushButton, SIGNAL(clicked()), this, SLOT(removeV1()));
	connect(filenameV2PushButton, SIGNAL(clicked()), this,
			SLOT(fromFilenameV2()));
	connect(m_id3V2PushButton, SIGNAL(clicked()), this, SLOT(fromID3V2()));
	connect(copyV2PushButton, SIGNAL(clicked()), this, SLOT(copyV2()));
	connect(pasteV2PushButton, SIGNAL(clicked()), this, SLOT(pasteV2()));
	connect(removeV2PushButton, SIGNAL(clicked()), this, SLOT(removeV2()));
#if QT_VERSION >= 0x040000
	connect(m_fileListBox, SIGNAL(itemSelectionChanged()), this,
			SLOT(fileSelected()));
#else
	connect(m_fileListBox, SIGNAL(selectionChanged()), this,
			SLOT(fileSelected()));
#endif
	connect(framesAddPushButton, SIGNAL(clicked()), this, SLOT(addFrame()));
	connect(deleteFramesPushButton, SIGNAL(clicked()), this,
			SLOT(deleteFrame()));
	connect(editFramesPushButton, SIGNAL(clicked()), this, SLOT(editFrame()));
	connect(m_fnV1Button, SIGNAL(clicked()), this, SLOT(fnFromID3V1()));
	connect(fnV2Button, SIGNAL(clicked()), this, SLOT(fnFromID3V2()));
	connect(m_nameLineEdit, SIGNAL(textChanged(const QString&)), this,
			SLOT(nameLineEditChanged(const QString&)));
#if QT_VERSION >= 0x040000
	connect(m_dirListBox, SIGNAL(itemActivated(QListWidgetItem*)), this,
			SLOT(dirSelected(QListWidgetItem*)));
#else
	connect(m_dirListBox, SIGNAL(doubleClicked(QListBoxItem *)), this,
			SLOT(dirSelected(QListBoxItem *)));
	connect(m_dirListBox, SIGNAL(returnPressed(QListBoxItem *)), this,
			SLOT(dirSelected(QListBoxItem *)));
	connect(this, SIGNAL(windowResized()),
					m_framesV1Table, SLOT(triggerResize())); 
	connect(this, SIGNAL(windowResized()),
					m_framesV2Table, SLOT(triggerResize())); 
#endif
	connect(m_fileListBox, SIGNAL(selectedFilesRenamed()),
					SIGNAL(selectedFilesRenamed()));

	// tab order
	setTabOrder(m_fileListBox, m_dirListBox);
	setTabOrder(m_dirListBox, filenameGroupBox);
	setTabOrder(filenameGroupBox, m_nameLineEdit);
	setTabOrder(m_nameLineEdit, m_formatComboBox);
	setTabOrder(m_formatComboBox, m_formatFromFilenameComboBox);
	setTabOrder(m_formatFromFilenameComboBox, m_fnV1Button);
	setTabOrder(m_fnV1Button, fnV2Button);
	setTabOrder(fnV2Button, m_framesV1Table);
	setTabOrder(m_framesV1Table, filenameV1PushButton);
	setTabOrder(filenameV1PushButton, id3V1PushButton);
	setTabOrder(id3V1PushButton, copyV1PushButton);
	setTabOrder(copyV1PushButton, pasteV1PushButton);
	setTabOrder(pasteV1PushButton, removeV1PushButton);
	setTabOrder(removeV1PushButton, m_framesV2Table);
	setTabOrder(m_framesV2Table, filenameV2PushButton);
	setTabOrder(filenameV2PushButton, m_id3V2PushButton);
	setTabOrder(m_id3V2PushButton, copyV2PushButton);
	setTabOrder(copyV2PushButton, pasteV2PushButton);
	setTabOrder(pasteV2PushButton, removeV2PushButton);
	setTabOrder(removeV2PushButton, editFramesPushButton);
	setTabOrder(editFramesPushButton, framesAddPushButton);
	setTabOrder(framesAddPushButton, deleteFramesPushButton);
}

/**
 * Destructor.
 */
Id3Form::~Id3Form()
{
	delete m_framelist;
}

/**
 * Button ID3v1 From Filename.
 */
void Id3Form::fromFilenameV1()
{
	theApp->getTagsFromFilenameV1();
}

/**
 * Button ID3v2 From Filename.
 */
void Id3Form::fromFilenameV2()
{
	theApp->getTagsFromFilenameV2();
}

/**
 * Button ID3v2 From ID3v1.
 */
void Id3Form::fromID3V2()
{
	theApp->copyV1ToV2();
}

/**
 * Button ID3v1 From ID3v2.
 */
void Id3Form::fromID3V1()
{
	theApp->copyV2ToV1();
}

/**
 * Button ID3v1 Copy.
 */
void Id3Form::copyV1()
{
	theApp->copyTagsV1();
}

/**
 * Button ID3v2 Copy.
 */
void Id3Form::copyV2()
{
	theApp->copyTagsV2();
}

/**
 * Button ID3v2 Remove.
 */
void Id3Form::removeV2()
{
	theApp->removeTagsV2();
}

/**
 * Button ID3v1 Paste.
 */
void Id3Form::pasteV1()
{
	theApp->pasteTagsV1();
}

/**
 * Button ID3v2 Paste.
 */
void Id3Form::pasteV2()
{
	theApp->pasteTagsV2();
}

/**
 * Button ID3v1 Remove.
 */
void Id3Form::removeV1()
{
	theApp->removeTagsV1();
}

/**
 * File list box file selected
 */
void Id3Form::fileSelected()
{
	theApp->fileSelected();
}

/**
 * Get number of files selected in file list box.
 *
 * @return number of files selected.
 */
int Id3Form::numFilesSelected()
{
	return m_fileListBox->numFilesSelected();
}

/**
 * Get the number of files or directories selected in the file list box.
 *
 * @return number of files or directories selected.
 */
int Id3Form::numFilesOrDirsSelected()
{
	return m_fileListBox->numFilesOrDirsSelected();
}

/**
 * Accept drag.
 *
 * @param ev drag event.
 */
void Id3Form::dragEnterEvent(QDragEnterEvent* ev)
{
#if QT_VERSION >= 0x040000
	if (ev->mimeData()->hasFormat("text/uri-list") ||
	    ev->mimeData()->hasImage())
		ev->acceptProposedAction();
#else
	ev->accept(QTextDrag::canDecode(ev));
#endif
}

/**
 * Handle drop event.
 *
 * @param ev drop event.
 */
void Id3Form::dropEvent(QDropEvent* ev)
{
#if QT_VERSION >= 0x040000
	if (ev->mimeData()->hasImage()) {
		QImage image = qvariant_cast<QImage>(ev->mimeData()->imageData());
		theApp->dropImage(image);
		return;
	}
	QList<QUrl> urls = ev->mimeData()->urls();
	if (urls.isEmpty())
		return;
	QString text = urls.first().toLocalFile();
	if (!text.isEmpty()) {
		theApp->openDrop(text);
	} else {
		text = urls.first().toString();
		if (text.startsWith("http://")) {
			theApp->dropUrl(text);
		}
	}
#else
	QString text;
	if (QTextDrag::decode(ev, text)) {
		if (text.startsWith("http://")) {
			theApp->dropUrl(text);
		} else {
			theApp->openDrop(text);
		}
	}
#endif
}

/**
 * Frame list button Edit.
 */
void Id3Form::editFrame()
{
	theApp->editFrame();
}

/**
 * Frame list button Add.
 */
void Id3Form::addFrame()
{
	theApp->addFrame();
}

/**
 * Frame list button Delete.
 */
void Id3Form::deleteFrame()
{
	theApp->deleteFrame();
}

/**
 * Set filename according to ID3v1 tags.
 */

void Id3Form::fnFromID3V1()
{
	theApp->getFilenameFromTags(1);
}

/**
 * Set filename according to ID3v1 tags.
 */

void Id3Form::fnFromID3V2()
{
	theApp->getFilenameFromTags(2);
}

/**
 * Filename line edit is changed.
 * @param txt contents of line edit
 */
void Id3Form::nameLineEditChanged(const QString& txt)
{
	formatLineEdit(m_nameLineEdit, txt, &theApp->s_fnFormatCfg);
}

/**
 * Mark the filename as changed.
 * @param en true to mark as changed
 */
void Id3Form::markChangedFilename(bool en)
{
#if QT_VERSION >= 0x040000
	if (en) {
		QPalette changedPalette(m_nameLabel->palette());
		changedPalette.setBrush(QPalette::Active, QPalette::Window, changedPalette.mid());
		m_nameLabel->setPalette(changedPalette);
	} else {
		m_nameLabel->setPalette(QPalette());
	}
	m_nameLabel->setAutoFillBackground(en);
#else
	m_nameLabel->setBackgroundMode(en ? PaletteMid : PaletteBackground);
#endif
}

/**
 * Format string within line edit.
 *
 * @param le   line edit
 * @param txt  text in line edit
 * @param fcfg format configuration
 */
void Id3Form::formatLineEdit(QLineEdit* le, const QString& txt,
							 const FormatConfig* fcfg)
{
	if (fcfg->m_formatWhileEditing) {
		QString str(txt);
		fcfg->formatString(str);
		if (str != txt) {
			int curPos = le->cursorPosition();
			le->setText(str);
			le->setCursorPosition(curPos);
		}
	}
}

/**
 * Directory list box directory selected.
 *
 * @param item selected item
 */
void Id3Form::dirSelected(
#if QT_VERSION >= 0x040000
	QListWidgetItem*
#else
	QListBoxItem*
#endif
	item) {
	QDir dir(m_dirListBox->getDirname() + QDir::separator() +
					 item->text());
	m_dirListBox->setEntryToSelect(
		item->text() == ".." ? QDir(m_dirListBox->getDirname()).dirName() :
		QString::null);
	QString dirPath = dir.QCM_absolutePath();
	if (!dirPath.isEmpty()) {
		theApp->openDirectory(dirPath, true);
	}
}

/**
 * Enable or disable controls requiring ID3v1 tags.
 *
 * @param enable true to enable
 */
void Id3Form::enableControlsV1(bool enable)
{
	m_fnV1Button->setEnabled(enable);
	m_id3V2PushButton->setEnabled(enable);
	m_idV1GroupBox->setEnabled(enable);
}

/**
 * Display the format of tag 1.
 *
 * @param str string describing format, e.g. "ID3v1.1"
 */
void Id3Form::setTagFormatV1(const QString& str)
{
	QString txt = i18n("Tag &1");
	if (!str.isEmpty()) {
		txt += ": ";
		txt += str;
	}
	m_idV1GroupBox->setTitle(txt);
}

/**
 * Display the format of tag 2.
 *
 * @param str string describing format, e.g. "ID3v2.4"
 */
void Id3Form::setTagFormatV2(const QString& str)
{
	QString txt = i18n("Tag &2");
	if (!str.isEmpty()) {
		txt += ": ";
		txt += str;
	}
	m_idV2GroupBox->setTitle(txt);
}

/**
 * Adjust the size of the right half box.
 */
void Id3Form::adjustRightHalfBoxSize()
{
	m_rightHalfVBox->adjustSize();
}

/**
 * Hide or show tag 1 controls.
 *
 * @param hide true to hide, false to show
 */
void Id3Form::hideV1(bool hide)
{
	if (hide) {
		m_idV1GroupBox->hide();
	} else {
		m_idV1GroupBox->show();
	}
}

/**
 * Hide or show tag 2 controls.
 *
 * @param hide true to hide, false to show
 */
void Id3Form::hideV2(bool hide)
{
	if (hide) {
		m_idV2GroupBox->hide();
	} else {
		m_idV2GroupBox->show();
	}
}

/**
 * Hide or show picture.
 *
 * @param hide true to hide, false to show
 */
void Id3Form::hidePicture(bool hide)
{
	if (hide) {
		m_pictureLabel->hide();
	} else {
		m_pictureLabel->show();
	}
}

/**
 * Set focus on filename controls.
 */
void Id3Form::setFocusFilename()
{
	m_nameLineEdit->setFocus();
}

/**
 * Set focus on tag 1 controls.
 */
void Id3Form::setFocusV1()
{
	m_framesV1Table->setFocus();
}

/**
 * Set focus on tag 2 controls.
 */
void Id3Form::setFocusV2()
{
	m_framesV2Table->setFocus();
}

/**
 * Get the items from a combo box.
 *
 * @param comboBox combo box
 *
 * @return item texts from combo box.
 */
static QStringList getItemsFromComboBox(const QComboBox* comboBox)
{
	QStringList lst;
	for (int i = 0; i < comboBox->count(); ++i) {
#if QT_VERSION >= 0x040000
		lst += comboBox->itemText(i);
#else
		lst += comboBox->text(i);
#endif
	}
	return lst;
}

/**
 * Save the local settings to the configuration.
 */
void Id3Form::saveConfig()
{
	Kid3App::s_miscCfg.m_splitterSizes = sizes();
	Kid3App::s_miscCfg.m_vSplitterSizes = m_vSplitter->sizes();
	Kid3App::s_miscCfg.m_formatItem = m_formatComboBox->QCM_currentIndex();
	Kid3App::s_miscCfg.m_formatText = m_formatComboBox->currentText();
	Kid3App::s_miscCfg.m_formatItems = getItemsFromComboBox(m_formatComboBox);
	Kid3App::s_miscCfg.m_formatFromFilenameItem = m_formatFromFilenameComboBox->QCM_currentIndex();
	Kid3App::s_miscCfg.m_formatFromFilenameText = m_formatFromFilenameComboBox->currentText();
	Kid3App::s_miscCfg.m_formatFromFilenameItems = getItemsFromComboBox(m_formatFromFilenameComboBox);
}

/**
 * Read the local settings from the configuration.
 */
void Id3Form::readConfig()
{
	if (!Kid3App::s_miscCfg.m_splitterSizes.empty()) {
		setSizes(Kid3App::s_miscCfg.m_splitterSizes);
	} else {
		setSizes(
#if QT_VERSION >= 0x040000
			QList<int>()
#else
			QValueList<int>()
#endif
			<< 307 << 601);
	}
	if (!Kid3App::s_miscCfg.m_vSplitterSizes.empty()) {
		m_vSplitter->setSizes(Kid3App::s_miscCfg.m_vSplitterSizes);
	} else {
		m_vSplitter->setSizes(
#if QT_VERSION >= 0x040000
			QList<int>()
#else
			QValueList<int>()
#endif
			<< 451 << 109);
	}
	if (!Kid3App::s_miscCfg.m_formatItems.isEmpty()) {
		m_formatComboBox->clear();
		m_formatComboBox->QCM_addItems(Kid3App::s_miscCfg.m_formatItems);
	}
	if (!Kid3App::s_miscCfg.m_formatFromFilenameItems.isEmpty()) {
		m_formatFromFilenameComboBox->clear();
		m_formatFromFilenameComboBox->QCM_addItems(Kid3App::s_miscCfg.m_formatFromFilenameItems);
	}
#if QT_VERSION >= 0x040000
	m_formatComboBox->setItemText(Kid3App::s_miscCfg.m_formatItem,
																Kid3App::s_miscCfg.m_formatText);
	m_formatComboBox->setCurrentIndex(Kid3App::s_miscCfg.m_formatItem);
	m_formatFromFilenameComboBox->setItemText(
		Kid3App::s_miscCfg.m_formatFromFilenameItem,
		Kid3App::s_miscCfg.m_formatFromFilenameText);
	m_formatFromFilenameComboBox->setCurrentIndex(
		Kid3App::s_miscCfg.m_formatFromFilenameItem);
#else
	m_formatComboBox->setCurrentItem(Kid3App::s_miscCfg.m_formatItem);
	m_formatComboBox->setCurrentText(Kid3App::s_miscCfg.m_formatText);
	m_formatFromFilenameComboBox->setCurrentItem(
		Kid3App::s_miscCfg.m_formatFromFilenameItem);
	m_formatFromFilenameComboBox->setCurrentText(
		Kid3App::s_miscCfg.m_formatFromFilenameText);
#endif
}

/**
 * Init GUI.
 */
void Id3Form::initView()
{
	QStringList strList;
	m_formatComboBox->setEditable(true);
	for (const char** sl = MiscConfig::s_defaultFnFmtList; *sl != 0; ++sl) {
		strList += *sl;
	}
	m_formatComboBox->QCM_addItems(strList);
	m_formatFromFilenameComboBox->QCM_addItems(strList);
}

/**
 * Set details info text.
 *
 * @param info detail information
 */
void Id3Form::setDetailInfo(const TaggedFile::DetailInfo& info)
{
	QString str;
	if (info.valid) {
		str = info.format;
		str += ' ';
		if (info.bitrate > 0 && info.bitrate < 999) {
			if (info.vbr) str += "VBR ";
			str += QString::number(info.bitrate);
			str += " kbps ";
		}
		if (info.sampleRate > 0) {
			str += QString::number(info.sampleRate);
			str += " Hz ";
		}
		switch (info.channelMode) {
			case TaggedFile::DetailInfo::CM_Stereo:
				str += "Stereo ";
				break;
			case TaggedFile::DetailInfo::CM_JointStereo:
				str += "Joint Stereo ";
				break;
			default:
				if (info.channels > 0) {
					str += QString::number(info.channels);
					str += " Channels ";
				}
		}
		if (info.duration > 0) {
			str += TaggedFile::formatTime(info.duration);
		}
	}
	m_detailsLabel->setText(str);
}

/**
 * Select all files.
 */
void Id3Form::selectAllFiles()
{
	m_fileListBox->selectAll(
#if QT_VERSION < 0x040000
		true
#endif
		);
}

/**
 * Deselect all files.
 */
void Id3Form::deselectAllFiles()
{
#if QT_VERSION >= 0x040000
	m_fileListBox->clearSelection();
#else
	m_fileListBox->selectAll(false);
#endif
}

/**
 * Select first file.
 *
 * @return true if a file exists.
 */
bool Id3Form::selectFirstFile()
{
	return m_fileListBox->selectFirstFile();
}

/**
 * Select next file.
 *
 * @return true if a next file exists.
 */
bool Id3Form::selectNextFile()
{
	return m_fileListBox->selectNextFile();
}

/**
 * Select previous file.
 *
 * @return true if a previous file exists.
 */
bool Id3Form::selectPreviousFile()
{
	return m_fileListBox->selectPreviousFile();
}

#if QT_VERSION < 0x040000
/**
 * Called when the widget is resized.
 * @param ev resize event
 */
void Id3Form::resizeEvent(QResizeEvent* ev)
{
	emit windowResized();
	QSplitter::resizeEvent(ev);
}
#endif
