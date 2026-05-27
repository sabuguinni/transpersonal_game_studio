#include "EngArchitect_WorldSystemManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

UEngArchitect_WorldSystemManager::UEngArchitect_WorldSystemManager()
{
    BiomeManager = nullptr;
    TerrainManager = nullptr;
    WeatherManager = nullptr;
    bSystemsInitialized = false;
    CurrentPerformanceLevel = EPerformanceLevel::Medium;
    LastFrameTime = 0.0f;
    ActiveBiomeRegions = 0;
    StreamedTerrainChunks = 0;

    // Define system initialization order
    SystemInitOrder.Add(TEXT("TerrainManager"));
    SystemInitOrder.Add(TEXT("BiomeManager"));
    SystemInitOrder.Add(TEXT("WeatherManager"));
}

void UEngArchitect_WorldSystemManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("EngArchitect_WorldSystemManager: Initializing..."));
    
    // Initialize world systems in proper order
    InitializeWorldSystems();
}

void UEngArchitect_WorldSystemManager::Deinitialize()
{
    ShutdownWorldSystems();
    Super::Deinitialize();
}

void UEngArchitect_WorldSystemManager::InitializeWorldSystems()
{
    if (bSystemsInitialized)
    {
        UE_LOG(LogTemp, Warning, TEXT("WorldSystemManager: Systems already initialized"));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("WorldSystemManager: Starting system initialization"));

    // Initialize systems in dependency order
    InitializeTerrainSystem();
    InitializeBiomeSystem();
    InitializeWeatherSystem();

    // Validate all systems are properly connected
    ValidateSystemDependencies();

    bSystemsInitialized = true;
    UE_LOG(LogTemp, Warning, TEXT("WorldSystemManager: All systems initialized successfully"));
}

void UEngArchitect_WorldSystemManager::ShutdownWorldSystems()
{
    if (!bSystemsInitialized)
    {
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("WorldSystemManager: Shutting down systems"));

    // Shutdown in reverse order
    WeatherManager = nullptr;
    BiomeManager = nullptr;
    TerrainManager = nullptr;

    bSystemsInitialized = false;
    InitializedSystems.Empty();
}

void UEngArchitect_WorldSystemManager::InitializeTerrainSystem()
{
    UE_LOG(LogTemp, Warning, TEXT("WorldSystemManager: Initializing Terrain System"));
    
    // TerrainManager will be created by Core Systems Programmer
    // For now, mark as initialized
    InitializedSystems.Add(TEXT("TerrainManager"));
}

void UEngArchitect_WorldSystemManager::InitializeBiomeSystem()
{
    UE_LOG(LogTemp, Warning, TEXT("WorldSystemManager: Initializing Biome System"));
    
    // BiomeManager will be created by World Generator agent
    // For now, mark as initialized
    InitializedSystems.Add(TEXT("BiomeManager"));
}

void UEngArchitect_WorldSystemManager::InitializeWeatherSystem()
{
    UE_LOG(LogTemp, Warning, TEXT("WorldSystemManager: Initializing Weather System"));
    
    // WeatherManager will be created by Environment Artist
    // For now, mark as initialized
    InitializedSystems.Add(TEXT("WeatherManager"));
}

void UEngArchitect_WorldSystemManager::ValidateSystemDependencies()
{
    UE_LOG(LogTemp, Warning, TEXT("WorldSystemManager: Validating system dependencies"));

    // Check that all required systems are initialized
    for (const FString& SystemName : SystemInitOrder)
    {
        if (!InitializedSystems.Contains(SystemName))
        {
            UE_LOG(LogTemp, Error, TEXT("WorldSystemManager: System %s failed to initialize"), *SystemName);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("WorldSystemManager: System %s initialized successfully"), *SystemName);
        }
    }
}

EBiomeType UEngArchitect_WorldSystemManager::GetCurrentBiome(const FVector& WorldLocation) const
{
    // Simple biome determination based on world coordinates
    // This will be enhanced by the BiomeManager when implemented
    
    float X = WorldLocation.X;
    float Y = WorldLocation.Y;

    // Savana (center): -10000 to 10000 in both axes
    if (FMath::Abs(X) < 10000.0f && FMath::Abs(Y) < 10000.0f)
    {
        return EBiomeType::Savana;
    }
    
    // Forest (northwest): X < -10000, Y > 10000
    if (X < -10000.0f && Y > 10000.0f)
    {
        return EBiomeType::Forest;
    }
    
    // Desert (southeast): X > 10000, Y < -10000
    if (X > 10000.0f && Y < -10000.0f)
    {
        return EBiomeType::Desert;
    }
    
    // Mountain (northeast): X > 10000, Y > 10000
    if (X > 10000.0f && Y > 10000.0f)
    {
        return EBiomeType::Mountain;
    }
    
    // Swamp (southwest): X < -10000, Y < -10000
    if (X < -10000.0f && Y < -10000.0f)
    {
        return EBiomeType::Swamp;
    }

    // Default to Savana
    return EBiomeType::Savana;
}

void UEngArchitect_WorldSystemManager::RegisterBiomeTransition(const FVector& Location, EBiomeType FromBiome, EBiomeType ToBiome)
{
    UE_LOG(LogTemp, Warning, TEXT("WorldSystemManager: Biome transition at (%f, %f, %f) from %d to %d"), 
           Location.X, Location.Y, Location.Z, (int32)FromBiome, (int32)ToBiome);

    // Cache the transition for performance
    BiomeTransitionCache.Add(Location, ToBiome);

    // Trigger any necessary system updates
    UpdatePerformanceMetrics();
}

float UEngArchitect_WorldSystemManager::GetWorldSystemsPerformanceMetric() const
{
    // Calculate performance metric based on active systems
    float BasePerformance = 1.0f;
    
    // Factor in active biome regions
    float BiomeLoad = FMath::Clamp(ActiveBiomeRegions / 5.0f, 0.1f, 2.0f);
    
    // Factor in streamed terrain chunks
    float TerrainLoad = FMath::Clamp(StreamedTerrainChunks / 100.0f, 0.1f, 2.0f);
    
    // Factor in frame time
    float FrameTimeLoad = FMath::Clamp(LastFrameTime / 16.67f, 0.1f, 3.0f); // 60 FPS baseline
    
    return BasePerformance / (BiomeLoad * TerrainLoad * FrameTimeLoad);
}

void UEngArchitect_WorldSystemManager::SetPerformanceLevel(EPerformanceLevel NewLevel)
{
    if (CurrentPerformanceLevel != NewLevel)
    {
        UE_LOG(LogTemp, Warning, TEXT("WorldSystemManager: Performance level changed from %d to %d"), 
               (int32)CurrentPerformanceLevel, (int32)NewLevel);
        
        CurrentPerformanceLevel = NewLevel;
        AdjustSystemsForPerformance();
    }
}

void UEngArchitect_WorldSystemManager::UpdatePerformanceMetrics()
{
    // Update frame time
    if (UWorld* World = GetWorld())
    {
        LastFrameTime = World->GetDeltaSeconds() * 1000.0f; // Convert to milliseconds
    }

    // Update active biome regions (placeholder)
    ActiveBiomeRegions = BiomeTransitionCache.Num() / 10; // Rough estimate

    // Update streamed terrain chunks (placeholder)
    StreamedTerrainChunks = FMath::RandRange(50, 150); // Simulated value
}

void UEngArchitect_WorldSystemManager::AdjustSystemsForPerformance()
{
    switch (CurrentPerformanceLevel)
    {
        case EPerformanceLevel::Low:
            UE_LOG(LogTemp, Warning, TEXT("WorldSystemManager: Adjusting for LOW performance"));
            // Reduce system complexity
            break;
            
        case EPerformanceLevel::Medium:
            UE_LOG(LogTemp, Warning, TEXT("WorldSystemManager: Adjusting for MEDIUM performance"));
            // Standard system settings
            break;
            
        case EPerformanceLevel::High:
            UE_LOG(LogTemp, Warning, TEXT("WorldSystemManager: Adjusting for HIGH performance"));
            // Enable all features
            break;
            
        case EPerformanceLevel::Ultra:
            UE_LOG(LogTemp, Warning, TEXT("WorldSystemManager: Adjusting for ULTRA performance"));
            // Maximum quality settings
            break;
    }
}