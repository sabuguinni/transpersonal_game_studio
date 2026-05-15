#include "Crowd_MassSimulationManager.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassRepresentationFragments.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

ACrowd_MassSimulationManager::ACrowd_MassSimulationManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.033f; // 30 FPS for crowd simulation
    
    // Initialize default simulation settings
    SimulationSettings.MaxAgents = 1000;
    SimulationSettings.SpawnRadius = 5000.0f;
    SimulationSettings.MovementSpeed = 150.0f;
    SimulationSettings.LODDistance1 = 1000.0f;
    SimulationSettings.LODDistance2 = 2500.0f;
    SimulationSettings.LODDistance3 = 5000.0f;
    SimulationSettings.bEnableFlocking = true;
    SimulationSettings.FlockingRadius = 300.0f;
    SimulationSettings.SeparationWeight = 2.0f;
    SimulationSettings.AlignmentWeight = 1.0f;
    SimulationSettings.CohesionWeight = 1.0f;
    
    // Create default agent group
    FCrowd_AgentGroup DefaultGroup;
    DefaultGroup.GroupName = TEXT("PrehistoricHumans");
    DefaultGroup.AgentCount = 50;
    DefaultGroup.SpawnLocation = FVector::ZeroVector;
    DefaultGroup.GroupRadius = 1000.0f;
    DefaultGroup.GroupMovementSpeed = 120.0f;
    DefaultGroup.PreferredBiome = BiomeType::Savanna;
    DefaultGroup.bIsNomadic = true;
    AgentGroups.Add(DefaultGroup);
    
    MassEntitySubsystem = nullptr;
    bIsInitialized = false;
    ActiveAgentCount = 0;
    CurrentSimulationTime = 0.0f;
    LastFrameTime = 0.0f;
    CurrentLODLevel = 0;
}

void ACrowd_MassSimulationManager::BeginPlay()
{
    Super::BeginPlay();
    
    if (UWorld* World = GetWorld())
    {
        MassEntitySubsystem = World->GetSubsystem<UMassEntitySubsystem>();
        if (MassEntitySubsystem)
        {
            InitializeMassEntity();
            if (bSimulationActive)
            {
                StartSimulation();
            }
            UE_LOG(LogTemp, Warning, TEXT("Crowd Mass Simulation Manager initialized with %d agent groups"), AgentGroups.Num());
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to get MassEntitySubsystem"));
        }
    }
}

void ACrowd_MassSimulationManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    StopSimulation();
    CleanupEntities();
    Super::EndPlay(EndPlayReason);
}

void ACrowd_MassSimulationManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (!bSimulationActive || !bIsInitialized)
    {
        return;
    }
    
    CurrentSimulationTime += DeltaTime;
    UpdateSimulationMetrics(DeltaTime);
    
    if (SimulationSettings.bEnableFlocking)
    {
        UpdateFlockingBehavior(DeltaTime);
    }
    
    if (bUsePerformanceLOD)
    {
        UpdateAgentLOD();
        ProcessPerformanceOptimization();
    }
}

void ACrowd_MassSimulationManager::StartSimulation()
{
    if (!MassEntitySubsystem || !bIsInitialized)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot start simulation - Mass Entity not initialized"));
        return;
    }
    
    bSimulationActive = true;
    
    // Spawn all configured agent groups
    for (const FCrowd_AgentGroup& Group : AgentGroups)
    {
        SpawnAgentGroup(Group);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd simulation started with %d active agents"), ActiveAgentCount);
}

void ACrowd_MassSimulationManager::StopSimulation()
{
    bSimulationActive = false;
    CleanupEntities();
    ActiveAgentCount = 0;
    CurrentSimulationTime = 0.0f;
    GroupEntityMap.Empty();
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd simulation stopped"));
}

void ACrowd_MassSimulationManager::PauseSimulation()
{
    bSimulationActive = false;
    UE_LOG(LogTemp, Warning, TEXT("Crowd simulation paused"));
}

void ACrowd_MassSimulationManager::ResumeSimulation()
{
    if (bIsInitialized)
    {
        bSimulationActive = true;
        UE_LOG(LogTemp, Warning, TEXT("Crowd simulation resumed"));
    }
}

void ACrowd_MassSimulationManager::SpawnAgentGroup(const FCrowd_AgentGroup& GroupSettings)
{
    if (!MassEntitySubsystem || !CrowdArchetype.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot spawn agent group - Mass Entity not properly initialized"));
        return;
    }
    
    TArray<FMassEntityHandle> GroupEntities;
    
    for (int32 i = 0; i < GroupSettings.AgentCount; ++i)
    {
        // Calculate spawn position within group radius
        float Angle = FMath::RandRange(0.0f, 2.0f * PI);
        float Distance = FMath::RandRange(0.0f, GroupSettings.GroupRadius);
        FVector SpawnOffset = FVector(
            FMath::Cos(Angle) * Distance,
            FMath::Sin(Angle) * Distance,
            0.0f
        );
        FVector SpawnLocation = GroupSettings.SpawnLocation + SpawnOffset;
        
        // Create entity
        FMassEntityHandle EntityHandle = MassEntitySubsystem->CreateEntity(CrowdArchetype);
        if (EntityHandle.IsValid())
        {
            // Set transform
            if (FMassTransformFragment* TransformFragment = MassEntitySubsystem->GetFragmentDataPtr<FMassTransformFragment>(EntityHandle))
            {
                TransformFragment->SetTransform(FTransform(FRotator::ZeroRotator, SpawnLocation, FVector::OneVector));
            }
            
            // Set velocity
            if (FMassVelocityFragment* VelocityFragment = MassEntitySubsystem->GetFragmentDataPtr<FMassVelocityFragment>(EntityHandle))
            {
                FVector RandomDirection = FVector(FMath::RandRange(-1.0f, 1.0f), FMath::RandRange(-1.0f, 1.0f), 0.0f).GetSafeNormal();
                VelocityFragment->Value = RandomDirection * GroupSettings.GroupMovementSpeed;
            }
            
            GroupEntities.Add(EntityHandle);
            SpawnedEntities.Add(EntityHandle);
        }
    }
    
    GroupEntityMap.Add(GroupSettings.GroupName, GroupEntities);
    ActiveAgentCount += GroupEntities.Num();
    
    UE_LOG(LogTemp, Warning, TEXT("Spawned agent group '%s' with %d agents"), *GroupSettings.GroupName, GroupEntities.Num());
}

void ACrowd_MassSimulationManager::RemoveAgentGroup(const FString& GroupName)
{
    if (TArray<FMassEntityHandle>* GroupEntities = GroupEntityMap.Find(GroupName))
    {
        for (const FMassEntityHandle& Entity : *GroupEntities)
        {
            if (Entity.IsValid() && MassEntitySubsystem)
            {
                MassEntitySubsystem->DestroyEntity(Entity);
                SpawnedEntities.Remove(Entity);
                ActiveAgentCount--;
            }
        }
        GroupEntityMap.Remove(GroupName);
        
        UE_LOG(LogTemp, Warning, TEXT("Removed agent group '%s'"), *GroupName);
    }
}

void ACrowd_MassSimulationManager::UpdateLODSettings()
{
    if (!MassEntitySubsystem)
    {
        return;
    }
    
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        return;
    }
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    for (const FMassEntityHandle& Entity : SpawnedEntities)
    {
        if (!Entity.IsValid())
        {
            continue;
        }
        
        if (FMassTransformFragment* TransformFragment = MassEntitySubsystem->GetFragmentDataPtr<FMassTransformFragment>(Entity))
        {
            float Distance = FVector::Dist(PlayerLocation, TransformFragment->GetTransform().GetLocation());
            
            int32 LODLevel = 0;
            if (Distance > SimulationSettings.LODDistance3)
            {
                LODLevel = 3;
            }
            else if (Distance > SimulationSettings.LODDistance2)
            {
                LODLevel = 2;
            }
            else if (Distance > SimulationSettings.LODDistance1)
            {
                LODLevel = 1;
            }
            
            // Apply LOD-based optimizations
            if (LODLevel >= 2)
            {
                // Reduce update frequency for distant agents
                // This would be implemented with Mass Entity processors
            }
        }
    }
}

float ACrowd_MassSimulationManager::GetCurrentFrameTime() const
{
    return LastFrameTime;
}

void ACrowd_MassSimulationManager::SetSimulationSettings(const FCrowd_SimulationSettings& NewSettings)
{
    SimulationSettings = NewSettings;
    UpdateLODSettings();
    UE_LOG(LogTemp, Warning, TEXT("Simulation settings updated"));
}

void ACrowd_MassSimulationManager::InitializeMassEntity()
{
    if (!MassEntitySubsystem)
    {
        return;
    }
    
    CreateAgentArchetypes();
    bIsInitialized = true;
    
    UE_LOG(LogTemp, Warning, TEXT("Mass Entity system initialized for crowd simulation"));
}

void ACrowd_MassSimulationManager::CreateAgentArchetypes()
{
    if (!MassEntitySubsystem)
    {
        return;
    }
    
    // Create archetype with required fragments
    FMassArchetypeCompositionDescriptor CompositionDescriptor;
    CompositionDescriptor.Fragments.Add<FMassTransformFragment>();
    CompositionDescriptor.Fragments.Add<FMassVelocityFragment>();
    CompositionDescriptor.Fragments.Add<FMassRepresentationFragment>();
    
    CrowdArchetype = MassEntitySubsystem->CreateArchetype(CompositionDescriptor);
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd archetype created successfully"));
}

void ACrowd_MassSimulationManager::UpdateFlockingBehavior(float DeltaTime)
{
    if (!MassEntitySubsystem || SpawnedEntities.Num() == 0)
    {
        return;
    }
    
    // Update flocking behavior for all entities
    for (int32 i = 0; i < SpawnedEntities.Num(); ++i)
    {
        const FMassEntityHandle& Entity = SpawnedEntities[i];
        if (!Entity.IsValid())
        {
            continue;
        }
        
        // Find nearby entities for flocking calculations
        TArray<FMassEntityHandle> NearbyEntities;
        if (FMassTransformFragment* TransformFragment = MassEntitySubsystem->GetFragmentDataPtr<FMassTransformFragment>(Entity))
        {
            FVector EntityLocation = TransformFragment->GetTransform().GetLocation();
            
            for (int32 j = 0; j < SpawnedEntities.Num(); ++j)
            {
                if (i == j) continue;
                
                const FMassEntityHandle& OtherEntity = SpawnedEntities[j];
                if (!OtherEntity.IsValid()) continue;
                
                if (FMassTransformFragment* OtherTransform = MassEntitySubsystem->GetFragmentDataPtr<FMassTransformFragment>(OtherEntity))
                {
                    float Distance = FVector::Dist(EntityLocation, OtherTransform->GetTransform().GetLocation());
                    if (Distance <= SimulationSettings.FlockingRadius)
                    {
                        NearbyEntities.Add(OtherEntity);
                    }
                }
            }
            
            // Calculate and apply flocking force
            FVector FlockingForce = CalculateFlockingForce(Entity, NearbyEntities);
            
            if (FMassVelocityFragment* VelocityFragment = MassEntitySubsystem->GetFragmentDataPtr<FMassVelocityFragment>(Entity))
            {
                VelocityFragment->Value += FlockingForce * DeltaTime;
                VelocityFragment->Value = VelocityFragment->Value.GetClampedToMaxSize(SimulationSettings.MovementSpeed);
            }
        }
    }
}

void ACrowd_MassSimulationManager::UpdateAgentLOD()
{
    // LOD updates are handled in UpdateLODSettings()
    UpdateLODSettings();
}

void ACrowd_MassSimulationManager::ProcessPerformanceOptimization()
{
    if (LastFrameTime > PerformanceThreshold)
    {
        // Reduce simulation quality if performance drops
        if (CurrentLODLevel < 3)
        {
            CurrentLODLevel++;
            UE_LOG(LogTemp, Warning, TEXT("Performance optimization: increased LOD level to %d"), CurrentLODLevel);
        }
    }
    else if (LastFrameTime < PerformanceThreshold * 0.8f && CurrentLODLevel > 0)
    {
        // Increase quality if performance allows
        CurrentLODLevel--;
        UE_LOG(LogTemp, Warning, TEXT("Performance optimization: decreased LOD level to %d"), CurrentLODLevel);
    }
}

void ACrowd_MassSimulationManager::CleanupEntities()
{
    if (MassEntitySubsystem)
    {
        for (const FMassEntityHandle& Entity : SpawnedEntities)
        {
            if (Entity.IsValid())
            {
                MassEntitySubsystem->DestroyEntity(Entity);
            }
        }
    }
    
    SpawnedEntities.Empty();
    GroupEntityMap.Empty();
}

void ACrowd_MassSimulationManager::UpdateSimulationMetrics(float DeltaTime)
{
    LastFrameTime = DeltaTime * 1000.0f; // Convert to milliseconds
    
    // Update active agent count (remove invalid entities)
    int32 ValidEntityCount = 0;
    for (int32 i = SpawnedEntities.Num() - 1; i >= 0; --i)
    {
        if (SpawnedEntities[i].IsValid())
        {
            ValidEntityCount++;
        }
        else
        {
            SpawnedEntities.RemoveAt(i);
        }
    }
    ActiveAgentCount = ValidEntityCount;
}

FVector ACrowd_MassSimulationManager::CalculateFlockingForce(const FMassEntityHandle& Entity, const TArray<FMassEntityHandle>& NearbyEntities)
{
    if (!MassEntitySubsystem || NearbyEntities.Num() == 0)
    {
        return FVector::ZeroVector;
    }
    
    FMassTransformFragment* EntityTransform = MassEntitySubsystem->GetFragmentDataPtr<FMassTransformFragment>(Entity);
    FMassVelocityFragment* EntityVelocity = MassEntitySubsystem->GetFragmentDataPtr<FMassVelocityFragment>(Entity);
    
    if (!EntityTransform || !EntityVelocity)
    {
        return FVector::ZeroVector;
    }
    
    FVector EntityLocation = EntityTransform->GetTransform().GetLocation();
    FVector EntityVel = EntityVelocity->Value;
    
    // Separation force
    FVector SeparationForce = FVector::ZeroVector;
    FVector AlignmentForce = FVector::ZeroVector;
    FVector CohesionForce = FVector::ZeroVector;
    
    FVector CenterOfMass = FVector::ZeroVector;
    FVector AverageVelocity = FVector::ZeroVector;
    
    for (const FMassEntityHandle& NearbyEntity : NearbyEntities)
    {
        if (FMassTransformFragment* NearbyTransform = MassEntitySubsystem->GetFragmentDataPtr<FMassTransformFragment>(NearbyEntity))
        {
            FVector NearbyLocation = NearbyTransform->GetTransform().GetLocation();
            FVector ToNearby = NearbyLocation - EntityLocation;
            float Distance = ToNearby.Size();
            
            if (Distance > 0.0f)
            {
                // Separation
                SeparationForce -= ToNearby.GetSafeNormal() / Distance;
                
                // Cohesion
                CenterOfMass += NearbyLocation;
                
                // Alignment
                if (FMassVelocityFragment* NearbyVelocity = MassEntitySubsystem->GetFragmentDataPtr<FMassVelocityFragment>(NearbyEntity))
                {
                    AverageVelocity += NearbyVelocity->Value;
                }
            }
        }
    }
    
    if (NearbyEntities.Num() > 0)
    {
        CenterOfMass /= NearbyEntities.Num();
        CohesionForce = (CenterOfMass - EntityLocation).GetSafeNormal();
        
        AverageVelocity /= NearbyEntities.Num();
        AlignmentForce = (AverageVelocity - EntityVel).GetSafeNormal();
    }
    
    // Combine forces with weights
    FVector TotalForce = 
        SeparationForce * SimulationSettings.SeparationWeight +
        AlignmentForce * SimulationSettings.AlignmentWeight +
        CohesionForce * SimulationSettings.CohesionWeight;
    
    return TotalForce.GetClampedToMaxSize(SimulationSettings.MovementSpeed * 0.1f);
}