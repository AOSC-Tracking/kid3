/**
 * \file editframefieldsdialog.cpp
 * Field edit dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 10 Jun 2009
 *
 * Copyright (C) 2003-2013  Urs Fleisch
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

#include "editframefieldsdialog.h"
#include <QPushButton>
#include <QImage>
#include <QClipboard>
#include <QTextEdit>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QApplication>
#include <QFile>
#include <QBuffer>
#include <QVBoxLayout>
#include <QMimeData>
#include "kid3application.h"
#include "imageviewer.h"
#include "taggedfile.h"
#include "qtcompatmac.h"
#include "config.h"
#include "configstore.h"
#ifdef CONFIG_USE_KDE
#include <kfiledialog.h>
#else
#include <QFileDialog>
#endif

/** QTextEdit with label above */
class LabeledTextEdit : public QWidget {
public:
  /**
   * Constructor.
   *
   * @param parent parent widget
   */
  LabeledTextEdit(QWidget* parent);

  /**
   * Get text.
   *
   * @return text.
   */
  QString text() const {
    return m_edit->toPlainText();
  }

  /**
   * Set text.
   *
   * @param txt text
   */
  void setText(const QString& txt) {
    m_edit->setPlainText(txt);
  }

  /**
   * Set focus to text field.
   */
  void setFocus() {
    m_edit->setFocus();
  }

  /**
   * Set label.
   *
   * @param txt label
   */
  void setLabel(const QString& txt) { m_label->setText(txt); }

private:
  /** Label above edit */
  QLabel* m_label;
  /** Text editor */
  QTextEdit* m_edit;
};


/** LineEdit with label above */
class LabeledLineEdit : public QWidget {
public:
  /**
   * Constructor.
   *
   * @param parent parent widget
   */
  LabeledLineEdit(QWidget* parent);

  /**
   * Get text.
   *
   * @return text.
   */
  QString text() const { return m_edit->text(); }

  /**
   * Set text.
   *
   * @param txt text
   */
  void setText(const QString& txt) { m_edit->setText(txt); }

  /**
   * Set label.
   *
   * @param txt label
   */
  void setLabel(const QString& txt) { m_label->setText(txt); }

private:
  /** Label above edit */
  QLabel* m_label;
  /** Line editor */
  QLineEdit* m_edit;
};


/** Combo box with label above */
class LabeledComboBox : public QWidget {
public:
  /**
   * Constructor.
   *
   * @param parent parent widget
   * @param strlst list with ComboBox items, terminated by NULL
   */
  LabeledComboBox(QWidget* parent, const char** strlst);

  /**
   * Get index of selected item.
   *
   * @return index.
   */
  int currentItem() const {
    return m_combo->currentIndex();
  }

  /**
   * Set index of selected item.
   *
   * @param idx index
   */
  void setCurrentItem(int idx) {
    m_combo->setCurrentIndex(idx);
  }

  /**
   * Set label.
   *
   * @param txt label
   */
  void setLabel(const QString& txt) { m_label->setText(txt); }

private:
  /** Label above combo box */
  QLabel* m_label;
  /** Combo box */
  QComboBox* m_combo;
};


/** QSpinBox with label above */
class LabeledSpinBox : public QWidget {
public:
  /**
   * Constructor.
   *
   * @param parent parent widget
   */
  LabeledSpinBox(QWidget* parent);

  /**
   * Get value.
   *
   * @return text.
   */
  int value() const { return m_spinbox->value(); }

  /**
   * Set value.
   *
   * @param value value
   */
  void setValue(int value) { m_spinbox->setValue(value); }

  /**
   * Set label.
   *
   * @param txt label
   */
  void setLabel(const QString& txt) { m_label->setText(txt); }

private:
  /** Label above edit */
  QLabel* m_label;
  /** Text editor */
  QSpinBox* m_spinbox;
};


/**
 * Constructor.
 *
 * @param parent parent widget
 */
LabeledTextEdit::LabeledTextEdit(QWidget* parent) :
  QWidget(parent)
{
  setObjectName(QLatin1String("LabeledTextEdit"));
  QVBoxLayout* layout = new QVBoxLayout(this);
  m_label = new QLabel(this);
  m_edit = new QTextEdit(this);
  layout->setContentsMargins(0, 0, 0, 0);
  m_edit->setAcceptRichText(false);
  layout->addWidget(m_label);
  layout->addWidget(m_edit);
}

/**
 * Constructor.
 *
 * @param parent parent widget
 */
LabeledLineEdit::LabeledLineEdit(QWidget* parent) :
  QWidget(parent)
{
  setObjectName(QLatin1String("LabeledLineEdit"));
  QVBoxLayout* layout = new QVBoxLayout(this);
  m_label = new QLabel(this);
  m_edit = new QLineEdit(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(m_label);
  layout->addWidget(m_edit);
}

/**
 * Constructor.
 *
 * @param parent parent widget
 * @param strlst list with ComboBox items, terminated by NULL
 */
LabeledComboBox::LabeledComboBox(QWidget* parent,
         const char **strlst) : QWidget(parent)
{
  setObjectName(QLatin1String("LabeledComboBox"));
  QVBoxLayout* layout = new QVBoxLayout(this);
  m_label = new QLabel(this);
  m_combo = new QComboBox(this);
  layout->setContentsMargins(0, 0, 0, 0);
  QStringList strList;
  while (*strlst) {
    strList += QCM_translate(*strlst++);
  }
  m_combo->addItems(strList);
  layout->addWidget(m_label);
  layout->addWidget(m_combo);
}

/**
 * Constructor.
 *
 * @param parent parent widget
 */
LabeledSpinBox::LabeledSpinBox(QWidget* parent) :
  QWidget(parent)
{
  setObjectName(QLatin1String("LabeledSpinBox"));
  QVBoxLayout* layout = new QVBoxLayout(this);
  m_label = new QLabel(this);
  m_spinbox = new QSpinBox(this);
  if (layout && m_label && m_spinbox) {
    m_spinbox->setRange(0, INT_MAX);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_label);
    layout->addWidget(m_spinbox);
  }
}


/** Base class for MP3 field controls */
class Mp3FieldControl : public FieldControl {
public:
  /**
   * Constructor.
   * @param field field to edit
   */
  Mp3FieldControl(Frame::Field& field) :
    m_field(field) {}

  /**
   * Destructor.
   */
  virtual ~Mp3FieldControl() {}

protected:
  /**
   * Get description for ID3_Field.
   *
   * @param id ID of field
   * @return description or NULL if id unknown.
   */
  const char* getFieldIDString(Frame::Field::Id id) const;

  /** field */
  Frame::Field& m_field;
};

/** Control to edit standard UTF text fields */
class TextFieldControl : public Mp3FieldControl {
public:
  /**
   * Constructor.
   * @param field field to edit
   */
  TextFieldControl(Frame::Field& field) :
    Mp3FieldControl(field), m_edit(0) {}

  /**
   * Destructor.
   */
  virtual ~TextFieldControl() {}

  /**
   * Update field from data in field control.
   */
  virtual void updateTag();

  /**
   * Create widget to edit field data.
   *
   * @param parent parent widget
   *
   * @return widget to edit field data.
   */
  virtual QWidget* createWidget(QWidget* parent);

protected:
  /** Text editor widget */
  LabeledTextEdit* m_edit;
};

/** Control to edit single line text fields */
class LineFieldControl : public Mp3FieldControl {
public:
  /**
   * Constructor.
   * @param field field to edit
   */
  LineFieldControl(Frame::Field& field) :
    Mp3FieldControl(field), m_edit(0) {}

  /**
   * Destructor.
   */
  virtual ~LineFieldControl() {}

  /**
   * Update field from data in field control.
   */
  virtual void updateTag();

  /**
   * Create widget to edit field data.
   *
   * @param parent parent widget
   *
   * @return widget to edit field data.
   */
  virtual QWidget* createWidget(QWidget* parent);

protected:
  /** Line editor widget */
  LabeledLineEdit* m_edit;
};

/** Control to edit integer fields */
class IntFieldControl : public Mp3FieldControl {
public:
  /**
   * Constructor.
   * @param field field to edit
   */
  IntFieldControl(Frame::Field& field) :
    Mp3FieldControl(field), m_numInp(0) {}

  /**
   * Destructor.
   */
  virtual ~IntFieldControl() {}

  /**
   * Update field from data in field control.
   */
  virtual void updateTag();

  /**
   * Create widget to edit field data.
   *
   * @param parent parent widget
   *
   * @return widget to edit field data.
   */
  virtual QWidget* createWidget(QWidget* parent);

protected:
  /** Spin box widget */
  LabeledSpinBox* m_numInp;
};

/** Control to edit integer fields using a combo box with given values */
class IntComboBoxControl : public Mp3FieldControl {
public:
  /**
   * Constructor.
   * @param field field to edit
   * @param lst list of strings with possible selections, NULL terminated
   */
  IntComboBoxControl(Frame::Field& field,
                     const char **lst) :
    Mp3FieldControl(field), m_ptInp(0), m_strLst(lst) {}

  /**
   * Destructor.
   */
  virtual ~IntComboBoxControl() {}

  /**
   * Update field from data in field control.
   */
  virtual void updateTag();

  /**
   * Create widget to edit field data.
   *
   * @param parent parent widget
   *
   * @return widget to edit field data.
   */
  virtual QWidget* createWidget(QWidget* parent);

protected:
  /** Combo box widget */
  LabeledComboBox* m_ptInp;
  /** List of strings with possible selections */
  const char** m_strLst;
};

/** Control to import, export and view data from binary fields */
class BinFieldControl : public Mp3FieldControl {
public:
  /**
   * Constructor.
   * @param field      field to edit
   * @param frame      frame with fields to edit
   * @param taggedFile file
   */
  BinFieldControl(Frame::Field& field,
                  const Frame& frame, const TaggedFile* taggedFile) :
    Mp3FieldControl(field), m_bos(0), m_frame(frame), m_taggedFile(taggedFile) {}

  /**
   * Destructor.
   */
  virtual ~BinFieldControl() {}

  /**
   * Update field from data in field control.
   */
  virtual void updateTag();

  /**
   * Create widget to edit field data.
   *
   * @param parent parent widget
   *
   * @return widget to edit field data.
   */
  virtual QWidget* createWidget(QWidget* parent);

protected:
  /** Import, Export, View buttons */
  BinaryOpenSave* m_bos;
  /** frame with fields to edit */
  const Frame& m_frame;
  /** tagged file */
  const TaggedFile* m_taggedFile;
};


/**
 * Constructor.
 *
 * @param parent parent widget
 * @param field  field containing binary data
 */
BinaryOpenSave::BinaryOpenSave(QWidget* parent, const Frame::Field& field) :
  QWidget(parent), m_byteArray(field.m_value.toByteArray()),
  m_isChanged(false)
{
  setObjectName(QLatin1String("BinaryOpenSave"));
  QHBoxLayout* layout = new QHBoxLayout(this);
  m_label = new QLabel(this);
  m_clipButton = new QPushButton(i18n("From Clip&board"), this);
  QPushButton* openButton = new QPushButton(i18n("&Import"), this);
  QPushButton* saveButton = new QPushButton(i18n("&Export"), this);
  QPushButton* viewButton = new QPushButton(i18n("&View"), this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(m_label);
  layout->addWidget(m_clipButton);
  layout->addWidget(openButton);
  layout->addWidget(saveButton);
  layout->addWidget(viewButton);
  connect(m_clipButton, SIGNAL(clicked()), this, SLOT(clipData()));
  connect(openButton, SIGNAL(clicked()), this, SLOT(loadData()));
  connect(saveButton, SIGNAL(clicked()), this, SLOT(saveData()));
  connect(viewButton, SIGNAL(clicked()), this, SLOT(viewData()));
  connect(QApplication::clipboard(), SIGNAL(dataChanged()), this, SLOT(setClipButtonState()));
  setClipButtonState();
}

/**
 * Enable the "From Clipboard" button if the clipboard contains an image.
 */
void BinaryOpenSave::setClipButtonState()
{
  QClipboard* cb = QApplication::clipboard();
  m_clipButton->setEnabled(
    cb && (cb->mimeData()->hasFormat(QLatin1String("image/jpeg")) ||
           cb->mimeData()->hasImage()));
}

/**
 * Load image from clipboard.
 */
void BinaryOpenSave::clipData()
{
  QClipboard* cb = QApplication::clipboard();
  if (cb) {
    if (cb->mimeData()->hasFormat(QLatin1String("image/jpeg"))) {
      m_byteArray = cb->mimeData()->data(QLatin1String("image/jpeg"));
      m_isChanged = true;
    } else if (cb->mimeData()->hasImage()) {
      QBuffer buffer(&m_byteArray);
      buffer.open(QIODevice::WriteOnly);
      cb->image().save(&buffer, "JPG");
      m_isChanged = true;
    }
  }
}

/**
 * Request name of file to import binary data from.
 * The data is imported later when Ok is pressed in the parent dialog.
 */
void BinaryOpenSave::loadData()
{
#ifdef CONFIG_USE_KDE
  QString loadfilename = KFileDialog::getOpenFileName(
    m_defaultDir.isEmpty() ? Kid3Application::getDirName() : m_defaultDir,
    m_filter, this);
#else
  QString loadfilename = QFileDialog::getOpenFileName(
    this, QString(),
    m_defaultDir.isEmpty() ? Kid3Application::getDirName() : m_defaultDir,
    m_filter, 0, ConfigStore::s_miscCfg.m_dontUseNativeDialogs
    ? QFileDialog::DontUseNativeDialog : QFileDialog::Options(0));
#endif
  if (!loadfilename.isEmpty()) {
    QFile file(loadfilename);
    if (file.open(QIODevice::ReadOnly)) {
      size_t size = file.size();
      char* data = new char[size];
      QDataStream stream(&file);
      stream.readRawData(data, size);
      m_byteArray = QByteArray(data, size);
      m_isChanged = true;
      delete [] data;
      file.close();
    }
  }
}

/**
 * Request name of file and export binary data.
 */
void BinaryOpenSave::saveData()
{
  QString dir = m_defaultDir.isEmpty() ? Kid3Application::getDirName() : m_defaultDir;
  if (!m_defaultFile.isEmpty()) {
    QChar separator = QDir::separator();
    if (!dir.endsWith(separator)) {
      dir += separator;
    }
    dir += m_defaultFile;
  }
#ifdef CONFIG_USE_KDE
  QString fn = KFileDialog::getSaveFileName(dir, m_filter, this);
#else
  QString fn = QFileDialog::getSaveFileName(this, QString(), dir,
    m_filter, 0, ConfigStore::s_miscCfg.m_dontUseNativeDialogs
    ? QFileDialog::DontUseNativeDialog : QFileDialog::Options(0));
#endif
  if (!fn.isEmpty()) {
    QFile file(fn);
    if (file.open(QIODevice::WriteOnly)) {
      QDataStream stream(&file);
      stream.writeRawData(m_byteArray.data(), m_byteArray.size());
      file.close();
    }
  }
}

/**
 * Create image from binary data and display it in window.
 */
void BinaryOpenSave::viewData()
{
  QImage image;
  if (image.loadFromData(m_byteArray)) {
    ImageViewer iv(this, image);
    iv.exec();
  }
}

/**
 * Get description for ID3_Field.
 *
 * @param id ID of field
 * @return description or NULL if id unknown.
 */
const char* Mp3FieldControl::getFieldIDString(Frame::Field::Id id) const
{
  static const char* const idStr[] = {
    "Unknown",
    I18N_NOOP("Text Encoding"),
    I18N_NOOP("Text"),
    I18N_NOOP("URL"),
    I18N_NOOP("Data"),
    I18N_NOOP("Description"),
    I18N_NOOP("Owner"),
    I18N_NOOP("Email"),
    I18N_NOOP("Rating"),
    I18N_NOOP("Filename"),
    I18N_NOOP("Language"),
    I18N_NOOP("Picture Type"),
    I18N_NOOP("Image format"),
    I18N_NOOP("Mimetype"),
    I18N_NOOP("Counter"),
    I18N_NOOP("Identifier"),
    I18N_NOOP("Volume Adjustment"),
    I18N_NOOP("Number of Bits"),
    I18N_NOOP("Volume Change Right"),
    I18N_NOOP("Volume Change Left"),
    I18N_NOOP("Peak Volume Right"),
    I18N_NOOP("Peak Volume Left"),
    I18N_NOOP("Timestamp Format"),
    I18N_NOOP("Content Type"),

    I18N_NOOP("Price"),
    I18N_NOOP("Date"),
    I18N_NOOP("Seller")
  };
  class not_used { int array_size_check[
      sizeof(idStr) / sizeof(idStr[0]) == Frame::Field::ID_Seller + 1
      ? 1 : -1 ]; };
  return idStr[id <= Frame::Field::ID_Seller ? id : 0];
}

/**
 * Update field with data from dialog.
 */
void TextFieldControl::updateTag()
{
  m_field.m_value = m_edit->text();
}

/**
 * Create widget for dialog.
 *
 * @param parent parent widget
 * @return widget to edit field.
 */
QWidget* TextFieldControl::createWidget(QWidget* parent)
{
  m_edit = new LabeledTextEdit(parent);
  if (m_edit == NULL)
    return NULL;

  m_edit->setLabel(QCM_translate(getFieldIDString(static_cast<Frame::Field::Id>(m_field.m_id))));
  m_edit->setText(m_field.m_value.toString());
  m_edit->setFocus();
  return m_edit;
}

/**
 * Update field with data from dialog.
 */
void LineFieldControl::updateTag()
{
  m_field.m_value = m_edit->text();
}

/**
 * Create widget for dialog.
 *
 * @param parent parent widget
 * @return widget to edit field.
 */
QWidget* LineFieldControl::createWidget(QWidget* parent)
{
  m_edit = new LabeledLineEdit(parent);
  m_edit->setLabel(QCM_translate(getFieldIDString(static_cast<Frame::Field::Id>(m_field.m_id))));
  m_edit->setText(m_field.m_value.toString());
  return m_edit;
}

/**
 * Update field with data from dialog.
 */
void IntFieldControl::updateTag()
{
  m_field.m_value = m_numInp->value();
}

/**
 * Create widget for dialog.
 *
 * @param parent parent widget
 * @return widget to edit field.
 */
QWidget* IntFieldControl::createWidget(QWidget* parent)
{
  m_numInp = new LabeledSpinBox(parent);
  m_numInp->setLabel(QCM_translate(getFieldIDString(static_cast<Frame::Field::Id>(m_field.m_id))));
  m_numInp->setValue(m_field.m_value.toInt());
  return m_numInp;
}

/**
 * Update field with data from dialog.
 */
void IntComboBoxControl::updateTag()
{
  m_field.m_value = m_ptInp->currentItem();
}

/**
 * Create widget for dialog.
 *
 * @param parent parent widget
 * @return widget to edit field.
 */
QWidget* IntComboBoxControl::createWidget(QWidget* parent)
{
  m_ptInp = new LabeledComboBox(parent, m_strLst);
  m_ptInp->setLabel(QCM_translate(getFieldIDString(static_cast<Frame::Field::Id>(m_field.m_id))));
  m_ptInp->setCurrentItem(m_field.m_value.toInt());
  return m_ptInp;
}

/**
 * Update field with data from dialog.
 */
void BinFieldControl::updateTag()
{
  if (m_bos && m_bos->isChanged()) {
    m_field.m_value = m_bos->getData();
  }
}

/**
 * Create widget for dialog.
 *
 * @param parent parent widget
 * @return widget to edit field.
 */
QWidget* BinFieldControl::createWidget(QWidget* parent)
{
  m_bos = new BinaryOpenSave(parent, m_field);
  m_bos->setLabel(QCM_translate(getFieldIDString(static_cast<Frame::Field::Id>(m_field.m_id))));
  if (m_taggedFile) {
    m_bos->setDefaultDir(m_taggedFile->getDirname());
  }
  if (m_frame.getType() == Frame::FT_Picture) {
    m_bos->setDefaultFile(ConfigStore::s_miscCfg.m_defaultCoverFileName);
    m_bos->setFilter(
#ifdef CONFIG_USE_KDE
          QLatin1String("*.jpg *.jpeg *.png|") +
          i18n("Images (*.jpg *.jpeg *.png)") + QLatin1String("\n*|") + i18n("All Files (*)")
#else
          i18n("Images (*.jpg *.jpeg *.png)") + QLatin1String(";;") + i18n("All Files (*)")
#endif
          );
  }
  return m_bos;
}


/**
 * Update fields and get edited fields.
 *
 * @return field list.
 */
const Frame::FieldList& EditFrameFieldsDialog::getUpdatedFieldList()
{
  QListIterator<FieldControl*> it(m_fieldcontrols);
  while (it.hasNext()) {
    it.next()->updateTag();
  }
  return m_fields;
}


/**
 * Constructor.
 *
 * @param parent     parent widget
 * @param caption    caption
 * @param frame      frame with fields to edit
 * @param taggedFile file
 */
EditFrameFieldsDialog::EditFrameFieldsDialog(
  QWidget* parent, const QString& caption,
  const Frame& frame, const TaggedFile* taggedFile) :
  QDialog(parent), m_fields(frame.getFieldList())
{
  setObjectName(QLatin1String("EditFrameFieldsDialog"));
  setModal(true);
  setWindowTitle(caption);
  qDeleteAll(m_fieldcontrols);
  m_fieldcontrols.clear();
  QVBoxLayout* vlayout = new QVBoxLayout(this);

  for (Frame::FieldList::iterator fldIt = m_fields.begin();
       fldIt != m_fields.end();
       ++fldIt) {
    Frame::Field& fld = *fldIt;
    switch (fld.m_value.type()) {
      case QVariant::Int:
      case QVariant::UInt:
        if (fld.m_id == Frame::Field::ID_TextEnc) {
          static const char* strlst[] = {
            I18N_NOOP("ISO-8859-1"),
            I18N_NOOP("UTF16"),
            I18N_NOOP("UTF16BE"),
            I18N_NOOP("UTF8"),
            NULL
          };
          IntComboBoxControl* cbox = new IntComboBoxControl(fld, strlst);
          m_fieldcontrols.append(cbox);
        }
        else if (fld.m_id == Frame::Field::ID_PictureType) {
          static const char* strlst[] = {
            I18N_NOOP("Other"),
            I18N_NOOP("32x32 pixels PNG file icon"),
            I18N_NOOP("Other file icon"),
            I18N_NOOP("Cover (front)"),
            I18N_NOOP("Cover (back)"),
            I18N_NOOP("Leaflet page"),
            I18N_NOOP("Media"),
            I18N_NOOP("Lead artist/lead performer/soloist"),
            I18N_NOOP("Artist/performer"),
            I18N_NOOP("Conductor"),
            I18N_NOOP("Band/Orchestra"),
            I18N_NOOP("Composer"),
            I18N_NOOP("Lyricist/text writer"),
            I18N_NOOP("Recording Location"),
            I18N_NOOP("During recording"),
            I18N_NOOP("During performance"),
            I18N_NOOP("Movie/video screen capture"),
            I18N_NOOP("A bright coloured fish"),
            I18N_NOOP("Illustration"),
            I18N_NOOP("Band/artist logotype"),
            I18N_NOOP("Publisher/Studio logotype"),
            NULL
          };
          IntComboBoxControl* cbox = new IntComboBoxControl(fld, strlst);
          m_fieldcontrols.append(cbox);
        }
        else if (fld.m_id == Frame::Field::ID_TimestampFormat) {
          static const char* strlst[] = {
            I18N_NOOP("Other"),
            I18N_NOOP("MPEG frames as unit"),
            I18N_NOOP("Milliseconds as unit"),
            NULL
          };
          IntComboBoxControl* cbox = new IntComboBoxControl(fld, strlst);
          m_fieldcontrols.append(cbox);
        }
        else if (fld.m_id == Frame::Field::ID_ContentType) {
          static const char* strlst[] = {
            I18N_NOOP("Other"),
            I18N_NOOP("Lyrics"),
            I18N_NOOP("Text transcription"),
            I18N_NOOP("Movement/part name"),
            I18N_NOOP("Events"),
            I18N_NOOP("Chord"),
            I18N_NOOP("Trivia/pop up"),
            NULL
          };
          IntComboBoxControl* cbox = new IntComboBoxControl(fld, strlst);
          m_fieldcontrols.append(cbox);
        }
        else {
          IntFieldControl* intctl = new IntFieldControl(fld);
          m_fieldcontrols.append(intctl);
        }
        break;

      case QVariant::String:
        if (fld.m_id == Frame::Field::ID_Text) {
          // Large textedit for text fields
          TextFieldControl* textctl = new TextFieldControl(fld);
          m_fieldcontrols.append(textctl);
        }
        else {
          LineFieldControl* textctl = new LineFieldControl(fld);
          m_fieldcontrols.append(textctl);
        }
        break;

      case QVariant::ByteArray:
      {
        BinFieldControl* binctl = new BinFieldControl(fld, frame, taggedFile);
        m_fieldcontrols.append(binctl);
        break;
      }

      default:
        qDebug("Unknown type %d in field %d", fld.m_value.type(), fld.m_id);
    }
  }

  QListIterator<FieldControl*> it(m_fieldcontrols);
  while (it.hasNext()) {
    vlayout->addWidget(it.next()->createWidget(this));
  }

  QHBoxLayout* hlayout = new QHBoxLayout;
  QSpacerItem* hspacer = new QSpacerItem(16, 0, QSizePolicy::Expanding,
             QSizePolicy::Minimum);
  QPushButton* okButton = new QPushButton(i18n("&OK"), this);
  QPushButton* cancelButton = new QPushButton(i18n("&Cancel"), this);
  hlayout->addItem(hspacer);
  hlayout->addWidget(okButton);
  hlayout->addWidget(cancelButton);
  okButton->setDefault(true);
  connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
  connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
  vlayout->addLayout(hlayout);
  setMinimumWidth(525);
}

/**
 * Destructor.
 */
EditFrameFieldsDialog::~EditFrameFieldsDialog()
{
  qDeleteAll(m_fieldcontrols);
  m_fieldcontrols.clear();
}
