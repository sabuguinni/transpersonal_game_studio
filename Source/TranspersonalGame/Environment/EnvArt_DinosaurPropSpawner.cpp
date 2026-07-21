#include "EnvArt_DinosaurPropSpawner.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"

AEnvArt_DinosaurPropSpawner::AEnvArt_DinosaurPropSpawner()
{
    PrimaryActorTick.bCanEverTick = false;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Initialize default values
    PropsPerBiome = 10;
    bAutoSpawnOnBeginPlay = false;
    MinDistanceBetweenProps = 500.0f;
    bDebugSpawning = false;

    // Default prop types to spawn
    PropTypesToSpawn.Add(EEnvArt_PropType::FallenLog);
    PropTypesToSpawn.Add(EEnvArt_PropType::Boulder);
    PropTypesToSpawn.Add(EEnvArt_PropType::Fern);
}

void AEnvArt_DinosaurPropSpawner::BeginPlay()
{
    Super::BeginPlay();

    InitializeBiomeCoordinates();

    if (bAutoSpawnOnBeginPlay)
    {
        SpawnPropsInAllBiomes();
    }

    if (bDebugSpawning)
    {
        LogBiomeInfo();
    }
}

void AEnvArt_DinosaurPropSpawner::InitializeBiomeCoordinates()
{
    BiomeCoordinates.Empty();

    // Initialize the 5 biomes with correct coordinates from memory
    BiomeCoordinates.Add(FEnvArt_BiomeCoordinates(TEXT("Savana"), FVector(0.0f, 0.0f, 100.0f)));
    BiomeCoordinates.Add(FEnvArt_BiomeCoordinates(TEXT("Pantano"), FVector(-50000.0f, -45000.0f, 100.0f)));
    BiomeCoordinates.Add(FEnvArt_BiomeCoordinates(TEXT("Floresta"), FVector(-45000.0f, 40000.0f, 100.0f)));
    BiomeCoordinates.Add(FEnvArt_BiomeCoordinates(TEXT("Deserto"), FVector(55000.0f, 0.0f, 100.0f)));
    BiomeCoordinates.Add(FEnvArt_BiomeCoordinates(TEXT("Montanha"), FVector(40000.0f, 50000.0f, 100.0f)));

    if (bDebugSpawning)
    {
        UE_LOG(LogTemp, Warning, TEXT("EnvArt_DinosaurPropSpawner: Initialized %d biomes"), BiomeCoordinates.Num());
    }
}

FVector AEnvArt_DinosaurPropSpawner::GetRandomLocationInBiome(const FEnvArt_BiomeCoordinates& Biome) const
{
    float RandomX = FMath::RandRange(-Biome.SpawnRadius, Biome.SpawnRadius);
    float RandomY = FMath::RandRange(-Biome.SpawnRadius, Biome.SpawnRadius);
    
    FVector RandomLocation = Biome.CenterLocation + FVector(RandomX, RandomY, 0.0f);
    
    // Ensure Z coordinate is at ground level (simplified)
    RandomLocation.Z = 100.0f;
    
    return RandomLocation;
}

bool AEnvArt_DinosaurPropSpawner::IsValidSpawnLocation(const FVector& Location) const
{
    // Check minimum distance from existing props
    for (const AActor* ExistingProp : SpawnedProps)
    {
        if (ExistingProp && FVector::Dist(ExistingProp->GetActorLocation(), Location) < MinDistanceBetweenProps)
        {
            return false;
        }
    }
    
    return true;
}

AActor* AEnvArt_DinosaurPropSpawner::SpawnPropAtLocation(EEnvArt_PropType PropType, const FVector& Location)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }

    UStaticMesh* PropMesh = GetMeshForPropType(PropType);
    if (!PropMesh)
    {
        if (bDebugSpawning)
        {
            UE_LOG(LogTemp, Warning, TEXT("EnvArt_DinosaurPropSpawner: No mesh found for prop type"));
        }
        return nullptr;
    }

    // Spawn a static mesh actor
    AStaticMeshActor* PropActor = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Location, FRotator::ZeroRotator);
    if (PropActor)
    {
        // Set the mesh
        PropActor->GetStaticMeshComponent()->SetStaticMesh(PropMesh);
        
        // Set a descriptive label
        FString PropLabel = FString::Printf(TEXT("EnvProp_%s_%d"), 
            *UEnum::GetValueAsString(PropType), SpawnedProps.Num());
        PropActor->SetActorLabel(PropLabel);
        
        // Add random rotation for natural look
        FRotator RandomRotation = FRotator(0.0f, FMath::RandRange(0.0f, 360.0f), 0.0f);
        PropActor->SetActorRotation(RandomRotation);
        
        SpawnedProps.Add(PropActor);
        
        if (bDebugSpawning)
        {
            UE_LOG(LogTemp, Log, TEXT("EnvArt_DinosaurPropSpawner: Spawned %s at %s"), 
                *PropLabel, *Location.ToString());
        }
        
        return PropActor;
    }
    
    return nullptr;
}

UStaticMesh* AEnvArt_DinosaurPropSpawner::GetMeshForPropType(EEnvArt_PropType PropType) const
{
    // Return nullptr for now - will be replaced with actual mesh loading in UE5
    // This prevents compilation errors while allowing the system to be tested
    return nullptr;
}

void AEnvArt_DinosaurPropSpawner::SpawnPropsInAllBiomes()
{
    if (PropTypesToSpawn.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("EnvArt_DinosaurPropSpawner: No prop types specified for spawning"));
        return;
    }

    int32 TotalSpawned = 0;

    for (const FEnvArt_BiomeCoordinates& Biome : BiomeCoordinates)
    {
        int32 SpawnedInBiome = 0;
        int32 Attempts = 0;
        const int32 MaxAttempts = PropsPerBiome * 3; // Allow multiple attempts per desired spawn

        while (SpawnedInBiome < PropsPerBiome && Attempts < MaxAttempts)
        {
            FVector SpawnLocation = GetRandomLocationInBiome(Biome);
            
            if (IsValidSpawnLocation(SpawnLocation))
            {
                // Pick a random prop type
                EEnvArt_PropType RandomPropType = PropTypesToSpawn[FMath::RandRange(0, PropTypesToSpawn.Num() - 1)];
                
                AActor* SpawnedProp = SpawnPropAtLocation(RandomPropType, SpawnLocation);
                if (SpawnedProp)
                {
                    SpawnedInBiome++;
                    TotalSpawned++;
                }
            }
            
            Attempts++;
        }

        if (bDebugSpawning)
        {
            UE_LOG(LogTemp, Log, TEXT("EnvArt_DinosaurPropSpawner: Spawned %d props in %s biome"), 
                SpawnedInBiome, *Biome.BiomeName);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("EnvArt_DinosaurPropSpawner: Total props spawned: %d"), TotalSpawned);
}

void AEnvArt_DinosaurPropSpawner::SpawnPropsInBiome(const FString& BiomeName, int32 Count)
{
    const FEnvArt_BiomeCoordinates* TargetBiome = BiomeCoordinates.FindByPredicate(
        [&BiomeName](const FEnvArt_BiomeCoordinates& Biome)
        {
            return Biome.BiomeName.Equals(BiomeName, ESearchCase::IgnoreCase);
        });

    if (!TargetBiome)
    {
        UE_LOG(LogTemp, Warning, TEXT("EnvArt_DinosaurPropSpawner: Biome '%s' not found"), *BiomeName);
        return;
    }

    int32 SpawnedCount = 0;
    int32 Attempts = 0;
    const int32 MaxAttempts = Count * 3;

    while (SpawnedCount < Count && Attempts < MaxAttempts)
    {
        FVector SpawnLocation = GetRandomLocationInBiome(*TargetBiome);
        
        if (IsValidSpawnLocation(SpawnLocation))
        {
            if (PropTypesToSpawn.Num() > 0)
            {
                EEnvArt_PropType RandomPropType = PropTypesToSpawn[FMath::RandRange(0, PropTypesToSpawn.Num() - 1)];
                AActor* SpawnedProp = SpawnPropAtLocation(RandomPropType, SpawnLocation);
                if (SpawnedProp)
                {
                    SpawnedCount++;
                }
            }
        }
        
        Attempts++;
    }

    UE_LOG(LogTemp, Log, TEXT("EnvArt_DinosaurPropSpawner: Spawned %d props in %s"), SpawnedCount, *BiomeName);
}

void AEnvArt_DinosaurPropSpawner::ClearAllSpawnedProps()
{
    for (AActor* PropActor : SpawnedProps)
    {
        if (PropActor && IsValid(PropActor))
        {
            PropActor->Destroy();
        }
    }
    
    SpawnedProps.Empty();
    UE_LOG(LogTemp, Log, TEXT("EnvArt_DinosaurPropSpawner: Cleared all spawned props"));
}

void AEnvArt_DinosaurPropSpawner::LogBiomeInfo() const
{
    UE_LOG(LogTemp, Log, TEXT("EnvArt_DinosaurPropSpawner: Biome Information:"));
    for (const FEnvArt_BiomeCoordinates& Biome : BiomeCoordinates)
    {
        UE_LOG(LogTemp, Log, TEXT("  %s: Center=%s, Radius=%.1f"), 
            *Biome.BiomeName, *Biome.CenterLocation.ToString(), Biome.SpawnRadius);
    }
}