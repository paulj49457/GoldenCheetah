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

#include "EquipCalculator.h"
#include "OverviewItems.h"

EquipCalculator::EquipCalculator(Context* context) : context_(context)
{
}

EquipCalculator::~EquipCalculator()
{
}

void
EquipCalculator::RecalculateEquipSpace(const QString& eqLinkName, ChartSpace* space)
{
    // already recalcuating !
    if (recalculationThreads_.count()) return;

    eqTiles_ = space->allItems();
    eqLinkName_ = eqLinkName;

    // Reset all the tiles distances
    for (ChartSpaceItem* eqTile : eqTiles_) {
        static_cast<EquipOverviewItem*>(eqTile)->resetDistanceCovered();
    }

    // Currently this code restricts the calculation to a single athlete, so possibly needs looking
    // at to provide a general calcluation of distances of equipment used by all athletes.

    // take a copy of the rides through the athlete's rides creating a ride List to process
    rideItemList_ = context_->athlete->rideCache->rides();

    // calculate number of threads and work per thread
    int maxthreads = QThreadPool::globalInstance()->maxThreadCount();
    int threads = maxthreads / 4; // Don't need many threads
    if (threads == 0) threads = 1; // but need at least one!

    // keep launching the threads
    while (threads--) {

        // if goes past last make it the last
        EquipCalculationThread* thread = new EquipCalculationThread(this);
        recalculationThreads_ << thread;

        thread->start();
    }
}

RideItem*
EquipCalculator::nextRideToCheck()
{
    RideItem* returning;
    updateMutex_.lock();

    if (rideItemList_.isEmpty()) {
        returning = nullptr;
    }
    else {
        returning = rideItemList_.takeLast();
    }
    updateMutex_.unlock();
    return(returning);
}

void
EquipCalculationThread::run() {

    RideItem* item;
    while (item = eqCalc_->nextRideToCheck()) {
        eqCalc_->RecalculateEq(item);
    }
    eqCalc_->threadCompleted(this);
    return;
}

void
EquipCalculator::threadCompleted(EquipCalculationThread* thread)
{
    updateMutex_.lock();
    recalculationThreads_.removeOne(thread);
    updateMutex_.unlock();

    if (recalculationThreads_.count() == 0) {

        // finished recalculation
        eqTiles_.clear();
    }
}

void
EquipCalculator::RecalculateEq(RideItem* rideItem)
{
    if (rideItem->getText("EquipmentLink", "abcde") == eqLinkName_) {

        QDate actDate(QDate(1900, 01, 01).addDays(rideItem->getText("Start Date", "0").toInt()));

        // get the distance metric
        double dist = rideItem->getStringForSymbol("total_distance", GlobalContext::context()->useMetricUnits).toDouble();

        for (ChartSpaceItem* eqTile : eqTiles_) {

            if (static_cast<EquipOverviewItem*>(eqTile)->isWithin(actDate)) {

                static_cast<EquipOverviewItem*>(eqTile)->incrementDistanceCovered(dist);
            }
        }
    }
}


