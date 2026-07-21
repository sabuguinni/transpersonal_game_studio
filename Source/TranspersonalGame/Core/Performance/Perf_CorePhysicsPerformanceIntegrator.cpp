#include "Perf_CorePhysicsPerformanceIntegrator.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Actor.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"

UPerf_CorePhysicsPerformanceIntegrator::UPerf_CorePhysicsPerformanceIntegrator()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update every 100ms
    
    CurrentQualityLevel = EPerf_PhysicsQualityLevel::Medium;
    bAutoOptimizePhysics = true;
    TargetPhysicsFrameTime = 16.67f; // 60 FPS target
    MaxPhysicsActors = 1000;
    
    FrameHistoryIndex = 0;
    LastOptimizationTime = 0.0f;
    bInitialized = false;
    
    // Initialize frame time history
    for (int32 i = 0; i < 60; ++i)
    {
        PhysicsFrameTimeHistory[i] = 0.0f;
    }
}

void UPerf_CorePhysicsPerformanceIntegrator::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Core Physics Performance Integrator initialized"));
    
    // Initialize physics settings based on current quality level
    SetPhysicsQualityLevel(CurrentQualityLevel);
    bInitialized = true;
}

void UPerf_CorePhysicsPerformanceIntegrator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bInitialized)
        return;
    
    // Update performance metrics
    UpdatePhysicsMetrics();
    
    // Monitor and adjust physics settings if auto-optimization is enabled
    if (bAutoOptimizePhysics)
    {
        MonitorPhysicsPerformance();
        
        // Only optimize every 2 seconds to avoid thrashing
        float CurrentTime = GetWorld()->GetTimeSeconds();
        if (CurrentTime - LastOptimizationTime > 2.0f)
        {
            AdjustPhysicsSettings();
            LastOptimizationTime = CurrentTime;
        }
    }
}

void UPerf_CorePhysicsPerformanceIntegrator::UpdatePhysicsMetrics()
{
    if (!GetWorld())
        return;
    
    // Calculate physics frame time
    float CurrentFrameTime = GetWorld()->GetDeltaSeconds() * 1000.0f; // Convert to ms
    PhysicsFrameTimeHistory[FrameHistoryIndex] = CurrentFrameTime;
    FrameHistoryIndex = (FrameHistoryIndex + 1) % 60;
    
    // Calculate average frame time over last 60 frames
    float TotalFrameTime = 0.0f;
    for (int32 i = 0; i < 60; ++i)
    {
        TotalFrameTime += PhysicsFrameTimeHistory[i];
    }
    CurrentMetrics.PhysicsFrameTime = TotalFrameTime / 60.0f;
    
    // Count active physics actors
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
    
    CurrentMetrics.ActivePhysicsActors = 0;
    for (AActor* Actor : AllActors)
    {
        if (Actor && Actor->GetRootComponent())
        {
            UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
            if (PrimComp && PrimComp->GetCollisionEnabled() != ECollisionEnabled::NoCollision)
            {
                CurrentMetrics.ActivePhysicsActors++;
            }
        }
    }
    
    // Estimate physics memory usage (simplified)
    CurrentMetrics.PhysicsMemoryUsage = CurrentMetrics.ActivePhysicsActors * 0.5f; // Rough estimate in MB
    
    // Calculate average physics step time
    CurrentMetrics.AveragePhysicsStepTime = CurrentMetrics.PhysicsFrameTime * 0.3f; // Physics typically 30% of frame
}

void UPerf_CorePhysicsPerformanceIntegrator::OptimizePhysicsSettings()
{
    if (!IsPhysicsPerformanceAcceptable())
    {
        // Downgrade quality level if performance is poor
        switch (CurrentQualityLevel)
        {
            case EPerf_PhysicsQualityLevel::Ultra:
                SetPhysicsQualityLevel(EPerf_PhysicsQualityLevel::High);
                break;
            case EPerf_PhysicsQualityLevel::High:
                SetPhysicsQualityLevel(EPerf_PhysicsQualityLevel::Medium);
                break;
            case EPerf_PhysicsQualityLevel::Medium:
                SetPhysicsQualityLevel(EPerf_PhysicsQualityLevel::Low);
                break;
            default:
                // Already at lowest setting
                break;
        }
    }
    else if (CurrentMetrics.PhysicsFrameTime < TargetPhysicsFrameTime * 0.7f)
    {
        // Upgrade quality level if we have performance headroom
        switch (CurrentQualityLevel)
        {
            case EPerf_PhysicsQualityLevel::Low:
                SetPhysicsQualityLevel(EPerf_PhysicsQualityLevel::Medium);
                break;
            case EPerf_PhysicsQualityLevel::Medium:
                SetPhysicsQualityLevel(EPerf_PhysicsQualityLevel::High);
                break;
            case EPerf_PhysicsQualityLevel::High:
                SetPhysicsQualityLevel(EPerf_PhysicsQualityLevel::Ultra);
                break;
            default:
                // Already at highest setting
                break;
        }
    }
}

void UPerf_CorePhysicsPerformanceIntegrator::SetPhysicsQualityLevel(EPerf_PhysicsQualityLevel NewLevel)
{
    CurrentQualityLevel = NewLevel;
    
    switch (NewLevel)
    {
        case EPerf_PhysicsQualityLevel::Low:
            ApplyLowQualitySettings();
            break;
        case EPerf_PhysicsQualityLevel::Medium:
            ApplyMediumQualitySettings();
            break;
        case EPerf_PhysicsQualityLevel::High:
            ApplyHighQualitySettings();
            break;
        case EPerf_PhysicsQualityLevel::Ultra:
            ApplyUltraQualitySettings();
            break;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Physics quality level set to: %d"), (int32)NewLevel);
}

bool UPerf_CorePhysicsPerformanceIntegrator::IsPhysicsPerformanceAcceptable() const
{
    return CurrentMetrics.PhysicsFrameTime <= TargetPhysicsFrameTime && 
           CurrentMetrics.ActivePhysicsActors <= MaxPhysicsActors;
}

void UPerf_CorePhysicsPerformanceIntegrator::ApplyPhysicsOptimizations()
{
    // Apply general physics optimizations
    if (UPhysicsSettings* PhysicsSettings = GetMutableDefault<UPhysicsSettings>())
    {
        // Optimize physics solver iterations based on quality level
        switch (CurrentQualityLevel)
        {
            case EPerf_PhysicsQualityLevel::Low:
                PhysicsSettings->SolverOptions.SolverIterations.SolverIterations = 4;
                PhysicsSettings->SolverOptions.SolverIterations.VelocityIterations = 1;
                break;
            case EPerf_PhysicsQualityLevel::Medium:
                PhysicsSettings->SolverOptions.SolverIterations.SolverIterations = 6;
                PhysicsSettings->SolverOptions.SolverIterations.VelocityIterations = 2;
                break;
            case EPerf_PhysicsQualityLevel::High:
                PhysicsSettings->SolverOptions.SolverIterations.SolverIterations = 8;
                PhysicsSettings->SolverOptions.SolverIterations.VelocityIterations = 3;
                break;
            case EPerf_PhysicsQualityLevel::Ultra:
                PhysicsSettings->SolverOptions.SolverIterations.SolverIterations = 10;
                PhysicsSettings->SolverOptions.SolverIterations.VelocityIterations = 4;
                break;
        }
    }
}

void UPerf_CorePhysicsPerformanceIntegrator::ResetPhysicsToDefaults()
{
    SetPhysicsQualityLevel(EPerf_PhysicsQualityLevel::Medium);
    bAutoOptimizePhysics = true;
    TargetPhysicsFrameTime = 16.67f;
    MaxPhysicsActors = 1000;
    
    UE_LOG(LogTemp, Warning, TEXT("Physics settings reset to defaults"));
}

void UPerf_CorePhysicsPerformanceIntegrator::ApplyLowQualitySettings()
{
    // Low quality: Prioritize performance over accuracy
    MaxPhysicsActors = 500;
    TargetPhysicsFrameTime = 20.0f; // Allow 50 FPS minimum
    
    ApplyPhysicsOptimizations();
}

void UPerf_CorePhysicsPerformanceIntegrator::ApplyMediumQualitySettings()
{
    // Medium quality: Balanced performance and accuracy
    MaxPhysicsActors = 1000;
    TargetPhysicsFrameTime = 16.67f; // 60 FPS target
    
    ApplyPhysicsOptimizations();
}

void UPerf_CorePhysicsPerformanceIntegrator::ApplyHighQualitySettings()
{
    // High quality: Favor accuracy with good performance
    MaxPhysicsActors = 1500;
    TargetPhysicsFrameTime = 13.33f; // 75 FPS target
    
    ApplyPhysicsOptimizations();
}

void UPerf_CorePhysicsPerformanceIntegrator::ApplyUltraQualitySettings()
{
    // Ultra quality: Maximum accuracy for high-end systems
    MaxPhysicsActors = 2000;
    TargetPhysicsFrameTime = 11.11f; // 90 FPS target
    
    ApplyPhysicsOptimizations();
}

void UPerf_CorePhysicsPerformanceIntegrator::MonitorPhysicsPerformance()
{
    // Check if we need to adjust settings
    if (CurrentMetrics.PhysicsFrameTime > TargetPhysicsFrameTime * 1.2f)
    {
        UE_LOG(LogTemp, Warning, TEXT("Physics performance below target: %.2fms (target: %.2fms)"), 
               CurrentMetrics.PhysicsFrameTime, TargetPhysicsFrameTime);
    }
    
    if (CurrentMetrics.ActivePhysicsActors > MaxPhysicsActors)
    {
        UE_LOG(LogTemp, Warning, TEXT("Physics actor count exceeded: %d (max: %d)"), 
               CurrentMetrics.ActivePhysicsActors, MaxPhysicsActors);
    }
}

void UPerf_CorePhysicsPerformanceIntegrator::AdjustPhysicsSettings()
{
    if (bAutoOptimizePhysics)
    {
        OptimizePhysicsSettings();
    }
}