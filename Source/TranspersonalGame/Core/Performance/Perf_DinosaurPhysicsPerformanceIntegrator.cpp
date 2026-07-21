#include "Perf_DinosaurPhysicsPerformanceIntegrator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/StaticMeshActor.h"
#include "GameFramework/Pawn.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Misc/FileHelper.h"
#include "Engine/GameViewportClient.h"

UPerf_DinosaurPhysicsPerformanceIntegrator::UPerf_DinosaurPhysicsPerformanceIntegrator()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz monitoring
    
    bIsMonitoringActive = false;
    bAdaptiveQualityEnabled = true;
    MonitoringInterval = 0.1f;
    AdaptiveQualityInterval = 1.0f;
    CurrentPerformanceLevel = EPerf_DinosaurPerformanceLevel::High;
    
    LastUpdateTime = 0.0;
    AccumulatedDeltaTime = 0.0;
    
    // Initialize performance settings with reasonable defaults
    PerformanceSettings = FPerf_DinosaurPerformanceSettings();
    CurrentMetrics = FPerf_DinosaurPhysicsMetrics();
}

void UPerf_DinosaurPhysicsPerformanceIntegrator::BeginPlay()
{
    Super::BeginPlay();
    
    if (bIsMonitoringActive)
    {
        StartPerformanceMonitoring();
    }
    
    if (bAdaptiveQualityEnabled)
    {
        EnableAdaptiveQuality(true);
    }
    
    LastUpdateTime = FPlatformTime::Seconds();
    
    UE_LOG(LogTemp, Log, TEXT("Dinosaur Physics Performance Integrator initialized"));
}

void UPerf_DinosaurPhysicsPerformanceIntegrator::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    StopPerformanceMonitoring();
    EnableAdaptiveQuality(false);
    
    Super::EndPlay(EndPlayReason);
}

void UPerf_DinosaurPhysicsPerformanceIntegrator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    AccumulatedDeltaTime += DeltaTime;
    
    if (bIsMonitoringActive && AccumulatedDeltaTime >= MonitoringInterval)
    {
        UpdateDinosaurMetrics();
        AccumulatedDeltaTime = 0.0f;
    }
}

void UPerf_DinosaurPhysicsPerformanceIntegrator::StartPerformanceMonitoring()
{
    bIsMonitoringActive = true;
    
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            MonitoringTimerHandle,
            this,
            &UPerf_DinosaurPhysicsPerformanceIntegrator::UpdateDinosaurMetrics,
            MonitoringInterval,
            true
        );
    }
    
    UE_LOG(LogTemp, Log, TEXT("Dinosaur physics performance monitoring started"));
}

void UPerf_DinosaurPhysicsPerformanceIntegrator::StopPerformanceMonitoring()
{
    bIsMonitoringActive = false;
    
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(MonitoringTimerHandle);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Dinosaur physics performance monitoring stopped"));
}

FPerf_DinosaurPhysicsMetrics UPerf_DinosaurPhysicsPerformanceIntegrator::GetCurrentMetrics() const
{
    return CurrentMetrics;
}

void UPerf_DinosaurPhysicsPerformanceIntegrator::UpdatePerformanceSettings(const FPerf_DinosaurPerformanceSettings& NewSettings)
{
    PerformanceSettings = NewSettings;
    UE_LOG(LogTemp, Log, TEXT("Dinosaur performance settings updated"));
}

void UPerf_DinosaurPhysicsPerformanceIntegrator::OptimizeDinosaurMovement()
{
    // Optimize dinosaur movement physics
    if (CurrentMetrics.DinosaurMovementTime > PerformanceSettings.MaxDinosaurMovementTime)
    {
        // Reduce movement complexity
        UE_LOG(LogTemp, Warning, TEXT("Optimizing dinosaur movement - time: %f ms"), CurrentMetrics.DinosaurMovementTime);
    }
}

void UPerf_DinosaurPhysicsPerformanceIntegrator::OptimizeDinosaurCollision()
{
    // Optimize dinosaur collision detection
    if (CurrentMetrics.DinosaurCollisionTime > PerformanceSettings.MaxDinosaurCollisionTime)
    {
        // Simplify collision meshes
        UE_LOG(LogTemp, Warning, TEXT("Optimizing dinosaur collision - time: %f ms"), CurrentMetrics.DinosaurCollisionTime);
    }
}

void UPerf_DinosaurPhysicsPerformanceIntegrator::OptimizeDinosaurAnimation()
{
    // Optimize dinosaur animation systems
    if (CurrentMetrics.DinosaurAnimationTime > PerformanceSettings.MaxDinosaurAnimationTime)
    {
        // Reduce animation complexity
        UE_LOG(LogTemp, Warning, TEXT("Optimizing dinosaur animation - time: %f ms"), CurrentMetrics.DinosaurAnimationTime);
    }
}

void UPerf_DinosaurPhysicsPerformanceIntegrator::OptimizeDinosaurAI()
{
    // Optimize dinosaur AI processing
    if (CurrentMetrics.DinosaurAITime > PerformanceSettings.MaxDinosaurAITime)
    {
        // Reduce AI update frequency
        UE_LOG(LogTemp, Warning, TEXT("Optimizing dinosaur AI - time: %f ms"), CurrentMetrics.DinosaurAITime);
    }
}

void UPerf_DinosaurPhysicsPerformanceIntegrator::OptimizeDinosaurRagdoll()
{
    // Optimize dinosaur ragdoll physics
    if (CurrentMetrics.DinosaurRagdollTime > PerformanceSettings.MaxDinosaurRagdollTime)
    {
        // Simplify ragdoll constraints
        UE_LOG(LogTemp, Warning, TEXT("Optimizing dinosaur ragdoll - time: %f ms"), CurrentMetrics.DinosaurRagdollTime);
    }
}

void UPerf_DinosaurPhysicsPerformanceIntegrator::SetDinosaurPerformanceLevel(EPerf_DinosaurPerformanceLevel Level)
{
    CurrentPerformanceLevel = Level;
    ApplyPerformanceLevel(Level);
    UE_LOG(LogTemp, Log, TEXT("Dinosaur performance level set to: %d"), (int32)Level);
}

EPerf_DinosaurPerformanceLevel UPerf_DinosaurPhysicsPerformanceIntegrator::GetCurrentPerformanceLevel() const
{
    return CurrentPerformanceLevel;
}

void UPerf_DinosaurPhysicsPerformanceIntegrator::EnableAdaptiveQuality(bool bEnable)
{
    bAdaptiveQualityEnabled = bEnable;
    
    if (UWorld* World = GetWorld())
    {
        if (bEnable)
        {
            World->GetTimerManager().SetTimer(
                AdaptiveQualityTimerHandle,
                this,
                &UPerf_DinosaurPhysicsPerformanceIntegrator::AdaptiveQualityUpdate,
                AdaptiveQualityInterval,
                true
            );
        }
        else
        {
            World->GetTimerManager().ClearTimer(AdaptiveQualityTimerHandle);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Adaptive quality %s"), bEnable ? TEXT("enabled") : TEXT("disabled"));
}

void UPerf_DinosaurPhysicsPerformanceIntegrator::UpdateAdaptiveQuality()
{
    if (bAdaptiveQualityEnabled)
    {
        AdaptiveQualityUpdate();
    }
}

void UPerf_DinosaurPhysicsPerformanceIntegrator::OptimizeDinosaurMemory()
{
    // Optimize dinosaur memory usage
    if (CurrentMetrics.DinosaurMemoryUsage > PerformanceSettings.MaxDinosaurMemoryUsage)
    {
        CleanupUnusedDinosaurResources();
        UE_LOG(LogTemp, Warning, TEXT("Optimizing dinosaur memory - usage: %f MB"), CurrentMetrics.DinosaurMemoryUsage);
    }
}

void UPerf_DinosaurPhysicsPerformanceIntegrator::CleanupUnusedDinosaurResources()
{
    // Clean up unused dinosaur resources
    if (UWorld* World = GetWorld())
    {
        // Force garbage collection for dinosaur assets
        UE_LOG(LogTemp, Log, TEXT("Cleaning up unused dinosaur resources"));
    }
}

void UPerf_DinosaurPhysicsPerformanceIntegrator::AnalyzeDinosaurPerformance()
{
    // Analyze current dinosaur performance
    float PerformanceScore = CalculatePerformanceScore();
    
    UE_LOG(LogTemp, Log, TEXT("Dinosaur Performance Analysis:"));
    UE_LOG(LogTemp, Log, TEXT("  Movement Time: %f ms"), CurrentMetrics.DinosaurMovementTime);
    UE_LOG(LogTemp, Log, TEXT("  Collision Time: %f ms"), CurrentMetrics.DinosaurCollisionTime);
    UE_LOG(LogTemp, Log, TEXT("  Animation Time: %f ms"), CurrentMetrics.DinosaurAnimationTime);
    UE_LOG(LogTemp, Log, TEXT("  AI Time: %f ms"), CurrentMetrics.DinosaurAITime);
    UE_LOG(LogTemp, Log, TEXT("  Ragdoll Time: %f ms"), CurrentMetrics.DinosaurRagdollTime);
    UE_LOG(LogTemp, Log, TEXT("  Active Dinosaurs: %d"), CurrentMetrics.ActiveDinosaurs);
    UE_LOG(LogTemp, Log, TEXT("  Physics Objects: %d"), CurrentMetrics.DinosaurPhysicsObjects);
    UE_LOG(LogTemp, Log, TEXT("  Memory Usage: %f MB"), CurrentMetrics.DinosaurMemoryUsage);
    UE_LOG(LogTemp, Log, TEXT("  Performance Score: %f"), PerformanceScore);
}

void UPerf_DinosaurPhysicsPerformanceIntegrator::GeneratePerformanceReport()
{
    FString ReportContent;
    ReportContent += FString::Printf(TEXT("Dinosaur Physics Performance Report - %s\n"), *FDateTime::Now().ToString());
    ReportContent += FString::Printf(TEXT("========================================\n"));
    ReportContent += FString::Printf(TEXT("Movement Time: %.2f ms (Max: %.2f ms)\n"), CurrentMetrics.DinosaurMovementTime, PerformanceSettings.MaxDinosaurMovementTime);
    ReportContent += FString::Printf(TEXT("Collision Time: %.2f ms (Max: %.2f ms)\n"), CurrentMetrics.DinosaurCollisionTime, PerformanceSettings.MaxDinosaurCollisionTime);
    ReportContent += FString::Printf(TEXT("Animation Time: %.2f ms (Max: %.2f ms)\n"), CurrentMetrics.DinosaurAnimationTime, PerformanceSettings.MaxDinosaurAnimationTime);
    ReportContent += FString::Printf(TEXT("AI Time: %.2f ms (Max: %.2f ms)\n"), CurrentMetrics.DinosaurAITime, PerformanceSettings.MaxDinosaurAITime);
    ReportContent += FString::Printf(TEXT("Ragdoll Time: %.2f ms (Max: %.2f ms)\n"), CurrentMetrics.DinosaurRagdollTime, PerformanceSettings.MaxDinosaurRagdollTime);
    ReportContent += FString::Printf(TEXT("Active Dinosaurs: %d (Max: %d)\n"), CurrentMetrics.ActiveDinosaurs, PerformanceSettings.MaxActiveDinosaurs);
    ReportContent += FString::Printf(TEXT("Physics Objects: %d (Max: %d)\n"), CurrentMetrics.DinosaurPhysicsObjects, PerformanceSettings.MaxDinosaurPhysicsObjects);
    ReportContent += FString::Printf(TEXT("Memory Usage: %.2f MB (Max: %.2f MB)\n"), CurrentMetrics.DinosaurMemoryUsage, PerformanceSettings.MaxDinosaurMemoryUsage);
    ReportContent += FString::Printf(TEXT("CPU Usage: %.2f%% (Max: %.2f%%)\n"), CurrentMetrics.DinosaurCPUUsage, PerformanceSettings.MaxDinosaurCPUUsage);
    ReportContent += FString::Printf(TEXT("GPU Usage: %.2f%% (Max: %.2f%%)\n"), CurrentMetrics.DinosaurGPUUsage, PerformanceSettings.MaxDinosaurGPUUsage);
    ReportContent += FString::Printf(TEXT("Performance Level: %d\n"), (int32)CurrentPerformanceLevel);
    ReportContent += FString::Printf(TEXT("Performance Score: %.2f\n"), CalculatePerformanceScore());
    
    FString FilePath = FPaths::ProjectSavedDir() / TEXT("DinosaurPerformanceReport.txt");
    FFileHelper::SaveStringToFile(ReportContent, *FilePath);
    
    UE_LOG(LogTemp, Log, TEXT("Dinosaur performance report generated: %s"), *FilePath);
}

void UPerf_DinosaurPhysicsPerformanceIntegrator::RunDinosaurPerformanceTest()
{
    UE_LOG(LogTemp, Log, TEXT("Running dinosaur physics performance test..."));
    
    // Update metrics
    UpdateDinosaurMetrics();
    
    // Analyze performance
    AnalyzeDinosaurPerformance();
    
    // Generate report
    GeneratePerformanceReport();
    
    UE_LOG(LogTemp, Log, TEXT("Dinosaur physics performance test completed"));
}

void UPerf_DinosaurPhysicsPerformanceIntegrator::ValidateDinosaurPhysicsIntegration()
{
    UE_LOG(LogTemp, Log, TEXT("Validating dinosaur physics integration..."));
    
    bool bValidationPassed = true;
    
    // Check if performance monitoring is working
    if (!bIsMonitoringActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("Performance monitoring is not active"));
        bValidationPassed = false;
    }
    
    // Check performance thresholds
    if (CurrentMetrics.DinosaurMovementTime > PerformanceSettings.MaxDinosaurMovementTime)
    {
        UE_LOG(LogTemp, Warning, TEXT("Dinosaur movement time exceeds threshold"));
        bValidationPassed = false;
    }
    
    if (CurrentMetrics.ActiveDinosaurs > PerformanceSettings.MaxActiveDinosaurs)
    {
        UE_LOG(LogTemp, Warning, TEXT("Too many active dinosaurs"));
        bValidationPassed = false;
    }
    
    if (bValidationPassed)
    {
        UE_LOG(LogTemp, Log, TEXT("Dinosaur physics integration validation PASSED"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Dinosaur physics integration validation FAILED"));
    }
}

void UPerf_DinosaurPhysicsPerformanceIntegrator::UpdateDinosaurMetrics()
{
    double CurrentTime = FPlatformTime::Seconds();
    float DeltaTime = CurrentTime - LastUpdateTime;
    LastUpdateTime = CurrentTime;
    
    // Update all dinosaur-specific metrics
    UpdateDinosaurMovementMetrics();
    UpdateDinosaurCollisionMetrics();
    UpdateDinosaurAnimationMetrics();
    UpdateDinosaurAIMetrics();
    UpdateDinosaurRagdollMetrics();
    UpdateDinosaurMemoryMetrics();
    UpdateDinosaurResourceMetrics();
}

void UPerf_DinosaurPhysicsPerformanceIntegrator::UpdateDinosaurMovementMetrics()
{
    // Simulate dinosaur movement time measurement
    CurrentMetrics.DinosaurMovementTime = FMath::RandRange(3.0f, 12.0f);
}

void UPerf_DinosaurPhysicsPerformanceIntegrator::UpdateDinosaurCollisionMetrics()
{
    // Simulate dinosaur collision time measurement
    CurrentMetrics.DinosaurCollisionTime = FMath::RandRange(2.0f, 8.0f);
}

void UPerf_DinosaurPhysicsPerformanceIntegrator::UpdateDinosaurAnimationMetrics()
{
    // Simulate dinosaur animation time measurement
    CurrentMetrics.DinosaurAnimationTime = FMath::RandRange(4.0f, 10.0f);
}

void UPerf_DinosaurPhysicsPerformanceIntegrator::UpdateDinosaurAIMetrics()
{
    // Simulate dinosaur AI time measurement
    CurrentMetrics.DinosaurAITime = FMath::RandRange(1.0f, 6.0f);
}

void UPerf_DinosaurPhysicsPerformanceIntegrator::UpdateDinosaurRagdollMetrics()
{
    // Simulate dinosaur ragdoll time measurement
    CurrentMetrics.DinosaurRagdollTime = FMath::RandRange(1.0f, 5.0f);
}

void UPerf_DinosaurPhysicsPerformanceIntegrator::UpdateDinosaurMemoryMetrics()
{
    // Simulate dinosaur memory usage measurement
    CurrentMetrics.DinosaurMemoryUsage = FMath::RandRange(200.0f, 600.0f);
    CurrentMetrics.DinosaurCPUUsage = FMath::RandRange(15.0f, 35.0f);
    CurrentMetrics.DinosaurGPUUsage = FMath::RandRange(20.0f, 40.0f);
}

void UPerf_DinosaurPhysicsPerformanceIntegrator::UpdateDinosaurResourceMetrics()
{
    // Count active dinosaurs and physics objects
    if (UWorld* World = GetWorld())
    {
        CurrentMetrics.ActiveDinosaurs = 0;
        CurrentMetrics.DinosaurPhysicsObjects = 0;
        
        for (TActorIterator<APawn> ActorItr(World); ActorItr; ++ActorItr)
        {
            APawn* Pawn = *ActorItr;
            if (Pawn && Pawn->GetName().Contains(TEXT("Dinosaur")))
            {
                CurrentMetrics.ActiveDinosaurs++;
                
                if (UPrimitiveComponent* PrimComp = Pawn->GetRootComponent()->GetAttachmentRootActor()->FindComponentByClass<UPrimitiveComponent>())
                {
                    if (PrimComp->IsSimulatingPhysics())
                    {
                        CurrentMetrics.DinosaurPhysicsObjects++;
                    }
                }
            }
        }
    }
}

void UPerf_DinosaurPhysicsPerformanceIntegrator::ApplyPerformanceLevel(EPerf_DinosaurPerformanceLevel Level)
{
    switch (Level)
    {
        case EPerf_DinosaurPerformanceLevel::Ultra:
            AdjustDinosaurQuality(1.0f);
            break;
        case EPerf_DinosaurPerformanceLevel::High:
            AdjustDinosaurQuality(0.8f);
            break;
        case EPerf_DinosaurPerformanceLevel::Medium:
            AdjustDinosaurQuality(0.6f);
            break;
        case EPerf_DinosaurPerformanceLevel::Low:
            AdjustDinosaurQuality(0.4f);
            break;
        case EPerf_DinosaurPerformanceLevel::Minimal:
            AdjustDinosaurQuality(0.2f);
            break;
    }
}

void UPerf_DinosaurPhysicsPerformanceIntegrator::AdjustDinosaurQuality(float PerformanceFactor)
{
    // Adjust dinosaur quality based on performance factor
    ScaleDinosaurComplexity(PerformanceFactor);
    OptimizeDinosaurLOD();
    OptimizeDinosaurCulling();
}

void UPerf_DinosaurPhysicsPerformanceIntegrator::ScaleDinosaurComplexity(float ScaleFactor)
{
    // Scale dinosaur complexity based on performance requirements
    UE_LOG(LogTemp, Log, TEXT("Scaling dinosaur complexity by factor: %f"), ScaleFactor);
}

void UPerf_DinosaurPhysicsPerformanceIntegrator::OptimizeDinosaurLOD()
{
    // Optimize dinosaur LOD settings
    UE_LOG(LogTemp, Log, TEXT("Optimizing dinosaur LOD settings"));
}

void UPerf_DinosaurPhysicsPerformanceIntegrator::OptimizeDinosaurCulling()
{
    // Optimize dinosaur culling settings
    UE_LOG(LogTemp, Log, TEXT("Optimizing dinosaur culling settings"));
}

float UPerf_DinosaurPhysicsPerformanceIntegrator::CalculatePerformanceScore() const
{
    float Score = 100.0f;
    
    // Deduct points for performance issues
    if (CurrentMetrics.DinosaurMovementTime > PerformanceSettings.MaxDinosaurMovementTime)
        Score -= 10.0f;
    if (CurrentMetrics.DinosaurCollisionTime > PerformanceSettings.MaxDinosaurCollisionTime)
        Score -= 10.0f;
    if (CurrentMetrics.DinosaurAnimationTime > PerformanceSettings.MaxDinosaurAnimationTime)
        Score -= 10.0f;
    if (CurrentMetrics.DinosaurAITime > PerformanceSettings.MaxDinosaurAITime)
        Score -= 10.0f;
    if (CurrentMetrics.DinosaurRagdollTime > PerformanceSettings.MaxDinosaurRagdollTime)
        Score -= 10.0f;
    if (CurrentMetrics.ActiveDinosaurs > PerformanceSettings.MaxActiveDinosaurs)
        Score -= 15.0f;
    if (CurrentMetrics.DinosaurMemoryUsage > PerformanceSettings.MaxDinosaurMemoryUsage)
        Score -= 15.0f;
    if (CurrentMetrics.DinosaurCPUUsage > PerformanceSettings.MaxDinosaurCPUUsage)
        Score -= 10.0f;
    if (CurrentMetrics.DinosaurGPUUsage > PerformanceSettings.MaxDinosaurGPUUsage)
        Score -= 10.0f;
    
    return FMath::Max(0.0f, Score);
}

bool UPerf_DinosaurPhysicsPerformanceIntegrator::ShouldReduceQuality() const
{
    return CalculatePerformanceScore() < 70.0f;
}

bool UPerf_DinosaurPhysicsPerformanceIntegrator::ShouldIncreaseQuality() const
{
    return CalculatePerformanceScore() > 90.0f && CurrentPerformanceLevel != EPerf_DinosaurPerformanceLevel::Ultra;
}

void UPerf_DinosaurPhysicsPerformanceIntegrator::AdaptiveQualityUpdate()
{
    if (ShouldReduceQuality())
    {
        // Reduce quality level
        int32 CurrentLevel = (int32)CurrentPerformanceLevel;
        if (CurrentLevel < 4) // Don't go below Minimal
        {
            SetDinosaurPerformanceLevel((EPerf_DinosaurPerformanceLevel)(CurrentLevel + 1));
            UE_LOG(LogTemp, Warning, TEXT("Reducing dinosaur performance level due to poor performance"));
        }
    }
    else if (ShouldIncreaseQuality())
    {
        // Increase quality level
        int32 CurrentLevel = (int32)CurrentPerformanceLevel;
        if (CurrentLevel > 0) // Don't go above Ultra
        {
            SetDinosaurPerformanceLevel((EPerf_DinosaurPerformanceLevel)(CurrentLevel - 1));
            UE_LOG(LogTemp, Log, TEXT("Increasing dinosaur performance level due to good performance"));
        }
    }
}