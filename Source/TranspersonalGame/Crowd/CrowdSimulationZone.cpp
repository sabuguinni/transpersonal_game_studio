#include "CrowdSimulationZone.h"
#include "Components/BoxComponent.h"
#include "Kismet/KismetMathLibrary.h"

ACrowdSimulationZone::ACrowdSimulationZone()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 1.0f; // Tick once per second for performance

    ZoneBounds = CreateDefaultSubobject<UBoxComponent>(TEXT("ZoneBounds"));
    SetRootComponent(ZoneBounds);
    ZoneBounds->SetBoxExtent(FVector(500.0f, 500.0f, 200.0f));
    ZoneBounds->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    ZoneBounds->SetCollisionResponseToAllChannels(ECR_Ignore);
    ZoneBounds->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    ZoneBounds->SetHiddenInGame(false); // Visible in editor for debugging
}

void ACrowdSimulationZone::BeginPlay()
{
    Super::BeginPlay();

    if (ZoneConfig.bIsActive)
    {
        ActivateZone();
    }
}

void ACrowdSimulationZone::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Periodic zone maintenance — clamp agent count to valid range
    if (CurrentAgentCount < 0)
    {
        CurrentAgentCount = 0;
    }
    if (CurrentAgentCount > ZoneConfig.MaxAgents)
    {
        CurrentAgentCount = ZoneConfig.MaxAgents;
    }
}

void ACrowdSimulationZone::ActivateZone()
{
    bZoneActive = true;
    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationZone [%s] ACTIVATED — Type=%d MaxAgents=%d"),
        *GetActorLabel(), (int32)ZoneConfig.ZoneType, ZoneConfig.MaxAgents);
}

void ACrowdSimulationZone::DeactivateZone()
{
    bZoneActive = false;
    CurrentAgentCount = 0;
    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationZone [%s] DEACTIVATED"), *GetActorLabel());
}

int32 ACrowdSimulationZone::GetAvailableSlots() const
{
    return FMath::Max(0, ZoneConfig.MaxAgents - CurrentAgentCount);
}

bool ACrowdSimulationZone::CanAcceptAgent() const
{
    return bZoneActive && CurrentAgentCount < ZoneConfig.MaxAgents;
}

FVector ACrowdSimulationZone::GetRandomSpawnLocation() const
{
    if (!ZoneBounds)
    {
        return GetActorLocation();
    }

    const FVector Origin = GetActorLocation();
    const float Radius = ZoneConfig.AgentSpawnRadius;

    // Random point within spawn radius
    const float Angle = FMath::RandRange(0.0f, 360.0f);
    const float Distance = FMath::RandRange(0.0f, Radius);
    const float X = Origin.X + Distance * FMath::Cos(FMath::DegreesToRadians(Angle));
    const float Y = Origin.Y + Distance * FMath::Sin(FMath::DegreesToRadians(Angle));
    const float Z = Origin.Z;

    return FVector(X, Y, Z);
}

void ACrowdSimulationZone::RegisterAgent()
{
    if (CanAcceptAgent())
    {
        CurrentAgentCount++;
        UE_LOG(LogTemp, Verbose, TEXT("CrowdZone [%s] Agent registered: %d/%d"),
            *GetActorLabel(), CurrentAgentCount, ZoneConfig.MaxAgents);
    }
}

void ACrowdSimulationZone::UnregisterAgent()
{
    if (CurrentAgentCount > 0)
    {
        CurrentAgentCount--;
        UE_LOG(LogTemp, Verbose, TEXT("CrowdZone [%s] Agent unregistered: %d/%d"),
            *GetActorLabel(), CurrentAgentCount, ZoneConfig.MaxAgents);
    }
}
