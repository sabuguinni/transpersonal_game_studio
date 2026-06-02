#include "Eng_BiomeManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"

UEng_BiomeManager::UEng_BiomeManager()
{
    MaxTotalActors = 20000;
    MaxActorsPerBiome = 4000;
    LastCleanupTime = 0.0f;
}

void UEng_BiomeManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect: BiomeManager Initialize"));
    
    SetupDefaultBiomes();
    InitializeBiomes();
}

void UEng_BiomeManager::Deinitialize()
{
    // Clean up biome actor registry
    BiomeActorRegistry.Empty();
    BiomeRegions.Empty();
    BiomeConfigurations.Empty();
    
    Super::Deinitialize();
}

void UEng_BiomeManager::InitializeBiomes()
{
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Initializing Biomes"));
    
    // Create 5 biome regions across the map
    CreateBiomeRegion(EBiomeType::Grassland, FVector(0, 0, 0), 15000.0f);
    CreateBiomeRegion(EBiomeType::Forest, FVector(20000, 0, 0), 12000.0f);
    CreateBiomeRegion(EBiomeType::Desert, FVector(-20000, 0, 0), 10000.0f);
    CreateBiomeRegion(EBiomeType::Mountain, FVector(0, 20000, 0), 8000.0f);
    CreateBiomeRegion(EBiomeType::Swamp, FVector(0, -20000, 0), 9000.0f);
    
    // Initialize actor registry for each biome
    for (const auto& BiomePair : BiomeConfigurations)
    {
        BiomeActorRegistry.Add(BiomePair.Key, TArray<TWeakObjectPtr<AActor>>());
    }
}

void UEng_BiomeManager::SetupDefaultBiomes()
{
    // Grassland Configuration
    FEng_BiomeData GrasslandData;
    GrasslandData.BiomeType = EBiomeType::Grassland;
    GrasslandData.Temperature = 22.0f;
    GrasslandData.Humidity = 0.6f;
    GrasslandData.Elevation = 100.0f;
    GrasslandData.MaxActorsPerBiome = 4000;
    BiomeConfigurations.Add(EBiomeType::Grassland, GrasslandData);
    
    // Forest Configuration
    FEng_BiomeData ForestData;
    ForestData.BiomeType = EBiomeType::Forest;
    ForestData.Temperature = 18.0f;
    ForestData.Humidity = 0.8f;
    ForestData.Elevation = 200.0f;
    ForestData.MaxActorsPerBiome = 4000;
    BiomeConfigurations.Add(EBiomeType::Forest, ForestData);
    
    // Desert Configuration
    FEng_BiomeData DesertData;
    DesertData.BiomeType = EBiomeType::Desert;
    DesertData.Temperature = 35.0f;
    DesertData.Humidity = 0.2f;
    DesertData.Elevation = 50.0f;
    DesertData.MaxActorsPerBiome = 4000;
    BiomeConfigurations.Add(EBiomeType::Desert, DesertData);
    
    // Mountain Configuration
    FEng_BiomeData MountainData;
    MountainData.BiomeType = EBiomeType::Mountain;
    MountainData.Temperature = 8.0f;
    MountainData.Humidity = 0.4f;
    MountainData.Elevation = 1500.0f;
    MountainData.MaxActorsPerBiome = 4000;
    BiomeConfigurations.Add(EBiomeType::Mountain, MountainData);
    
    // Swamp Configuration
    FEng_BiomeData SwampData;
    SwampData.BiomeType = EBiomeType::Swamp;
    SwampData.Temperature = 26.0f;
    SwampData.Humidity = 0.9f;
    SwampData.Elevation = 20.0f;
    SwampData.MaxActorsPerBiome = 4000;
    BiomeConfigurations.Add(EBiomeType::Swamp, SwampData);
}

void UEng_BiomeManager::CreateBiomeRegion(EBiomeType BiomeType, const FVector& Center, float Radius)
{
    FEng_BiomeRegion NewRegion;
    NewRegion.Center = Center;
    NewRegion.Radius = Radius;
    
    if (BiomeConfigurations.Contains(BiomeType))
    {
        NewRegion.BiomeData = BiomeConfigurations[BiomeType];
    }
    
    BiomeRegions.Add(BiomeType, NewRegion);
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Created biome region %d at %s with radius %f"), 
           (int32)BiomeType, *Center.ToString(), Radius);
}

EBiomeType UEng_BiomeManager::GetBiomeAtLocation(const FVector& Location) const
{
    float ClosestDistance = FLT_MAX;
    EBiomeType ClosestBiome = EBiomeType::Grassland;
    
    for (const auto& RegionPair : BiomeRegions)
    {
        const FEng_BiomeRegion& Region = RegionPair.Value;
        float Distance = FVector::Dist(Location, Region.Center);
        
        if (Distance < Region.Radius && Distance < ClosestDistance)
        {
            ClosestDistance = Distance;
            ClosestBiome = RegionPair.Key;
        }
    }
    
    return ClosestBiome;
}

FEng_BiomeData UEng_BiomeManager::GetBiomeData(EBiomeType BiomeType) const
{
    if (BiomeConfigurations.Contains(BiomeType))
    {
        return BiomeConfigurations[BiomeType];
    }
    
    return FEng_BiomeData(); // Return default
}

bool UEng_BiomeManager::CanSpawnActorInBiome(EBiomeType BiomeType) const
{
    int32 CurrentCount = GetBiomeActorCount(BiomeType);
    int32 MaxCount = MaxActorsPerBiome;
    
    if (BiomeConfigurations.Contains(BiomeType))
    {
        MaxCount = BiomeConfigurations[BiomeType].MaxActorsPerBiome;
    }
    
    return CurrentCount < MaxCount && GetTotalActorCount() < MaxTotalActors;
}

void UEng_BiomeManager::RegisterActorToBiome(AActor* Actor, EBiomeType BiomeType)
{
    if (!Actor)
    {
        return;
    }
    
    if (!BiomeActorRegistry.Contains(BiomeType))
    {
        BiomeActorRegistry.Add(BiomeType, TArray<TWeakObjectPtr<AActor>>());
    }
    
    BiomeActorRegistry[BiomeType].Add(Actor);
    
    // Check if we need cleanup
    if (!IsWithinActorLimits())
    {
        CleanupExcessActors();
    }
}

void UEng_BiomeManager::UnregisterActorFromBiome(AActor* Actor, EBiomeType BiomeType)
{
    if (!Actor || !BiomeActorRegistry.Contains(BiomeType))
    {
        return;
    }
    
    BiomeActorRegistry[BiomeType].RemoveAll([Actor](const TWeakObjectPtr<AActor>& WeakActor)
    {
        return !WeakActor.IsValid() || WeakActor.Get() == Actor;
    });
}

int32 UEng_BiomeManager::GetTotalActorCount() const
{
    int32 TotalCount = 0;
    
    for (const auto& RegistryPair : BiomeActorRegistry)
    {
        // Count only valid actors
        for (const TWeakObjectPtr<AActor>& WeakActor : RegistryPair.Value)
        {
            if (WeakActor.IsValid())
            {
                TotalCount++;
            }
        }
    }
    
    return TotalCount;
}

int32 UEng_BiomeManager::GetBiomeActorCount(EBiomeType BiomeType) const
{
    if (!BiomeActorRegistry.Contains(BiomeType))
    {
        return 0;
    }
    
    int32 ValidCount = 0;
    for (const TWeakObjectPtr<AActor>& WeakActor : BiomeActorRegistry[BiomeType])
    {
        if (WeakActor.IsValid())
        {
            ValidCount++;
        }
    }
    
    return ValidCount;
}

void UEng_BiomeManager::CleanupExcessActors()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    float CurrentTime = World->GetTimeSeconds();
    if (CurrentTime - LastCleanupTime < CleanupInterval)
    {
        return; // Don't cleanup too frequently
    }
    
    LastCleanupTime = CurrentTime;
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Cleaning up excess actors"));
    
    // Clean up invalid references first
    for (auto& RegistryPair : BiomeActorRegistry)
    {
        RegistryPair.Value.RemoveAll([](const TWeakObjectPtr<AActor>& WeakActor)
        {
            return !WeakActor.IsValid();
        });
    }
    
    // Remove excess actors from biomes that exceed limits
    for (const auto& BiomePair : BiomeConfigurations)
    {
        EBiomeType BiomeType = BiomePair.Key;
        int32 MaxActors = BiomePair.Value.MaxActorsPerBiome;
        int32 CurrentActors = GetBiomeActorCount(BiomeType);
        
        if (CurrentActors > MaxActors)
        {
            int32 ExcessCount = CurrentActors - MaxActors;
            RemoveOldestActorsFromBiome(BiomeType, ExcessCount);
        }
    }
}

void UEng_BiomeManager::RemoveOldestActorsFromBiome(EBiomeType BiomeType, int32 CountToRemove)
{
    if (!BiomeActorRegistry.Contains(BiomeType) || CountToRemove <= 0)
    {
        return;
    }
    
    TArray<TWeakObjectPtr<AActor>>& ActorArray = BiomeActorRegistry[BiomeType];
    int32 ActorsRemoved = 0;
    
    // Remove from the beginning (oldest actors)
    for (int32 i = 0; i < ActorArray.Num() && ActorsRemoved < CountToRemove; ++i)
    {
        if (ActorArray[i].IsValid())
        {
            AActor* ActorToDestroy = ActorArray[i].Get();
            if (ActorToDestroy)
            {
                // Don't destroy critical actors like PlayerStart, GameMode, etc.
                if (!ActorToDestroy->IsA<APlayerStart>() && 
                    !ActorToDestroy->IsA<AGameModeBase>() &&
                    !ActorToDestroy->GetName().Contains(TEXT("PlayerStart")))
                {
                    ActorToDestroy->Destroy();
                    ActorsRemoved++;
                }
            }
        }
    }
    
    // Clean up the array
    ActorArray.RemoveAll([](const TWeakObjectPtr<AActor>& WeakActor)
    {
        return !WeakActor.IsValid();
    });
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Removed %d excess actors from biome %d"), 
           ActorsRemoved, (int32)BiomeType);
}

bool UEng_BiomeManager::IsWithinActorLimits() const
{
    return GetTotalActorCount() <= MaxTotalActors;
}

float UEng_BiomeManager::GetTemperatureAtLocation(const FVector& Location) const
{
    EBiomeType BiomeType = GetBiomeAtLocation(Location);
    FEng_BiomeData BiomeData = GetBiomeData(BiomeType);
    
    // Add some variation based on elevation
    float ElevationModifier = (Location.Z - BiomeData.Elevation) * -0.01f; // Cooler at higher elevation
    
    return BiomeData.Temperature + ElevationModifier;
}

float UEng_BiomeManager::GetHumidityAtLocation(const FVector& Location) const
{
    EBiomeType BiomeType = GetBiomeAtLocation(Location);
    FEng_BiomeData BiomeData = GetBiomeData(BiomeType);
    
    return BiomeData.Humidity;
}

float UEng_BiomeManager::GetElevationAtLocation(const FVector& Location) const
{
    EBiomeType BiomeType = GetBiomeAtLocation(Location);
    FEng_BiomeData BiomeData = GetBiomeData(BiomeType);
    
    return BiomeData.Elevation;
}

float UEng_BiomeManager::GetBiomeTransitionWeight(const FVector& Location, EBiomeType BiomeType) const
{
    if (!BiomeRegions.Contains(BiomeType))
    {
        return 0.0f;
    }
    
    const FEng_BiomeRegion& Region = BiomeRegions[BiomeType];
    float Distance = FVector::Dist(Location, Region.Center);
    
    if (Distance >= Region.Radius)
    {
        return 0.0f;
    }
    
    // Linear falloff from center to edge
    return 1.0f - (Distance / Region.Radius);
}

TArray<EBiomeType> UEng_BiomeManager::GetNearbyBiomes(const FVector& Location, float Radius) const
{
    TArray<EBiomeType> NearbyBiomes;
    
    for (const auto& RegionPair : BiomeRegions)
    {
        const FEng_BiomeRegion& Region = RegionPair.Value;
        float Distance = FVector::Dist(Location, Region.Center);
        
        if (Distance <= (Region.Radius + Radius))
        {
            NearbyBiomes.Add(RegionPair.Key);
        }
    }
    
    return NearbyBiomes;
}

void UEng_BiomeManager::ValidateActorLimits()
{
    int32 TotalActors = GetTotalActorCount();
    
    if (TotalActors > MaxTotalActors)
    {
        UE_LOG(LogTemp, Error, TEXT("Engine Architect: CRITICAL - Total actors (%d) exceeds limit (%d)"), 
               TotalActors, MaxTotalActors);
        CleanupExcessActors();
    }
    
    for (const auto& BiomePair : BiomeConfigurations)
    {
        EBiomeType BiomeType = BiomePair.Key;
        int32 BiomeActors = GetBiomeActorCount(BiomeType);
        int32 BiomeLimit = BiomePair.Value.MaxActorsPerBiome;
        
        if (BiomeActors > BiomeLimit)
        {
            UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Biome %d has %d actors (limit: %d)"), 
                   (int32)BiomeType, BiomeActors, BiomeLimit);
        }
    }
}