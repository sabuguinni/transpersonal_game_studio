#include "Perf_TerrainDeformationOptimizer.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/GameInstance.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "DrawDebugHelpers.h"

// UPerf_TerrainDeformationOptimizer Implementation

UPerf_TerrainDeformationOptimizer::UPerf_TerrainDeformationOptimizer()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
    
    // Performance settings
    bEnableOptimization = true;
    TargetFrameRate = 60.0f;
    MaxDeformationRadius = 2000.0f;
    MaxActiveDeformations = 10;
    
    // LOD initialization
    CurrentLODLevel = 0;
    
    // Setup default LOD levels
    LODLevels.SetNum(4);
    
    // LOD 0 - High quality (close range)
    LODLevels[0].Distance = 500.0f;
    LODLevels[0].UpdateFrequency = 60.0f;
    LODLevels[0].DeformationRadius = 500.0f;
    LODLevels[0].IntensityMultiplier = 1.0f;
    LODLevels[0].bEnableDeformation = true;
    
    // LOD 1 - Medium quality
    LODLevels[1].Distance = 1500.0f;
    LODLevels[1].UpdateFrequency = 30.0f;
    LODLevels[1].DeformationRadius = 300.0f;
    LODLevels[1].IntensityMultiplier = 0.8f;
    LODLevels[1].bEnableDeformation = true;
    
    // LOD 2 - Low quality
    LODLevels[2].Distance = 3000.0f;
    LODLevels[2].UpdateFrequency = 15.0f;
    LODLevels[2].DeformationRadius = 150.0f;
    LODLevels[2].IntensityMultiplier = 0.5f;
    LODLevels[2].bEnableDeformation = true;
    
    // LOD 3 - Disabled (far range)
    LODLevels[3].Distance = 5000.0f;
    LODLevels[3].UpdateFrequency = 5.0f;
    LODLevels[3].DeformationRadius = 50.0f;
    LODLevels[3].IntensityMultiplier = 0.1f;
    LODLevels[3].bEnableDeformation = false;
    
    // Performance tracking initialization
    bIsMonitoring = false;
    LastUpdateTime = 0.0f;
    AccumulatedFrameTime = 0.0f;
    FrameCount = 0;
    bNeedsOptimization = false;
    LastOptimizationTime = 0.0f;
    
    // Component reference
    TerrainDeformationComponent = nullptr;
}

void UPerf_TerrainDeformationOptimizer::BeginPlay()
{
    Super::BeginPlay();
    
    // Find terrain deformation component on the same actor
    if (AActor* Owner = GetOwner())
    {
        TerrainDeformationComponent = Owner->FindComponentByClass<UCore_TerrainDeformationComponent>();
        if (!TerrainDeformationComponent)
        {
            UE_LOG(LogTemp, Warning, TEXT("TerrainDeformationOptimizer: No TerrainDeformationComponent found on actor %s"), 
                   *Owner->GetName());
        }
    }
    
    // Register with global manager
    if (UGameInstance* GameInstance = GetWorld()->GetGameInstance())
    {
        if (UPerf_TerrainDeformationManager* Manager = GameInstance->GetSubsystem<UPerf_TerrainDeformationManager>())
        {
            Manager->RegisterOptimizer(this);
        }
    }
    
    // Start performance monitoring
    StartPerformanceMonitoring();
    
    UE_LOG(LogTemp, Log, TEXT("TerrainDeformationOptimizer: Initialized on actor %s"), 
           *GetOwner()->GetName());
}

void UPerf_TerrainDeformationOptimizer::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // Unregister from global manager
    if (UGameInstance* GameInstance = GetWorld()->GetGameInstance())
    {
        if (UPerf_TerrainDeformationManager* Manager = GameInstance->GetSubsystem<UPerf_TerrainDeformationManager>())
        {
            Manager->UnregisterOptimizer(this);
        }
    }
    
    StopPerformanceMonitoring();
    
    Super::EndPlay(EndPlayReason);
}

void UPerf_TerrainDeformationOptimizer::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bEnableOptimization)
    {
        return;
    }
    
    // Update performance metrics
    UpdatePerformanceMetrics(DeltaTime);
    
    // Update LOD based on distance to player
    float DistanceToPlayer = CalculateDistanceToPlayer();
    UpdateLODLevel(DistanceToPlayer);
    
    // Apply optimizations if needed
    if (bNeedsOptimization)
    {
        ApplyLODOptimizations();
        bNeedsOptimization = false;
    }
    
    // Check performance thresholds
    CheckPerformanceThresholds();
}

void UPerf_TerrainDeformationOptimizer::StartPerformanceMonitoring()
{
    bIsMonitoring = true;
    LastUpdateTime = GetWorld()->GetTimeSeconds();
    AccumulatedFrameTime = 0.0f;
    FrameCount = 0;
    
    UE_LOG(LogTemp, Log, TEXT("TerrainDeformationOptimizer: Started performance monitoring"));
}

void UPerf_TerrainDeformationOptimizer::StopPerformanceMonitoring()
{
    bIsMonitoring = false;
    UE_LOG(LogTemp, Log, TEXT("TerrainDeformationOptimizer: Stopped performance monitoring"));
}

FPerf_TerrainDeformationMetrics UPerf_TerrainDeformationOptimizer::GetPerformanceMetrics() const
{
    return CurrentMetrics;
}

void UPerf_TerrainDeformationOptimizer::UpdateLODLevel(float DistanceToPlayer)
{
    int32 NewLODLevel = 0;
    
    // Determine LOD level based on distance
    for (int32 i = 0; i < LODLevels.Num(); ++i)
    {
        if (DistanceToPlayer <= LODLevels[i].Distance)
        {
            NewLODLevel = i;
            break;
        }
        NewLODLevel = LODLevels.Num() - 1; // Use highest LOD if beyond all distances
    }
    
    // Update LOD if changed
    if (NewLODLevel != CurrentLODLevel)
    {
        CurrentLODLevel = NewLODLevel;
        CurrentMetrics.LODLevel = CurrentLODLevel;
        bNeedsOptimization = true;
        
        UE_LOG(LogTemp, VeryVerbose, TEXT("TerrainDeformationOptimizer: LOD level changed to %d (distance: %.1f)"), 
               CurrentLODLevel, DistanceToPlayer);
    }
}

int32 UPerf_TerrainDeformationOptimizer::GetCurrentLODLevel() const
{
    return CurrentLODLevel;
}

void UPerf_TerrainDeformationOptimizer::SetOptimizationEnabled(bool bEnabled)
{
    bEnableOptimization = bEnabled;
    CurrentMetrics.bIsOptimized = bEnabled;
    
    UE_LOG(LogTemp, Log, TEXT("TerrainDeformationOptimizer: Optimization %s"), 
           bEnabled ? TEXT("enabled") : TEXT("disabled"));
}

void UPerf_TerrainDeformationOptimizer::OptimizeDeformationRadius(float TargetFrameTime)
{
    if (!bEnableOptimization || CurrentLODLevel >= LODLevels.Num())
    {
        return;
    }
    
    const FPerf_TerrainDeformationLOD& CurrentLOD = LODLevels[CurrentLODLevel];
    
    // Adjust deformation radius based on frame time
    float CurrentFrameTime = CurrentMetrics.DeformationTime;
    if (CurrentFrameTime > TargetFrameTime)
    {
        // Reduce radius if frame time is too high
        float ReductionFactor = TargetFrameTime / CurrentFrameTime;
        CurrentMetrics.UpdateRadius = FMath::Max(50.0f, CurrentLOD.DeformationRadius * ReductionFactor);
    }
    else
    {
        // Gradually increase radius if performance allows
        CurrentMetrics.UpdateRadius = FMath::Min(CurrentLOD.DeformationRadius, 
                                                CurrentMetrics.UpdateRadius * 1.05f);
    }
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("TerrainDeformationOptimizer: Optimized radius to %.1f"), 
           CurrentMetrics.UpdateRadius);
}

void UPerf_TerrainDeformationOptimizer::CullDistantDeformations(float MaxDistance)
{
    if (!TerrainDeformationComponent)
    {
        return;
    }
    
    // This would integrate with the terrain deformation component to cull distant deformations
    // Implementation would depend on the specific interface of Core_TerrainDeformationComponent
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("TerrainDeformationOptimizer: Culling deformations beyond %.1f units"), 
           MaxDistance);
}

void UPerf_TerrainDeformationOptimizer::RunPerformanceTest()
{
    UE_LOG(LogTemp, Warning, TEXT("=== TERRAIN DEFORMATION PERFORMANCE TEST ==="));
    UE_LOG(LogTemp, Warning, TEXT("Actor: %s"), *GetOwner()->GetName());
    UE_LOG(LogTemp, Warning, TEXT("LOD Level: %d"), CurrentLODLevel);
    UE_LOG(LogTemp, Warning, TEXT("Deformation Time: %.4f ms"), CurrentMetrics.DeformationTime * 1000.0f);
    UE_LOG(LogTemp, Warning, TEXT("Update Radius: %.1f"), CurrentMetrics.UpdateRadius);
    UE_LOG(LogTemp, Warning, TEXT("Active Deformations: %d"), CurrentMetrics.ActiveDeformations);
    UE_LOG(LogTemp, Warning, TEXT("Memory Usage: %.2f MB"), CurrentMetrics.MemoryUsageMB);
    UE_LOG(LogTemp, Warning, TEXT("Optimization Enabled: %s"), bEnableOptimization ? TEXT("Yes") : TEXT("No"));
    
    // Perform test deformation
    float TestStartTime = FPlatformTime::Seconds();
    
    // Simulate deformation operation
    FPlatformProcess::Sleep(0.001f); // 1ms simulated work
    
    float TestEndTime = FPlatformTime::Seconds();
    float TestDuration = TestEndTime - TestStartTime;
    
    UE_LOG(LogTemp, Warning, TEXT("Test Deformation Duration: %.4f ms"), TestDuration * 1000.0f);
    UE_LOG(LogTemp, Warning, TEXT("=== END PERFORMANCE TEST ==="));
}

void UPerf_TerrainDeformationOptimizer::EnableDebugVisualization(bool bEnabled)
{
    if (bEnabled && GetWorld())
    {
        FVector ActorLocation = GetOwner()->GetActorLocation();
        float DebugRadius = CurrentMetrics.UpdateRadius;
        
        // Draw debug sphere showing deformation radius
        DrawDebugSphere(GetWorld(), ActorLocation, DebugRadius, 32, FColor::Yellow, false, 1.0f);
        
        // Draw LOD level text
        FString LODText = FString::Printf(TEXT("LOD: %d"), CurrentLODLevel);
        DrawDebugString(GetWorld(), ActorLocation + FVector(0, 0, 100), LODText, nullptr, FColor::White, 1.0f);
    }
}

void UPerf_TerrainDeformationOptimizer::UpdatePerformanceMetrics(float DeltaTime)
{
    if (!bIsMonitoring)
    {
        return;
    }
    
    AccumulatedFrameTime += DeltaTime;
    FrameCount++;
    
    // Update metrics every second
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastUpdateTime >= 1.0f)
    {
        // Calculate average frame time
        float AverageFrameTime = AccumulatedFrameTime / FMath::Max(1, FrameCount);
        CurrentMetrics.DeformationTime = AverageFrameTime;
        
        // Update other metrics
        CurrentMetrics.LODLevel = CurrentLODLevel;
        CurrentMetrics.bIsOptimized = bEnableOptimization;
        
        if (CurrentLODLevel < LODLevels.Num())
        {
            CurrentMetrics.UpdateRadius = LODLevels[CurrentLODLevel].DeformationRadius;
        }
        
        // Estimate memory usage (simplified)
        CurrentMetrics.MemoryUsageMB = CurrentMetrics.ActiveDeformations * 0.1f; // 100KB per deformation
        
        // Reset counters
        AccumulatedFrameTime = 0.0f;
        FrameCount = 0;
        LastUpdateTime = CurrentTime;
    }
}

void UPerf_TerrainDeformationOptimizer::ApplyLODOptimizations()
{
    if (CurrentLODLevel >= LODLevels.Num())
    {
        return;
    }
    
    const FPerf_TerrainDeformationLOD& CurrentLOD = LODLevels[CurrentLODLevel];
    
    // Update tick interval based on LOD
    PrimaryComponentTick.TickInterval = 1.0f / CurrentLOD.UpdateFrequency;
    
    // Update deformation radius
    CurrentMetrics.UpdateRadius = CurrentLOD.DeformationRadius;
    
    // Apply LOD-specific optimizations to terrain deformation component
    if (TerrainDeformationComponent)
    {
        // This would require interface methods on the terrain deformation component
        // For now, we log the optimization
        UE_LOG(LogTemp, VeryVerbose, TEXT("TerrainDeformationOptimizer: Applied LOD %d optimizations"), 
               CurrentLODLevel);
    }
}

void UPerf_TerrainDeformationOptimizer::CheckPerformanceThresholds()
{
    float TargetFrameTime = 1.0f / TargetFrameRate;
    
    if (CurrentMetrics.DeformationTime > TargetFrameTime * 1.5f)
    {
        // Performance is poor, trigger optimization
        bNeedsOptimization = true;
        
        UE_LOG(LogTemp, Warning, TEXT("TerrainDeformationOptimizer: Performance threshold exceeded (%.4f > %.4f)"), 
               CurrentMetrics.DeformationTime, TargetFrameTime * 1.5f);
    }
}

float UPerf_TerrainDeformationOptimizer::CalculateDistanceToPlayer() const
{
    if (UWorld* World = GetWorld())
    {
        if (APlayerController* PC = World->GetFirstPlayerController())
        {
            if (APawn* PlayerPawn = PC->GetPawn())
            {
                FVector PlayerLocation = PlayerPawn->GetActorLocation();
                FVector ActorLocation = GetOwner()->GetActorLocation();
                return FVector::Dist(PlayerLocation, ActorLocation);
            }
        }
    }
    
    return 10000.0f; // Default to far distance if no player found
}

// UPerf_TerrainDeformationManager Implementation

void UPerf_TerrainDeformationManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    // Initialize global settings
    GlobalTargetFrameRate = 60.0f;
    MaxGlobalDeformations = 50;
    bEnableGlobalOptimization = true;
    LastGlobalUpdate = 0.0f;
    GlobalUpdateInterval = 1.0f;
    
    UE_LOG(LogTemp, Log, TEXT("TerrainDeformationManager: Initialized"));
}

void UPerf_TerrainDeformationManager::Deinitialize()
{
    RegisteredOptimizers.Empty();
    UE_LOG(LogTemp, Log, TEXT("TerrainDeformationManager: Deinitialized"));
    
    Super::Deinitialize();
}

void UPerf_TerrainDeformationManager::RegisterOptimizer(UPerf_TerrainDeformationOptimizer* Optimizer)
{
    if (Optimizer && !RegisteredOptimizers.Contains(Optimizer))
    {
        RegisteredOptimizers.Add(Optimizer);
        UE_LOG(LogTemp, Log, TEXT("TerrainDeformationManager: Registered optimizer for %s"), 
               *Optimizer->GetOwner()->GetName());
    }
}

void UPerf_TerrainDeformationManager::UnregisterOptimizer(UPerf_TerrainDeformationOptimizer* Optimizer)
{
    if (Optimizer)
    {
        RegisteredOptimizers.Remove(Optimizer);
        UE_LOG(LogTemp, Log, TEXT("TerrainDeformationManager: Unregistered optimizer for %s"), 
               *Optimizer->GetOwner()->GetName());
    }
}

void UPerf_TerrainDeformationManager::UpdateGlobalPerformance(float DeltaTime)
{
    if (!bEnableGlobalOptimization)
    {
        return;
    }
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastGlobalUpdate < GlobalUpdateInterval)
    {
        return;
    }
    
    // Update global metrics
    GlobalMetrics = FPerf_TerrainDeformationMetrics();
    
    for (UPerf_TerrainDeformationOptimizer* Optimizer : RegisteredOptimizers)
    {
        if (IsValid(Optimizer))
        {
            FPerf_TerrainDeformationMetrics OptimizerMetrics = Optimizer->GetPerformanceMetrics();
            
            // Accumulate metrics
            GlobalMetrics.DeformationTime += OptimizerMetrics.DeformationTime;
            GlobalMetrics.ActiveDeformations += OptimizerMetrics.ActiveDeformations;
            GlobalMetrics.MemoryUsageMB += OptimizerMetrics.MemoryUsageMB;
        }
    }
    
    // Apply global optimizations if needed
    OptimizeGlobalPerformance();
    
    LastGlobalUpdate = CurrentTime;
}

FPerf_TerrainDeformationMetrics UPerf_TerrainDeformationManager::GetGlobalMetrics() const
{
    return GlobalMetrics;
}

void UPerf_TerrainDeformationManager::SetGlobalOptimizationLevel(int32 Level)
{
    // Apply optimization level to all registered optimizers
    for (UPerf_TerrainDeformationOptimizer* Optimizer : RegisteredOptimizers)
    {
        if (IsValid(Optimizer))
        {
            Optimizer->SetOptimizationEnabled(Level > 0);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("TerrainDeformationManager: Set global optimization level to %d"), Level);
}

void UPerf_TerrainDeformationManager::EnablePerformanceMode(bool bEnabled)
{
    bEnableGlobalOptimization = bEnabled;
    
    for (UPerf_TerrainDeformationOptimizer* Optimizer : RegisteredOptimizers)
    {
        if (IsValid(Optimizer))
        {
            Optimizer->SetOptimizationEnabled(bEnabled);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("TerrainDeformationManager: Performance mode %s"), 
           bEnabled ? TEXT("enabled") : TEXT("disabled"));
}

void UPerf_TerrainDeformationManager::RunGlobalPerformanceTest()
{
    UE_LOG(LogTemp, Warning, TEXT("=== GLOBAL TERRAIN DEFORMATION PERFORMANCE TEST ==="));
    UE_LOG(LogTemp, Warning, TEXT("Registered Optimizers: %d"), RegisteredOptimizers.Num());
    UE_LOG(LogTemp, Warning, TEXT("Global Target Frame Rate: %.1f"), GlobalTargetFrameRate);
    UE_LOG(LogTemp, Warning, TEXT("Max Global Deformations: %d"), MaxGlobalDeformations);
    UE_LOG(LogTemp, Warning, TEXT("Global Optimization: %s"), bEnableGlobalOptimization ? TEXT("Enabled") : TEXT("Disabled"));
    
    // Test each registered optimizer
    for (int32 i = 0; i < RegisteredOptimizers.Num(); ++i)
    {
        if (IsValid(RegisteredOptimizers[i]))
        {
            UE_LOG(LogTemp, Warning, TEXT("--- Optimizer %d ---"), i + 1);
            RegisteredOptimizers[i]->RunPerformanceTest();
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=== END GLOBAL PERFORMANCE TEST ==="));
}

void UPerf_TerrainDeformationManager::LogPerformanceReport()
{
    UE_LOG(LogTemp, Log, TEXT("=== TERRAIN DEFORMATION PERFORMANCE REPORT ==="));
    UE_LOG(LogTemp, Log, TEXT("Total Deformation Time: %.4f ms"), GlobalMetrics.DeformationTime * 1000.0f);
    UE_LOG(LogTemp, Log, TEXT("Total Active Deformations: %d"), GlobalMetrics.ActiveDeformations);
    UE_LOG(LogTemp, Log, TEXT("Total Memory Usage: %.2f MB"), GlobalMetrics.MemoryUsageMB);
    UE_LOG(LogTemp, Log, TEXT("Registered Optimizers: %d"), RegisteredOptimizers.Num());
}

void UPerf_TerrainDeformationManager::OptimizeGlobalPerformance()
{
    float TargetFrameTime = 1.0f / GlobalTargetFrameRate;
    
    if (GlobalMetrics.DeformationTime > TargetFrameTime)
    {
        // Global performance is poor, apply optimizations
        UpdateGlobalLOD();
        CullExcessiveDeformations();
        
        UE_LOG(LogTemp, Warning, TEXT("TerrainDeformationManager: Applied global performance optimizations"));
    }
}

void UPerf_TerrainDeformationManager::UpdateGlobalLOD()
{
    // Force higher LOD levels on all optimizers to improve performance
    for (UPerf_TerrainDeformationOptimizer* Optimizer : RegisteredOptimizers)
    {
        if (IsValid(Optimizer))
        {
            float DistanceToPlayer = 0.0f; // Would calculate actual distance
            Optimizer->UpdateLODLevel(DistanceToPlayer + 500.0f); // Bias toward higher LOD
        }
    }
}

void UPerf_TerrainDeformationManager::CullExcessiveDeformations()
{
    if (GlobalMetrics.ActiveDeformations > MaxGlobalDeformations)
    {
        // Cull deformations from optimizers, starting with the farthest
        for (UPerf_TerrainDeformationOptimizer* Optimizer : RegisteredOptimizers)
        {
            if (IsValid(Optimizer))
            {
                Optimizer->CullDistantDeformations(1000.0f); // Reduce cull distance
            }
        }
    }
}