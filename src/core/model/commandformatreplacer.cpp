/**
 * \file commandformatreplacer.cpp
 * Replaces context command format codes in a string.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 09 Aug 2011
 *
 * Copyright (C) 2011  Urs Fleisch
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

#include "commandformatreplacer.h"
#include <QStringList>
#include <QUrl>
#include <QDir>
#include "configstore.h"
#include "qtcompatmac.h"

/**
 * Constructor.
 *
 * @param frames frame collection
 * @param str    string with format codes
 * @param files  file list
 * @param isDir  true if directory
 */
CommandFormatReplacer::CommandFormatReplacer(
  const FrameCollection& frames, const QString& str,
  const QStringList& files, bool isDir) :
  FrameFormatReplacer(frames, str), m_files(files), m_isDir(isDir) {}

/**
 * Destructor.
 */
CommandFormatReplacer::~CommandFormatReplacer() {}

/**
 * Replace a format code (one character %c or multiple characters %{chars}).
 * Supported format fields:
 * Those supported by FrameFormatReplacer::getReplacement()
 * %f %{file} filename
 * %d %{directory} directory name
 * %b %{browser} the web browser set in the configuration
 *
 * @param code format code
 *
 * @return replacement string,
 *         QString::null if code not found.
 */
QString CommandFormatReplacer::getReplacement(const QString& code) const
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
        { 'd', "directory" },
        { 'b', "browser" }
      };
      const char c = code[0].toLatin1();
      for (unsigned i = 0; i < sizeof(shortToLong) / sizeof(shortToLong[0]); ++i) {
        if (shortToLong[i].shortCode == c) {
          name = shortToLong[i].longCode;
          break;
        }
      }
    } else if (code.length() > 1) {
      name = code;
    }

    if (!name.isNull()) {
      if (name == "file") {
        result = m_files.front();
      } else if (name == "directory") {
        result = m_files.front();
        if (!m_isDir) {
          int sepPos = result.lastIndexOf('/');
          if (sepPos < 0) {
            sepPos = result.lastIndexOf(QDir::separator());
          }
          if (sepPos >= 0) {
            result.truncate(sepPos);
          }
        }
      } else if (name == "browser") {
        result = ConfigStore::s_miscCfg.m_browser;
      } else if (name == "url") {
        if (!m_files.empty()) {
          QUrl url;
          url.setScheme("file");
          url.setPath(m_files.front());
          result = url.toString();
        }
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
QString CommandFormatReplacer::getToolTip(bool onlyRows)
{
  QString str;
  if (!onlyRows) str += "<table>\n";
  str += FrameFormatReplacer::getToolTip(true);

  str += "<tr><td>%f</td><td>%{file}</td><td>";
  str += QCM_translate("Filename");
  str += "</td></tr>\n";

  str += "<tr><td>%F</td><td>%{files}</td><td>";
  str += QCM_translate(I18N_NOOP("Filenames"));
  str += "</td></tr>\n";

  str += "<tr><td>%uf</td><td>%{url}</td><td>";
  str += QCM_translate("URL");
  str += "</td></tr>\n";

  str += "<tr><td>%uF</td><td>%{urls}</td><td>";
  str += QCM_translate(I18N_NOOP("URLs"));
  str += "</td></tr>\n";

  str += "<tr><td>%d</td><td>%{directory}</td><td>";
  str += QCM_translate(I18N_NOOP("Directory name"));
  str += "</td></tr>\n";

  str += "<tr><td>%b</td><td>%{browser}</td><td>";
  str += QCM_translate("Browser");
  str += "</td></tr>\n";

  str += "<tr><td>%ua...</td><td>%u{artist}...</td><td>";
  str += QCM_translate(I18N_NOOP("Encode as URL"));
  str += "</td></tr>\n";

  if (!onlyRows) str += "</table>\n";
  return str;
}
