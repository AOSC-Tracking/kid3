/**
 * \file trackdata.cpp
 * Track data, frames with association to tagged file.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 23 Feb 2007
 *
 * Copyright (C) 2007-2013  Urs Fleisch
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

#include "trackdata.h"
#include <QString>
#include <QUrl>
#include <QDir>
#include <QCoreApplication>
#include "fileproxymodel.h"

/**
 * Constructor.
 *
 * @param trackData track data
 * @param str       string with format codes
 */
TrackDataFormatReplacer::TrackDataFormatReplacer(
  const TrackData& trackData, const QString& str) :
  FrameFormatReplacer(trackData, str), m_trackData(trackData) {}

/**
 * Destructor.
 */
TrackDataFormatReplacer::~TrackDataFormatReplacer() {}

/**
 * Replace a format code (one character %c or multiple characters %{chars}).
 * Supported format fields:
 * Those supported by FrameFormatReplacer::getReplacement()
 * %f filename
 * %p path to file
 * %u URL of file
 * %d duration in minutes:seconds
 * %D duration in seconds
 * %n number of tracks
 *
 * @param code format code
 *
 * @return replacement string,
 *         QString::null if code not found.
 */
QString TrackDataFormatReplacer::getReplacement(const QString& code) const
{
  QString result = FrameFormatReplacer::getReplacement(code);
  if (result.isNull()) {
    QString name;

    if (code.length() == 1) {
      static const struct {
        char shortCode;
        const char* longCode;
      } shortToLong[] = {
        { 'f', "file" },
        { 'p', "filepath" },
        { 'u', "url" },
        { 'd', "duration" },
        { 'D', "seconds" },
        { 'n', "tracks" },
        { 'e', "extension" },
        { 'O', "tag1" },
        { 'o', "tag2" },
        { 'b', "bitrate" },
        { 'v', "vbr" },
        { 'r', "samplerate" },
        { 'm', "mode" },
        { 'h', "channels" },
        { 'k', "codec" }
      };
      const char c = code[0].toLatin1();
      for (unsigned i = 0; i < sizeof(shortToLong) / sizeof(shortToLong[0]); ++i) {
        if (shortToLong[i].shortCode == c) {
          name = QString::fromLatin1(shortToLong[i].longCode);
          break;
        }
      }
    } else if (code.length() > 1) {
      name = code;
    }

    if (!name.isNull()) {
      TaggedFile::DetailInfo info;
      m_trackData.getDetailInfo(info);
      if (name == QLatin1String("file")) {
        QString filename(m_trackData.getAbsFilename());
        int sepPos = filename.lastIndexOf(QLatin1Char('/'));
        if (sepPos < 0) {
          sepPos = filename.lastIndexOf(QDir::separator());
        }
        if (sepPos >= 0) {
          filename.remove(0, sepPos + 1);
        }
        result = filename;
      } else if (name == QLatin1String("filepath")) {
        result = m_trackData.getAbsFilename();
      } else if (name == QLatin1String("url")) {
        QUrl url;
        url.setPath(m_trackData.getAbsFilename());
        url.setScheme(QLatin1String("file"));
        result = url.toString();
      } else if (name == QLatin1String("duration")) {
        result = TaggedFile::formatTime(m_trackData.getFileDuration());
      } else if (name == QLatin1String("seconds")) {
        result = QString::number(m_trackData.getFileDuration());
      } else if (name == QLatin1String("tracks")) {
        result = QString::number(m_trackData.getTotalNumberOfTracksInDir());
      } else if (name == QLatin1String("extension")) {
        result = m_trackData.getFileExtension();
      } else if (name == QLatin1String("tag1")) {
        result = m_trackData.getTagFormatV1();
      } else if (name == QLatin1String("tag2")) {
        result = m_trackData.getTagFormatV2();
      } else if (name == QLatin1String("bitrate")) {
        result.setNum(info.bitrate);
      } else if (name == QLatin1String("vbr")) {
        result = info.vbr ? QLatin1String("VBR") : QLatin1String("");
      } else if (name == QLatin1String("samplerate")) {
        result.setNum(info.sampleRate);
      } else if (name == QLatin1String("mode")) {
        switch (info.channelMode) {
          case TaggedFile::DetailInfo::CM_Stereo:
            result = QLatin1String("Stereo");
            break;
          case TaggedFile::DetailInfo::CM_JointStereo:
            result = QLatin1String("Joint Stereo");
            break;
          case TaggedFile::DetailInfo::CM_None:
          default:
            result = QLatin1String("");
        }
      } else if (name == QLatin1String("channels")) {
        result.setNum(info.channels);
      } else if (name == QLatin1String("codec")) {
        result = info.format;
      }
    }
  }

  return result;
}

/**
 * Get help text for supported format codes.
 *
 * @param onlyRows if true only the tr elements are returned,
 *                 not the surrounding table
 *
 * @return help text.
 */
QString TrackDataFormatReplacer::getToolTip(bool onlyRows)
{
  QString str;
  if (!onlyRows) str += QLatin1String("<table>\n");
  str += FrameFormatReplacer::getToolTip(true);

  str += QLatin1String("<tr><td>%f</td><td>%{file}</td><td>");
  str += QCoreApplication::translate("@default", "Filename");
  str += QLatin1String("</td></tr>\n");

  str += QLatin1String("<tr><td>%p</td><td>%{filepath}</td><td>");
  str += QCoreApplication::translate("@default", QT_TRANSLATE_NOOP("@default", "Absolute path to file"));
  str += QLatin1String("</td></tr>\n");

  str += QLatin1String("<tr><td>%u</td><td>%{url}</td><td>");
  str += QCoreApplication::translate("@default", "URL");
  str += QLatin1String("</td></tr>\n");

  str += QLatin1String("<tr><td>%d</td><td>%{duration}</td><td>");
  str += QCoreApplication::translate("@default", QT_TRANSLATE_NOOP("@default", "Length"));
  str += QLatin1String(" &quot;M:S&quot;</td></tr>\n");

  str += QLatin1String("<tr><td>%D</td><td>%{seconds}</td><td>");
  str += QCoreApplication::translate("@default", QT_TRANSLATE_NOOP("@default", "Length"));
  str += QLatin1String(" &quot;S&quot;</td></tr>\n");

  str += QLatin1String("<tr><td>%n</td><td>%{tracks}</td><td>");
  str += QCoreApplication::translate("@default", QT_TRANSLATE_NOOP("@default", "Number of tracks"));
  str += QLatin1String("</td></tr>\n");

  str += QLatin1String("<tr><td>%e</td><td>%{extension}</td><td>");
  str += QCoreApplication::translate("@default", QT_TRANSLATE_NOOP("@default", "Extension"));
  str += QLatin1String("</td></tr>\n");

  str += QLatin1String("<tr><td>%O</td><td>%{tag1}</td><td>");
  str += QCoreApplication::translate("@default", "Tag 1");
  str += QLatin1String("</td></tr>\n");

  str += QLatin1String("<tr><td>%o</td><td>%{tag2}</td><td>");
  str += QCoreApplication::translate("@default", "Tag 2");
  str += QLatin1String("</td></tr>\n");

  str += QLatin1String("<tr><td>%b</td><td>%{bitrate}</td><td>");
  str += QCoreApplication::translate("@default", QT_TRANSLATE_NOOP("@default", "Bitrate"));
  str += QLatin1String("</td></tr>\n");

  str += QLatin1String("<tr><td>%v</td><td>%{vbr}</td><td>");
  str += QCoreApplication::translate("@default", QT_TRANSLATE_NOOP("@default", "VBR"));
  str += QLatin1String("</td></tr>\n");

  str += QLatin1String("<tr><td>%r</td><td>%{samplerate}</td><td>");
  str += QCoreApplication::translate("@default", QT_TRANSLATE_NOOP("@default", "Samplerate"));
  str += QLatin1String("</td></tr>\n");

  str += QLatin1String("<tr><td>%m</td><td>%{mode}</td><td>Stereo, Joint Stereo</td></tr>\n");

  str += QLatin1String("<tr><td>%h</td><td>%{channels}</td><td>");
  str += QCoreApplication::translate("@default", QT_TRANSLATE_NOOP("@default", "Channels"));
  str += QLatin1String("</td></tr>\n");

  str += QLatin1String("<tr><td>%k</td><td>%{codec}</td><td>");
  str += QCoreApplication::translate("@default", QT_TRANSLATE_NOOP("@default", "Codec"));
  str += QLatin1String("</td></tr>\n");

  if (!onlyRows) str += QLatin1String("</table>\n");
  return str;
}


/**
 * Constructor.
 */
TrackData::TrackData()
{}

/**
 * Constructor.
 * All fields except the import duration are set from the tagged file,
 * which should be read using readTags() before.
 *
 * @param taggedFile tagged file providing track data
 * @param tagVersion source of frames
 */
TrackData::TrackData(TaggedFile& taggedFile, TagVersion tagVersion) :
  m_taggedFileIndex(taggedFile.getIndex())
{
  switch (tagVersion) {
  case TagV1:
    taggedFile.getAllFramesV1(*this);
    break;
  case TagV2:
    taggedFile.getAllFramesV2(*this);
    break;
  case TagV2V1:
  {
    FrameCollection framesV1;
    taggedFile.getAllFramesV1(framesV1);
    taggedFile.getAllFramesV2(*this);
    merge(framesV1);
    break;
  }
  case TagNone:
    ;
  }
}

/**
 * Get tagged file associated with this track data.
 * @return tagged file, 0 if none assigned.
 */
TaggedFile* TrackData::getTaggedFile() const {
  return FileProxyModel::getTaggedFileOfIndex(m_taggedFileIndex);
}

/**
 * Get duration of file.
 * @return duration of file.
 */
int TrackData::getFileDuration() const
{
  TaggedFile* taggedFile = getTaggedFile();
  return taggedFile ? taggedFile->getDuration() : 0;
}

/**
 * Get absolute filename.
 *
 * @return absolute file path.
 */
QString TrackData::getAbsFilename() const
{
  TaggedFile* taggedFile = getTaggedFile();
  return taggedFile ? taggedFile->getAbsFilename() : QString();
}

/**
 * Get filename.
 *
 * @return filename.
 */
QString TrackData::getFilename() const
{
  TaggedFile* taggedFile = getTaggedFile();
  return taggedFile ? taggedFile->getFilename() : QString();
}

/**
 * Get the format of tag 1.
 *
 * @return string describing format of tag 1,
 *         e.g. "ID3v1.1", "ID3v2.3", "Vorbis", "APE",
 *         QString::null if unknown.
 */
QString TrackData::getTagFormatV1() const
{
  TaggedFile* taggedFile = getTaggedFile();
  return taggedFile ? taggedFile->getTagFormatV1() : QString();
}

/**
 * Get the format of tag 2.
 *
 * @return string describing format of tag 2,
 *         e.g. "ID3v2.3", "Vorbis", "APE",
 *         QString::null if unknown.
 */
QString TrackData::getTagFormatV2() const
{
  TaggedFile* taggedFile = getTaggedFile();
  return taggedFile ? taggedFile->getTagFormatV2() : QString();
}

/**
 * Get detail info.
 * @param info the detail information is returned here
 */
void TrackData::getDetailInfo(TaggedFile::DetailInfo& info) const
{
  if (TaggedFile* taggedFile = getTaggedFile()) {
    taggedFile->getDetailInfo(info);
  }
}

/**
 * Format a string from track data.
 * Supported format fields:
 * Those supported by TrackDataFormatReplacer::getReplacement()
 *
 * @param format    format specification
 *
 * @return formatted string.
 */
QString TrackData::formatString(const QString& format) const
{
  TrackDataFormatReplacer fmt(*this, format);
  fmt.replaceEscapedChars();
  fmt.replacePercentCodes();
  return fmt.getString();
}

/**
 * Create filename from tags according to format string.
 *
 * @param str       format string containing codes supported by
 *                  TrackDataFormatReplacer::getReplacement()
 * @param isDirname true to generate a directory name
 *
 * @return format string with format codes replaced by tags.
 */
QString TrackData::formatFilenameFromTags(QString str, bool isDirname) const
{
  if (!isDirname) {
    // first remove directory part from str
    const int sepPos = str.lastIndexOf(QLatin1Char('/'));
    if (sepPos >= 0) {
      str.remove(0, sepPos + 1);
    }
    // add extension to str
    str += getFileExtension();
  }

  TrackDataFormatReplacer fmt(*this, str);
  fmt.replacePercentCodes(isDirname ?
                          FormatReplacer::FSF_ReplaceSeparators : 0);
  return fmt.getString();
}

/**
 * Get help text for format codes supported by formatString().
 *
 * @param onlyRows if true only the tr elements are returned,
 *                 not the surrounding table
 *
 * @return help text.
 */
QString TrackData::getFormatToolTip(bool onlyRows)
{
  return TrackDataFormatReplacer::getToolTip(onlyRows);
}

/**
 * Get file extension including the dot.
 *
 * @return file extension, e.g. ".mp3".
 */
QString TrackData::getFileExtension() const
{
  QString fileExtension;
  QString absFilename;
  if (TaggedFile* taggedFile = getTaggedFile()) {
    fileExtension = taggedFile->getFileExtension();
    absFilename = taggedFile->getAbsFilename();
  }
  if (!fileExtension.isEmpty()) {
    return fileExtension;
  } else {
    int dotPos = absFilename.lastIndexOf(QLatin1Char('.'));
    return dotPos != -1 ? absFilename.mid(dotPos) : QString();
  }
}

/**
 * Get the total number of tracks in the directory.
 *
 * @return total number of tracks, -1 if unavailable.
 */
int TrackData::getTotalNumberOfTracksInDir() const
{
  TaggedFile* taggedFile = getTaggedFile();
  return taggedFile ? taggedFile->getTotalNumberOfTracksInDir() : -1;
}


/**
 * Get the difference between the imported duration and the track's duration.
 * @return absolute value of time difference in seconds, -1 if not available.
 */
int ImportTrackData::getTimeDifference() const
{
  int fileDuration = getFileDuration();
  int importDuration = getImportDuration();
  return fileDuration != 0 && importDuration != 0
      ? fileDuration > importDuration
        ? fileDuration - importDuration
        : importDuration - fileDuration
      : -1;
}

namespace {

/**
 * Get lower case words found in string.
 * @return lower case words.
 */
QSet<QString> getLowerCaseWords(const QString& str)
{
  if (!str.isEmpty()) {
    QString normalized = str.normalized(QString::NormalizationForm_D).toLower();
    QString simplified;
    for (QString::const_iterator it = normalized.constBegin();
         it != normalized.constEnd();
         ++it) {
      if (it->isLetter()) {
        simplified += *it;
      } else if (it->isPunct() || it->isSpace() || it->isSymbol()) {
        simplified += QLatin1Char(' ');
      }
    }
    return simplified.split(QLatin1Char(' '), QString::SkipEmptyParts).toSet();
  }
  return QSet<QString>();
}

}

/**
 * Get words of file name.
 * @return lower case words found in file name.
 */
QSet<QString> ImportTrackData::getFilenameWords() const
{
  QString fileName = getFilename();
  int endIndex = fileName.lastIndexOf(QLatin1Char('.'));
  if (endIndex > 0) {
    fileName.truncate(endIndex);
  }
  return getLowerCaseWords(fileName);
}

/**
 * Get words of title.
 * @return lower case words found in title.
 */
QSet<QString> ImportTrackData::getTitleWords() const
{
  return getLowerCaseWords(getTitle());
}


/**
 * Clear vector and associated data.
 */
void ImportTrackDataVector::clearData()
{
  clear();
  m_coverArtUrl = QString();
}

/**
 * Get album artist.
 * @return album artist.
 */
QString ImportTrackDataVector::getArtist() const
{
  return getFrame(Frame::FT_Artist);
}

/**
 * Get album title.
 * @return album title.
 */
QString ImportTrackDataVector::getAlbum() const
{
  return getFrame(Frame::FT_Album);
}

/**
 * Check if tag 1 is supported in the first track.
 * @return true if tag 1 is supported.
 */
bool ImportTrackDataVector::isTagV1Supported() const
{
  if (!isEmpty()) {
    TaggedFile* taggedFile = at(0).getTaggedFile();
    if (taggedFile) {
      return taggedFile->isTagV1Supported();
    }
  }
  return true;
}

/**
 * Get frame from first track.
 * @param type frame type
 * @return value of frame.
 */
QString ImportTrackDataVector::getFrame(Frame::Type type) const
{
  QString result;
  if (!isEmpty()) {
    const ImportTrackData& trackData = at(0);
    result = trackData.getValue(type);
    if (!result.isEmpty())
      return result;
    TaggedFile* taggedFile = trackData.getTaggedFile();
    FrameCollection frames;
    taggedFile->getAllFramesV2(frames);
    result = frames.getValue(type);
    if (!result.isEmpty())
      return result;
    taggedFile->getAllFramesV1(frames);
    result = frames.getValue(type);
  }
  return result;
}

/**
 * Read the tags from the files.
 * This can be used to fill the track data with another tag version.
 *
 * @param tagVersion tag version to read
 */
void ImportTrackDataVector::readTags(ImportTrackData::TagVersion tagVersion)
{
  for (iterator it = begin(); it != end(); ++it) {
    if (TaggedFile* taggedFile = it->getTaggedFile()) {
      switch (tagVersion) {
      case ImportTrackData::TagV1:
        taggedFile->getAllFramesV1(*it);
        break;
      case ImportTrackData::TagV2:
        taggedFile->getAllFramesV2(*it);
        break;
      case ImportTrackData::TagV2V1:
      {
        FrameCollection framesV1;
        taggedFile->getAllFramesV1(framesV1);
        taggedFile->getAllFramesV2(*it);
        it->merge(framesV1);
        break;
      }
      case ImportTrackData::TagNone:
        ;
      }
    }
    it->setImportDuration(0);
    it->setEnabled(true);
  }
  setCoverArtUrl(QString());
}

#ifndef QT_NO_DEBUG
/**
 * Dump contents of tracks to debug console.
 */
void ImportTrackDataVector::dump() const
{
  qDebug("ImportTrackDataVector (%s - %s, %s):",
         qPrintable(getArtist()), qPrintable(getAlbum()),
         qPrintable(getCoverArtUrl()));
  for (const_iterator it = constBegin();
       it != constEnd();
       ++it) {
    const ImportTrackData& trackData = *it;
    int fileDuration = trackData.getFileDuration();
    int importDuration = trackData.getImportDuration();
    qDebug("%d:%02d, %d:%02d, %s, %d, %s, %s, %s, %d, %s",
           fileDuration / 60, fileDuration % 60,
           importDuration / 60, importDuration % 60,
           qPrintable(trackData.getFilename()),
           trackData.getTrack(),
           qPrintable(trackData.getTitle()),
           qPrintable(trackData.getArtist()),
           qPrintable(trackData.getAlbum()),
           trackData.getYear(),
           qPrintable(trackData.getGenre()));
  }
}
#endif
