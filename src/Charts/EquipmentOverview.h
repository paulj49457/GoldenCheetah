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

class EquipmentOverviewWindow : public OverviewWindow
{
    Q_OBJECT

    public:

        EquipmentOverviewWindow(Context* context, bool blank = false);
        virtual ~EquipmentOverviewWindow();

        void showChart(bool visible) override;

    public slots:

        ChartSpaceItem* addTile() override;
        void configItem(ChartSpaceItem*, QPoint) override;
        void cloneTile(ChartSpaceItem*);
        void calculationComplete();

        // athlete opening
        void openingAthlete(QString, Context*);
        void loadDone(QString, Context*);

        void configChanged(qint32 cfg);

    protected:

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

};

class EquipmentOverviewConfigDialog : public OverviewConfigDialog {

    public:
        EquipmentOverviewConfigDialog(ChartSpaceItem* item, QPoint pos);

    protected:
        QString getViewForExport() const override { return "equipment"; }
        QString getTypeForExport() const override { return QString::number(GcWindowTypes::EquipmentOverview); }
};

#endif // _GC_EquipmentOverviewWindow_h
