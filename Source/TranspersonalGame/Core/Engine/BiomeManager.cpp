#include "BiomeManager.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "UObject/ConstructorHelpers.h"

ABiomeManager::ABiomeManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Initialize components
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    
    BiomeVisualizationMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BiomeVisualizationMesh"));
    BiomeVisualizationMesh->SetupAttachment(RootComponent);
    
    BiomeBounds = CreateDefaultSubobject<UBoxComponent>(TEXT("BiomeBounds"));
    BiomeBounds->SetupAttachment(RootComponent);
    
    // Set default values
    CurrentBiomeType = EEng_BiomeType::Forest;
    BiomeRadius = 5000.0f;
    TemperatureBase = 25.0f;
    HumidityLevel = 0.6f;
    FoodAvailability = 0.7f;
    WaterAvailability = 0.8f;
    DangerLevel = 0.3f;
    MaxDinosaurPopulation = 20;
    VegetationDensity = 0.8f;
    
    // Internal state
    LastUpdateTime = 0.0f;
    bBiomeInitialized = false;
    
    // Configure bounds
    BiomeBounds->SetBoxExtent(FVector(BiomeRadius, BiomeRadius, 1000.0f));
}

void ABiomeManager::BeginPlay()
{
    Super::BeginPlay();
    
    if (!bBiomeInitialized)
    {
        InitializeBiome(CurrentBiomeType, GetActorLocation(), BiomeRadius);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("BiomeManager initialized: %s biome at %s"), 
           *UEnum::GetValueAsString(CurrentBiomeType), 
           *GetActorLocation().ToString());
}

void ABiomeManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    LastUpdateTime += DeltaTime;
    
    // Update ecosystem every 5 seconds
    if (LastUpdateTime >= 5.0f)
    {
        UpdateEcosystemBalance();
        ValidateBiomeIntegrity();
        LastUpdateTime = 0.0f;
    }
}

EEng_BiomeType ABiomeManager::GetBiomeTypeAtLocation(FVector WorldLocation)
{
    if (IsLocationInBiome(WorldLocation))
    {
        return CurrentBiomeType;
    }
    
    // Default fallback
    return EEng_BiomeType::Grassland;
}

float ABiomeManager::GetTemperatureAtLocation(FVector WorldLocation)
{
    if (!IsLocationInBiome(WorldLocation))
    {
        return 20.0f; // Default temperature
    }
    
    // Add some variation based on location within biome
    float DistanceFromCenter = FVector::Dist(WorldLocation, GetActorLocation());
    float TemperatureVariation = FMath::Sin(DistanceFromCenter * 0.001f) * 5.0f;
    
    return TemperatureBase + TemperatureVariation;
}

float ABiomeManager::GetHumidityAtLocation(FVector WorldLocation)
{
    if (!IsLocationInBiome(WorldLocation))
    {
        return 0.5f; // Default humidity
    }
    
    return HumidityLevel;
}

bool ABiomeManager::IsLocationInBiome(FVector WorldLocation)
{
    float Distance = FVector::Dist(WorldLocation, GetActorLocation());
    return Distance <= BiomeRadius;
}

void ABiomeManager::InitializeBiome(EEng_BiomeType BiomeType, FVector CenterLocation, float Radius)
{
    CurrentBiomeType = BiomeType;
    BiomeRadius = Radius;
    SetActorLocation(CenterLocation);
    
    // Configure biome parameters based on type
    switch (BiomeType)
    {
        case EEng_BiomeType::Forest:
            TemperatureBase = 22.0f;
            HumidityLevel = 0.8f;
            FoodAvailability = 0.9f;
            WaterAvailability = 0.8f;
            DangerLevel = 0.4f;
            VegetationDensity = 0.9f;
            break;
            
        case EEng_BiomeType::Desert:
            TemperatureBase = 35.0f;
            HumidityLevel = 0.2f;
            FoodAvailability = 0.3f;
            WaterAvailability = 0.2f;
            DangerLevel = 0.7f;
            VegetationDensity = 0.1f;
            break;
            
        case EEng_BiomeType::Swamp:
            TemperatureBase = 28.0f;
            HumidityLevel = 0.95f;
            FoodAvailability = 0.7f;
            WaterAvailability = 0.95f;
            DangerLevel = 0.8f;
            VegetationDensity = 0.8f;
            break;
            
        case EEng_BiomeType::Mountain:
            TemperatureBase = 15.0f;
            HumidityLevel = 0.4f;
            FoodAvailability = 0.4f;
            WaterAvailability = 0.6f;
            DangerLevel = 0.6f;
            VegetationDensity = 0.3f;
            break;
            
        default: // Grassland
            TemperatureBase = 25.0f;
            HumidityLevel = 0.6f;
            FoodAvailability = 0.8f;
            WaterAvailability = 0.7f;
            DangerLevel = 0.3f;
            VegetationDensity = 0.6f;
            break;
    }
    
    // Update bounds
    BiomeBounds->SetBoxExtent(FVector(BiomeRadius, BiomeRadius, 1000.0f));
    
    bBiomeInitialized = true;
    
    UE_LOG(LogTemp, Warning, TEXT("Biome initialized: %s at %s with radius %.0f"), 
           *UEnum::GetValueAsString(CurrentBiomeType), 
           *CenterLocation.ToString(), 
           Radius);
}

void ABiomeManager::UpdateBiomeParameters(float Temperature, float Humidity, float Food, float Water)
{
    TemperatureBase = Temperature;
    HumidityLevel = FMath::Clamp(Humidity, 0.0f, 1.0f);
    FoodAvailability = FMath::Clamp(Food, 0.0f, 1.0f);
    WaterAvailability = FMath::Clamp(Water, 0.0f, 1.0f);
    
    UE_LOG(LogTemp, Log, TEXT("Biome parameters updated: T=%.1f, H=%.2f, F=%.2f, W=%.2f"), 
           Temperature, Humidity, Food, Water);
}

void ABiomeManager::RegisterDinosaurInBiome(ADinosaurBase* Dinosaur)
{
    if (Dinosaur && !RegisteredDinosaurs.Contains(Dinosaur))
    {
        RegisteredDinosaurs.Add(Dinosaur);
        UE_LOG(LogTemp, Log, TEXT("Dinosaur registered in biome: %s (Total: %d)"), 
               *Dinosaur->GetName(), RegisteredDinosaurs.Num());
    }
}

void ABiomeManager::UnregisterDinosaurFromBiome(ADinosaurBase* Dinosaur)
{
    if (Dinosaur)
    {
        RegisteredDinosaurs.Remove(Dinosaur);
        UE_LOG(LogTemp, Log, TEXT("Dinosaur unregistered from biome: %s (Total: %d)"), 
               *Dinosaur->GetName(), RegisteredDinosaurs.Num());
    }
}

bool ABiomeManager::CanSupportDinosaurSpecies(EEng_DinosaurSpecies Species)
{
    // Check if species is native to this biome
    if (NativeDinosaurSpecies.Contains(Species))
    {
        return GetCurrentDinosaurCount() < MaxDinosaurPopulation;
    }
    
    // Non-native species have reduced capacity
    return GetCurrentDinosaurCount() < (MaxDinosaurPopulation / 2);
}

int32 ABiomeManager::GetCurrentDinosaurCount()
{
    // Clean up null references
    RegisteredDinosaurs.RemoveAll([](ADinosaurBase* Dino) { return !IsValid(Dino); });
    return RegisteredDinosaurs.Num();
}

float ABiomeManager::GetEcosystemHealthScore()
{
    float HealthScore = 1.0f;
    
    // Factor in population pressure
    float PopulationRatio = (float)GetCurrentDinosaurCount() / (float)MaxDinosaurPopulation;
    if (PopulationRatio > 1.0f)
    {
        HealthScore *= (1.0f / PopulationRatio); // Overpopulation penalty
    }
    
    // Factor in resource availability
    float ResourceScore = (FoodAvailability + WaterAvailability) * 0.5f;
    HealthScore *= ResourceScore;
    
    // Factor in environmental stress
    float StressScore = 1.0f - (DangerLevel * 0.5f);
    HealthScore *= StressScore;
    
    return FMath::Clamp(HealthScore, 0.0f, 1.0f);
}

void ABiomeManager::UpdateEcosystemBalance()
{
    float EcosystemHealth = GetEcosystemHealthScore();
    
    // Adjust resource availability based on ecosystem health
    if (EcosystemHealth < 0.5f)
    {
        FoodAvailability *= 0.95f; // Gradual resource depletion
        WaterAvailability *= 0.98f;
    }
    else if (EcosystemHealth > 0.8f)
    {
        FoodAvailability = FMath::Min(1.0f, FoodAvailability * 1.02f); // Gradual recovery
        WaterAvailability = FMath::Min(1.0f, WaterAvailability * 1.01f);
    }
}

void ABiomeManager::ValidateBiomeIntegrity()
{
    if (!bBiomeInitialized)
    {
        UE_LOG(LogTemp, Error, TEXT("BiomeManager validation failed: Not initialized"));
        return;
    }
    
    int32 DinosaurCount = GetCurrentDinosaurCount();
    float HealthScore = GetEcosystemHealthScore();
    
    if (HealthScore < 0.3f)
    {
        UE_LOG(LogTemp, Warning, TEXT("Biome ecosystem critical: Health=%.2f, Dinosaurs=%d"), 
               HealthScore, DinosaurCount);
    }
}

FVector ABiomeManager::GetBiomeCenterLocation()
{
    return GetActorLocation();
}