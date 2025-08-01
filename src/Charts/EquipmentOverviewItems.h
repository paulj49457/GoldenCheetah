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

#ifndef _GC_EquipmentOverviewItem_h
#define _GC_EquipmentOverviewItem_h 1

#include <QMap>
#include <memory> // Atomics
#include "OverviewItems.h"

// to allow integral type atomics (c++11) to be used and to get them to hold values to 3 decimal places the following
// factors are used to scale the values, this is sufficient for equipment overview usage. When c++23 is available
// this can changed to use atomic<double> and the scaling can be removed.
#define EQ_REAL_TO_SCALED 10000
#define EQ_SCALED_TO_REAL 0.0001

class ColorButton;

class EqTimeWindow
{
    public:
        EqTimeWindow();
        EqTimeWindow(const QString& eqLinkName);
        EqTimeWindow(const QString& eqLinkName, bool startSet, const QDate& startDate, bool endSet, const QDate& endDate);

        QString eqLinkName() const { return eqLinkName_; }

        bool startSet_, endSet_;
        QDate startDate_, endDate_;

        void reset();
        bool isWithin(const QDate& actDate) const;
        bool rangeIsValid() const;
        bool eqLinkIsCompleterVal() const { return eqLinkIsCompleterVal_; }

    private:

        QString eqLinkName_;
        bool eqLinkIsCompleterVal_;
};

//
// Configuration widget for ALL Equipment Overview Items
//
class EquipmentOverviewItemConfig : public OverviewItemConfig
{
    Q_OBJECT

    public:

        EquipmentOverviewItemConfig(ChartSpaceItem *, Context*);
        virtual ~EquipmentOverviewItemConfig();

        static bool registerItems();

        // set the config widgets to reflect current config
        virtual void setWidgets() override;

    protected slots:

        // retrieve values when user edits them (if they're valid)
        virtual void dataChanged() override;

        void addEqLinkRow();
        void removeEqLinkRow();
        void addHistoryRow();
        void removeHistoryRow();
        void tableCellClicked(int row, int column);
        void repDateSetClicked();

    protected:

        // before show, lets make sure the background color and widgets are set correctly
        void showEvent(QShowEvent *) override { setWidgets(); }

    private:

        void setEqLinkRowWidgets(int tableRow, const EqTimeWindow* eqUse);
        void setEqHistoryEntryRowWidgets(int tableRow);

        // Equipment items
        QLineEdit *eqLinkName;
        QCompleter* eqLinkCompleter; // EquipmentLink completer
        QLineEdit *nonGCDistance, *nonGCElevation;
        QLineEdit *replaceDistance, *replaceElevation;
        QPushButton* replaceDateSet;
        QDateEdit* replaceDate;
        QCheckBox *eqCheckBox;
        QPlainTextEdit *notes;
        QTableWidget *eqTimeWindows;

};

class CommonEquipmentItem : public ChartSpaceItem
{
    Q_OBJECT

    public:

        virtual ~CommonEquipmentItem() {}

        void configChanged(qint32) override;

        // The following don't apply to most Equipment Items.
        void setData(RideItem*) override {}
        void setDateRange(DateRange) override {}
        void itemGeometryChanged() override {}
        void itemPaint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*) override {}

        virtual void resetForRecalc() {}
        void showEvent(QShowEvent* event) override;
        void displayTileEditMenu(const QPoint& pos) override;

        QWidget* config() override { return configwidget_; }

        QUuid equipmentRef_;
        double tileDisplayHeight_;
        EquipmentOverviewItemConfig* configwidget_ = nullptr;

    protected slots:

        void popupAction(QAction*);

    protected:

        QFont eqbigfont, eqmidfont, eqsmallfont;
        QColor inactiveColor_, textColor_, alertColor_;

        // Hide constructor to create an Abstract class
        CommonEquipmentItem(ChartSpace* parent, const QString& name, const QUuid& equipmentRef);

        QMap<int, QString> scrollableDisplayText_;
        int setupScrollableText(const QFontMetrics& fm, const QString& tileText, QMap<int, QString>& rowTextMap,
                                int rowOffset = 0, int protectOffset = -1);

};

class EquipmentItem : public CommonEquipmentItem
{
    Q_OBJECT

    public:

        // create using existing Uuid
        EquipmentItem(ChartSpace* parent, const QString& name, const QUuid& equipmentRef);

        // create using new Uuid
        EquipmentItem(ChartSpace *parent, const QString& name, QVector<EqTimeWindow>& eqLinkUse,
                        const uint64_t nonGCDistanceScaled, const uint64_t nonGCElevationScaled,
                        const uint64_t repDistanceScaled, const uint64_t repElevationScaled,
                        const bool repDateSet, const QDate& repDate, const QString& notes);

        virtual ~EquipmentItem() {}

        void itemPaint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) override;
        void configChanged(qint32) override;
        void itemGeometryChanged() override;
        void setData(RideItem*) override;

        bool isWithin(const QDate& actDate) const;
        bool isWithin(const QStringList& rideEqLinkNameList, const QDate& actDate) const;
        bool rangeIsValid() const;
        bool allEqLinkNamesCompleterVals() const;

        void resetForRecalc() override;
        void unitsChanged();
        void addActivity(uint64_t rideDistanceScaled, uint64_t rideElevationScaled, uint64_t rideTimeInSecs);
                        uint64_t getNumActivities() const { return activities_; }

        void setNonGCDistance(double nonGCDistance);
        void setNonGCDistanceScaled(uint64_t nonGCDistanceScaled);
        uint64_t getNonGCDistanceScaled() const { return nonGCDistanceScaled_; }
        uint64_t getGCDistanceScaled() const { return gcDistanceScaled_; }
        uint64_t getTotalDistanceScaled() const { return totalDistanceScaled_; }

        void setNonGCElevation(double nonGCElevation);
        void setNonGCElevationScaled(uint64_t nonGCElevationScaled);
        uint64_t getNonGCElevationScaled() const { return nonGCElevationScaled_; }
        uint64_t getGCElevationScaled() const { return gcElevationScaled_; }
        uint64_t getTotalElevationScaled() const { return totalElevationScaled_; }

        void sortEquipmentWindows();

        // create and config using new Uuid
        static ChartSpaceItem* create(ChartSpace* parent) {
            return new EquipmentItem(parent, tr("Equipment Item"), QUuid::createUuid()); }

        QVector<EqTimeWindow> eqLinkUseList_;
        bool repDateSet_;
        QDate repDate_;
        uint64_t repDistanceScaled_, repElevationScaled_;
        QString notes_;

    private:

        std::atomic<uint64_t> activities_;
        std::atomic<uint64_t> activityTimeInSecs_;

        uint64_t nonGCDistanceScaled_;
        std::atomic<uint64_t> gcDistanceScaled_;
        std::atomic<uint64_t> totalDistanceScaled_;

        uint64_t nonGCElevationScaled_;
        std::atomic<uint64_t> gcElevationScaled_;
        std::atomic<uint64_t> totalElevationScaled_;
};

class EquipmentSummary : public CommonEquipmentItem
{
    Q_OBJECT

    public:

        // create using existing Uuid
        EquipmentSummary(ChartSpace* parent, const QString& name, const QUuid& equipmentRef);

        // create using new Uuid
        EquipmentSummary(ChartSpace* parent, const QString& name, const QString& eqLinkName, bool showActivitiesPerAthlete);

        virtual ~EquipmentSummary() {}

        void itemPaint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) override;

        void resetForRecalc() override;
        void addActivity(const QString& athleteName, const QDate& activityDate, const uint64_t rideDistanceScaled,
                            const uint64_t eqElevationScaled, const uint64_t rideTimeInSecs);

        // create and config using new Uuid
        static ChartSpaceItem* create(ChartSpace* parent) {
            return new EquipmentSummary(parent, tr("Summary Item"), QUuid::createUuid()); }

        void setEqLinkName(const QString& eqLinkName);
        QString getEqLinkName() const { return eqLinkName_; }

        bool showActivitiesPerAthlete_;

    private:

        QString eqLinkName_;
        bool eqLinkIsCompleterVal_;

        std::atomic<uint64_t> eqLinkTotalTimeInSecs_;
        std::atomic<uint64_t> eqLinkTotalDistanceScaled_;
        std::atomic<uint64_t> eqLinkTotalElevationScaled_;
        std::atomic<uint64_t> eqLinkNumActivities_;

        QMutex activityMutex_;
        QDate eqLinkEarliestDate_, eqLinkLatestDate_;
        QMap<QString, uint32_t> athleteActivityMap_;
};

class EqHistoryEntry
{
public:
    EqHistoryEntry();
    EqHistoryEntry(const QDate& date, const QString& text);

    void reset();

    QDate date_;
    QString text_;
};

class EquipmentHistory : public CommonEquipmentItem
{
    Q_OBJECT

    public:

        // create using existing Uuid
        EquipmentHistory(ChartSpace* parent, const QString& name, const QUuid& equipmentRef);

        // create using new Uuid
        EquipmentHistory(ChartSpace* parent, const QString& name, const QVector<EqHistoryEntry>& eqHistory, const bool sortMostRecentFirst);

        virtual ~EquipmentHistory() {}

        void itemPaint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) override;
        void itemGeometryChanged() override;
        void wheelEvent(QGraphicsSceneWheelEvent* event) override;

        void sortHistoryEntries();

        // create and config using new Uuid
        static ChartSpaceItem* create(ChartSpace* parent) {
            return new EquipmentHistory(parent, tr("History Item"), QUuid::createUuid()); }

        bool sortMostRecentFirst_;
        QVector<EqHistoryEntry> eqHistoryList_;

    private:

        int scrollPosn_;
        VScrollBar* scrollbar_;
};

class EquipmentNotes : public CommonEquipmentItem
{
    Q_OBJECT

    public:

        // create using existing Uuid
        EquipmentNotes(ChartSpace* parent, const QString& name, const QUuid& equipmentRef);

        // create using new Uuid
        EquipmentNotes(ChartSpace* parent, const QString& name, const QString& notes);

        virtual ~EquipmentNotes() {}

        void itemPaint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) override;
        void itemGeometryChanged() override;
        void wheelEvent(QGraphicsSceneWheelEvent* event) override;

        // create and config using new Uuid
        static ChartSpaceItem* create(ChartSpace* parent) {
            return new EquipmentNotes(parent, tr("Notes Item"), QUuid::createUuid()); }

        QString notes_;

    private:

        int scrollPosn_;
        VScrollBar* scrollbar_;

};

#endif // _GC_EquipmentOverviewItem_h
