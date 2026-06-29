// CrowdNPCSpawner.cpp
// Agent #13 — Crowd & Traffic Simulation
// Implementation of prehistoric NPC crowd spawner

#include "Crowd/CrowdNPCSpawner.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "NavigationSystem.h"
#include "AI/NavigationSystemBase.h"

ACrowdNPCSpawner::ACrowdNPCSpawner()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = UpdateTickRate;

    CurrentGroupState = ECrowd_GroupState::Idle;
    MaxNPCsPerSpawner = 20;
    LOD_FullDetailDistance = 3000.f;
    LOD_CullDistance = 8000.f;
    UpdateTickRate = 0.25f;
}

void ACrowdNPCSpawner::BeginPlay()
{
    Super::BeginPlay();

    // Auto-spawn default group on begin play if configured
    if (DefaultGroupConfig.GroupSize > 0)
    {
        SpawnGroup(DefaultGroupConfig);
    }
}

void ACrowdNPCSpawner::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    TimeSinceLastTick += DeltaTime;
    if (TimeSinceLastTick >= UpdateTickRate)
    {
        TimeSinceLastTick = 0.f;
        UpdateGroupBehavior(DeltaTime);
    }
}

void ACrowdNPCSpawner::SpawnGroup(const FCrowd_GroupConfig& Config)
{
    UWorld* World = GetWorld();
    if (!World) return;

    int32 ToSpawn = FMath::Min(Config.GroupSize, MaxNPCsPerSpawner - SpawnedNPCs.Num());
    if (ToSpawn <= 0) return;

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    for (int32 i = 0; i < ToSpawn; ++i)
    {
        FVector SpawnLoc = GetRandomPointInRadius(GetActorLocation(), Config.SpawnRadius);
        SpawnLoc.Z += 100.f; // Offset above ground

        // Spawn a basic pawn as placeholder — replace with actual NPC class when available
        AActor* NewNPC = World->SpawnActor<AActor>(AActor::StaticClass(), SpawnLoc, FRotator::ZeroRotator, SpawnParams);
        if (NewNPC)
        {
            NewNPC->SetActorLabel(FString::Printf(TEXT("CrowdNPC_%d_%d"), GetUniqueID(), i));
            SpawnedNPCs.Add(NewNPC);
        }
    }

    CurrentGroupState = Config.InitialState;
    DefaultGroupConfig.HomeLocation = GetActorLocation();

    UE_LOG(LogTemp, Log, TEXT("CrowdNPCSpawner: Spawned %d NPCs. Total: %d"), ToSpawn, SpawnedNPCs.Num());
}

void ACrowdNPCSpawner::DespawnAllGroups()
{
    for (AActor* NPC : SpawnedNPCs)
    {
        if (IsValid(NPC))
        {
            NPC->Destroy();
        }
    }
    SpawnedNPCs.Empty();
    UE_LOG(LogTemp, Log, TEXT("CrowdNPCSpawner: All NPCs despawned."));
}

void ACrowdNPCSpawner::SetGroupState(ECrowd_GroupState NewState)
{
    CurrentGroupState = NewState;
    UE_LOG(LogTemp, Log, TEXT("CrowdNPCSpawner: Group state -> %d"), (int32)NewState);
}

int32 ACrowdNPCSpawner::GetActiveNPCCount() const
{
    int32 Count = 0;
    for (const AActor* NPC : SpawnedNPCs)
    {
        if (IsValid(NPC)) ++Count;
    }
    return Count;
}

void ACrowdNPCSpawner::TriggerFleeResponse(FVector ThreatLocation, float ThreatRadius)
{
    // Switch group to fleeing state
    SetGroupState(ECrowd_GroupState::Fleeing);

    for (AActor* NPC : SpawnedNPCs)
    {
        if (!IsValid(NPC)) continue;

        float Dist = FVector::Dist(NPC->GetActorLocation(), ThreatLocation);
        if (Dist <= ThreatRadius)
        {
            // Move NPC away from threat
            FVector FleeDir = (NPC->GetActorLocation() - ThreatLocation).GetSafeNormal();
            FVector FleeTarget = NPC->GetActorLocation() + FleeDir * 2000.f;
            NPC->SetActorLocation(FleeTarget, true);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("CrowdNPCSpawner: Flee triggered from %s, radius %.0f"),
        *ThreatLocation.ToString(), ThreatRadius);
}

void ACrowdNPCSpawner::UpdateLOD(float DistanceFromPlayer)
{
    // Full detail: < LOD_FullDetailDistance — tick every frame
    // Medium detail: LOD_FullDetailDistance to LOD_CullDistance — tick every 0.5s
    // Culled: > LOD_CullDistance — disable tick entirely

    if (DistanceFromPlayer > LOD_CullDistance)
    {
        SetActorTickEnabled(false);
        for (AActor* NPC : SpawnedNPCs)
        {
            if (IsValid(NPC)) NPC->SetActorHiddenInGame(true);
        }
    }
    else if (DistanceFromPlayer > LOD_FullDetailDistance)
    {
        PrimaryActorTick.TickInterval = 0.5f;
        for (AActor* NPC : SpawnedNPCs)
        {
            if (IsValid(NPC)) NPC->SetActorHiddenInGame(false);
        }
    }
    else
    {
        PrimaryActorTick.TickInterval = UpdateTickRate;
        for (AActor* NPC : SpawnedNPCs)
        {
            if (IsValid(NPC)) NPC->SetActorHiddenInGame(false);
        }
    }
}

void ACrowdNPCSpawner::UpdateGroupBehavior(float DeltaTime)
{
    switch (CurrentGroupState)
    {
        case ECrowd_GroupState::Fleeing:
            HandleFleeState(DeltaTime);
            break;

        case ECrowd_GroupState::Foraging:
            // NPCs wander within WanderRadius of home
            for (AActor* NPC : SpawnedNPCs)
            {
                if (!IsValid(NPC)) continue;
                FVector Target = GetRandomPointInRadius(DefaultGroupConfig.HomeLocation, DefaultGroupConfig.WanderRadius);
                // Gradual movement toward target (simplified — real impl uses NavMesh)
                FVector Dir = (Target - NPC->GetActorLocation()).GetSafeNormal();
                NPC->SetActorLocation(NPC->GetActorLocation() + Dir * 50.f, true);
            }
            break;

        case ECrowd_GroupState::Idle:
        case ECrowd_GroupState::Camping:
            // No movement — just idle animations (handled by AnimBP)
            break;

        case ECrowd_GroupState::Migrating:
            // Move group toward a migration target
            for (AActor* NPC : SpawnedNPCs)
            {
                if (!IsValid(NPC)) continue;
                FVector Dir = (DefaultGroupConfig.HomeLocation - NPC->GetActorLocation()).GetSafeNormal();
                NPC->SetActorLocation(NPC->GetActorLocation() + Dir * 80.f, true);
            }
            break;

        default:
            break;
    }
}

void ACrowdNPCSpawner::HandleFleeState(float DeltaTime)
{
    // After 10 seconds of fleeing, return to idle
    static float FleeTimer = 0.f;
    FleeTimer += DeltaTime;
    if (FleeTimer >= 10.f)
    {
        FleeTimer = 0.f;
        SetGroupState(ECrowd_GroupState::Idle);
        UE_LOG(LogTemp, Log, TEXT("CrowdNPCSpawner: Flee ended — returning to Idle."));
    }
}

FVector ACrowdNPCSpawner::GetRandomPointInRadius(FVector Center, float Radius) const
{
    float Angle = FMath::RandRange(0.f, 360.f);
    float Dist = FMath::RandRange(0.f, Radius);
    return Center + FVector(
        FMath::Cos(FMath::DegreesToRadians(Angle)) * Dist,
        FMath::Sin(FMath::DegreesToRadians(Angle)) * Dist,
        0.f
    );
}
