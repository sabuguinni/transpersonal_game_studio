#include "CrowdMassSpawner.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

ACrowd_MassSpawner::ACrowd_MassSpawner()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = TickInterval;

    DefaultSpawnConfig.Pattern = ECrowd_SpawnPattern::HerdFormation;
    DefaultSpawnConfig.Count = 30;
    DefaultSpawnConfig.SpawnRadius = 2000.0f;
    DefaultSpawnConfig.MinSpacing = 200.0f;

    LODConfig.FullDetailDistance = 3000.0f;
    LODConfig.MediumDetailDistance = 8000.0f;
    LODConfig.LowDetailDistance = 20000.0f;
    LODConfig.bUseBillboardAtMaxDistance = true;

    MaxAgentsInWorld = 500;
    bAutoSpawnOnBeginPlay = true;
}

void ACrowd_MassSpawner::BeginPlay()
{
    Super::BeginPlay();

    if (bAutoSpawnOnBeginPlay)
    {
        FCrowd_SpawnRequest Req = DefaultSpawnConfig;
        Req.SpawnCenter = GetActorLocation();
        SpawnHerd(Req);
    }
}

void ACrowd_MassSpawner::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    TimeSinceLastTick += DeltaTime;
    if (TimeSinceLastTick < TickInterval)
        return;
    TimeSinceLastTick = 0.0f;

    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (PC && PC->GetPawn())
    {
        UpdateLODForAllAgents(PC->GetPawn()->GetActorLocation());
    }
}

void ACrowd_MassSpawner::SpawnHerd(const FCrowd_SpawnRequest& Request)
{
    UWorld* World = GetWorld();
    if (!World) return;

    int32 ToSpawn = FMath::Min(Request.Count, MaxAgentsInWorld - SpawnedAgents.Num());

    for (int32 i = 0; i < ToSpawn; ++i)
    {
        FVector SpawnPos;
        switch (Request.Pattern)
        {
            case ECrowd_SpawnPattern::HerdFormation:
                SpawnPos = GetHerdFormationPosition(Request, i);
                break;
            case ECrowd_SpawnPattern::MigrationLine:
                SpawnPos = GetMigrationLinePosition(Request, i);
                break;
            default:
                SpawnPos = GetScatteredPosition(Request, i);
                break;
        }

        // Ensure minimum spacing from existing agents
        bool bTooClose = false;
        for (AActor* Existing : SpawnedAgents)
        {
            if (Existing && FVector::Dist(Existing->GetActorLocation(), SpawnPos) < Request.MinSpacing)
            {
                bTooClose = true;
                break;
            }
        }
        if (bTooClose) continue;

        FActorSpawnParameters Params;
        Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

        AActor* NewAgent = World->SpawnActor<AActor>(AActor::StaticClass(), SpawnPos, FRotator::ZeroRotator, Params);
        if (NewAgent)
        {
            SpawnedAgents.Add(NewAgent);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("CrowdMassSpawner: Spawned %d agents (total: %d)"), ToSpawn, SpawnedAgents.Num());
}

void ACrowd_MassSpawner::DespawnAllAgents()
{
    for (AActor* Agent : SpawnedAgents)
    {
        if (Agent)
        {
            Agent->Destroy();
        }
    }
    SpawnedAgents.Empty();
    UE_LOG(LogTemp, Log, TEXT("CrowdMassSpawner: All agents despawned"));
}

int32 ACrowd_MassSpawner::GetActiveAgentCount() const
{
    int32 Count = 0;
    for (const AActor* Agent : SpawnedAgents)
    {
        if (Agent) ++Count;
    }
    return Count;
}

void ACrowd_MassSpawner::UpdateLODForAllAgents(const FVector& PlayerLocation)
{
    for (AActor* Agent : SpawnedAgents)
    {
        if (!Agent) continue;

        float Dist = FVector::Dist(Agent->GetActorLocation(), PlayerLocation);

        // Scale based on LOD distance — full/medium/low/billboard
        float Scale = 1.0f;
        if (Dist > LODConfig.LowDetailDistance)
        {
            Scale = 0.5f; // Billboard-like reduction
        }
        else if (Dist > LODConfig.MediumDetailDistance)
        {
            Scale = 0.75f;
        }
        else if (Dist > LODConfig.FullDetailDistance)
        {
            Scale = 0.9f;
        }

        Agent->SetActorScale3D(FVector(Scale));
    }
}

void ACrowd_MassSpawner::TriggerStampedeFromLocation(const FVector& ThreatLocation, float ThreatRadius)
{
    UWorld* World = GetWorld();
    if (!World) return;

    int32 AffectedCount = 0;
    for (AActor* Agent : SpawnedAgents)
    {
        if (!Agent) continue;

        float Dist = FVector::Dist(Agent->GetActorLocation(), ThreatLocation);
        if (Dist <= ThreatRadius)
        {
            // Flee direction = away from threat
            FVector FleeDir = (Agent->GetActorLocation() - ThreatLocation).GetSafeNormal();
            FVector FleeTarget = Agent->GetActorLocation() + FleeDir * 3000.0f;

            // Snap to ground level (approximate)
            FleeTarget.Z = Agent->GetActorLocation().Z;
            Agent->SetActorLocation(FleeTarget, true);
            ++AffectedCount;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("CrowdMassSpawner: Stampede triggered — %d agents fleeing from radius %.0f"),
        AffectedCount, ThreatRadius);

    // Debug sphere
    DrawDebugSphere(World, ThreatLocation, ThreatRadius, 16, FColor::Red, false, 5.0f);
}

FVector ACrowd_MassSpawner::GetScatteredPosition(const FCrowd_SpawnRequest& Request, int32 Index) const
{
    float Angle = FMath::FRandRange(0.0f, 360.0f);
    float Radius = FMath::FRandRange(0.0f, Request.SpawnRadius);
    float X = Request.SpawnCenter.X + Radius * FMath::Cos(FMath::DegreesToRadians(Angle));
    float Y = Request.SpawnCenter.Y + Radius * FMath::Sin(FMath::DegreesToRadians(Angle));
    return FVector(X, Y, Request.SpawnCenter.Z);
}

FVector ACrowd_MassSpawner::GetHerdFormationPosition(const FCrowd_SpawnRequest& Request, int32 Index) const
{
    // Concentric rings: alpha in center, rest in rings
    if (Index == 0)
        return Request.SpawnCenter;

    int32 Ring = FMath::CeilToInt(FMath::Sqrt((float)Index));
    float RingRadius = Ring * Request.MinSpacing * 1.5f;
    int32 PerRing = FMath::Max(1, 6 * Ring);
    float Angle = (float)(Index % PerRing) / (float)PerRing * 360.0f;

    float X = Request.SpawnCenter.X + RingRadius * FMath::Cos(FMath::DegreesToRadians(Angle));
    float Y = Request.SpawnCenter.Y + RingRadius * FMath::Sin(FMath::DegreesToRadians(Angle));
    return FVector(X, Y, Request.SpawnCenter.Z);
}

FVector ACrowd_MassSpawner::GetMigrationLinePosition(const FCrowd_SpawnRequest& Request, int32 Index) const
{
    // Line along X axis with slight lateral spread
    float LineOffset = Index * Request.MinSpacing * 1.2f;
    float LateralSpread = FMath::FRandRange(-Request.MinSpacing * 0.5f, Request.MinSpacing * 0.5f);
    return FVector(
        Request.SpawnCenter.X + LineOffset,
        Request.SpawnCenter.Y + LateralSpread,
        Request.SpawnCenter.Z
    );
}
