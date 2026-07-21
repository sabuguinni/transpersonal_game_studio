#include "EngArch_BiomeSystemManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"

UEngArch_BiomeSystemManager::UEngArch_BiomeSystemManager()
{
    bDebugMode = false;
    DefaultBiomeRadius = 20000.0f;
}

void UEngArch_BiomeSystemManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("EngArch_BiomeSystemManager: Initializing biome system"));
    
    SetupDefaultBiomeConfigurations();
    CreateBiomeZones();
    
    UE_LOG(LogTemp, Warning, TEXT("EngArch_BiomeSystemManager: Biome system initialized with %d biome types"), BiomeConfigurations.Num());
}

void UEngArch_BiomeSystemManager::Deinitialize()
{
    RegisteredBiomeZones.Empty();
    BiomeConfigurations.Empty();
    BiomeTransitions.Empty();
    
    Super::Deinitialize();
}

void UEngArch_BiomeSystemManager::InitializeBiomeSystem()
{
    UE_LOG(LogTemp, Warning, TEXT("EngArch_BiomeSystemManager: Manual biome system initialization"));
    
    SetupDefaultBiomeConfigurations();
    CreateBiomeZones();
}

EEng_BiomeType UEngArch_BiomeSystemManager::GetBiomeAtLocation(const FVector& WorldLocation) const
{
    float MinDistance = FLT_MAX;
    EEng_BiomeType ClosestBiome = EEng_BiomeType::Savanna;
    
    for (const FEng_BiomeZone& Zone : RegisteredBiomeZones)
    {
        float Distance = FVector::Dist(WorldLocation, Zone.Center);
        if (Distance <= Zone.Radius && Distance < MinDistance)
        {
            MinDistance = Distance;
            ClosestBiome = Zone.BiomeType;
        }
    }
    
    return ClosestBiome;
}

FEng_BiomeConfiguration UEngArch_BiomeSystemManager::GetBiomeConfiguration(EEng_BiomeType BiomeType) const
{
    if (const FEng_BiomeConfiguration* Config = BiomeConfigurations.Find(BiomeType))
    {
        return *Config;
    }
    
    // Return default savanna configuration if not found
    FEng_BiomeConfiguration DefaultConfig;
    DefaultConfig.BiomeType = EEng_BiomeType::Savanna;
    return DefaultConfig;
}

float UEngArch_BiomeSystemManager::GetTemperatureAtLocation(const FVector& WorldLocation) const
{
    EEng_BiomeType BiomeType = GetBiomeAtLocation(WorldLocation);
    FEng_BiomeConfiguration Config = GetBiomeConfiguration(BiomeType);
    return Config.Temperature;
}

float UEngArch_BiomeSystemManager::GetHumidityAtLocation(const FVector& WorldLocation) const
{
    EEng_BiomeType BiomeType = GetBiomeAtLocation(WorldLocation);
    FEng_BiomeConfiguration Config = GetBiomeConfiguration(BiomeType);
    return Config.Humidity;
}

TArray<FString> UEngArch_BiomeSystemManager::GetAllowedDinosaursAtLocation(const FVector& WorldLocation) const
{
    EEng_BiomeType BiomeType = GetBiomeAtLocation(WorldLocation);
    FEng_BiomeConfiguration Config = GetBiomeConfiguration(BiomeType);
    return Config.AllowedDinosaurSpecies;
}

void UEngArch_BiomeSystemManager::RegisterBiomeZone(EEng_BiomeType BiomeType, const FVector& Center, float Radius)
{
    FEng_BiomeZone NewZone;
    NewZone.BiomeType = BiomeType;
    NewZone.Center = Center;
    NewZone.Radius = Radius;
    
    RegisteredBiomeZones.Add(NewZone);
    
    UE_LOG(LogTemp, Warning, TEXT("EngArch_BiomeSystemManager: Registered biome zone %d at location (%f, %f, %f) with radius %f"), 
           (int32)BiomeType, Center.X, Center.Y, Center.Z, Radius);
}

bool UEngArch_BiomeSystemManager::IsLocationInBiome(const FVector& WorldLocation, EEng_BiomeType BiomeType) const
{
    for (const FEng_BiomeZone& Zone : RegisteredBiomeZones)
    {
        if (Zone.BiomeType == BiomeType)
        {
            float Distance = FVector::Dist(WorldLocation, Zone.Center);
            if (Distance <= Zone.Radius)
            {
                return true;
            }
        }
    }
    
    return false;
}

FEng_BiomeTransition UEngArch_BiomeSystemManager::CalculateBiomeTransition(const FVector& WorldLocation) const
{
    FEng_BiomeTransition Transition;
    
    // Find the two closest biomes
    float FirstDistance = FLT_MAX;
    float SecondDistance = FLT_MAX;
    EEng_BiomeType FirstBiome = EEng_BiomeType::Savanna;
    EEng_BiomeType SecondBiome = EEng_BiomeType::Forest;
    
    for (const FEng_BiomeZone& Zone : RegisteredBiomeZones)
    {
        float Distance = FVector::Dist(WorldLocation, Zone.Center);
        
        if (Distance < FirstDistance)
        {
            SecondDistance = FirstDistance;
            SecondBiome = FirstBiome;
            FirstDistance = Distance;
            FirstBiome = Zone.BiomeType;
        }
        else if (Distance < SecondDistance)
        {
            SecondDistance = Distance;
            SecondBiome = Zone.BiomeType;
        }
    }
    
    Transition.FromBiome = FirstBiome;
    Transition.ToBiome = SecondBiome;
    Transition.TransitionDistance = FMath::Abs(SecondDistance - FirstDistance);
    Transition.BlendFactor = FMath::Clamp(FirstDistance / (FirstDistance + SecondDistance), 0.0f, 1.0f);
    
    return Transition;
}

void UEngArch_BiomeSystemManager::ValidateBiomeConfiguration()
{
    UE_LOG(LogTemp, Warning, TEXT("EngArch_BiomeSystemManager: Validating biome configuration"));
    
    int32 ValidBiomes = 0;
    int32 ValidZones = 0;
    
    for (const auto& BiomePair : BiomeConfigurations)
    {
        const FEng_BiomeConfiguration& Config = BiomePair.Value;
        if (Config.AllowedDinosaurSpecies.Num() > 0 && Config.VegetationTypes.Num() > 0)
        {
            ValidBiomes++;
        }
    }
    
    for (const FEng_BiomeZone& Zone : RegisteredBiomeZones)
    {
        if (Zone.Radius > 0.0f)
        {
            ValidZones++;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("EngArch_BiomeSystemManager: Validation complete - %d valid biomes, %d valid zones"), ValidBiomes, ValidZones);
}

void UEngArch_BiomeSystemManager::SetupDefaultBiomeConfigurations()
{
    // Savanna Configuration
    FEng_BiomeConfiguration SavannaConfig;
    SavannaConfig.BiomeType = EEng_BiomeType::Savanna;
    SavannaConfig.Temperature = 28.0f;
    SavannaConfig.Humidity = 0.3f;
    SavannaConfig.WindStrength = 0.4f;
    SavannaConfig.AllowedDinosaurSpecies = {"TRex", "Velociraptor", "Triceratops", "Ankylosaurus"};
    SavannaConfig.VegetationTypes = {"Grass", "Acacia_Trees", "Scattered_Bushes"};
    SavannaConfig.ResourceDensity = 0.8f;
    BiomeConfigurations.Add(EEng_BiomeType::Savanna, SavannaConfig);
    
    // Forest Configuration
    FEng_BiomeConfiguration ForestConfig;
    ForestConfig.BiomeType = EEng_BiomeType::Forest;
    ForestConfig.Temperature = 22.0f;
    ForestConfig.Humidity = 0.8f;
    ForestConfig.WindStrength = 0.2f;
    ForestConfig.AllowedDinosaurSpecies = {"Brachiosaurus", "Parasaurolophus", "Protoceratops", "Pachycephalo"};
    ForestConfig.VegetationTypes = {"Dense_Trees", "Ferns", "Moss", "Vines"};
    ForestConfig.ResourceDensity = 1.2f;
    BiomeConfigurations.Add(EEng_BiomeType::Forest, ForestConfig);
    
    // Desert Configuration
    FEng_BiomeConfiguration DesertConfig;
    DesertConfig.BiomeType = EEng_BiomeType::Desert;
    DesertConfig.Temperature = 35.0f;
    DesertConfig.Humidity = 0.1f;
    DesertConfig.WindStrength = 0.6f;
    DesertConfig.AllowedDinosaurSpecies = {"Tsintaosaurus", "Protoceratops"};
    DesertConfig.VegetationTypes = {"Cacti", "Desert_Shrubs", "Sand_Grass"};
    DesertConfig.ResourceDensity = 0.4f;
    BiomeConfigurations.Add(EEng_BiomeType::Desert, DesertConfig);
    
    // Mountain Configuration
    FEng_BiomeConfiguration MountainConfig;
    MountainConfig.BiomeType = EEng_BiomeType::Mountain;
    MountainConfig.Temperature = 15.0f;
    MountainConfig.Humidity = 0.6f;
    MountainConfig.WindStrength = 0.8f;
    MountainConfig.AllowedDinosaurSpecies = {"Ankylosaurus", "Triceratops"};
    MountainConfig.VegetationTypes = {"Pine_Trees", "Mountain_Grass", "Rocky_Vegetation"};
    MountainConfig.ResourceDensity = 0.6f;
    BiomeConfigurations.Add(EEng_BiomeType::Mountain, MountainConfig);
    
    UE_LOG(LogTemp, Warning, TEXT("EngArch_BiomeSystemManager: Default biome configurations created"));
}

void UEngArch_BiomeSystemManager::CreateBiomeZones()
{
    // Clear existing zones
    RegisteredBiomeZones.Empty();
    
    // Register biome zones based on world coordinates from memory
    RegisterBiomeZone(EEng_BiomeType::Savanna, FVector(0.0f, 0.0f, 0.0f), 25000.0f);
    RegisterBiomeZone(EEng_BiomeType::Forest, FVector(-45000.0f, 40000.0f, 0.0f), 20000.0f);
    RegisterBiomeZone(EEng_BiomeType::Desert, FVector(50000.0f, -40000.0f, 0.0f), 18000.0f);
    RegisterBiomeZone(EEng_BiomeType::Mountain, FVector(-30000.0f, -30000.0f, 2000.0f), 15000.0f);
    
    UE_LOG(LogTemp, Warning, TEXT("EngArch_BiomeSystemManager: Created %d biome zones"), RegisteredBiomeZones.Num());
}

float UEngArch_BiomeSystemManager::CalculateDistanceToNearestBiome(const FVector& Location, EEng_BiomeType BiomeType) const
{
    float MinDistance = FLT_MAX;
    
    for (const FEng_BiomeZone& Zone : RegisteredBiomeZones)
    {
        if (Zone.BiomeType == BiomeType)
        {
            float Distance = FVector::Dist(Location, Zone.Center);
            MinDistance = FMath::Min(MinDistance, Distance);
        }
    }
    
    return MinDistance;
}