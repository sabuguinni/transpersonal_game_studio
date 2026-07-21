// PerformanceOptimizer.cpp
// Prehistoric Dinosaur Survival Game — Performance Optimizer Implementation
// Agent #5 — Procedural World Generator

#include "PerformanceOptimizer.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

UPerformanceOptimizer::UPerformanceOptimizer()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Check every second

    TargetFrameRate = 60.0f;
    CurrentLODLevel = 0;
    bDynamicLODEnabled = true;
    MaxVisibleActors = 500;
    CullDistance = 10000.0f;
    FrameTimeThresholdMS = 16.67f; // 60fps target
}

void UPerformanceOptimizer::BeginPlay()
{
    Super::BeginPlay();
    InitializeOptimizer();
}

void UPerformanceOptimizer::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bDynamicLODEnabled)
    {
        UpdateDynamicLOD(DeltaTime);
    }

    UpdateFrameTimeHistory(DeltaTime);
}

void UPerformanceOptimizer::InitializeOptimizer()
{
    FrameTimeHistory.Reserve(60);
    UE_LOG(LogTemp, Log, TEXT("[PerformanceOptimizer] Initialized — Target: %.1f fps, MaxActors: %d, CullDist: %.0f"),
        TargetFrameRate, MaxVisibleActors, CullDistance);
}

void UPerformanceOptimizer::UpdateDynamicLOD(float DeltaTime)
{
    float AvgFrameTime = GetAverageFrameTimeMS();
    float TargetFrameTimeMS = 1000.0f / TargetFrameRate;

    if (AvgFrameTime > TargetFrameTimeMS * 1.2f && CurrentLODLevel < 3)
    {
        // Performance below target — increase LOD aggressiveness
        CurrentLODLevel++;
        ApplyLODLevel(CurrentLODLevel);
        UE_LOG(LogTemp, Warning, TEXT("[PerformanceOptimizer] LOD increased to %d (avg frame: %.2fms)"),
            CurrentLODLevel, AvgFrameTime);
    }
    else if (AvgFrameTime < TargetFrameTimeMS * 0.8f && CurrentLODLevel > 0)
    {
        // Performance headroom — reduce LOD aggressiveness
        CurrentLODLevel--;
        ApplyLODLevel(CurrentLODLevel);
        UE_LOG(LogTemp, Log, TEXT("[PerformanceOptimizer] LOD decreased to %d (avg frame: %.2fms)"),
            CurrentLODLevel, AvgFrameTime);
    }
}

void UPerformanceOptimizer::ApplyLODLevel(int32 LODLevel)
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Scale cull distance based on LOD level
    float LODCullMultiplier = 1.0f - (LODLevel * 0.15f); // 15% reduction per LOD level
    float EffectiveCullDistance = CullDistance * LODCullMultiplier;

    // Apply to all static mesh actors in world
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);

    int32 CulledCount = 0;
    for (AActor* Actor : AllActors)
    {
        if (!Actor) continue;

        UStaticMeshComponent* MeshComp = Actor->FindComponentByClass<UStaticMeshComponent>();
        if (MeshComp)
        {
            MeshComp->SetCullDistance(EffectiveCullDistance);
            CulledCount++;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("[PerformanceOptimizer] Applied LOD %d to %d mesh components (cull dist: %.0f)"),
        LODLevel, CulledCount, EffectiveCullDistance);
}

void UPerformanceOptimizer::UpdateFrameTimeHistory(float DeltaTime)
{
    float FrameTimeMS = DeltaTime * 1000.0f;
    FrameTimeHistory.Add(FrameTimeMS);

    // Keep rolling window of 60 frames
    if (FrameTimeHistory.Num() > 60)
    {
        FrameTimeHistory.RemoveAt(0);
    }
}

float UPerformanceOptimizer::GetAverageFrameTimeMS() const
{
    if (FrameTimeHistory.Num() == 0) return 16.67f;

    float Total = 0.0f;
    for (float FT : FrameTimeHistory)
    {
        Total += FT;
    }
    return Total / FrameTimeHistory.Num();
}

float UPerformanceOptimizer::GetCurrentFPS() const
{
    float AvgFrameTime = GetAverageFrameTimeMS();
    if (AvgFrameTime <= 0.0f) return 60.0f;
    return 1000.0f / AvgFrameTime;
}

void UPerformanceOptimizer::SetTargetFrameRate(float InTargetFPS)
{
    TargetFrameRate = FMath::Clamp(InTargetFPS, 15.0f, 144.0f);
    FrameTimeThresholdMS = 1000.0f / TargetFrameRate;
    UE_LOG(LogTemp, Log, TEXT("[PerformanceOptimizer] Target FPS set to %.1f (threshold: %.2fms)"),
        TargetFrameRate, FrameTimeThresholdMS);
}

void UPerformanceOptimizer::SetDynamicLODEnabled(bool bEnabled)
{
    bDynamicLODEnabled = bEnabled;
    UE_LOG(LogTemp, Log, TEXT("[PerformanceOptimizer] Dynamic LOD %s"),
        bEnabled ? TEXT("ENABLED") : TEXT("DISABLED"));
}

FString UPerformanceOptimizer::GetPerformanceReport() const
{
    return FString::Printf(
        TEXT("FPS: %.1f | FrameTime: %.2fms | LOD: %d | DynamicLOD: %s | CullDist: %.0f"),
        GetCurrentFPS(),
        GetAverageFrameTimeMS(),
        CurrentLODLevel,
        bDynamicLODEnabled ? TEXT("ON") : TEXT("OFF"),
        CullDistance
    );
}
