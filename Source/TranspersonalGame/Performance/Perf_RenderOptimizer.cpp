#include "Perf_RenderOptimizer.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Materials/MaterialInterface.h"
#include "Engine/GameViewportClient.h"
#include "Kismet/GameplayStatics.h"

UPerf_RenderOptimizer::UPerf_RenderOptimizer()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
    
    // Initialize frame time history
    FrameTimeHistory.SetNum(MaxFrameTimeHistory);
    for (int32 i = 0; i < MaxFrameTimeHistory; i++)
    {
        FrameTimeHistory[i] = 16.67f; // Default to 60 FPS
    }
}

void UPerf_RenderOptimizer::BeginPlay()
{
    Super::BeginPlay();
    
    // Cache static mesh actors for optimization
    UWorld* World = GetWorld();
    if (World)
    {
        for (TActorIterator<AStaticMeshActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AStaticMeshActor* StaticMeshActor = *ActorItr;
            if (StaticMeshActor && IsValid(StaticMeshActor))
            {
                CachedStaticMeshActors.Add(StaticMeshActor);
                
                UStaticMeshComponent* MeshComp = StaticMeshActor->GetStaticMeshComponent();
                if (MeshComp && IsValid(MeshComp))
                {
                    CachedMeshComponents.Add(MeshComp);
                }
            }
        }
        
        UE_LOG(LogTemp, Warning, TEXT("RenderOptimizer: Cached %d static mesh actors"), CachedStaticMeshActors.Num());
    }
    
    // Apply initial optimization
    OptimizeRenderingForPerformance();
}

void UPerf_RenderOptimizer::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Update metrics every frame
    UpdateRenderMetrics();
    
    // Apply optimizations at specified interval
    if (bAutoOptimizeEnabled && (CurrentTime - LastOptimizationTime) >= OptimizationUpdateInterval)
    {
        OptimizeRenderingForPerformance();
        LastOptimizationTime = CurrentTime;
    }
}

void UPerf_RenderOptimizer::UpdateRenderMetrics()
{
    // Update frame time history
    float CurrentFrameTime = FApp::GetDeltaTime() * 1000.0f; // Convert to milliseconds
    FrameTimeHistory[FrameTimeHistoryIndex] = CurrentFrameTime;
    FrameTimeHistoryIndex = (FrameTimeHistoryIndex + 1) % MaxFrameTimeHistory;
    
    // Calculate average frame time
    float TotalFrameTime = 0.0f;
    for (float FrameTime : FrameTimeHistory)
    {
        TotalFrameTime += FrameTime;
    }
    CurrentMetrics.GPUFrameTime = TotalFrameTime / MaxFrameTimeHistory;
    
    // Estimate render metrics (simplified for performance)
    CurrentMetrics.VisibleActors = CachedStaticMeshActors.Num();
    CurrentMetrics.CurrentDrawCalls = FMath::Min(CurrentMetrics.VisibleActors * 2, RenderSettings.MaxDrawCalls);
    CurrentMetrics.CurrentTriangles = FMath::Min(CurrentMetrics.VisibleActors * 1000, RenderSettings.MaxTriangles);
}

void UPerf_RenderOptimizer::OptimizeRenderingForPerformance()
{
    if (!GetWorld())
    {
        return;
    }
    
    // Check if we need to optimize based on current performance
    CheckRenderBudget();
    
    // Apply various optimization techniques
    ApplyLODOptimizations();
    UpdateCullingDistances();
    OptimizeStaticMeshLODs();
    UpdateInstancedRendering();
    
    UE_LOG(LogTemp, Log, TEXT("RenderOptimizer: Applied optimizations - DrawCalls: %d, Triangles: %d"), 
           CurrentMetrics.CurrentDrawCalls, CurrentMetrics.CurrentTriangles);
}

void UPerf_RenderOptimizer::SetRenderQuality(EPerformanceLevel Quality)
{
    switch (Quality)
    {
        case EPerformanceLevel::Ultra:
            RenderSettings.ViewDistanceScale = 1.0f;
            RenderSettings.MaxDrawCalls = 3000;
            RenderSettings.MaxTriangles = 1000000;
            CullingDistanceMultiplier = 1.0f;
            break;
            
        case EPerformanceLevel::High:
            RenderSettings.ViewDistanceScale = 0.8f;
            RenderSettings.MaxDrawCalls = 2000;
            RenderSettings.MaxTriangles = 500000;
            CullingDistanceMultiplier = 0.8f;
            break;
            
        case EPerformanceLevel::Medium:
            RenderSettings.ViewDistanceScale = 0.6f;
            RenderSettings.MaxDrawCalls = 1500;
            RenderSettings.MaxTriangles = 300000;
            CullingDistanceMultiplier = 0.6f;
            break;
            
        case EPerformanceLevel::Low:
            RenderSettings.ViewDistanceScale = 0.4f;
            RenderSettings.MaxDrawCalls = 1000;
            RenderSettings.MaxTriangles = 150000;
            CullingDistanceMultiplier = 0.4f;
            break;
    }
    
    // Apply the new settings immediately
    OptimizeRenderingForPerformance();
}

void UPerf_RenderOptimizer::UpdateCullingDistances()
{
    for (TWeakObjectPtr<UStaticMeshComponent> MeshCompPtr : CachedMeshComponents)
    {
        if (UStaticMeshComponent* MeshComp = MeshCompPtr.Get())
        {
            // Adjust cull distance based on performance level
            float BaseCullDistance = MeshComp->GetCachedMaxDrawDistance();
            if (BaseCullDistance <= 0.0f)
            {
                BaseCullDistance = 10000.0f; // Default cull distance
            }
            
            float NewCullDistance = BaseCullDistance * CullingDistanceMultiplier;
            MeshComp->SetCachedMaxDrawDistance(NewCullDistance);
        }
    }
}

void UPerf_RenderOptimizer::OptimizeStaticMeshLODs()
{
    for (TWeakObjectPtr<UStaticMeshComponent> MeshCompPtr : CachedMeshComponents)
    {
        if (UStaticMeshComponent* MeshComp = MeshCompPtr.Get())
        {
            // Force LOD based on distance and performance settings
            if (AActor* Owner = MeshComp->GetOwner())
            {
                FVector ActorLocation = Owner->GetActorLocation();
                FVector ViewLocation = FVector::ZeroVector;
                
                // Get player location for distance calculation
                if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
                {
                    ViewLocation = PlayerPawn->GetActorLocation();
                }
                
                float Distance = FVector::Dist(ActorLocation, ViewLocation);
                
                // Set forced LOD based on distance and performance settings
                int32 ForcedLOD = -1; // -1 means automatic
                
                if (Distance > LOD2Distance * CullingDistanceMultiplier)
                {
                    ForcedLOD = 2; // Lowest detail
                }
                else if (Distance > LOD1Distance * CullingDistanceMultiplier)
                {
                    ForcedLOD = 1; // Medium detail
                }
                else if (Distance > LOD0Distance * CullingDistanceMultiplier)
                {
                    ForcedLOD = 0; // High detail
                }
                
                if (ForcedLOD >= 0)
                {
                    MeshComp->SetForcedLodModel(ForcedLOD + 1); // UE5 uses 1-based LOD indices
                }
                else
                {
                    MeshComp->SetForcedLodModel(0); // Automatic LOD selection
                }
            }
        }
    }
}

void UPerf_RenderOptimizer::ApplyLODOptimizations()
{
    // This would typically involve more complex LOD management
    // For now, we'll focus on distance-based optimizations
    OptimizeStaticMeshLODs();
}

void UPerf_RenderOptimizer::OptimizeMaterialComplexity()
{
    // Material complexity optimization would go here
    // This is a placeholder for future implementation
}

void UPerf_RenderOptimizer::UpdateInstancedRendering()
{
    if (!RenderSettings.bEnableInstancing)
    {
        return;
    }
    
    // Instanced rendering optimization would go here
    // This is a placeholder for future implementation
}

void UPerf_RenderOptimizer::CheckRenderBudget()
{
    // Check if we're exceeding our render budget
    if (CurrentMetrics.CurrentDrawCalls > RenderSettings.MaxDrawCalls)
    {
        // Reduce quality to meet budget
        CullingDistanceMultiplier = FMath::Max(0.2f, CullingDistanceMultiplier * 0.9f);
        UE_LOG(LogTemp, Warning, TEXT("RenderOptimizer: Reducing cull distance to meet draw call budget"));
    }
    
    if (CurrentMetrics.CurrentTriangles > RenderSettings.MaxTriangles)
    {
        // Force higher LODs to reduce triangle count
        LOD0Distance = FMath::Max(500.0f, LOD0Distance * 0.9f);
        LOD1Distance = FMath::Max(1000.0f, LOD1Distance * 0.9f);
        UE_LOG(LogTemp, Warning, TEXT("RenderOptimizer: Reducing LOD distances to meet triangle budget"));
    }
}

FPerf_RenderMetrics UPerf_RenderOptimizer::GetCurrentRenderMetrics() const
{
    return CurrentMetrics;
}

void UPerf_RenderOptimizer::SetViewDistanceScale(float Scale)
{
    RenderSettings.ViewDistanceScale = FMath::Clamp(Scale, 0.1f, 2.0f);
    CullingDistanceMultiplier = RenderSettings.ViewDistanceScale;
    UpdateCullingDistances();
}

void UPerf_RenderOptimizer::EnableDynamicBatching(bool bEnable)
{
    RenderSettings.bEnableDynamicBatching = bEnable;
    // Dynamic batching settings would be applied here
}

void UPerf_RenderOptimizer::SetMaxDrawCalls(int32 MaxCalls)
{
    RenderSettings.MaxDrawCalls = FMath::Max(100, MaxCalls);
}