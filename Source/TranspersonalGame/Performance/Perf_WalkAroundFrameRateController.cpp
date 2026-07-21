#include "Perf_WalkAroundFrameRateController.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/DirectionalLight.h"

APerf_WalkAroundFrameRateController::APerf_WalkAroundFrameRateController()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;
    PrimaryActorTick.TickInterval = 0.0f; // Tick every frame for accurate performance monitoring
    
    // Initialize default values
    TargetFrameRate = EPerf_FrameRateTarget::Target_60FPS;
    bEnableAdaptiveFrameRate = true;
    FrameRateThreshold = 55.0f;
    bEnablePerformanceLogging = true;
    LoggingInterval = 1.0f;
    
    // Initialize optimization settings
    OptimizationSettings = FPerf_WalkAroundOptimizationSettings();
    
    // Initialize metrics
    CurrentMetrics = FPerf_FrameRateMetrics();
    
    // Initialize private variables
    LastLogTime = 0.0f;
    FrameCounter = 0;
    TotalFrameTime = 0.0f;
    FPSHistory.Reserve(300); // Store 5 seconds of history at 60fps
}

void APerf_WalkAroundFrameRateController::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("WalkAroundFrameRateController: Starting performance monitoring for MILESTONE 1"));
    
    // Apply initial optimizations
    ApplyOptimizations();
    
    // Set initial frame rate target
    SetTargetFrameRate(TargetFrameRate);
}

void APerf_WalkAroundFrameRateController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update frame rate metrics
    UpdateFrameRateMetrics(DeltaTime);
    
    // Apply adaptive optimizations if enabled
    if (bEnableAdaptiveFrameRate)
    {
        if (CurrentMetrics.CurrentFPS < FrameRateThreshold)
        {
            ApplyOptimizations();
        }
    }
    
    // Log performance metrics at intervals
    if (bEnablePerformanceLogging)
    {
        LastLogTime += DeltaTime;
        if (LastLogTime >= LoggingInterval)
        {
            LogPerformanceMetrics();
            LastLogTime = 0.0f;
        }
    }
}

void APerf_WalkAroundFrameRateController::UpdateFrameRateMetrics(float DeltaTime)
{
    // Calculate current FPS
    CurrentMetrics.CurrentFPS = (DeltaTime > 0.0f) ? (1.0f / DeltaTime) : 0.0f;
    CurrentMetrics.FrameTime = DeltaTime * 1000.0f; // Convert to milliseconds
    
    // Update frame counter and total time
    FrameCounter++;
    TotalFrameTime += DeltaTime;
    
    // Add to history
    FPSHistory.Add(CurrentMetrics.CurrentFPS);
    if (FPSHistory.Num() > 300) // Keep only last 5 seconds at 60fps
    {
        FPSHistory.RemoveAt(0);
    }
    
    // Calculate average FPS
    if (FrameCounter > 0)
    {
        CurrentMetrics.AverageFPS = FrameCounter / TotalFrameTime;
    }
    
    // Update min/max FPS
    if (CurrentMetrics.CurrentFPS < CurrentMetrics.MinFPS)
    {
        CurrentMetrics.MinFPS = CurrentMetrics.CurrentFPS;
    }
    if (CurrentMetrics.CurrentFPS > CurrentMetrics.MaxFPS)
    {
        CurrentMetrics.MaxFPS = CurrentMetrics.CurrentFPS;
    }
    
    // Count dropped frames (frames below 90% of target)
    float TargetFPS = 60.0f;
    switch (TargetFrameRate)
    {
        case EPerf_FrameRateTarget::Target_30FPS:
            TargetFPS = 30.0f;
            break;
        case EPerf_FrameRateTarget::Target_60FPS:
            TargetFPS = 60.0f;
            break;
        case EPerf_FrameRateTarget::Target_120FPS:
            TargetFPS = 120.0f;
            break;
        case EPerf_FrameRateTarget::Target_Variable:
            TargetFPS = 60.0f; // Default for variable
            break;
    }
    
    if (CurrentMetrics.CurrentFPS < (TargetFPS * 0.9f))
    {
        CurrentMetrics.DroppedFrames++;
    }
    
    // Get thread times from engine stats
    CurrentMetrics.GameThreadTime = FPlatformTime::ToMilliseconds(GGameThreadTime);
    CurrentMetrics.RenderThreadTime = FPlatformTime::ToMilliseconds(GRenderThreadTime);
}

void APerf_WalkAroundFrameRateController::ApplyOptimizations()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Get player character for distance calculations
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
    if (!PlayerPawn)
    {
        return;
    }
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    // Optimize characters
    TArray<AActor*> Characters;
    UGameplayStatics::GetAllActorsOfClass(World, ACharacter::StaticClass(), Characters);
    
    int32 ActiveCharacters = 0;
    for (AActor* Actor : Characters)
    {
        if (Actor == PlayerPawn) continue; // Skip player
        
        float Distance = FVector::Dist(PlayerLocation, Actor->GetActorLocation());
        
        if (Distance > OptimizationSettings.CharacterCullingDistance || ActiveCharacters >= OptimizationSettings.MaxActiveCharacters)
        {
            Actor->SetActorHiddenInGame(true);
            Actor->SetActorTickEnabled(false);
        }
        else
        {
            Actor->SetActorHiddenInGame(false);
            Actor->SetActorTickEnabled(true);
            ActiveCharacters++;
        }
    }
    
    // Optimize static meshes (basic LOD)
    TArray<AActor*> StaticMeshActors;
    UGameplayStatics::GetAllActorsOfClass(World, AStaticMeshActor::StaticClass(), StaticMeshActors);
    
    for (AActor* Actor : StaticMeshActors)
    {
        AStaticMeshActor* MeshActor = Cast<AStaticMeshActor>(Actor);
        if (MeshActor && MeshActor->GetStaticMeshComponent())
        {
            float Distance = FVector::Dist(PlayerLocation, Actor->GetActorLocation());
            
            if (Distance > OptimizationSettings.FoliageCullingDistance)
            {
                MeshActor->SetActorHiddenInGame(true);
            }
            else
            {
                MeshActor->SetActorHiddenInGame(false);
                
                // Simple LOD based on distance
                if (OptimizationSettings.bEnableLODOptimization)
                {
                    UStaticMeshComponent* MeshComp = MeshActor->GetStaticMeshComponent();
                    if (Distance > 1000.0f * OptimizationSettings.LODDistanceMultiplier)
                    {
                        MeshComp->SetForcedLodModel(2); // Low LOD
                    }
                    else if (Distance > 500.0f * OptimizationSettings.LODDistanceMultiplier)
                    {
                        MeshComp->SetForcedLodModel(1); // Medium LOD
                    }
                    else
                    {
                        MeshComp->SetForcedLodModel(0); // High LOD
                    }
                }
            }
        }
    }
}

void APerf_WalkAroundFrameRateController::LogPerformanceMetrics()
{
    UE_LOG(LogTemp, Log, TEXT("PERFORMANCE METRICS - FPS: %.1f (Avg: %.1f, Min: %.1f, Max: %.1f) | Frame Time: %.2fms | Dropped: %d"), 
        CurrentMetrics.CurrentFPS, 
        CurrentMetrics.AverageFPS, 
        CurrentMetrics.MinFPS, 
        CurrentMetrics.MaxFPS, 
        CurrentMetrics.FrameTime, 
        CurrentMetrics.DroppedFrames);
}

void APerf_WalkAroundFrameRateController::SetTargetFrameRate(EPerf_FrameRateTarget NewTarget)
{
    TargetFrameRate = NewTarget;
    
    // Apply engine frame rate settings
    switch (NewTarget)
    {
        case EPerf_FrameRateTarget::Target_30FPS:
            GEngine->SetMaxFPS(30.0f);
            FrameRateThreshold = 27.0f;
            break;
        case EPerf_FrameRateTarget::Target_60FPS:
            GEngine->SetMaxFPS(60.0f);
            FrameRateThreshold = 55.0f;
            break;
        case EPerf_FrameRateTarget::Target_120FPS:
            GEngine->SetMaxFPS(120.0f);
            FrameRateThreshold = 110.0f;
            break;
        case EPerf_FrameRateTarget::Target_Variable:
            GEngine->SetMaxFPS(0.0f); // Unlimited
            FrameRateThreshold = 55.0f;
            break;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("WalkAroundFrameRateController: Target frame rate set to %d"), (int32)NewTarget);
}

void APerf_WalkAroundFrameRateController::EnableAdaptiveFrameRate(bool bEnable)
{
    bEnableAdaptiveFrameRate = bEnable;
    UE_LOG(LogTemp, Warning, TEXT("WalkAroundFrameRateController: Adaptive frame rate %s"), bEnable ? TEXT("enabled") : TEXT("disabled"));
}

FPerf_FrameRateMetrics APerf_WalkAroundFrameRateController::GetCurrentMetrics() const
{
    return CurrentMetrics;
}

void APerf_WalkAroundFrameRateController::ResetMetrics()
{
    CurrentMetrics = FPerf_FrameRateMetrics();
    FPSHistory.Empty();
    FrameCounter = 0;
    TotalFrameTime = 0.0f;
    LastLogTime = 0.0f;
    
    UE_LOG(LogTemp, Warning, TEXT("WalkAroundFrameRateController: Performance metrics reset"));
}

void APerf_WalkAroundFrameRateController::ApplyWalkAroundOptimizations()
{
    UE_LOG(LogTemp, Warning, TEXT("WalkAroundFrameRateController: Applying MILESTONE 1 optimizations"));
    
    // Apply specific optimizations for walk around gameplay
    OptimizationSettings.CharacterCullingDistance = 3000.0f; // Closer culling for better performance
    OptimizationSettings.MaxActiveCharacters = 5; // Fewer characters for stable FPS
    OptimizationSettings.DinosaurCullingDistance = 5000.0f; // Keep dinosaurs visible longer
    OptimizationSettings.MaxActiveDinosaurs = 3; // Limit active dinosaurs
    OptimizationSettings.FoliageCullingDistance = 2000.0f; // Aggressive foliage culling
    OptimizationSettings.bEnablePhysicsOptimization = true;
    OptimizationSettings.bEnableLODOptimization = true;
    OptimizationSettings.LODDistanceMultiplier = 0.8f; // More aggressive LOD
    
    ApplyOptimizations();
}

void APerf_WalkAroundFrameRateController::SetOptimizationLevel(EOptimizationLevel Level)
{
    switch (Level)
    {
        case EOptimizationLevel::Disabled:
            OptimizationSettings.CharacterCullingDistance = 10000.0f;
            OptimizationSettings.MaxActiveCharacters = 50;
            OptimizationSettings.bEnablePhysicsOptimization = false;
            OptimizationSettings.bEnableLODOptimization = false;
            break;
        case EOptimizationLevel::Low:
            OptimizationSettings.CharacterCullingDistance = 8000.0f;
            OptimizationSettings.MaxActiveCharacters = 20;
            OptimizationSettings.bEnablePhysicsOptimization = true;
            OptimizationSettings.bEnableLODOptimization = true;
            OptimizationSettings.LODDistanceMultiplier = 1.2f;
            break;
        case EOptimizationLevel::Medium:
            OptimizationSettings.CharacterCullingDistance = 5000.0f;
            OptimizationSettings.MaxActiveCharacters = 10;
            OptimizationSettings.LODDistanceMultiplier = 1.0f;
            break;
        case EOptimizationLevel::High:
            OptimizationSettings.CharacterCullingDistance = 3000.0f;
            OptimizationSettings.MaxActiveCharacters = 5;
            OptimizationSettings.LODDistanceMultiplier = 0.8f;
            break;
        case EOptimizationLevel::Ultra:
            OptimizationSettings.CharacterCullingDistance = 2000.0f;
            OptimizationSettings.MaxActiveCharacters = 3;
            OptimizationSettings.LODDistanceMultiplier = 0.6f;
            break;
    }
    
    ApplyOptimizations();
    UE_LOG(LogTemp, Warning, TEXT("WalkAroundFrameRateController: Optimization level set to %d"), (int32)Level);
}

void APerf_WalkAroundFrameRateController::TestFrameRateControl()
{
    UE_LOG(LogTemp, Warning, TEXT("WalkAroundFrameRateController: Testing frame rate control system"));
    
    // Log current state
    UE_LOG(LogTemp, Warning, TEXT("Current FPS: %.1f | Target: %d | Adaptive: %s"), 
        CurrentMetrics.CurrentFPS, 
        (int32)TargetFrameRate, 
        bEnableAdaptiveFrameRate ? TEXT("ON") : TEXT("OFF"));
    
    // Test optimization application
    ApplyWalkAroundOptimizations();
    
    UE_LOG(LogTemp, Warning, TEXT("Frame rate control test completed"));
}