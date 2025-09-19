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
#include "EquipmentOverview.h"
#include "EquipmentCalculator.h"
#include "EquipmentOverviewItems.h"

// for equipment-perspective.xml conversion to equipment-data.xml
enum class eqFormat : int { NOT_SET = 1,  JSON = 0, XML = 1 };
static eqFormat newFormat_ = eqFormat::NOT_SET;

EquipmentOverviewWindow::EquipmentOverviewWindow(Context* context, bool blank) :
    OverviewWindow(context, OverviewScope::EQUIPMENT, blank), reCalcOnVisible_(false), chartExportInProgress_(false)
{
    eqCalc_ = new EquipCalculator(context->mainWindow, this);

    space->setWhatsThis(help->getWhatsThisText(HelpWhatsThis::ChartEquip_Overview));
    space->configIcon = ":images/tile-edit.png";
    space->editIcon = ":images/equipment-popup.png";
    space->configChanged(CONFIG_APPEARANCE);

    textSize = new QComboBox(this);
    textSize->addItem(tr("Small"));
    textSize->addItem(tr("Medium"));
    textSize->addItem(tr("Large"));
    formlayout->addRow(new QLabel(tr("Text Size")), textSize);
    textSize->setCurrentIndex(eqTextSizeType::MEDIUM);

    showElevation = new QCheckBox(tr(""), this);
    showElevation->setCheckState(Qt::Unchecked);
    formlayout->addRow(new QLabel("Elevation Fields"), showElevation);

    showNotes = new QCheckBox(tr(""), this);
    showNotes->setCheckState(Qt::Unchecked);
    formlayout->addRow(new QLabel("Notes Fields"), showNotes);

    // cannot use athlete specific signals, as there is only one equipment view.
    connect(GlobalContext::context(), SIGNAL(configChanged(qint32)), this, SLOT(configChanged(qint32)));
    connect(context->mainWindow, SIGNAL(openingAthlete(QString, Context*)), this, SLOT(openingAthlete(QString, Context*)));

    connect(textSize, SIGNAL(currentIndexChanged(int)), this, SLOT(setTextSize(int)));
    connect(showElevation, SIGNAL(stateChanged(int)), this, SLOT(setShowElevation(int)));
    connect(showNotes, SIGNAL(stateChanged(int)), this, SLOT(setShowNotes(int)));
}

EquipmentOverviewWindow::~EquipmentOverviewWindow()
{
    delete eqCalc_;
}

void
EquipmentOverviewWindow::setTextSize(int value)
{
    textSize->setCurrentIndex(value);
}

void
EquipmentOverviewWindow::setShowElevation(int value)
{
    showElevation->setChecked(value);
}

void
EquipmentOverviewWindow::setShowNotes(int value)
{
    showNotes->setChecked(value);
}

void
EquipmentOverviewWindow::showChart(bool visible)
{
    reCalcOnVisible_ = visible;

    if (reCalcOnVisible_) eqCalc_->recalculateEquipSpace(space->allItems());

    GcChartWindow::showChart(visible);
}

ChartSpaceItem*
EquipmentOverviewWindow::addTile()
{
    ChartSpaceItem* item = OverviewWindow::addTile();

    if (item) eqCalc_->recalculateEquipTile(item);
    return item;
}

void
EquipmentOverviewWindow::cloneTile(ChartSpaceItem* item)
{
    switch (item->type) {

    case OverviewItemType::EQ_ITEM:
    {
        EquipmentItem* meta = reinterpret_cast<EquipmentItem*>(item);

        // clone the equipment item
        EquipmentItem* clonedItem = new EquipmentItem(meta->parent, meta->name + " clone", meta->eqLinkUseList_,
            meta->getNonGCDistance(), meta->getNonGCElevation(),
            meta->repDistance_, meta->repElevation_,
            meta->repDateSet_, meta->repDate_, meta->notes_);

        clonedItem->bgcolor = meta->bgcolor;
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
                                                            meta->getEqLinkName(), meta->showActivitiesPerAthlete_);

        clonedItem->bgcolor = meta->bgcolor;
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

        clonedItem->bgcolor = meta->bgcolor;
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

        clonedItem->bgcolor = meta->bgcolor;
        space->addItem(meta->order, meta->column, meta->span, meta->deep, clonedItem);

        // update geometry
        space->updateGeometry();
        space->updateView();
    }
    break;
    }
}

void
EquipmentOverviewWindow::configItem(ChartSpaceItem* item, QPoint pos)
{
    OverviewConfigDialog* p = new EquipmentOverviewConfigDialog(item, pos);
    p->exec(); // no mem leak as delete on close

    // Called for both item updates and deletion, if the item is still in the chart space list then
    // its an update, otherwise it's a tile deletion which requires no recaculation.
    if (space->allItems().indexOf(item) != -1) {

        // recalculate the affected tile
        eqCalc_->recalculateEquipTile(item);
    }
}

void
EquipmentOverviewWindow::configChanged(qint32 cfg)
{
    // Update in case the metric/imperial units have changed
    if (cfg & CONFIG_UNITS) {

        // need to update all the nonGCDistances in all the tiles before recalc
        for (ChartSpaceItem* item : space->allItems()) {

            if (item->type == OverviewItemType::EQ_ITEM) {
                dynamic_cast<EquipmentItem*>(item)->unitsChanged();
            }
        }
        if (reCalcOnVisible_) eqCalc_->recalculateEquipSpace(space->allItems());
    }

    if (cfg & CONFIG_APPEARANCE) {

        for (ChartSpaceItem* item : space->allItems()) {
            item->configChanged(CONFIG_APPEARANCE);
        }
    }
}

void
EquipmentOverviewWindow::getTileConfig(ChartSpaceItem* item, QString& config) const
{

    // now the actual card settings
    switch (item->type) {

    case OverviewItemType::EQ_ITEM:
    case OverviewItemType::EQ_SUMMARY:
    case OverviewItemType::EQ_HISTORY:
    case OverviewItemType::EQ_NOTES:
    {
        // Due to lazy loading of perspectives, save the units in case
        // they are changed before the equipment perspective is loaded.
        CommonEquipmentItem* meta = reinterpret_cast<CommonEquipmentItem*>(item);
        if (chartExportInProgress_) {
            // for special case of exported charts, provide a default equipment id.
            config += "\"equipmentRef\":\"" + QUuid().toString() + "\",";
        } else {
            config += "\"equipmentRef\":\"" + meta->equipmentRef_.toString(QUuid::WithoutBraces) + "\",";
        }
    }
    break;

    default:
    {
        OverviewWindow::getTileConfig(item, config);
    }
    break;
    }
}

void
EquipmentOverviewWindow::setTileConfig(const QJsonObject& obj, int type, const QString& name, const QString& datafilter,
                                       int order, int column, int span, int deep, ChartSpaceItem* add) const
{
    // the first call determines the json equipment file format to be parsed, this ensures content in the old
    // equipment-perspective.xml file is automatically converted/transfered to the new equipment-data.xml file.
    if (newFormat_ == eqFormat::NOT_SET) newFormat_ = obj.contains("equipmentRef") ? eqFormat::XML : eqFormat::JSON;

    QUuid equipmentRef;
    if (newFormat_ == eqFormat::XML) {

        equipmentRef = QUuid::fromString(obj["equipmentRef"].toString());

        // when charts are imported the tiles equipmentRef will be null, so as we
        // are creating new tiles we need to assign them a unique id.
        if (equipmentRef.isNull()) equipmentRef = QUuid::createUuid();
    }

    // now the actual card settings
    switch (type) {

    case OverviewItemType::EQ_ITEM:
    {
        if (newFormat_ == eqFormat::XML) {
            add = new EquipmentItem(space, name, equipmentRef);
            add->datafilter = datafilter;
            space->addItem(order, column, span, deep, add);

        } else {

            // Legacy code for conversion, will be removed in future release.

            bool savedAsMetric = (obj["metric"].toString() == "1") ? true : false;
            double nonGCDistance = obj["nonGCDistanceScaled"].toString().toULongLong()*0.0001;
            double nonGCElevation = obj["nonGCElevationScaled"].toString().toULongLong()*0.0001;
            double repDistance = obj["repDistance"].toString().toULongLong()*0.0001;
            double repElevation = obj["repElevation"].toString().toULongLong()*0.0001;
            bool repDateSet = (obj["repDateSet"].toString() == "1") ? true : false;
            QDate repDate;
            if (repDateSet) repDate = QDate::fromString(obj["repDate"].toString());

            // Due to lazy loading of perspectives, saved distances might need converting 
            // as the units might have changed before the equipment perspective is loaded.
            if (savedAsMetric && !GlobalContext::context()->useMetricUnits) {

                nonGCDistance = nonGCDistance * KM_PER_MILE;
                nonGCElevation = nonGCElevation * METERS_PER_FOOT;
                repDistance = repDistance * KM_PER_MILE;
                repElevation = repElevation * METERS_PER_FOOT;

            }
            if (!savedAsMetric && GlobalContext::context()->useMetricUnits) {

                nonGCDistance = nonGCDistance * MILES_PER_KM;
                nonGCElevation = nonGCElevation * FEET_PER_METER;
                repDistance = repDistance * MILES_PER_KM;
                repElevation = repElevation * FEET_PER_METER;
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

            add = new EquipmentItem(space, name, eqLinkUse, nonGCDistance, nonGCElevation,
                                    repDistance, repElevation, repDateSet, repDate, notes);
            add->datafilter = datafilter;
            space->addItem(order, column, span, deep, add);
        }
    }
    break;

    case OverviewItemType::EQ_SUMMARY:
    {
        if (newFormat_ == eqFormat::XML) {
            add = new EquipmentSummary(space, name, equipmentRef);
            add->datafilter = datafilter;
            space->addItem(order, column, span, deep, add);

        } else {

            QString eqLinkName = obj["eqLink"].toString();
            bool showActivitiesPerAthlete = (obj["showAthleteActivities"].toString() == "1") ? true : false;
            add = new EquipmentSummary(space, name, eqLinkName, showActivitiesPerAthlete);
            add->datafilter = datafilter;
            space->addItem(order, column, span, deep, add);
        }
    }
    break;

    case OverviewItemType::EQ_HISTORY:
    {
        if (newFormat_ == eqFormat::XML) {
            add = new EquipmentHistory(space, name, equipmentRef);
            add->datafilter = datafilter;
            space->addItem(order, column, span, deep, add);

        } else {

            bool sortMostRecentFirst = (obj["sortMostRecentFirst"].toString() == "1") ? true : false;

            QVector<EqHistoryEntry> eqHistory;
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
    }
    break;

    case OverviewItemType::EQ_NOTES:
    {
        if (newFormat_ == eqFormat::XML) {
            add = new EquipmentNotes(space, name, equipmentRef);
            add->datafilter = datafilter;
            space->addItem(order, column, span, deep, add);

        } else {
            QString notes = Utils::jsonunprotect(obj["notes"].toString());
            add = new EquipmentNotes(space, name, notes);
            add->datafilter = datafilter;
            space->addItem(order, column, span, deep, add);
        }
    }
    break;

    default:
    {
        OverviewWindow::setTileConfig(obj, type, name, datafilter, order, column, span, deep, add);
    }
    break;
    }
}

void
EquipmentOverviewWindow::openingAthlete(QString, Context* context)
{
    if (reCalcOnVisible_) {

        // If a new athlete is opened whilst the equipment window is displayed then this openingAthlete event
        // for the new athelete is too early, so need to wait for their activities to be loaded so register
        // the new athlete for loadDone temporarily and update then (if necessary)
        connect(context, SIGNAL(loadDone(QString, Context*)), this, SLOT(loadDone(QString, Context*)));
    }
}

void
EquipmentOverviewWindow::loadDone(QString, Context* context)
{
    // de-register the athlete's load event, and recalculate if currently visible.
    disconnect(context, SIGNAL(loadDone(QString, Context*)), this, SLOT(loadDone(QString, Context*)));
    if (reCalcOnVisible_) eqCalc_->recalculateEquipSpace(space->allItems());
}

void
EquipmentOverviewWindow::calculationComplete()
{
    // ensure the displayed tiles are up to date now calculation has finished
    foreach(ChartSpaceItem *item, space->allItems()) {
        item->update();
    }
}

EquipmentOverviewConfigDialog::EquipmentOverviewConfigDialog(ChartSpaceItem* item, QPoint pos)
    : OverviewConfigDialog(item, pos)
{
    setWhatsThis(help->getWhatsThisText(HelpWhatsThis::Chart_Overview_Config).arg(itemDetail.quick, itemDetail.description));
}

void
EquipmentOverviewWindow::writeXML(QTextStream &xmlOut) const
{
    xmlOut.setRealNumberNotation(QTextStream::FixedNotation);
    xmlOut.setRealNumberPrecision(EQ_DECIMAL_PRECISION);

    for (ChartSpaceItem* item : space->allItems()) {

        switch (item->type) {

        case OverviewItemType::EQ_ITEM:
        {
            EquipmentItem* meta = reinterpret_cast<EquipmentItem*>(item);

            xmlOut << "\t\t<equipmentitem>\n";
            xmlOut << "\t\t\t<eqreference>" << meta->equipmentRef_.toString(QUuid::WithoutBraces) << "</eqreference>    <!-- Do not modify -->\n";
            xmlOut << QString("\t\t\t<eqtilename>%1</eqtilename>    <!-- For reference -->\n").arg(Utils::xmlprotect(item->name));

            xmlOut << "\t\t\t<nongcdistance>" << meta->getNonGCDistance() << "</nongcdistance>\n";
            xmlOut << "\t\t\t<nongcelevation>" << meta->getNonGCElevation() << "</nongcelevation>\n";
            xmlOut << "\t\t\t<repdistance>" << meta->repDistance_ << "</repdistance>\n";
            xmlOut << "\t\t\t<repelevation>" << meta->repElevation_ << "</repelevation>\n";
            xmlOut << QString("\t\t\t<repdate>%1</repdate>\n").arg(meta->repDateSet_ ? Utils::xmlprotect(meta->repDate_.toString()) : "");

            for (const auto& eqUse : meta->eqLinkUseList_) {
                xmlOut << "\t\t\t<equipmentitemuse>\n";
                xmlOut << QString("\t\t\t\t<eqlink>%1</eqlink>\n").arg(Utils::xmlprotect(eqUse.eqLinkName()));
                xmlOut << QString("\t\t\t\t<startdate>%1</startdate>\n").arg(eqUse.startSet_ ? Utils::xmlprotect(eqUse.startDate_.toString()) : "");
                xmlOut << QString("\t\t\t\t<enddate>%1</enddate>\n").arg(eqUse.endSet_ ? Utils::xmlprotect(eqUse.endDate_.toString()) : "");
                xmlOut << "\t\t\t</equipmentitemuse>\n";
            }
            xmlOut << QString("\t\t\t<notes>%1</notes>\n").arg(Utils::xmlprotect(meta->notes_));
            xmlOut << "\t\t</equipmentitem>\n";
        } break;

        case OverviewItemType::EQ_SUMMARY:
        {
            EquipmentSummary* meta = reinterpret_cast<EquipmentSummary*>(item);

            xmlOut << "\t\t<equipmentsummary>\n";
            xmlOut << "\t\t\t<eqreference>" << meta->equipmentRef_.toString(QUuid::WithoutBraces) << "</eqreference>    <!-- Do not modify -->\n";
            xmlOut << QString("\t\t\t<eqtilename>%1</eqtilename>    <!-- For reference -->\n").arg(Utils::xmlprotect(item->name));
            xmlOut << QString("\t\t\t<eqlink>%1</eqlink>\n").arg(Utils::xmlprotect(meta->getEqLinkName()));
            xmlOut << QString("\t\t\t<showathleteactivities>%1</showathleteactivities>\n").arg(Utils::xmlprotect(meta->showActivitiesPerAthlete_ ? "true" : "false"));
            xmlOut << "\t\t</equipmentsummary>\n";
        }
        break;

        case OverviewItemType::EQ_HISTORY:
        {
            EquipmentHistory* meta = reinterpret_cast<EquipmentHistory*>(item);

            xmlOut << "\t\t<equipmenthistory>\n";
            xmlOut << "\t\t\t<eqreference>" << meta->equipmentRef_.toString(QUuid::WithoutBraces) << "</eqreference>    <!-- Do not modify -->\n";
            xmlOut << QString("\t\t\t<eqtilename>%1</eqtilename>    <!-- For reference -->\n").arg(Utils::xmlprotect(item->name));
            xmlOut << QString("\t\t\t<sortmostrecentfirst>%1</sortmostrecentfirst>\n").arg(Utils::xmlprotect(meta->sortMostRecentFirst_ ? "true" : "false"));

            for (const auto& eqHistory : meta->eqHistoryList_) {
                xmlOut << "\t\t\t<historyentry>\n";
                xmlOut << QString("\t\t\t\t<historydate>%1</historydate>\n").arg(Utils::xmlprotect(eqHistory.date_.toString()));
                xmlOut << QString("\t\t\t\t<historytext>%1</historytext>\n").arg(Utils::xmlprotect(eqHistory.text_));
                xmlOut << "\t\t\t</historyentry>\n";
            }
            xmlOut << "\t\t</equipmenthistory>\n";
        }
        break;

        case OverviewItemType::EQ_NOTES:
        {
            EquipmentNotes* meta = reinterpret_cast<EquipmentNotes*>(item);

            xmlOut << "\t\t<equipmentnotes>\n";
            xmlOut << "\t\t\t<eqreference>" << meta->equipmentRef_.toString(QUuid::WithoutBraces) << "</eqreference>    <!-- Do not modify -->\n";
            xmlOut << QString("\t\t\t<eqtilename>%1</eqtilename>    <!-- For reference -->\n").arg(Utils::xmlprotect(item->name));
            xmlOut << QString("\t\t\t<notes>%1</notes>\n").arg(Utils::xmlprotect(meta->notes_));
            xmlOut << "\t\t</equipmentnotes>\n";
        }
        break;

        default: {
            qDebug() << "Unsupported tile type:" << item->type << " in Equipment Overview Window xml writing";
        }
        break;
        }
    }
}

void
EquipmentOverviewWindow::saveChart()
{
    chartExportInProgress_ = true;

    // saving the chart calls getTileConfig() via the "config" property read, but
    // we must not export the unique tile ids, otherwise when the chart is imported
    // it leads to duplicate tiles, this allows getTileConfig() to know what's happening.
    GcChartWindow::saveChart();

    chartExportInProgress_ = false;
}

#ifdef GC_HAS_CLOUD_DB
void
EquipmentOverviewWindow::exportChartToCloudDB()
{
    chartExportInProgress_ = true;

    // saving the chart calls getTileConfig() via the "config" property read, but
    // we must not export the unique tile ids, otherwise when the chart is imported
    // it leads to duplicate tiles, this allows getTileConfig() to know what's happening.
    GcChartWindow::saveChart();

    chartExportInProgress_ = false;
}
#endif

void
EquipmentOverviewWindow::loadItemSpecificData()
{
    if (newFormat_ == eqFormat::XML) readXML(QDir(gcroot).absolutePath()+"/equipment-data.xml");
}

void
EquipmentOverviewWindow::readXML(const QString& filename)
{
    QFile metadataFile(filename);
    QXmlInputSource source( &metadataFile );
    QXmlSimpleReader xmlReader;
    EquipmentXMLParser handler(space->allItems());

    xmlReader.setContentHandler(&handler);
    xmlReader.setErrorHandler(&handler);
    xmlReader.parse( source );
}

EquipmentXMLParser::EquipmentXMLParser(const QList<ChartSpaceItem*>& allItems)
    :   allItems_(allItems), typeToLoad_(0), savedAsMetric_(false), itemToLoad_(nullptr),
        scalerKmMile_(1.0), scalerMtrFoot_(1.0)
{
}

// to see the format of the equipment xml file, look at the serialize()
// function above in this source file.
bool EquipmentXMLParser::endElement( const QString&, const QString&, const QString &qName)
{
    if (qName == "uom") {

        savedAsMetric_ = (Utils::unprotect(buffer) == "metric");

        // Due to lazy loading of perspectives, saved distances might need converting 
        // as the units might have changed before the equipment perspective is loaded.
        if (savedAsMetric_ && !GlobalContext::context()->useMetricUnits) {
            scalerKmMile_ = KM_PER_MILE; scalerMtrFoot_ = METERS_PER_FOOT;
        } else if (!savedAsMetric_ && GlobalContext::context()->useMetricUnits) {
            scalerKmMile_ = MILES_PER_KM; scalerMtrFoot_ = FEET_PER_METER;
        } else {
            scalerKmMile_ = scalerMtrFoot_ = 1.0;
        }
    }
    else if (qName == "eqreference") {

        itemToLoad_ = nullptr;
        QUuid eqXmlRef = QUuid::fromString(Utils::unprotect(buffer.trimmed()));

        // find the associated chart space item matching the xml info to be loaded.
        foreach(ChartSpaceItem *item, allItems_) {

            if (item->type == typeToLoad_) {

                switch (typeToLoad_) {

                case OverviewItemType::EQ_ITEM:
                case OverviewItemType::EQ_SUMMARY:
                case OverviewItemType::EQ_HISTORY:
                case OverviewItemType::EQ_NOTES: {
                    CommonEquipmentItem* meta = reinterpret_cast<CommonEquipmentItem*>(item);
                    if (eqXmlRef == meta->equipmentRef_) itemToLoad_ = item;
                } break;

                default: {
                    qDebug() << "Unsupported tile type:" << typeToLoad_ << " in Equipment XML Parser";
                } break;
                }
            }
        }
    }
    else if (itemToLoad_) {

        switch (typeToLoad_) {

        case OverviewItemType::EQ_ITEM: {
            EquipmentItem* eqItem = reinterpret_cast<EquipmentItem*>(itemToLoad_);

            if (qName == "nongcdistance") eqItem->setNonGCDistance(Utils::unprotect(buffer.trimmed()).toDouble() * scalerKmMile_);
            else if (qName == "nongcelevation") eqItem->setNonGCElevation(Utils::unprotect(buffer.trimmed()).toDouble() * scalerMtrFoot_);
            else if (qName == "repdistance") eqItem->repDistance_ = Utils::unprotect(buffer.trimmed()).toDouble() * scalerKmMile_;
            else if (qName == "repelevation") eqItem->repElevation_ = Utils::unprotect(buffer.trimmed()).toDouble() * scalerMtrFoot_;
            else if (qName == "repdate") {
                eqItem->repDateSet_ = (buffer.trimmed() != "");
                eqItem->repDate_ = eqItem->repDateSet_ ? QDate::fromString(Utils::unprotect((buffer.trimmed()))) : QDate();
            }
            else if (qName == "eqlink") windowToLoad_ = EqTimeWindow(Utils::unprotect(buffer.trimmed()));
            else if (qName == "startdate") {
                windowToLoad_.startSet_ = (buffer.trimmed() != "");
                windowToLoad_.startDate_ = windowToLoad_.startSet_ ? QDate::fromString(Utils::unprotect((buffer.trimmed()))) : QDate();
            }
            else if (qName == "enddate") {
                windowToLoad_.endSet_ = (buffer.trimmed() != "");
                windowToLoad_.endDate_ = windowToLoad_.endSet_ ? QDate::fromString(Utils::unprotect((buffer.trimmed()))) : QDate();
            }
            else if (qName == "equipmentitemuse") eqItem->eqLinkUseList_.push_back(windowToLoad_);
            else if (qName == "notes") eqItem->notes_ = Utils::unprotect(buffer.trimmed());
            else if (qName == "equipmentitem") eqItem->configwidget_->setWidgets();

        } break;

        case OverviewItemType::EQ_SUMMARY: {
            EquipmentSummary* eqSumm = reinterpret_cast<EquipmentSummary*>(itemToLoad_);

            if (qName == "eqlink") eqSumm->setEqLinkName(Utils::unprotect(buffer.trimmed()));
            else if (qName == "showathleteactivities") eqSumm->showActivitiesPerAthlete_ = (Utils::unprotect(buffer) == "true");
            else if (qName == "equipmentsummary") eqSumm->configwidget_->setWidgets();

        } break;

        case OverviewItemType::EQ_HISTORY:
        {
            EquipmentHistory* eqHist = reinterpret_cast<EquipmentHistory*>(itemToLoad_);

            if (qName == "sortmostrecentfirst") eqHist->sortMostRecentFirst_ = (Utils::unprotect(buffer) == "true");
            else if (qName == "historydate") eqHistoryEntry_.date_ = QDate::fromString(Utils::unprotect((buffer.trimmed())));
            else if (qName == "historytext") eqHistoryEntry_.text_ = Utils::unprotect(buffer.trimmed());
            else if (qName == "historyentry") eqHist->eqHistoryList_.push_back(eqHistoryEntry_);
            else if (qName == "equipmenthistory") eqHist->configwidget_->setWidgets();

        } break;

        case OverviewItemType::EQ_NOTES:
        {
            EquipmentNotes* eqNotes = reinterpret_cast<EquipmentNotes*>(itemToLoad_);

            if (qName == "notes") eqNotes->notes_ = Utils::unprotect(buffer.trimmed());
            else if (qName == "equipmentnotes") eqNotes->configwidget_->setWidgets();

        } break;

        default: {
            qDebug() << "Unsupported tile type:" << typeToLoad_ << " in Equipment XML Parser";
        } break;
        }
    }
    return true;
}

bool EquipmentXMLParser::startElement( const QString&, const QString&, const QString &name, const QXmlAttributes & )
{
    buffer.clear();

    if (name == "equipmentitem") {
        itemToLoad_ = nullptr;
        typeToLoad_ = OverviewItemType::EQ_ITEM;
    }
    else if (name == "equipmenthistory") {
        itemToLoad_ = nullptr;
        typeToLoad_ = OverviewItemType::EQ_HISTORY;
    }
    else if (name == "equipmentsummary") {
        itemToLoad_ = nullptr;
        typeToLoad_ = OverviewItemType::EQ_SUMMARY;
    }
    else if (name == "equipmentnotes") {
        itemToLoad_ = nullptr;
        typeToLoad_ = OverviewItemType::EQ_NOTES;
    }
    else if(name == "equipmentitemuse") windowToLoad_.reset();
    else if(name == "historyentry") eqHistoryEntry_.reset();

    return true;
}

bool EquipmentXMLParser::characters( const QString& str )
{
    buffer += str;
    return true;
}

bool EquipmentXMLParser::endDocument()
{
    return true;
}

