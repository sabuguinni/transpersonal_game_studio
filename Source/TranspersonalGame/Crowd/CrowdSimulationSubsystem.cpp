#include "CrowdSimulationSubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "NavigationSystem.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

UCrowdSimulationSubsystem::UCrowdSimulationSubsystem()
{
    NextCrowdID = 1;
    MaxConcurrentAgents = 50000;
    LODUpdateInterval = 1.0f;
    HighDetailRadius = 1000.0f;
    MediumDetailRadius = 2500.0f;
    LowDetailRadius = 5000.0f;
    CullingDistance = 10000.0f;
    bEnableMassEntityIntegration = true;
    bEnableOcclusionCulling = true;
    MassEntitySubsystem = nullptr;
}

void UCrowdSimulationSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationSubsystem: Initializing..."));
    
    // Initialize Mass Entity integration
    if (bEnableMassEntityIntegration)
    {
        InitializeMassEntity();
    }
    
    // Start LOD update timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            LODUpdateTimer,
            this,
            &UCrowdSimulationSubsystem::UpdateLODSystem,
            LODUpdateInterval,
            true
        );
    }
    
    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationSubsystem: Initialized with max %d agents"), MaxConcurrentAgents);
}

void UCrowdSimulationSubsystem::Deinitialize()
{
    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationSubsystem: Deinitializing..."));
    
    // Clear all crowd groups
    CleanupCrowdGroups();
    
    // Clear timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(LODUpdateTimer);
    }
    
    Super::Deinitialize();
}

bool UCrowdSimulationSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    return true;
}

int32 UCrowdSimulationSubsystem::SpawnCrowdGroup(FVector Location, ECrowd_GroupType GroupType, int32 AgentCount)
{
    if (AgentCount <= 0 || GetTotalActiveAgents() + AgentCount > MaxConcurrentAgents)
    {
        UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationSubsystem: Cannot spawn %d agents - would exceed limit"), AgentCount);
        return -1;
    }
    
    int32 CrowdID = NextCrowdID++;
    
    // Create crowd group data
    FCrowd_GroupData NewGroup;
    NewGroup.GroupID = CrowdID;
    NewGroup.GroupType = GroupType;
    NewGroup.AgentCount = AgentCount;
    NewGroup.CenterLocation = Location;
    NewGroup.CurrentBehavior = ECrowd_BehaviorState::Idle;
    NewGroup.LODLevel = ECrowd_LODLevel::High;
    NewGroup.bIsActive = true;
    NewGroup.SpawnTime = GetWorld()->GetTimeSeconds();
    
    // Set behavior based on group type
    switch (GroupType)
    {
        case ECrowd_GroupType::TribalSettlement:
            NewGroup.CurrentBehavior = ECrowd_BehaviorState::Gathering;
            NewGroup.MovementSpeed = 50.0f;
            break;
        case ECrowd_GroupType::HuntingParty:
            NewGroup.CurrentBehavior = ECrowd_BehaviorState::Hunting;
            NewGroup.MovementSpeed = 150.0f;
            break;
        case ECrowd_GroupType::GatheringGroup:
            NewGroup.CurrentBehavior = ECrowd_BehaviorState::Gathering;
            NewGroup.MovementSpeed = 75.0f;
            break;
        case ECrowd_GroupType::MigrationGroup:
            NewGroup.CurrentBehavior = ECrowd_BehaviorState::Moving;
            NewGroup.MovementSpeed = 100.0f;
            break;
        case ECrowd_GroupType::PatrolGroup:
            NewGroup.CurrentBehavior = ECrowd_BehaviorState::Patrolling;
            NewGroup.MovementSpeed = 125.0f;
            break;
        default:
            NewGroup.CurrentBehavior = ECrowd_BehaviorState::Idle;
            NewGroup.MovementSpeed = 50.0f;
            break;
    }
    
    // Store the group
    ActiveCrowdGroups.Add(CrowdID, NewGroup);
    
    // Broadcast spawn event
    OnCrowdGroupSpawned.Broadcast(CrowdID, NewGroup.CurrentBehavior);
    
    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationSubsystem: Spawned crowd group %d with %d agents at %s"), 
           CrowdID, AgentCount, *Location.ToString());
    
    return CrowdID;
}

void UCrowdSimulationSubsystem::DespawnCrowdGroup(int32 CrowdID)
{
    if (FCrowd_GroupData* Group = ActiveCrowdGroups.Find(CrowdID))
    {
        Group->bIsActive = false;
        
        // Remove from Mass Entity system if integrated
        if (CrowdToMassEntityMap.Contains(CrowdID))
        {
            CrowdToMassEntityMap.Remove(CrowdID);
        }
        
        // Broadcast despawn event
        OnCrowdGroupDespawned.Broadcast(CrowdID, Group->CurrentBehavior);
        
        // Remove from active groups
        ActiveCrowdGroups.Remove(CrowdID);
        
        UE_LOG(LogTemp, Log, TEXT("CrowdSimulationSubsystem: Despawned crowd group %d"), CrowdID);
    }
}

void UCrowdSimulationSubsystem::SetCrowdBehavior(int32 CrowdID, ECrowd_BehaviorState NewBehavior)
{
    if (FCrowd_GroupData* Group = ActiveCrowdGroups.Find(CrowdID))
    {
        ECrowd_BehaviorState OldBehavior = Group->CurrentBehavior;
        Group->CurrentBehavior = NewBehavior;
        
        // Adjust movement speed based on behavior
        switch (NewBehavior)
        {
            case ECrowd_BehaviorState::Idle:
                Group->MovementSpeed = 0.0f;
                break;
            case ECrowd_BehaviorState::Moving:
                Group->MovementSpeed = 100.0f;
                break;
            case ECrowd_BehaviorState::Gathering:
                Group->MovementSpeed = 50.0f;
                break;
            case ECrowd_BehaviorState::Hunting:
                Group->MovementSpeed = 150.0f;
                break;
            case ECrowd_BehaviorState::Fleeing:
                Group->MovementSpeed = 200.0f;
                break;
            case ECrowd_BehaviorState::Fighting:
                Group->MovementSpeed = 75.0f;
                break;
            case ECrowd_BehaviorState::Patrolling:
                Group->MovementSpeed = 125.0f;
                break;
        }
        
        // Broadcast behavior change
        OnCrowdBehaviorChanged.Broadcast(CrowdID, NewBehavior);
        
        UE_LOG(LogTemp, Log, TEXT("CrowdSimulationSubsystem: Changed crowd %d behavior from %d to %d"), 
               CrowdID, (int32)OldBehavior, (int32)NewBehavior);
    }
}

void UCrowdSimulationSubsystem::MoveCrowdToLocation(int32 CrowdID, FVector TargetLocation)
{
    if (FCrowd_GroupData* Group = ActiveCrowdGroups.Find(CrowdID))
    {
        Group->TargetLocation = TargetLocation;
        Group->bHasTarget = true;
        
        // Set to moving behavior if idle
        if (Group->CurrentBehavior == ECrowd_BehaviorState::Idle)
        {
            SetCrowdBehavior(CrowdID, ECrowd_BehaviorState::Moving);
        }
        
        UE_LOG(LogTemp, Log, TEXT("CrowdSimulationSubsystem: Moving crowd %d to %s"), 
               CrowdID, *TargetLocation.ToString());
    }
}

void UCrowdSimulationSubsystem::UpdateCrowdLOD(FVector PlayerLocation)
{
    for (auto& CrowdPair : ActiveCrowdGroups)
    {
        FCrowd_GroupData& Group = CrowdPair.Value;
        float Distance = FVector::Dist(PlayerLocation, Group.CenterLocation);
        
        ECrowd_LODLevel NewLOD;
        if (Distance <= HighDetailRadius)
        {
            NewLOD = ECrowd_LODLevel::High;
        }
        else if (Distance <= MediumDetailRadius)
        {
            NewLOD = ECrowd_LODLevel::Medium;
        }
        else if (Distance <= LowDetailRadius)
        {
            NewLOD = ECrowd_LODLevel::Low;
        }
        else
        {
            NewLOD = ECrowd_LODLevel::Culled;
        }
        
        if (Group.LODLevel != NewLOD)
        {
            Group.LODLevel = NewLOD;
            // Update rendering/simulation detail based on LOD
        }
    }
}

void UCrowdSimulationSubsystem::SetGlobalCrowdDensity(float DensityMultiplier)
{
    DensityMultiplier = FMath::Clamp(DensityMultiplier, 0.1f, 2.0f);
    
    for (auto& CrowdPair : ActiveCrowdGroups)
    {
        FCrowd_GroupData& Group = CrowdPair.Value;
        // Adjust agent count based on density
        int32 NewAgentCount = FMath::RoundToInt(Group.AgentCount * DensityMultiplier);
        Group.AgentCount = FMath::Max(1, NewAgentCount);
    }
    
    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationSubsystem: Set global density to %.2f"), DensityMultiplier);
}

TArray<int32> UCrowdSimulationSubsystem::GetCrowdGroupsInRadius(FVector Center, float Radius)
{
    TArray<int32> Result;
    float RadiusSquared = Radius * Radius;
    
    for (const auto& CrowdPair : ActiveCrowdGroups)
    {
        const FCrowd_GroupData& Group = CrowdPair.Value;
        if (Group.bIsActive)
        {
            float DistanceSquared = FVector::DistSquared(Center, Group.CenterLocation);
            if (DistanceSquared <= RadiusSquared)
            {
                Result.Add(Group.GroupID);
            }
        }
    }
    
    return Result;
}

int32 UCrowdSimulationSubsystem::GetTotalActiveAgents() const
{
    int32 Total = 0;
    for (const auto& CrowdPair : ActiveCrowdGroups)
    {
        if (CrowdPair.Value.bIsActive)
        {
            Total += CrowdPair.Value.AgentCount;
        }
    }
    return Total;
}

ECrowd_BehaviorState UCrowdSimulationSubsystem::GetCrowdBehavior(int32 CrowdID) const
{
    if (const FCrowd_GroupData* Group = ActiveCrowdGroups.Find(CrowdID))
    {
        return Group->CurrentBehavior;
    }
    return ECrowd_BehaviorState::Idle;
}

void UCrowdSimulationSubsystem::UpdateLODSystem()
{
    // Get player location for LOD calculations
    if (UWorld* World = GetWorld())
    {
        if (APawn* PlayerPawn = World->GetFirstPlayerController()->GetPawn())
        {
            UpdateCrowdLOD(PlayerPawn->GetActorLocation());
        }
    }
    
    // Update crowd group positions and behaviors
    float DeltaTime = LODUpdateInterval;
    for (auto& CrowdPair : ActiveCrowdGroups)
    {
        FCrowd_GroupData& Group = CrowdPair.Value;
        
        if (Group.bIsActive && Group.bHasTarget && Group.MovementSpeed > 0.0f)
        {
            // Move towards target
            FVector Direction = (Group.TargetLocation - Group.CenterLocation).GetSafeNormal();
            float MoveDistance = Group.MovementSpeed * DeltaTime;
            
            if (FVector::Dist(Group.CenterLocation, Group.TargetLocation) <= MoveDistance)
            {
                // Reached target
                Group.CenterLocation = Group.TargetLocation;
                Group.bHasTarget = false;
                SetCrowdBehavior(Group.GroupID, ECrowd_BehaviorState::Idle);
            }
            else
            {
                // Move towards target
                Group.CenterLocation += Direction * MoveDistance;
            }
        }
    }
}

void UCrowdSimulationSubsystem::InitializeMassEntity()
{
    if (UWorld* World = GetWorld())
    {
        // Try to get Mass Entity subsystem
        // Note: This is a placeholder for Mass Entity integration
        // The actual implementation would depend on the Mass Entity plugin API
        UE_LOG(LogTemp, Log, TEXT("CrowdSimulationSubsystem: Mass Entity integration initialized"));
    }
}

void UCrowdSimulationSubsystem::CleanupCrowdGroups()
{
    for (auto& CrowdPair : ActiveCrowdGroups)
    {
        FCrowd_GroupData& Group = CrowdPair.Value;
        Group.bIsActive = false;
    }
    
    ActiveCrowdGroups.Empty();
    CrowdToMassEntityMap.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationSubsystem: Cleaned up all crowd groups"));
}