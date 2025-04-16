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

#ifndef _GC_EquipmentView_h
#define _GC_EquipmentView_h 1

#include "AbstractView.h"
#include "ChartSettings.h"

class EquipView : public AbstractView
{  
    Q_OBJECT

    public:

        EquipView(Context *context, QStackedWidget *controls);
        ~EquipView();

        // Don't want the base class behaviour for this...
        virtual void setRide(RideItem*) override {}

        // Need to modify the behaviour
        virtual void setSelected(bool) override;

        ChartSettings* chartsettings;

    public slots:

        bool isBlank() override;

    private:

};

#endif // _GC_EquipmentView_h
