#include "Perf_PerformanceMonitor.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/PlatformFilemanager.h"
#include "Stats/Stats.h"
#include "RenderingThread.h"
#include "Engine/GameViewportClient.h"

APerf_PerformanceMonitor::APerf_PerformanceMonitor()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f;

    // Create monitoring sphere component
    MonitoringSphere = CreateDefaultSubobject<USphereComponent>(TEXT("MonitoringSphere"));
    RootComponent = MonitoringSphere;
    MonitoringSphere->SetSphereRadius(MonitoringRadius);
    MonitoringSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    MonitoringSphere->SetVisibility(true);

    // Create visualization mesh
    VisualizationMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualizationMesh"));
    VisualizationMesh->SetupAttachment(RootComponent);
    VisualizationMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    VisualizationMesh->SetCastShadow(false);

    // Initialize performance metrics
    CurrentMetrics = FPerf_PerformanceMetrics();
    FPSHistory.Reserve(MaxHistorySize);

    // Set default values
    UpdateInterval = 0.1f;
    bEnableDetailedProfiling = true;
    bShowDebugInfo = true;
    TargetPerformanceLevel = EPerf_PerformanceLevel::High;
}

void APerf_PerformanceMonitor::BeginPlay()
{
    Super::BeginPlay();
    
    ResetPerformanceHistory();
    LastUpdateTime = GetWorld()->GetTimeSeconds();
    
    UE_LOG(LogTemp, Warning, TEXT("Performance Monitor started at location: %s"), 
           *GetActorLocation().ToString());
}

void APerf_PerformanceMonitor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    if (CurrentTime - LastUpdateTime >= UpdateInterval)
    {
        UpdatePerformanceMetrics();
        LastUpdateTime = CurrentTime;
        
        if (bShowDebugInfo)
        {
            UpdateVisualization();
        }
    }
}

void APerf_PerformanceMonitor::UpdatePerformanceMetrics()
{
    CollectFrameStats();
    CollectMemoryStats();
    CollectRenderingStats();

    // Update FPS history
    FPSHistory.Add(CurrentMetrics.CurrentFPS);
    if (FPSHistory.Num() > MaxHistorySize)
    {
        FPSHistory.RemoveAt(0);
    }

    // Calculate average FPS
    float TotalFPS = 0.0f;
    for (float FPS : FPSHistory)
    {
        TotalFPS += FPS;
    }
    CurrentMetrics.AverageFPS = FPSHistory.Num() > 0 ? TotalFPS / FPSHistory.Num() : 60.0f;

    // Update min/max FPS
    if (CurrentMetrics.CurrentFPS < MinRecordedFPS)
    {
        MinRecordedFPS = CurrentMetrics.CurrentFPS;
        CurrentMetrics.MinFPS = MinRecordedFPS;
    }
    if (CurrentMetrics.CurrentFPS > MaxRecordedFPS)
    {
        MaxRecordedFPS = CurrentMetrics.CurrentFPS;
        CurrentMetrics.MaxFPS = MaxRecordedFPS;
    }

    // Check performance thresholds
    EPerf_PerformanceLevel RecommendedLevel = CalculateRecommendedLevel();
    if (RecommendedLevel != TargetPerformanceLevel)
    {
        OnPerformanceThresholdReached(CurrentMetrics.CurrentFPS, RecommendedLevel);
    }

    // Performance change events
    static float LastFPS = 60.0f;
    if (CurrentMetrics.CurrentFPS > LastFPS + 5.0f)
    {
        OnPerformanceImproved(CurrentMetrics.CurrentFPS, LastFPS);
    }
    else if (CurrentMetrics.CurrentFPS < LastFPS - 5.0f)
    {
        OnPerformanceDegraded(CurrentMetrics.CurrentFPS, LastFPS);
    }
    LastFPS = CurrentMetrics.CurrentFPS;
}

void APerf_PerformanceMonitor::CollectFrameStats()
{
    // Get current FPS
    if (GEngine && GEngine->GetGameViewport())
    {
        UGameViewportClient* ViewportClient = GEngine->GetGameViewport();
        if (ViewportClient)
        {
            float DeltaTime = GetWorld()->GetDeltaSeconds();
            CurrentMetrics.CurrentFPS = DeltaTime > 0.0f ? 1.0f / DeltaTime : 60.0f;
            CurrentMetrics.FrameTime = DeltaTime * 1000.0f; // Convert to milliseconds
        }
    }

    // Estimate thread times (simplified)
    CurrentMetrics.GameThreadTime = CurrentMetrics.FrameTime * 0.4f;
    CurrentMetrics.RenderThreadTime = CurrentMetrics.FrameTime * 0.35f;
    CurrentMetrics.GPUTime = CurrentMetrics.FrameTime * 0.25f;
}

void APerf_PerformanceMonitor::CollectMemoryStats()
{
    // Get memory statistics
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    
    CurrentMetrics.UsedMemoryMB = static_cast<float>(MemStats.UsedPhysical) / (1024.0f * 1024.0f);
    CurrentMetrics.AvailableMemoryMB = static_cast<float>(MemStats.AvailablePhysical) / (1024.0f * 1024.0f);
}

void APerf_PerformanceMonitor::CollectRenderingStats()
{
    // Count actors in the world
    UWorld* World = GetWorld();
    if (World)
    {
        CurrentMetrics.ActiveActors = World->GetActorCount();
        
        // Estimate visible actors (simplified)
        CurrentMetrics.VisibleActors = FMath::Max(1, CurrentMetrics.ActiveActors / 2);
        
        // Estimate draw calls and triangles (simplified)
        CurrentMetrics.DrawCalls = CurrentMetrics.VisibleActors * 2;
        CurrentMetrics.Triangles = CurrentMetrics.VisibleActors * 1000;
    }
}

void APerf_PerformanceMonitor::UpdateVisualization()
{
    if (!VisualizationMesh)
        return;

    // Change color based on performance
    UMaterialInterface* Material = nullptr;
    
    if (CurrentMetrics.CurrentFPS >= 55.0f)
    {
        // Green for good performance
        VisualizationMesh->SetVectorParameterValueOnMaterials(TEXT("Color"), FVector(0.0f, 1.0f, 0.0f));
    }
    else if (CurrentMetrics.CurrentFPS >= 25.0f)
    {
        // Yellow for moderate performance
        VisualizationMesh->SetVectorParameterValueOnMaterials(TEXT("Color"), FVector(1.0f, 1.0f, 0.0f));
    }
    else
    {
        // Red for poor performance
        VisualizationMesh->SetVectorParameterValueOnMaterials(TEXT("Color"), FVector(1.0f, 0.0f, 0.0f));
    }

    // Scale based on performance level
    float Scale = FMath::Clamp(CurrentMetrics.CurrentFPS / 60.0f, 0.2f, 2.0f);
    VisualizationMesh->SetWorldScale3D(FVector(Scale));
}

EPerf_PerformanceLevel APerf_PerformanceMonitor::CalculateRecommendedLevel() const
{
    if (CurrentMetrics.CurrentFPS >= 55.0f)
        return EPerf_PerformanceLevel::Ultra;
    else if (CurrentMetrics.CurrentFPS >= 45.0f)
        return EPerf_PerformanceLevel::High;
    else if (CurrentMetrics.CurrentFPS >= 30.0f)
        return EPerf_PerformanceLevel::Medium;
    else if (CurrentMetrics.CurrentFPS >= 20.0f)
        return EPerf_PerformanceLevel::Low;
    else
        return EPerf_PerformanceLevel::Potato;
}

void APerf_PerformanceMonitor::ResetPerformanceHistory()
{
    FPSHistory.Empty();
    MinRecordedFPS = 999.0f;
    MaxRecordedFPS = 0.0f;
    TotalFrameTime = 0.0f;
    FrameCount = 0;
    
    UE_LOG(LogTemp, Warning, TEXT("Performance history reset"));
}

float APerf_PerformanceMonitor::GetAverageFPS() const
{
    return CurrentMetrics.AverageFPS;
}

bool APerf_PerformanceMonitor::IsPerformanceAcceptable() const
{
    switch (TargetPerformanceLevel)
    {
        case EPerf_PerformanceLevel::Ultra:
            return CurrentMetrics.CurrentFPS >= 55.0f;
        case EPerf_PerformanceLevel::High:
            return CurrentMetrics.CurrentFPS >= 45.0f;
        case EPerf_PerformanceLevel::Medium:
            return CurrentMetrics.CurrentFPS >= 30.0f;
        case EPerf_PerformanceLevel::Low:
            return CurrentMetrics.CurrentFPS >= 20.0f;
        case EPerf_PerformanceLevel::Potato:
            return CurrentMetrics.CurrentFPS >= 15.0f;
        default:
            return CurrentMetrics.CurrentFPS >= 30.0f;
    }
}

void APerf_PerformanceMonitor::SetPerformanceLevel(EPerf_PerformanceLevel NewLevel)
{
    TargetPerformanceLevel = NewLevel;
    UE_LOG(LogTemp, Warning, TEXT("Performance level set to: %d"), static_cast<int32>(NewLevel));
}

FString APerf_PerformanceMonitor::GetPerformanceReport() const
{
    FString Report = FString::Printf(TEXT(
        "=== PERFORMANCE REPORT ===\n"
        "Current FPS: %.1f\n"
        "Average FPS: %.1f\n"
        "Min FPS: %.1f\n"
        "Max FPS: %.1f\n"
        "Frame Time: %.2f ms\n"
        "Game Thread: %.2f ms\n"
        "Render Thread: %.2f ms\n"
        "GPU Time: %.2f ms\n"
        "Draw Calls: %d\n"
        "Triangles: %d\n"
        "Memory Used: %.1f MB\n"
        "Memory Available: %.1f MB\n"
        "Active Actors: %d\n"
        "Visible Actors: %d\n"
        "Performance Level: %s\n"
        "Performance Acceptable: %s\n"
    ),
    CurrentMetrics.CurrentFPS,
    CurrentMetrics.AverageFPS,
    CurrentMetrics.MinFPS,
    CurrentMetrics.MaxFPS,
    CurrentMetrics.FrameTime,
    CurrentMetrics.GameThreadTime,
    CurrentMetrics.RenderThreadTime,
    CurrentMetrics.GPUTime,
    CurrentMetrics.DrawCalls,
    CurrentMetrics.Triangles,
    CurrentMetrics.UsedMemoryMB,
    CurrentMetrics.AvailableMemoryMB,
    CurrentMetrics.ActiveActors,
    CurrentMetrics.VisibleActors,
    *UEnum::GetValueAsString(TargetPerformanceLevel),
    IsPerformanceAcceptable() ? TEXT("YES") : TEXT("NO")
    );

    return Report;
}

void APerf_PerformanceMonitor::StartPerformanceTest()
{
    ResetPerformanceHistory();
    bEnableDetailedProfiling = true;
    bShowDebugInfo = true;
    
    UE_LOG(LogTemp, Warning, TEXT("Performance test started"));
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
            TEXT("Performance Test Started"));
    }
}

void APerf_PerformanceMonitor::StopPerformanceTest()
{
    bEnableDetailedProfiling = false;
    
    FString Report = GetPerformanceReport();
    UE_LOG(LogTemp, Warning, TEXT("%s"), *Report);
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, 
            FString::Printf(TEXT("Performance Test Complete - Avg FPS: %.1f"), GetAverageFPS()));
    }
}