#include "DinosaurHerdManager.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassNavigationFragments.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"

ADinosaurHerdManager::ADinosaurHerdManager()
{
    PrimaryActorTick.bCanEverTick = true;
    LastUpdateTime = 0.0f;
    CurrentUpdateIndex = 0;

    // Default herd configurations for prehistoric ecosystem
    FDinosaurHerdConfig HerbivoreSmall;
    HerbivoreSmall.HerdType = EDinosaurHerdType::Herbivore_Small;
    HerbivoreSmall.MinHerdSize = 8;
    HerbivoreSmall.MaxHerdSize = 20;
    HerbivoreSmall.MovementSpeed = 400.0f;
    HerbivoreSmall.CohesionRadius = 300.0f;
    HerbivoreSmall.SeparationRadius = 150.0f;
    HerbivoreSmall.DangerResponseRadius = 1200.0f;
    HerbivoreSmall.PredatorTypes = {EDinosaurHerdType::Carnivore_Pack, EDinosaurHerdType::Carnivore_Solo};

    FDinosaurHerdConfig HerbivoreMedium;
    HerbivoreMedium.HerdType = EDinosaurHerdType::Herbivore_Medium;
    HerbivoreMedium.MinHerdSize = 4;
    HerbivoreMedium.MaxHerdSize = 12;
    HerbivoreMedium.MovementSpeed = 250.0f;
    HerbivoreMedium.CohesionRadius = 500.0f;
    HerbivoreMedium.SeparationRadius = 300.0f;
    HerbivoreMedium.DangerResponseRadius = 1000.0f;
    HerbivoreMedium.PredatorTypes = {EDinosaurHerdType::Carnivore_Pack, EDinosaurHerdType::Carnivore_Solo};

    FDinosaurHerdConfig CarnivorePack;
    CarnivorePack.HerdType = EDinosaurHerdType::Carnivore_Pack;
    CarnivorePack.MinHerdSize = 3;
    CarnivorePack.MaxHerdSize = 8;
    CarnivorePack.MovementSpeed = 500.0f;
    CarnivorePack.CohesionRadius = 400.0f;
    CarnivorePack.SeparationRadius = 200.0f;
    CarnivorePack.bAvoidsPredators = false;

    HerdConfigurations.Add(HerbivoreSmall);
    HerdConfigurations.Add(HerbivoreMedium);
    HerdConfigurations.Add(CarnivorePack);
}

void ADinosaurHerdManager::BeginPlay()
{
    Super::BeginPlay();

    MassEntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
    MassSpawnerSubsystem = GetWorld()->GetSubsystem<UMassSpawnerSubsystem>();

    if (!MassEntitySubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("DinosaurHerdManager: MassEntitySubsystem not found!"));
        return;
    }

    // Initialize with some herds
    SpawnNewHerds();
}

void ADinosaurHerdManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!MassEntitySubsystem)
    {
        return;
    }

    // Staggered updates for performance
    if (GetWorld()->GetTimeSeconds() - LastUpdateTime >= UpdateFrequency)
    {
        UpdateHerdBehaviors(DeltaTime);
        LastUpdateTime = GetWorld()->GetTimeSeconds();
    }

    // Continuous spawning/despawning
    SpawnNewHerds();
    DespawnDistantHerds();
}

void ADinosaurHerdManager::UpdateHerdBehaviors(float DeltaTime)
{
    if (ActiveHerds.Num() == 0)
    {
        return;
    }

    // Update a subset of herds each frame for performance
    int32 HerdsToUpdate = FMath::Min(MaxEntitiesPerFrame / 10, ActiveHerds.Num());
    
    for (int32 i = 0; i < HerdsToUpdate; ++i)
    {
        int32 HerdIndex = (CurrentUpdateIndex + i) % ActiveHerds.Num();
        FDinosaurHerdInstance& Herd = ActiveHerds[HerdIndex];

        UpdateHerdMovement(Herd, DeltaTime);
        UpdateHerdDanger(Herd);
    }

    CurrentUpdateIndex = (CurrentUpdateIndex + HerdsToUpdate) % ActiveHerds.Num();
}

void ADinosaurHerdManager::UpdateHerdMovement(FDinosaurHerdInstance& Herd, float DeltaTime)
{
    if (!Herd.LeaderEntity.IsValid())
    {
        return;
    }

    // Get herd configuration
    const FDinosaurHerdConfig* Config = HerdConfigurations.FindByPredicate(
        [&Herd](const FDinosaurHerdConfig& Cfg) { return Cfg.HerdType == Herd.HerdType; });

    if (!Config)
    {
        return;
    }

    // Calculate flocking forces
    FVector CohesionForce = CalculateCohesion(Herd);
    FVector SeparationForce = CalculateSeparation(Herd);
    FVector AlignmentForce = CalculateAlignment(Herd);
    FVector AvoidanceForce = CalculateAvoidance(Herd);

    // Combine forces with weights
    FVector FinalForce = FVector::ZeroVector;
    FinalForce += CohesionForce * 0.3f;
    FinalForce += SeparationForce * 0.5f;
    FinalForce += AlignmentForce * 0.2f;
    FinalForce += AvoidanceForce * 0.8f;

    // Add destination seeking
    FVector ToDestination = (Herd.CurrentDestination - GetActorLocation()).GetSafeNormal();
    FinalForce += ToDestination * 0.4f;

    // Apply danger response
    if (Herd.bInDanger)
    {
        FVector FleeDirection = (GetActorLocation() - Herd.DangerSource).GetSafeNormal();
        FinalForce += FleeDirection * 2.0f; // Strong flee response
    }

    // Normalize and apply to entities
    FinalForce = FinalForce.GetClampedToMaxSize(Config->MovementSpeed);

    // Apply movement to all herd members through Mass Entity system
    // This would integrate with Mass Movement processors
}

FVector ADinosaurHerdManager::CalculateCohesion(const FDinosaurHerdInstance& Herd)
{
    if (Herd.MemberEntities.Num() <= 1)
    {
        return FVector::ZeroVector;
    }

    FVector CenterOfMass = FVector::ZeroVector;
    int32 ValidMembers = 0;

    // Calculate center of mass for the herd
    for (const FMassEntityHandle& Entity : Herd.MemberEntities)
    {
        if (Entity.IsValid())
        {
            // Get entity position through Mass Entity system
            // This would query FTransformFragment
            ValidMembers++;
        }
    }

    if (ValidMembers > 0)
    {
        CenterOfMass /= ValidMembers;
        return (CenterOfMass - GetActorLocation()).GetSafeNormal();
    }

    return FVector::ZeroVector;
}

FVector ADinosaurHerdManager::CalculateSeparation(const FDinosaurHerdInstance& Herd)
{
    const FDinosaurHerdConfig* Config = HerdConfigurations.FindByPredicate(
        [&Herd](const FDinosaurHerdConfig& Cfg) { return Cfg.HerdType == Herd.HerdType; });

    if (!Config)
    {
        return FVector::ZeroVector;
    }

    FVector SeparationForce = FVector::ZeroVector;
    
    // Calculate separation from nearby herd members
    for (const FMassEntityHandle& Entity : Herd.MemberEntities)
    {
        if (Entity.IsValid())
        {
            // Get distance to this entity
            // If too close, add repulsion force
        }
    }

    return SeparationForce.GetSafeNormal();
}

FVector ADinosaurHerdManager::CalculateAlignment(const FDinosaurHerdInstance& Herd)
{
    if (Herd.MemberEntities.Num() <= 1)
    {
        return FVector::ZeroVector;
    }

    FVector AverageVelocity = FVector::ZeroVector;
    int32 ValidMembers = 0;

    // Calculate average velocity of herd members
    for (const FMassEntityHandle& Entity : Herd.MemberEntities)
    {
        if (Entity.IsValid())
        {
            // Get entity velocity through Mass Entity system
            // This would query FMassVelocityFragment
            ValidMembers++;
        }
    }

    if (ValidMembers > 0)
    {
        AverageVelocity /= ValidMembers;
        return AverageVelocity.GetSafeNormal();
    }

    return FVector::ZeroVector;
}

FVector ADinosaurHerdManager::CalculateAvoidance(const FDinosaurHerdInstance& Herd)
{
    FVector AvoidanceForce = FVector::ZeroVector;

    // Avoid obstacles, predators, and dangerous areas
    // This would integrate with navigation system and threat detection

    return AvoidanceForce.GetSafeNormal();
}

void ADinosaurHerdManager::SpawnNewHerds()
{
    if (ActiveHerds.Num() >= MaxActiveHerds)
    {
        return;
    }

    // Get player location for proximity spawning
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        return;
    }

    FVector PlayerLocation = PlayerPawn->GetActorLocation();

    // Spawn herds at random locations within spawn radius
    for (const FDinosaurHerdConfig& Config : HerdConfigurations)
    {
        if (FMath::RandRange(0.0f, 1.0f) < 0.1f) // 10% chance per frame
        {
            FVector SpawnLocation = PlayerLocation + FMath::VRand() * FMath::RandRange(2000.0f, SpawnRadius);
            SpawnLocation.Z = 0.0f; // Project to ground

            SpawnHerdAtLocation(Config.HerdType, SpawnLocation);
        }
    }
}

void ADinosaurHerdManager::DespawnDistantHerds()
{
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        return;
    }

    FVector PlayerLocation = PlayerPawn->GetActorLocation();

    // Remove herds that are too far from player
    for (int32 i = ActiveHerds.Num() - 1; i >= 0; --i)
    {
        FDinosaurHerdInstance& Herd = ActiveHerds[i];
        float DistanceToPlayer = FVector::Dist(Herd.HomeTerritory, PlayerLocation);

        if (DistanceToPlayer > DespawnRadius)
        {
            DespawnHerd(i);
        }
    }
}

void ADinosaurHerdManager::SpawnHerdAtLocation(EDinosaurHerdType HerdType, const FVector& Location)
{
    const FDinosaurHerdConfig* Config = HerdConfigurations.FindByPredicate(
        [HerdType](const FDinosaurHerdConfig& Cfg) { return Cfg.HerdType == HerdType; });

    if (!Config || !MassEntitySubsystem)
    {
        return;
    }

    FDinosaurHerdInstance NewHerd;
    NewHerd.HerdType = HerdType;
    NewHerd.HomeTerritory = Location;
    NewHerd.TerritoryRadius = 2000.0f;
    NewHerd.CurrentDestination = Location;
    NewHerd.bInDanger = false;
    NewHerd.LastActivityTime = GetWorld()->GetTimeSeconds();

    // Spawn entities using Mass Entity system
    int32 HerdSize = FMath::RandRange(Config->MinHerdSize, Config->MaxHerdSize);
    
    for (int32 i = 0; i < HerdSize; ++i)
    {
        FVector SpawnPos = Location + FMath::VRand() * 300.0f;
        
        // Create Mass Entity with appropriate fragments
        // This would use Mass Spawner to create entities with:
        // - FTransformFragment
        // - FMassVelocityFragment
        // - FMassForceFragment
        // - Custom dinosaur fragments
        
        FMassEntityHandle NewEntity; // Would be created through Mass system
        NewHerd.MemberEntities.Add(NewEntity);

        if (i == 0)
        {
            NewHerd.LeaderEntity = NewEntity;
        }
    }

    ActiveHerds.Add(NewHerd);

    UE_LOG(LogTemp, Log, TEXT("Spawned herd of %d %s at location %s"), 
           HerdSize, 
           *UEnum::GetValueAsString(HerdType),
           *Location.ToString());
}

void ADinosaurHerdManager::DespawnHerd(int32 HerdIndex)
{
    if (!ActiveHerds.IsValidIndex(HerdIndex))
    {
        return;
    }

    FDinosaurHerdInstance& Herd = ActiveHerds[HerdIndex];

    // Despawn all entities in the herd
    for (const FMassEntityHandle& Entity : Herd.MemberEntities)
    {
        if (Entity.IsValid() && MassEntitySubsystem)
        {
            // Destroy entity through Mass Entity system
            MassEntitySubsystem->DestroyEntity(Entity);
        }
    }

    ActiveHerds.RemoveAt(HerdIndex);
}

TArray<FDinosaurHerdInstance> ADinosaurHerdManager::GetNearbyHerds(const FVector& Location, float Radius)
{
    TArray<FDinosaurHerdInstance> NearbyHerds;

    for (const FDinosaurHerdInstance& Herd : ActiveHerds)
    {
        if (FVector::Dist(Herd.HomeTerritory, Location) <= Radius)
        {
            NearbyHerds.Add(Herd);
        }
    }

    return NearbyHerds;
}

void ADinosaurHerdManager::SetPlayerLocation(const FVector& PlayerLocation)
{
    // Update all herds about player presence
    for (FDinosaurHerdInstance& Herd : ActiveHerds)
    {
        const FDinosaurHerdConfig* Config = HerdConfigurations.FindByPredicate(
            [&Herd](const FDinosaurHerdConfig& Cfg) { return Cfg.HerdType == Herd.HerdType; });

        if (Config && Config->bAvoidsPredators)
        {
            float DistanceToPlayer = FVector::Dist(Herd.HomeTerritory, PlayerLocation);
            
            if (DistanceToPlayer < Config->DangerResponseRadius)
            {
                TriggerHerdPanicResponse(Herd, PlayerLocation);
            }
        }
    }
}

void ADinosaurHerdManager::TriggerHerdPanicResponse(FDinosaurHerdInstance& Herd, const FVector& DangerSource)
{
    Herd.bInDanger = true;
    Herd.DangerSource = DangerSource;
    
    // Find safe destination away from danger
    FVector FleeDirection = (Herd.HomeTerritory - DangerSource).GetSafeNormal();
    Herd.CurrentDestination = Herd.HomeTerritory + FleeDirection * 1500.0f;

    UE_LOG(LogTemp, Log, TEXT("Herd %s triggered panic response, fleeing to %s"), 
           *UEnum::GetValueAsString(Herd.HerdType),
           *Herd.CurrentDestination.ToString());
}

void ADinosaurHerdManager::DebugDrawHerds()
{
    if (!GetWorld())
    {
        return;
    }

    for (const FDinosaurHerdInstance& Herd : ActiveHerds)
    {
        // Draw herd territory
        DrawDebugSphere(GetWorld(), Herd.HomeTerritory, Herd.TerritoryRadius, 12, FColor::Green, false, 0.1f);
        
        // Draw current destination
        DrawDebugSphere(GetWorld(), Herd.CurrentDestination, 100.0f, 8, FColor::Blue, false, 0.1f);
        DrawDebugLine(GetWorld(), Herd.HomeTerritory, Herd.CurrentDestination, FColor::Yellow, false, 0.1f);

        // Draw danger indicators
        if (Herd.bInDanger)
        {
            DrawDebugSphere(GetWorld(), Herd.DangerSource, 200.0f, 8, FColor::Red, false, 0.1f);
            DrawDebugLine(GetWorld(), Herd.HomeTerritory, Herd.DangerSource, FColor::Red, false, 0.1f);
        }
    }
}