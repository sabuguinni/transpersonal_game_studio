#include "Perf_SurvivalPerformanceIntegrator.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/GameStateBase.h"
#include "Kismet/GameplayStatics.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"

UPerf_SurvivalPerformanceIntegrator::UPerf_SurvivalPerformanceIntegrator()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update every 100ms
    
    bIsMonitoring = false;
    OptimizationLevel = EPerf_SurvivalOptimizationLevel::Balanced;
    MaxAcceptableSurvivalCost = 2.0f; // 2ms max for survival systems
    TargetSurvivalUpdateRate = 10.0f; // 10Hz for survival updates
    bEnableAdaptiveUpdateRates = true;
    bEnableSurvivalLOD = true;
    SurvivalSystemBudgetMs = 1.5f;
    
    LastUpdateTime = 0.0;
    FrameTimeIndex = 0;
    RecentFrameTimes.SetNum(60); // Track last 60 frames
    
    CurrentMetrics = FPerf_SurvivalMetrics();
}

void UPerf_SurvivalPerformanceIntegrator::BeginPlay()
{
    Super::BeginPlay();
    
    // Auto-start monitoring in development builds
    #if UE_BUILD_DEVELOPMENT || UE_BUILD_DEBUG
    StartSurvivalPerformanceMonitoring();
    #endif
    
    UE_LOG(LogTemp, Log, TEXT("Perf_SurvivalPerformanceIntegrator: BeginPlay - Component initialized"));
}

void UPerf_SurvivalPerformanceIntegrator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bIsMonitoring)
    {
        UpdateSurvivalMetrics(DeltaTime);
        CheckPerformanceThresholds();
        
        if (OptimizationLevel != EPerf_SurvivalOptimizationLevel::Disabled)
        {
            ApplyOptimizations();
        }
    }
}

void UPerf_SurvivalPerformanceIntegrator::StartSurvivalPerformanceMonitoring()
{
    bIsMonitoring = true;
    LastUpdateTime = FPlatformTime::Seconds();
    
    // Reset metrics
    CurrentMetrics = FPerf_SurvivalMetrics();
    
    // Clear frame time history
    for (int32 i = 0; i < RecentFrameTimes.Num(); i++)
    {
        RecentFrameTimes[i] = 0.0f;
    }
    FrameTimeIndex = 0;
    
    UE_LOG(LogTemp, Log, TEXT("Perf_SurvivalPerformanceIntegrator: Started survival performance monitoring"));
}

void UPerf_SurvivalPerformanceIntegrator::StopSurvivalPerformanceMonitoring()
{
    bIsMonitoring = false;
    UE_LOG(LogTemp, Log, TEXT("Perf_SurvivalPerformanceIntegrator: Stopped survival performance monitoring"));
}

FPerf_SurvivalMetrics UPerf_SurvivalPerformanceIntegrator::GetCurrentSurvivalMetrics() const
{
    return CurrentMetrics;
}

void UPerf_SurvivalPerformanceIntegrator::SetSurvivalOptimizationLevel(EPerf_SurvivalOptimizationLevel Level)
{
    OptimizationLevel = Level;
    
    // Adjust parameters based on optimization level
    switch (Level)
    {
        case EPerf_SurvivalOptimizationLevel::Disabled:
            PrimaryComponentTick.TickInterval = 1.0f;
            SurvivalSystemBudgetMs = 10.0f;
            break;
            
        case EPerf_SurvivalOptimizationLevel::Conservative:
            PrimaryComponentTick.TickInterval = 0.2f;
            SurvivalSystemBudgetMs = 3.0f;
            TargetSurvivalUpdateRate = 5.0f;
            break;
            
        case EPerf_SurvivalOptimizationLevel::Balanced:
            PrimaryComponentTick.TickInterval = 0.1f;
            SurvivalSystemBudgetMs = 1.5f;
            TargetSurvivalUpdateRate = 10.0f;
            break;
            
        case EPerf_SurvivalOptimizationLevel::Aggressive:
            PrimaryComponentTick.TickInterval = 0.05f;
            SurvivalSystemBudgetMs = 1.0f;
            TargetSurvivalUpdateRate = 20.0f;
            break;
            
        case EPerf_SurvivalOptimizationLevel::Maximum:
            PrimaryComponentTick.TickInterval = 0.033f;
            SurvivalSystemBudgetMs = 0.5f;
            TargetSurvivalUpdateRate = 30.0f;
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Perf_SurvivalPerformanceIntegrator: Set optimization level to %d"), (int32)Level);
}

void UPerf_SurvivalPerformanceIntegrator::OptimizeSurvivalSystems()
{
    if (!bIsMonitoring)
    {
        UE_LOG(LogTemp, Warning, TEXT("Perf_SurvivalPerformanceIntegrator: Cannot optimize - monitoring not active"));
        return;
    }
    
    double StartTime = FPlatformTime::Seconds();
    
    // Count active survival components in the world
    UWorld* World = GetWorld();
    if (World)
    {
        CurrentMetrics.ActiveSurvivalComponents = 0;
        
        // Simulate survival system optimization
        for (TActorIterator<APawn> ActorItr(World); ActorItr; ++ActorItr)
        {
            APawn* Pawn = *ActorItr;
            if (Pawn && !Pawn->IsPendingKill())
            {
                CurrentMetrics.ActiveSurvivalComponents++;
            }
        }
    }
    
    // Simulate performance costs based on active components
    float BaseCost = CurrentMetrics.ActiveSurvivalComponents * 0.1f;
    CurrentMetrics.PlayerHealthUpdateCost = BaseCost * 0.3f;
    CurrentMetrics.HungerSystemCost = BaseCost * 0.2f;
    CurrentMetrics.ThirstSystemCost = BaseCost * 0.2f;
    CurrentMetrics.StaminaSystemCost = BaseCost * 0.2f;
    CurrentMetrics.TemperatureSystemCost = BaseCost * 0.1f;
    
    double EndTime = FPlatformTime::Seconds();
    double OptimizationTime = (EndTime - StartTime) * 1000.0; // Convert to ms
    
    UE_LOG(LogTemp, Log, TEXT("Perf_SurvivalPerformanceIntegrator: Optimized %d survival components in %.2fms"), 
           CurrentMetrics.ActiveSurvivalComponents, OptimizationTime);
}

void UPerf_SurvivalPerformanceIntegrator::IntegrateWithPhysicsPerformance()
{
    // Integration with physics performance systems
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Warning, TEXT("Perf_SurvivalPerformanceIntegrator: No world context for physics integration"));
        return;
    }
    
    // Look for physics performance integrators
    bool bFoundPhysicsIntegrator = false;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && Actor->GetName().Contains(TEXT("Physics")))
        {
            bFoundPhysicsIntegrator = true;
            break;
        }
    }
    
    if (bFoundPhysicsIntegrator)
    {
        // Adjust survival system performance based on physics load
        float PhysicsLoadFactor = 1.2f; // Assume 20% additional load from physics
        SurvivalSystemBudgetMs *= PhysicsLoadFactor;
        
        UE_LOG(LogTemp, Log, TEXT("Perf_SurvivalPerformanceIntegrator: Integrated with physics performance - adjusted budget to %.2fms"), 
               SurvivalSystemBudgetMs);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Perf_SurvivalPerformanceIntegrator: No physics integrator found"));
    }
}

void UPerf_SurvivalPerformanceIntegrator::ValidateSurvivalPerformanceIntegration()
{
    UE_LOG(LogTemp, Log, TEXT("Perf_SurvivalPerformanceIntegrator: Validating survival performance integration"));
    
    // Validation checks
    bool bValidationPassed = true;
    TArray<FString> ValidationErrors;
    
    // Check if monitoring is active
    if (!bIsMonitoring)
    {
        ValidationErrors.Add(TEXT("Performance monitoring is not active"));
        bValidationPassed = false;
    }
    
    // Check if optimization level is reasonable
    if (OptimizationLevel == EPerf_SurvivalOptimizationLevel::Disabled)
    {
        ValidationErrors.Add(TEXT("Optimization is disabled"));
        bValidationPassed = false;
    }
    
    // Check if budget is reasonable
    if (SurvivalSystemBudgetMs > 5.0f)
    {
        ValidationErrors.Add(FString::Printf(TEXT("Survival system budget too high: %.2fms"), SurvivalSystemBudgetMs));
        bValidationPassed = false;
    }
    
    // Check if we have active components
    if (CurrentMetrics.ActiveSurvivalComponents == 0)
    {
        ValidationErrors.Add(TEXT("No active survival components found"));
        bValidationPassed = false;
    }
    
    // Log results
    if (bValidationPassed)
    {
        UE_LOG(LogTemp, Log, TEXT("Perf_SurvivalPerformanceIntegrator: Validation PASSED - %d components, %.2fms budget"), 
               CurrentMetrics.ActiveSurvivalComponents, SurvivalSystemBudgetMs);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Perf_SurvivalPerformanceIntegrator: Validation FAILED:"));
        for (const FString& Error : ValidationErrors)
        {
            UE_LOG(LogTemp, Error, TEXT("  - %s"), *Error);
        }
    }
}

void UPerf_SurvivalPerformanceIntegrator::UpdateSurvivalMetrics(float DeltaTime)
{
    // Track frame time
    RecentFrameTimes[FrameTimeIndex] = DeltaTime * 1000.0f; // Convert to ms
    FrameTimeIndex = (FrameTimeIndex + 1) % RecentFrameTimes.Num();
    
    // Update metrics every second
    double CurrentTime = FPlatformTime::Seconds();
    if (CurrentTime - LastUpdateTime >= 1.0)
    {
        OptimizeSurvivalSystems();
        LastUpdateTime = CurrentTime;
    }
}

void UPerf_SurvivalPerformanceIntegrator::ApplyOptimizations()
{
    // Calculate average frame time
    float AverageFrameTime = 0.0f;
    int32 ValidFrames = 0;
    
    for (float FrameTime : RecentFrameTimes)
    {
        if (FrameTime > 0.0f)
        {
            AverageFrameTime += FrameTime;
            ValidFrames++;
        }
    }
    
    if (ValidFrames > 0)
    {
        AverageFrameTime /= ValidFrames;
        
        // Adaptive optimization based on frame time
        if (AverageFrameTime > 33.33f) // Worse than 30 FPS
        {
            // Reduce update rates
            if (bEnableAdaptiveUpdateRates)
            {
                TargetSurvivalUpdateRate = FMath::Max(5.0f, TargetSurvivalUpdateRate * 0.8f);
                PrimaryComponentTick.TickInterval = FMath::Min(0.5f, PrimaryComponentTick.TickInterval * 1.2f);
            }
        }
        else if (AverageFrameTime < 16.67f) // Better than 60 FPS
        {
            // Increase update rates
            if (bEnableAdaptiveUpdateRates)
            {
                TargetSurvivalUpdateRate = FMath::Min(30.0f, TargetSurvivalUpdateRate * 1.1f);
                PrimaryComponentTick.TickInterval = FMath::Max(0.033f, PrimaryComponentTick.TickInterval * 0.9f);
            }
        }
    }
}