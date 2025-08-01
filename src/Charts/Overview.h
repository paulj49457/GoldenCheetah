/*
 * Copyright (c) 2020 Mark Liversedge (liversedge@gmail.com)
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

#ifndef _GC_OverviewWindow_h
#define _GC_OverviewWindow_h 1

// basics
#include "GoldenCheetah.h"
#include "Settings.h"
#include "Units.h"
#include "Colors.h"
#include "Context.h"
#include "Athlete.h"
#include "RideItem.h"
#include "RideMetric.h"
#include "HrZones.h"
#include <QSpinBox>

#include "ChartSpace.h"
#include "OverviewItems.h"
#include "AddTileWizard.h"
#include "HelpWhatsThis.h"

class OverviewWindow : public GcChartWindow
{
    Q_OBJECT

    Q_PROPERTY(QString config READ getConfiguration WRITE setConfiguration USER true)
    Q_PROPERTY(int minimumColumns READ minimumColumns WRITE setMinimumColumns USER true)

    public:

        // used by children
        Context *context;

    public slots:

        // get/set config
        QString getConfiguration() const;
        void setConfiguration(QString x);

        int minimumColumns() const { return mincolsEdit->value(); }
        void setMinimumColumns(int x) { if (x>0 && x< 11) {mincolsEdit->setValue(x); space->setMinimumColumns(x); }}

        // add a tile to the window
        virtual ChartSpaceItem* addTile();
        void importChart();
        void settings();

        // config item requested
        virtual void configItem(ChartSpaceItem *item, QPoint pos) = 0;

        const ChartSpace* getSpace() const { return space; };

    protected:

        // Hide constructor to create an Abstract class
        OverviewWindow(Context* context, int scope, bool blank);

        // Support optional derived window behaviour
        virtual void tileAddedNotication(ChartSpaceItem* /* added */) {}
        virtual void importChartNotification(ChartSpaceItem* /* add */) {}

        // Support derived window behaviour
        virtual void loadItemSpecificData() {};
        virtual QString getChartSource() const = 0;
        virtual GcWindowTypes::gcwinid getWindowType() const = 0;
        virtual AddTileWizard* getTileWizard(ChartSpaceItem* added) const = 0;

        // Support additional tile types in derived classes for get/set tile config.
        virtual void getTileConfig(ChartSpaceItem* item, QString& config) const;
        virtual void setTileConfig(const QJsonObject& obj, int type, const QString& name,
                                   const QString& datafilter, int order, int column,
                                   int span, int deep, ChartSpaceItem* add) const;

        HelpWhatsThis* help;
        ChartSpace* space;
        QFormLayout *formlayout;

    private:

        // gui setup
        bool configured;
        bool blank;

        QSpinBox *mincolsEdit;
};

class AnalysisOverviewWindow : public OverviewWindow {

    public:
        AnalysisOverviewWindow(Context* context, bool blank = false);

        void configItem(ChartSpaceItem* item, QPoint pos) override;

    protected:
        void tileAddedNotication(ChartSpaceItem* added) override { if (added->parent->currentRideItem) added->setData(added->parent->currentRideItem); }
        void importChartNotification(ChartSpaceItem* add) override { if (space->currentRideItem) add->setData(space->currentRideItem); }
        QString getChartSource() const override { return ":charts/overview-analysis.gchart"; }
        virtual GcWindowTypes::gcwinid getWindowType() const override { return GcWindowTypes::UserAnalysis; }
        AddTileWizard* getTileWizard(ChartSpaceItem* added) const override { return new AddTileWizard(context, space, OverviewScope::ANALYSIS, added); }
};

class TrendsOverviewWindow : public OverviewWindow {

    public:
        TrendsOverviewWindow(Context* context, bool blank = false);

        void configItem(ChartSpaceItem* item, QPoint pos) override;

    protected:
        void tileAddedNotication(ChartSpaceItem* added) override { added->setDateRange(added->parent->currentDateRange); }
        void importChartNotification(ChartSpaceItem* add) override { add->setDateRange(space->currentDateRange); }
        QString getChartSource() const override { return ":charts/overview-trends.gchart"; }
        virtual GcWindowTypes::gcwinid getWindowType() const override { return GcWindowTypes::UserTrends; }
        AddTileWizard* getTileWizard(ChartSpaceItem* added) const override { return new AddTileWizard(context, space, OverviewScope::TRENDS, added); }
};


class OverviewConfigDialog : public QDialog
{
    Q_OBJECT

    public:

        ~OverviewConfigDialog();

    public slots:

        void removeItem();
        void exportUserChart();
        void close();

    protected:

        // Hide constructor to create an Abstract class
        OverviewConfigDialog(ChartSpaceItem*, QPoint pos);

        void showEvent(QShowEvent*) override;

        // Support optional derived config dialog behaviour
        virtual void updateItemNotification() {};

        // Support derived config dialog behaviour
        virtual QString getViewForExport() const = 0;
        virtual QString getTypeForExport() const = 0;

        HelpWhatsThis* help;
        ChartSpaceItemDetail itemDetail;
        ChartSpaceItem* item;

    private:
        QPoint pos;
        QVBoxLayout *main;
        QPushButton *remove, *ok, *exp;
};

class AnalysisOverviewConfigDialog : public OverviewConfigDialog {

    public:
        AnalysisOverviewConfigDialog(ChartSpaceItem* item, QPoint pos);

    protected:
        void updateItemNotification() override { if (item->parent->currentRideItem) item->setData(item->parent->currentRideItem); }
        QString getViewForExport() const override { return "analysis"; }
        QString getTypeForExport() const override { return QString::number(GcWindowTypes::UserAnalysis); }
};

class TrendsOverviewConfigDialog : public OverviewConfigDialog {

    public:
        TrendsOverviewConfigDialog(ChartSpaceItem* item, QPoint pos);

    protected:
        void updateItemNotification() override { item->setDateRange(item->parent->currentDateRange); }
        QString getViewForExport() const override { return "home"; }
        QString getTypeForExport() const override { return QString::number(GcWindowTypes::UserTrends); }
};

#endif // _GC_OverviewWindow_h
