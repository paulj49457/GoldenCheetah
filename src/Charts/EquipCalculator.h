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

#ifndef _GC_EquipCalculator_h
#define _GC_EquipCalculator_h 1

// basics
#include "Context.h"
#include "ChartSpace.h"
#include "RideItem.h"

class EquipCalculator;

class EquipCalculationThread : public QThread
{
    public:
        EquipCalculationThread(EquipCalculator* eqCalc) : eqCalc_(eqCalc) {}

    protected:

        // recalculate distances
        virtual void run() override;

    private:
        EquipCalculator* eqCalc_;
};

class EquipCalculator
{
    public:
        EquipCalculator(Context* context);
        virtual ~EquipCalculator();

        // void RecalculateEquipTile(const QString& eqLink, ChartSpaceItem* eqTile); ptj
        void RecalculateEquipSpace(const QString& eqLink, ChartSpace* space);

    protected:

        friend class ::EquipCalculationThread;

        // distance calculation
        void RecalculateEq(RideItem* rideItem);
        RideItem* nextRideToCheck();
        void threadCompleted(EquipCalculationThread* thread);

        // Equipment distance recalculation
        QMutex updateMutex_;
        QVector<EquipCalculationThread*> recalculationThreads_;
        QVector<RideItem*>  rideItemList_;

        QString eqLinkName_;
        QList<ChartSpaceItem*> eqTiles_;
        Context* context_;

};

#endif // _GC_EquipCalculator_h
