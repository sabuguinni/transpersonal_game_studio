#include "Crowd_MassEntityManager.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "MassArchetypeData.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassRepresentationFragments.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

ACrowd_MassEntityManager::ACrowd_MassEntityManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f;

    MassEntitySubsystem = nullptr;
    MassSpawnerSubsystem = nullptr;
    LastUpdateTime = 0.0f;
    TotalSpawnedEntities = 0;
    CrowdDensityMultiplier = 1.0f;
    UpdateFrequency = 0.1f;
    bEnableLODSystem = true;

    // Default spawn configurations
    FCrowd_SpawnConfig CivilianConfig;
    CivilianConfig.EntityType = ECrowd_EntityType::Civilian;
    CivilianConfig.SpawnCount = 200;
    CivilianConfig.SpawnCenter = FVector(0, 0, 100);
    CivilianConfig.SpawnRadius = 3000.0f;
    CivilianConfig.MovementSpeed = 150.0f;
    CivilianConfig.WanderRadius = 1500.0f;
    DefaultSpawnConfigs.Add(CivilianConfig);

    FCrowd_SpawnConfig WarriorConfig;
    WarriorConfig.EntityType = ECrowd_EntityType::Warrior;
    WarriorConfig.SpawnCount = 50;
    WarriorConfig.SpawnCenter = FVector(2000, 0, 100);
    WarriorConfig.SpawnRadius = 1500.0f;
    WarriorConfig.MovementSpeed = 250.0f;
    WarriorConfig.WanderRadius = 2000.0f;
    DefaultSpawnConfigs.Add(WarriorConfig);

    LODDistance1 = 1000.0f;
    LODDistance2 = 5000.0f;
    LODDistance3 = 15000.0f;
}

void ACrowd_MassEntityManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeMassSystem();
    CreateCrowdArchetype();
    
    // Spawn default crowds after a short delay
    FTimerHandle SpawnTimer;
    GetWorld()->GetTimerManager().SetTimer(SpawnTimer, [this]()
    {
        for (const FCrowd_SpawnConfig& Config : DefaultSpawnConfigs)
        {
            SpawnCrowdEntities(Config);
        }
    }, 2.0f, false);
}

void ACrowd_MassEntityManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    DespawnAllEntities();
    Super::EndPlay(EndPlayReason);
}

void ACrowd_MassEntityManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    LastUpdateTime += DeltaTime;
    
    if (LastUpdateTime >= UpdateFrequency)
    {
        if (bEnableLODSystem)
        {
            ProcessEntityLOD();
        }
        
        UpdateEntityBehaviors(DeltaTime);
        LastUpdateTime = 0.0f;
    }
}

void ACrowd_MassEntityManager::InitializeMassSystem()
{
    if (UWorld* World = GetWorld())
    {
        MassEntitySubsystem = World->GetSubsystem<UMassEntitySubsystem>();
        MassSpawnerSubsystem = World->GetSubsystem<UMassSpawnerSubsystem>();
        
        if (!MassEntitySubsystem)
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to get MassEntitySubsystem"));
        }
        
        if (!MassSpawnerSubsystem)
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to get MassSpawnerSubsystem"));
        }
    }
}

void ACrowd_MassEntityManager::CreateCrowdArchetype()
{
    if (!MassEntitySubsystem)
    {
        return;
    }

    // Create archetype with basic fragments
    FMassArchetypeCompositionDescriptor Descriptor;
    
    // Add transform fragment for position/rotation
    Descriptor.Fragments.Add(FMassTransformFragment::StaticStruct());
    
    // Add velocity fragment for movement
    Descriptor.Fragments.Add(FMassVelocityFragment::StaticStruct());
    
    // Add representation fragment for visual
    Descriptor.Fragments.Add(FMassRepresentationFragment::StaticStruct());
    
    // Add LOD fragment for performance
    Descriptor.Fragments.Add(FMassRepresentationLODFragment::StaticStruct());

    CrowdArchetype = MassEntitySubsystem->CreateArchetype(Descriptor);
    
    UE_LOG(LogTemp, Log, TEXT("Crowd archetype created successfully"));
}

void ACrowd_MassEntityManager::SpawnCrowdEntities(const FCrowd_SpawnConfig& Config)
{
    if (!MassEntitySubsystem || !CrowdArchetype.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot spawn entities - Mass system not initialized"));
        return;
    }

    int32 ActualSpawnCount = FMath::RoundToInt(Config.SpawnCount * CrowdDensityMultiplier);
    
    for (int32 i = 0; i < ActualSpawnCount; i++)
    {
        FVector SpawnLocation = GetRandomSpawnLocation(Config);
        FRotator SpawnRotation = FRotator(0, FMath::RandRange(0.0f, 360.0f), 0);
        
        FMassEntityHandle EntityHandle = MassEntitySubsystem->CreateEntity(CrowdArchetype);
        
        if (EntityHandle.IsValid())
        {
            // Set transform
            if (FMassTransformFragment* TransformFragment = 
                MassEntitySubsystem->GetFragmentDataPtr<FMassTransformFragment>(EntityHandle))
            {
                TransformFragment->SetTransform(FTransform(SpawnRotation, SpawnLocation));
            }
            
            // Set initial velocity
            if (FMassVelocityFragment* VelocityFragment = 
                MassEntitySubsystem->GetFragmentDataPtr<FMassVelocityFragment>(EntityHandle))
            {
                FVector RandomDirection = FVector(
                    FMath::RandRange(-1.0f, 1.0f),
                    FMath::RandRange(-1.0f, 1.0f),
                    0
                ).GetSafeNormal();
                
                VelocityFragment->Value = RandomDirection * Config.MovementSpeed;
            }
            
            SpawnedEntities.Add(EntityHandle);
            TotalSpawnedEntities++;
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Spawned %d entities of type %s"), 
        ActualSpawnCount, *UEnum::GetValueAsString(Config.EntityType));
}

void ACrowd_MassEntityManager::DespawnAllEntities()
{
    if (!MassEntitySubsystem)
    {
        return;
    }

    for (const FMassEntityHandle& EntityHandle : SpawnedEntities)
    {
        if (EntityHandle.IsValid())
        {
            MassEntitySubsystem->DestroyEntity(EntityHandle);
        }
    }
    
    SpawnedEntities.Empty();
    TotalSpawnedEntities = 0;
    
    UE_LOG(LogTemp, Log, TEXT("All crowd entities despawned"));
}

void ACrowd_MassEntityManager::SetCrowdDensity(float NewDensity)
{
    CrowdDensityMultiplier = FMath::Clamp(NewDensity, 0.1f, 5.0f);
    UE_LOG(LogTemp, Log, TEXT("Crowd density set to %f"), CrowdDensityMultiplier);
}

int32 ACrowd_MassEntityManager::GetActiveEntityCount() const
{
    return SpawnedEntities.Num();
}

void ACrowd_MassEntityManager::UpdateCrowdBehavior(ECrowd_EntityType EntityType, float NewSpeed, float NewRadius)
{
    // Update behavior parameters for specific entity type
    for (FCrowd_SpawnConfig& Config : DefaultSpawnConfigs)
    {
        if (Config.EntityType == EntityType)
        {
            Config.MovementSpeed = NewSpeed;
            Config.WanderRadius = NewRadius;
            break;
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Updated behavior for entity type %s"), 
        *UEnum::GetValueAsString(EntityType));
}

void ACrowd_MassEntityManager::ProcessEntityLOD()
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
    
    for (const FMassEntityHandle& EntityHandle : SpawnedEntities)
    {
        if (!EntityHandle.IsValid())
        {
            continue;
        }
        
        FMassTransformFragment* TransformFragment = 
            MassEntitySubsystem->GetFragmentDataPtr<FMassTransformFragment>(EntityHandle);
        
        if (!TransformFragment)
        {
            continue;
        }
        
        float Distance = FVector::Dist(PlayerLocation, TransformFragment->GetTransform().GetLocation());
        
        // Set LOD based on distance
        FMassRepresentationLODFragment* LODFragment = 
            MassEntitySubsystem->GetFragmentDataPtr<FMassRepresentationLODFragment>(EntityHandle);
        
        if (LODFragment)
        {
            if (Distance <= LODDistance1)
            {
                LODFragment->LOD = EMassLOD::High;
            }
            else if (Distance <= LODDistance2)
            {
                LODFragment->LOD = EMassLOD::Medium;
            }
            else if (Distance <= LODDistance3)
            {
                LODFragment->LOD = EMassLOD::Low;
            }
            else
            {
                LODFragment->LOD = EMassLOD::Off;
            }
        }
    }
}

void ACrowd_MassEntityManager::UpdateEntityBehaviors(float DeltaTime)
{
    if (!MassEntitySubsystem)
    {
        return;
    }

    for (const FMassEntityHandle& EntityHandle : SpawnedEntities)
    {
        if (!EntityHandle.IsValid())
        {
            continue;
        }
        
        FMassTransformFragment* TransformFragment = 
            MassEntitySubsystem->GetFragmentDataPtr<FMassTransformFragment>(EntityHandle);
        FMassVelocityFragment* VelocityFragment = 
            MassEntitySubsystem->GetFragmentDataPtr<FMassVelocityFragment>(EntityHandle);
        
        if (!TransformFragment || !VelocityFragment)
        {
            continue;
        }
        
        // Simple wandering behavior
        FVector CurrentLocation = TransformFragment->GetTransform().GetLocation();
        FVector CurrentVelocity = VelocityFragment->Value;
        
        // Randomly change direction occasionally
        if (FMath::RandRange(0.0f, 1.0f) < 0.02f) // 2% chance per update
        {
            FVector NewDirection = FVector(
                FMath::RandRange(-1.0f, 1.0f),
                FMath::RandRange(-1.0f, 1.0f),
                0
            ).GetSafeNormal();
            
            VelocityFragment->Value = NewDirection * CurrentVelocity.Size();
        }
        
        // Update position
        FVector NewLocation = CurrentLocation + (CurrentVelocity * DeltaTime);
        TransformFragment->SetTransform(FTransform(
            TransformFragment->GetTransform().GetRotation(),
            NewLocation
        ));
    }
}

FVector ACrowd_MassEntityManager::GetRandomSpawnLocation(const FCrowd_SpawnConfig& Config) const
{
    FVector2D RandomCircle = FMath::RandPointInCircle(Config.SpawnRadius);
    return Config.SpawnCenter + FVector(RandomCircle.X, RandomCircle.Y, 0);
}