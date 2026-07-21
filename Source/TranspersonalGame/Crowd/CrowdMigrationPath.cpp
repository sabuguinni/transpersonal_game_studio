#include "CrowdMigrationPath.h"
#include "DrawDebugHelpers.h"

ACrowdMigrationPath::ACrowdMigrationPath()
{
    PrimaryActorTick.bCanEverTick = false;

    // Default migration path: a simple arc across the plains
    FCrowd_WaypointData WP0;
    WP0.Location = FVector(-2000.0f, -1500.0f, 50.0f);
    WP0.Radius = 300.0f;
    WP0.MaxAgentsAtWaypoint = 20;
    Waypoints.Add(WP0);

    FCrowd_WaypointData WP1;
    WP1.Location = FVector(0.0f, -2500.0f, 50.0f);
    WP1.Radius = 300.0f;
    WP1.MaxAgentsAtWaypoint = 20;
    Waypoints.Add(WP1);

    FCrowd_WaypointData WP2;
    WP2.Location = FVector(2000.0f, -1500.0f, 50.0f);
    WP2.Radius = 300.0f;
    WP2.MaxAgentsAtWaypoint = 20;
    Waypoints.Add(WP2);

    FCrowd_WaypointData WP3;
    WP3.Location = FVector(2500.0f, 0.0f, 50.0f);
    WP3.Radius = 300.0f;
    WP3.MaxAgentsAtWaypoint = 20;
    Waypoints.Add(WP3);
}

void ACrowdMigrationPath::BeginPlay()
{
    Super::BeginPlay();
}

int32 ACrowdMigrationPath::GetNextWaypointIndex(int32 CurrentIndex) const
{
    if (Waypoints.Num() == 0)
    {
        return 0;
    }

    int32 NextIndex = CurrentIndex + 1;
    if (NextIndex >= Waypoints.Num())
    {
        return bLooping ? 0 : Waypoints.Num() - 1;
    }
    return NextIndex;
}

FVector ACrowdMigrationPath::GetWaypointLocation(int32 Index) const
{
    if (!Waypoints.IsValidIndex(Index))
    {
        return GetActorLocation();
    }
    return Waypoints[Index].Location;
}

float ACrowdMigrationPath::GetTotalPathLength() const
{
    float TotalLength = 0.0f;
    for (int32 i = 0; i < Waypoints.Num() - 1; ++i)
    {
        TotalLength += FVector::Dist(Waypoints[i].Location, Waypoints[i + 1].Location);
    }
    if (bLooping && Waypoints.Num() > 1)
    {
        TotalLength += FVector::Dist(Waypoints.Last().Location, Waypoints[0].Location);
    }
    return TotalLength;
}

#if WITH_EDITOR
void ACrowdMigrationPath::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);
    // Debug draw the path in editor when properties change
    if (GetWorld())
    {
        for (int32 i = 0; i < Waypoints.Num(); ++i)
        {
            int32 NextIdx = GetNextWaypointIndex(i);
            if (NextIdx != i)
            {
                DrawDebugLine(
                    GetWorld(),
                    Waypoints[i].Location,
                    Waypoints[NextIdx].Location,
                    FColor::Yellow,
                    false,
                    5.0f,
                    0,
                    5.0f
                );
            }
            DrawDebugSphere(
                GetWorld(),
                Waypoints[i].Location,
                Waypoints[i].Radius,
                12,
                FColor::Green,
                false,
                5.0f
            );
        }
    }
}
#endif
