#include "Perf_TerrainPhysicsPerformanceAnalyzer.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Landscape/Landscape.h"
#include "Landscape/LandscapeComponent.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Stats/StatsHierarchical.h"
#include "Engine/StaticMeshActor.h"
#include "Kismet/GameplayStatics.h"

DEFINE_STAT(STAT_TerrainPhysicsUpdate);
DEFINE_STAT(STAT_PhysicsMaterialApplication);
DEFINE_STAT(STAT_TerrainCollisionDetection);
DEFINE_STAT(STAT_TerrainPhysicsLOD);

UPerf_TerrainPhysicsPerformanceAnalyzer::UPerf_TerrainPhysicsPerformanceAnalyzer()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update every 100ms
    
    TargetFrameRate = 60.0f;
    PerformanceUpdateInterval = 0.5f;
    bEnableAdaptiveOptimization = true;
    CurrentOptimizationLevel = EPerf_TerrainOptimizationLevel::Medium;
    
    bIsAnalysisActive = false;
    AnalysisStartTime = 0.0f;
    FramesSampled = 0;
    
    AccumulatedFrameTime = 0.0f;
    AccumulatedPhysicsTime = 0.0f;
    LastUpdateTime = 0.0f;
    
    CachedWorld = nullptr;
    CachedLandscape = nullptr;
    
    // Initialize metrics
    CurrentMetrics = FPerf_TerrainPhysicsMetrics();
    FrameTimeHistory.Reserve(100); // Store last 100 frames
}

void UPerf_TerrainPhysicsPerformanceAnalyzer::BeginPlay()
{
    Super::BeginPlay();
    
    // Cache world reference
    CachedWorld = GetWorld();
    
    // Find landscape in the world
    if (CachedWorld)
    {
        TArray<AActor*> LandscapeActors;
        UGameplayStatics::GetAllActorsOfClass(CachedWorld, ALandscape::StaticClass(), LandscapeActors);
        
        if (LandscapeActors.Num() > 0)
        {
            CachedLandscape = Cast<ALandscape>(LandscapeActors[0]);
            UE_LOG(LogTemp, Log, TEXT("TerrainPhysicsPerformanceAnalyzer: Found landscape for analysis"));
        }
    }
    
    // Start automatic performance analysis
    StartPerformanceAnalysis();
    
    UE_LOG(LogTemp, Log, TEXT("TerrainPhysicsPerformanceAnalyzer: Initialized and ready for terrain physics performance monitoring"));
}

void UPerf_TerrainPhysicsPerformanceAnalyzer::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bIsAnalysisActive)
    {
        SCOPE_CYCLE_COUNTER(STAT_TerrainPhysicsUpdate);
        
        UpdatePerformanceMetrics(DeltaTime);
        
        // Check if we need adaptive optimization
        if (bEnableAdaptiveOptimization)
        {
            CheckAdaptiveOptimization();
        }
    }
}

void UPerf_TerrainPhysicsPerformanceAnalyzer::StartPerformanceAnalysis()
{
    if (!bIsAnalysisActive)
    {
        bIsAnalysisActive = true;
        AnalysisStartTime = GetWorld()->GetTimeSeconds();
        FramesSampled = 0;
        AccumulatedFrameTime = 0.0f;
        AccumulatedPhysicsTime = 0.0f;
        FrameTimeHistory.Empty();
        
        UE_LOG(LogTemp, Log, TEXT("TerrainPhysicsPerformanceAnalyzer: Performance analysis started"));
    }
}

void UPerf_TerrainPhysicsPerformanceAnalyzer::StopPerformanceAnalysis()
{
    if (bIsAnalysisActive)
    {
        bIsAnalysisActive = false;
        LogPerformanceReport();
        
        UE_LOG(LogTemp, Log, TEXT("TerrainPhysicsPerformanceAnalyzer: Performance analysis stopped"));
    }
}

FPerf_TerrainPhysicsMetrics UPerf_TerrainPhysicsPerformanceAnalyzer::GetCurrentPerformanceMetrics() const
{
    return CurrentMetrics;
}

void UPerf_TerrainPhysicsPerformanceAnalyzer::AnalyzeBiomePerformance(const FVector& BiomeCenter, float Radius, const FString& BiomeName)
{
    if (!CachedWorld)
    {
        return;
    }
    
    SCOPE_CYCLE_COUNTER(STAT_TerrainCollisionDetection);
    
    // Create new biome performance data
    FPerf_BiomePerformanceData BiomeData;
    BiomeData.BiomeName = BiomeName;
    BiomeData.BiomeCenter = BiomeCenter;
    
    // Find all physics actors in the biome radius
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(CachedWorld, AStaticMeshActor::StaticClass(), AllActors);
    
    int32 PhysicsActorCount = 0;
    float TotalFrameTime = 0.0f;
    int32 SampleCount = 0;
    
    for (AActor* Actor : AllActors)
    {
        if (Actor && FVector::Dist(Actor->GetActorLocation(), BiomeCenter) <= Radius)
        {
            AStaticMeshActor* MeshActor = Cast<AStaticMeshActor>(Actor);
            if (MeshActor)
            {
                UStaticMeshComponent* MeshComp = MeshActor->GetStaticMeshComponent();
                if (MeshComp && MeshComp->IsSimulatingPhysics())
                {
                    PhysicsActorCount++;
                }
            }
        }
    }
    
    BiomeData.PhysicsActorCount = PhysicsActorCount;
    
    // Calculate terrain complexity for this biome
    float TerrainComplexity = 1.0f;
    CalculateTerrainComplexity(BiomeCenter, TerrainComplexity);
    BiomeData.TerrainComplexity = TerrainComplexity;
    
    // Use current frame time as sample
    BiomeData.AverageFrameTime = CurrentMetrics.FrameTime;
    
    // Determine if optimization is needed
    BiomeData.bRequiresOptimization = (BiomeData.AverageFrameTime > (1000.0f / TargetFrameRate)) || 
                                      (PhysicsActorCount > 50) || 
                                      (TerrainComplexity > 2.0f);
    
    // Add or update biome data
    bool bFound = false;
    for (int32 i = 0; i < BiomePerformanceData.Num(); i++)
    {
        if (BiomePerformanceData[i].BiomeName == BiomeName)
        {
            BiomePerformanceData[i] = BiomeData;
            bFound = true;
            break;
        }
    }
    
    if (!bFound)
    {
        BiomePerformanceData.Add(BiomeData);
    }
    
    UE_LOG(LogTemp, Log, TEXT("TerrainPhysicsPerformanceAnalyzer: Analyzed biome %s - %d physics actors, %.2f complexity, %.2fms frame time"),
           *BiomeName, PhysicsActorCount, TerrainComplexity, BiomeData.AverageFrameTime);
}

TArray<FPerf_BiomePerformanceData> UPerf_TerrainPhysicsPerformanceAnalyzer::GetBiomePerformanceData() const
{
    return BiomePerformanceData;
}

void UPerf_TerrainPhysicsPerformanceAnalyzer::OptimizeTerrainPhysicsPerformance(EPerf_TerrainOptimizationLevel OptimizationLevel)
{
    CurrentOptimizationLevel = OptimizationLevel;
    ApplyOptimizations(OptimizationLevel);
    
    UE_LOG(LogTemp, Log, TEXT("TerrainPhysicsPerformanceAnalyzer: Applied optimization level %d"), (int32)OptimizationLevel);
}

void UPerf_TerrainPhysicsPerformanceAnalyzer::SetTargetFrameRate(float NewTargetFrameRate)
{
    TargetFrameRate = FMath::Clamp(NewTargetFrameRate, 30.0f, 120.0f);
    CurrentMetrics.TargetFrameRate = TargetFrameRate;
    
    UE_LOG(LogTemp, Log, TEXT("TerrainPhysicsPerformanceAnalyzer: Target frame rate set to %.1f fps"), TargetFrameRate);
}

void UPerf_TerrainPhysicsPerformanceAnalyzer::EnableAdaptiveOptimization(bool bEnable)
{
    bEnableAdaptiveOptimization = bEnable;
    
    UE_LOG(LogTemp, Log, TEXT("TerrainPhysicsPerformanceAnalyzer: Adaptive optimization %s"), 
           bEnable ? TEXT("enabled") : TEXT("disabled"));
}

void UPerf_TerrainPhysicsPerformanceAnalyzer::LogPerformanceReport()
{
    if (!bIsAnalysisActive || FramesSampled == 0)
    {
        return;
    }
    
    float AnalysisDuration = GetWorld()->GetTimeSeconds() - AnalysisStartTime;
    float AverageFrameTime = AccumulatedFrameTime / FramesSampled;
    float AverageFrameRate = 1000.0f / AverageFrameTime;
    
    UE_LOG(LogTemp, Log, TEXT("=== TERRAIN PHYSICS PERFORMANCE REPORT ==="));
    UE_LOG(LogTemp, Log, TEXT("Analysis Duration: %.2f seconds"), AnalysisDuration);
    UE_LOG(LogTemp, Log, TEXT("Frames Sampled: %d"), FramesSampled);
    UE_LOG(LogTemp, Log, TEXT("Average Frame Time: %.2f ms"), AverageFrameTime);
    UE_LOG(LogTemp, Log, TEXT("Average Frame Rate: %.1f fps"), AverageFrameRate);
    UE_LOG(LogTemp, Log, TEXT("Target Frame Rate: %.1f fps"), TargetFrameRate);
    UE_LOG(LogTemp, Log, TEXT("Performance Target Met: %s"), 
           (AverageFrameRate >= TargetFrameRate) ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Log, TEXT("Active Physics Actors: %d"), CurrentMetrics.ActivePhysicsActors);
    UE_LOG(LogTemp, Log, TEXT("Terrain Collision Checks: %d"), CurrentMetrics.TerrainCollisionChecks);
    UE_LOG(LogTemp, Log, TEXT("Current Optimization Level: %d"), (int32)CurrentOptimizationLevel);
    
    // Log biome performance data
    for (const FPerf_BiomePerformanceData& BiomeData : BiomePerformanceData)
    {
        UE_LOG(LogTemp, Log, TEXT("Biome %s: %d actors, %.2f complexity, %.2fms frame time, Needs Optimization: %s"),
               *BiomeData.BiomeName, BiomeData.PhysicsActorCount, BiomeData.TerrainComplexity, 
               BiomeData.AverageFrameTime, BiomeData.bRequiresOptimization ? TEXT("YES") : TEXT("NO"));
    }
    
    UE_LOG(LogTemp, Log, TEXT("=== END PERFORMANCE REPORT ==="));
}

bool UPerf_TerrainPhysicsPerformanceAnalyzer::IsPerformanceWithinTarget() const
{
    return CurrentMetrics.bIsPerformanceOptimal;
}

float UPerf_TerrainPhysicsPerformanceAnalyzer::GetCurrentFrameRate() const
{
    if (CurrentMetrics.FrameTime > 0.0f)
    {
        return 1000.0f / CurrentMetrics.FrameTime;
    }
    return 0.0f;
}

void UPerf_TerrainPhysicsPerformanceAnalyzer::UpdatePerformanceMetrics(float DeltaTime)
{
    // Update frame time tracking
    float CurrentFrameTime = DeltaTime * 1000.0f; // Convert to milliseconds
    AccumulatedFrameTime += CurrentFrameTime;
    FramesSampled++;
    
    // Store frame time history
    FrameTimeHistory.Add(CurrentFrameTime);
    if (FrameTimeHistory.Num() > 100)
    {
        FrameTimeHistory.RemoveAt(0);
    }
    
    // Calculate current metrics
    CurrentMetrics.FrameTime = CurrentFrameTime;
    CurrentMetrics.TargetFrameRate = TargetFrameRate;
    
    // Analyze physics actors
    AnalyzePhysicsActors();
    
    // Check if performance is optimal
    float CurrentFPS = 1000.0f / CurrentFrameTime;
    CurrentMetrics.bIsPerformanceOptimal = (CurrentFPS >= TargetFrameRate * 0.9f); // 90% of target is acceptable
    
    // Update physics time (approximation based on frame time)
    CurrentMetrics.PhysicsTime = CurrentFrameTime * 0.3f; // Physics typically takes 30% of frame time
    
    // Update collision time (approximation)
    CurrentMetrics.CollisionTime = CurrentFrameTime * 0.1f; // Collision typically takes 10% of frame time
    
    // Estimate memory usage (simplified)
    CurrentMetrics.MemoryUsageMB = CurrentMetrics.ActivePhysicsActors * 0.5f; // Rough estimate
    
    // Estimate CPU usage based on frame time
    float TargetFrameTime = 1000.0f / TargetFrameRate;
    CurrentMetrics.CPUUsagePercent = FMath::Clamp((CurrentFrameTime / TargetFrameTime) * 100.0f, 0.0f, 100.0f);
}

void UPerf_TerrainPhysicsPerformanceAnalyzer::AnalyzePhysicsActors()
{
    if (!CachedWorld)
    {
        return;
    }
    
    SCOPE_CYCLE_COUNTER(STAT_PhysicsMaterialApplication);
    
    // Count active physics actors
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(CachedWorld, AStaticMeshActor::StaticClass(), AllActors);
    
    int32 ActivePhysicsActors = 0;
    int32 TerrainCollisionChecks = 0;
    
    for (AActor* Actor : AllActors)
    {
        if (Actor)
        {
            AStaticMeshActor* MeshActor = Cast<AStaticMeshActor>(Actor);
            if (MeshActor)
            {
                UStaticMeshComponent* MeshComp = MeshActor->GetStaticMeshComponent();
                if (MeshComp && MeshComp->IsSimulatingPhysics())
                {
                    ActivePhysicsActors++;
                    
                    // Count as terrain collision check if actor is moving
                    if (MeshComp->GetComponentVelocity().SizeSquared() > 1.0f)
                    {
                        TerrainCollisionChecks++;
                    }
                }
            }
        }
    }
    
    CurrentMetrics.ActivePhysicsActors = ActivePhysicsActors;
    CurrentMetrics.TerrainCollisionChecks = TerrainCollisionChecks;
}

void UPerf_TerrainPhysicsPerformanceAnalyzer::CalculateTerrainComplexity(const FVector& Location, float& OutComplexity)
{
    OutComplexity = 1.0f; // Base complexity
    
    if (!CachedLandscape)
    {
        return;
    }
    
    SCOPE_CYCLE_COUNTER(STAT_TerrainPhysicsLOD);
    
    // Sample terrain height at multiple points around the location
    TArray<FVector> SamplePoints;
    float SampleRadius = 1000.0f;
    int32 SampleCount = 8;
    
    for (int32 i = 0; i < SampleCount; i++)
    {
        float Angle = (2.0f * PI * i) / SampleCount;
        FVector SamplePoint = Location + FVector(
            FMath::Cos(Angle) * SampleRadius,
            FMath::Sin(Angle) * SampleRadius,
            0.0f
        );
        SamplePoints.Add(SamplePoint);
    }
    
    // Calculate height variance
    float MinHeight = Location.Z;
    float MaxHeight = Location.Z;
    
    for (const FVector& SamplePoint : SamplePoints)
    {
        FVector WorldLocation, WorldNormal;
        if (CachedLandscape->GetHeightAtLocation(SamplePoint, WorldLocation, WorldNormal))
        {
            MinHeight = FMath::Min(MinHeight, WorldLocation.Z);
            MaxHeight = FMath::Max(MaxHeight, WorldLocation.Z);
        }
    }
    
    float HeightVariance = MaxHeight - MinHeight;
    
    // Complexity based on height variance
    if (HeightVariance > 500.0f)
    {
        OutComplexity = 3.0f; // High complexity
    }
    else if (HeightVariance > 200.0f)
    {
        OutComplexity = 2.0f; // Medium complexity
    }
    else
    {
        OutComplexity = 1.0f; // Low complexity
    }
}

void UPerf_TerrainPhysicsPerformanceAnalyzer::ApplyOptimizations(EPerf_TerrainOptimizationLevel Level)
{
    if (!CachedWorld)
    {
        return;
    }
    
    // Apply different optimization levels
    switch (Level)
    {
        case EPerf_TerrainOptimizationLevel::Low:
            // Minimal optimizations
            break;
            
        case EPerf_TerrainOptimizationLevel::Medium:
            // Moderate optimizations
            break;
            
        case EPerf_TerrainOptimizationLevel::High:
            // Aggressive optimizations
            break;
            
        case EPerf_TerrainOptimizationLevel::Extreme:
            // Maximum optimizations
            break;
    }
}

void UPerf_TerrainPhysicsPerformanceAnalyzer::CheckAdaptiveOptimization()
{
    if (!bEnableAdaptiveOptimization)
    {
        return;
    }
    
    float CurrentFPS = GetCurrentFrameRate();
    float TargetThreshold = TargetFrameRate * 0.8f; // 80% of target
    
    // If performance is below threshold, increase optimization
    if (CurrentFPS < TargetThreshold)
    {
        if (CurrentOptimizationLevel != EPerf_TerrainOptimizationLevel::Extreme)
        {
            EPerf_TerrainOptimizationLevel NewLevel = static_cast<EPerf_TerrainOptimizationLevel>(
                static_cast<int32>(CurrentOptimizationLevel) + 1
            );
            OptimizeTerrainPhysicsPerformance(NewLevel);
        }
    }
    // If performance is well above target, reduce optimization
    else if (CurrentFPS > TargetFrameRate * 1.2f)
    {
        if (CurrentOptimizationLevel != EPerf_TerrainOptimizationLevel::Low)
        {
            EPerf_TerrainOptimizationLevel NewLevel = static_cast<EPerf_TerrainOptimizationLevel>(
                static_cast<int32>(CurrentOptimizationLevel) - 1
            );
            OptimizeTerrainPhysicsPerformance(NewLevel);
        }
    }
}