/*
 * Copyright (c) 2024 Paul Johnson (paulj49457@gmail.com)
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

#include "AthleteTab.h"
#include "OverviewEquipment.h"
#include "EquipmentCalculator.h"
#include "OverviewEquipmentItems.h"

OverviewEquipmentWindow::OverviewEquipmentWindow(Context* context, int scope, bool blank) :
    OverviewWindow(context, scope, blank), reCalcOnVisible_(false)
{
    eqCalc_ = new EquipCalculator(context->mainWindow, this);

    space->setWhatsThis(help->getWhatsThisText(HelpWhatsThis::ChartEquip_Overview));
    space->configIcon = ":images/tile-edit.png";
    space->editIcon = ":images/equipment-popup.png";
    space->configChanged(CONFIG_APPEARANCE);

    // cannot use athlete specific signals, as there is only one equipment view.
    connect(GlobalContext::context(), SIGNAL(configChanged(qint32)), this, SLOT(configChanged(qint32)));
    connect(context->mainWindow, SIGNAL(openingAthlete(QString, Context*)), this, SLOT(openingAthlete(QString, Context*)));
}

OverviewEquipmentWindow::~OverviewEquipmentWindow()
{
    delete eqCalc_;
}

void
OverviewEquipmentWindow::showChart(bool visible)
{
    reCalcOnVisible_ = visible;

    if (reCalcOnVisible_) eqCalc_->recalculateEquipSpace(space->allItems());

    GcChartWindow::showChart(reCalcOnVisible_);
}

ChartSpaceItem*
OverviewEquipmentWindow::addTile()
{
    ChartSpaceItem* item = OverviewWindow::addTile();

    if (item != nullptr) eqCalc_->recalculateEquipTile(item);
    return item;
}

void
OverviewEquipmentWindow::cloneTile(ChartSpaceItem* item)
{
    switch (item->type) {

    case OverviewItemType::EQ_ITEM:
    {
        EquipmentItem* meta = reinterpret_cast<EquipmentItem*>(item);

        // clone the equipment item
        EquipmentItem* clonedItem = new EquipmentItem(meta->parent, meta->name + " clone", meta->eqLinkUseList_,
            meta->getNonGCDistanceScaled(), meta->getNonGCElevationScaled(),
            meta->repDistanceScaled_, meta->repElevationScaled_,
            meta->repDateSet_, meta->repDate_, meta->notes_);

        space->addItem(meta->order, meta->column, meta->span, meta->deep, clonedItem);

        // update geometry
        space->updateGeometry();
        space->updateView();

        eqCalc_->recalculateEquipTile(clonedItem);
    }
    break;

    case OverviewItemType::EQ_SUMMARY:
    {
        EquipmentSummary* meta = reinterpret_cast<EquipmentSummary*>(item);

        // clone the equipment summary item
        EquipmentSummary* clonedItem = new EquipmentSummary(meta->parent, meta->name + " clone",
                                                            meta->eqLinkName_, meta->showActivitiesPerAthlete_);

        space->addItem(meta->order, meta->column, meta->span, meta->deep, clonedItem);

        // update geometry
        space->updateGeometry();
        space->updateView();

        eqCalc_->recalculateEquipTile(clonedItem);
    }
    break;

    case OverviewItemType::EQ_HISTORY:
    {
        EquipmentHistory* meta = reinterpret_cast<EquipmentHistory*>(item);

        // clone the equipment summary item
        EquipmentHistory* clonedItem = new EquipmentHistory(meta->parent, meta->name + " clone", meta->eqHistoryList_, meta->sortMostRecentFirst_);

        space->addItem(meta->order, meta->column, meta->span, meta->deep, clonedItem);

        // update geometry
        space->updateGeometry();
        space->updateView();
    }
    break;

    case OverviewItemType::EQ_NOTES:
    {
        EquipmentNotes* meta = reinterpret_cast<EquipmentNotes*>(item);

        // clone the equipment notes item
        EquipmentNotes* clonedItem = new EquipmentNotes(meta->parent, meta->name + " clone", meta->notes_);

        space->addItem(meta->order, meta->column, meta->span, meta->deep, clonedItem);

        // update geometry
        space->updateGeometry();
        space->updateView();
    }
    break;
    }
}

void
OverviewEquipmentWindow::configItem(ChartSpaceItem* item, QPoint pos)
{
    OverviewWindow::configItem(item, pos);

    // Called for both item updates and deletion, if the item is still in the chart space list then
    // its an update, otherwise it's a tile deletion which requires no recaculation.
    if (space->allItems().indexOf(item) != -1) {

        // recalculate the affected tile
        eqCalc_->recalculateEquipTile(item);
    }
}

void
OverviewEquipmentWindow::configChanged(qint32 cfg) {

    // Update in case the metric/imperial units have changed
    if (cfg & CONFIG_UNITS) {

        // need to update all the nonGCDistances in all the tiles before recalc
        for (ChartSpaceItem* item : space->allItems()) {

            if (item->type == OverviewItemType::EQ_ITEM)
            {
                static_cast<EquipmentItem*>(item)->unitsChanged();
            }
        }

        if (reCalcOnVisible_) eqCalc_->recalculateEquipSpace(space->allItems());
    }
}

QString
OverviewEquipmentWindow::getChartSource() const
{
    return ":charts/overview-equipment.gchart";
}

void
OverviewEquipmentWindow::getExtraConfiguration(ChartSpaceItem* item, QString& config) const
{
    // now the actual card settings
    switch (item->type) {

    case OverviewItemType::EQ_ITEM:
    {
        // Due to lazy loading of perspectives, save the units in case
        // they are changed before the equipment perspective is loaded.
        EquipmentItem* meta = reinterpret_cast<EquipmentItem*>(item);
        config += "\"metric\":\"" + QString(GlobalContext::context()->useMetricUnits ? "1" : "0") + "\",";
        config += "\"nonGCDistanceScaled\":\"" + QString("%1").arg(meta->getNonGCDistanceScaled()) + "\",";
        config += "\"nonGCElevationScaled\":\"" + QString("%1").arg(meta->getNonGCElevationScaled()) + "\",";
        config += "\"repDistance\":\"" + QString("%1").arg(meta->repDistanceScaled_) + "\",";
        config += "\"repElevation\":\"" + QString("%1").arg(meta->repElevationScaled_) + "\",";
        QString repDateSetStr = meta->repDateSet_ ? "1" : "0";
        config += "\"repDateSet\":\"" + repDateSetStr + "\",";
        config += "\"repDate\":\"" + meta->repDate_.toString() + "\",";

        QJsonArray EqLinkUses;
        QJsonDocument eqDoc;
        for (const auto& eqUse : meta->eqLinkUseList_) {

            QJsonObject EqLinkUse;
            EqLinkUse["eqLink"] = eqUse.eqLinkName_;
            EqLinkUse["startSet"] = eqUse.startSet_ ? "1" : "0";
            EqLinkUse["startDate"] = eqUse.startDate_.toString();
            EqLinkUse["endSet"] = eqUse.endSet_ ? "1" : "0";
            EqLinkUse["endDate"] = eqUse.endDate_.toString();

            EqLinkUses.push_back(EqLinkUse);
        }

        eqDoc.setArray(EqLinkUses);
        config += "\"eqUseList\":" + eqDoc.toJson(QJsonDocument::Compact) + ",";
        config += "\"notes\":\"" + Utils::jsonprotect(meta->notes_) + "\",";
    }
    break;

    case OverviewItemType::EQ_SUMMARY:
    {
        EquipmentSummary* meta = reinterpret_cast<EquipmentSummary*>(item);
        config += "\"eqLink\":\"" + meta->eqLinkName_ + "\",";
        config += "\"showAthleteActivities\":\"" + QString("%1").arg(meta->showActivitiesPerAthlete_ ? "1" : "0") + "\",";
    }
    break;

    case OverviewItemType::EQ_HISTORY:
    {
        EquipmentHistory* meta = reinterpret_cast<EquipmentHistory*>(item);

        config += "\"sortMostRecentFirst\":\"" + QString("%1").arg(meta->sortMostRecentFirst_ ? "1" : "0") + "\",";

        QJsonArray EquipHistory;
        QJsonDocument historyDoc;
        for (const auto& eqHistory : meta->eqHistoryList_) {

            QJsonObject historyRow;
            historyRow["historyDate"] = eqHistory.date_.toString();
            historyRow["historyText"] = eqHistory.text_;

            EquipHistory.push_back(historyRow);
        }

        historyDoc.setArray(EquipHistory);
        config += "\"historyList\":" + historyDoc.toJson(QJsonDocument::Compact) + ",";
    }
    break;

    case OverviewItemType::EQ_NOTES:
    {
        EquipmentNotes* meta = reinterpret_cast<EquipmentNotes*>(item);
        config += "\"notes\":\"" + Utils::jsonprotect(meta->notes_) + "\",";
    }
    break;
    }
}

void
OverviewEquipmentWindow::setExtraConfiguration(QJsonObject& obj, int type, ChartSpaceItem* add, QString& name,
                                                QString& datafilter, int order, int column, int span, int deep) const
{
    // now the actual card settings
    switch (type) {

    case OverviewItemType::EQ_ITEM:
    {
        bool savedAsMetric = (obj["metric"].toString() == "1") ? true : false;
        uint64_t nonGCDistanceScaled = obj["nonGCDistanceScaled"].toString().toULongLong();
        uint64_t nonGCElevationScaled = obj["nonGCElevationScaled"].toString().toULongLong();
        uint64_t repDistance = obj["repDistance"].toString().toULongLong();
        uint64_t repElevation = obj["repElevation"].toString().toULongLong();
        bool repDateSet = (obj["repDateSet"].toString() == "1") ? true : false;
        QDate repDate;
        if (repDateSet) repDate = QDate::fromString(obj["repDate"].toString());

        // Due to lazy loading of perspectives, saved distances might need converting 
        // as the units might have changed before the equipment perspective is loaded.
        if (savedAsMetric && !GlobalContext::context()->useMetricUnits) {

            nonGCDistanceScaled = round(nonGCDistanceScaled * KM_PER_MILE);
            nonGCElevationScaled = round(nonGCElevationScaled * METERS_PER_FOOT);
            repDistance = round(repDistance * KM_PER_MILE);
            repElevation = round(repElevation * METERS_PER_FOOT);

        }
        if (!savedAsMetric && GlobalContext::context()->useMetricUnits) {

            nonGCDistanceScaled = round(nonGCDistanceScaled * MILES_PER_KM);
            nonGCElevationScaled = round(nonGCElevationScaled * FEET_PER_METER);
            repDistance = round(repDistance * MILES_PER_KM);
            repElevation = round(repElevation * FEET_PER_METER);
        }

        QVector<EqTimeWindow> eqLinkUse;
        QJsonArray jsonArray = obj["eqUseList"].toArray();

        foreach(const QJsonValue & value, jsonArray) {

            QJsonObject objVals = value.toObject();

            QString eqLinkName = objVals["eqLink"].toString();
            bool startSet = (objVals["startSet"].toString() == "1") ? true : false;
            QDate startDate;
            if (startSet) startDate = QDate::fromString(objVals["startDate"].toString());
            bool endSet = (objVals["endSet"].toString() == "1") ? true : false;
            QDate endDate;
            if (endSet) endDate = QDate::fromString(objVals["endDate"].toString());

            eqLinkUse.push_back(EqTimeWindow(eqLinkName, startSet, startDate, endSet, endDate));
        }

        QString notes = Utils::jsonunprotect(obj["notes"].toString());

        add = new EquipmentItem(space, name, eqLinkUse, nonGCDistanceScaled, nonGCElevationScaled,
                                repDistance, repElevation, repDateSet, repDate, notes);
        add->datafilter = datafilter;
        space->addItem(order, column, span, deep, add);
    }
    break;

    case OverviewItemType::EQ_SUMMARY:
    {
        QString eqLinkName = obj["eqLink"].toString();
        bool showActivitiesPerAthlete = (obj["showAthleteActivities"].toString() == "1") ? true : false;
        add = new EquipmentSummary(space, name, eqLinkName, showActivitiesPerAthlete);
        add->datafilter = datafilter;
        space->addItem(order, column, span, deep, add);
    }
    break;

    case OverviewItemType::EQ_HISTORY:
    {
        bool sortMostRecentFirst = (obj["sortMostRecentFirst"].toString() == "1") ? true : false;

        QVector<EqHistoryEntry> eqHistory;

        // parse in the new EqTimeWindow format
        QJsonArray jsonArray = obj["historyList"].toArray();

        foreach(const QJsonValue & value, jsonArray) {

            QJsonObject objVals = value.toObject();

            QDate historyDate = QDate::fromString(objVals["historyDate"].toString());
            QString historyText = objVals["historyText"].toString();

            eqHistory.push_back(EqHistoryEntry(historyDate, historyText));
        }

        add = new EquipmentHistory(space, name, eqHistory, sortMostRecentFirst);
        add->datafilter = datafilter;
        space->addItem(order, column, span, deep, add);
    }
    break;

    case OverviewItemType::EQ_NOTES:
    {
        QString notes = Utils::jsonunprotect(obj["notes"].toString());
        add = new EquipmentNotes(space, name, notes);
        add->datafilter = datafilter;
        space->addItem(order, column, span, deep, add);
    }
    break;
    }
}

void
OverviewEquipmentWindow::openingAthlete(QString, Context* context)
{
    if (reCalcOnVisible_) {

        // If a new athlete is opened whilst the equipment window is displayed then this openingAthlete event
        // for the new athelete is too early, so need to wait for their activities to be loaded so register
        // the new athlete for loadDone temporarily and update then (if necessary)
        connect(context, SIGNAL(loadDone(QString, Context*)), this, SLOT(loadDone(QString, Context*)));
    }
}

void
OverviewEquipmentWindow::loadDone(QString, Context* context)
{
    // de-register the athlete's load event, and recalculate if currently visible.
    disconnect(context, SIGNAL(loadDone(QString, Context*)), this, SLOT(loadDone(QString, Context*)));
    if (reCalcOnVisible_) eqCalc_->recalculateEquipSpace(space->allItems());
}

void
OverviewEquipmentWindow::calculationComplete()
{
    // ensure the displayed tiles are up to date now calculation has finished
    foreach(ChartSpaceItem *item, space->allItems()) {
        item->update();
    }
}