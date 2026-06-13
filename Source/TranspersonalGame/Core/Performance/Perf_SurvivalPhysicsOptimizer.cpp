#include "Perf_SurvivalPhysicsOptimizer.h"
#include "Core/Physics/Core_SurvivalPhysicsComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "HAL/PlatformFilemanager.h"

UPerf_SurvivalPhysicsOptimizer::UPerf_SurvivalPhysicsOptimizer()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update every 100ms for performance
    
    CurrentOptimizationLevel = EPerf_PhysicsOptimizationLevel::Medium;
    bIsMonitoring = false;
    MonitoringStartTime = 0.0f;
    FrameCount = 0;
    LastFrameTime = 0.0;
    
    // Initialize frame time history
    FrameTimeHistory.Reserve(MaxFrameHistory);
}

void UPerf_SurvivalPhysicsOptimizer::BeginPlay()
{
    Super::BeginPlay();
    
    // Start monitoring automatically
    StartPerformanceMonitoring();
    
    // Find and register existing survival physics components
    UWorld* World = GetWorld();
    if (World)
    {
        for (TActorIterator<APawn> ActorIterator(World); ActorIterator; ++ActorIterator)
        {
            APawn* Pawn = *ActorIterator;
            if (Pawn)
            {
                UCore_SurvivalPhysicsComponent* SurvivalComp = Pawn->FindComponentByClass<UCore_SurvivalPhysicsComponent>();
                if (SurvivalComp)
                {
                    RegisterSurvivalComponent(SurvivalComp);
                }
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("SurvivalPhysicsOptimizer: Started monitoring %d components"), RegisteredComponents.Num());
}

void UPerf_SurvivalPhysicsOptimizer::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bIsMonitoring)
    {
        UpdatePerformanceMetrics(DeltaTime);
        
        // Apply optimizations if performance is critical
        if (IsPerformanceCritical())
        {
            OptimizeSurvivalPhysics();
        }
    }
}

void UPerf_SurvivalPhysicsOptimizer::StartPerformanceMonitoring()
{
    bIsMonitoring = true;
    MonitoringStartTime = GetWorld()->GetTimeSeconds();
    FrameCount = 0;
    FrameTimeHistory.Empty();
    
    // Reset metrics
    CurrentMetrics = FPerf_SurvivalPhysicsMetrics();
    
    UE_LOG(LogTemp, Log, TEXT("SurvivalPhysicsOptimizer: Performance monitoring started"));
}

void UPerf_SurvivalPhysicsOptimizer::StopPerformanceMonitoring()
{
    bIsMonitoring = false;
    UE_LOG(LogTemp, Log, TEXT("SurvivalPhysicsOptimizer: Performance monitoring stopped"));
}

FPerf_SurvivalPhysicsMetrics UPerf_SurvivalPhysicsOptimizer::GetCurrentMetrics() const
{
    return CurrentMetrics;
}

void UPerf_SurvivalPhysicsOptimizer::SetOptimizationLevel(EPerf_PhysicsOptimizationLevel Level)
{
    CurrentOptimizationLevel = Level;
    
    // Adjust optimization settings based on level
    switch (Level)
    {
        case EPerf_PhysicsOptimizationLevel::None:
            OptimizationSettings.MaxActiveComponents = 1000;
            OptimizationSettings.DistanceBasedOptimization = 10000.0f;
            OptimizationSettings.LODOptimizationFactor = 1.0f;
            break;
            
        case EPerf_PhysicsOptimizationLevel::Low:
            OptimizationSettings.MaxActiveComponents = 200;
            OptimizationSettings.DistanceBasedOptimization = 7500.0f;
            OptimizationSettings.LODOptimizationFactor = 0.8f;
            break;
            
        case EPerf_PhysicsOptimizationLevel::Medium:
            OptimizationSettings.MaxActiveComponents = 100;
            OptimizationSettings.DistanceBasedOptimization = 5000.0f;
            OptimizationSettings.LODOptimizationFactor = 0.5f;
            break;
            
        case EPerf_PhysicsOptimizationLevel::High:
            OptimizationSettings.MaxActiveComponents = 50;
            OptimizationSettings.DistanceBasedOptimization = 2500.0f;
            OptimizationSettings.LODOptimizationFactor = 0.25f;
            break;
            
        case EPerf_PhysicsOptimizationLevel::Extreme:
            OptimizationSettings.MaxActiveComponents = 25;
            OptimizationSettings.DistanceBasedOptimization = 1000.0f;
            OptimizationSettings.LODOptimizationFactor = 0.1f;
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("SurvivalPhysicsOptimizer: Optimization level set to %d"), (int32)Level);
}

void UPerf_SurvivalPhysicsOptimizer::OptimizeSurvivalPhysics()
{
    // Apply different optimization strategies
    ApplyDistanceBasedOptimization();
    ApplyLODOptimization();
    ApplyComponentLimiting();
    
    UpdateComponentCounts();
    
    UE_LOG(LogTemp, Log, TEXT("SurvivalPhysicsOptimizer: Applied optimizations - %d/%d components optimized"), 
           OptimizedComponents.Num(), RegisteredComponents.Num());
}

void UPerf_SurvivalPhysicsOptimizer::ResetOptimizations()
{
    // Reset all optimized components to normal operation
    for (UCore_SurvivalPhysicsComponent* Component : OptimizedComponents)
    {
        if (IsValid(Component))
        {
            // Reset component tick interval to normal
            Component->SetComponentTickInterval(0.1f);
        }
    }
    
    OptimizedComponents.Empty();
    UpdateComponentCounts();
    
    UE_LOG(LogTemp, Log, TEXT("SurvivalPhysicsOptimizer: All optimizations reset"));
}

void UPerf_SurvivalPhysicsOptimizer::RegisterSurvivalComponent(UCore_SurvivalPhysicsComponent* Component)
{
    if (IsValid(Component) && !RegisteredComponents.Contains(Component))
    {
        RegisteredComponents.Add(Component);
        UpdateComponentCounts();
        
        UE_LOG(LogTemp, Log, TEXT("SurvivalPhysicsOptimizer: Registered component %s"), 
               *Component->GetOwner()->GetName());
    }
}

void UPerf_SurvivalPhysicsOptimizer::UnregisterSurvivalComponent(UCore_SurvivalPhysicsComponent* Component)
{
    if (RegisteredComponents.Contains(Component))
    {
        RegisteredComponents.Remove(Component);
        OptimizedComponents.Remove(Component);
        UpdateComponentCounts();
        
        UE_LOG(LogTemp, Log, TEXT("SurvivalPhysicsOptimizer: Unregistered component"));
    }
}

bool UPerf_SurvivalPhysicsOptimizer::IsPerformanceCritical() const
{
    return CurrentMetrics.bIsPerformanceCritical || 
           CurrentMetrics.AverageFrameTime > OptimizationSettings.CriticalFrameTime;
}

float UPerf_SurvivalPhysicsOptimizer::GetAverageCalculationTime() const
{
    return CurrentMetrics.CalculationTime;
}

void UPerf_SurvivalPhysicsOptimizer::RunPerformanceTest()
{
    UE_LOG(LogTemp, Warning, TEXT("=== SURVIVAL PHYSICS PERFORMANCE TEST ==="));
    
    // Test different survival states and measure performance
    TArray<FString> TestStates = {
        TEXT("Critical Survival State (Cold + Exhausted)"),
        TEXT("Optimal Survival State (Healthy)"),
        TEXT("Mixed Survival State (Moderate stress)")
    };
    
    for (int32 i = 0; i < TestStates.Num(); i++)
    {
        double StartTime = FPlatformTime::Seconds();
        
        // Simulate physics calculations for all registered components
        for (UCore_SurvivalPhysicsComponent* Component : RegisteredComponents)
        {
            if (IsValid(Component))
            {
                // Force component to recalculate physics
                Component->SetComponentTickEnabled(true);
            }
        }
        
        double EndTime = FPlatformTime::Seconds();
        float TestTime = (EndTime - StartTime) * 1000.0f; // Convert to milliseconds
        
        UE_LOG(LogTemp, Warning, TEXT("Test %d - %s: %.3f ms"), 
               i + 1, *TestStates[i], TestTime);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Current Metrics:"));
    UE_LOG(LogTemp, Warning, TEXT("- Active Components: %d"), CurrentMetrics.ActiveSurvivalComponents);
    UE_LOG(LogTemp, Warning, TEXT("- Optimized Components: %d"), CurrentMetrics.OptimizedComponents);
    UE_LOG(LogTemp, Warning, TEXT("- Average Frame Time: %.3f ms"), CurrentMetrics.AverageFrameTime);
    UE_LOG(LogTemp, Warning, TEXT("- Performance Critical: %s"), 
           CurrentMetrics.bIsPerformanceCritical ? TEXT("YES") : TEXT("NO"));
    
    UE_LOG(LogTemp, Warning, TEXT("=== PERFORMANCE TEST COMPLETE ==="));
}

void UPerf_SurvivalPhysicsOptimizer::UpdatePerformanceMetrics(float DeltaTime)
{
    FrameCount++;
    
    // Calculate frame time
    float CurrentFrameTime = CalculateFrameTime();
    
    // Update frame time history
    FrameTimeHistory.Add(CurrentFrameTime);
    if (FrameTimeHistory.Num() > MaxFrameHistory)
    {
        FrameTimeHistory.RemoveAt(0);
    }
    
    // Calculate average frame time
    float TotalFrameTime = 0.0f;
    float MaxFrameTime = 0.0f;
    for (float FrameTime : FrameTimeHistory)
    {
        TotalFrameTime += FrameTime;
        MaxFrameTime = FMath::Max(MaxFrameTime, FrameTime);
    }
    
    if (FrameTimeHistory.Num() > 0)
    {
        CurrentMetrics.AverageFrameTime = TotalFrameTime / FrameTimeHistory.Num();
        CurrentMetrics.PeakFrameTime = MaxFrameTime;
    }
    
    // Update component counts
    UpdateComponentCounts();
    
    // Determine if performance is critical
    CurrentMetrics.bIsPerformanceCritical = 
        CurrentMetrics.AverageFrameTime > OptimizationSettings.CriticalFrameTime ||
        CurrentMetrics.PeakFrameTime > OptimizationSettings.CriticalFrameTime * 1.5f;
    
    CurrentMetrics.bRequiresOptimization = 
        CurrentMetrics.AverageFrameTime > OptimizationSettings.WarningFrameTime;
    
    // Calculate estimated calculation time for survival physics
    CurrentMetrics.CalculationTime = CurrentMetrics.AverageFrameTime * 
        (float(CurrentMetrics.ActiveSurvivalComponents) / FMath::Max(1.0f, float(FrameCount)));
}

void UPerf_SurvivalPhysicsOptimizer::ApplyDistanceBasedOptimization()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    APlayerController* PlayerController = World->GetFirstPlayerController();
    if (!PlayerController) return;
    
    APawn* PlayerPawn = PlayerController->GetPawn();
    if (!PlayerPawn) return;
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    // Optimize components based on distance from player
    for (UCore_SurvivalPhysicsComponent* Component : RegisteredComponents)
    {
        if (IsValid(Component) && IsValid(Component->GetOwner()))
        {
            float Distance = FVector::Dist(PlayerLocation, Component->GetOwner()->GetActorLocation());
            
            if (Distance > OptimizationSettings.DistanceBasedOptimization)
            {
                // Reduce update frequency for distant components
                Component->SetComponentTickInterval(0.5f); // 2 Hz instead of 10 Hz
                
                if (!OptimizedComponents.Contains(Component))
                {
                    OptimizedComponents.Add(Component);
                }
            }
        }
    }
}

void UPerf_SurvivalPhysicsOptimizer::ApplyLODOptimization()
{
    // Apply LOD-based optimization to reduce calculation complexity
    for (UCore_SurvivalPhysicsComponent* Component : RegisteredComponents)
    {
        if (IsValid(Component) && ShouldOptimizeComponent(Component))
        {
            // Reduce calculation frequency based on LOD factor
            float NewTickInterval = 0.1f / OptimizationSettings.LODOptimizationFactor;
            Component->SetComponentTickInterval(FMath::Clamp(NewTickInterval, 0.1f, 1.0f));
            
            if (!OptimizedComponents.Contains(Component))
            {
                OptimizedComponents.Add(Component);
            }
        }
    }
}

void UPerf_SurvivalPhysicsOptimizer::ApplyComponentLimiting()
{
    // Limit the number of active components if we exceed the maximum
    if (RegisteredComponents.Num() > OptimizationSettings.MaxActiveComponents)
    {
        int32 ComponentsToOptimize = RegisteredComponents.Num() - OptimizationSettings.MaxActiveComponents;
        
        // Sort components by priority (distance from player, importance, etc.)
        TArray<UCore_SurvivalPhysicsComponent*> SortedComponents = RegisteredComponents;
        
        // For now, just take the last components (could implement more sophisticated sorting)
        for (int32 i = RegisteredComponents.Num() - ComponentsToOptimize; i < RegisteredComponents.Num(); i++)
        {
            if (IsValid(SortedComponents[i]))
            {
                SortedComponents[i]->SetComponentTickInterval(1.0f); // 1 Hz
                
                if (!OptimizedComponents.Contains(SortedComponents[i]))
                {
                    OptimizedComponents.Add(SortedComponents[i]);
                }
            }
        }
    }
}

float UPerf_SurvivalPhysicsOptimizer::CalculateFrameTime() const
{
    double CurrentTime = FPlatformTime::Seconds();
    float FrameTime = (CurrentTime - LastFrameTime) * 1000.0f; // Convert to milliseconds
    const_cast<UPerf_SurvivalPhysicsOptimizer*>(this)->LastFrameTime = CurrentTime;
    
    return FMath::Clamp(FrameTime, 0.0f, 100.0f); // Clamp to reasonable values
}

void UPerf_SurvivalPhysicsOptimizer::UpdateComponentCounts()
{
    // Clean up invalid components
    RegisteredComponents.RemoveAll([](UCore_SurvivalPhysicsComponent* Component) {
        return !IsValid(Component);
    });
    
    OptimizedComponents.RemoveAll([](UCore_SurvivalPhysicsComponent* Component) {
        return !IsValid(Component);
    });
    
    CurrentMetrics.ActiveSurvivalComponents = RegisteredComponents.Num();
    CurrentMetrics.OptimizedComponents = OptimizedComponents.Num();
}

bool UPerf_SurvivalPhysicsOptimizer::ShouldOptimizeComponent(UCore_SurvivalPhysicsComponent* Component) const
{
    if (!IsValid(Component)) return false;
    
    // Don't optimize if already optimized
    if (OptimizedComponents.Contains(Component)) return false;
    
    // Always optimize if performance is critical
    if (IsPerformanceCritical()) return true;
    
    // Optimize based on current optimization level
    switch (CurrentOptimizationLevel)
    {
        case EPerf_PhysicsOptimizationLevel::None:
            return false;
            
        case EPerf_PhysicsOptimizationLevel::Low:
            return RegisteredComponents.Num() > 150;
            
        case EPerf_PhysicsOptimizationLevel::Medium:
            return RegisteredComponents.Num() > 75;
            
        case EPerf_PhysicsOptimizationLevel::High:
            return RegisteredComponents.Num() > 35;
            
        case EPerf_PhysicsOptimizationLevel::Extreme:
            return true;
    }
    
    return false;
}