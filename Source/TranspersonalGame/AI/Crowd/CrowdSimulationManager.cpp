// CrowdSimulationManager.cpp
// Agent #13 — Crowd & Traffic Simulation
// Implements Mass AI crowd simulation for prehistoric survival game
// Supports up to 50,000 simultaneous agents with LOD chain

#include "CrowdSimulationManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "NavMesh/NavMeshBoundsVolume.h"

// ============================================================
// UCrowdAgentComponent Implementation
// ============================================================

UCrowdAgentComponent::UCrowdAgentComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz update for performance

    AgentType = ECrowd_AgentType::HerbivoreHerd;
    CurrentState = ECrowd_AgentState::Idle;
    MovementSpeed = 200.0f;
    DetectionRadius = 1500.0f;
    FlockingRadius = 300.0f;
    bIsLODActive = true;
    LODLevel = 0;
    FlockID = -1;
    TargetLocation = FVector::ZeroVector;
    bHasTarget = false;
}

void UCrowdAgentComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Register with crowd manager
    if (UWorld* World = GetWorld())
    {
        if (ACrowdSimulationManager* Manager = Cast<ACrowdSimulationManager>(
            UGameplayStatics::GetActorOfClass(World, ACrowdSimulationManager::StaticClass())))
        {
            Manager->RegisterAgent(this);
        }
    }
    
    // Start behavior tick
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            BehaviorTimerHandle,
            this,
            &UCrowdAgentComponent::UpdateBehavior,
            0.5f,
            true
        );
    }
}

void UCrowdAgentComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bIsLODActive) return;
    
    // Move toward target
    if (bHasTarget && GetOwner())
    {
        FVector CurrentPos = GetOwner()->GetActorLocation();
        FVector Direction = (TargetLocation - CurrentPos).GetSafeNormal();
        float DistToTarget = FVector::Dist(CurrentPos, TargetLocation);
        
        if (DistToTarget > 50.0f)
        {
            FVector NewPos = CurrentPos + Direction * MovementSpeed * DeltaTime;
            GetOwner()->SetActorLocation(NewPos, true);
            
            // Face movement direction
            if (!Direction.IsNearlyZero())
            {
                FRotator NewRot = Direction.Rotation();
                GetOwner()->SetActorRotation(NewRot);
            }
        }
        else
        {
            // Reached target
            bHasTarget = false;
            CurrentState = ECrowd_AgentState::Idle;
            OnTargetReached();
        }
    }
}

void UCrowdAgentComponent::UpdateBehavior()
{
    if (!GetOwner()) return;
    
    switch (AgentType)
    {
        case ECrowd_AgentType::HerbivoreHerd:
            UpdateHerdbehavior();
            break;
        case ECrowd_AgentType::PredatorPack:
            UpdatePredatorBehavior();
            break;
        case ECrowd_AgentType::ScavengerGroup:
            UpdateScavengerBehavior();
            break;
        case ECrowd_AgentType::MigrationColumn:
            UpdateMigrationBehavior();
            break;
        default:
            break;
    }
}

void UCrowdAgentComponent::UpdateHerdbehavior()
{
    // Herbivores: graze, flock together, flee from predators
    if (CurrentState == ECrowd_AgentState::Idle)
    {
        // Random wander within flock radius
        if (FMath::RandBool())
        {
            FVector RandomOffset = FVector(
                FMath::RandRange(-FlockingRadius, FlockingRadius),
                FMath::RandRange(-FlockingRadius, FlockingRadius),
                0.0f
            );
            SetTargetLocation(GetOwner()->GetActorLocation() + RandomOffset);
            CurrentState = ECrowd_AgentState::Wandering;
        }
    }
}

void UCrowdAgentComponent::UpdatePredatorBehavior()
{
    // Predators: patrol, stalk, coordinate pack attacks
    if (CurrentState == ECrowd_AgentState::Idle || CurrentState == ECrowd_AgentState::Wandering)
    {
        // Patrol in wider radius
        FVector PatrolOffset = FVector(
            FMath::RandRange(-800.0f, 800.0f),
            FMath::RandRange(-800.0f, 800.0f),
            0.0f
        );
        SetTargetLocation(GetOwner()->GetActorLocation() + PatrolOffset);
        CurrentState = ECrowd_AgentState::Patrolling;
    }
}

void UCrowdAgentComponent::UpdateScavengerBehavior()
{
    // Scavengers: move toward food sources, avoid large predators
    if (CurrentState == ECrowd_AgentState::Idle)
    {
        FVector ScavengeOffset = FVector(
            FMath::RandRange(-400.0f, 400.0f),
            FMath::RandRange(-400.0f, 400.0f),
            0.0f
        );
        SetTargetLocation(GetOwner()->GetActorLocation() + ScavengeOffset);
        CurrentState = ECrowd_AgentState::Wandering;
    }
}

void UCrowdAgentComponent::UpdateMigrationBehavior()
{
    // Migration: follow column leader, maintain spacing
    if (CurrentState == ECrowd_AgentState::Idle && bHasTarget == false)
    {
        // Continue along migration path
        FVector MigrationDir = FVector(1.0f, 0.2f, 0.0f).GetSafeNormal();
        SetTargetLocation(GetOwner()->GetActorLocation() + MigrationDir * 500.0f);
        CurrentState = ECrowd_AgentState::Migrating;
    }
}

void UCrowdAgentComponent::SetTargetLocation(const FVector& NewTarget)
{
    TargetLocation = NewTarget;
    bHasTarget = true;
}

void UCrowdAgentComponent::OnTargetReached()
{
    // Reset to idle after reaching target — behavior update will pick next action
    CurrentState = ECrowd_AgentState::Idle;
}

void UCrowdAgentComponent::SetLODLevel(int32 NewLOD)
{
    LODLevel = NewLOD;
    
    switch (LODLevel)
    {
        case 0: // Full simulation — close range
            PrimaryComponentTick.TickInterval = 0.05f;
            MovementSpeed = BaseMovementSpeed;
            bIsLODActive = true;
            break;
        case 1: // Reduced simulation — medium range
            PrimaryComponentTick.TickInterval = 0.2f;
            MovementSpeed = BaseMovementSpeed * 0.8f;
            bIsLODActive = true;
            break;
        case 2: // Minimal simulation — far range
            PrimaryComponentTick.TickInterval = 0.5f;
            MovementSpeed = BaseMovementSpeed * 0.5f;
            bIsLODActive = true;
            break;
        case 3: // Frozen — very far, no movement
            bIsLODActive = false;
            break;
    }
}

void UCrowdAgentComponent::FleeFromThreat(const FVector& ThreatLocation)
{
    FVector FleeDir = (GetOwner()->GetActorLocation() - ThreatLocation).GetSafeNormal();
    FVector FleeTarget = GetOwner()->GetActorLocation() + FleeDir * DetectionRadius;
    SetTargetLocation(FleeTarget);
    CurrentState = ECrowd_AgentState::Fleeing;
    MovementSpeed = BaseMovementSpeed * 2.0f; // Sprint when fleeing
}

// ============================================================
// ACrowdSimulationManager Implementation
// ============================================================

ACrowdSimulationManager::ACrowdSimulationManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.25f; // 4Hz manager update

    MaxAgents = 50000;
    LODDistance_Near = 2000.0f;
    LODDistance_Mid = 5000.0f;
    LODDistance_Far = 10000.0f;
    LODDistance_Cull = 20000.0f;
    bEnableLODSystem = true;
    bEnableFlocking = true;
    bEnablePredatorPreyDynamics = true;
    TotalRegisteredAgents = 0;
}

void ACrowdSimulationManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize flock groups
    InitializeFlockGroups();
    
    // Start LOD update timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            LODUpdateTimer,
            this,
            &ACrowdSimulationManager::UpdateLODForAllAgents,
            1.0f,
            true
        );
        
        World->GetTimerManager().SetTimer(
            PredatorPreyTimer,
            this,
            &ACrowdSimulationManager::UpdatePredatorPreyInteractions,
            2.0f,
            true
        );
    }
    
    UE_LOG(LogTemp, Log, TEXT("[CrowdSimulation] Manager initialized. Max agents: %d"), MaxAgents);
}

void ACrowdSimulationManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bEnableFlocking)
    {
        UpdateFlockingBehavior(DeltaTime);
    }
}

void ACrowdSimulationManager::RegisterAgent(UCrowdAgentComponent* Agent)
{
    if (!Agent) return;
    
    if (TotalRegisteredAgents >= MaxAgents)
    {
        UE_LOG(LogTemp, Warning, TEXT("[CrowdSimulation] Max agent limit reached: %d"), MaxAgents);
        return;
    }
    
    AllAgents.Add(Agent);
    TotalRegisteredAgents++;
    
    // Assign to flock based on type
    AssignAgentToFlock(Agent);
    
    UE_LOG(LogTemp, Verbose, TEXT("[CrowdSimulation] Agent registered. Total: %d"), TotalRegisteredAgents);
}

void ACrowdSimulationManager::UnregisterAgent(UCrowdAgentComponent* Agent)
{
    if (!Agent) return;
    AllAgents.Remove(Agent);
    TotalRegisteredAgents = FMath::Max(0, TotalRegisteredAgents - 1);
}

void ACrowdSimulationManager::InitializeFlockGroups()
{
    // Create default flock groups for each agent type
    FCrowd_FlockGroup HerbivoreGroup;
    HerbivoreGroup.FlockID = 0;
    HerbivoreGroup.FlockType = ECrowd_AgentType::HerbivoreHerd;
    HerbivoreGroup.FlockCenter = FVector::ZeroVector;
    HerbivoreGroup.MaxFlockSize = 50;
    HerbivoreGroup.SeparationRadius = 150.0f;
    HerbivoreGroup.CohesionRadius = 400.0f;
    HerbivoreGroup.AlignmentRadius = 300.0f;
    FlockGroups.Add(HerbivoreGroup);

    FCrowd_FlockGroup PredatorGroup;
    PredatorGroup.FlockID = 1;
    PredatorGroup.FlockType = ECrowd_AgentType::PredatorPack;
    PredatorGroup.FlockCenter = FVector::ZeroVector;
    PredatorGroup.MaxFlockSize = 8;
    PredatorGroup.SeparationRadius = 200.0f;
    PredatorGroup.CohesionRadius = 600.0f;
    PredatorGroup.AlignmentRadius = 400.0f;
    FlockGroups.Add(PredatorGroup);

    FCrowd_FlockGroup MigrationGroup;
    MigrationGroup.FlockID = 2;
    MigrationGroup.FlockType = ECrowd_AgentType::MigrationColumn;
    MigrationGroup.FlockCenter = FVector::ZeroVector;
    MigrationGroup.MaxFlockSize = 200;
    MigrationGroup.SeparationRadius = 100.0f;
    MigrationGroup.CohesionRadius = 800.0f;
    MigrationGroup.AlignmentRadius = 600.0f;
    FlockGroups.Add(MigrationGroup);
}

void ACrowdSimulationManager::AssignAgentToFlock(UCrowdAgentComponent* Agent)
{
    if (!Agent) return;
    
    int32 TargetFlockID = -1;
    
    switch (Agent->AgentType)
    {
        case ECrowd_AgentType::HerbivoreHerd:
            TargetFlockID = 0;
            break;
        case ECrowd_AgentType::PredatorPack:
            TargetFlockID = 1;
            break;
        case ECrowd_AgentType::MigrationColumn:
            TargetFlockID = 2;
            break;
        default:
            TargetFlockID = 0;
            break;
    }
    
    Agent->FlockID = TargetFlockID;
    
    // Add to flock group
    for (FCrowd_FlockGroup& Group : FlockGroups)
    {
        if (Group.FlockID == TargetFlockID)
        {
            if (Group.Members.Num() < Group.MaxFlockSize)
            {
                Group.Members.Add(Agent);
            }
            break;
        }
    }
}

void ACrowdSimulationManager::UpdateLODForAllAgents()
{
    if (!bEnableLODSystem) return;
    
    APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
    if (!PC || !PC->GetPawn()) return;
    
    FVector PlayerLocation = PC->GetPawn()->GetActorLocation();
    
    for (UCrowdAgentComponent* Agent : AllAgents)
    {
        if (!Agent || !Agent->GetOwner()) continue;
        
        float Distance = FVector::Dist(PlayerLocation, Agent->GetOwner()->GetActorLocation());
        
        int32 NewLOD = 0;
        if (Distance > LODDistance_Cull)
        {
            // Beyond cull distance — hide actor
            Agent->GetOwner()->SetActorHiddenInGame(true);
            Agent->SetLODLevel(3);
        }
        else if (Distance > LODDistance_Far)
        {
            Agent->GetOwner()->SetActorHiddenInGame(false);
            NewLOD = 3;
            Agent->SetLODLevel(NewLOD);
        }
        else if (Distance > LODDistance_Mid)
        {
            Agent->GetOwner()->SetActorHiddenInGame(false);
            NewLOD = 2;
            Agent->SetLODLevel(NewLOD);
        }
        else if (Distance > LODDistance_Near)
        {
            Agent->GetOwner()->SetActorHiddenInGame(false);
            NewLOD = 1;
            Agent->SetLODLevel(NewLOD);
        }
        else
        {
            Agent->GetOwner()->SetActorHiddenInGame(false);
            NewLOD = 0;
            Agent->SetLODLevel(NewLOD);
        }
    }
}

void ACrowdSimulationManager::UpdateFlockingBehavior(float DeltaTime)
{
    for (FCrowd_FlockGroup& Group : FlockGroups)
    {
        if (Group.Members.Num() == 0) continue;
        
        // Calculate flock center
        FVector Center = FVector::ZeroVector;
        int32 ValidCount = 0;
        
        for (UCrowdAgentComponent* Member : Group.Members)
        {
            if (Member && Member->GetOwner())
            {
                Center += Member->GetOwner()->GetActorLocation();
                ValidCount++;
            }
        }
        
        if (ValidCount > 0)
        {
            Group.FlockCenter = Center / ValidCount;
        }
    }
}

void ACrowdSimulationManager::UpdatePredatorPreyInteractions()
{
    if (!bEnablePredatorPreyDynamics) return;
    
    // Find predator agents
    TArray<UCrowdAgentComponent*> Predators;
    TArray<UCrowdAgentComponent*> Prey;
    
    for (UCrowdAgentComponent* Agent : AllAgents)
    {
        if (!Agent) continue;
        if (Agent->AgentType == ECrowd_AgentType::PredatorPack)
            Predators.Add(Agent);
        else if (Agent->AgentType == ECrowd_AgentType::HerbivoreHerd || 
                 Agent->AgentType == ECrowd_AgentType::MigrationColumn)
            Prey.Add(Agent);
    }
    
    // Check predator-prey proximity — trigger flee behavior
    for (UCrowdAgentComponent* Predator : Predators)
    {
        if (!Predator || !Predator->GetOwner()) continue;
        FVector PredatorLoc = Predator->GetOwner()->GetActorLocation();
        
        for (UCrowdAgentComponent* PreyAgent : Prey)
        {
            if (!PreyAgent || !PreyAgent->GetOwner()) continue;
            
            float Distance = FVector::Dist(PredatorLoc, PreyAgent->GetOwner()->GetActorLocation());
            if (Distance < PreyAgent->DetectionRadius)
            {
                PreyAgent->FleeFromThreat(PredatorLoc);
            }
        }
    }
}

void ACrowdSimulationManager::BroadcastThreatEvent(const FVector& ThreatLocation, float ThreatRadius)
{
    for (UCrowdAgentComponent* Agent : AllAgents)
    {
        if (!Agent || !Agent->GetOwner()) continue;
        
        float Distance = FVector::Dist(ThreatLocation, Agent->GetOwner()->GetActorLocation());
        if (Distance < ThreatRadius)
        {
            Agent->FleeFromThreat(ThreatLocation);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("[CrowdSimulation] Threat broadcast at %s, radius %.0f. Agents alerted."),
        *ThreatLocation.ToString(), ThreatRadius);
}

int32 ACrowdSimulationManager::GetActiveAgentCount() const
{
    return AllAgents.Num();
}

int32 ACrowdSimulationManager::GetAgentCountByType(ECrowd_AgentType AgentType) const
{
    int32 Count = 0;
    for (const UCrowdAgentComponent* Agent : AllAgents)
    {
        if (Agent && Agent->AgentType == AgentType)
            Count++;
    }
    return Count;
}
