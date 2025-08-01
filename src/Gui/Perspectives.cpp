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

#include "Perspectives.h"
#include "MainWindow.h"
#include "Colors.h"
#include "Views.h"
#include "EquipmentView.h"
#include "EquipmentCalculator.h"
#include "ChartBar.h"
#include "DataFilter.h" // for Result

AnalysisPerspective::AnalysisPerspective(Context* context, const QString& title)
    : Perspective(context, context->mainWindow, title)
{
}

QString
AnalysisPerspective::viewsInternalName() const
{
    return AnalysisView::internalName;
}

ViewParser*
AnalysisPerspective::getViewParser(bool useDefault) const
{
    return new AnalysisViewParser(context, useDefault);
}

bool
AnalysisPerspective::relevant(RideItem* item) const
{
    if ((df == NULL) || (item == NULL)) return false;

    // validate
    Result ret = df->evaluate(item, NULL);
    return ret.number();
}

PlanPerspective::PlanPerspective(Context* context, const QString& title)
    : Perspective(context, context->mainWindow, title)
{
    // plan view we should select a library chart when a chart is selected.
    connect(context, &Context::presetSelected, this, &PlanPerspective::presetSelected);
}

QString
PlanPerspective::viewsInternalName() const
{
    return PlanView::internalName;
}

ViewParser*
PlanPerspective::getViewParser(bool useDefault) const
{
    return new PlanViewParser(context, useDefault);
}

TrendsPerspective::TrendsPerspective(Context* context, const QString& title)
    : Perspective(context, context->mainWindow, title)
{
    // trends view we should select a library chart when a chart is selected.
    connect(context, &Context::presetSelected, this, &TrendsPerspective::presetSelected);
}

QString
TrendsPerspective::viewsInternalName() const
{
    return TrendsView::internalName;
}

ViewParser*
TrendsPerspective::getViewParser(bool useDefault) const
{
    return new TrendsViewParser(context, useDefault);
}

bool
TrendsPerspective::setExpression(const QString& expr)
{
    if (Perspective::setExpression(expr)) {
        // notify charts that the filter changed
        // but only for trends views where it matters
        foreach(GcWindow * chart, charts)
            chart->notifyPerspectiveFilterChanged(expression_);
        return true;
    }
    return false;
}

TrainPerspective::TrainPerspective(Context* context, const QString& title)
    : Perspective(context, context->mainWindow, title)
{
    // Allow realtime controllers to scroll train view with steering movements
    connect(context, &Context::steerScroll, this, &TrainPerspective::steerScroll);
}

QString
TrainPerspective::viewsInternalName() const
{
    return TrainView::internalName;
}

QColor&
TrainPerspective::getBackgroundColor() const
{
    static QColor col = GColor(CTRAINPLOTBACKGROUND);
    return col;
}

ViewParser*
TrainPerspective::getViewParser(bool useDefault) const
{
    return new TrainViewParser(context, useDefault);
}

EquipmentPerspective::EquipmentPerspective(MainWindow *mainWindow, const QString& title)
    : Perspective(nullptr, mainWindow, title), mainWindow_(mainWindow)
{
    connect(GlobalContext::context(), &GlobalContext::eqRecalculationComplete, this, &EquipmentPerspective::eqRecalculationComplete);
}

QString
EquipmentPerspective::viewsInternalName() const
{
    return EquipmentView::internalName;

}

GcChartWindow* 
EquipmentPerspective::getNewGcWindow(GcWinID) const
{
    return GcWindowRegistry::newGcWindow(GcWinID::EquipmentOverview, mainWindow, nullptr);
}

ViewParser*
EquipmentPerspective::getViewParser(bool useDefault) const
{
    return new EquipmentViewParser(mainWindow_, useDefault);
}

void
EquipmentPerspective::showControls()
{
    mainWindow_->equipView()->chartsettings->adjustSize();
    mainWindow_->equipView()->chartsettings->show();
}

void
EquipmentPerspective::configChanged(qint32 cfg)
{
    Perspective::configChanged(cfg);

    eqRecalculationComplete();
}

void
EquipmentPerspective::eqRecalculationComplete()
{
    if (charts.count() == 0) return;

    const QVector<AbstractEqItem*>& itemWarnings = EquipmentCalculator::getInstance().itemWarnings();

    for (int idx = 0; idx < charts.count(); ++idx) {

        bool chartWarningActive = false;

        for (const AbstractEqItem* eqItem : itemWarnings) {

            if (charts[idx]->title() == eqItem->xmlChartName_) {
                chartWarningActive = true;
                break; // out of this loop, job done
            }
        }
        chartbar->setWarning(idx, chartWarningActive);
    }
}