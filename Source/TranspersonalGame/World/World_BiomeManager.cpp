#include "World_BiomeManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/StaticMesh.h"
#include "Math/UnrealMathUtility.h"

void UWorld_BiomeManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    UE_LOG(LogTemp, Warning, TEXT("World_BiomeManager: Initializing biome management system"));
    SetupDefaultBiomes();
}

void UWorld_BiomeManager::Deinitialize()
{
    BiomeActors.Empty();
    Super::Deinitialize();
}

void UWorld_BiomeManager::SetupDefaultBiomes()
{
    BiomeConfigs.Empty();

    // Savana (center of map)
    FWorld_BiomeConfig Savana;
    Savana.BiomeName = TEXT("Savana");
    Savana.CenterLocation = FVector(0.0f, 0.0f, 100.0f);
    Savana.Radius = 15000.0f;
    Savana.VegetationDensity = 0.3f;
    Savana.RockDensity = 0.2f;
    Savana.MaxActorsPerBiome = 1000;
    BiomeConfigs.Add(Savana);

    // Pantano (swamp)
    FWorld_BiomeConfig Pantano;
    Pantano.BiomeName = TEXT("Pantano");
    Pantano.CenterLocation = FVector(-50000.0f, -45000.0f, 100.0f);
    Pantano.Radius = 15000.0f;
    Pantano.VegetationDensity = 0.7f;
    Pantano.RockDensity = 0.1f;
    Pantano.MaxActorsPerBiome = 1000;
    BiomeConfigs.Add(Pantano);

    // Floresta (forest)
    FWorld_BiomeConfig Floresta;
    Floresta.BiomeName = TEXT("Floresta");
    Floresta.CenterLocation = FVector(-45000.0f, 40000.0f, 100.0f);
    Floresta.Radius = 15000.0f;
    Floresta.VegetationDensity = 0.8f;
    Floresta.RockDensity = 0.3f;
    Floresta.MaxActorsPerBiome = 1000;
    BiomeConfigs.Add(Floresta);

    // Deserto (desert)
    FWorld_BiomeConfig Deserto;
    Deserto.BiomeName = TEXT("Deserto");
    Deserto.CenterLocation = FVector(55000.0f, 0.0f, 100.0f);
    Deserto.Radius = 15000.0f;
    Deserto.VegetationDensity = 0.1f;
    Deserto.RockDensity = 0.5f;
    Deserto.MaxActorsPerBiome = 1000;
    BiomeConfigs.Add(Deserto);

    // Montanha (mountain)
    FWorld_BiomeConfig Montanha;
    Montanha.BiomeName = TEXT("Montanha");
    Montanha.CenterLocation = FVector(40000.0f, 50000.0f, 100.0f);
    Montanha.Radius = 15000.0f;
    Montanha.VegetationDensity = 0.4f;
    Montanha.RockDensity = 0.8f;
    Montanha.MaxActorsPerBiome = 1000;
    BiomeConfigs.Add(Montanha);

    UE_LOG(LogTemp, Warning, TEXT("World_BiomeManager: Setup %d default biomes"), BiomeConfigs.Num());
}

void UWorld_BiomeManager::InitializeBiomes()
{
    UE_LOG(LogTemp, Warning, TEXT("World_BiomeManager: Initializing all biomes"));
    
    for (const FWorld_BiomeConfig& Biome : BiomeConfigs)
    {
        BiomeActors.Add(Biome.BiomeName, TArray<AActor*>());
        UE_LOG(LogTemp, Warning, TEXT("World_BiomeManager: Initialized biome %s at %s"), 
               *Biome.BiomeName, *Biome.CenterLocation.ToString());
    }
}

void UWorld_BiomeManager::PopulateBiome(const FString& BiomeName, int32 VegetationCount, int32 RockCount)
{
    const FWorld_BiomeConfig* BiomeConfig = BiomeConfigs.FindByPredicate([&BiomeName](const FWorld_BiomeConfig& Config)
    {
        return Config.BiomeName == BiomeName;
    });

    if (!BiomeConfig)
    {
        UE_LOG(LogTemp, Error, TEXT("World_BiomeManager: Biome %s not found"), *BiomeName);
        return;
    }

    // Check current actor count
    int32 CurrentActors = GetActorCountInBiome(BiomeName);
    int32 TotalToSpawn = VegetationCount + RockCount;
    
    if (CurrentActors + TotalToSpawn > BiomeConfig->MaxActorsPerBiome)
    {
        UE_LOG(LogTemp, Warning, TEXT("World_BiomeManager: Would exceed max actors in %s. Current: %d, Trying to add: %d, Max: %d"), 
               *BiomeName, CurrentActors, TotalToSpawn, BiomeConfig->MaxActorsPerBiome);
        return;
    }

    // Spawn vegetation
    for (int32 i = 0; i < VegetationCount; i++)
    {
        FVector Location = GetRandomLocationInBiome(BiomeName);
        AActor* VegActor = SpawnVegetationActor(Location);
        if (VegActor)
        {
            BiomeActors[BiomeName].Add(VegActor);
        }
    }

    // Spawn rocks
    for (int32 i = 0; i < RockCount; i++)
    {
        FVector Location = GetRandomLocationInBiome(BiomeName);
        AActor* RockActor = SpawnRockActor(Location);
        if (RockActor)
        {
            BiomeActors[BiomeName].Add(RockActor);
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("World_BiomeManager: Populated %s with %d vegetation and %d rocks"), 
           *BiomeName, VegetationCount, RockCount);
}

FVector UWorld_BiomeManager::GetRandomLocationInBiome(const FString& BiomeName)
{
    const FWorld_BiomeConfig* BiomeConfig = BiomeConfigs.FindByPredicate([&BiomeName](const FWorld_BiomeConfig& Config)
    {
        return Config.BiomeName == BiomeName;
    });

    if (!BiomeConfig)
    {
        return FVector::ZeroVector;
    }

    float RandomAngle = FMath::RandRange(0.0f, 2.0f * PI);
    float RandomRadius = FMath::RandRange(0.0f, BiomeConfig->Radius);
    
    FVector RandomOffset;
    RandomOffset.X = RandomRadius * FMath::Cos(RandomAngle);
    RandomOffset.Y = RandomRadius * FMath::Sin(RandomAngle);
    RandomOffset.Z = 0.0f;

    return BiomeConfig->CenterLocation + RandomOffset;
}

int32 UWorld_BiomeManager::GetActorCountInBiome(const FString& BiomeName)
{
    if (BiomeActors.Contains(BiomeName))
    {
        // Clean up null actors
        BiomeActors[BiomeName].RemoveAll([](AActor* Actor) { return !IsValid(Actor); });
        return BiomeActors[BiomeName].Num();
    }
    return 0;
}

void UWorld_BiomeManager::CleanupExcessActors()
{
    for (auto& BiomePair : BiomeActors)
    {
        TArray<AActor*>& Actors = BiomePair.Value;
        const FString& BiomeName = BiomePair.Key;
        
        const FWorld_BiomeConfig* BiomeConfig = BiomeConfigs.FindByPredicate([&BiomeName](const FWorld_BiomeConfig& Config)
        {
            return Config.BiomeName == BiomeName;
        });

        if (!BiomeConfig) continue;

        // Remove invalid actors
        Actors.RemoveAll([](AActor* Actor) { return !IsValid(Actor); });

        // If still over limit, remove oldest actors
        if (Actors.Num() > BiomeConfig->MaxActorsPerBiome)
        {
            int32 ToRemove = Actors.Num() - BiomeConfig->MaxActorsPerBiome;
            for (int32 i = 0; i < ToRemove; i++)
            {
                if (Actors.IsValidIndex(0) && IsValid(Actors[0]))
                {
                    Actors[0]->Destroy();
                    Actors.RemoveAt(0);
                }
            }
            UE_LOG(LogTemp, Warning, TEXT("World_BiomeManager: Cleaned up %d excess actors from %s"), ToRemove, *BiomeName);
        }
    }
}

TArray<FString> UWorld_BiomeManager::GetAllBiomeNames()
{
    TArray<FString> Names;
    for (const FWorld_BiomeConfig& Biome : BiomeConfigs)
    {
        Names.Add(Biome.BiomeName);
    }
    return Names;
}

AActor* UWorld_BiomeManager::SpawnVegetationActor(const FVector& Location)
{
    UWorld* World = GetWorld();
    if (!World) return nullptr;

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    AStaticMeshActor* VegActor = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Location, FRotator::ZeroRotator, SpawnParams);
    if (VegActor)
    {
        VegActor->SetActorLabel(TEXT("BiomeVegetation"));
        // Use basic cube for now - will be replaced with proper vegetation meshes
        UStaticMeshComponent* MeshComp = VegActor->GetStaticMeshComponent();
        if (MeshComp)
        {
            // Scale to look like a tree
            VegActor->SetActorScale3D(FVector(1.0f, 1.0f, 3.0f));
        }
    }
    return VegActor;
}

AActor* UWorld_BiomeManager::SpawnRockActor(const FVector& Location)
{
    UWorld* World = GetWorld();
    if (!World) return nullptr;

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    AStaticMeshActor* RockActor = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Location, FRotator::ZeroRotator, SpawnParams);
    if (RockActor)
    {
        RockActor->SetActorLabel(TEXT("BiomeRock"));
        // Random scale for variety
        float Scale = FMath::RandRange(0.5f, 2.0f);
        RockActor->SetActorScale3D(FVector(Scale, Scale, Scale * 0.7f));
    }
    return RockActor;
}

bool UWorld_BiomeManager::IsLocationInBiome(const FVector& Location, const FWorld_BiomeConfig& Biome)
{
    float Distance = FVector::Dist2D(Location, Biome.CenterLocation);
    return Distance <= Biome.Radius;
}