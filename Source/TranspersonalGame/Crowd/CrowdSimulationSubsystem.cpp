#include "CrowdSimulationSubsystem.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "MassCommandBuffer.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

void UCrowdSimulationSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // Get Mass Entity subsystem references
    MassEntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
    MassSpawnerSubsystem = GetWorld()->GetSubsystem<UMassSpawnerSubsystem>();

    // Initialize default herd configurations
    InitializeDefaultHerdConfigs();

    UE_LOG(LogTemp, Warning, TEXT("Crowd Simulation Subsystem Initialized - Ready for up to %d agents"), MaxSimultaneousAgents);
}

void UCrowdSimulationSubsystem::Deinitialize()
{
    // Clean up all active herds
    for (auto& HerdPair : ActiveHerds)
    {
        for (FEntityHandle& Entity : HerdPair.Value)
        {
            if (MassEntitySubsystem && MassEntitySubsystem->IsEntityValid(Entity))
            {
                MassEntitySubsystem->DestroyEntity(Entity);
            }
        }
    }
    
    ActiveHerds.Empty();
    ActiveMigrations.Empty();

    Super::Deinitialize();
}

bool UCrowdSimulationSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    return Super::ShouldCreateSubsystem(Outer);
}

void UCrowdSimulationSubsystem::SpawnHerd(const FHerdConfiguration& Config, const FVector& SpawnLocation)
{
    if (!MassEntitySubsystem || !MassSpawnerSubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("Mass Entity subsystems not available"));
        return;
    }

    // Check if we're at capacity
    if (CurrentActiveAgents >= MaxSimultaneousAgents)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot spawn herd - at maximum capacity (%d agents)"), MaxSimultaneousAgents);
        return;
    }

    // Calculate herd size
    int32 HerdSize = FMath::RandRange(Config.MinHerdSize, Config.MaxHerdSize);
    
    // Adjust for capacity
    int32 RemainingCapacity = MaxSimultaneousAgents - CurrentActiveAgents;
    HerdSize = FMath::Min(HerdSize, RemainingCapacity);

    TArray<FEntityHandle> NewHerd;
    NewHerd.Reserve(HerdSize);

    // Spawn individual agents in formation
    for (int32 i = 0; i < HerdSize; i++)
    {
        // Create formation position
        float Angle = (2.0f * PI * i) / HerdSize;
        float Distance = FMath::RandRange(0.0f, Config.CohesionRadius * 0.5f);
        FVector AgentLocation = SpawnLocation + FVector(
            Distance * FMath::Cos(Angle),
            Distance * FMath::Sin(Angle),
            0.0f
        );

        // Create entity using Mass framework
        FEntityHandle NewEntity = MassEntitySubsystem->CreateEntity();
        if (MassEntitySubsystem->IsEntityValid(NewEntity))
        {
            // Add required fragments for crowd simulation
            // This would be expanded with actual Mass fragments
            NewHerd.Add(NewEntity);
        }
    }

    // Store the herd
    if (!ActiveHerds.Contains(Config.HerdType))
    {
        ActiveHerds.Add(Config.HerdType, TArray<FEntityHandle>());
    }
    ActiveHerds[Config.HerdType].Append(NewHerd);

    CurrentActiveAgents += NewHerd.Num();

    UE_LOG(LogTemp, Log, TEXT("Spawned herd of %d %s at %s"), 
        NewHerd.Num(), 
        *UEnum::GetValueAsString(Config.HerdType),
        *SpawnLocation.ToString());
}

void UCrowdSimulationSubsystem::TriggerPanicInRadius(const FVector& Location, float Radius, float Duration)
{
    // Find all agents within radius and set them to panic state
    for (auto& HerdPair : ActiveHerds)
    {
        for (FEntityHandle& Entity : HerdPair.Value)
        {
            if (MassEntitySubsystem && MassEntitySubsystem->IsEntityValid(Entity))
            {
                // Get entity location and check distance
                // This would use actual Mass fragment data
                // For now, we'll simulate the panic trigger
                
                // Set panic behavior on affected entities
                // Implementation would use Mass processors
            }
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Panic triggered at %s with radius %.1f for %.1f seconds"), 
        *Location.ToString(), Radius, Duration);
}

void UCrowdSimulationSubsystem::StartMigration(EDinosaurHerdType HerdType, const FMigrationRoute& Route)
{
    if (!ActiveHerds.Contains(HerdType))
    {
        UE_LOG(LogTemp, Warning, TEXT("No herds of type %s to migrate"), *UEnum::GetValueAsString(HerdType));
        return;
    }

    // Add to active migrations
    ActiveMigrations.Add(Route);

    // Set migration behavior on all herds of this type
    TArray<FEntityHandle>& Herds = ActiveHerds[HerdType];
    for (FEntityHandle& Entity : Herds)
    {
        if (MassEntitySubsystem && MassEntitySubsystem->IsEntityValid(Entity))
        {
            // Set migration target and behavior
            // Implementation would use Mass movement processors
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Started migration for %s herds with %d waypoints"), 
        *UEnum::GetValueAsString(HerdType), Route.Waypoints.Num());
}

void UCrowdSimulationSubsystem::SetGlobalHerdBehavior(EHerdBehaviorState NewState)
{
    EHerdBehaviorState PreviousState = GlobalBehaviorState;
    GlobalBehaviorState = NewState;

    // Apply behavior change to all active herds
    for (auto& HerdPair : ActiveHerds)
    {
        for (FEntityHandle& Entity : HerdPair.Value)
        {
            if (MassEntitySubsystem && MassEntitySubsystem->IsEntityValid(Entity))
            {
                // Update entity behavior state
                // Implementation would use Mass behavior processors
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Global herd behavior changed from %s to %s"), 
        *UEnum::GetValueAsString(PreviousState),
        *UEnum::GetValueAsString(NewState));
}

int32 UCrowdSimulationSubsystem::GetTotalActiveAgents() const
{
    return CurrentActiveAgents;
}

TArray<FVector> UCrowdSimulationSubsystem::GetHerdLocations(EDinosaurHerdType HerdType) const
{
    TArray<FVector> Locations;
    
    if (ActiveHerds.Contains(HerdType))
    {
        const TArray<FEntityHandle>& Herds = ActiveHerds[HerdType];
        for (const FEntityHandle& Entity : Herds)
        {
            if (MassEntitySubsystem && MassEntitySubsystem->IsEntityValid(Entity))
            {
                // Get entity location from Mass transform fragment
                // For now, return placeholder
                Locations.Add(FVector::ZeroVector);
            }
        }
    }
    
    return Locations;
}

bool UCrowdSimulationSubsystem::IsAreaCrowded(const FVector& Location, float Radius, int32 Threshold) const
{
    int32 AgentCount = 0;
    
    for (const auto& HerdPair : ActiveHerds)
    {
        for (const FEntityHandle& Entity : HerdPair.Value)
        {
            if (MassEntitySubsystem && MassEntitySubsystem->IsEntityValid(Entity))
            {
                // Check distance to location
                // Implementation would use spatial queries
                AgentCount++;
                
                if (AgentCount >= Threshold)
                {
                    return true;
                }
            }
        }
    }
    
    return false;
}

void UCrowdSimulationSubsystem::OnPredatorDetected(const FVector& PredatorLocation, float ThreatRadius)
{
    // Trigger flee behavior for all herbivores in range
    TriggerPanicInRadius(PredatorLocation, ThreatRadius, 60.0f);
    
    // Set global state to alerting
    SetGlobalHerdBehavior(EHerdBehaviorState::Alerting);
    
    UE_LOG(LogTemp, Warning, TEXT("Predator detected at %s - herds entering alert state"), 
        *PredatorLocation.ToString());
}

void UCrowdSimulationSubsystem::OnPlayerDetected(const FVector& PlayerLocation)
{
    // Player detection causes cautious behavior
    // Herbivores become alert, carnivores may investigate
    
    float PlayerThreatRadius = 500.0f; // Smaller than predator threat
    
    for (auto& HerdPair : ActiveHerds)
    {
        EDinosaurHerdType HerdType = HerdPair.Key;
        
        // Different reactions based on herd type
        if (HerdType == EDinosaurHerdType::SmallHerbivores || 
            HerdType == EDinosaurHerdType::MediumHerbivores)
        {
            // Herbivores flee or become alert
            TriggerPanicInRadius(PlayerLocation, PlayerThreatRadius, 15.0f);
        }
        else if (HerdType == EDinosaurHerdType::Carnivores)
        {
            // Carnivores may investigate (handled by Combat AI Agent)
            // Just log for now
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Player detected at %s - herds reacting"), *PlayerLocation.ToString());
}

void UCrowdSimulationSubsystem::OnEnvironmentalChange(const FVector& Location, float Radius, bool bIsPositive)
{
    if (bIsPositive)
    {
        // Positive change (water source, food) attracts herds
        // Implementation would adjust herd movement targets
        UE_LOG(LogTemp, Log, TEXT("Positive environmental change at %s - attracting herds"), *Location.ToString());
    }
    else
    {
        // Negative change (fire, destruction) repels herds
        TriggerPanicInRadius(Location, Radius, 120.0f);
        UE_LOG(LogTemp, Warning, TEXT("Negative environmental change at %s - repelling herds"), *Location.ToString());
    }
}

void UCrowdSimulationSubsystem::InitializeDefaultHerdConfigs()
{
    // Small Herbivores (Compsognathus, Dryosaurus)
    FHerdConfiguration SmallHerb;
    SmallHerb.HerdType = EDinosaurHerdType::SmallHerbivores;
    SmallHerb.MinHerdSize = 50;
    SmallHerb.MaxHerdSize = 200;
    SmallHerb.MovementSpeed = 400.0f;
    SmallHerb.FleeSpeed = 1200.0f;
    SmallHerb.AlertRadius = 1500.0f;
    SmallHerb.CohesionRadius = 300.0f;
    SmallHerb.SeparationRadius = 50.0f;
    SmallHerb.AlignmentRadius = 200.0f;
    DefaultHerdConfigs.Add(EDinosaurHerdType::SmallHerbivores, SmallHerb);

    // Medium Herbivores (Triceratops, Parasaurolophus)
    FHerdConfiguration MediumHerb;
    MediumHerb.HerdType = EDinosaurHerdType::MediumHerbivores;
    MediumHerb.MinHerdSize = 20;
    MediumHerb.MaxHerdSize = 80;
    MediumHerb.MovementSpeed = 250.0f;
    MediumHerb.FleeSpeed = 600.0f;
    MediumHerb.AlertRadius = 2000.0f;
    MediumHerb.CohesionRadius = 500.0f;
    MediumHerb.SeparationRadius = 150.0f;
    MediumHerb.AlignmentRadius = 350.0f;
    DefaultHerdConfigs.Add(EDinosaurHerdType::MediumHerbivores, MediumHerb);

    // Large Herbivores (Brontosaurus, Diplodocus)
    FHerdConfiguration LargeHerb;
    LargeHerb.HerdType = EDinosaurHerdType::LargeHerbivores;
    LargeHerb.MinHerdSize = 5;
    LargeHerb.MaxHerdSize = 20;
    LargeHerb.MovementSpeed = 150.0f;
    LargeHerb.FleeSpeed = 300.0f;
    LargeHerb.AlertRadius = 3000.0f;
    LargeHerb.CohesionRadius = 800.0f;
    LargeHerb.SeparationRadius = 300.0f;
    LargeHerb.AlignmentRadius = 600.0f;
    DefaultHerdConfigs.Add(EDinosaurHerdType::LargeHerbivores, LargeHerb);

    // Carnivores (Velociraptors, Allosaurus)
    FHerdConfiguration Carnivores;
    Carnivores.HerdType = EDinosaurHerdType::Carnivores;
    Carnivores.MinHerdSize = 3;
    Carnivores.MaxHerdSize = 12;
    Carnivores.MovementSpeed = 500.0f;
    Carnivores.FleeSpeed = 800.0f;
    Carnivores.AlertRadius = 2500.0f;
    Carnivores.CohesionRadius = 400.0f;
    Carnivores.SeparationRadius = 200.0f;
    Carnivores.AlignmentRadius = 300.0f;
    DefaultHerdConfigs.Add(EDinosaurHerdType::Carnivores, Carnivores);

    // Scavengers (Pteranodons, small theropods)
    FHerdConfiguration Scavengers;
    Scavengers.HerdType = EDinosaurHerdType::Scavengers;
    Scavengers.MinHerdSize = 10;
    Scavengers.MaxHerdSize = 50;
    Scavengers.MovementSpeed = 600.0f;
    Scavengers.FleeSpeed = 1000.0f;
    Scavengers.AlertRadius = 1800.0f;
    Scavengers.CohesionRadius = 350.0f;
    Scavengers.SeparationRadius = 100.0f;
    Scavengers.AlignmentRadius = 250.0f;
    DefaultHerdConfigs.Add(EDinosaurHerdType::Scavengers, Scavengers);

    // Aquatic (Plesiosaurs, marine reptiles)
    FHerdConfiguration Aquatic;
    Aquatic.HerdType = EDinosaurHerdType::Aquatic;
    Aquatic.MinHerdSize = 5;
    Aquatic.MaxHerdSize = 30;
    Aquatic.MovementSpeed = 300.0f;
    Aquatic.FleeSpeed = 700.0f;
    Aquatic.AlertRadius = 2200.0f;
    Aquatic.CohesionRadius = 600.0f;
    Aquatic.SeparationRadius = 200.0f;
    Aquatic.AlignmentRadius = 400.0f;
    DefaultHerdConfigs.Add(EDinosaurHerdType::Aquatic, Aquatic);
}

void UCrowdSimulationSubsystem::UpdatePerformanceMetrics()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    if (CurrentTime - LastPerformanceCheck >= PerformanceUpdateInterval)
    {
        // Count active agents
        int32 ActualActiveAgents = 0;
        for (const auto& HerdPair : ActiveHerds)
        {
            for (const FEntityHandle& Entity : HerdPair.Value)
            {
                if (MassEntitySubsystem && MassEntitySubsystem->IsEntityValid(Entity))
                {
                    ActualActiveAgents++;
                }
            }
        }
        
        CurrentActiveAgents = ActualActiveAgents;
        LastPerformanceCheck = CurrentTime;
        
        // Log performance if we're near capacity
        if (CurrentActiveAgents > MaxSimultaneousAgents * 0.8f)
        {
            UE_LOG(LogTemp, Warning, TEXT("Crowd Simulation at %d/%d agents (%.1f%% capacity)"), 
                CurrentActiveAgents, MaxSimultaneousAgents, 
                (float)CurrentActiveAgents / MaxSimultaneousAgents * 100.0f);
        }
    }
}

void UCrowdSimulationSubsystem::ProcessHerdBehaviors(float DeltaTime)
{
    // Update all active herds based on current global state
    for (auto& HerdPair : ActiveHerds)
    {
        EDinosaurHerdType HerdType = HerdPair.Key;
        TArray<FEntityHandle>& Herds = HerdPair.Value;
        
        for (FEntityHandle& Entity : Herds)
        {
            if (MassEntitySubsystem && MassEntitySubsystem->IsEntityValid(Entity))
            {
                // Process individual agent behavior
                // This would be handled by Mass processors in full implementation
                ProcessIndividualAgent(Entity, HerdType, DeltaTime);
            }
        }
    }
}

void UCrowdSimulationSubsystem::ProcessIndividualAgent(FEntityHandle Entity, EDinosaurHerdType HerdType, float DeltaTime)
{
    // Placeholder for individual agent processing
    // In full implementation, this would be handled by Mass processors:
    // - Movement processor for flocking behavior
    // - State processor for behavior transitions
    // - LOD processor for performance optimization
    // - Avoidance processor for collision avoidance
}

void UCrowdSimulationSubsystem::HandleLODSystem()
{
    // Implement Level of Detail system for crowd agents
    // Agents far from player get simplified processing
    // This would integrate with Mass LOD processors
}

void UCrowdSimulationSubsystem::CleanupInactiveHerds()
{
    // Remove invalid entities from active herds
    for (auto& HerdPair : ActiveHerds)
    {
        TArray<FEntityHandle>& Herds = HerdPair.Value;
        
        for (int32 i = Herds.Num() - 1; i >= 0; i--)
        {
            if (!MassEntitySubsystem || !MassEntitySubsystem->IsEntityValid(Herds[i]))
            {
                Herds.RemoveAt(i);
                CurrentActiveAgents--;
            }
        }
    }
}