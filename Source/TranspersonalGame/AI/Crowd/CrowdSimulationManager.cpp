#include "CrowdSimulationManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"

ACrowdSimulationManager::ACrowdSimulationManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Tick at 10Hz for performance

    MaxAgentsPerGroup = 20;
    MaxActiveGroups = 50;
    SimulationRadius = 10000.f;
    UpdateIntervalSeconds = 0.5f;
    LODDistanceClose = 2000.f;
    LODDistanceMedium = 5000.f;
    LODDistanceFar = 10000.f;
    bSimulationEnabled = true;
    NextGroupID = 0;
    TimeSinceLastUpdate = 0.f;
    TotalActiveAgents = 0;
    TotalActiveGroups = 0;
}

void ACrowdSimulationManager::BeginPlay()
{
    Super::BeginPlay();

    // Register default spawn points if none set
    if (TribeSpawnPoints.Num() == 0)
    {
        FVector Origin = GetActorLocation();
        TribeSpawnPoints.Add(Origin + FVector(2000.f, 0.f, 0.f));
        TribeSpawnPoints.Add(Origin + FVector(-2000.f, 500.f, 0.f));
        TribeSpawnPoints.Add(Origin + FVector(0.f, 2500.f, 0.f));
    }

    if (HerdSpawnPoints.Num() == 0)
    {
        FVector Origin = GetActorLocation();
        HerdSpawnPoints.Add(Origin + FVector(3000.f, 1000.f, 0.f));
        HerdSpawnPoints.Add(Origin + FVector(-3000.f, -1000.f, 0.f));
    }

    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationManager: BeginPlay — MaxGroups=%d, SimRadius=%.0f"),
        MaxActiveGroups, SimulationRadius);
}

void ACrowdSimulationManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    ClearAllCrowdAgents();
    Super::EndPlay(EndPlayReason);
}

void ACrowdSimulationManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bSimulationEnabled) return;

    TimeSinceLastUpdate += DeltaTime;
    if (TimeSinceLastUpdate >= UpdateIntervalSeconds)
    {
        TimeSinceLastUpdate = 0.f;
        UpdateAllGroups(DeltaTime * UpdateIntervalSeconds / DeltaTime);
    }
}

// ============================================================
// Public API
// ============================================================

void ACrowdSimulationManager::SpawnCrowdGroup(ECrowd_AgentType AgentType, FVector SpawnCenter, int32 AgentCount)
{
    if (TotalActiveGroups >= MaxActiveGroups)
    {
        UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationManager: MaxActiveGroups (%d) reached — cannot spawn new group"), MaxActiveGroups);
        return;
    }

    int32 ClampedCount = FMath::Clamp(AgentCount, 1, MaxAgentsPerGroup);
    int32 NewGroupID = RegisterNewGroup(AgentType, SpawnCenter);

    FCrowd_GroupData& Group = ActiveGroups[NewGroupID];
    Group.GroupCenter = SpawnCenter;

    UWorld* World = GetWorld();
    if (!World) return;

    // Spawn placeholder actors for each agent (StaticMeshActor as stand-in)
    // In production, replace with actual character blueprints
    UClass* ActorClass = AActor::StaticClass();

    for (int32 i = 0; i < ClampedCount; ++i)
    {
        FVector SpawnOffset = GetRandomPointNearLocation(SpawnCenter, 300.f);
        FTransform SpawnTransform(FRotator::ZeroRotator, SpawnOffset);

        AActor* NewAgent = World->SpawnActor<AActor>(ActorClass, SpawnTransform);
        if (NewAgent)
        {
            NewAgent->SetActorLabel(FString::Printf(TEXT("CrowdAgent_G%d_%d"), NewGroupID, i));
            Group.Members.Add(NewAgent);

            if (i == 0)
            {
                Group.Leader = NewAgent;
            }
        }
    }

    TotalActiveAgents += Group.Members.Num();
    TotalActiveGroups = ActiveGroups.Num();

    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationManager: Spawned group %d | Type=%d | Agents=%d | Center=%s"),
        NewGroupID, (int32)AgentType, Group.Members.Num(), *SpawnCenter.ToString());
}

void ACrowdSimulationManager::TriggerMassFlee(FVector ThreatLocation, float ThreatRadius)
{
    int32 AffectedGroups = 0;

    for (FCrowd_GroupData& Group : ActiveGroups)
    {
        float Dist = FVector::Dist(Group.GroupCenter, ThreatLocation);
        if (Dist <= ThreatRadius)
        {
            Group.bIsFleeing = true;

            // Calculate flee direction (away from threat)
            FVector FleeDir = (Group.GroupCenter - ThreatLocation).GetSafeNormal();
            Group.GroupCenter = Group.GroupCenter + FleeDir * 3000.f;

            // Update all member targets
            for (AActor* Member : Group.Members)
            {
                if (Member)
                {
                    FVector MemberFleeTarget = Member->GetActorLocation() + FleeDir * 3000.f;
                    // Set flee target — in full implementation, update movement component
                }
            }

            OnGroupFleeTriggered(Group.GroupID, ThreatLocation);
            AffectedGroups++;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationManager: MassFlee triggered | ThreatLoc=%s | Radius=%.0f | AffectedGroups=%d"),
        *ThreatLocation.ToString(), ThreatRadius, AffectedGroups);
}

void ACrowdSimulationManager::TriggerMassFlee_ByDinosaur(AActor* DinosaurActor)
{
    if (!DinosaurActor) return;

    FVector DinoLocation = DinosaurActor->GetActorLocation();
    float DinoFleeRadius = 2500.f; // Default dino threat radius

    TriggerMassFlee(DinoLocation, DinoFleeRadius);
}

void ACrowdSimulationManager::SetGroupMigrationTarget(int32 GroupID, FVector Destination)
{
    for (FCrowd_GroupData& Group : ActiveGroups)
    {
        if (Group.GroupID == GroupID)
        {
            Group.GroupCenter = Destination;
            Group.bIsFleeing = false;

            UE_LOG(LogTemp, Log, TEXT("CrowdSimulationManager: Group %d migrating to %s"),
                GroupID, *Destination.ToString());
            return;
        }
    }
}

int32 ACrowdSimulationManager::GetActiveAgentCount() const
{
    return TotalActiveAgents;
}

FCrowd_GroupData ACrowdSimulationManager::GetGroupData(int32 GroupID) const
{
    for (const FCrowd_GroupData& Group : ActiveGroups)
    {
        if (Group.GroupID == GroupID)
        {
            return Group;
        }
    }
    return FCrowd_GroupData();
}

void ACrowdSimulationManager::DisbandGroup(int32 GroupID)
{
    for (int32 i = 0; i < ActiveGroups.Num(); ++i)
    {
        if (ActiveGroups[i].GroupID == GroupID)
        {
            // Destroy all member actors
            for (AActor* Member : ActiveGroups[i].Members)
            {
                if (Member)
                {
                    Member->Destroy();
                }
            }
            TotalActiveAgents -= ActiveGroups[i].Members.Num();
            ActiveGroups.RemoveAt(i);
            TotalActiveGroups = ActiveGroups.Num();
            UE_LOG(LogTemp, Log, TEXT("CrowdSimulationManager: Group %d disbanded"), GroupID);
            return;
        }
    }
}

void ACrowdSimulationManager::SetSimulationEnabled(bool bEnabled)
{
    bSimulationEnabled = bEnabled;
    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationManager: Simulation %s"), bEnabled ? TEXT("ENABLED") : TEXT("DISABLED"));
}

void ACrowdSimulationManager::SpawnDebugCrowdGroups()
{
    FVector Origin = GetActorLocation();

    // Spawn a human tribe group
    SpawnCrowdGroup(ECrowd_AgentType::HumanTribe, Origin + FVector(1000.f, 0.f, 0.f), 8);

    // Spawn a herbivore herd
    SpawnCrowdGroup(ECrowd_AgentType::HerbivoreHerd, Origin + FVector(-1500.f, 1000.f, 0.f), 12);

    // Spawn a scavenger pack
    SpawnCrowdGroup(ECrowd_AgentType::ScavengerPack, Origin + FVector(0.f, -2000.f, 0.f), 5);

    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationManager: Debug groups spawned — TotalAgents=%d"), TotalActiveAgents);
}

void ACrowdSimulationManager::ClearAllCrowdAgents()
{
    for (FCrowd_GroupData& Group : ActiveGroups)
    {
        for (AActor* Member : Group.Members)
        {
            if (Member)
            {
                Member->Destroy();
            }
        }
    }
    ActiveGroups.Empty();
    TotalActiveAgents = 0;
    TotalActiveGroups = 0;
    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationManager: All crowd agents cleared"));
}

// ============================================================
// Private Helpers
// ============================================================

void ACrowdSimulationManager::UpdateAllGroups(float DeltaTime)
{
    AActor* Player = GetPlayerActor();

    for (FCrowd_GroupData& Group : ActiveGroups)
    {
        // Remove any destroyed actors
        RemoveDeadAgentsFromGroup(Group);

        if (Group.Members.Num() == 0) continue;

        // LOD based on distance to player
        float DistToPlayer = Player ? GetDistanceToPlayer(Group.GroupCenter) : LODDistanceFar + 1.f;
        UpdateGroupLOD(Group, DistToPlayer);

        // Only full-update close groups
        if (DistToPlayer <= LODDistanceMedium)
        {
            UpdateGroupBehavior(Group, DeltaTime);
            UpdateGroupCohesion(Group);
        }
    }

    // Update counters
    TotalActiveAgents = 0;
    for (const FCrowd_GroupData& Group : ActiveGroups)
    {
        TotalActiveAgents += Group.Members.Num();
    }
    TotalActiveGroups = ActiveGroups.Num();
}

void ACrowdSimulationManager::UpdateGroupLOD(FCrowd_GroupData& Group, float DistanceToPlayer)
{
    // LOD 0 (close): Full simulation — movement, animation, collision
    // LOD 1 (medium): Simplified movement, no animation
    // LOD 2 (far): Static, no movement
    for (AActor* Member : Group.Members)
    {
        if (!Member) continue;

        if (DistanceToPlayer <= LODDistanceClose)
        {
            Member->SetActorTickEnabled(true);
            Member->SetActorHiddenInGame(false);
        }
        else if (DistanceToPlayer <= LODDistanceMedium)
        {
            Member->SetActorTickEnabled(false);
            Member->SetActorHiddenInGame(false);
        }
        else
        {
            Member->SetActorTickEnabled(false);
            Member->SetActorHiddenInGame(true); // Cull far agents
        }
    }
}

void ACrowdSimulationManager::UpdateGroupBehavior(FCrowd_GroupData& Group, float DeltaTime)
{
    if (Group.bIsFleeing)
    {
        // Move all members toward flee target
        for (AActor* Member : Group.Members)
        {
            if (Member)
            {
                MoveAgentToward(Member, Group.GroupCenter, 400.f, DeltaTime);
            }
        }

        // Check if fled far enough
        float DistFromCenter = FVector::Dist(Group.GroupCenter, GetActorLocation());
        if (DistFromCenter > SimulationRadius * 0.5f)
        {
            Group.bIsFleeing = false;
        }
    }
    else
    {
        // Idle wandering — move leader, others follow
        if (Group.Leader)
        {
            float DistToTarget = FVector::Dist(Group.Leader->GetActorLocation(), Group.GroupCenter);
            if (DistToTarget < 100.f)
            {
                // Pick new wander target
                Group.GroupCenter = GetRandomPointNearLocation(Group.Leader->GetActorLocation(), 500.f);
            }
            MoveAgentToward(Group.Leader, Group.GroupCenter, 150.f, DeltaTime);
        }

        // Non-leaders follow leader with offset
        for (int32 i = 1; i < Group.Members.Num(); ++i)
        {
            AActor* Member = Group.Members[i];
            if (Member && Group.Leader)
            {
                FVector FollowTarget = Group.Leader->GetActorLocation() +
                    FVector(FMath::RandRange(-200.f, 200.f), FMath::RandRange(-200.f, 200.f), 0.f);
                MoveAgentToward(Member, FollowTarget, 130.f, DeltaTime);
            }
        }
    }
}

void ACrowdSimulationManager::UpdateGroupCohesion(FCrowd_GroupData& Group)
{
    if (Group.Members.Num() == 0) return;

    // Recalculate group center
    FVector Sum = FVector::ZeroVector;
    int32 ValidCount = 0;
    for (AActor* Member : Group.Members)
    {
        if (Member)
        {
            Sum += Member->GetActorLocation();
            ValidCount++;
        }
    }
    if (ValidCount > 0)
    {
        Group.GroupCenter = Sum / (float)ValidCount;
    }
}

void ACrowdSimulationManager::MoveAgentToward(AActor* Agent, FVector Target, float Speed, float DeltaTime)
{
    if (!Agent) return;

    FVector CurrentLoc = Agent->GetActorLocation();
    FVector Direction = (Target - CurrentLoc).GetSafeNormal();
    FVector NewLoc = CurrentLoc + Direction * Speed * DeltaTime;

    // Keep Z (don't fall through ground in simple mode)
    NewLoc.Z = CurrentLoc.Z;

    Agent->SetActorLocation(NewLoc, true);

    // Face movement direction
    if (!Direction.IsNearlyZero())
    {
        FRotator NewRot = Direction.Rotation();
        Agent->SetActorRotation(NewRot);
    }
}

FVector ACrowdSimulationManager::GetRandomPointNearLocation(FVector Center, float Radius) const
{
    float Angle = FMath::RandRange(0.f, 360.f);
    float Dist = FMath::RandRange(50.f, Radius);
    return Center + FVector(
        FMath::Cos(FMath::DegreesToRadians(Angle)) * Dist,
        FMath::Sin(FMath::DegreesToRadians(Angle)) * Dist,
        0.f
    );
}

float ACrowdSimulationManager::GetDistanceToPlayer(FVector Location) const
{
    AActor* Player = GetPlayerActor();
    if (!Player) return LODDistanceFar + 1.f;
    return FVector::Dist(Location, Player->GetActorLocation());
}

AActor* ACrowdSimulationManager::GetPlayerActor() const
{
    UWorld* World = GetWorld();
    if (!World) return nullptr;
    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC) return nullptr;
    return PC->GetPawn();
}

void ACrowdSimulationManager::RemoveDeadAgentsFromGroup(FCrowd_GroupData& Group)
{
    Group.Members.RemoveAll([](AActor* A) { return A == nullptr || !IsValid(A); });
}

int32 ACrowdSimulationManager::RegisterNewGroup(ECrowd_AgentType AgentType, FVector Center)
{
    FCrowd_GroupData NewGroup;
    NewGroup.GroupID = NextGroupID++;
    NewGroup.GroupType = AgentType;
    NewGroup.GroupCenter = Center;
    NewGroup.bIsFleeing = false;
    ActiveGroups.Add(NewGroup);
    return ActiveGroups.Num() - 1;
}
