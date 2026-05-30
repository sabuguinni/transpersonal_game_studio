#include "Perf_SurvivalPerformanceOptimizer.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Kismet/GameplayStatics.h"
#include "HAL/PlatformFilemanager.h"
#include "../Core/Physics/Core_SurvivalPhysics.h"

UPerf_SurvivalPerformanceOptimizer::UPerf_SurvivalPerformanceOptimizer()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Check performance every 100ms
    
    OptimizationLevel = EPerf_SurvivalOptimizationLevel::Medium;
    TargetFrameRate = 60.0f;
    MinFrameRate = 30.0f;
    SurvivalUpdateInterval = 0.1f;
    HealthyStateUpdateInterval = 0.5f;
    CriticalStateUpdateInterval = 0.05f;
    MaxSurvivalComponentsPerFrame = 50;
}

void UPerf_SurvivalPerformanceOptimizer::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Survival Performance Optimizer initialized - Target FPS: %.1f"), TargetFrameRate);
    
    // Initialize performance tracking
    CurrentMetrics = FPerf_SurvivalPerformanceMetrics();
    LastOptimizationTime = GetWorld()->GetTimeSeconds();
    
    // Find all survival components in the world
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
    
    for (AActor* Actor : AllActors)
    {
        if (UCore_SurvivalPhysics* SurvivalComp = Actor->FindComponentByClass<UCore_SurvivalPhysics>())
        {
            TrackedSurvivalComponents.Add(SurvivalComp);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Found %d survival components to optimize"), TrackedSurvivalComponents.Num());
    
    // Apply initial optimization settings
    ApplyOptimizationSettings();
}

void UPerf_SurvivalPerformanceOptimizer::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Check if it's time for optimization update
    if (CurrentTime - LastOptimizationTime >= OptimizationCheckInterval)
    {
        UpdatePerformanceMetrics();
        OptimizeBasedOnFrameRate();
        LastOptimizationTime = CurrentTime;
    }
    
    // Pool survival component updates to spread load across frames
    PoolSurvivalUpdates();
}

void UPerf_SurvivalPerformanceOptimizer::OptimizeSurvivalSystems()
{
    UE_LOG(LogTemp, Warning, TEXT("Optimizing survival systems - Level: %d"), (int32)OptimizationLevel);
    
    OptimizeSurvivalTickFrequency();
    OptimizeStatUpdateFrequency();
    OptimizePhysicsMultiplierUpdates();
    
    UpdatePerformanceMetrics();
}

void UPerf_SurvivalPerformanceOptimizer::SetOptimizationLevel(EPerf_SurvivalOptimizationLevel NewLevel)
{
    OptimizationLevel = NewLevel;
    ApplyOptimizationSettings();
    
    UE_LOG(LogTemp, Warning, TEXT("Survival optimization level changed to: %d"), (int32)NewLevel);
}

FPerf_SurvivalPerformanceMetrics UPerf_SurvivalPerformanceOptimizer::GetPerformanceMetrics() const
{
    return CurrentMetrics;
}

void UPerf_SurvivalPerformanceOptimizer::OptimizeSurvivalTickFrequency()
{
    float CurrentFPS = 1.0f / GetWorld()->GetDeltaSeconds();
    
    // Adjust tick frequency based on performance
    if (CurrentFPS < MinFrameRate)
    {
        // Reduce tick frequency for all survival components
        for (UCore_SurvivalPhysics* SurvivalComp : TrackedSurvivalComponents)
        {
            if (IsValid(SurvivalComp))
            {
                SurvivalComp->SetComponentTickInterval(SurvivalUpdateInterval * 2.0f);
            }
        }
        UE_LOG(LogTemp, Warning, TEXT("Reduced survival tick frequency due to low FPS: %.1f"), CurrentFPS);
    }
    else if (CurrentFPS > TargetFrameRate)
    {
        // Can afford more frequent updates
        for (UCore_SurvivalPhysics* SurvivalComp : TrackedSurvivalComponents)
        {
            if (IsValid(SurvivalComp))
            {
                SurvivalComp->SetComponentTickInterval(SurvivalUpdateInterval);
            }
        }
    }
}

void UPerf_SurvivalPerformanceOptimizer::OptimizeStatUpdateFrequency()
{
    // Clean up invalid references
    TrackedSurvivalComponents.RemoveAll([](UCore_SurvivalPhysics* Comp) { return !IsValid(Comp); });
    
    CurrentMetrics.CriticalStateCharacters = 0;
    
    for (UCore_SurvivalPhysics* SurvivalComp : TrackedSurvivalComponents)
    {
        if (IsValid(SurvivalComp))
        {
            // Get current survival state (assuming we have access to this)
            // For now, we'll use a simplified approach based on component tick interval
            float CurrentTickInterval = SurvivalComp->GetComponentTickInterval();
            
            // Adjust update frequency based on survival state urgency
            if (CurrentTickInterval < CriticalStateUpdateInterval * 1.5f)
            {
                CurrentMetrics.CriticalStateCharacters++;
                // Keep fast updates for critical state
                SurvivalComp->SetComponentTickInterval(CriticalStateUpdateInterval);
            }
            else
            {
                // Use slower updates for healthy characters
                SurvivalComp->SetComponentTickInterval(HealthyStateUpdateInterval);
            }
        }
    }
}

void UPerf_SurvivalPerformanceOptimizer::OptimizePhysicsMultiplierUpdates()
{
    // Optimize physics multiplier calculations
    // These should only update when survival stats actually change significantly
    
    for (UCore_SurvivalPhysics* SurvivalComp : TrackedSurvivalComponents)
    {
        if (IsValid(SurvivalComp))
        {
            // Reduce physics multiplier update frequency for stable characters
            // This would need integration with the actual SurvivalPhysics component
            // For now, we adjust the overall component tick rate
            
            if (OptimizationLevel >= EPerf_SurvivalOptimizationLevel::High)
            {
                // High optimization: Update physics multipliers less frequently
                SurvivalComp->SetComponentTickInterval(SurvivalComp->GetComponentTickInterval() * 1.5f);
            }
        }
    }
}

bool UPerf_SurvivalPerformanceOptimizer::ShouldReduceSurvivalUpdates() const
{
    float CurrentFPS = 1.0f / GetWorld()->GetDeltaSeconds();
    return CurrentFPS < MinFrameRate || CurrentMetrics.SurvivalTickTime > 5.0f; // 5ms budget
}

void UPerf_SurvivalPerformanceOptimizer::UpdatePerformanceMetrics()
{
    double StartTime = FPlatformTime::Seconds();
    
    // Clean up invalid references
    TrackedSurvivalComponents.RemoveAll([](UCore_SurvivalPhysics* Comp) { return !IsValid(Comp); });
    
    CurrentMetrics.ActiveSurvivalComponents = TrackedSurvivalComponents.Num();
    
    // Calculate average tick times
    float TotalTickTime = 0.0f;
    int32 ValidComponents = 0;
    
    for (UCore_SurvivalPhysics* SurvivalComp : TrackedSurvivalComponents)
    {
        if (IsValid(SurvivalComp))
        {
            // Estimate tick time based on component complexity
            TotalTickTime += SurvivalComp->GetComponentTickInterval();
            ValidComponents++;
        }
    }
    
    if (ValidComponents > 0)
    {
        CurrentMetrics.AverageStatUpdateTime = TotalTickTime / ValidComponents;
    }
    
    double EndTime = FPlatformTime::Seconds();
    CurrentMetrics.SurvivalTickTime = (EndTime - StartTime) * 1000.0f; // Convert to milliseconds
    
    UE_LOG(LogTemp, Log, TEXT("Survival Performance: %d components, %.2fms total, %.2fms avg"),
           CurrentMetrics.ActiveSurvivalComponents,
           CurrentMetrics.SurvivalTickTime,
           CurrentMetrics.AverageStatUpdateTime);
}

void UPerf_SurvivalPerformanceOptimizer::ApplyOptimizationSettings()
{
    switch (OptimizationLevel)
    {
        case EPerf_SurvivalOptimizationLevel::None:
            SurvivalUpdateInterval = 0.05f; // 20 FPS updates
            MaxSurvivalComponentsPerFrame = 100;
            break;
            
        case EPerf_SurvivalOptimizationLevel::Low:
            SurvivalUpdateInterval = 0.1f; // 10 FPS updates
            MaxSurvivalComponentsPerFrame = 75;
            break;
            
        case EPerf_SurvivalOptimizationLevel::Medium:
            SurvivalUpdateInterval = 0.2f; // 5 FPS updates
            MaxSurvivalComponentsPerFrame = 50;
            break;
            
        case EPerf_SurvivalOptimizationLevel::High:
            SurvivalUpdateInterval = 0.33f; // 3 FPS updates
            MaxSurvivalComponentsPerFrame = 25;
            break;
            
        case EPerf_SurvivalOptimizationLevel::Extreme:
            SurvivalUpdateInterval = 0.5f; // 2 FPS updates
            MaxSurvivalComponentsPerFrame = 10;
            break;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Applied survival optimization settings - Interval: %.2fs, Max per frame: %d"),
           SurvivalUpdateInterval, MaxSurvivalComponentsPerFrame);
}

void UPerf_SurvivalPerformanceOptimizer::OptimizeBasedOnFrameRate()
{
    float CurrentFPS = 1.0f / GetWorld()->GetDeltaSeconds();
    
    if (CurrentFPS < MinFrameRate)
    {
        // Automatically increase optimization level
        if (OptimizationLevel < EPerf_SurvivalOptimizationLevel::Extreme)
        {
            SetOptimizationLevel(static_cast<EPerf_SurvivalOptimizationLevel>((int32)OptimizationLevel + 1));
        }
    }
    else if (CurrentFPS > TargetFrameRate * 1.2f) // 20% above target
    {
        // Can reduce optimization level
        if (OptimizationLevel > EPerf_SurvivalOptimizationLevel::Low)
        {
            SetOptimizationLevel(static_cast<EPerf_SurvivalOptimizationLevel>((int32)OptimizationLevel - 1));
        }
    }
}

void UPerf_SurvivalPerformanceOptimizer::AdjustSurvivalTickRates()
{
    // Implement dynamic tick rate adjustment based on distance from player
    APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
    if (!PlayerPawn) return;
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    for (UCore_SurvivalPhysics* SurvivalComp : TrackedSurvivalComponents)
    {
        if (IsValid(SurvivalComp) && IsValid(SurvivalComp->GetOwner()))
        {
            float Distance = FVector::Dist(PlayerLocation, SurvivalComp->GetOwner()->GetActorLocation());
            
            // Adjust tick rate based on distance
            float DistanceMultiplier = FMath::Clamp(Distance / 5000.0f, 0.5f, 3.0f); // 5km max distance
            float AdjustedInterval = SurvivalUpdateInterval * DistanceMultiplier;
            
            SurvivalComp->SetComponentTickInterval(AdjustedInterval);
        }
    }
}

void UPerf_SurvivalPerformanceOptimizer::PoolSurvivalUpdates()
{
    // Spread survival component updates across multiple frames
    if (TrackedSurvivalComponents.Num() == 0) return;
    
    int32 ComponentsToUpdate = FMath::Min(MaxSurvivalComponentsPerFrame, TrackedSurvivalComponents.Num());
    
    for (int32 i = 0; i < ComponentsToUpdate; ++i)
    {
        int32 Index = (CurrentUpdateIndex + i) % TrackedSurvivalComponents.Num();
        
        if (IsValid(TrackedSurvivalComponents[Index]))
        {
            // Force a tick update for this component
            // This would need integration with the actual component's update logic
        }
    }
    
    CurrentUpdateIndex = (CurrentUpdateIndex + ComponentsToUpdate) % TrackedSurvivalComponents.Num();
}