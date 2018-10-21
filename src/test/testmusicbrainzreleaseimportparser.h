/**
 * \file testmusicbrainzreleaseimportparser.h
 * Test parsing of import data from MusicBrainz server.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 07 Oct 2012
 *
 * Copyright (C) 2012-2018  Urs Fleisch
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

#pragma once

#include "testserverimporterbase.h"

/**
 * Test parsing of import data from MusicBrainz server.
 */
class TestMusicBrainzReleaseImportParser : public TestServerImporterBase {
  Q_OBJECT
private slots:
  void initTestCase();
  void testParseAlbums();
  void testParseTracks();
};
