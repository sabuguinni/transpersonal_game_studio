/**
 * @file PhysicsOptimizer.cpp
 * @brief Implementation of physics performance optimization system
 */

#include "PhysicsOptimizer.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/PrimitiveComponent.h"
#include "HAL/PlatformMemory.h"
#include "Stats/Stats.h"
#include "Misc/DateTime.h"

DECLARE_CYCLE_STAT(TEXT("Physics LOD Update"), STAT_PhysicsLODUpdate, STATGROUP_TranspersonalPhysics);
DECLARE_CYCLE_STAT(TEXT("Spatial Grid Update"), STAT_SpatialGridUpdate, STATGROUP_TranspersonalPhysics);
DECLARE_CYCLE_STAT(TEXT("Consciousness Optimization"), STAT_ConsciousnessOptimization, STATGROUP_TranspersonalPhysics);

UPhysicsOptimizer::UPhysicsOptimizer()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz optimization updates
    
    InitializeLODSettings();
}

void UPhysicsOptimizer::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize spatial partitioning for 50km² world
    InitializeSpatialPartitioning(50000.0f, 50);
    
    // Set initial performance targets
    SetTargetFrameRate(60.0f);
    
    UE_LOG(LogTemp, Log, TEXT("PhysicsOptimizer initialized for 50km² world"));
}

void UPhysicsOptimizer::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update performance metrics
    UpdatePerformanceMetrics();
    
    // Adaptive optimization based on current performance
    if (bAdaptiveOptimization && 
        FPlatformTime::Seconds() - LastOptimizationTime > OptimizationInterval)
    {
        AdaptLODBasedOnPerformance();
        LastOptimizationTime = FPlatformTime::Seconds();
    }
    
    // Optimize physics substep timing
    OptimizePhysicsSubstep(DeltaTime);
}

void UPhysicsOptimizer::UpdatePhysicsLOD(const FVector& PlayerLocation)
{
    SCOPE_CYCLE_COUNTER(STAT_PhysicsLODUpdate);
    
    if (!GetWorld()) return;
    
    // Get all physics objects in world
    for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor || !Actor->GetRootComponent()) continue;
        
        UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
        if (!PrimComp || !PrimComp->IsSimulatingPhysics()) continue;
        
        float Distance = FVector::Dist(PlayerLocation, Actor->GetActorLocation());
        int32 Complexity = PrimComp->GetCollisionShape().GetShapeType(); // Simplified complexity metric
        
        EPhysicsLOD LODLevel = CalculateLODLevel(Distance, Complexity);
        
        // Apply LOD settings
        const FPhysicsLODSettings& Settings = LODSettings[LODLevel];
        
        // Adjust physics simulation frequency
        if (Distance > Settings.MaxDistance)
        {
            PrimComp->SetSimulatePhysics(false);
        }
        else
        {
            PrimComp->SetSimulatePhysics(true);
            
            // Adjust update frequency based on LOD
            float UpdateRate = Settings.UpdateFrequency / 60.0f; // Normalize to 60fps base
            PrimComp->SetComponentTickInterval(1.0f / UpdateRate);
        }
    }
}

EPhysicsLOD UPhysicsOptimizer::CalculateLODLevel(float Distance, int32 ObjectComplexity)
{
    // Distance-based LOD with complexity weighting
    float AdjustedDistance = Distance * (1.0f + ObjectComplexity * 0.1f);
    
    if (AdjustedDistance < 500.0f)
        return EPhysicsLOD::Highest;
    else if (AdjustedDistance < 1500.0f)
        return EPhysicsLOD::High;
    else if (AdjustedDistance < 5000.0f)
        return EPhysicsLOD::Medium;
    else if (AdjustedDistance < 15000.0f)
        return EPhysicsLOD::Low;
    else
        return EPhysicsLOD::Minimal;
}

void UPhysicsOptimizer::SetGlobalLODSettings(const FPhysicsLODSettings& Settings)
{
    // Apply settings to all LOD levels with scaling
    for (auto& LODPair : LODSettings)
    {
        EPhysicsLOD Level = LODPair.Key;
        FPhysicsLODSettings& LevelSettings = LODPair.Value;
        
        float LODScale = 1.0f;
        switch (Level)
        {
            case EPhysicsLOD::Highest: LODScale = 1.0f; break;
            case EPhysicsLOD::High: LODScale = 0.8f; break;
            case EPhysicsLOD::Medium: LODScale = 0.6f; break;
            case EPhysicsLOD::Low: LODScale = 0.4f; break;
            case EPhysicsLOD::Minimal: LODScale = 0.2f; break;
        }
        
        LevelSettings.MaxSimulatedObjects = FMath::RoundToInt(Settings.MaxSimulatedObjects * LODScale);
        LevelSettings.UpdateFrequency = Settings.UpdateFrequency * LODScale;
        LevelSettings.ConsciousnessFieldResolution = Settings.ConsciousnessFieldResolution * LODScale;
    }
}

void UPhysicsOptimizer::InitializeSpatialPartitioning(float WorldSize, int32 GridResolution)
{
    this->GridResolution = GridResolution;
    this->GridCellSize = WorldSize / GridResolution;
    
    // Initialize 2D grid
    SpatialGrid.Empty();
    SpatialGrid.SetNum(GridResolution * GridResolution);
    
    UE_LOG(LogTemp, Log, TEXT("Spatial partitioning initialized: %dx%d grid, cell size: %.2f"), 
           GridResolution, GridResolution, GridCellSize);
}

TArray<AActor*> UPhysicsOptimizer::GetNearbyPhysicsObjects(const FVector& Location, float Radius)
{
    SCOPE_CYCLE_COUNTER(STAT_SpatialGridUpdate);
    
    TArray<AActor*> NearbyActors;
    
    // Calculate grid cells to check
    int32 CellsToCheck = FMath::CeilToInt(Radius / GridCellSize);
    int32 CenterX = FMath::FloorToInt(Location.X / GridCellSize) + GridResolution / 2;
    int32 CenterY = FMath::FloorToInt(Location.Y / GridCellSize) + GridResolution / 2;
    
    for (int32 X = CenterX - CellsToCheck; X <= CenterX + CellsToCheck; X++)
    {
        for (int32 Y = CenterY - CellsToCheck; Y <= CenterY + CellsToCheck; Y++)
        {
            if (X >= 0 && X < GridResolution && Y >= 0 && Y < GridResolution)
            {
                int32 GridIndex = Y * GridResolution + X;
                if (SpatialGrid.IsValidIndex(GridIndex))
                {
                    for (AActor* Actor : SpatialGrid[GridIndex])
                    {
                        if (Actor && FVector::Dist(Location, Actor->GetActorLocation()) <= Radius)
                        {
                            NearbyActors.AddUnique(Actor);
                        }
                    }
                }
            }
        }
    }
    
    return NearbyActors;
}

void UPhysicsOptimizer::UpdateSpatialGrid(AActor* Actor, const FVector& OldLocation, const FVector& NewLocation)
{
    if (!Actor) return;
    
    int32 OldIndex = GetGridIndex(OldLocation);
    int32 NewIndex = GetGridIndex(NewLocation);
    
    if (OldIndex != NewIndex)
    {
        // Remove from old cell
        if (SpatialGrid.IsValidIndex(OldIndex))
        {
            SpatialGrid[OldIndex].Remove(Actor);
        }
        
        // Add to new cell
        if (SpatialGrid.IsValidIndex(NewIndex))
        {
            SpatialGrid[NewIndex].AddUnique(Actor);
        }
    }
}

void UPhysicsOptimizer::OptimizeConsciousnessField(const FVector& FocusPoint, float Intensity)
{
    SCOPE_CYCLE_COUNTER(STAT_ConsciousnessOptimization);
    
    // Get nearby objects that could be affected by consciousness field
    TArray<AActor*> NearbyObjects = GetNearbyPhysicsObjects(FocusPoint, 2000.0f * Intensity);
    
    for (AActor* Actor : NearbyObjects)
    {
        if (!Actor) continue;
        
        float Distance = FVector::Dist(FocusPoint, Actor->GetActorLocation());
        float FieldStrength = FMath::Clamp(Intensity / (Distance * 0.001f + 1.0f), 0.0f, 1.0f);
        
        // Adjust physics properties based on consciousness field strength
        if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent()))
        {
            if (PrimComp->IsSimulatingPhysics())
            {
                // Increase simulation quality for objects in strong consciousness fields
                float QualityMultiplier = 1.0f + FieldStrength * 2.0f;
                PrimComp->SetComponentTickInterval(1.0f / (60.0f * QualityMultiplier));
            }
        }
    }
}

void UPhysicsOptimizer::AdaptiveConsciousnessResolution(float TargetFrameTime)
{
    float CurrentFrameTime = CurrentMetrics.PhysicsFrameTime;
    
    if (CurrentFrameTime > TargetFrameTime * 1.2f)
    {
        // Reduce consciousness field resolution
        for (auto& LODPair : LODSettings)
        {
            LODPair.Value.ConsciousnessFieldResolution *= 0.9f;
            LODPair.Value.ConsciousnessFieldResolution = FMath::Max(LODPair.Value.ConsciousnessFieldResolution, 0.1f);
        }
    }
    else if (CurrentFrameTime < TargetFrameTime * 0.8f)
    {
        // Increase consciousness field resolution
        for (auto& LODPair : LODSettings)
        {
            LODPair.Value.ConsciousnessFieldResolution *= 1.1f;
            LODPair.Value.ConsciousnessFieldResolution = FMath::Min(LODPair.Value.ConsciousnessFieldResolution, 2.0f);
        }
    }
}

void UPhysicsOptimizer::CullDistantConsciousnessEffects(const FVector& PlayerLocation)
{
    // Disable consciousness physics for very distant objects
    float CullDistance = 10000.0f; // 10km
    
    if (!GetWorld()) return;
    
    for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor) continue;
        
        float Distance = FVector::Dist(PlayerLocation, Actor->GetActorLocation());
        
        // Disable consciousness effects for distant objects
        if (Distance > CullDistance)
        {
            // TODO: Disable consciousness component on this actor
            // This would connect to the ConsciousnessPhysics system
        }
    }
}

FPerformanceMetrics UPhysicsOptimizer::GetCurrentMetrics() const
{
    return CurrentMetrics;
}

void UPhysicsOptimizer::SetTargetFrameRate(float TargetFPS)
{
    TargetFrameRate = FMath::Clamp(TargetFPS, 30.0f, 120.0f);
    MaxPhysicsFrameTime = 1000.0f / TargetFPS; // Convert to milliseconds
}

bool UPhysicsOptimizer::IsPerformanceAcceptable() const
{
    return CurrentMetrics.PhysicsFrameTime <= MaxPhysicsFrameTime &&
           CurrentMetrics.CPUUsagePercent < 80.0f &&
           CurrentMetrics.MemoryUsageMB < 4096.0f; // 4GB limit
}

void UPhysicsOptimizer::CleanupUnusedPhysicsData()
{
    // Force garbage collection of unused physics assets
    if (GEngine)
    {
        GEngine->ForceGarbageCollection(true);
    }
    
    // Clear empty spatial grid cells
    for (auto& GridCell : SpatialGrid)
    {
        GridCell.RemoveAll([](AActor* Actor) { return !IsValid(Actor); });
    }
}

void UPhysicsOptimizer::PreloadCriticalPhysicsAssets(const TArray<FString>& AssetPaths)
{
    // Preload physics assets that are likely to be needed soon
    for (const FString& AssetPath : AssetPaths)
    {
        // TODO: Implement asset preloading
        UE_LOG(LogTemp, Log, TEXT("Preloading physics asset: %s"), *AssetPath);
    }
}

void UPhysicsOptimizer::UnloadDistantPhysicsAssets(const FVector& PlayerLocation, float UnloadDistance)
{
    // Unload physics assets that are too far from player
    if (!GetWorld()) return;
    
    for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor) continue;
        
        float Distance = FVector::Dist(PlayerLocation, Actor->GetActorLocation());
        
        if (Distance > UnloadDistance)
        {
            if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent()))
            {
                // Reduce physics complexity for distant objects
                PrimComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
                PrimComp->SetSimulatePhysics(false);
            }
        }
    }
}

void UPhysicsOptimizer::UpdatePerformanceMetrics()
{
    // Update physics frame time
    CurrentMetrics.PhysicsFrameTime = FPlatformTime::ToMilliseconds(GFrameTime);
    
    // Count active physics objects
    CurrentMetrics.ActivePhysicsObjects = 0;
    if (GetWorld())
    {
        for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor && Actor->GetRootComponent())
            {
                if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent()))
                {
                    if (PrimComp->IsSimulatingPhysics())
                    {
                        CurrentMetrics.ActivePhysicsObjects++;
                    }
                }
            }
        }
    }
    
    // Update memory usage
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    CurrentMetrics.MemoryUsageMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);
    
    // Estimate CPU usage (simplified)
    CurrentMetrics.CPUUsagePercent = FMath::Clamp(CurrentMetrics.PhysicsFrameTime / 16.67f * 100.0f, 0.0f, 100.0f);
}

void UPhysicsOptimizer::AdaptLODBasedOnPerformance()
{
    if (!IsPerformanceAcceptable())
    {
        // Reduce quality across all LOD levels
        for (auto& LODPair : LODSettings)
        {
            FPhysicsLODSettings& Settings = LODPair.Value;
            Settings.MaxSimulatedObjects = FMath::Max(Settings.MaxSimulatedObjects - 10, 10);
            Settings.UpdateFrequency *= 0.9f;
            Settings.ConsciousnessFieldResolution *= 0.9f;
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Performance below target, reducing physics quality"));
    }
    else if (CurrentMetrics.PhysicsFrameTime < MaxPhysicsFrameTime * 0.7f)
    {
        // Increase quality if we have performance headroom
        for (auto& LODPair : LODSettings)
        {
            FPhysicsLODSettings& Settings = LODPair.Value;
            Settings.MaxSimulatedObjects = FMath::Min(Settings.MaxSimulatedObjects + 5, 500);
            Settings.UpdateFrequency *= 1.05f;
            Settings.ConsciousnessFieldResolution *= 1.05f;
        }
    }
}

int32 UPhysicsOptimizer::GetGridIndex(const FVector& Location) const
{
    // Convert world coordinates to grid coordinates
    int32 GridX = FMath::FloorToInt(Location.X / GridCellSize) + GridResolution / 2;
    int32 GridY = FMath::FloorToInt(Location.Y / GridCellSize) + GridResolution / 2;
    
    // Clamp to grid bounds
    GridX = FMath::Clamp(GridX, 0, GridResolution - 1);
    GridY = FMath::Clamp(GridY, 0, GridResolution - 1);
    
    return GridY * GridResolution + GridX;
}

void UPhysicsOptimizer::InitializeLODSettings()
{
    // Initialize default LOD settings
    FPhysicsLODSettings HighestSettings;
    HighestSettings.MaxDistance = 500.0f;
    HighestSettings.MaxSimulatedObjects = 200;
    HighestSettings.UpdateFrequency = 120.0f;
    HighestSettings.bEnableConsciousnessPhysics = true;
    HighestSettings.ConsciousnessFieldResolution = 2.0f;
    LODSettings.Add(EPhysicsLOD::Highest, HighestSettings);
    
    FPhysicsLODSettings HighSettings;
    HighSettings.MaxDistance = 1500.0f;
    HighSettings.MaxSimulatedObjects = 150;
    HighSettings.UpdateFrequency = 90.0f;
    HighSettings.bEnableConsciousnessPhysics = true;
    HighSettings.ConsciousnessFieldResolution = 1.5f;
    LODSettings.Add(EPhysicsLOD::High, HighSettings);
    
    FPhysicsLODSettings MediumSettings;
    MediumSettings.MaxDistance = 5000.0f;
    MediumSettings.MaxSimulatedObjects = 100;
    MediumSettings.UpdateFrequency = 60.0f;
    MediumSettings.bEnableConsciousnessPhysics = true;
    MediumSettings.ConsciousnessFieldResolution = 1.0f;
    LODSettings.Add(EPhysicsLOD::Medium, MediumSettings);
    
    FPhysicsLODSettings LowSettings;
    LowSettings.MaxDistance = 15000.0f;
    LowSettings.MaxSimulatedObjects = 50;
    LowSettings.UpdateFrequency = 30.0f;
    LowSettings.bEnableConsciousnessPhysics = false;
    LowSettings.ConsciousnessFieldResolution = 0.5f;
    LODSettings.Add(EPhysicsLOD::Low, LowSettings);
    
    FPhysicsLODSettings MinimalSettings;
    MinimalSettings.MaxDistance = 25000.0f;
    MinimalSettings.MaxSimulatedObjects = 20;
    MinimalSettings.UpdateFrequency = 15.0f;
    MinimalSettings.bEnableConsciousnessPhysics = false;
    MinimalSettings.ConsciousnessFieldResolution = 0.2f;
    LODSettings.Add(EPhysicsLOD::Minimal, MinimalSettings);
}

void UPhysicsOptimizer::OptimizePhysicsSubstep(float DeltaTime)
{
    // Adaptive physics substep based on current performance
    if (GetWorld() && GetWorld()->GetPhysicsScene())
    {
        float TargetSubstepSize = 1.0f / 60.0f; // 60Hz base
        
        if (CurrentMetrics.PhysicsFrameTime > MaxPhysicsFrameTime)
        {
            // Increase substep size to reduce computational load
            TargetSubstepSize *= 1.5f;
        }
        else if (CurrentMetrics.PhysicsFrameTime < MaxPhysicsFrameTime * 0.5f)
        {
            // Decrease substep size for better accuracy
            TargetSubstepSize *= 0.8f;
        }
        
        // Apply substep optimization
        TargetSubstepSize = FMath::Clamp(TargetSubstepSize, 1.0f / 120.0f, 1.0f / 30.0f);
        
        // TODO: Apply to physics scene
        // GetWorld()->GetPhysicsScene()->SetSubstepSize(TargetSubstepSize);
    }
}