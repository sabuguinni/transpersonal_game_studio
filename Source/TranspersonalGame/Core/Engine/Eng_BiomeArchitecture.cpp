#include "Eng_BiomeArchitecture.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"

UEng_BiomeArchitecture::UEng_BiomeArchitecture()
{
    GlobalActorLimit = 8000;
    GlobalDinosaurLimit = 150;
}

void UEng_BiomeArchitecture::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("BiomeArchitecture: Initializing biome system"));
    SetupDefaultBiomes();
}

void UEng_BiomeArchitecture::Deinitialize()
{
    BiomeConfigurations.Empty();
    BiomeSpawnRules.Empty();
    
    Super::Deinitialize();
}

void UEng_BiomeArchitecture::InitializeBiomeSystem()
{
    UE_LOG(LogTemp, Warning, TEXT("BiomeArchitecture: Setting up 5-biome world configuration"));
    
    SetupDefaultBiomes();
    
    // Log biome setup completion
    for (const auto& BiomePair : BiomeConfigurations)
    {
        const FEng_BiomeConfiguration& Config = BiomePair.Value;
        UE_LOG(LogTemp, Warning, TEXT("BiomeArchitecture: Configured %s at location (%f, %f, %f) with radius %f"), 
            *UEnum::GetValueAsString(Config.BiomeType), 
            Config.CenterLocation.X, Config.CenterLocation.Y, Config.CenterLocation.Z,
            Config.Radius);
    }
}

void UEng_BiomeArchitecture::RegisterBiome(const FEng_BiomeConfiguration& BiomeConfig)
{
    BiomeConfigurations.Add(BiomeConfig.BiomeType, BiomeConfig);
    
    // Setup default spawn rules if not exists
    if (!BiomeSpawnRules.Contains(BiomeConfig.BiomeType))
    {
        FEng_BiomeSpawnRules DefaultRules;
        BiomeSpawnRules.Add(BiomeConfig.BiomeType, DefaultRules);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("BiomeArchitecture: Registered biome %s"), 
        *UEnum::GetValueAsString(BiomeConfig.BiomeType));
}

EBiomeType UEng_BiomeArchitecture::GetBiomeAtLocation(const FVector& WorldLocation) const
{
    return DetermineBiomeFromLocation(WorldLocation);
}

FEng_BiomeConfiguration UEng_BiomeArchitecture::GetBiomeConfiguration(EBiomeType BiomeType) const
{
    if (const FEng_BiomeConfiguration* Config = BiomeConfigurations.Find(BiomeType))
    {
        return *Config;
    }
    
    // Return default savanna configuration
    FEng_BiomeConfiguration DefaultConfig;
    DefaultConfig.BiomeType = EBiomeType::Savanna;
    return DefaultConfig;
}

bool UEng_BiomeArchitecture::IsLocationValidForBiome(const FVector& Location, EBiomeType BiomeType) const
{
    EBiomeType LocationBiome = GetBiomeAtLocation(Location);
    return LocationBiome == BiomeType;
}

bool UEng_BiomeArchitecture::CanSpawnActorInBiome(EBiomeType BiomeType, TSubclassOf<AActor> ActorClass) const
{
    // Check global limits first
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    int32 TotalActors = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        TotalActors++;
    }
    
    if (TotalActors >= GlobalActorLimit)
    {
        UE_LOG(LogTemp, Warning, TEXT("BiomeArchitecture: Global actor limit reached (%d/%d)"), 
            TotalActors, GlobalActorLimit);
        return false;
    }
    
    // Check biome-specific limits
    int32 BiomeActorCount = GetActorCountInBiome(BiomeType);
    const FEng_BiomeConfiguration& Config = GetBiomeConfiguration(BiomeType);
    
    if (BiomeActorCount >= Config.MaxActors)
    {
        UE_LOG(LogTemp, Warning, TEXT("BiomeArchitecture: Biome %s actor limit reached (%d/%d)"), 
            *UEnum::GetValueAsString(BiomeType), BiomeActorCount, Config.MaxActors);
        return false;
    }
    
    return true;
}

int32 UEng_BiomeArchitecture::GetActorCountInBiome(EBiomeType BiomeType) const
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return 0;
    }
    
    int32 Count = 0;
    const FEng_BiomeConfiguration& Config = GetBiomeConfiguration(BiomeType);
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && IsLocationValidForBiome(Actor->GetActorLocation(), BiomeType))
        {
            Count++;
        }
    }
    
    return Count;
}

void UEng_BiomeArchitecture::EnforceActorLimits()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Count total actors
    TArray<AActor*> AllActors;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AllActors.Add(*ActorItr);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("BiomeArchitecture: Total actors in world: %d (limit: %d)"), 
        AllActors.Num(), GlobalActorLimit);
    
    // If over global limit, remove oldest actors (excluding critical ones)
    if (AllActors.Num() > GlobalActorLimit)
    {
        int32 ToRemove = AllActors.Num() - GlobalActorLimit;
        int32 Removed = 0;
        
        for (AActor* Actor : AllActors)
        {
            if (Removed >= ToRemove) break;
            
            // Don't remove critical actors
            if (Actor && !Actor->GetName().Contains(TEXT("PlayerStart")) && 
                !Actor->GetName().Contains(TEXT("GameMode")) &&
                !Actor->GetName().Contains(TEXT("Character")))
            {
                Actor->Destroy();
                Removed++;
            }
        }
        
        UE_LOG(LogTemp, Warning, TEXT("BiomeArchitecture: Removed %d excess actors"), Removed);
    }
}

FVector UEng_BiomeArchitecture::GetRandomLocationInBiome(EBiomeType BiomeType) const
{
    const FEng_BiomeConfiguration& Config = GetBiomeConfiguration(BiomeType);
    
    // Generate random point within biome radius
    float RandomAngle = FMath::RandRange(0.0f, 2.0f * PI);
    float RandomRadius = FMath::RandRange(0.0f, Config.Radius * 0.8f); // Stay within 80% of radius
    
    FVector RandomOffset;
    RandomOffset.X = FMath::Cos(RandomAngle) * RandomRadius;
    RandomOffset.Y = FMath::Sin(RandomAngle) * RandomRadius;
    RandomOffset.Z = 100.0f; // Default height
    
    return Config.CenterLocation + RandomOffset;
}

float UEng_BiomeArchitecture::GetTemperatureAtLocation(const FVector& Location) const
{
    EBiomeType BiomeType = GetBiomeAtLocation(Location);
    const FEng_BiomeConfiguration& Config = GetBiomeConfiguration(BiomeType);
    
    // Base temperature varies by biome type
    float BaseTemp = 20.0f;
    switch (BiomeType)
    {
        case EBiomeType::Desert:
            BaseTemp = 35.0f;
            break;
        case EBiomeType::Swamp:
            BaseTemp = 28.0f;
            break;
        case EBiomeType::Forest:
            BaseTemp = 22.0f;
            break;
        case EBiomeType::Mountain:
            BaseTemp = 15.0f;
            break;
        case EBiomeType::Savanna:
        default:
            BaseTemp = 25.0f;
            break;
    }
    
    return BaseTemp + FMath::RandRange(-Config.TemperatureRange * 0.2f, Config.TemperatureRange * 0.2f);
}

float UEng_BiomeArchitecture::GetHumidityAtLocation(const FVector& Location) const
{
    EBiomeType BiomeType = GetBiomeAtLocation(Location);
    const FEng_BiomeConfiguration& Config = GetBiomeConfiguration(BiomeType);
    
    return Config.HumidityLevel + FMath::RandRange(-0.1f, 0.1f);
}

FEng_BiomeSpawnRules UEng_BiomeArchitecture::GetSpawnRulesForBiome(EBiomeType BiomeType) const
{
    if (const FEng_BiomeSpawnRules* Rules = BiomeSpawnRules.Find(BiomeType))
    {
        return *Rules;
    }
    
    return FEng_BiomeSpawnRules();
}

void UEng_BiomeArchitecture::ValidateBiomeDistribution()
{
    UE_LOG(LogTemp, Warning, TEXT("BiomeArchitecture: Validating biome distribution across world"));
    
    for (const auto& BiomePair : BiomeConfigurations)
    {
        EBiomeType BiomeType = BiomePair.Key;
        int32 ActorCount = GetActorCountInBiome(BiomeType);
        const FEng_BiomeConfiguration& Config = BiomePair.Value;
        
        UE_LOG(LogTemp, Warning, TEXT("BiomeArchitecture: %s - Actors: %d/%d (%.1f%% capacity)"), 
            *UEnum::GetValueAsString(BiomeType), 
            ActorCount, Config.MaxActors, 
            (float)ActorCount / Config.MaxActors * 100.0f);
    }
}

void UEng_BiomeArchitecture::LogBiomeStatistics()
{
    UE_LOG(LogTemp, Warning, TEXT("=== BIOME ARCHITECTURE STATISTICS ==="));
    
    for (const auto& BiomePair : BiomeConfigurations)
    {
        const FEng_BiomeConfiguration& Config = BiomePair.Value;
        int32 ActorCount = GetActorCountInBiome(BiomePair.Key);
        
        UE_LOG(LogTemp, Warning, TEXT("%s: Center(%f,%f,%f) Radius:%.0f Actors:%d/%d Temp:%.1f Humidity:%.2f"), 
            *UEnum::GetValueAsString(Config.BiomeType),
            Config.CenterLocation.X, Config.CenterLocation.Y, Config.CenterLocation.Z,
            Config.Radius, ActorCount, Config.MaxActors,
            Config.TemperatureRange, Config.HumidityLevel);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=== END BIOME STATISTICS ==="));
}

void UEng_BiomeArchitecture::SetupDefaultBiomes()
{
    BiomeConfigurations.Empty();
    BiomeSpawnRules.Empty();
    
    InitializeSavannaBiome();
    InitializeSwampBiome();
    InitializeForestBiome();
    InitializeDesertBiome();
    InitializeMountainBiome();
}

void UEng_BiomeArchitecture::InitializeSavannaBiome()
{
    FEng_BiomeConfiguration SavannaConfig;
    SavannaConfig.BiomeType = EBiomeType::Savanna;
    SavannaConfig.CenterLocation = FVector(0.0f, 0.0f, 100.0f);
    SavannaConfig.Radius = 15000.0f;
    SavannaConfig.TemperatureRange = 25.0f;
    SavannaConfig.HumidityLevel = 0.3f;
    SavannaConfig.MaxActors = 1600; // 20% of 8000 global limit
    SavannaConfig.MaxDinosaurs = 30;
    
    FEng_BiomeSpawnRules SavannaRules;
    SavannaRules.VegetationDensity = 0.2f;
    SavannaRules.RockDensity = 0.1f;
    SavannaRules.WaterPresence = 0.1f;
    SavannaRules.bAllowCarnivores = true;
    SavannaRules.bAllowHerbivores = true;
    
    BiomeConfigurations.Add(EBiomeType::Savanna, SavannaConfig);
    BiomeSpawnRules.Add(EBiomeType::Savanna, SavannaRules);
}

void UEng_BiomeArchitecture::InitializeSwampBiome()
{
    FEng_BiomeConfiguration SwampConfig;
    SwampConfig.BiomeType = EBiomeType::Swamp;
    SwampConfig.CenterLocation = FVector(-50000.0f, -45000.0f, 100.0f);
    SwampConfig.Radius = 15000.0f;
    SwampConfig.TemperatureRange = 28.0f;
    SwampConfig.HumidityLevel = 0.9f;
    SwampConfig.MaxActors = 1600;
    SwampConfig.MaxDinosaurs = 30;
    
    FEng_BiomeSpawnRules SwampRules;
    SwampRules.VegetationDensity = 0.6f;
    SwampRules.RockDensity = 0.05f;
    SwampRules.WaterPresence = 0.8f;
    SwampRules.bAllowCarnivores = true;
    SwampRules.bAllowHerbivores = false; // Mostly carnivorous environment
    
    BiomeConfigurations.Add(EBiomeType::Swamp, SwampConfig);
    BiomeSpawnRules.Add(EBiomeType::Swamp, SwampRules);
}

void UEng_BiomeArchitecture::InitializeForestBiome()
{
    FEng_BiomeConfiguration ForestConfig;
    ForestConfig.BiomeType = EBiomeType::Forest;
    ForestConfig.CenterLocation = FVector(-45000.0f, 40000.0f, 100.0f);
    ForestConfig.Radius = 15000.0f;
    ForestConfig.TemperatureRange = 22.0f;
    ForestConfig.HumidityLevel = 0.7f;
    ForestConfig.MaxActors = 1600;
    ForestConfig.MaxDinosaurs = 30;
    
    FEng_BiomeSpawnRules ForestRules;
    ForestRules.VegetationDensity = 0.8f;
    ForestRules.RockDensity = 0.15f;
    ForestRules.WaterPresence = 0.3f;
    ForestRules.bAllowCarnivores = true;
    ForestRules.bAllowHerbivores = true;
    
    BiomeConfigurations.Add(EBiomeType::Forest, ForestConfig);
    BiomeSpawnRules.Add(EBiomeType::Forest, ForestRules);
}

void UEng_BiomeArchitecture::InitializeDesertBiome()
{
    FEng_BiomeConfiguration DesertConfig;
    DesertConfig.BiomeType = EBiomeType::Desert;
    DesertConfig.CenterLocation = FVector(55000.0f, 0.0f, 100.0f);
    DesertConfig.Radius = 15000.0f;
    DesertConfig.TemperatureRange = 35.0f;
    DesertConfig.HumidityLevel = 0.1f;
    DesertConfig.MaxActors = 1600;
    DesertConfig.MaxDinosaurs = 30;
    
    FEng_BiomeSpawnRules DesertRules;
    DesertRules.VegetationDensity = 0.05f;
    DesertRules.RockDensity = 0.3f;
    DesertRules.WaterPresence = 0.0f;
    DesertRules.bAllowCarnivores = true;
    DesertRules.bAllowHerbivores = false; // Harsh environment
    
    BiomeConfigurations.Add(EBiomeType::Desert, DesertConfig);
    BiomeSpawnRules.Add(EBiomeType::Desert, DesertRules);
}

void UEng_BiomeArchitecture::InitializeMountainBiome()
{
    FEng_BiomeConfiguration MountainConfig;
    MountainConfig.BiomeType = EBiomeType::Mountain;
    MountainConfig.CenterLocation = FVector(40000.0f, 50000.0f, 100.0f);
    MountainConfig.Radius = 15000.0f;
    MountainConfig.TemperatureRange = 15.0f;
    MountainConfig.HumidityLevel = 0.4f;
    MountainConfig.MaxActors = 1600;
    MountainConfig.MaxDinosaurs = 30;
    
    FEng_BiomeSpawnRules MountainRules;
    MountainRules.VegetationDensity = 0.1f;
    MountainRules.RockDensity = 0.6f;
    MountainRules.WaterPresence = 0.2f;
    MountainRules.bAllowCarnivores = true;
    MountainRules.bAllowHerbivores = true;
    
    BiomeConfigurations.Add(EBiomeType::Mountain, MountainConfig);
    BiomeSpawnRules.Add(EBiomeType::Mountain, MountainRules);
}

float UEng_BiomeArchitecture::CalculateDistanceToNearestBiomeCenter(const FVector& Location) const
{
    float MinDistance = FLT_MAX;
    
    for (const auto& BiomePair : BiomeConfigurations)
    {
        float Distance = FVector::Dist(Location, BiomePair.Value.CenterLocation);
        MinDistance = FMath::Min(MinDistance, Distance);
    }
    
    return MinDistance;
}

EBiomeType UEng_BiomeArchitecture::DetermineBiomeFromLocation(const FVector& Location) const
{
    EBiomeType ClosestBiome = EBiomeType::Savanna;
    float MinDistance = FLT_MAX;
    
    for (const auto& BiomePair : BiomeConfigurations)
    {
        const FEng_BiomeConfiguration& Config = BiomePair.Value;
        float Distance = FVector::Dist(Location, Config.CenterLocation);
        
        if (Distance <= Config.Radius && Distance < MinDistance)
        {
            MinDistance = Distance;
            ClosestBiome = Config.BiomeType;
        }
    }
    
    return ClosestBiome;
}