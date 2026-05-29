#include "Perf_DynamicPerformanceManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "Misc/CoreStats.h"
#include "Stats/Stats.h"
#include "RenderingThread.h"
#include "HAL/PlatformApplicationMisc.h"
#include "Engine/GameViewportClient.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/BodyInstance.h"
#include "Engine/StaticMeshActor.h"
#include "GameFramework/Pawn.h"

APerf_DynamicPerformanceManager::APerf_DynamicPerformanceManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Update every 100ms
    
    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;
    
    // Initialize default values
    CurrentPerformanceLevel = EPerf_PerformanceLevel::High;
    PerformanceUpdateInterval = 1.0f;
    TargetFPS = 60.0f;
    MinimumFPS = 30.0f;
    bAutoOptimize = true;
    bEnablePerformanceLogging = true;
    
    LastUpdateTime = 0.0f;
    FrameTimeAccumulator = 0.0f;
    FrameCount = 0;
    
    // Initialize frame time history
    FrameTimeHistory.Reserve(MaxFrameHistorySize);
    
    // Initialize optimization settings
    OptimizationSettings.MaxViewDistance = 15000.0f;
    OptimizationSettings.LODDistanceScale = 1.0f;
    OptimizationSettings.MaxPhysicsObjects = 100;
    OptimizationSettings.ShadowResolutionScale = 1.0f;
    OptimizationSettings.bEnableDistanceCulling = true;
    OptimizationSettings.bOptimizePhysics = true;
}

void APerf_DynamicPerformanceManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Apply initial performance settings
    ApplyPerformanceSettings();
    
    // Log initialization
    if (bEnablePerformanceLogging)
    {
        UE_LOG(LogTemp, Warning, TEXT("Dynamic Performance Manager initialized - Target FPS: %.1f"), TargetFPS);
    }
}

void APerf_DynamicPerformanceManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
    
    if (bEnablePerformanceLogging)
    {
        UE_LOG(LogTemp, Warning, TEXT("Dynamic Performance Manager shutdown"));
    }
}

void APerf_DynamicPerformanceManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update frame time tracking
    UpdateFrameTimeHistory(DeltaTime);
    
    // Check if it's time to update performance metrics
    LastUpdateTime += DeltaTime;
    if (LastUpdateTime >= PerformanceUpdateInterval)
    {
        UpdatePerformanceMetrics();
        CheckPerformanceThresholds();
        
        if (bAutoOptimize)
        {
            AutoOptimizeIfNeeded();
        }
        
        LastUpdateTime = 0.0f;
    }
}

void APerf_DynamicPerformanceManager::UpdatePerformanceMetrics()
{
    // Calculate current FPS
    float CurrentFrameTime = CalculateAverageFrameTime();
    CurrentMetrics.CurrentFPS = (CurrentFrameTime > 0.0f) ? (1000.0f / CurrentFrameTime) : 0.0f;
    CurrentMetrics.AverageFrameTime = CurrentFrameTime;
    
    // Get GPU frame time (approximation)
    CurrentMetrics.GPUFrameTime = CurrentFrameTime * 0.6f; // Rough estimate
    
    // Get memory usage
    CurrentMetrics.MemoryUsageMB = GetMemoryUsageMB();
    
    // Count visible actors
    if (UWorld* World = GetWorld())
    {
        CurrentMetrics.VisibleActors = 0;
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor && !Actor->IsHidden())
            {
                CurrentMetrics.VisibleActors++;
            }
        }
    }
    
    // Estimate draw calls (rough approximation)
    CurrentMetrics.DrawCalls = CurrentMetrics.VisibleActors * 2; // Rough estimate
    
    // Log performance metrics if enabled
    if (bEnablePerformanceLogging)
    {
        UE_LOG(LogTemp, Log, TEXT("Performance Metrics - FPS: %.1f, Frame Time: %.2fms, Memory: %.1fMB, Visible Actors: %d"), 
               CurrentMetrics.CurrentFPS, CurrentMetrics.AverageFrameTime, CurrentMetrics.MemoryUsageMB, CurrentMetrics.VisibleActors);
    }
}

void APerf_DynamicPerformanceManager::SetPerformanceLevel(EPerf_PerformanceLevel NewLevel)
{
    if (CurrentPerformanceLevel != NewLevel)
    {
        CurrentPerformanceLevel = NewLevel;
        ApplyPerformanceSettings();
        
        if (bEnablePerformanceLogging)
        {
            UE_LOG(LogTemp, Warning, TEXT("Performance level changed to: %d"), (int32)NewLevel);
        }
    }
}

void APerf_DynamicPerformanceManager::OptimizeScene()
{
    if (!GetWorld()) return;
    
    // Apply LOD optimizations
    ApplyLODOptimizations();
    
    // Optimize physics objects
    OptimizePhysicsObjects();
    
    // Optimize lighting
    OptimizeLighting();
    
    if (bEnablePerformanceLogging)
    {
        UE_LOG(LogTemp, Warning, TEXT("Scene optimization applied"));
    }
}

void APerf_DynamicPerformanceManager::OptimizePhysicsObjects()
{
    if (!GetWorld() || !OptimizationSettings.bOptimizePhysics) return;
    
    int32 PhysicsObjectCount = 0;
    
    // Count and potentially disable physics on distant objects
    for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor) continue;
        
        // Check static mesh components
        if (UStaticMeshComponent* MeshComp = Actor->FindComponentByClass<UStaticMeshComponent>())
        {
            if (MeshComp->IsSimulatingPhysics())
            {
                PhysicsObjectCount++;
                
                // If we have too many physics objects, disable physics on distant ones
                if (PhysicsObjectCount > OptimizationSettings.MaxPhysicsObjects)
                {
                    float DistanceToPlayer = 10000.0f; // Default large distance
                    
                    if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
                    {
                        DistanceToPlayer = FVector::Dist(Actor->GetActorLocation(), PlayerPawn->GetActorLocation());
                    }
                    
                    if (DistanceToPlayer > OptimizationSettings.MaxViewDistance * 0.5f)
                    {
                        MeshComp->SetSimulatePhysics(false);
                        PhysicsObjectCount--;
                    }
                }
            }
        }
    }
    
    if (bEnablePerformanceLogging)
    {
        UE_LOG(LogTemp, Log, TEXT("Physics optimization complete - Active physics objects: %d"), PhysicsObjectCount);
    }
}

void APerf_DynamicPerformanceManager::OptimizeLighting()
{
    // Apply shadow resolution scaling
    if (IConsoleVariable* ShadowResolutionCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.Shadow.MaxResolution")))
    {
        int32 BaseShadowResolution = 2048;
        int32 ScaledResolution = FMath::RoundToInt(BaseShadowResolution * OptimizationSettings.ShadowResolutionScale);
        ShadowResolutionCVar->Set(ScaledResolution);
    }
    
    // Adjust shadow distance based on performance level
    if (IConsoleVariable* ShadowDistanceCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.Shadow.DistanceScale")))
    {
        float ShadowDistanceScale = 1.0f;
        switch (CurrentPerformanceLevel)
        {
            case EPerf_PerformanceLevel::Ultra:
                ShadowDistanceScale = 1.0f;
                break;
            case EPerf_PerformanceLevel::High:
                ShadowDistanceScale = 0.8f;
                break;
            case EPerf_PerformanceLevel::Medium:
                ShadowDistanceScale = 0.6f;
                break;
            case EPerf_PerformanceLevel::Low:
                ShadowDistanceScale = 0.4f;
                break;
            case EPerf_PerformanceLevel::Potato:
                ShadowDistanceScale = 0.2f;
                break;
        }
        ShadowDistanceCVar->Set(ShadowDistanceScale);
    }
}

void APerf_DynamicPerformanceManager::ApplyLODOptimizations()
{
    // Set LOD distance scale based on performance level
    if (IConsoleVariable* LODDistanceCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.StaticMeshLODDistanceScale")))
    {
        float LODScale = OptimizationSettings.LODDistanceScale;
        
        switch (CurrentPerformanceLevel)
        {
            case EPerf_PerformanceLevel::Ultra:
                LODScale *= 1.2f;
                break;
            case EPerf_PerformanceLevel::High:
                LODScale *= 1.0f;
                break;
            case EPerf_PerformanceLevel::Medium:
                LODScale *= 0.8f;
                break;
            case EPerf_PerformanceLevel::Low:
                LODScale *= 0.6f;
                break;
            case EPerf_PerformanceLevel::Potato:
                LODScale *= 0.4f;
                break;
        }
        
        LODDistanceCVar->Set(LODScale);
    }
    
    // Apply skeletal mesh LOD scaling
    if (IConsoleVariable* SkeletalLODCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.SkeletalMeshLODBias")))
    {
        int32 LODBias = 0;
        switch (CurrentPerformanceLevel)
        {
            case EPerf_PerformanceLevel::Ultra:
                LODBias = -1;
                break;
            case EPerf_PerformanceLevel::High:
                LODBias = 0;
                break;
            case EPerf_PerformanceLevel::Medium:
                LODBias = 1;
                break;
            case EPerf_PerformanceLevel::Low:
                LODBias = 2;
                break;
            case EPerf_PerformanceLevel::Potato:
                LODBias = 3;
                break;
        }
        SkeletalLODCVar->Set(LODBias);
    }
}

void APerf_DynamicPerformanceManager::ForceGarbageCollection()
{
    GEngine->ForceGarbageCollection(true);
    
    if (bEnablePerformanceLogging)
    {
        UE_LOG(LogTemp, Warning, TEXT("Forced garbage collection"));
    }
}

float APerf_DynamicPerformanceManager::GetMemoryUsageMB() const
{
    // Get memory stats
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    return MemStats.UsedPhysical / (1024.0f * 1024.0f);
}

void APerf_DynamicPerformanceManager::ExecutePerformanceCommand(const FString& Command)
{
    if (IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(*Command))
    {
        UE_LOG(LogTemp, Log, TEXT("Executing performance command: %s"), *Command);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Performance command not found: %s"), *Command);
    }
}

void APerf_DynamicPerformanceManager::ApplyPerformanceSettings()
{
    // Apply view distance settings
    if (IConsoleVariable* ViewDistanceCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.ViewDistanceScale")))
    {
        float ViewDistanceScale = OptimizationSettings.MaxViewDistance / 15000.0f;
        ViewDistanceCVar->Set(ViewDistanceScale);
    }
    
    // Apply performance level specific settings
    switch (CurrentPerformanceLevel)
    {
        case EPerf_PerformanceLevel::Ultra:
            ExecutePerformanceCommand(TEXT("r.PostProcessAAQuality 6"));
            ExecutePerformanceCommand(TEXT("r.ShadowQuality 5"));
            ExecutePerformanceCommand(TEXT("r.TextureQuality 3"));
            break;
        case EPerf_PerformanceLevel::High:
            ExecutePerformanceCommand(TEXT("r.PostProcessAAQuality 4"));
            ExecutePerformanceCommand(TEXT("r.ShadowQuality 4"));
            ExecutePerformanceCommand(TEXT("r.TextureQuality 2"));
            break;
        case EPerf_PerformanceLevel::Medium:
            ExecutePerformanceCommand(TEXT("r.PostProcessAAQuality 2"));
            ExecutePerformanceCommand(TEXT("r.ShadowQuality 3"));
            ExecutePerformanceCommand(TEXT("r.TextureQuality 1"));
            break;
        case EPerf_PerformanceLevel::Low:
            ExecutePerformanceCommand(TEXT("r.PostProcessAAQuality 1"));
            ExecutePerformanceCommand(TEXT("r.ShadowQuality 2"));
            ExecutePerformanceCommand(TEXT("r.TextureQuality 0"));
            break;
        case EPerf_PerformanceLevel::Potato:
            ExecutePerformanceCommand(TEXT("r.PostProcessAAQuality 0"));
            ExecutePerformanceCommand(TEXT("r.ShadowQuality 1"));
            ExecutePerformanceCommand(TEXT("r.TextureQuality 0"));
            break;
    }
}

void APerf_DynamicPerformanceManager::UpdateFrameTimeHistory(float DeltaTime)
{
    float FrameTimeMs = DeltaTime * 1000.0f;
    
    // Add to history
    FrameTimeHistory.Add(FrameTimeMs);
    
    // Keep history size manageable
    if (FrameTimeHistory.Num() > MaxFrameHistorySize)
    {
        FrameTimeHistory.RemoveAt(0);
    }
}

float APerf_DynamicPerformanceManager::CalculateAverageFrameTime() const
{
    if (FrameTimeHistory.Num() == 0)
    {
        return 16.67f; // Default 60 FPS
    }
    
    float Sum = 0.0f;
    for (float FrameTime : FrameTimeHistory)
    {
        Sum += FrameTime;
    }
    
    return Sum / FrameTimeHistory.Num();
}

void APerf_DynamicPerformanceManager::CheckPerformanceThresholds()
{
    float CurrentFPS = CurrentMetrics.CurrentFPS;
    
    // Check if we need to automatically adjust performance level
    if (CurrentFPS < MinimumFPS && CurrentPerformanceLevel != EPerf_PerformanceLevel::Potato)
    {
        // Decrease performance level
        int32 CurrentLevelInt = (int32)CurrentPerformanceLevel;
        CurrentLevelInt = FMath::Min(CurrentLevelInt + 1, (int32)EPerf_PerformanceLevel::Potato);
        SetPerformanceLevel((EPerf_PerformanceLevel)CurrentLevelInt);
        
        if (bEnablePerformanceLogging)
        {
            UE_LOG(LogTemp, Warning, TEXT("Performance threshold breached - FPS: %.1f, Lowering performance level"), CurrentFPS);
        }
    }
    else if (CurrentFPS > TargetFPS * 1.2f && CurrentPerformanceLevel != EPerf_PerformanceLevel::Ultra)
    {
        // Increase performance level if we have headroom
        int32 CurrentLevelInt = (int32)CurrentPerformanceLevel;
        CurrentLevelInt = FMath::Max(CurrentLevelInt - 1, (int32)EPerf_PerformanceLevel::Ultra);
        SetPerformanceLevel((EPerf_PerformanceLevel)CurrentLevelInt);
        
        if (bEnablePerformanceLogging)
        {
            UE_LOG(LogTemp, Log, TEXT("Performance headroom detected - FPS: %.1f, Raising performance level"), CurrentFPS);
        }
    }
}

void APerf_DynamicPerformanceManager::AutoOptimizeIfNeeded()
{
    // Check if memory usage is too high
    if (CurrentMetrics.MemoryUsageMB > 8000.0f) // 8GB threshold
    {
        ForceGarbageCollection();
    }
    
    // Check if too many visible actors
    if (CurrentMetrics.VisibleActors > 1000)
    {
        OptimizeScene();
    }
    
    // Check frame time consistency
    if (FrameTimeHistory.Num() >= 10)
    {
        float MaxFrameTime = 0.0f;
        float MinFrameTime = 1000.0f;
        
        for (int32 i = FrameTimeHistory.Num() - 10; i < FrameTimeHistory.Num(); i++)
        {
            MaxFrameTime = FMath::Max(MaxFrameTime, FrameTimeHistory[i]);
            MinFrameTime = FMath::Min(MinFrameTime, FrameTimeHistory[i]);
        }
        
        // If frame time variance is too high, apply optimizations
        if (MaxFrameTime - MinFrameTime > 10.0f) // 10ms variance threshold
        {
            OptimizeScene();
            
            if (bEnablePerformanceLogging)
            {
                UE_LOG(LogTemp, Warning, TEXT("High frame time variance detected - applying optimizations"));
            }
        }
    }
}