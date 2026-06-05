#include "Perf_RealTimePerformanceController.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/Material.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/StaticMeshActor.h"
#include "Components/PrimitiveComponent.h"
#include "HAL/IConsoleManager.h"

APerf_RealTimePerformanceController::APerf_RealTimePerformanceController()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.0f; // Tick every frame for real-time monitoring

    // Create visualization mesh component
    VisualizationMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualizationMesh"));
    RootComponent = VisualizationMesh;

    // Set default mesh (cube)
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshAsset(TEXT("/Engine/BasicShapes/Cube"));
    if (CubeMeshAsset.Succeeded())
    {
        VisualizationMesh->SetStaticMesh(CubeMeshAsset.Object);
    }

    // Set default material
    static ConstructorHelpers::FObjectFinder<UMaterial> DefaultMaterial(TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
    if (DefaultMaterial.Succeeded())
    {
        VisualizationMesh->SetMaterial(0, DefaultMaterial.Object);
    }

    // Scale down for visualization
    VisualizationMesh->SetWorldScale3D(FVector(0.5f, 0.5f, 0.5f));

    // Initialize default settings
    OptimizationSettings = FPerf_OptimizationSettings();
    MetricsUpdateInterval = 0.1f; // Update metrics 10 times per second
    bEnablePerformanceLogging = true;
    bEnableAutomaticOptimization = true;

    // Initialize performance history
    LastMetricsUpdateTime = 0.0f;
    HistoryIndex = 0;
    for (int32 i = 0; i < 60; ++i)
    {
        PerformanceHistory[i] = 16.67f; // Default to 60 FPS
    }
}

void APerf_RealTimePerformanceController::BeginPlay()
{
    Super::BeginPlay();

    // Initial performance metrics update
    UpdatePerformanceMetrics();

    if (bEnablePerformanceLogging)
    {
        UE_LOG(LogTemp, Warning, TEXT("Performance Controller initialized - Target FPS: %.1f"), OptimizationSettings.TargetFPS);
    }
}

void APerf_RealTimePerformanceController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update performance history
    PerformanceHistory[HistoryIndex] = DeltaTime * 1000.0f; // Convert to milliseconds
    HistoryIndex = (HistoryIndex + 1) % 60;

    // Update metrics at specified interval
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastMetricsUpdateTime >= MetricsUpdateInterval)
    {
        UpdatePerformanceMetrics();
        LastMetricsUpdateTime = CurrentTime;

        // Apply automatic optimizations if enabled
        if (bEnableAutomaticOptimization && !IsPerformanceTargetMet())
        {
            ApplyOptimizations();
        }
    }

    // Update visualization based on performance
    if (VisualizationMesh)
    {
        // Change color based on performance
        float PerformanceRatio = CurrentMetrics.CurrentFPS / OptimizationSettings.TargetFPS;
        if (PerformanceRatio < 0.8f)
        {
            // Red for poor performance
            VisualizationMesh->SetVectorParameterValueOnMaterials(TEXT("BaseColor"), FVector(1.0f, 0.0f, 0.0f));
        }
        else if (PerformanceRatio < 0.95f)
        {
            // Yellow for moderate performance
            VisualizationMesh->SetVectorParameterValueOnMaterials(TEXT("BaseColor"), FVector(1.0f, 1.0f, 0.0f));
        }
        else
        {
            // Green for good performance
            VisualizationMesh->SetVectorParameterValueOnMaterials(TEXT("BaseColor"), FVector(0.0f, 1.0f, 0.0f));
        }
    }
}

void APerf_RealTimePerformanceController::UpdatePerformanceMetrics()
{
    if (!GetWorld())
    {
        return;
    }

    // Calculate current FPS from frame time history
    float TotalFrameTime = 0.0f;
    for (int32 i = 0; i < 60; ++i)
    {
        TotalFrameTime += PerformanceHistory[i];
    }
    float AverageFrameTime = TotalFrameTime / 60.0f;
    CurrentMetrics.FrameTimeMS = AverageFrameTime;
    CurrentMetrics.CurrentFPS = (AverageFrameTime > 0.0f) ? (1000.0f / AverageFrameTime) : 60.0f;

    // Get actor counts
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
    CurrentMetrics.ActiveActorCount = AllActors.Num();

    // Count visible actors (simplified - actors with mesh components)
    CurrentMetrics.VisibleActorCount = 0;
    for (AActor* Actor : AllActors)
    {
        if (Actor && Actor->FindComponentByClass<UStaticMeshComponent>())
        {
            CurrentMetrics.VisibleActorCount++;
        }
    }

    // Calculate culling efficiency
    if (CurrentMetrics.ActiveActorCount > 0)
    {
        CurrentMetrics.CullingEfficiency = 1.0f - (float(CurrentMetrics.VisibleActorCount) / float(CurrentMetrics.ActiveActorCount));
    }

    // Estimate GPU time (simplified)
    CurrentMetrics.GPUTimeMS = CurrentMetrics.FrameTimeMS * 0.6f; // Assume 60% of frame time is GPU

    // Estimate memory usage (simplified)
    CurrentMetrics.MemoryUsageMB = 1024.0f + (CurrentMetrics.ActiveActorCount * 0.5f);

    if (bEnablePerformanceLogging)
    {
        UE_LOG(LogTemp, Log, TEXT("Performance: FPS=%.1f, FrameTime=%.2fms, Actors=%d, Visible=%d"), 
               CurrentMetrics.CurrentFPS, CurrentMetrics.FrameTimeMS, 
               CurrentMetrics.ActiveActorCount, CurrentMetrics.VisibleActorCount);
    }
}

void APerf_RealTimePerformanceController::ApplyOptimizations()
{
    if (!GetWorld())
    {
        return;
    }

    // Apply LOD optimizations if enabled
    if (OptimizationSettings.bEnableAdaptiveLOD)
    {
        EnableAdaptiveLOD(true);
    }

    // Apply culling optimizations if enabled
    if (OptimizationSettings.bEnableDynamicCulling)
    {
        EnableDynamicCulling(true);
    }

    // Apply memory optimizations if enabled
    if (OptimizationSettings.bEnableMemoryOptimization)
    {
        // Force garbage collection if memory usage is high
        if (CurrentMetrics.MemoryUsageMB > 2048.0f)
        {
            GEngine->ForceGarbageCollection(true);
        }
    }

    if (bEnablePerformanceLogging)
    {
        UE_LOG(LogTemp, Warning, TEXT("Applied performance optimizations"));
    }
}

void APerf_RealTimePerformanceController::SetTargetFPS(float NewTargetFPS)
{
    OptimizationSettings.TargetFPS = FMath::Clamp(NewTargetFPS, 15.0f, 120.0f);
    OptimizationSettings.MaxFrameTimeMS = 1000.0f / OptimizationSettings.TargetFPS;

    if (bEnablePerformanceLogging)
    {
        UE_LOG(LogTemp, Warning, TEXT("Target FPS set to: %.1f"), OptimizationSettings.TargetFPS);
    }
}

float APerf_RealTimePerformanceController::GetAverageFrameTime() const
{
    return CurrentMetrics.FrameTimeMS;
}

bool APerf_RealTimePerformanceController::IsPerformanceTargetMet() const
{
    return CurrentMetrics.CurrentFPS >= (OptimizationSettings.TargetFPS * 0.9f);
}

void APerf_RealTimePerformanceController::EnableAdaptiveLOD(bool bEnable)
{
    OptimizationSettings.bEnableAdaptiveLOD = bEnable;

    if (bEnable)
    {
        // Apply LOD settings to all static mesh actors
        TArray<AActor*> StaticMeshActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), AStaticMeshActor::StaticClass(), StaticMeshActors);

        for (AActor* Actor : StaticMeshActors)
        {
            if (AStaticMeshActor* MeshActor = Cast<AStaticMeshActor>(Actor))
            {
                UStaticMeshComponent* MeshComp = MeshActor->GetStaticMeshComponent();
                if (MeshComp)
                {
                    // Adjust LOD bias based on performance
                    float LODBias = (CurrentMetrics.CurrentFPS < OptimizationSettings.TargetFPS) ? 1.0f : 0.0f;
                    MeshComp->SetForcedLodModel(LODBias > 0.0f ? 2 : 0);
                }
            }
        }
    }

    if (bEnablePerformanceLogging)
    {
        UE_LOG(LogTemp, Warning, TEXT("Adaptive LOD %s"), bEnable ? TEXT("enabled") : TEXT("disabled"));
    }
}

void APerf_RealTimePerformanceController::EnableDynamicCulling(bool bEnable)
{
    OptimizationSettings.bEnableDynamicCulling = bEnable;

    if (bEnable)
    {
        // Adjust culling distances based on performance
        float CullingMultiplier = (CurrentMetrics.CurrentFPS < OptimizationSettings.TargetFPS) ? 0.7f : 1.0f;
        OptimizationSettings.CullingDistanceMultiplier = CullingMultiplier;

        // Apply to all primitive components
        TArray<AActor*> AllActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);

        for (AActor* Actor : AllActors)
        {
            if (Actor)
            {
                TArray<UPrimitiveComponent*> PrimitiveComponents;
                Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);

                for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
                {
                    if (PrimComp)
                    {
                        float NewCullDistance = PrimComp->GetCachedMaxDrawDistance() * CullingMultiplier;
                        PrimComp->SetCullDistance(NewCullDistance);
                    }
                }
            }
        }
    }

    if (bEnablePerformanceLogging)
    {
        UE_LOG(LogTemp, Warning, TEXT("Dynamic culling %s"), bEnable ? TEXT("enabled") : TEXT("disabled"));
    }
}

void APerf_RealTimePerformanceController::OptimizeForDinosaurs()
{
    // Specific optimizations for dinosaur actors
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);

    for (AActor* Actor : AllActors)
    {
        if (Actor && Actor->GetName().Contains(TEXT("Dinosaur")))
        {
            // Apply aggressive LOD for dinosaurs when performance is poor
            TArray<UStaticMeshComponent*> MeshComponents;
            Actor->GetComponents<UStaticMeshComponent>(MeshComponents);

            for (UStaticMeshComponent* MeshComp : MeshComponents)
            {
                if (MeshComp)
                {
                    if (CurrentMetrics.CurrentFPS < OptimizationSettings.TargetFPS * 0.8f)
                    {
                        MeshComp->SetForcedLodModel(2); // Force higher LOD
                    }
                    else
                    {
                        MeshComp->SetForcedLodModel(0); // Use best LOD
                    }
                }
            }
        }
    }

    if (bEnablePerformanceLogging)
    {
        UE_LOG(LogTemp, Warning, TEXT("Dinosaur-specific optimizations applied"));
    }
}

void APerf_RealTimePerformanceController::OptimizeForTerrain()
{
    // Specific optimizations for terrain/landscape
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);

    for (AActor* Actor : AllActors)
    {
        if (Actor && (Actor->GetName().Contains(TEXT("Terrain")) || Actor->GetName().Contains(TEXT("Landscape"))))
        {
            // Apply terrain-specific optimizations
            TArray<UPrimitiveComponent*> PrimitiveComponents;
            Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);

            for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
            {
                if (PrimComp)
                {
                    // Adjust terrain detail based on performance
                    if (CurrentMetrics.CurrentFPS < OptimizationSettings.TargetFPS * 0.8f)
                    {
                        PrimComp->SetCullDistance(5000.0f); // Reduce terrain draw distance
                    }
                    else
                    {
                        PrimComp->SetCullDistance(10000.0f); // Normal terrain draw distance
                    }
                }
            }
        }
    }

    if (bEnablePerformanceLogging)
    {
        UE_LOG(LogTemp, Warning, TEXT("Terrain-specific optimizations applied"));
    }
}

void APerf_RealTimePerformanceController::ResetOptimizations()
{
    // Reset all optimization settings to defaults
    OptimizationSettings = FPerf_OptimizationSettings();

    // Reset LOD levels for all actors
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);

    for (AActor* Actor : AllActors)
    {
        if (Actor)
        {
            TArray<UStaticMeshComponent*> MeshComponents;
            Actor->GetComponents<UStaticMeshComponent>(MeshComponents);

            for (UStaticMeshComponent* MeshComp : MeshComponents)
            {
                if (MeshComp)
                {
                    MeshComp->SetForcedLodModel(0); // Reset to automatic LOD
                }
            }

            TArray<UPrimitiveComponent*> PrimitiveComponents;
            Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);

            for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
            {
                if (PrimComp)
                {
                    PrimComp->SetCullDistance(0.0f); // Reset to default cull distance
                }
            }
        }
    }

    if (bEnablePerformanceLogging)
    {
        UE_LOG(LogTemp, Warning, TEXT("All optimizations reset to defaults"));
    }
}