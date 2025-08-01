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

#ifndef _GC_EquipmentView_h
#define _GC_EquipmentView_h 1

#include "Views.h"

class EquipmentViewParser : public ViewParser {

    public:
        EquipmentViewParser(MainWindow *mainWindow, bool useDefault) : ViewParser(nullptr, mainWindow, useDefault), mainWindow_(mainWindow) {}

    protected:
        Perspective* getViewParsersPerspective(const QString& name) const override;

    private:
        MainWindow *mainWindow_;
};

class EquipmentView : public AbstractView
{  
    Q_OBJECT

    public:

        EquipmentView(MainWindow *mainWindow, QStackedWidget *controls);
        ~EquipmentView();

        // the view's user name must be translated for display
        static constexpr const char* userName = "Equipment";
        QString viewsUserName() const override { return userName; }

        static constexpr const char* internalName = "equipment";
        QString viewsInternalName() const override { return internalName; }

        GcViewType viewType() const override { return GcViewType::VIEW_EQUIPMENT; }

        // Need to modify the behaviour
        virtual void selectionChanged() override;

        // Don't want the base class behaviour for these...
        void setRide(RideItem*) override {}
        void notifyViewStateRestored() override {}
        void sidebarChanged() override {}
        void tileModeChanged() override {}
        void resetLayout(QComboBox *) override {}

        ChartSettings* chartsettings;

    public slots:

        bool isBlank() override;
        void addChart(GcWinID id) override;

    protected:

        Perspective* getViewsPerspective(const QString& name) const override;
        ViewParser* getViewParser(bool useDefault) const override;

    private:
};

#endif // _GC_EquipmentView_h
