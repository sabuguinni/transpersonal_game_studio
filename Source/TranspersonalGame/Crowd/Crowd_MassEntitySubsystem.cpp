#include "Crowd_MassEntitySubsystem.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "MassArchetypeData.h"
#include "MassEntityTemplateRegistry.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

UCrowd_MassEntitySubsystem::UCrowd_MassEntitySubsystem()
{
    // Initialize biome centers based on established coordinates
    BiomeCenters.Empty();
    BiomeCenters.Add(FVector(0.0f, 0.0f, 100.0f));           // Savana
    BiomeCenters.Add(FVector(-45000.0f, 40000.0f, 100.0f));  // Floresta
    BiomeCenters.Add(FVector(55000.0f, 0.0f, 100.0f));       // Deserto
    BiomeCenters.Add(FVector(-50000.0f, -45000.0f, 100.0f)); // Pantano
    BiomeCenters.Add(FVector(40000.0f, 50000.0f, 500.0f));   // Montanha

    // Initialize biome radii
    BiomeRadii.Empty();
    BiomeRadii.Add(15000.0f); // Savana radius
    BiomeRadii.Add(12000.0f); // Floresta radius
    BiomeRadii.Add(18000.0f); // Deserto radius
    BiomeRadii.Add(10000.0f); // Pantano radius
    BiomeRadii.Add(8000.0f);  // Montanha radius

    MaxCrowdEntities = 50000;
    UpdateFrequency = 0.1f;
    bEnableLODSystem = true;
}

void UCrowd_MassEntitySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // Get Mass Entity subsystem references
    MassEntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
    MassSpawnerSubsystem = GetWorld()->GetSubsystem<UMassSpawnerSubsystem>();

    if (MassEntitySubsystem && MassSpawnerSubsystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("Crowd_MassEntitySubsystem initialized successfully"));
        InitializeCrowdSimulation();
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to get Mass Entity subsystems"));
    }
}

void UCrowd_MassEntitySubsystem::Deinitialize()
{
    // Clear timer
    if (GetWorld() && GetWorld()->GetTimerManager().IsTimerActive(CrowdUpdateTimer))
    {
        GetWorld()->GetTimerManager().ClearTimer(CrowdUpdateTimer);
    }

    // Clear all crowd entities
    ClearAllCrowdEntities();

    Super::Deinitialize();
}

void UCrowd_MassEntitySubsystem::InitializeCrowdSimulation()
{
    if (!MassEntitySubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("MassEntitySubsystem not available"));
        return;
    }

    // Create crowd archetype
    CreateCrowdArchetype();

    // Start periodic update timer
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            CrowdUpdateTimer,
            FTimerDelegate::CreateUObject(this, &UCrowd_MassEntitySubsystem::UpdateCrowdBehavior, UpdateFrequency),
            UpdateFrequency,
            true
        );
    }

    UE_LOG(LogTemp, Warning, TEXT("Crowd simulation initialized with %d biomes"), BiomeCenters.Num());
}

void UCrowd_MassEntitySubsystem::CreateCrowdArchetype()
{
    if (!MassEntitySubsystem)
    {
        return;
    }

    // Create archetype composition
    FMassArchetypeCompositionDescriptor CompositionDesc;
    CompositionDesc.Fragments.Add<FCrowd_MovementFragment>();
    CompositionDesc.Fragments.Add<FCrowd_BehaviorFragment>();
    CompositionDesc.Fragments.Add<FCrowd_VisualFragment>();
    CompositionDesc.Fragments.Add<FTransformFragment>();

    // Create the archetype
    CrowdArchetype = MassEntitySubsystem->CreateArchetype(CompositionDesc);

    UE_LOG(LogTemp, Warning, TEXT("Crowd archetype created successfully"));
}

void UCrowd_MassEntitySubsystem::SpawnCrowdEntities(int32 EntityCount, FVector SpawnCenter, float SpawnRadius)
{
    if (!MassEntitySubsystem || !CrowdArchetype.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot spawn crowd entities - invalid subsystem or archetype"));
        return;
    }

    // Clamp entity count to maximum
    EntityCount = FMath::Min(EntityCount, MaxCrowdEntities - CrowdEntities.Num());

    if (EntityCount <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot spawn entities - limit reached or invalid count"));
        return;
    }

    // Spawn entities in batches for performance
    const int32 BatchSize = 1000;
    int32 RemainingEntities = EntityCount;

    while (RemainingEntities > 0)
    {
        int32 CurrentBatchSize = FMath::Min(BatchSize, RemainingEntities);
        TArray<FMassEntityHandle> NewEntities;

        // Create entities
        MassEntitySubsystem->BatchCreateEntities(CrowdArchetype, CurrentBatchSize, NewEntities);

        // Initialize entity data
        for (const FMassEntityHandle& Entity : NewEntities)
        {
            // Set transform
            FTransformFragment& TransformFragment = MassEntitySubsystem->GetFragmentDataChecked<FTransformFragment>(Entity);
            FVector RandomOffset = FVector(
                FMath::RandRange(-SpawnRadius, SpawnRadius),
                FMath::RandRange(-SpawnRadius, SpawnRadius),
                0.0f
            );
            TransformFragment.GetMutableTransform().SetLocation(SpawnCenter + RandomOffset);

            // Set movement data
            FCrowd_MovementFragment& MovementFragment = MassEntitySubsystem->GetFragmentDataChecked<FCrowd_MovementFragment>(Entity);
            MovementFragment.TargetLocation = SpawnCenter + RandomOffset;
            MovementFragment.MovementSpeed = FMath::RandRange(100.0f, 200.0f);
            MovementFragment.WanderRadius = FMath::RandRange(300.0f, 800.0f);
            MovementFragment.FlockingStrength = FMath::RandRange(0.2f, 0.5f);
            MovementFragment.SeparationDistance = FMath::RandRange(80.0f, 150.0f);

            // Set behavior data
            FCrowd_BehaviorFragment& BehaviorFragment = MassEntitySubsystem->GetFragmentDataChecked<FCrowd_BehaviorFragment>(Entity);
            BehaviorFragment.AggressionLevel = FMath::RandRange(0.0f, 0.3f);
            BehaviorFragment.FearLevel = FMath::RandRange(0.3f, 0.8f);
            BehaviorFragment.CuriosityLevel = FMath::RandRange(0.1f, 0.6f);
            BehaviorFragment.HerdInstinct = FMath::RandRange(0.5f, 0.9f);
            BehaviorFragment.BiomePreference = FMath::RandRange(0, BiomeCenters.Num() - 1);
            BehaviorFragment.bIsPredator = FMath::RandBool() && (FMath::RandRange(0.0f, 1.0f) < 0.1f); // 10% predators

            // Set visual data
            FCrowd_VisualFragment& VisualFragment = MassEntitySubsystem->GetFragmentDataChecked<FCrowd_VisualFragment>(Entity);
            VisualFragment.Scale = FVector(FMath::RandRange(0.8f, 1.2f));
            VisualFragment.Rotation = FRotator(0.0f, FMath::RandRange(0.0f, 360.0f), 0.0f);
            VisualFragment.LODLevel = 0;

            CrowdEntities.Add(Entity);
        }

        RemainingEntities -= CurrentBatchSize;
    }

    UE_LOG(LogTemp, Warning, TEXT("Spawned %d crowd entities at %s (radius: %.1f)"), EntityCount, *SpawnCenter.ToString(), SpawnRadius);
}

void UCrowd_MassEntitySubsystem::UpdateCrowdBehavior(float DeltaTime)
{
    if (!MassEntitySubsystem || CrowdEntities.Num() == 0)
    {
        return;
    }

    // Process movement and behavior in chunks for performance
    ProcessCrowdMovement(DeltaTime);
    ProcessCrowdBehavior(DeltaTime);

    if (bEnableLODSystem)
    {
        UpdateCrowdLOD();
    }
}

void UCrowd_MassEntitySubsystem::ProcessCrowdMovement(float DeltaTime)
{
    const int32 ProcessingChunkSize = 5000;
    static int32 ProcessingIndex = 0;

    int32 EndIndex = FMath::Min(ProcessingIndex + ProcessingChunkSize, CrowdEntities.Num());

    for (int32 i = ProcessingIndex; i < EndIndex; ++i)
    {
        const FMassEntityHandle& Entity = CrowdEntities[i];
        
        if (!MassEntitySubsystem->IsEntityValid(Entity))
        {
            continue;
        }

        FTransformFragment& TransformFragment = MassEntitySubsystem->GetFragmentDataChecked<FTransformFragment>(Entity);
        FCrowd_MovementFragment& MovementFragment = MassEntitySubsystem->GetFragmentDataChecked<FCrowd_MovementFragment>(Entity);
        const FCrowd_BehaviorFragment& BehaviorFragment = MassEntitySubsystem->GetFragmentDataChecked<FCrowd_BehaviorFragment>(Entity);

        FVector CurrentLocation = TransformFragment.GetTransform().GetLocation();
        FVector TargetDirection = (MovementFragment.TargetLocation - CurrentLocation).GetSafeNormal();

        // Apply movement
        if (MovementFragment.bIsMoving && TargetDirection.SizeSquared() > 0.01f)
        {
            FVector NewLocation = CurrentLocation + (TargetDirection * MovementFragment.MovementSpeed * DeltaTime);
            TransformFragment.GetMutableTransform().SetLocation(NewLocation);

            // Check if reached target
            if (FVector::Dist(NewLocation, MovementFragment.TargetLocation) < 100.0f)
            {
                // Set new random target within wander radius
                FVector BiomeCenter = BiomeCenters.IsValidIndex(BehaviorFragment.BiomePreference) ? 
                    BiomeCenters[BehaviorFragment.BiomePreference] : FVector::ZeroVector;
                
                FVector RandomOffset = FVector(
                    FMath::RandRange(-MovementFragment.WanderRadius, MovementFragment.WanderRadius),
                    FMath::RandRange(-MovementFragment.WanderRadius, MovementFragment.WanderRadius),
                    0.0f
                );
                MovementFragment.TargetLocation = BiomeCenter + RandomOffset;
            }
        }
        else
        {
            // Start moving if not already
            MovementFragment.bIsMoving = true;
        }
    }

    ProcessingIndex = (EndIndex >= CrowdEntities.Num()) ? 0 : EndIndex;
}

void UCrowd_MassEntitySubsystem::ProcessCrowdBehavior(float DeltaTime)
{
    // Simplified behavior processing for performance
    // In a full implementation, this would handle flocking, predator avoidance, etc.
}

void UCrowd_MassEntitySubsystem::UpdateCrowdLOD()
{
    // Get player location for LOD calculations
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        return;
    }

    FVector PlayerLocation = PlayerPawn->GetActorLocation();

    // Update LOD levels based on distance to player
    for (const FMassEntityHandle& Entity : CrowdEntities)
    {
        if (!MassEntitySubsystem->IsEntityValid(Entity))
        {
            continue;
        }

        const FTransformFragment& TransformFragment = MassEntitySubsystem->GetFragmentDataChecked<FTransformFragment>(Entity);
        FCrowd_VisualFragment& VisualFragment = MassEntitySubsystem->GetFragmentDataChecked<FCrowd_VisualFragment>(Entity);

        float DistanceToPlayer = FVector::Dist(TransformFragment.GetTransform().GetLocation(), PlayerLocation);

        // Set LOD based on distance
        if (DistanceToPlayer < 1000.0f)
        {
            VisualFragment.LODLevel = 0; // High detail
        }
        else if (DistanceToPlayer < 5000.0f)
        {
            VisualFragment.LODLevel = 1; // Medium detail
        }
        else
        {
            VisualFragment.LODLevel = 2; // Low detail
        }
    }
}

void UCrowd_MassEntitySubsystem::SetBiomePreferences(int32 BiomeIndex, float AggressionMod, float FearMod)
{
    if (!BiomeCenters.IsValidIndex(BiomeIndex))
    {
        return;
    }

    // Apply biome-specific behavior modifications to entities
    for (const FMassEntityHandle& Entity : CrowdEntities)
    {
        if (!MassEntitySubsystem->IsEntityValid(Entity))
        {
            continue;
        }

        FCrowd_BehaviorFragment& BehaviorFragment = MassEntitySubsystem->GetFragmentDataChecked<FCrowd_BehaviorFragment>(Entity);
        
        if (BehaviorFragment.BiomePreference == BiomeIndex)
        {
            BehaviorFragment.AggressionLevel = FMath::Clamp(BehaviorFragment.AggressionLevel * AggressionMod, 0.0f, 1.0f);
            BehaviorFragment.FearLevel = FMath::Clamp(BehaviorFragment.FearLevel * FearMod, 0.0f, 1.0f);
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Updated biome %d preferences: Aggression x%.2f, Fear x%.2f"), BiomeIndex, AggressionMod, FearMod);
}

int32 UCrowd_MassEntitySubsystem::GetActiveCrowdEntityCount() const
{
    return CrowdEntities.Num();
}

void UCrowd_MassEntitySubsystem::ClearAllCrowdEntities()
{
    if (MassEntitySubsystem && CrowdEntities.Num() > 0)
    {
        // Destroy all crowd entities
        for (const FMassEntityHandle& Entity : CrowdEntities)
        {
            if (MassEntitySubsystem->IsEntityValid(Entity))
            {
                MassEntitySubsystem->DestroyEntity(Entity);
            }
        }

        CrowdEntities.Empty();
        UE_LOG(LogTemp, Warning, TEXT("Cleared all crowd entities"));
    }
}