#include "PhysicsOptimizer.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Components/PrimitiveComponent.h"
#include "../Core/ConsciousnessSystem.h"
#include "../Physics/PhysicsManager.h"

UPhysicsOptimizer::UPhysicsOptimizer()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.2f; // Update 5 times per second
    
    CurrentOptimizationLevel = EPhysicsOptimizationLevel::None;
    TargetPhysicsTime = 8.0f; // 8ms target for physics
    MaxAllowedPhysicsObjects = 500;
    ConsciousnessRadius = 2000.0f; // 20 meters
    EmotionalIntensityMultiplier = 1.0f;
    
    LastPhysicsTime = 0.0f;
    LastPhysicsObjectCount = 0;
    LastOptimizationUpdate = 0.0f;
    
    // Initialize optimization presets
    InitializeOptimizationPresets();
}

void UPhysicsOptimizer::BeginPlay()
{
    Super::BeginPlay();
    
    // Store default physics settings
    if (UPhysicsSettings* PhysicsSettings = UPhysicsSettings::Get())
    {
        DefaultSettings.PhysicsSubstepDeltaTime = PhysicsSettings->FixedFrameRate > 0 ? 1.0f / PhysicsSettings->FixedFrameRate : 0.016667f;
        DefaultSettings.MaxPhysicsSubsteps = PhysicsSettings->MaxSubsteps;
        DefaultSettings.SolverIterationCount = 8; // Default solver iterations
        DefaultSettings.bEnableAsyncPhysics = PhysicsSettings->bEnableAsyncScene;
        DefaultSettings.SleepThresholdMultiplier = 1.0f;
        DefaultSettings.CollisionDistanceCulling = 1000.0f;
    }
    
    CurrentSettings = DefaultSettings;
}

void UPhysicsOptimizer::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    LastOptimizationUpdate += DeltaTime;
    
    if (LastOptimizationUpdate >= 0.2f) // Update every 200ms
    {
        UpdatePerformanceMetrics();
        
        // Auto-adjust optimization level based on performance
        if (ShouldIncreaseOptimization())
        {
            EPhysicsOptimizationLevel NewLevel = static_cast<EPhysicsOptimizationLevel>(
                FMath::Min(static_cast<int32>(CurrentOptimizationLevel) + 1, 
                          static_cast<int32>(EPhysicsOptimizationLevel::Emergency)));
            SetOptimizationLevel(NewLevel);
        }
        else if (ShouldDecreaseOptimization())
        {
            EPhysicsOptimizationLevel NewLevel = static_cast<EPhysicsOptimizationLevel>(
                FMath::Max(static_cast<int32>(CurrentOptimizationLevel) - 1, 
                          static_cast<int32>(EPhysicsOptimizationLevel::None)));
            SetOptimizationLevel(NewLevel);
        }
        
        UpdatePhysicsLOD();
        LastOptimizationUpdate = 0.0f;
    }
}

void UPhysicsOptimizer::SetOptimizationLevel(EPhysicsOptimizationLevel Level)
{
    if (CurrentOptimizationLevel == Level)
        return;
        
    CurrentOptimizationLevel = Level;
    
    switch (Level)
    {
        case EPhysicsOptimizationLevel::None:
            ApplyOptimizationSettings(DefaultSettings);
            break;
        case EPhysicsOptimizationLevel::Light:
            ApplyOptimizationSettings(LightOptimization);
            break;
        case EPhysicsOptimizationLevel::Moderate:
            ApplyOptimizationSettings(ModerateOptimization);
            break;
        case EPhysicsOptimizationLevel::Aggressive:
            ApplyOptimizationSettings(AggressiveOptimization);
            break;
        case EPhysicsOptimizationLevel::Emergency:
            ApplyOptimizationSettings(EmergencyOptimization);
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Physics Optimizer: Set optimization level to %d"), static_cast<int32>(Level));
}

void UPhysicsOptimizer::ApplyCustomOptimizationSettings(const FPhysicsOptimizationSettings& Settings)
{
    CurrentOptimizationLevel = EPhysicsOptimizationLevel::None; // Mark as custom
    ApplyOptimizationSettings(Settings);
}

void UPhysicsOptimizer::ResetToDefaultSettings()
{
    SetOptimizationLevel(EPhysicsOptimizationLevel::None);
}

void UPhysicsOptimizer::OptimizeForConsciousnessDistance(float PlayerConsciousnessRadius)
{
    ConsciousnessRadius = PlayerConsciousnessRadius;
    
    // Get player location (simplified - would integrate with actual player consciousness system)
    if (APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn())
    {
        FVector PlayerLocation = PlayerPawn->GetActorLocation();
        
        // Apply distance-based optimization
        float OptimizationFactor = FMath::Clamp(ConsciousnessRadius / 5000.0f, 0.1f, 1.0f);
        OptimizePhysicsObjectsInRadius(PlayerLocation, ConsciousnessRadius, OptimizationFactor);
    }
}

void UPhysicsOptimizer::SetEmotionalPhysicsIntensity(float Intensity)
{
    EmotionalIntensityMultiplier = FMath::Clamp(Intensity, 0.1f, 3.0f);
    UpdateEmotionalPhysicsSettings();
}

float UPhysicsOptimizer::GetCurrentPhysicsTime() const
{
    return LastPhysicsTime;
}

int32 UPhysicsOptimizer::GetActivePhysicsObjectCount() const
{
    return LastPhysicsObjectCount;
}

bool UPhysicsOptimizer::IsPhysicsPerformanceOptimal() const
{
    return LastPhysicsTime <= TargetPhysicsTime && 
           LastPhysicsObjectCount <= MaxAllowedPhysicsObjects;
}

void UPhysicsOptimizer::UpdatePhysicsLOD()
{
    // Get player location for LOD calculations
    if (APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn())
    {
        FVector PlayerLocation = PlayerPawn->GetActorLocation();
        
        // Iterate through physics objects and apply LOD
        for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (!Actor || !Actor->GetRootComponent())
                continue;
                
            UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
            if (!PrimComp || !PrimComp->IsSimulatingPhysics())
                continue;
                
            float Distance = FVector::Dist(PlayerLocation, Actor->GetActorLocation());
            
            // Apply LOD based on distance and consciousness radius
            if (Distance > ConsciousnessRadius * 2.0f)
            {
                // Very far - disable physics simulation
                PrimComp->SetSimulatePhysics(false);
            }
            else if (Distance > ConsciousnessRadius)
            {
                // Far - reduce physics quality
                PrimComp->SetSimulatePhysics(true);
                // Could reduce collision complexity here
            }
            else
            {
                // Close - full physics quality
                PrimComp->SetSimulatePhysics(true);
            }
        }
    }
}

void UPhysicsOptimizer::InitializeOptimizationPresets()
{
    // Light Optimization
    LightOptimization = DefaultSettings;
    LightOptimization.MaxPhysicsSubsteps = 4;
    LightOptimization.SolverIterationCount = 6;
    LightOptimization.SleepThresholdMultiplier = 0.8f;
    
    // Moderate Optimization
    ModerateOptimization = DefaultSettings;
    ModerateOptimization.PhysicsSubstepDeltaTime = 0.02f; // 50Hz
    ModerateOptimization.MaxPhysicsSubsteps = 3;
    ModerateOptimization.SolverIterationCount = 4;
    ModerateOptimization.SleepThresholdMultiplier = 0.6f;
    ModerateOptimization.CollisionDistanceCulling = 800.0f;
    
    // Aggressive Optimization
    AggressiveOptimization = DefaultSettings;
    AggressiveOptimization.PhysicsSubstepDeltaTime = 0.025f; // 40Hz
    AggressiveOptimization.MaxPhysicsSubsteps = 2;
    AggressiveOptimization.SolverIterationCount = 2;
    AggressiveOptimization.SleepThresholdMultiplier = 0.4f;
    AggressiveOptimization.CollisionDistanceCulling = 600.0f;
    AggressiveOptimization.bEnableAsyncPhysics = false;
    
    // Emergency Optimization
    EmergencyOptimization = DefaultSettings;
    EmergencyOptimization.PhysicsSubstepDeltaTime = 0.033f; // 30Hz
    EmergencyOptimization.MaxPhysicsSubsteps = 1;
    EmergencyOptimization.SolverIterationCount = 1;
    EmergencyOptimization.SleepThresholdMultiplier = 0.2f;
    EmergencyOptimization.CollisionDistanceCulling = 400.0f;
    EmergencyOptimization.bEnableAsyncPhysics = false;
}

void UPhysicsOptimizer::ApplyOptimizationSettings(const FPhysicsOptimizationSettings& Settings)
{
    CurrentSettings = Settings;
    
    if (UPhysicsSettings* PhysicsSettings = UPhysicsSettings::Get())
    {
        // Apply settings to UE5 physics system
        PhysicsSettings->FixedFrameRate = 1.0f / Settings.PhysicsSubstepDeltaTime;
        PhysicsSettings->MaxSubsteps = Settings.MaxPhysicsSubsteps;
        PhysicsSettings->bEnableAsyncScene = Settings.bEnableAsyncPhysics;
    }
    
    // Apply consciousness-specific optimizations
    UpdateEmotionalPhysicsSettings();
    CullDistantPhysicsObjects();
}

void UPhysicsOptimizer::OptimizePhysicsObjectsInRadius(const FVector& Center, float Radius, float OptimizationFactor)
{
    for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor)
            continue;
            
        float Distance = FVector::Dist(Center, Actor->GetActorLocation());
        if (Distance <= Radius)
        {
            UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
            if (PrimComp && PrimComp->IsSimulatingPhysics())
            {
                // Adjust physics quality based on optimization factor
                float QualityMultiplier = FMath::Lerp(0.5f, 1.0f, OptimizationFactor);
                
                // This would adjust physics properties like:
                // - Collision complexity
                // - Update frequency
                // - Solver iterations for this specific object
            }
        }
    }
}

void UPhysicsOptimizer::UpdateEmotionalPhysicsSettings()
{
    // Adjust physics intensity based on emotional state
    // Higher emotional intensity = more responsive physics
    
    if (UPhysicsSettings* PhysicsSettings = UPhysicsSettings::Get())
    {
        float IntensityFactor = EmotionalIntensityMultiplier;
        
        // Adjust solver iterations based on emotional intensity
        int32 BaseIterations = DefaultSettings.SolverIterationCount;
        int32 AdjustedIterations = FMath::RoundToInt(BaseIterations * IntensityFactor);
        AdjustedIterations = FMath::Clamp(AdjustedIterations, 1, 16);
        
        // Apply emotional physics adjustments
        // This would integrate with the consciousness system to:
        // - Increase physics responsiveness during high emotional states
        // - Reduce physics quality during calm states to save performance
        // - Adjust gravity and forces based on emotional context
    }
}

void UPhysicsOptimizer::CullDistantPhysicsObjects()
{
    float CullingDistance = CurrentSettings.CollisionDistanceCulling;
    
    if (APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn())
    {
        FVector PlayerLocation = PlayerPawn->GetActorLocation();
        
        for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (!Actor)
                continue;
                
            float Distance = FVector::Dist(PlayerLocation, Actor->GetActorLocation());
            
            UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
            if (PrimComp && Distance > CullingDistance)
            {
                // Disable physics for very distant objects
                if (PrimComp->IsSimulatingPhysics())
                {
                    PrimComp->SetSimulatePhysics(false);
                }
            }
        }
    }
}

void UPhysicsOptimizer::UpdatePerformanceMetrics()
{
    // Get physics timing from PhysicsManager
    if (UPhysicsManager* PhysicsManager = UPhysicsManager::GetInstance(GetWorld()))
    {
        LastPhysicsTime = PhysicsManager->GetLastFramePhysicsTime();
        LastPhysicsObjectCount = PhysicsManager->GetActivePhysicsObjectCount();
    }
}

bool UPhysicsOptimizer::ShouldIncreaseOptimization() const
{
    return LastPhysicsTime > TargetPhysicsTime * 1.2f || // 20% over target
           LastPhysicsObjectCount > MaxAllowedPhysicsObjects;
}

bool UPhysicsOptimizer::ShouldDecreaseOptimization() const
{
    return LastPhysicsTime < TargetPhysicsTime * 0.7f && // 30% under target
           LastPhysicsObjectCount < MaxAllowedPhysicsObjects * 0.8f &&
           CurrentOptimizationLevel != EPhysicsOptimizationLevel::None;
}