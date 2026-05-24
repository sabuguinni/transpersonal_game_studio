#include "Perf_PerformanceManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/GameViewportClient.h"
#include "HAL/PlatformFilemanager.h"
#include "HAL/PlatformMemory.h"
#include "RenderingThread.h"
#include "RHI.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/SkeletalMeshActor.h"

UPerf_PerformanceManager::UPerf_PerformanceManager()
{
    // Initialize default values
    CurrentOptimizationLevel = EPerf_OptimizationLevel::High;
    CurrentTargetPlatform = EPerf_TargetPlatform::PC_HighEnd;
    bIsMonitoring = false;
    MonitoringInterval = 1.0f;
    FrameCounter = 0;
    TotalFrameTime = 0.0f;
    
    // Set target FPS based on platform
    TargetFPS_PC = 60.0f;
    TargetFPS_Console = 30.0f;
    MinAcceptableFPS = 20.0f;
    
    // Initialize LOD configuration
    LODConfig = FPerf_LODConfiguration();
    
    // Initialize metrics
    CurrentMetrics = FPerf_PerformanceMetrics();
}

void UPerf_PerformanceManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Performance Manager initialized"));
    
    // Start monitoring by default
    StartPerformanceMonitoring();
    
    // Apply default optimization settings
    ApplyOptimizationSettings();
}

void UPerf_PerformanceManager::Deinitialize()
{
    StopPerformanceMonitoring();
    Super::Deinitialize();
}

void UPerf_PerformanceManager::StartPerformanceMonitoring()
{
    if (bIsMonitoring)
    {
        return;
    }
    
    bIsMonitoring = true;
    FrameCounter = 0;
    TotalFrameTime = 0.0f;
    FPSHistory.Empty();
    
    // Set up timer for periodic updates
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            MonitoringTimerHandle,
            this,
            &UPerf_PerformanceManager::InternalUpdateMetrics,
            MonitoringInterval,
            true
        );
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Performance monitoring started"));
}

void UPerf_PerformanceManager::StopPerformanceMonitoring()
{
    if (!bIsMonitoring)
    {
        return;
    }
    
    bIsMonitoring = false;
    
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(MonitoringTimerHandle);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Performance monitoring stopped"));
}

FPerf_PerformanceMetrics UPerf_PerformanceManager::GetCurrentMetrics() const
{
    return CurrentMetrics;
}

void UPerf_PerformanceManager::UpdatePerformanceMetrics()
{
    if (!bIsMonitoring)
    {
        return;
    }
    
    // Get current FPS
    if (GEngine && GEngine->GetGameViewport())
    {
        float DeltaTime = GetWorld()->GetDeltaSeconds();
        if (DeltaTime > 0.0f)
        {
            CurrentMetrics.CurrentFPS = 1.0f / DeltaTime;
        }
    }
    
    // Update FPS history
    FPSHistory.Add(CurrentMetrics.CurrentFPS);
    if (FPSHistory.Num() > 60) // Keep last 60 samples
    {
        FPSHistory.RemoveAt(0);
    }
    
    // Calculate average, min, max FPS
    CalculateAverageFPS();
    
    // Count active physics actors
    CurrentMetrics.ActivePhysicsActors = 0;
    if (UWorld* World = GetWorld())
    {
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor && Actor->GetRootComponent())
            {
                if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent()))
                {
                    if (PrimComp->IsSimulatingPhysics())
                    {
                        CurrentMetrics.ActivePhysicsActors++;
                    }
                }
            }
        }
    }
    
    // Get memory usage
    CurrentMetrics.MemoryUsageMB = GetMemoryUsageMB();
    
    // Check performance thresholds
    CheckPerformanceThresholds();
}

void UPerf_PerformanceManager::SetLODConfiguration(const FPerf_LODConfiguration& NewConfig)
{
    LODConfig = NewConfig;
    UpdateLODDistances();
    UE_LOG(LogTemp, Warning, TEXT("LOD configuration updated"));
}

FPerf_LODConfiguration UPerf_PerformanceManager::GetLODConfiguration() const
{
    return LODConfig;
}

void UPerf_PerformanceManager::ApplyLODToActor(AActor* Actor, float Distance)
{
    if (!Actor || !LODConfig.bEnableAutomaticLOD)
    {
        return;
    }
    
    int32 LODLevel = 0;
    
    if (Distance > LODConfig.CullingDistance)
    {
        // Cull the actor
        Actor->SetActorHiddenInGame(true);
        return;
    }
    else if (Distance > LODConfig.LOD2Distance)
    {
        LODLevel = 2;
    }
    else if (Distance > LODConfig.LOD1Distance)
    {
        LODLevel = 1;
    }
    else
    {
        LODLevel = 0;
    }
    
    Actor->SetActorHiddenInGame(false);
    SetActorLODLevel(Actor, LODLevel);
}

void UPerf_PerformanceManager::UpdateAllActorLODs()
{
    if (!LODConfig.bEnableAutomaticLOD)
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
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && Actor != PlayerPawn)
        {
            float Distance = FVector::Dist(Actor->GetActorLocation(), PlayerLocation);
            ApplyLODToActor(Actor, Distance);
        }
    }
}

void UPerf_PerformanceManager::SetOptimizationLevel(EPerf_OptimizationLevel Level)
{
    CurrentOptimizationLevel = Level;
    ApplyOptimizationSettings();
    UE_LOG(LogTemp, Warning, TEXT("Optimization level set to: %d"), (int32)Level);
}

void UPerf_PerformanceManager::SetTargetPlatform(EPerf_TargetPlatform Platform)
{
    CurrentTargetPlatform = Platform;
    
    // Adjust target FPS based on platform
    switch (Platform)
    {
        case EPerf_TargetPlatform::PC_HighEnd:
            TargetFPS_PC = 60.0f;
            break;
        case EPerf_TargetPlatform::PC_MidRange:
            TargetFPS_PC = 45.0f;
            break;
        case EPerf_TargetPlatform::PC_LowEnd:
            TargetFPS_PC = 30.0f;
            break;
        case EPerf_TargetPlatform::Console_PS5:
        case EPerf_TargetPlatform::Console_XboxSX:
            TargetFPS_Console = 30.0f;
            break;
        case EPerf_TargetPlatform::Console_Switch:
            TargetFPS_Console = 30.0f;
            MinAcceptableFPS = 20.0f;
            break;
    }
    
    ApplyOptimizationSettings();
}

void UPerf_PerformanceManager::OptimizeForCurrentPlatform()
{
    ApplyOptimizationSettings();
    OptimizePhysicsActors();
    UpdateAllActorLODs();
    OptimizeMemoryUsage();
    
    UE_LOG(LogTemp, Warning, TEXT("Platform optimization applied"));
}

void UPerf_PerformanceManager::OptimizePhysicsActors()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    int32 OptimizedCount = 0;
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && Actor->GetRootComponent())
        {
            if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent()))
            {
                if (PrimComp->IsSimulatingPhysics())
                {
                    // Optimize physics settings based on optimization level
                    switch (CurrentOptimizationLevel)
                    {
                        case EPerf_OptimizationLevel::Low:
                        case EPerf_OptimizationLevel::Potato:
                            // Reduce physics complexity
                            PrimComp->SetCollisionResponseToAllChannels(ECR_Ignore);
                            PrimComp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
                            break;
                        case EPerf_OptimizationLevel::Medium:
                            // Moderate physics optimization
                            PrimComp->SetGenerateOverlapEvents(false);
                            break;
                        default:
                            // Keep full physics for high/ultra
                            break;
                    }
                    OptimizedCount++;
                }
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Optimized %d physics actors"), OptimizedCount);
}

void UPerf_PerformanceManager::DisableDistantPhysics(float MaxDistance)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
    if (!PlayerPawn)
    {
        return;
    }
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    int32 DisabledCount = 0;
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && Actor != PlayerPawn && Actor->GetRootComponent())
        {
            float Distance = FVector::Dist(Actor->GetActorLocation(), PlayerLocation);
            
            if (Distance > MaxDistance)
            {
                if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent()))
                {
                    if (PrimComp->IsSimulatingPhysics())
                    {
                        PrimComp->SetSimulatePhysics(false);
                        DisabledCount++;
                    }
                }
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Disabled physics on %d distant actors"), DisabledCount);
}

void UPerf_PerformanceManager::EnablePhysicsLOD()
{
    // Enable physics LOD based on distance
    DisableDistantPhysics(LODConfig.LOD1Distance);
}

void UPerf_PerformanceManager::ForceGarbageCollection()
{
    GEngine->ForceGarbageCollection(true);
    UE_LOG(LogTemp, Warning, TEXT("Forced garbage collection"));
}

void UPerf_PerformanceManager::OptimizeMemoryUsage()
{
    // Force garbage collection
    ForceGarbageCollection();
    
    // Flush rendering commands
    FlushRenderingCommands();
    
    UE_LOG(LogTemp, Warning, TEXT("Memory optimization completed"));
}

float UPerf_PerformanceManager::GetMemoryUsageMB() const
{
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    return MemStats.UsedPhysical / (1024.0f * 1024.0f);
}

void UPerf_PerformanceManager::EnablePerformanceHUD(bool bEnable)
{
    if (GEngine && GEngine->GetGameViewport())
    {
        if (bEnable)
        {
            GEngine->GetGameViewport()->GetEngineShowFlags()->SetStat(true);
        }
        else
        {
            GEngine->GetGameViewport()->GetEngineShowFlags()->SetStat(false);
        }
    }
}

void UPerf_PerformanceManager::LogPerformanceReport()
{
    UE_LOG(LogTemp, Warning, TEXT("=== PERFORMANCE REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Current FPS: %.2f"), CurrentMetrics.CurrentFPS);
    UE_LOG(LogTemp, Warning, TEXT("Average FPS: %.2f"), CurrentMetrics.AverageFPS);
    UE_LOG(LogTemp, Warning, TEXT("Min FPS: %.2f"), CurrentMetrics.MinFPS);
    UE_LOG(LogTemp, Warning, TEXT("Max FPS: %.2f"), CurrentMetrics.MaxFPS);
    UE_LOG(LogTemp, Warning, TEXT("Active Physics Actors: %d"), CurrentMetrics.ActivePhysicsActors);
    UE_LOG(LogTemp, Warning, TEXT("Memory Usage: %.2f MB"), CurrentMetrics.MemoryUsageMB);
    UE_LOG(LogTemp, Warning, TEXT("========================="));
}

void UPerf_PerformanceManager::DumpPerformanceStats()
{
    LogPerformanceReport();
    
    // Write to file for external analysis
    FString FilePath = FPaths::ProjectLogDir() + TEXT("PerformanceStats.txt");
    FString Content = FString::Printf(
        TEXT("Performance Statistics\n")
        TEXT("Current FPS: %.2f\n")
        TEXT("Average FPS: %.2f\n")
        TEXT("Min FPS: %.2f\n")
        TEXT("Max FPS: %.2f\n")
        TEXT("Active Physics Actors: %d\n")
        TEXT("Memory Usage: %.2f MB\n"),
        CurrentMetrics.CurrentFPS,
        CurrentMetrics.AverageFPS,
        CurrentMetrics.MinFPS,
        CurrentMetrics.MaxFPS,
        CurrentMetrics.ActivePhysicsActors,
        CurrentMetrics.MemoryUsageMB
    );
    
    FFileHelper::SaveStringToFile(Content, *FilePath);
}

void UPerf_PerformanceManager::InternalUpdateMetrics()
{
    UpdatePerformanceMetrics();
}

void UPerf_PerformanceManager::CalculateAverageFPS()
{
    if (FPSHistory.Num() == 0)
    {
        return;
    }
    
    float Sum = 0.0f;
    float Min = FPSHistory[0];
    float Max = FPSHistory[0];
    
    for (float FPS : FPSHistory)
    {
        Sum += FPS;
        Min = FMath::Min(Min, FPS);
        Max = FMath::Max(Max, FPS);
    }
    
    CurrentMetrics.AverageFPS = Sum / FPSHistory.Num();
    CurrentMetrics.MinFPS = Min;
    CurrentMetrics.MaxFPS = Max;
}

void UPerf_PerformanceManager::CheckPerformanceThresholds()
{
    float TargetFPS = (CurrentTargetPlatform == EPerf_TargetPlatform::PC_HighEnd ||
                       CurrentTargetPlatform == EPerf_TargetPlatform::PC_MidRange ||
                       CurrentTargetPlatform == EPerf_TargetPlatform::PC_LowEnd) ? 
                       TargetFPS_PC : TargetFPS_Console;
    
    if (CurrentMetrics.CurrentFPS < MinAcceptableFPS)
    {
        // Critical performance issue - apply emergency optimizations
        if (CurrentOptimizationLevel != EPerf_OptimizationLevel::Potato)
        {
            SetOptimizationLevel(EPerf_OptimizationLevel::Low);
            DisableDistantPhysics(1000.0f);
        }
    }
    else if (CurrentMetrics.CurrentFPS < TargetFPS * 0.8f)
    {
        // Performance below target - apply moderate optimizations
        EnablePhysicsLOD();
        UpdateAllActorLODs();
    }
}

void UPerf_PerformanceManager::ApplyOptimizationSettings()
{
    // Apply settings based on current optimization level
    switch (CurrentOptimizationLevel)
    {
        case EPerf_OptimizationLevel::Ultra:
            LODConfig.LOD0Distance = 2000.0f;
            LODConfig.LOD1Distance = 4000.0f;
            LODConfig.LOD2Distance = 8000.0f;
            LODConfig.CullingDistance = 15000.0f;
            break;
        case EPerf_OptimizationLevel::High:
            LODConfig.LOD0Distance = 1500.0f;
            LODConfig.LOD1Distance = 3000.0f;
            LODConfig.LOD2Distance = 6000.0f;
            LODConfig.CullingDistance = 12000.0f;
            break;
        case EPerf_OptimizationLevel::Medium:
            LODConfig.LOD0Distance = 1000.0f;
            LODConfig.LOD1Distance = 2000.0f;
            LODConfig.LOD2Distance = 4000.0f;
            LODConfig.CullingDistance = 8000.0f;
            break;
        case EPerf_OptimizationLevel::Low:
            LODConfig.LOD0Distance = 500.0f;
            LODConfig.LOD1Distance = 1000.0f;
            LODConfig.LOD2Distance = 2000.0f;
            LODConfig.CullingDistance = 4000.0f;
            break;
        case EPerf_OptimizationLevel::Potato:
            LODConfig.LOD0Distance = 300.0f;
            LODConfig.LOD1Distance = 600.0f;
            LODConfig.LOD2Distance = 1200.0f;
            LODConfig.CullingDistance = 2000.0f;
            break;
    }
    
    UpdateLODDistances();
}

void UPerf_PerformanceManager::UpdateLODDistances()
{
    // LOD distances have been updated - apply to all actors
    UpdateAllActorLODs();
}

float UPerf_PerformanceManager::CalculateActorDistance(AActor* Actor) const
{
    if (!Actor)
    {
        return 0.0f;
    }
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return 0.0f;
    }
    
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
    if (!PlayerPawn)
    {
        return 0.0f;
    }
    
    return FVector::Dist(Actor->GetActorLocation(), PlayerPawn->GetActorLocation());
}

void UPerf_PerformanceManager::SetActorLODLevel(AActor* Actor, int32 LODLevel)
{
    if (!Actor)
    {
        return;
    }
    
    // Apply LOD to static mesh actors
    if (AStaticMeshActor* StaticMeshActor = Cast<AStaticMeshActor>(Actor))
    {
        if (UStaticMeshComponent* MeshComp = StaticMeshActor->GetStaticMeshComponent())
        {
            MeshComp->SetForcedLodModel(LODLevel + 1);
        }
    }
    
    // Apply LOD to skeletal mesh actors
    if (ASkeletalMeshActor* SkeletalMeshActor = Cast<ASkeletalMeshActor>(Actor))
    {
        if (USkeletalMeshComponent* MeshComp = SkeletalMeshActor->GetSkeletalMeshComponent())
        {
            MeshComp->SetForcedLOD(LODLevel + 1);
        }
    }
}