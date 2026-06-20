#include "CrowdWaypointSystem.h"

ACrowd_WaypointActor::ACrowd_WaypointActor()
{
    PrimaryActorTick.bCanEverTick = false;
}

void ACrowd_WaypointActor::BeginPlay()
{
    Super::BeginPlay();
}

bool ACrowd_WaypointActor::IsAvailable() const
{
    return CurrentOccupants.Num() < WaypointData.MaxOccupants;
}

bool ACrowd_WaypointActor::RegisterOccupant(AActor* Occupant)
{
    if (!Occupant) return false;
    if (!IsAvailable()) return false;
    if (CurrentOccupants.Contains(Occupant)) return false;
    CurrentOccupants.Add(Occupant);
    return true;
}

void ACrowd_WaypointActor::UnregisterOccupant(AActor* Occupant)
{
    if (!Occupant) return;
    CurrentOccupants.Remove(Occupant);
}

float ACrowd_WaypointActor::GetOccupancyRatio() const
{
    if (WaypointData.MaxOccupants <= 0) return 1.0f;
    return static_cast<float>(CurrentOccupants.Num()) / static_cast<float>(WaypointData.MaxOccupants);
}
