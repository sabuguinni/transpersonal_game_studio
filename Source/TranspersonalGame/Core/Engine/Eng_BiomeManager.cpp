#include "Eng_BiomeManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

UEng_BiomeManager::UEng_BiomeManager()
{
    // Constructor
}

void UEng_BiomeManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("BiomeManager: Initializing biome subsystem"));
    SetupDefaultBiomes();
    InitializeBiomes();
}

void UEng_BiomeManager::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("BiomeManager: Deinitializing biome subsystem"));
    BiomeConfigurations.Empty();
    
    Super::Deinitialize();
}

void UEng_BiomeManager::SetupDefaultBiomes()
{
    // Savanna biome (center of map)
    FEng_BiomeConfiguration Savanna;
    Savanna.BiomeType = EBiomeType::Savanna;
    Savanna.CenterLocation = FVector(0, 0, 100);
    Savanna.Radius = 15000.0f;
    Savanna.Temperature = 28.0f;
    Savanna.Humidity = 0.3f;
    Savanna.Density = 0.7f;
    Savanna.AllowedVegetation = {"AcaciaTree", "SavannaGrass", "BaobabTree"};
    Savanna.AllowedDinosaurs = {"TRex", "Triceratops", "Parasaurolophus"};
    BiomeConfigurations.Add(EBiomeType::Savanna, Savanna);

    // Swamp biome (southwest)
    FEng_BiomeConfiguration Swamp;
    Swamp.BiomeType = EBiomeType::Swamp;
    Swamp.CenterLocation = FVector(-50000, -45000, 50);
    Swamp.Radius = 15000.0f;
    Swamp.Temperature = 24.0f;
    Swamp.Humidity = 0.9f;
    Swamp.Density = 1.2f;
    Swamp.AllowedVegetation = {"CypressTree", "SwampGrass", "Ferns"};
    Swamp.AllowedDinosaurs = {"Velociraptor", "Ankylosaurus", "Parasaurolophus"};
    BiomeConfigurations.Add(EBiomeType::Swamp, Swamp);

    // Forest biome (northwest)
    FEng_BiomeConfiguration Forest;
    Forest.BiomeType = EBiomeType::Forest;
    Forest.CenterLocation = FVector(-45000, 40000, 200);
    Forest.Radius = 15000.0f;
    Forest.Temperature = 20.0f;
    Forest.Humidity = 0.8f;
    Forest.Density = 1.5f;
    Forest.AllowedVegetation = {"PineTree", "OakTree", "Ferns", "Bushes"};
    Forest.AllowedDinosaurs = {"Brachiosaurus", "Velociraptor", "Triceratops"};
    BiomeConfigurations.Add(EBiomeType::Forest, Forest);

    // Desert biome (east)
    FEng_BiomeConfiguration Desert;
    Desert.BiomeType = EBiomeType::Desert;
    Desert.CenterLocation = FVector(55000, 0, 150);
    Desert.Radius = 15000.0f;
    Desert.Temperature = 35.0f;
    Desert.Humidity = 0.1f;
    Desert.Density = 0.3f;
    Desert.AllowedVegetation = {"Cactus", "DesertShrub"};
    Desert.AllowedDinosaurs = {"TRex", "Ankylosaurus"};
    BiomeConfigurations.Add(EBiomeType::Desert, Desert);

    // Mountain biome (northeast)
    FEng_BiomeConfiguration Mountain;
    Mountain.BiomeType = EBiomeType::Mountain;
    Mountain.CenterLocation = FVector(40000, 50000, 500);
    Mountain.Radius = 15000.0f;
    Mountain.Temperature = 15.0f;
    Mountain.Humidity = 0.6f;
    Mountain.Density = 0.8f;
    Mountain.AllowedVegetation = {"PineTree", "MountainGrass", "Rocks"};
    Mountain.AllowedDinosaurs = {"Velociraptor", "Triceratops"};
    BiomeConfigurations.Add(EBiomeType::Mountain, Mountain);
}

void UEng_BiomeManager::InitializeBiomes()
{
    UE_LOG(LogTemp, Warning, TEXT("BiomeManager: Initialized %d biomes"), BiomeConfigurations.Num());
    
    for (auto& BiomePair : BiomeConfigurations)
    {
        FEng_BiomeConfiguration& Config = BiomePair.Value;
        Config.CurrentActorCount = 0;
        
        UE_LOG(LogTemp, Log, TEXT("BiomeManager: Setup biome %s at location (%f, %f, %f)"), 
            *UEnum::GetValueAsString(Config.BiomeType),
            Config.CenterLocation.X, Config.CenterLocation.Y, Config.CenterLocation.Z);
    }
}

EBiomeType UEng_BiomeManager::GetBiomeAtLocation(const FVector& Location) const
{
    return FindClosestBiome(Location);
}

FEng_BiomeConfiguration UEng_BiomeManager::GetBiomeConfiguration(EBiomeType BiomeType) const
{
    if (const FEng_BiomeConfiguration* Config = BiomeConfigurations.Find(BiomeType))
    {
        return *Config;
    }
    
    // Return default savanna if not found
    return BiomeConfigurations[EBiomeType::Savanna];
}

bool UEng_BiomeManager::CanSpawnActorInBiome(EBiomeType BiomeType, const FString& ActorType) const
{
    const FEng_BiomeConfiguration* Config = BiomeConfigurations.Find(BiomeType);
    if (!Config)
    {
        return false;
    }

    // Check if biome is overcrowded
    if (Config->CurrentActorCount >= Config->MaxActors)
    {
        return false;
    }

    // Check if global limit is reached
    if (GlobalCurrentActors >= GlobalMaxActors)
    {
        return false;
    }

    // Check if actor type is allowed in this biome
    return Config->AllowedVegetation.Contains(ActorType) || Config->AllowedDinosaurs.Contains(ActorType);
}

void UEng_BiomeManager::RegisterActorInBiome(EBiomeType BiomeType)
{
    if (FEng_BiomeConfiguration* Config = BiomeConfigurations.Find(BiomeType))
    {
        Config->CurrentActorCount++;
        GlobalCurrentActors++;
    }
}

void UEng_BiomeManager::UnregisterActorFromBiome(EBiomeType BiomeType)
{
    if (FEng_BiomeConfiguration* Config = BiomeConfigurations.Find(BiomeType))
    {
        Config->CurrentActorCount = FMath::Max(0, Config->CurrentActorCount - 1);
        GlobalCurrentActors = FMath::Max(0, GlobalCurrentActors - 1);
    }
}

TArray<FVector> UEng_BiomeManager::GetSpawnLocationsInBiome(EBiomeType BiomeType, int32 Count) const
{
    TArray<FVector> Locations;
    
    const FEng_BiomeConfiguration* Config = BiomeConfigurations.Find(BiomeType);
    if (!Config)
    {
        return Locations;
    }

    for (int32 i = 0; i < Count; i++)
    {
        // Generate random location within biome radius
        float Angle = FMath::RandRange(0.0f, 2.0f * PI);
        float Distance = FMath::RandRange(0.0f, Config->Radius * 0.8f); // Stay within 80% of radius
        
        FVector RandomOffset;
        RandomOffset.X = FMath::Cos(Angle) * Distance;
        RandomOffset.Y = FMath::Sin(Angle) * Distance;
        RandomOffset.Z = FMath::RandRange(-100.0f, 100.0f); // Some height variation
        
        FVector SpawnLocation = Config->CenterLocation + RandomOffset;
        Locations.Add(SpawnLocation);
    }

    return Locations;
}

float UEng_BiomeManager::GetBiomeTemperature(const FVector& Location) const
{
    EBiomeType BiomeType = GetBiomeAtLocation(Location);
    const FEng_BiomeConfiguration* Config = BiomeConfigurations.Find(BiomeType);
    return Config ? Config->Temperature : 25.0f;
}

float UEng_BiomeManager::GetBiomeHumidity(const FVector& Location) const
{
    EBiomeType BiomeType = GetBiomeAtLocation(Location);
    const FEng_BiomeConfiguration* Config = BiomeConfigurations.Find(BiomeType);
    return Config ? Config->Humidity : 0.5f;
}

void UEng_BiomeManager::UpdateBiomeActorCounts()
{
    // Reset all counts
    GlobalCurrentActors = 0;
    for (auto& BiomePair : BiomeConfigurations)
    {
        BiomePair.Value.CurrentActorCount = 0;
    }

    // Count all actors in the world
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);

    for (AActor* Actor : AllActors)
    {
        if (!Actor)
        {
            continue;
        }

        EBiomeType BiomeType = GetBiomeAtLocation(Actor->GetActorLocation());
        RegisterActorInBiome(BiomeType);
    }

    UE_LOG(LogTemp, Warning, TEXT("BiomeManager: Updated actor counts - Global: %d"), GlobalCurrentActors);
}

bool UEng_BiomeManager::IsBiomeOvercrowded(EBiomeType BiomeType) const
{
    const FEng_BiomeConfiguration* Config = BiomeConfigurations.Find(BiomeType);
    return Config ? (Config->CurrentActorCount >= Config->MaxActors) : false;
}

void UEng_BiomeManager::CleanupOvercrowdedBiomes()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    for (auto& BiomePair : BiomeConfigurations)
    {
        EBiomeType BiomeType = BiomePair.Key;
        FEng_BiomeConfiguration& Config = BiomePair.Value;
        
        if (Config.CurrentActorCount > Config.MaxActors)
        {
            int32 ExcessActors = Config.CurrentActorCount - Config.MaxActors;
            UE_LOG(LogTemp, Warning, TEXT("BiomeManager: Cleaning up %d excess actors in biome %s"), 
                ExcessActors, *UEnum::GetValueAsString(BiomeType));
            
            // This would need to be implemented with specific cleanup logic
            // For now, just log the need for cleanup
        }
    }
}

float UEng_BiomeManager::CalculateDistanceFromBiomeCenter(const FVector& Location, const FEng_BiomeConfiguration& Biome) const
{
    return FVector::Dist2D(Location, Biome.CenterLocation);
}

EBiomeType UEng_BiomeManager::FindClosestBiome(const FVector& Location) const
{
    EBiomeType ClosestBiome = EBiomeType::Savanna;
    float ClosestDistance = MAX_FLT;

    for (const auto& BiomePair : BiomeConfigurations)
    {
        const FEng_BiomeConfiguration& Config = BiomePair.Value;
        float Distance = CalculateDistanceFromBiomeCenter(Location, Config);
        
        if (Distance < ClosestDistance)
        {
            ClosestDistance = Distance;
            ClosestBiome = Config.BiomeType;
        }
    }

    return ClosestBiome;
}