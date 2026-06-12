#include "World_PerformanceLODManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Landscape/Classes/Landscape.h"
#include "ProceduralWorldGenerator.h"

UWorld_PerformanceLODManager::UWorld_PerformanceLODManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.5f; // Update every 0.5 seconds
    
    // Initialize LOD settings with performance-focused defaults
    LODSettings.NearDistance = 1000.0f;
    LODSettings.MidDistance = 3000.0f;
    LODSettings.FarDistance = 8000.0f;
    LODSettings.CullingDistance = 15000.0f;
    LODSettings.MaxVegetationInstances = 50000;
    LODSettings.MaxRockInstances = 10000;
    LODSettings.UpdateFrequency = 0.5f;
    
    // Initialize performance tracking
    TargetFrameTime = 16.67f; // 60 FPS target
    AverageFrameTime = 16.67f;
    FrameTimeHistory.Reserve(FRAME_HISTORY_SIZE);
    
    // Initialize biome LOD data
    for (int32 i = 0; i < static_cast<int32>(EBiomeType::MAX); ++i)
    {
        EBiomeType BiomeType = static_cast<EBiomeType>(i);
        FWorld_BiomeLODData BiomeLOD;
        BiomeLOD.BiomeType = BiomeType;
        BiomeLOD.ActiveVegetationCount = 0;
        BiomeLOD.ActiveRockCount = 0;
        BiomeLOD.AverageFrameTime = 16.67f;
        BiomeLOD.bIsOptimized = false;
        BiomeLODMap.Add(BiomeType, BiomeLOD);
    }
}

void UWorld_PerformanceLODManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Find world generator and related components
    if (UWorld* World = GetWorld())
    {
        WorldGenerator = Cast<AProceduralWorldGenerator>(UGameplayStatics::GetActorOfClass(World, AProceduralWorldGenerator::StaticClass()));
        
        // Register with subsystem
        if (UWorld_PerformanceLODSubsystem* Subsystem = UWorld_PerformanceLODSubsystem::Get(World))
        {
            Subsystem->RegisterLODManager(this);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("World_PerformanceLODManager: Initialized with target frame time %.2fms"), TargetFrameTime);
}

void UWorld_PerformanceLODManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Monitor frame performance
    MonitorFramePerformance(DeltaTime);
    
    // Update LOD if auto-LOD is enabled
    if (bAutoLODEnabled)
    {
        if (UWorld* World = GetWorld())
        {
            if (APlayerController* PC = World->GetFirstPlayerController())
            {
                if (APawn* PlayerPawn = PC->GetPawn())
                {
                    FVector PlayerLocation = PlayerPawn->GetActorLocation();
                    UpdateLODLevels(PlayerLocation);
                }
            }
        }
    }
}

void UWorld_PerformanceLODManager::UpdateLODLevels(const FVector& PlayerLocation)
{
    LastPlayerLocation = PlayerLocation;
    
    // Update different LOD systems
    UpdateVegetationLOD(PlayerLocation);
    UpdateRockLOD(PlayerLocation);
    UpdateWaterLOD(PlayerLocation);
    UpdateTerrainLOD(PlayerLocation);
    
    // Analyze performance and adjust if needed
    if (bAutoLODEnabled)
    {
        AdjustLODBasedOnPerformance();
    }
    
    LastUpdateTime = GetWorld()->GetTimeSeconds();
}

void UWorld_PerformanceLODManager::SetGlobalLODLevel(EWorld_LODLevel NewLODLevel)
{
    if (CurrentLODLevel != NewLODLevel)
    {
        CurrentLODLevel = NewLODLevel;
        
        // Apply LOD changes immediately
        ApplyLODToVegetation(NewLODLevel);
        ApplyLODToTerrain(NewLODLevel);
        ApplyLODToBiomes(NewLODLevel);
        
        UE_LOG(LogTemp, Log, TEXT("World_PerformanceLODManager: Global LOD level set to %d"), static_cast<int32>(NewLODLevel));
    }
}

void UWorld_PerformanceLODManager::MonitorFramePerformance(float DeltaTime)
{
    // Convert to milliseconds
    float FrameTimeMs = DeltaTime * 1000.0f;
    
    // Update frame time history
    UpdateFrameTimeHistory(FrameTimeMs);
    
    // Calculate rolling average
    AverageFrameTime = CalculateAverageFrameTime();
}

bool UWorld_PerformanceLODManager::IsPerformanceOptimal() const
{
    return AverageFrameTime <= PERFORMANCE_GOOD_THRESHOLD;
}

void UWorld_PerformanceLODManager::UpdateBiomeLOD(EBiomeType BiomeType, const FVector& BiomeCenter, float BiomeRadius)
{
    if (FWorld_BiomeLODData* BiomeLOD = BiomeLODMap.Find(BiomeType))
    {
        // Calculate distance to player
        float DistanceToPlayer = CalculateDistanceToPlayer(BiomeCenter);
        
        // Determine appropriate LOD level for this biome
        EWorld_LODLevel BiomeLODLevel = DetermineLODLevel(DistanceToPlayer);
        
        // Update biome-specific metrics
        BiomeLOD->AverageFrameTime = AverageFrameTime;
        BiomeLOD->bIsOptimized = (BiomeLODLevel >= EWorld_LODLevel::Medium);
        
        // Apply biome-specific optimizations based on distance and performance
        if (DistanceToPlayer > LODSettings.FarDistance)
        {
            // Far biomes: minimal detail
            BiomeLOD->ActiveVegetationCount = FMath::Min(BiomeLOD->ActiveVegetationCount, 100);
            BiomeLOD->ActiveRockCount = FMath::Min(BiomeLOD->ActiveRockCount, 50);
        }
        else if (DistanceToPlayer > LODSettings.MidDistance)
        {
            // Mid-distance biomes: reduced detail
            BiomeLOD->ActiveVegetationCount = FMath::Min(BiomeLOD->ActiveVegetationCount, 1000);
            BiomeLOD->ActiveRockCount = FMath::Min(BiomeLOD->ActiveRockCount, 200);
        }
        // Near biomes keep full detail
    }
}

FWorld_BiomeLODData UWorld_PerformanceLODManager::GetBiomeLODData(EBiomeType BiomeType) const
{
    if (const FWorld_BiomeLODData* BiomeLOD = BiomeLODMap.Find(BiomeType))
    {
        return *BiomeLOD;
    }
    
    // Return default data if not found
    FWorld_BiomeLODData DefaultData;
    DefaultData.BiomeType = BiomeType;
    return DefaultData;
}

void UWorld_PerformanceLODManager::OptimizeVegetationLOD(const FVector& PlayerLocation)
{
    if (!WorldGenerator)
        return;
    
    // Find all instanced static mesh components (vegetation)
    TArray<UInstancedStaticMeshComponent*> VegetationComponents;
    WorldGenerator->GetComponents<UInstancedStaticMeshComponent>(VegetationComponents);
    
    for (UInstancedStaticMeshComponent* Component : VegetationComponents)
    {
        if (!Component)
            continue;
        
        // Get all instances
        int32 InstanceCount = Component->GetInstanceCount();
        TArray<int32> InstancesToRemove;
        
        for (int32 i = 0; i < InstanceCount; ++i)
        {
            FTransform InstanceTransform;
            if (Component->GetInstanceTransform(i, InstanceTransform, true))
            {
                float Distance = FVector::Dist(PlayerLocation, InstanceTransform.GetLocation());
                
                // Cull based on distance and current LOD level
                bool bShouldCull = false;
                
                switch (CurrentLODLevel)
                {
                case EWorld_LODLevel::Ultra:
                    bShouldCull = Distance > LODSettings.CullingDistance;
                    break;
                case EWorld_LODLevel::High:
                    bShouldCull = Distance > (LODSettings.CullingDistance * 0.8f);
                    break;
                case EWorld_LODLevel::Medium:
                    bShouldCull = Distance > (LODSettings.CullingDistance * 0.6f);
                    break;
                case EWorld_LODLevel::Low:
                    bShouldCull = Distance > (LODSettings.CullingDistance * 0.4f);
                    break;
                case EWorld_LODLevel::Minimal:
                    bShouldCull = Distance > (LODSettings.CullingDistance * 0.2f);
                    break;
                }
                
                if (bShouldCull)
                {
                    InstancesToRemove.Add(i);
                }
            }
        }
        
        // Remove distant instances (in reverse order to maintain indices)
        for (int32 i = InstancesToRemove.Num() - 1; i >= 0; --i)
        {
            Component->RemoveInstance(InstancesToRemove[i]);
        }
    }
}

void UWorld_PerformanceLODManager::CullDistantVegetation(float CullingDistance)
{
    if (!WorldGenerator)
        return;
    
    FVector PlayerLocation = LastPlayerLocation;
    
    // Find all vegetation components and cull based on distance
    TArray<UInstancedStaticMeshComponent*> VegetationComponents;
    WorldGenerator->GetComponents<UInstancedStaticMeshComponent>(VegetationComponents);
    
    int32 TotalCulled = 0;
    
    for (UInstancedStaticMeshComponent* Component : VegetationComponents)
    {
        if (!Component)
            continue;
        
        int32 InstanceCount = Component->GetInstanceCount();
        TArray<int32> InstancesToRemove;
        
        for (int32 i = 0; i < InstanceCount; ++i)
        {
            FTransform InstanceTransform;
            if (Component->GetInstanceTransform(i, InstanceTransform, true))
            {
                float Distance = FVector::Dist(PlayerLocation, InstanceTransform.GetLocation());
                if (Distance > CullingDistance)
                {
                    InstancesToRemove.Add(i);
                }
            }
        }
        
        // Remove instances in reverse order
        for (int32 i = InstancesToRemove.Num() - 1; i >= 0; --i)
        {
            Component->RemoveInstance(InstancesToRemove[i]);
            TotalCulled++;
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("World_PerformanceLODManager: Culled %d vegetation instances beyond %.0f units"), TotalCulled, CullingDistance);
}

void UWorld_PerformanceLODManager::UpdateTerrainLOD(const FVector& PlayerLocation)
{
    // Find landscape actors and adjust their LOD
    if (UWorld* World = GetWorld())
    {
        TArray<AActor*> LandscapeActors;
        UGameplayStatics::GetAllActorsOfClass(World, ALandscape::StaticClass(), LandscapeActors);
        
        for (AActor* Actor : LandscapeActors)
        {
            if (ALandscape* Landscape = Cast<ALandscape>(Actor))
            {
                // Adjust landscape LOD based on distance and performance
                float Distance = FVector::Dist(PlayerLocation, Landscape->GetActorLocation());
                
                int32 LODBias = 0;
                switch (CurrentLODLevel)
                {
                case EWorld_LODLevel::Ultra:
                    LODBias = 0;
                    break;
                case EWorld_LODLevel::High:
                    LODBias = 1;
                    break;
                case EWorld_LODLevel::Medium:
                    LODBias = 2;
                    break;
                case EWorld_LODLevel::Low:
                    LODBias = 3;
                    break;
                case EWorld_LODLevel::Minimal:
                    LODBias = 4;
                    break;
                }
                
                // Apply LOD bias to landscape
                Landscape->SetLODBias(LODBias);
            }
        }
    }
}

void UWorld_PerformanceLODManager::SetTerrainDetailLevel(int32 DetailLevel)
{
    if (UWorld* World = GetWorld())
    {
        TArray<AActor*> LandscapeActors;
        UGameplayStatics::GetAllActorsOfClass(World, ALandscape::StaticClass(), LandscapeActors);
        
        for (AActor* Actor : LandscapeActors)
        {
            if (ALandscape* Landscape = Cast<ALandscape>(Actor))
            {
                Landscape->SetLODBias(FMath::Clamp(DetailLevel, 0, 8));
            }
        }
    }
}

void UWorld_PerformanceLODManager::IntegrateWithSurvivalPerformance()
{
    // Integration with survival performance systems
    // This would coordinate with the survival performance integrator from Agent #4
    
    UE_LOG(LogTemp, Log, TEXT("World_PerformanceLODManager: Integrating with survival performance systems"));
    
    // Adjust world LOD based on survival system performance budget
    if (AverageFrameTime > TargetFrameTime * 1.2f) // 20% over target
    {
        // Reduce world detail to free up performance for survival systems
        if (CurrentLODLevel < EWorld_LODLevel::Minimal)
        {
            SetGlobalLODLevel(static_cast<EWorld_LODLevel>(static_cast<int32>(CurrentLODLevel) + 1));
        }
    }
    else if (AverageFrameTime < TargetFrameTime * 0.8f) // 20% under target
    {
        // Increase world detail since we have performance headroom
        if (CurrentLODLevel > EWorld_LODLevel::Ultra)
        {
            SetGlobalLODLevel(static_cast<EWorld_LODLevel>(static_cast<int32>(CurrentLODLevel) - 1));
        }
    }
}

void UWorld_PerformanceLODManager::SetPerformanceBudget(float MaxFrameTimeMs)
{
    TargetFrameTime = MaxFrameTimeMs;
    UE_LOG(LogTemp, Log, TEXT("World_PerformanceLODManager: Performance budget set to %.2fms"), MaxFrameTimeMs);
}

// Private Methods

void UWorld_PerformanceLODManager::UpdateVegetationLOD(const FVector& PlayerLocation)
{
    OptimizeVegetationLOD(PlayerLocation);
}

void UWorld_PerformanceLODManager::UpdateRockLOD(const FVector& PlayerLocation)
{
    // Similar to vegetation but for rock/geological formations
    if (!WorldGenerator)
        return;
    
    // This would handle rock formations, cliffs, etc.
    // Implementation similar to vegetation but with different distance thresholds
}

void UWorld_PerformanceLODManager::UpdateWaterLOD(const FVector& PlayerLocation)
{
    // Handle water body LOD (rivers, lakes, ocean)
    // Reduce water simulation quality at distance
}

void UWorld_PerformanceLODManager::AnalyzePerformanceMetrics()
{
    // Analyze current performance trends
    if (FrameTimeHistory.Num() >= FRAME_HISTORY_SIZE)
    {
        // Calculate performance stability
        float MinFrameTime = FMath::Min(FrameTimeHistory);
        float MaxFrameTime = FMath::Max(FrameTimeHistory);
        float FrameTimeVariance = MaxFrameTime - MinFrameTime;
        
        // Log performance analysis
        UE_LOG(LogTemp, VeryVerbose, TEXT("Performance Analysis - Avg: %.2fms, Min: %.2fms, Max: %.2fms, Variance: %.2fms"), 
               AverageFrameTime, MinFrameTime, MaxFrameTime, FrameTimeVariance);
    }
}

void UWorld_PerformanceLODManager::AdjustLODBasedOnPerformance()
{
    if (AverageFrameTime > PERFORMANCE_BAD_THRESHOLD)
    {
        // Performance is bad, reduce LOD
        if (CurrentLODLevel < EWorld_LODLevel::Minimal)
        {
            EWorld_LODLevel NewLOD = static_cast<EWorld_LODLevel>(static_cast<int32>(CurrentLODLevel) + 1);
            SetGlobalLODLevel(NewLOD);
            UE_LOG(LogTemp, Warning, TEXT("Performance degraded (%.2fms), reducing LOD to %d"), AverageFrameTime, static_cast<int32>(NewLOD));
        }
    }
    else if (AverageFrameTime < PERFORMANCE_GOOD_THRESHOLD)
    {
        // Performance is good, can increase LOD
        if (CurrentLODLevel > EWorld_LODLevel::Ultra)
        {
            EWorld_LODLevel NewLOD = static_cast<EWorld_LODLevel>(static_cast<int32>(CurrentLODLevel) - 1);
            SetGlobalLODLevel(NewLOD);
            UE_LOG(LogTemp, Log, TEXT("Performance improved (%.2fms), increasing LOD to %d"), AverageFrameTime, static_cast<int32>(NewLOD));
        }
    }
}

float UWorld_PerformanceLODManager::CalculateDistanceToPlayer(const FVector& Location) const
{
    return FVector::Dist(LastPlayerLocation, Location);
}

EWorld_LODLevel UWorld_PerformanceLODManager::DetermineLODLevel(float Distance) const
{
    if (Distance <= LODSettings.NearDistance)
        return EWorld_LODLevel::Ultra;
    else if (Distance <= LODSettings.MidDistance)
        return EWorld_LODLevel::High;
    else if (Distance <= LODSettings.FarDistance)
        return EWorld_LODLevel::Medium;
    else if (Distance <= LODSettings.CullingDistance)
        return EWorld_LODLevel::Low;
    else
        return EWorld_LODLevel::Minimal;
}

void UWorld_PerformanceLODManager::UpdateFrameTimeHistory(float DeltaTime)
{
    FrameTimeHistory.Add(DeltaTime);
    
    // Keep history size manageable
    if (FrameTimeHistory.Num() > FRAME_HISTORY_SIZE)
    {
        FrameTimeHistory.RemoveAt(0);
    }
}

float UWorld_PerformanceLODManager::CalculateAverageFrameTime() const
{
    if (FrameTimeHistory.Num() == 0)
        return 16.67f;
    
    float Sum = 0.0f;
    for (float FrameTime : FrameTimeHistory)
    {
        Sum += FrameTime;
    }
    
    return Sum / FrameTimeHistory.Num();
}

void UWorld_PerformanceLODManager::ApplyLODToVegetation(EWorld_LODLevel LODLevel)
{
    // Apply LOD settings to vegetation systems
    float CullingMultiplier = 1.0f;
    
    switch (LODLevel)
    {
    case EWorld_LODLevel::Ultra:
        CullingMultiplier = 1.0f;
        break;
    case EWorld_LODLevel::High:
        CullingMultiplier = 0.8f;
        break;
    case EWorld_LODLevel::Medium:
        CullingMultiplier = 0.6f;
        break;
    case EWorld_LODLevel::Low:
        CullingMultiplier = 0.4f;
        break;
    case EWorld_LODLevel::Minimal:
        CullingMultiplier = 0.2f;
        break;
    }
    
    CullDistantVegetation(LODSettings.CullingDistance * CullingMultiplier);
}

void UWorld_PerformanceLODManager::ApplyLODToTerrain(EWorld_LODLevel LODLevel)
{
    SetTerrainDetailLevel(static_cast<int32>(LODLevel));
}

void UWorld_PerformanceLODManager::ApplyLODToBiomes(EWorld_LODLevel LODLevel)
{
    // Apply LOD to all registered biomes
    for (auto& BiomePair : BiomeLODMap)
    {
        FWorld_BiomeLODData& BiomeLOD = BiomePair.Value;
        BiomeLOD.bIsOptimized = (LODLevel >= EWorld_LODLevel::Medium);
    }
}

// Subsystem Implementation

void UWorld_PerformanceLODSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    UE_LOG(LogTemp, Log, TEXT("World_PerformanceLODSubsystem: Initialized"));
}

void UWorld_PerformanceLODSubsystem::Deinitialize()
{
    RegisteredLODManagers.Empty();
    Super::Deinitialize();
}

UWorld_PerformanceLODSubsystem* UWorld_PerformanceLODSubsystem::Get(const UWorld* World)
{
    if (World)
    {
        return World->GetSubsystem<UWorld_PerformanceLODSubsystem>();
    }
    return nullptr;
}

void UWorld_PerformanceLODSubsystem::RegisterLODManager(UWorld_PerformanceLODManager* LODManager)
{
    if (LODManager && !RegisteredLODManagers.Contains(LODManager))
    {
        RegisteredLODManagers.Add(LODManager);
        UE_LOG(LogTemp, Log, TEXT("World_PerformanceLODSubsystem: Registered LOD Manager"));
    }
}

void UWorld_PerformanceLODSubsystem::UnregisterLODManager(UWorld_PerformanceLODManager* LODManager)
{
    RegisteredLODManagers.Remove(LODManager);
}

void UWorld_PerformanceLODSubsystem::UpdateAllLODManagers(const FVector& PlayerLocation)
{
    for (UWorld_PerformanceLODManager* Manager : RegisteredLODManagers)
    {
        if (IsValid(Manager))
        {
            Manager->UpdateLODLevels(PlayerLocation);
        }
    }
}

void UWorld_PerformanceLODSubsystem::SetGlobalPerformanceTarget(float TargetFrameTimeMs)
{
    GlobalPerformanceTarget = TargetFrameTimeMs;
    
    for (UWorld_PerformanceLODManager* Manager : RegisteredLODManagers)
    {
        if (IsValid(Manager))
        {
            Manager->SetPerformanceBudget(TargetFrameTimeMs);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("World_PerformanceLODSubsystem: Global performance target set to %.2fms for %d managers"), 
           TargetFrameTimeMs, RegisteredLODManagers.Num());
}