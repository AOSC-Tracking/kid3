/**
 * \file dirlist.cpp
 * List of directories to operate on.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 5 Jul 2005
 */

#include "dirlist.h"
#include <qfileinfo.h>
#include <qdir.h>
#include <qstringlist.h>

/**
 * Constructor.
 */
DirList::DirList(QWidget* parent, const char* name, WFlags f) :
	QListBox(parent, name, f) {}

/**
 * Destructor.
 */
DirList::~DirList() {}

/**
 * Fill the dirlist with the directories found in a directory.
 *
 * @param name path of directory
 * @return false if name is not directory path, else true.
 */
bool DirList::readDir(const QString& name) {
	QFileInfo file(name);
	if(file.isDir()) {
		clear();
		m_dirname = name;
		QDir dir(file.filePath());
		insertStringList(dir.entryList(QDir::Dirs | QDir::Drives));
		return true;
	}
	return false;
}
