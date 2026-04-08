/**
 * @file PhysicsOptimizer.cpp
 * @brief Implementation of consciousness physics performance optimization
 */

#include "PhysicsOptimizer.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "HAL/PlatformFilemanager.h"
#include "Stats/Stats.h"

DECLARE_STATS_GROUP(TEXT("Consciousness Physics"), STATGROUP_ConsciousnessPhysics, STATCAT_Advanced);
DECLARE_CYCLE_STAT(TEXT("Consciousness Field Update"), STAT_ConsciousnessFieldUpdate, STATGROUP_ConsciousnessPhysics);
DECLARE_CYCLE_STAT(TEXT("Consciousness LOD Update"), STAT_ConsciousnessLODUpdate, STATGROUP_ConsciousnessPhysics);
DECLARE_DWORD_COUNTER_STAT(TEXT("Active Consciousness Actors"), STAT_ActiveConsciousnessActors, STATGROUP_ConsciousnessPhysics);

UPhysicsOptimizer::UPhysicsOptimizer()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = PerformanceCheckInterval;
    
    // Initialize frame time history
    FrameTimeHistory.Reserve(FrameHistorySize);
    for (int32 i = 0; i < FrameHistorySize; ++i)
    {
        FrameTimeHistory.Add(1.0f / 60.0f); // Start with 60 FPS assumption
    }
}

void UPhysicsOptimizer::BeginPlay()
{
    Super::BeginPlay();
    
    // Set initial LOD based on platform
    if (GEngine && GEngine->GetGameUserSettings())
    {
        int32 QualityLevel = GEngine->GetGameUserSettings()->GetOverallScalabilityLevel();
        switch (QualityLevel)
        {
            case 0: SetConsciousnessLOD(EConsciousnessLOD::Minimal); break;
            case 1: SetConsciousnessLOD(EConsciousnessLOD::Low); break;
            case 2: SetConsciousnessLOD(EConsciousnessLOD::Medium); break;
            default: SetConsciousnessLOD(EConsciousnessLOD::High); break;
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("PhysicsOptimizer initialized with LOD: %d"), (int32)CurrentLOD);
}

void UPhysicsOptimizer::TickComponent(float DeltaTime, ELevelTick TickType, 
                                    FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdatePerformanceMetrics(DeltaTime);
    
    PerformanceTimer += DeltaTime;
    if (PerformanceTimer >= PerformanceCheckInterval)
    {
        if (bAdaptiveQualityEnabled)
        {
            CheckAdaptiveQuality();
        }
        
        UpdateConsciousnessActorCulling();
        PerformanceTimer = 0.0f;
    }
}

void UPhysicsOptimizer::UpdatePerformanceMetrics(float DeltaTime)
{
    SCOPE_CYCLE_COUNTER(STAT_ConsciousnessFieldUpdate);
    
    // Update frame time history
    FrameTimeHistory.RemoveAt(0);
    FrameTimeHistory.Add(DeltaTime);
    
    // Calculate average frame time
    float TotalFrameTime = 0.0f;
    for (float FrameTime : FrameTimeHistory)
    {
        TotalFrameTime += FrameTime;
    }
    CurrentMetrics.FrameTime = TotalFrameTime / FrameHistorySize;
    
    // Count active consciousness actors
    CurrentMetrics.ActiveConsciousnessActors = 0;
    if (UWorld* World = GetWorld())
    {
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor && Actor->GetClass()->GetName().Contains(TEXT("Consciousness")))
            {
                CurrentMetrics.ActiveConsciousnessActors++;
            }
        }
    }
    
    SET_DWORD_STAT(STAT_ActiveConsciousnessActors, CurrentMetrics.ActiveConsciousnessActors);
    
    // Estimate physics calculations per frame
    CurrentMetrics.PhysicsCalculationsPerFrame = CurrentMetrics.ActiveConsciousnessActors * 
                                               (CurrentLOD == EConsciousnessLOD::High ? 100 : 
                                                CurrentLOD == EConsciousnessLOD::Medium ? 50 : 
                                                CurrentLOD == EConsciousnessLOD::Low ? 25 : 10);
    
    // Calculate consciousness field complexity
    CurrentMetrics.ConsciousnessFieldComplexity = FMath::Clamp(
        CurrentMetrics.ActiveConsciousnessActors / 100.0f, 0.0f, 1.0f);
    
    // Estimate memory usage (simplified)
    CurrentMetrics.MemoryUsageMB = CurrentMetrics.ActiveConsciousnessActors * 0.5f + 
                                  CurrentMetrics.PhysicsCalculationsPerFrame * 0.001f;
}

void UPhysicsOptimizer::CheckAdaptiveQuality()
{
    float CurrentFPS = 1.0f / CurrentMetrics.FrameTime;
    EConsciousnessLOD OptimalLOD = CalculateOptimalLOD();
    
    if (OptimalLOD != CurrentLOD)
    {
        UE_LOG(LogTemp, Log, TEXT("Adaptive quality changing LOD from %d to %d (FPS: %.1f)"), 
               (int32)CurrentLOD, (int32)OptimalLOD, CurrentFPS);
        SetConsciousnessLOD(OptimalLOD);
    }
}

EConsciousnessLOD UPhysicsOptimizer::CalculateOptimalLOD() const
{
    float CurrentFPS = 1.0f / CurrentMetrics.FrameTime;
    float TargetFrameTime = 1.0f / TargetFrameRate;
    
    // Performance-based LOD selection
    if (CurrentMetrics.FrameTime > TargetFrameTime * 1.5f) // Significantly below target
    {
        return EConsciousnessLOD::Minimal;
    }
    else if (CurrentMetrics.FrameTime > TargetFrameTime * 1.2f) // Below target
    {
        return EConsciousnessLOD::Low;
    }
    else if (CurrentMetrics.FrameTime > TargetFrameTime * 1.1f) // Slightly below target
    {
        return EConsciousnessLOD::Medium;
    }
    else // Meeting or exceeding target
    {
        return EConsciousnessLOD::High;
    }
}

void UPhysicsOptimizer::SetConsciousnessLOD(EConsciousnessLOD NewLOD)
{
    if (CurrentLOD == NewLOD) return;
    
    CurrentLOD = NewLOD;
    ApplyLODSettings(NewLOD);
    
    // Broadcast LOD change to consciousness systems
    if (UWorld* World = GetWorld())
    {
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor && Actor->GetClass()->GetName().Contains(TEXT("Consciousness")))
            {
                // Call LOD update method if available
                if (UFunction* LODFunction = Actor->FindFunction(TEXT("OnConsciousnessLODChanged")))
                {
                    struct { EConsciousnessLOD NewLOD; } Params;
                    Params.NewLOD = NewLOD;
                    Actor->ProcessEvent(LODFunction, &Params);
                }
            }
        }
    }
}

void UPhysicsOptimizer::ApplyLODSettings(EConsciousnessLOD LOD)
{
    SCOPE_CYCLE_COUNTER(STAT_ConsciousnessLODUpdate);
    
    // Adjust tick intervals based on LOD
    float TickInterval = 0.0f;
    switch (LOD)
    {
        case EConsciousnessLOD::High:
            TickInterval = 0.0f; // Every frame
            break;
        case EConsciousnessLOD::Medium:
            TickInterval = 1.0f / 30.0f; // 30 FPS
            break;
        case EConsciousnessLOD::Low:
            TickInterval = 1.0f / 15.0f; // 15 FPS
            break;
        case EConsciousnessLOD::Minimal:
            TickInterval = 1.0f / 10.0f; // 10 FPS
            break;
    }
    
    // Update our own tick interval
    PrimaryComponentTick.TickInterval = FMath::Max(TickInterval, PerformanceCheckInterval);
}

void UPhysicsOptimizer::UpdateConsciousnessActorCulling()
{
    if (!GetWorld()) return;
    
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PC || !PC->GetPawn()) return;
    
    FVector PlayerLocation = PC->GetPawn()->GetActorLocation();
    
    for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && Actor->GetClass()->GetName().Contains(TEXT("Consciousness")))
        {
            float Distance = FVector::Dist(PlayerLocation, Actor->GetActorLocation());
            
            // Cull actors beyond maximum distance
            bool bShouldBeVisible = Distance <= ConsciousnessCullingDistance;
            if (Actor->GetRootComponent())
            {
                Actor->GetRootComponent()->SetVisibility(bShouldBeVisible, true);
            }
            
            // Update LOD based on distance
            if (bShouldBeVisible)
            {
                UpdateConsciousnessActorLOD(Cast<class AConsciousnessActor>(Actor), Distance);
            }
        }
    }
}

void UPhysicsOptimizer::UpdateConsciousnessActorLOD(class AConsciousnessActor* Actor, float Distance)
{
    if (!Actor) return;
    
    EConsciousnessLOD DistanceLOD = EConsciousnessLOD::High;
    
    if (Distance > ConsciousnessLODDistance3)
    {
        DistanceLOD = EConsciousnessLOD::Minimal;
    }
    else if (Distance > ConsciousnessLODDistance2)
    {
        DistanceLOD = EConsciousnessLOD::Low;
    }
    else if (Distance > ConsciousnessLODDistance1)
    {
        DistanceLOD = EConsciousnessLOD::Medium;
    }
    
    // Use the more restrictive LOD between global and distance-based
    EConsciousnessLOD FinalLOD = (CurrentLOD > DistanceLOD) ? CurrentLOD : DistanceLOD;
    
    // Apply LOD to actor if it has the appropriate method
    if (UFunction* SetLODFunction = Actor->FindFunction(TEXT("SetConsciousnessLOD")))
    {
        struct { EConsciousnessLOD LOD; } Params;
        Params.LOD = FinalLOD;
        Actor->ProcessEvent(SetLODFunction, &Params);
    }
}

FPhysicsPerformanceMetrics UPhysicsOptimizer::GetCurrentMetrics() const
{
    return CurrentMetrics;
}

void UPhysicsOptimizer::SetTargetFrameRate(float TargetFPS)
{
    TargetFrameRate = FMath::Clamp(TargetFPS, 30.0f, 120.0f);
}

void UPhysicsOptimizer::EnableAdaptiveQuality(bool bEnable)
{
    bAdaptiveQualityEnabled = bEnable;
    UE_LOG(LogTemp, Log, TEXT("Adaptive quality %s"), bEnable ? TEXT("enabled") : TEXT("disabled"));
}

void UPhysicsOptimizer::SetConsciousnessCullingDistance(float Distance)
{
    ConsciousnessCullingDistance = FMath::Max(Distance, 100.0f);
}

void UPhysicsOptimizer::OptimizeConsciousnessMemory()
{
    // Force garbage collection for consciousness-related objects
    if (GEngine)
    {
        GEngine->ForceGarbageCollection(true);
    }
    
    // Clear unused consciousness field data
    FlushUnusedConsciousnessData();
    
    UE_LOG(LogTemp, Log, TEXT("Consciousness memory optimization completed"));
}

void UPhysicsOptimizer::FlushUnusedConsciousnessData()
{
    // Implementation would clear cached consciousness field calculations,
    // unused spiritual energy pools, and other consciousness-specific data
    // This is a placeholder for the actual implementation
    
    UE_LOG(LogTemp, Log, TEXT("Flushed unused consciousness data"));
}