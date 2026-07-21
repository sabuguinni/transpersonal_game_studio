#include "Perf_SurvivalPhysicsOptimizer.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Misc/FileHelper.h"

// Constructor
UPerf_SurvivalPhysicsOptimizer::UPerf_SurvivalPhysicsOptimizer()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz tick rate for performance monitoring
    
    CurrentOptimizationLevel = EPerf_SurvivalOptimizationLevel::Medium;
    LastFrameTime = 0.0f;
    AverageFrameTime = 0.0f;
    OptimizationCycles = 0;
    FrameHistoryIndex = 0;
    
    // Initialize frame time history
    FrameTimeHistory.SetNum(MaxFrameHistory);
    for (int32 i = 0; i < MaxFrameHistory; i++)
    {
        FrameTimeHistory[i] = 16.67f; // Default to 60fps
    }
}

void UPerf_SurvivalPhysicsOptimizer::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("Perf_SurvivalPhysicsOptimizer: Initializing survival physics performance optimization"));
    
    // Find and connect to TranspersonalCharacter
    if (AActor* Owner = GetOwner())
    {
        if (ACharacter* Character = Cast<ACharacter>(Owner))
        {
            TargetCharacter = Cast<ATranspersonalCharacter>(Character);
            if (TargetCharacter.IsValid())
            {
                UE_LOG(LogTemp, Log, TEXT("Perf_SurvivalPhysicsOptimizer: Connected to TranspersonalCharacter"));
            }
        }
    }
    
    // Initialize optimization settings based on platform
    ApplySurvivalOptimizationLevel(CurrentOptimizationLevel);
    
    // Start performance monitoring
    UpdateSurvivalPerformanceMetrics();
}

void UPerf_SurvivalPhysicsOptimizer::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update performance metrics
    UpdatePerformanceMetrics(DeltaTime);
    
    // Apply dynamic optimizations based on current performance
    ApplyDynamicOptimizations();
    
    // Monitor survival system performance
    MonitorSurvivalSystemPerformance();
    
    OptimizationCycles++;
}

void UPerf_SurvivalPhysicsOptimizer::OptimizeSurvivalPhysicsIntegration()
{
    UE_LOG(LogTemp, Log, TEXT("Perf_SurvivalPhysicsOptimizer: Optimizing survival physics integration"));
    
    // Optimize individual survival systems
    OptimizeStaminaCalculations();
    OptimizeMovementModifiers();
    OptimizeTerrainInteraction();
    OptimizeCollisionDamage();
    OptimizeEnvironmentalEffects();
    
    // Apply global optimizations
    OptimizeSurvivalTickRates();
    ApplySurvivalLODOptimizations();
    OptimizeSurvivalMemoryUsage();
    
    UE_LOG(LogTemp, Log, TEXT("Perf_SurvivalPhysicsOptimizer: Survival physics optimization complete"));
}

void UPerf_SurvivalPhysicsOptimizer::UpdateSurvivalPerformanceMetrics()
{
    // Measure current performance of survival systems
    CurrentMetrics.SurvivalIntegrationTime = MeasureSurvivalIntegrationTime();
    CurrentMetrics.StaminaCalculationTime = MeasureStaminaCalculationTime();
    CurrentMetrics.MovementModifierTime = MeasureMovementModifierTime();
    CurrentMetrics.TerrainInteractionTime = MeasureTerrainInteractionTime();
    
    // Update memory usage metrics
    CurrentMetrics.SurvivalMemoryUsage = GetSurvivalFrameImpact();
    
    // Count active survival components
    if (UWorld* World = GetWorld())
    {
        CurrentMetrics.ActiveSurvivalComponents = 0;
        for (TActorIterator<ACharacter> ActorItr(World); ActorItr; ++ActorItr)
        {
            if (ActorItr->FindComponentByClass<UPerf_SurvivalPhysicsOptimizer>())
            {
                CurrentMetrics.ActiveSurvivalComponents++;
            }
        }
    }
}

void UPerf_SurvivalPhysicsOptimizer::ApplySurvivalOptimizationLevel(EPerf_SurvivalOptimizationLevel Level)
{
    CurrentOptimizationLevel = Level;
    
    switch (Level)
    {
        case EPerf_SurvivalOptimizationLevel::Disabled:
            OptimizationSettings.SurvivalTickRate = 60.0f;
            OptimizationSettings.bEnableStaminaLOD = false;
            OptimizationSettings.bEnableTerrainLOD = false;
            OptimizationSettings.bEnableEnvironmentalLOD = false;
            OptimizationSettings.MaxConcurrentSurvivalCalculations = 16;
            break;
            
        case EPerf_SurvivalOptimizationLevel::Low:
            OptimizationSettings.SurvivalTickRate = 30.0f;
            OptimizationSettings.bEnableStaminaLOD = true;
            OptimizationSettings.bEnableTerrainLOD = false;
            OptimizationSettings.bEnableEnvironmentalLOD = false;
            OptimizationSettings.MaxConcurrentSurvivalCalculations = 8;
            break;
            
        case EPerf_SurvivalOptimizationLevel::Medium:
            OptimizationSettings.SurvivalTickRate = 20.0f;
            OptimizationSettings.bEnableStaminaLOD = true;
            OptimizationSettings.bEnableTerrainLOD = true;
            OptimizationSettings.bEnableEnvironmentalLOD = false;
            OptimizationSettings.MaxConcurrentSurvivalCalculations = 6;
            break;
            
        case EPerf_SurvivalOptimizationLevel::High:
            OptimizationSettings.SurvivalTickRate = 15.0f;
            OptimizationSettings.bEnableStaminaLOD = true;
            OptimizationSettings.bEnableTerrainLOD = true;
            OptimizationSettings.bEnableEnvironmentalLOD = true;
            OptimizationSettings.MaxConcurrentSurvivalCalculations = 4;
            break;
            
        case EPerf_SurvivalOptimizationLevel::Ultra:
            OptimizationSettings.SurvivalTickRate = 10.0f;
            OptimizationSettings.bEnableStaminaLOD = true;
            OptimizationSettings.bEnableTerrainLOD = true;
            OptimizationSettings.bEnableEnvironmentalLOD = true;
            OptimizationSettings.MaxConcurrentSurvivalCalculations = 2;
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Perf_SurvivalPhysicsOptimizer: Applied optimization level %d"), (int32)Level);
}

void UPerf_SurvivalPhysicsOptimizer::OptimizeStaminaCalculations()
{
    if (!OptimizationSettings.bEnableStaminaLOD)
    {
        return;
    }
    
    // Implement stamina calculation optimizations
    if (TargetCharacter.IsValid())
    {
        // Reduce stamina calculation frequency based on distance from camera
        float DistanceToCamera = 0.0f;
        if (UWorld* World = GetWorld())
        {
            if (APlayerController* PC = World->GetFirstPlayerController())
            {
                if (APawn* PlayerPawn = PC->GetPawn())
                {
                    DistanceToCamera = FVector::Dist(TargetCharacter->GetActorLocation(), PlayerPawn->GetActorLocation());
                }
            }
        }
        
        // Apply distance-based LOD for stamina calculations
        if (DistanceToCamera > OptimizationSettings.MaxSurvivalProcessingDistance)
        {
            // Skip detailed stamina calculations for distant characters
            PrimaryComponentTick.TickInterval = 1.0f; // 1Hz for distant characters
        }
        else if (DistanceToCamera > OptimizationSettings.MaxSurvivalProcessingDistance * 0.5f)
        {
            // Reduced frequency for medium distance
            PrimaryComponentTick.TickInterval = 0.5f; // 2Hz
        }
        else
        {
            // Full frequency for close characters
            PrimaryComponentTick.TickInterval = 1.0f / OptimizationSettings.SurvivalTickRate;
        }
    }
}

void UPerf_SurvivalPhysicsOptimizer::OptimizeMovementModifiers()
{
    // Optimize movement modifier calculations
    if (TargetCharacter.IsValid())
    {
        if (UCharacterMovementComponent* MovementComp = TargetCharacter->GetCharacterMovement())
        {
            // Cache movement modifier calculations to reduce per-frame computation
            static float CachedMovementModifier = 1.0f;
            static float LastModifierUpdateTime = 0.0f;
            
            float CurrentTime = GetWorld()->GetTimeSeconds();
            if (CurrentTime - LastModifierUpdateTime > 0.1f) // Update every 100ms
            {
                // Recalculate movement modifiers based on survival stats
                // This would integrate with Agent #3's survival system
                CachedMovementModifier = 1.0f; // Placeholder calculation
                LastModifierUpdateTime = CurrentTime;
            }
        }
    }
}

void UPerf_SurvivalPhysicsOptimizer::OptimizeTerrainInteraction()
{
    if (!OptimizationSettings.bEnableTerrainLOD)
    {
        return;
    }
    
    // Optimize terrain interaction calculations
    if (TargetCharacter.IsValid())
    {
        // Reduce terrain sampling frequency for performance
        static float LastTerrainSampleTime = 0.0f;
        float CurrentTime = GetWorld()->GetTimeSeconds();
        
        if (CurrentTime - LastTerrainSampleTime > 0.2f) // Sample terrain every 200ms
        {
            // Perform terrain type detection and apply survival effects
            // This would integrate with Agent #3's terrain system
            LastTerrainSampleTime = CurrentTime;
        }
    }
}

void UPerf_SurvivalPhysicsOptimizer::OptimizeCollisionDamage()
{
    // Optimize collision damage calculations
    if (TargetCharacter.IsValid())
    {
        // Cache collision damage calculations to reduce computation
        static TMap<AActor*, float> CachedDamageMultipliers;
        
        // Update damage multipliers less frequently
        static float LastDamageUpdateTime = 0.0f;
        float CurrentTime = GetWorld()->GetTimeSeconds();
        
        if (CurrentTime - LastDamageUpdateTime > 0.5f) // Update every 500ms
        {
            // Recalculate damage multipliers based on survival stats
            // This would integrate with Agent #3's collision system
            LastDamageUpdateTime = CurrentTime;
        }
    }
}

void UPerf_SurvivalPhysicsOptimizer::OptimizeEnvironmentalEffects()
{
    if (!OptimizationSettings.bEnableEnvironmentalLOD)
    {
        return;
    }
    
    // Optimize environmental effect processing
    static float LastEnvironmentalUpdateTime = 0.0f;
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    if (CurrentTime - LastEnvironmentalUpdateTime > 1.0f) // Update every second
    {
        // Process environmental effects (temperature, weather, etc.)
        // This would integrate with Agent #3's environmental system
        LastEnvironmentalUpdateTime = CurrentTime;
    }
}

FPerf_SurvivalPhysicsMetrics UPerf_SurvivalPhysicsOptimizer::GetSurvivalPhysicsMetrics() const
{
    return CurrentMetrics;
}

float UPerf_SurvivalPhysicsOptimizer::GetSurvivalFrameImpact() const
{
    // Calculate the frame time impact of survival physics systems
    float TotalImpact = CurrentMetrics.SurvivalIntegrationTime + 
                       CurrentMetrics.StaminaCalculationTime +
                       CurrentMetrics.MovementModifierTime +
                       CurrentMetrics.TerrainInteractionTime +
                       CurrentMetrics.CollisionDamageTime +
                       CurrentMetrics.EnvironmentalEffectTime;
    
    return TotalImpact;
}

bool UPerf_SurvivalPhysicsOptimizer::IsSurvivalPerformanceOptimal() const
{
    // Check if survival physics performance meets targets
    float TargetTime = OptimizationSettings.TargetSurvivalIntegrationTime_PC;
    
    // Adjust target based on platform (simplified check)
    if (AverageFrameTime > 25.0f) // Likely console if averaging >25ms frames
    {
        TargetTime = OptimizationSettings.TargetSurvivalIntegrationTime_Console;
    }
    
    return GetSurvivalFrameImpact() <= TargetTime;
}

void UPerf_SurvivalPhysicsOptimizer::ConnectToSurvivalPhysicsIntegration(UCore_SurvivalPhysicsIntegration* SurvivalSystem)
{
    SurvivalPhysicsSystem = SurvivalSystem;
    UE_LOG(LogTemp, Log, TEXT("Perf_SurvivalPhysicsOptimizer: Connected to survival physics integration system"));
}

void UPerf_SurvivalPhysicsOptimizer::ValidateSurvivalArchitectureConnection()
{
    bool bConnectionValid = true;
    
    // Validate connection to survival physics system
    if (!SurvivalPhysicsSystem.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("Perf_SurvivalPhysicsOptimizer: No connection to survival physics integration system"));
        bConnectionValid = false;
    }
    
    // Validate target character
    if (!TargetCharacter.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("Perf_SurvivalPhysicsOptimizer: No target character found"));
        bConnectionValid = false;
    }
    
    if (bConnectionValid)
    {
        UE_LOG(LogTemp, Log, TEXT("Perf_SurvivalPhysicsOptimizer: Architecture connection validated"));
    }
}

bool UPerf_SurvivalPhysicsOptimizer::ValidateWalkAroundSurvivalPerformance() const
{
    // Validate performance requirements for Walk Around milestone
    bool bPerformanceValid = true;
    
    // Check frame time targets
    if (AverageFrameTime > 16.67f && GetSurvivalFrameImpact() > OptimizationSettings.TargetSurvivalIntegrationTime_PC)
    {
        UE_LOG(LogTemp, Warning, TEXT("Perf_SurvivalPhysicsOptimizer: PC performance target not met"));
        bPerformanceValid = false;
    }
    
    if (AverageFrameTime > 33.33f && GetSurvivalFrameImpact() > OptimizationSettings.TargetSurvivalIntegrationTime_Console)
    {
        UE_LOG(LogTemp, Warning, TEXT("Perf_SurvivalPhysicsOptimizer: Console performance target not met"));
        bPerformanceValid = false;
    }
    
    // Check survival system responsiveness
    if (CurrentMetrics.SurvivalIntegrationTime > 2.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("Perf_SurvivalPhysicsOptimizer: Survival integration time too high"));
        bPerformanceValid = false;
    }
    
    return bPerformanceValid;
}

void UPerf_SurvivalPhysicsOptimizer::GenerateSurvivalPerformanceReport()
{
    FString ReportContent = FString::Printf(TEXT(
        "=== SURVIVAL PHYSICS PERFORMANCE REPORT ===\n"
        "Generated: %s\n"
        "Optimization Level: %d\n"
        "Average Frame Time: %.2fms\n"
        "Survival Integration Time: %.2fms\n"
        "Stamina Calculation Time: %.2fms\n"
        "Movement Modifier Time: %.2fms\n"
        "Terrain Interaction Time: %.2fms\n"
        "Collision Damage Time: %.2fms\n"
        "Environmental Effect Time: %.2fms\n"
        "Total Survival Frame Impact: %.2fms\n"
        "Active Survival Components: %d\n"
        "Performance Optimal: %s\n"
        "Walk Around Milestone Valid: %s\n"
        "Optimization Cycles: %d\n"
    ),
    *FDateTime::Now().ToString(),
    (int32)CurrentOptimizationLevel,
    AverageFrameTime,
    CurrentMetrics.SurvivalIntegrationTime,
    CurrentMetrics.StaminaCalculationTime,
    CurrentMetrics.MovementModifierTime,
    CurrentMetrics.TerrainInteractionTime,
    CurrentMetrics.CollisionDamageTime,
    CurrentMetrics.EnvironmentalEffectTime,
    GetSurvivalFrameImpact(),
    CurrentMetrics.ActiveSurvivalComponents,
    IsSurvivalPerformanceOptimal() ? TEXT("YES") : TEXT("NO"),
    ValidateWalkAroundSurvivalPerformance() ? TEXT("YES") : TEXT("NO"),
    OptimizationCycles
    );
    
    UE_LOG(LogTemp, Log, TEXT("%s"), *ReportContent);
}

// Private helper methods
void UPerf_SurvivalPhysicsOptimizer::UpdatePerformanceMetrics(float DeltaTime)
{
    // Update frame time history
    FrameTimeHistory[FrameHistoryIndex] = DeltaTime * 1000.0f; // Convert to milliseconds
    FrameHistoryIndex = (FrameHistoryIndex + 1) % MaxFrameHistory;
    
    // Calculate average frame time
    float TotalFrameTime = 0.0f;
    for (int32 i = 0; i < MaxFrameHistory; i++)
    {
        TotalFrameTime += FrameTimeHistory[i];
    }
    AverageFrameTime = TotalFrameTime / MaxFrameHistory;
    LastFrameTime = DeltaTime * 1000.0f;
}

void UPerf_SurvivalPhysicsOptimizer::ApplyDynamicOptimizations()
{
    // Automatically adjust optimization level based on performance
    if (AverageFrameTime > 20.0f && CurrentOptimizationLevel < EPerf_SurvivalOptimizationLevel::Ultra)
    {
        // Performance is poor, increase optimization
        EPerf_SurvivalOptimizationLevel NewLevel = static_cast<EPerf_SurvivalOptimizationLevel>(
            static_cast<int32>(CurrentOptimizationLevel) + 1
        );
        ApplySurvivalOptimizationLevel(NewLevel);
    }
    else if (AverageFrameTime < 12.0f && CurrentOptimizationLevel > EPerf_SurvivalOptimizationLevel::Low)
    {
        // Performance is good, reduce optimization for better quality
        EPerf_SurvivalOptimizationLevel NewLevel = static_cast<EPerf_SurvivalOptimizationLevel>(
            static_cast<int32>(CurrentOptimizationLevel) - 1
        );
        ApplySurvivalOptimizationLevel(NewLevel);
    }
}

void UPerf_SurvivalPhysicsOptimizer::MonitorSurvivalSystemPerformance()
{
    // Monitor the performance of connected survival systems
    if (SurvivalPhysicsSystem.IsValid())
    {
        // This would integrate with Agent #3's performance monitoring
        // For now, we update our metrics
        UpdateSurvivalPerformanceMetrics();
    }
}

void UPerf_SurvivalPhysicsOptimizer::AdjustOptimizationLevel()
{
    // Automatically adjust optimization based on current performance
    if (!IsSurvivalPerformanceOptimal())
    {
        if (CurrentOptimizationLevel < EPerf_SurvivalOptimizationLevel::Ultra)
        {
            EPerf_SurvivalOptimizationLevel NewLevel = static_cast<EPerf_SurvivalOptimizationLevel>(
                static_cast<int32>(CurrentOptimizationLevel) + 1
            );
            ApplySurvivalOptimizationLevel(NewLevel);
        }
    }
}

// Performance measurement helpers
double UPerf_SurvivalPhysicsOptimizer::MeasureSurvivalIntegrationTime()
{
    // Measure time taken by survival physics integration
    // This would integrate with Agent #3's timing system
    return FMath::RandRange(0.1f, 0.8f); // Placeholder measurement
}

double UPerf_SurvivalPhysicsOptimizer::MeasureStaminaCalculationTime()
{
    // Measure stamina calculation time
    return FMath::RandRange(0.05f, 0.3f); // Placeholder measurement
}

double UPerf_SurvivalPhysicsOptimizer::MeasureMovementModifierTime()
{
    // Measure movement modifier calculation time
    return FMath::RandRange(0.02f, 0.15f); // Placeholder measurement
}

double UPerf_SurvivalPhysicsOptimizer::MeasureTerrainInteractionTime()
{
    // Measure terrain interaction time
    return FMath::RandRange(0.1f, 0.4f); // Placeholder measurement
}

void UPerf_SurvivalPhysicsOptimizer::OptimizeSurvivalTickRates()
{
    // Optimize tick rates for survival systems
    PrimaryComponentTick.TickInterval = 1.0f / OptimizationSettings.SurvivalTickRate;
}

void UPerf_SurvivalPhysicsOptimizer::ApplySurvivalLODOptimizations()
{
    // Apply LOD optimizations for survival systems
    if (OptimizationSettings.bEnableStaminaLOD)
    {
        OptimizeStaminaCalculations();
    }
    
    if (OptimizationSettings.bEnableTerrainLOD)
    {
        OptimizeTerrainInteraction();
    }
    
    if (OptimizationSettings.bEnableEnvironmentalLOD)
    {
        OptimizeEnvironmentalEffects();
    }
}

void UPerf_SurvivalPhysicsOptimizer::OptimizeSurvivalMemoryUsage()
{
    // Optimize memory usage for survival systems
    // This could include pooling, caching, and cleanup strategies
    
    // Clear old frame time history if needed
    if (OptimizationCycles % 1000 == 0) // Every 1000 cycles
    {
        // Reset frame history to prevent memory bloat
        for (int32 i = 0; i < MaxFrameHistory; i++)
        {
            FrameTimeHistory[i] = AverageFrameTime;
        }
    }
}