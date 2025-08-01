/*
 * Copyright (c) 2013 Mark Liversedge (liversedge@gmail.com)
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

#ifndef _GC_Views_h
#define _GC_Views_h 1

#include "AbstractView.h"
class TrainSidebar;
class AnalysisSidebar;
class IntervalSidebar;
class QDialog;
class RideNavigator;
class TrainBottom;


class AnalysisViewParser : public ViewParser {

    public:
        AnalysisViewParser(Context* context, bool useDefault) : ViewParser(context, useDefault) {}

    protected:
        Perspective* getViewParsersPerspective(const QString& name) const override;
};

class AnalysisView : public AbstractView
{
    Q_OBJECT

    public:

        AnalysisView(Context *context, QStackedWidget *controls);
        ~AnalysisView();
        void close() override;
        void setRide(RideItem*ride) override;
        void addIntervals();

        int viewType() const override { return VIEW_ANALYSIS; }

        RideNavigator *rideNavigator();
        AnalysisSidebar *analSidebar;

    public slots:

        bool isBlank() override;
        void compareChanged(bool);

    protected:

        Perspective* getViewsPerspective(const QString& name) const override;
        ViewParser* getViewParser(Context* context, bool useDefault) const override;

        void notifyViewStateRestored() override;
        void notifyViewSidebarChanged() override;
        void setViewSpecificPerspective() override;
        void notifyViewSplitterMoved() override;

};

class DiaryViewParser : public ViewParser {

    public:
        DiaryViewParser(Context* context, bool useDefault) : ViewParser(context, useDefault) {}

    protected:
        Perspective* getViewParsersPerspective(const QString& name) const override;
};

class DiarySidebar;
class DiaryView : public AbstractView
{
    Q_OBJECT

    public:

        DiaryView(Context *context, QStackedWidget *controls);
        ~DiaryView();
        void setRide(RideItem*ride) override;

        int viewType() const override { return VIEW_DIARY; }

    public slots:

        bool isBlank() override;
        void dateRangeChanged(DateRange);

    protected:

        Perspective* getViewsPerspective(const QString& name) const override;
        ViewParser* getViewParser(Context* context, bool useDefault) const override;

    private:
        DiarySidebar *diarySidebar;

};

class TrainViewParser : public ViewParser {

    public:
        TrainViewParser(Context* context, bool useDefault) : ViewParser(context, useDefault) {}

    protected:
        Perspective* getViewParsersPerspective(const QString& name) const override;
};

class TrainView : public AbstractView
{
    Q_OBJECT

    public:

        TrainView(Context *context, QStackedWidget *controls);
        ~TrainView();
        void close() override;

        int viewType() const override { return VIEW_TRAIN; }

    public slots:

        bool isBlank() override;
        void onSelectionChanged();

    protected:

        void notifyViewPerspectiveAdded(Perspective* page) override;
        Perspective* getViewsPerspective(const QString& name) const override;
        ViewParser* getViewParser(Context* context, bool useDefault) const override;

    private:

        TrainSidebar *trainTool;
        TrainBottom *trainBottom;

    private slots:
            void onAutoHideChanged(bool enabled);
};

class TrendsViewParser : public ViewParser {

    public:
        TrendsViewParser(Context* context, bool useDefault) : ViewParser(context, useDefault) {}

    protected:
        Perspective* getViewParsersPerspective(const QString& name) const override;
};

class LTMSidebar;
class TrendsView : public AbstractView
{
    Q_OBJECT

    public:

        TrendsView(Context *context, QStackedWidget *controls);
        ~TrendsView();

        LTMSidebar *sidebar;

        int countActivities(Perspective *, DateRange dr);
        int viewType() const override { return VIEW_TRENDS; }

    signals:
        void dateChanged(DateRange);

    public slots:

        bool isBlank() override;
        void justSelected();
        void dateRangeChanged(DateRange);
        void compareChanged(bool);

    protected:

        Perspective* getViewsPerspective(const QString& name) const override;
        ViewParser* getViewParser(Context* context, bool useDefault) const override;

};

class EquipmentViewParser : public ViewParser {

    public:
        EquipmentViewParser(Context* context, bool useDefault) : ViewParser(context, useDefault) {}

    protected:
        Perspective* getViewParsersPerspective(const QString& name) const override;
};

class EquipmentView : public AbstractView
{  
    Q_OBJECT

    public:

        EquipmentView(Context *context, QStackedWidget *controls);
        ~EquipmentView();

        int viewType() const override { return VIEW_EQUIPMENT; }

        // Don't want the base class behaviour for this...
        virtual void setRide(RideItem*) override {}

        // Need to modify the behaviour
        virtual void selectionChanged() override;

        ChartSettings* chartsettings;

    public slots:

        bool isBlank() override;
        void addChart(GcWinID id) override;

    protected:

        Perspective* getViewsPerspective(const QString& name) const override;
        ViewParser* getViewParser(Context* context, bool useDefault) const override;

    private:
};

#endif // _GC_Views_h
