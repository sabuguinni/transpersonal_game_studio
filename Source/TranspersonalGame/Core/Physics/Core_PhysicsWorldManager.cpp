#include "Core_PhysicsWorldManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"

UCore_PhysicsWorldManager::UCore_PhysicsWorldManager()
{
    // Initialize default physics settings
    GlobalPhysicsSettings.GlobalGravityScale = 1.0f;
    GlobalPhysicsSettings.AirDensity = 1.225f;
    GlobalPhysicsSettings.WaterDensity = 1000.0f;
    GlobalPhysicsSettings.WindStrength = 0.0f;
    GlobalPhysicsSettings.WindDirection = FVector(1.0f, 0.0f, 0.0f);
    GlobalPhysicsSettings.TemperatureKelvin = 288.15f;
    GlobalPhysicsSettings.bEnableAdvancedPhysics = true;
    GlobalPhysicsSettings.bEnableFluidSimulation = false;
    GlobalPhysicsSettings.bEnableWeatherPhysics = false;

    bIsSystemHealthy = true;
    PhysicsUpdateInterval = 0.016f;
    CurrentPhysicsLOD = 0;
    LastPhysicsUpdate = 0.0f;
    bNeedsPhysicsUpdate = false;
}

void UCore_PhysicsWorldManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsWorldManager: Initializing world physics system"));
    
    InitializeWorldPhysics();
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsWorldManager: World physics system initialized successfully"));
}

void UCore_PhysicsWorldManager::Deinitialize()
{
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsWorldManager: Deinitializing world physics system"));
    
    // Clear all physics regions
    PhysicsRegions.Empty();
    
    Super::Deinitialize();
}

bool UCore_PhysicsWorldManager::ShouldCreateSubsystem(UObject* Outer) const
{
    // Only create in game worlds, not in editor preview worlds
    if (UWorld* World = Cast<UWorld>(Outer))
    {
        return World->IsGameWorld();
    }
    return false;
}

void UCore_PhysicsWorldManager::InitializeWorldPhysics()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("Core_PhysicsWorldManager: Failed to get world reference"));
        bIsSystemHealthy = false;
        return;
    }

    // Apply global gravity settings
    UpdateGravitySettings();
    
    // Initialize environmental forces
    UpdateEnvironmentalForces();
    
    // Set up default physics regions if none exist
    if (PhysicsRegions.Num() == 0)
    {
        FCore_PhysicsRegion DefaultRegion;
        DefaultRegion.RegionBounds = FBox(FVector(-50000.0f), FVector(50000.0f));
        DefaultRegion.RegionSettings = GlobalPhysicsSettings;
        DefaultRegion.BiomeType = EBiomeType::Grassland;
        DefaultRegion.bIsActive = true;
        DefaultRegion.Priority = 1.0f;
        
        PhysicsRegions.Add(DefaultRegion);
    }
    
    bIsSystemHealthy = true;
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsWorldManager: World physics initialized with %d regions"), PhysicsRegions.Num());
}

void UCore_PhysicsWorldManager::UpdateWorldPhysics(float DeltaTime)
{
    if (!bIsSystemHealthy)
    {
        return;
    }

    LastPhysicsUpdate += DeltaTime;
    
    // Update physics at specified interval
    if (LastPhysicsUpdate >= PhysicsUpdateInterval)
    {
        ProcessPhysicsRegions();
        UpdateEnvironmentalForces();
        
        LastPhysicsUpdate = 0.0f;
        bNeedsPhysicsUpdate = false;
    }
}

void UCore_PhysicsWorldManager::SetGlobalPhysicsSettings(const FCore_WorldPhysicsSettings& NewSettings)
{
    GlobalPhysicsSettings = NewSettings;
    UpdateGravitySettings();
    UpdateEnvironmentalForces();
    bNeedsPhysicsUpdate = true;
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsWorldManager: Global physics settings updated"));
}

void UCore_PhysicsWorldManager::RegisterPhysicsRegion(const FCore_PhysicsRegion& Region)
{
    PhysicsRegions.Add(Region);
    bNeedsPhysicsUpdate = true;
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsWorldManager: Physics region registered. Total regions: %d"), PhysicsRegions.Num());
}

void UCore_PhysicsWorldManager::UnregisterPhysicsRegion(const FBox& RegionBounds)
{
    PhysicsRegions.RemoveAll([&RegionBounds](const FCore_PhysicsRegion& Region)
    {
        return Region.RegionBounds.Equals(RegionBounds, 1.0f);
    });
    
    bNeedsPhysicsUpdate = true;
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsWorldManager: Physics region unregistered. Total regions: %d"), PhysicsRegions.Num());
}

FCore_WorldPhysicsSettings UCore_PhysicsWorldManager::GetPhysicsSettingsAtLocation(const FVector& WorldLocation) const
{
    FCore_PhysicsRegion* HighestPriorityRegion = FindHighestPriorityRegion(WorldLocation);
    
    if (HighestPriorityRegion && HighestPriorityRegion->bIsActive)
    {
        return HighestPriorityRegion->RegionSettings;
    }
    
    return GlobalPhysicsSettings;
}

void UCore_PhysicsWorldManager::SetWindParameters(float Strength, const FVector& Direction)
{
    GlobalPhysicsSettings.WindStrength = Strength;
    GlobalPhysicsSettings.WindDirection = Direction.GetSafeNormal();
    bNeedsPhysicsUpdate = true;
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsWorldManager: Wind parameters updated - Strength: %f, Direction: %s"), 
           Strength, *Direction.ToString());
}

FVector UCore_PhysicsWorldManager::GetWindForceAtLocation(const FVector& WorldLocation) const
{
    FCore_WorldPhysicsSettings LocalSettings = GetPhysicsSettingsAtLocation(WorldLocation);
    return LocalSettings.WindDirection * LocalSettings.WindStrength;
}

void UCore_PhysicsWorldManager::SetTemperature(float TemperatureInKelvin)
{
    GlobalPhysicsSettings.TemperatureKelvin = FMath::Clamp(TemperatureInKelvin, 173.15f, 373.15f); // -100°C to 100°C
    bNeedsPhysicsUpdate = true;
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsWorldManager: Temperature set to %f K (%.2f°C)"), 
           TemperatureInKelvin, TemperatureInKelvin - 273.15f);
}

float UCore_PhysicsWorldManager::GetTemperatureAtLocation(const FVector& WorldLocation) const
{
    FCore_WorldPhysicsSettings LocalSettings = GetPhysicsSettingsAtLocation(WorldLocation);
    return LocalSettings.TemperatureKelvin;
}

void UCore_PhysicsWorldManager::SetGravityScale(float NewGravityScale)
{
    GlobalPhysicsSettings.GlobalGravityScale = FMath::Clamp(NewGravityScale, 0.1f, 10.0f);
    UpdateGravitySettings();
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsWorldManager: Gravity scale set to %f"), NewGravityScale);
}

FVector UCore_PhysicsWorldManager::GetGravityVector() const
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return FVector(0.0f, 0.0f, -980.0f);
    }
    
    return World->GetGravityZ() * GlobalPhysicsSettings.GlobalGravityScale * FVector::UpVector;
}

void UCore_PhysicsWorldManager::EnableFluidSimulation(bool bEnable)
{
    GlobalPhysicsSettings.bEnableFluidSimulation = bEnable;
    bNeedsPhysicsUpdate = true;
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsWorldManager: Fluid simulation %s"), 
           bEnable ? TEXT("enabled") : TEXT("disabled"));
}

float UCore_PhysicsWorldManager::GetFluidDensityAtLocation(const FVector& WorldLocation) const
{
    FCore_WorldPhysicsSettings LocalSettings = GetPhysicsSettingsAtLocation(WorldLocation);
    
    // Check if location is underwater (simplified check)
    if (IsLocationUnderwater(WorldLocation))
    {
        return LocalSettings.WaterDensity;
    }
    
    return LocalSettings.AirDensity;
}

bool UCore_PhysicsWorldManager::IsLocationUnderwater(const FVector& WorldLocation) const
{
    // Simplified underwater check - in a real implementation, this would check against water volumes
    return WorldLocation.Z < 0.0f; // Assume sea level at Z=0
}

void UCore_PhysicsWorldManager::SetPhysicsLOD(int32 LODLevel)
{
    CurrentPhysicsLOD = FMath::Clamp(LODLevel, 0, 3);
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsWorldManager: Physics LOD set to %d"), CurrentPhysicsLOD);
}

void UCore_PhysicsWorldManager::OptimizePhysicsForPerformance()
{
    // Adjust physics settings based on current performance
    if (CurrentPhysicsLOD > 0)
    {
        PhysicsUpdateInterval = 0.033f; // 30 FPS for lower performance
    }
    else
    {
        PhysicsUpdateInterval = 0.016f; // 60 FPS for high performance
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsWorldManager: Physics optimized for performance level %d"), CurrentPhysicsLOD);
}

void UCore_PhysicsWorldManager::ValidateWorldPhysics()
{
    bIsSystemHealthy = true;
    
    // Validate world reference
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("Core_PhysicsWorldManager: World reference is null"));
        bIsSystemHealthy = false;
        return;
    }
    
    // Validate physics regions
    for (int32 i = 0; i < PhysicsRegions.Num(); i++)
    {
        const FCore_PhysicsRegion& Region = PhysicsRegions[i];
        
        if (!Region.RegionBounds.IsValid)
        {
            UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsWorldManager: Invalid bounds for physics region %d"), i);
        }
        
        if (Region.RegionSettings.GlobalGravityScale <= 0.0f)
        {
            UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsWorldManager: Invalid gravity scale for physics region %d"), i);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsWorldManager: Physics validation completed. System healthy: %s"), 
           bIsSystemHealthy ? TEXT("true") : TEXT("false"));
}

void UCore_PhysicsWorldManager::DebugDrawPhysicsRegions()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    for (const FCore_PhysicsRegion& Region : PhysicsRegions)
    {
        FColor RegionColor = Region.bIsActive ? FColor::Green : FColor::Red;
        DrawDebugBox(World, Region.RegionBounds.GetCenter(), Region.RegionBounds.GetExtent(), 
                     RegionColor, false, 5.0f, 0, 2.0f);
    }
}

void UCore_PhysicsWorldManager::UpdateGravitySettings()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Apply gravity scale to world settings
    if (AWorldSettings* WorldSettings = World->GetWorldSettings())
    {
        WorldSettings->GlobalGravityZ = -980.0f * GlobalPhysicsSettings.GlobalGravityScale;
    }
}

void UCore_PhysicsWorldManager::UpdateEnvironmentalForces()
{
    // Update environmental physics parameters
    // This would integrate with wind, temperature, and other environmental systems
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("Core_PhysicsWorldManager: Environmental forces updated"));
}

void UCore_PhysicsWorldManager::ProcessPhysicsRegions()
{
    // Process active physics regions and apply their effects
    for (FCore_PhysicsRegion& Region : PhysicsRegions)
    {
        if (Region.bIsActive)
        {
            ApplyRegionalPhysics(Region);
        }
    }
}

FCore_PhysicsRegion* UCore_PhysicsWorldManager::FindHighestPriorityRegion(const FVector& WorldLocation) const
{
    FCore_PhysicsRegion* HighestPriorityRegion = nullptr;
    float HighestPriority = -1.0f;
    
    for (int32 i = 0; i < PhysicsRegions.Num(); i++)
    {
        const FCore_PhysicsRegion& Region = PhysicsRegions[i];
        
        if (Region.bIsActive && Region.RegionBounds.IsInside(WorldLocation))
        {
            if (Region.Priority > HighestPriority)
            {
                HighestPriority = Region.Priority;
                HighestPriorityRegion = const_cast<FCore_PhysicsRegion*>(&Region);
            }
        }
    }
    
    return HighestPriorityRegion;
}

void UCore_PhysicsWorldManager::ApplyRegionalPhysics(const FCore_PhysicsRegion& Region)
{
    // Apply region-specific physics settings
    // This would modify physics parameters for objects within the region
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("Core_PhysicsWorldManager: Applied physics for region with biome type %d"), 
           (int32)Region.BiomeType);
}