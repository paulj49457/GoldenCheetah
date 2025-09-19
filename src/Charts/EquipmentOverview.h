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

#ifndef _GC_EquipmentOverviewWindow_h
#define _GC_EquipmentOverviewWindow_h 1

#include "Overview.h"
#include "ChartSpace.h"
#include "EquipmentOverviewItems.h"

static const uint64_t EQ_DECIMAL_PRECISION = 1;

namespace eqTextSizeType {
    enum eqTxtType { SMALL = 0, MEDIUM, LARGE };
};

class EquipmentOverviewWindow : public OverviewWindow
{
    Q_OBJECT
    G_OBJECT

    // the display properties are used by the layout manager to save and restore
    // the parameters, this is so we can have multiple windows open at once with
    // different settings managed by the user.
    Q_PROPERTY(int textSize READ isTextSize WRITE setTextSize USER true)
    Q_PROPERTY(int showElevation READ isShowElevation WRITE setShowElevation USER true)
    Q_PROPERTY(int showNotes READ isShowNotes WRITE setShowNotes USER true)

    public:

        EquipmentOverviewWindow(Context* context, bool blank = false);
        virtual ~EquipmentOverviewWindow();

        void showChart(bool visible) override;

        int isTextSize() const { return textSize->currentIndex(); }
        int isShowElevation() const { return showElevation->checkState(); }
        int isShowNotes() const { return showNotes->checkState(); }

    public slots:

        ChartSpaceItem* addTile() override;
        void configItem(ChartSpaceItem*, QPoint) override;
        void cloneTile(ChartSpaceItem*);
        void calculationComplete();

        // athlete opening
        void openingAthlete(QString, Context*);
        void loadDone(QString, Context*);

        void setTextSize(int);
        void setShowElevation(int);
        void setShowNotes(int);

        void configChanged(qint32 cfg);
        void writeXML(QTextStream &xmlOut) const;

        void saveChart() override;
#ifdef GC_HAS_CLOUD_DB
        void exportChartToCloudDB() override;
#endif

    protected:

        void readXML(const QString& filename);

        void loadItemSpecificData() override;
        QString getChartSource() const override { return ":charts/overview-equipment.gchart"; }
        virtual GcWindowTypes::gcwinid getWindowType() const override { return GcWindowTypes::EquipmentOverview; }
        AddTileWizard* getTileWizard(ChartSpaceItem* added) const override { return new AddTileWizard(context, space, OverviewScope::EQUIPMENT, added); }

        void getTileConfig(ChartSpaceItem* item, QString& config) const override;
        void setTileConfig(const QJsonObject& obj, int type, const QString& name,
                           const QString& datafilter, int order, int column,
                           int span, int deep, ChartSpaceItem* add) const override;

    private:
        bool reCalcOnVisible_;
        EquipCalculator* eqCalc_;
        bool chartExportInProgress_;
        QComboBox* textSize;
        QCheckBox* showElevation;
        QCheckBox* showNotes;
};

class EquipmentOverviewConfigDialog : public OverviewConfigDialog
{

    public:
        EquipmentOverviewConfigDialog(ChartSpaceItem* item, QPoint pos);

    protected:
        QString getViewForExport() const override { return "equipment"; }
        QString getTypeForExport() const override { return QString::number(GcWindowTypes::EquipmentOverview); }
};

class EqHistoryEntry;
class EqTimeWindow;

class EquipmentXMLParser : public QXmlDefaultHandler
{

    public:
        EquipmentXMLParser(const QList<ChartSpaceItem*>& allItems);

        bool startDocument() { return true; }
        bool endDocument();
        bool endElement( const QString&, const QString&, const QString &qName );
        bool startElement( const QString&, const QString&, const QString &name, const QXmlAttributes &attrs );
        bool characters( const QString& str );

    private:
        QString buffer;

        // whilst parsing elements are stored here
        int typeToLoad_;
        bool savedAsMetric_;
        double scalerKmMile_, scalerMtrFoot_;
        EqTimeWindow windowToLoad_;
        EqHistoryEntry eqHistoryEntry_;

        // the chart items to be loaded with the xml data
        ChartSpaceItem* itemToLoad_;
        const QList<ChartSpaceItem*>& allItems_;
};

#endif // _GC_EquipmentOverviewWindow_h
