/**
 * \file stringlistedit.cpp
 * Widget to edit a string list.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 14 Apr 2007
 *
 * Copyright (C) 2007  Urs Fleisch
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

#include "stringlistedit.h"
#include <QPushButton>
#include <QLineEdit>
#include <QInputDialog>
#include <QLayout>
#include <QListWidget>

/**
 * Constructor.
 *
 * @param parent parent widget
 */
StringListEdit::StringListEdit(QWidget* parent) :
	QWidget(parent)
{
	QHBoxLayout* hlayout = new QHBoxLayout(this);
	m_stringListBox = new QListWidget(this);
	if (hlayout && m_stringListBox) {
		hlayout->setSpacing(6);
		hlayout->addWidget(m_stringListBox);
		QVBoxLayout* vlayout = new QVBoxLayout;
		m_addPushButton = new QPushButton(i18n("&Add..."), this);
		m_moveUpPushButton = new QPushButton(i18n("Move &Up"), this);
		m_moveDownPushButton = new QPushButton(i18n("Move &Down"), this);
		m_editPushButton = new QPushButton(i18n("&Edit..."), this);
		m_removePushButton = new QPushButton(i18n("&Remove"), this);
		if (vlayout && m_addPushButton && m_moveUpPushButton &&
				m_moveDownPushButton && m_editPushButton && m_removePushButton) {
			vlayout->addWidget(m_addPushButton);
			vlayout->addWidget(m_moveUpPushButton);
			vlayout->addWidget(m_moveDownPushButton);
			vlayout->addWidget(m_editPushButton);
			vlayout->addWidget(m_removePushButton);
			vlayout->addStretch();

			connect(m_addPushButton, SIGNAL(clicked()), this, SLOT(addItem()));
			connect(m_moveUpPushButton, SIGNAL(clicked()), this, SLOT(moveUpItem()));
			connect(m_moveDownPushButton, SIGNAL(clicked()), this, SLOT(moveDownItem()));
			connect(m_editPushButton, SIGNAL(clicked()), this, SLOT(editItem()));
			connect(m_removePushButton, SIGNAL(clicked()), this, SLOT(removeItem()));
			connect(m_stringListBox, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)), this, SLOT(setButtonEnableState()));
			connect(m_stringListBox, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(editItem()));

			setButtonEnableState();
			hlayout->addLayout(vlayout);
		}
	}
}

/**
 * Destructor.
 */
StringListEdit::~StringListEdit()
{
}

/**
 * Set the string list in the list box.
 *
 * @param strList string list
 */
void StringListEdit::setStrings(const QStringList& strList)
{
	m_stringListBox->clear();
	m_stringListBox->addItems(strList);
}

/**
 * Store the string list from the list box.
 *
 * @param strList the string list is stored here
 */
void StringListEdit::getStrings(QStringList& strList) const
{
	strList.clear();
	for (int i = 0; i < m_stringListBox->count(); ++i) {
		strList.append(m_stringListBox->item(i)->text());
	}
}

/**
 * Add a new item.
 */
void StringListEdit::addItem()
{
	bool ok;
	QString txt = QInputDialog::getText(
		this, i18n("Add Item"), QString::null, QLineEdit::Normal,
		QString::null, &ok);
	if (ok && !txt.isEmpty()) {
		m_stringListBox->addItem(txt);
	}
}

/**
 * Remove the selected item.
 */
void StringListEdit::removeItem()
{
	int idx = m_stringListBox->currentRow();
	QListWidgetItem* lwi = m_stringListBox->item(idx);
	if (idx >= 0 && lwi) {
		delete m_stringListBox->takeItem(idx);
		if (idx < static_cast<int>(m_stringListBox->count())) {
			m_stringListBox->setCurrentRow(idx);
		} else if (idx > 0 && idx - 1 < static_cast<int>(m_stringListBox->count())) {
			m_stringListBox->setCurrentRow(idx - 1);
		}
		setButtonEnableState();
	}
}

/**
 * Edit the selected item.
 */
void StringListEdit::editItem()
{
	QListWidgetItem* lwi = m_stringListBox->currentItem();
	if (lwi) {
		bool ok;
		QString txt = QInputDialog::getText(
			this, i18n("Edit Item"), QString::null, QLineEdit::Normal,
			lwi->text(), &ok);
		if (ok && !txt.isEmpty()) {
			lwi->setText(txt);
		}
	}
}

/**
 * Move the selected item up.
 */
void StringListEdit::moveUpItem()
{
	int idx = m_stringListBox->currentRow();
	QListWidgetItem* lwi = m_stringListBox->item(idx);
	if (idx > 0 && lwi) {
		m_stringListBox->insertItem(idx - 1, m_stringListBox->takeItem(idx));
		m_stringListBox->clearSelection();
		m_stringListBox->setCurrentRow(idx - 1);
	}
}

/**
 * Move the selected item down.
 */
void StringListEdit::moveDownItem()
{
	int idx = m_stringListBox->currentRow();
	QListWidgetItem* lwi = m_stringListBox->item(idx);
	if (idx >= 0 && idx < static_cast<int>(m_stringListBox->count()) - 1 &&
			lwi) {
		m_stringListBox->insertItem(idx + 1, m_stringListBox->takeItem(idx));
		m_stringListBox->clearSelection();
		m_stringListBox->setCurrentRow(idx + 1);
	}
}

/**
 * Change state of buttons according to the current item and the count.
 */
void StringListEdit::setButtonEnableState()
{
	int idx = m_stringListBox->currentRow();
	QListWidgetItem* lwi = m_stringListBox->item(idx);
	if (!lwi) idx = -1;
	m_moveUpPushButton->setEnabled(idx > 0);
	m_moveDownPushButton->setEnabled(
		idx >= 0 &&
		idx < static_cast<int>(m_stringListBox->count()) - 1);
	m_editPushButton->setEnabled(idx >= 0);
	m_removePushButton->setEnabled(idx >= 0);
}
