/**
 * \file configdialogpages.h
 * Pages for configuration dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 17 Sep 2003
 *
 * Copyright (C) 2003-2012  Urs Fleisch
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

#ifndef CONFIGDIALOGPAGES_H
#define CONFIGDIALOGPAGES_H

#include <QObject>
#include <QStringList>
#include "config.h"
#include "kid3api.h"

class QCheckBox;
class QString;
class QWidget;
class QComboBox;
class QLineEdit;
class QSpinBox;
class QStringListModel;
class QStandardItemModel;
class FormatBox;
class ConfigTable;
class CommandsTableModel;
class CheckableStringListModel;

/**
 * Pages for configuration dialog.
 */
class KID3_GUI_EXPORT ConfigDialogPages : public QObject {
  Q_OBJECT
public:
  /**
   * Constructor.
   */
  explicit ConfigDialogPages(QObject* parent = 0);

  /**
   * Destructor.
   */
  virtual ~ConfigDialogPages();

  /**
   * Create page with tags settings.
   * @return tags page.
   */
  QWidget* createTagsPage();

  /**
   * Create page with files settings.
   * @return files page.
   */
  QWidget* createFilesPage();

  /**
   * Create page with actions settings.
   * @return actions page.
   */
  QWidget* createActionsPage();

  /**
   * Create page with network settings.
   * @return network page.
   */
  QWidget* createNetworkPage();

  /**
   * Create page with plugins settings.
   * @return plugins page.
   */
  QWidget* createPluginsPage();

  /**
   * Set values in pages from current configuration.
   */
  void setConfig();

  /**
   * Get values from pages and store them in the current configuration.
   */
  void getConfig() const;

private:
  /** Load last-opened files checkbox */
  QCheckBox* m_loadLastOpenedFileCheckBox;
  /** Preserve timestamp checkbox */
  QCheckBox* m_preserveTimeCheckBox;
  /** Mark changes checkbox */
  QCheckBox* m_markChangesCheckBox;
  /** Filename for cover lineedit */
  QLineEdit* m_coverFileNameLineEdit;
  /** Mark truncated fields checkbox */
  QCheckBox* m_markTruncationsCheckBox;
  /** ID3v1 text encoding combo box */
  QComboBox* m_textEncodingV1ComboBox;
  /** Use track/total number of tracks format checkbox */
  QCheckBox* m_totalNumTracksCheckBox;
  /** Comment field name combo box */
  QComboBox* m_commentNameComboBox;
  /** Picture field name combo box */
  QComboBox* m_pictureNameComboBox;
  /** Mark if larger check box */
  QCheckBox* m_markOversizedPicturesCheckBox;
  /** Maximum picture size spin box */
  QSpinBox* m_maximumPictureSizeSpinBox;
  /** Genre as text instead of numeric string checkbox */
  QCheckBox* m_genreNotNumericCheckBox;
  /** ID3v2 text encoding combo box */
  QComboBox* m_textEncodingComboBox;
  /** ID3v2 version combo box */
  QComboBox* m_id3v2VersionComboBox;
  /** Number of digits in track number spin box */
  QSpinBox* m_trackNumberDigitsSpinBox;
  /** Filename Format box */
  FormatBox* m_fnFormatBox;
  /** ID3 Format box */
  FormatBox* m_tagFormatBox;
  /** Only custom genres checkbox */
  QCheckBox* m_onlyCustomGenresCheckBox;
  /** Model with list of custom genres */
  QStringListModel* m_genresEditModel;
  /** Model with standard tags selection */
  QStandardItemModel* m_quickAccessTagsModel;
  /** Play on double click checkbox */
  QCheckBox* m_playOnDoubleClickCheckBox;
  /** Commands table */
  ConfigTable* m_commandsTable;
  /** Commands table model */
  CommandsTableModel* m_commandsTableModel;
  /** Browser line edit */
  QLineEdit* m_browserLineEdit;
  /** Use proxy check box */
  QCheckBox* m_proxyCheckBox;
  /** Proxy line edit */
  QLineEdit* m_proxyLineEdit;
  /** Use proxy authentication check box */
  QCheckBox* m_proxyAuthenticationCheckBox;
  /** Proxy user name line edit */
  QLineEdit* m_proxyUserNameLineEdit;
  /** Proxy password line edit */
  QLineEdit* m_proxyPasswordLineEdit;
  /** Model with enabled metadata plugins */
  CheckableStringListModel* m_enabledMetadataPluginsModel;
  /** Model with enabled plugins */
  CheckableStringListModel* m_enabledPluginsModel;
};

#endif
