#include "PerformanceOptimizer.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "HAL/PlatformMemory.h"
#include "Stats/Stats.h"
#include "RenderingThread.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameViewportClient.h"

// Initialize static instance
UPerformanceOptimizer* UPerformanceOptimizer::Instance = nullptr;

UPerformanceOptimizer::UPerformanceOptimizer()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
    
    // Default settings optimized for 60fps PC target
    TargetFrameRate = 60.0f;
    CurrentPerformanceLevel = EPerformanceLevel::Ultra;
    bAutoScalingEnabled = true;
    bPhysicsLODEnabled = true;
    MaxActivePhysicsObjects = 100;
    PhysicsLODDistance = 2000.0f; // 20 meters
    
    TimeSinceLastOptimization = 0.0f;
    
    // Initialize frame time history
    FrameTimeHistory.Reserve(MaxFrameTimeHistory);
}

void UPerformanceOptimizer::BeginPlay()
{
    Super::BeginPlay();
    
    // Set singleton instance
    Instance = this;
    
    InitializeOptimizer();
    
    UE_LOG(LogTemp, Log, TEXT("PerformanceOptimizer: System initialized with target %f FPS"), TargetFrameRate);
}

void UPerformanceOptimizer::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // Clear singleton instance
    if (Instance == this)
    {
        Instance = nullptr;
    }
    
    Super::EndPlay(EndPlayReason);
}

void UPerformanceOptimizer::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update performance metrics
    UpdatePerformanceMetrics(DeltaTime);
    
    // Check if we need to optimize
    TimeSinceLastOptimization += DeltaTime;
    if (TimeSinceLastOptimization >= OptimizationInterval)
    {
        if (bAutoScalingEnabled)
        {
            AnalyzeAndOptimize();
        }
        TimeSinceLastOptimization = 0.0f;
    }
}

void UPerformanceOptimizer::InitializeOptimizer()
{
    UE_LOG(LogTemp, Log, TEXT("PerformanceOptimizer: Initializing performance optimization system"));
    
    // Set initial physics settings for optimal performance
    if (UPhysicsSettings* PhysicsSettings = UPhysicsSettings::Get())
    {
        // Optimize physics timestep for target framerate
        float OptimalSubstepTime = 1.0f / (TargetFrameRate * 2.0f); // Half of target frame time
        PhysicsSettings->MaxSubstepDeltaTime = OptimalSubstepTime;
        PhysicsSettings->MaxSubsteps = 6;
        PhysicsSettings->bSyncSceneQueryAndPhysicsSimulation = true;
        
        UE_LOG(LogTemp, Log, TEXT("PerformanceOptimizer: Physics settings optimized"));
    }
    
    // Initialize performance level based on current hardware
    SetPerformanceLevel(EPerformanceLevel::Ultra);
}

FPerformanceMetrics UPerformanceOptimizer::GetPerformanceMetrics() const
{
    return CurrentMetrics;
}

void UPerformanceOptimizer::SetTargetFrameRate(float TargetFPS)
{
    TargetFrameRate = FMath::Clamp(TargetFPS, 15.0f, 120.0f);
    UE_LOG(LogTemp, Log, TEXT("PerformanceOptimizer: Target frame rate set to %f FPS"), TargetFrameRate);
}

void UPerformanceOptimizer::SetPerformanceLevel(EPerformanceLevel Level)
{
    if (CurrentPerformanceLevel != Level)
    {
        CurrentPerformanceLevel = Level;
        AdjustRenderingQuality(Level);
        
        UE_LOG(LogTemp, Log, TEXT("PerformanceOptimizer: Performance level changed to %d"), (int32)Level);
    }
}

void UPerformanceOptimizer::SetAutoScalingEnabled(bool bEnabled)
{
    bAutoScalingEnabled = bEnabled;
    UE_LOG(LogTemp, Log, TEXT("PerformanceOptimizer: Auto-scaling %s"), bEnabled ? TEXT("enabled") : TEXT("disabled"));
}

void UPerformanceOptimizer::OptimizePhysicsObjects()
{
    if (!bPhysicsLODEnabled)
    {
        return;
    }
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Get player location for distance calculations
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
    FVector PlayerLocation = PlayerPawn ? PlayerPawn->GetActorLocation() : FVector::ZeroVector;
    
    // Find all physics objects and optimize based on distance
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AStaticMeshActor::StaticClass(), AllActors);
    
    int32 OptimizedCount = 0;
    int32 ActivePhysicsCount = 0;
    
    for (AActor* Actor : AllActors)
    {
        if (AStaticMeshActor* MeshActor = Cast<AStaticMeshActor>(Actor))
        {
            UStaticMeshComponent* MeshComp = MeshActor->GetStaticMeshComponent();
            if (MeshComp && MeshComp->IsSimulatingPhysics())
            {
                float Distance = FVector::Dist(Actor->GetActorLocation(), PlayerLocation);
                
                // Apply LOD based on distance
                if (Distance > PhysicsLODDistance)
                {
                    // Disable physics for very distant objects
                    MeshComp->SetSimulatePhysics(false);
                    MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
                    OptimizedCount++;
                }
                else if (Distance > PhysicsLODDistance * 0.5f)
                {
                    // Reduce physics quality for medium distance objects
                    MeshComp->SetMassOverrideInKg(NAME_None, 5.0f, true);
                }
                else
                {
                    // Full physics for nearby objects
                    ActivePhysicsCount++;
                }
            }
        }
    }
    
    CurrentMetrics.ActivePhysicsObjects = ActivePhysicsCount;
    
    UE_LOG(LogTemp, Log, TEXT("PerformanceOptimizer: Optimized %d physics objects, %d remain active"), 
           OptimizedCount, ActivePhysicsCount);
}

void UPerformanceOptimizer::AdjustRenderingQuality(EPerformanceLevel Level)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Adjust shadow quality
    AdjustShadowQuality(Level);
    
    // Adjust LOD bias based on performance level
    float LODBias = 0.0f;
    float ViewDistanceScale = 1.0f;
    
    switch (Level)
    {
        case EPerformanceLevel::Ultra:
            LODBias = 0.0f;
            ViewDistanceScale = 1.0f;
            break;
        case EPerformanceLevel::High:
            LODBias = 0.5f;
            ViewDistanceScale = 0.9f;
            break;
        case EPerformanceLevel::Medium:
            LODBias = 1.0f;
            ViewDistanceScale = 0.8f;
            break;
        case EPerformanceLevel::Low:
            LODBias = 1.5f;
            ViewDistanceScale = 0.7f;
            break;
        case EPerformanceLevel::Emergency:
            LODBias = 2.0f;
            ViewDistanceScale = 0.5f;
            break;
    }
    
    // Apply console commands for immediate effect
    if (UGameViewportClient* ViewportClient = World->GetGameViewport())
    {
        ViewportClient->ConsoleCommand(FString::Printf(TEXT("r.StaticMeshLODBias %f"), LODBias));
        ViewportClient->ConsoleCommand(FString::Printf(TEXT("r.SkeletalMeshLODBias %f"), LODBias));
        ViewportClient->ConsoleCommand(FString::Printf(TEXT("r.ViewDistanceScale %f"), ViewDistanceScale));
    }
    
    UE_LOG(LogTemp, Log, TEXT("PerformanceOptimizer: Rendering quality adjusted for level %d"), (int32)Level);
}

void UPerformanceOptimizer::ForceMemoryCleanup()
{
    // Force garbage collection
    GEngine->ForceGarbageCollection(true);
    
    // Clear texture streaming pool
    if (UGameViewportClient* ViewportClient = GetWorld()->GetGameViewport())
    {
        ViewportClient->ConsoleCommand(TEXT("r.Streaming.PoolSize 0"));
        ViewportClient->ConsoleCommand(TEXT("r.Streaming.PoolSize 1000"));
    }
    
    UE_LOG(LogTemp, Log, TEXT("PerformanceOptimizer: Memory cleanup completed"));
}

FString UPerformanceOptimizer::GeneratePerformanceReport() const
{
    FString Report;
    Report += TEXT("=== PERFORMANCE OPTIMIZATION REPORT ===\n");
    Report += FString::Printf(TEXT("Target FPS: %.1f\n"), TargetFrameRate);
    Report += FString::Printf(TEXT("Current FPS: %.1f\n"), CurrentMetrics.CurrentFPS);
    Report += FString::Printf(TEXT("Frame Time: %.2f ms\n"), CurrentMetrics.FrameTimeMS);
    Report += FString::Printf(TEXT("Game Thread: %.2f ms\n"), CurrentMetrics.GameThreadTimeMS);
    Report += FString::Printf(TEXT("Render Thread: %.2f ms\n"), CurrentMetrics.RenderThreadTimeMS);
    Report += FString::Printf(TEXT("Physics Time: %.2f ms\n"), CurrentMetrics.PhysicsTimeMS);
    Report += FString::Printf(TEXT("Memory Usage: %.1f MB\n"), CurrentMetrics.MemoryUsageMB);
    Report += FString::Printf(TEXT("Active Physics Objects: %d\n"), CurrentMetrics.ActivePhysicsObjects);
    Report += FString::Printf(TEXT("Performance Level: %d\n"), (int32)CurrentPerformanceLevel);
    Report += FString::Printf(TEXT("Performance Score: %.1f/100\n"), CalculatePerformanceScore());
    Report += TEXT("==========================================");
    
    return Report;
}

UPerformanceOptimizer* UPerformanceOptimizer::GetPerformanceOptimizer(UWorld* World)
{
    if (Instance)
    {
        return Instance;
    }
    
    // Search for existing instance in world
    if (World)
    {
        for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
        {
            AActor* Actor = *ActorIterator;
            if (UPerformanceOptimizer* Optimizer = Actor->FindComponentByClass<UPerformanceOptimizer>())
            {
                Instance = Optimizer;
                return Instance;
            }
        }
    }
    
    return nullptr;
}

void UPerformanceOptimizer::UpdatePerformanceMetrics(float DeltaTime)
{
    // Calculate current FPS
    CurrentMetrics.CurrentFPS = 1.0f / DeltaTime;
    CurrentMetrics.FrameTimeMS = DeltaTime * 1000.0f;
    
    // Update frame time history
    FrameTimeHistory.Add(DeltaTime);
    if (FrameTimeHistory.Num() > MaxFrameTimeHistory)
    {
        FrameTimeHistory.RemoveAt(0);
    }
    
    // Get memory usage
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    CurrentMetrics.MemoryUsageMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);
    
    // Update thread times (simplified - would need more complex implementation for accurate measurement)
    CurrentMetrics.GameThreadTimeMS = DeltaTime * 1000.0f * 0.6f; // Estimated
    CurrentMetrics.RenderThreadTimeMS = DeltaTime * 1000.0f * 0.3f; // Estimated
    CurrentMetrics.PhysicsTimeMS = DeltaTime * 1000.0f * 0.1f; // Estimated
}

void UPerformanceOptimizer::AnalyzeAndOptimize()
{
    // Calculate average FPS from recent history
    float AverageFPS = 0.0f;
    if (FrameTimeHistory.Num() > 0)
    {
        float TotalTime = 0.0f;
        for (float FrameTime : FrameTimeHistory)
        {
            TotalTime += FrameTime;
        }
        AverageFPS = FrameTimeHistory.Num() / TotalTime;
    }
    else
    {
        AverageFPS = CurrentMetrics.CurrentFPS;
    }
    
    // Determine appropriate performance level
    EPerformanceLevel NewLevel = CurrentPerformanceLevel;
    
    if (AverageFPS >= UltraFPSThreshold)
    {
        NewLevel = EPerformanceLevel::Ultra;
    }
    else if (AverageFPS >= HighFPSThreshold)
    {
        NewLevel = EPerformanceLevel::High;
    }
    else if (AverageFPS >= MediumFPSThreshold)
    {
        NewLevel = EPerformanceLevel::Medium;
    }
    else if (AverageFPS >= LowFPSThreshold)
    {
        NewLevel = EPerformanceLevel::Low;
    }
    else
    {
        NewLevel = EPerformanceLevel::Emergency;
    }
    
    // Apply optimizations if performance level changed
    if (NewLevel != CurrentPerformanceLevel)
    {
        SetPerformanceLevel(NewLevel);
        OptimizePhysicsObjects();
        
        // Force memory cleanup if performance is critical
        if (NewLevel == EPerformanceLevel::Emergency)
        {
            ForceMemoryCleanup();
        }
    }
}

void UPerformanceOptimizer::ApplyPhysicsLOD()
{
    OptimizePhysicsObjects();
}

void UPerformanceOptimizer::AdjustShadowQuality(EPerformanceLevel Level)
{
    UWorld* World = GetWorld();
    if (!World || !World->GetGameViewport())
    {
        return;
    }
    
    int32 ShadowQuality = 3;
    int32 ShadowResolution = 2048;
    
    switch (Level)
    {
        case EPerformanceLevel::Ultra:
            ShadowQuality = 3;
            ShadowResolution = 2048;
            break;
        case EPerformanceLevel::High:
            ShadowQuality = 2;
            ShadowResolution = 1024;
            break;
        case EPerformanceLevel::Medium:
            ShadowQuality = 1;
            ShadowResolution = 512;
            break;
        case EPerformanceLevel::Low:
        case EPerformanceLevel::Emergency:
            ShadowQuality = 0;
            ShadowResolution = 256;
            break;
    }
    
    UGameViewportClient* ViewportClient = World->GetGameViewport();
    ViewportClient->ConsoleCommand(FString::Printf(TEXT("r.ShadowQuality %d"), ShadowQuality));
    ViewportClient->ConsoleCommand(FString::Printf(TEXT("r.Shadow.MaxResolution %d"), ShadowResolution));
}

void UPerformanceOptimizer::OptimizeTextureStreaming()
{
    UWorld* World = GetWorld();
    if (!World || !World->GetGameViewport())
    {
        return;
    }
    
    // Adjust texture streaming based on memory pressure
    float MemoryPressure = CurrentMetrics.MemoryUsageMB / 8192.0f; // Assume 8GB target
    
    if (MemoryPressure > 0.8f)
    {
        // High memory pressure - reduce texture quality
        World->GetGameViewport()->ConsoleCommand(TEXT("r.Streaming.MipBias 2"));
        World->GetGameViewport()->ConsoleCommand(TEXT("r.Streaming.PoolSize 512"));
    }
    else if (MemoryPressure > 0.6f)
    {
        // Medium memory pressure
        World->GetGameViewport()->ConsoleCommand(TEXT("r.Streaming.MipBias 1"));
        World->GetGameViewport()->ConsoleCommand(TEXT("r.Streaming.PoolSize 1024"));
    }
    else
    {
        // Low memory pressure - full quality
        World->GetGameViewport()->ConsoleCommand(TEXT("r.Streaming.MipBias 0"));
        World->GetGameViewport()->ConsoleCommand(TEXT("r.Streaming.PoolSize 2048"));
    }
}

float UPerformanceOptimizer::CalculatePerformanceScore() const
{
    // Calculate performance score based on multiple factors
    float FPSScore = FMath::Clamp(CurrentMetrics.CurrentFPS / TargetFrameRate, 0.0f, 1.0f) * 40.0f;
    float FrameTimeScore = FMath::Clamp(1.0f - (CurrentMetrics.FrameTimeMS / 33.33f), 0.0f, 1.0f) * 30.0f;
    float MemoryScore = FMath::Clamp(1.0f - (CurrentMetrics.MemoryUsageMB / 8192.0f), 0.0f, 1.0f) * 20.0f;
    float PhysicsScore = FMath::Clamp(1.0f - (CurrentMetrics.PhysicsTimeMS / 5.0f), 0.0f, 1.0f) * 10.0f;
    
    return FPSScore + FrameTimeScore + MemoryScore + PhysicsScore;
}