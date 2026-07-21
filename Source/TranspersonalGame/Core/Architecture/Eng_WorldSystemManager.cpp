#include "Eng_WorldSystemManager.h"
#include "Eng_BiomeSystemManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

void UEng_WorldSystemManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    bWorldGenerated = false;
    CurrentWorldSize = 8000.0f;
    
    // Set default world parameters
    WorldParams.WorldSize = 8000.0f;
    WorldParams.BiomeCount = 6;
    WorldParams.RiverDensity = 0.3f;
    WorldParams.MountainHeight = 2000.0f;
    WorldParams.ForestCoverage = 0.4f;
    WorldParams.WaterCoverage = 0.15f;
    WorldParams.DesertCoverage = 0.1f;
    WorldParams.Seed = FMath::RandRange(1, 999999);
    
    CreateSubsystems();
    
    UE_LOG(LogTemp, Warning, TEXT("WorldSystemManager initialized with seed %d"), WorldParams.Seed);
}

void UEng_WorldSystemManager::Deinitialize()
{
    BiomeManager = nullptr;
    WeatherManager = nullptr;
    TimeManager = nullptr;
    
    Super::Deinitialize();
}

void UEng_WorldSystemManager::InitializeWorldSystems()
{
    if (!BiomeManager)
    {
        UE_LOG(LogTemp, Error, TEXT("BiomeManager not available for world initialization"));
        return;
    }
    
    // Initialize biome system first
    BiomeManager->InitializeBiomes();
    
    // TODO: Initialize weather and time systems when implemented
    
    bWorldGenerated = true;
    UE_LOG(LogTemp, Warning, TEXT("World systems initialized successfully"));
}

void UEng_WorldSystemManager::RegenerateWorld(float WorldSize)
{
    CurrentWorldSize = WorldSize;
    WorldParams.WorldSize = WorldSize;
    WorldParams.Seed = FMath::RandRange(1, 999999);
    
    bWorldGenerated = false;
    
    InitializeWorldSystems();
    
    UE_LOG(LogTemp, Warning, TEXT("World regenerated with size %.1f and new seed %d"), WorldSize, WorldParams.Seed);
}

void UEng_WorldSystemManager::SetWorldParameters(const FEng_WorldGenerationParams& Params)
{
    WorldParams = Params;
    CurrentWorldSize = Params.WorldSize;
    
    // Apply parameters to subsystems
    if (BiomeManager)
    {
        BiomeManager->ApplyWorldParameters(Params);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("World parameters updated"));
}

void UEng_WorldSystemManager::CreateSubsystems()
{
    // Get biome manager from game instance subsystems
    BiomeManager = GetGameInstance()->GetSubsystem<UEng_BiomeSystemManager>();
    
    // TODO: Create weather and time managers when implemented
    WeatherManager = nullptr;
    TimeManager = nullptr;
    
    if (!BiomeManager)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to get BiomeSystemManager subsystem"));
    }
}

void UEng_WorldSystemManager::ValidateWorldIntegrity()
{
    // Validate all systems are properly initialized
    bool bValid = true;
    
    if (!BiomeManager)
    {
        UE_LOG(LogTemp, Error, TEXT("BiomeManager validation failed"));
        bValid = false;
    }
    
    if (CurrentWorldSize <= 0.0f)
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid world size: %.1f"), CurrentWorldSize);
        bValid = false;
    }
    
    if (bValid)
    {
        UE_LOG(LogTemp, Warning, TEXT("World integrity validation passed"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("World integrity validation failed"));
        bWorldGenerated = false;
    }
}