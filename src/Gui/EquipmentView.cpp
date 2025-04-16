/*
 * Copyright (c) 2025 Paul Johnson (paulj49457@gmail.com)
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc., 51
 * Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "EquipmentView.h"

EquipView::EquipView(Context *context, QStackedWidget *controls) :
        AbstractView(context, VIEW_EQUIPMENT, "equipment", "Equipment Management")
{
    // perspectives are stacked
    pstack = new QStackedWidget(this);
    setPages(pstack);

    setSidebarEnabled(false);

    // each perspective has a stack of controls
    cstack = new QStackedWidget(this);
    controls->addWidget(cstack);
    controls->setCurrentIndex(0);

    // the dialog box for the chart settings
    chartsettings = new ChartSettings(this, controls);
    chartsettings->setFixedWidth(650);
    chartsettings->setFixedHeight(600);
    chartsettings->hide();

    // load the default single hidden perspective
    restoreState(false);
    loaded = true;
    perspectiveSelected(0);
}

EquipView::~EquipView()
{
    // No sidebar to delete
}

void
EquipView::setSelected(bool selected)
{
    _selected = selected;

    // selects the equipment tab which invokes the tile calculation.
    if (selected) perspective_->tabSelected(perspective_->currentTab());
}

bool
EquipView::isBlank()
{
    return true;
}
