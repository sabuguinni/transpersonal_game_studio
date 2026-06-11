#include "Perf_PerformanceMonitor.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/StaticMesh.h"

APerf_PerformanceMonitor::APerf_PerformanceMonitor()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Update 10 times per second

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    RootComponent = RootSceneComponent;

    // Create indicator mesh component
    IndicatorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("IndicatorMesh"));
    IndicatorMesh->SetupAttachment(RootComponent);

    // Try to load a simple cube mesh for the indicator
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshAsset(TEXT("/Engine/BasicShapes/Cube"));
    if (CubeMeshAsset.Succeeded())
    {
        IndicatorMesh->SetStaticMesh(CubeMeshAsset.Object);
        IndicatorMesh->SetWorldScale3D(FVector(0.5f, 0.5f, 0.5f));
    }

    // Initialize default values
    UpdateInterval = 1.0f;
    bEnableVisualIndicator = true;
    bLogPerformanceData = true;
    TimeSinceLastUpdate = 0.0f;
    AccumulatedFrameTime = 0.0f;
    FrameCount = 0;

    // Set default performance budget
    PerformanceBudget = FPerf_PerformanceBudget();
}

void APerf_PerformanceMonitor::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Performance Monitor started - Target FPS: %.1f"), PerformanceBudget.TargetFPS_PC);
    
    // Initial metrics update
    UpdatePerformanceMetrics();
    UpdateVisualIndicator();
}

void APerf_PerformanceMonitor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Accumulate frame data
    AccumulatedFrameTime += DeltaTime;
    FrameCount++;
    TimeSinceLastUpdate += DeltaTime;

    // Update metrics at specified interval
    if (TimeSinceLastUpdate >= UpdateInterval)
    {
        UpdatePerformanceMetrics();
        UpdateVisualIndicator();
        
        if (bLogPerformanceData)
        {
            LogPerformanceReport();
        }

        // Reset accumulators
        TimeSinceLastUpdate = 0.0f;
        AccumulatedFrameTime = 0.0f;
        FrameCount = 0;
    }
}

void APerf_PerformanceMonitor::UpdatePerformanceMetrics()
{
    if (FrameCount > 0)
    {
        // Calculate average frame time and FPS
        CurrentMetrics.FrameTime = (AccumulatedFrameTime / FrameCount) * 1000.0f; // Convert to milliseconds
        CurrentMetrics.CurrentFPS = 1.0f / (AccumulatedFrameTime / FrameCount);
    }

    // Get engine stats if available
    if (GEngine)
    {
        // Try to get render stats
        if (GEngine->GetGameViewport())
        {
            // Estimate draw calls and triangles (simplified approach)
            UWorld* World = GetWorld();
            if (World)
            {
                int32 ActorCount = 0;
                int32 StaticMeshCount = 0;
                
                for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
                {
                    AActor* Actor = *ActorIterator;
                    if (Actor && !Actor->IsPendingKill())
                    {
                        ActorCount++;
                        
                        // Count static mesh components
                        TArray<UStaticMeshComponent*> MeshComponents;
                        Actor->GetComponents<UStaticMeshComponent>(MeshComponents);
                        StaticMeshCount += MeshComponents.Num();
                    }
                }
                
                // Rough estimates for performance metrics
                CurrentMetrics.DrawCalls = StaticMeshCount + (ActorCount / 4); // Estimate
                CurrentMetrics.TriangleCount = StaticMeshCount * 1000; // Very rough estimate
            }
        }

        // Get memory usage (simplified)
        FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
        CurrentMetrics.MemoryUsageMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);
    }

    // Check if performance is within budget
    CurrentMetrics.bIsPerformanceGood = IsPerformanceWithinBudget();
}

bool APerf_PerformanceMonitor::IsPerformanceWithinBudget() const
{
    bool bFPSGood = CurrentMetrics.CurrentFPS >= PerformanceBudget.TargetFPS_PC;
    bool bDrawCallsGood = CurrentMetrics.DrawCalls <= PerformanceBudget.MaxDrawCalls;
    bool bTrianglesGood = CurrentMetrics.TriangleCount <= PerformanceBudget.MaxTriangles;
    bool bMemoryGood = CurrentMetrics.MemoryUsageMB <= PerformanceBudget.MaxMemoryMB;

    return bFPSGood && bDrawCallsGood && bTrianglesGood && bMemoryGood;
}

void APerf_PerformanceMonitor::SetPerformanceBudget(const FPerf_PerformanceBudget& NewBudget)
{
    PerformanceBudget = NewBudget;
    UE_LOG(LogTemp, Warning, TEXT("Performance budget updated - Target FPS: %.1f, Max Draw Calls: %d"), 
           PerformanceBudget.TargetFPS_PC, PerformanceBudget.MaxDrawCalls);
}

void APerf_PerformanceMonitor::LogPerformanceReport() const
{
    FString StatusText = CurrentMetrics.bIsPerformanceGood ? TEXT("GOOD") : TEXT("OVER BUDGET");
    
    UE_LOG(LogTemp, Warning, TEXT("=== PERFORMANCE REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("FPS: %.1f (Target: %.1f) - Frame Time: %.2f ms"), 
           CurrentMetrics.CurrentFPS, PerformanceBudget.TargetFPS_PC, CurrentMetrics.FrameTime);
    UE_LOG(LogTemp, Warning, TEXT("Draw Calls: %d (Max: %d)"), 
           CurrentMetrics.DrawCalls, PerformanceBudget.MaxDrawCalls);
    UE_LOG(LogTemp, Warning, TEXT("Triangles: %d (Max: %d)"), 
           CurrentMetrics.TriangleCount, PerformanceBudget.MaxTriangles);
    UE_LOG(LogTemp, Warning, TEXT("Memory: %.1f MB (Max: %.1f MB)"), 
           CurrentMetrics.MemoryUsageMB, PerformanceBudget.MaxMemoryMB);
    UE_LOG(LogTemp, Warning, TEXT("Status: %s"), *StatusText);

    // Also log to screen if in development
    if (GEngine && GetWorld())
    {
        FString ScreenMessage = FString::Printf(TEXT("Performance: %s - FPS: %.1f - Draw Calls: %d"), 
                                               *StatusText, CurrentMetrics.CurrentFPS, CurrentMetrics.DrawCalls);
        GEngine->AddOnScreenDebugMessage(-1, UpdateInterval, 
                                       CurrentMetrics.bIsPerformanceGood ? FColor::Green : FColor::Red, 
                                       ScreenMessage);
    }
}

void APerf_PerformanceMonitor::UpdateVisualIndicator()
{
    if (!bEnableVisualIndicator || !IndicatorMesh)
    {
        return;
    }

    // Change indicator color based on performance
    if (CurrentMetrics.bIsPerformanceGood)
    {
        // Good performance - green indicator
        IndicatorMesh->SetVectorParameterValueOnMaterials(TEXT("BaseColor"), FVector(0.0f, 1.0f, 0.0f));
    }
    else
    {
        // Poor performance - red indicator
        IndicatorMesh->SetVectorParameterValueOnMaterials(TEXT("BaseColor"), FVector(1.0f, 0.0f, 0.0f));
        
        // Trigger Blueprint event
        OnPerformanceBudgetExceeded();
    }

    // Scale indicator based on FPS ratio
    float FPSRatio = CurrentMetrics.CurrentFPS / PerformanceBudget.TargetFPS_PC;
    FPSRatio = FMath::Clamp(FPSRatio, 0.1f, 2.0f);
    IndicatorMesh->SetWorldScale3D(FVector(0.5f * FPSRatio, 0.5f * FPSRatio, 0.5f * FPSRatio));
}