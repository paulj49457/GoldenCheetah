/*
 * EquipmentView Copyright (c) 2025 Paul Johnson (paulj49457@gmail.com)
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
#include "Perspectives.h"
#include "MainWindow.h" // for gcroot

Perspective*
EquipmentViewParser::getViewParsersPerspective(const QString& name) const
{
    return new EquipmentPerspective(mainWindow_, name);
}

EquipmentView::EquipmentView(MainWindow *mainWindow, QStackedWidget *controls) :
    AbstractView(mainWindow, internalName, tr("Equipment Management"))
{
    // remove splitter signal registered by AbstractView as its not relevant to the equipment view
    disconnect(splitter, SIGNAL(splitterMoved(int,int)), this, SLOT(splitterMoved(int,int)));

    // perspectives are stacked
    pstack = new QStackedWidget(this);

    // startof - implement a cut down version of AbstractView::setPages(pstack);

    // add to mainSplitter now reset the splitter
    mainSplitter->insertWidget(-1, pstack); // pages);
    mainSplitter->setStretchFactor(0,0);
    mainSplitter->setCollapsible(0, false);
    splitter->insertWidget(-1, mainSplitter);

    // prevent the pages being collapsed by the splitter
    int index = splitter->indexOf(mainSplitter);
    splitter->setCollapsible(index, false);

    // endof - implement a cut down version of AbstractView::setPages(pstack);

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

EquipmentView::~EquipmentView()
{
    // No sidebar to delete
}

Perspective*
EquipmentView::getViewsPerspective(const QString& name) const
{
    return new EquipmentPerspective(mainWindow, name);
}

ViewParser*
EquipmentView::getViewParser(bool useDefault) const
{
    return new EquipmentViewParser(mainWindow, useDefault);
}

void
EquipmentView::selectionChanged()
{
    // selects the equipment tab
    if (isSelected()) perspective_->tabSelected(perspective_->currentTab());
}

bool
EquipmentView::isBlank()
{
    return true;
}

void
EquipmentView::addChart(GcWinID id)
{
    AbstractView::addChart(id);

    // need to recalculate the equipment cache after a user creates a new Equipment Overview chart.
    GlobalContext::context()->requestEqRecalculation("EquipmentOverviewWindow created");
}

