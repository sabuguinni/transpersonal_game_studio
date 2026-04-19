#include "Perf_PerformanceMonitor.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "DrawDebugHelpers.h"
#include "HAL/PlatformMemory.h"
#include "Stats/Stats.h"

APerf_PerformanceMonitor::APerf_PerformanceMonitor()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Update 10 times per second

    // Create components
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    
    SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
    SphereComponent->SetupAttachment(RootComponent);
    SphereComponent->SetSphereRadius(100.0f);
    SphereComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    MeshComponent->SetupAttachment(SphereComponent);
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Initialize performance monitoring settings
    MonitoringRadius = 1000.0f;
    UpdateInterval = 1.0f;
    bLogPerformanceData = true;
    bShowDebugInfo = true;
    MaxHistorySize = 60; // Keep 60 seconds of data at 1Hz

    // Initialize private variables
    TimeSinceLastUpdate = 0.0f;
    FPSHistory.Reserve(MaxHistorySize);
    FrameTimeHistory.Reserve(MaxHistorySize);

    // Initialize metrics
    CurrentMetrics = FPerf_PerformanceMetrics();
    CurrentPerformanceLevel = EPerf_PerformanceLevel::Good;
}

void APerf_PerformanceMonitor::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("Performance Monitor %s started monitoring"), *GetName());
    
    // Set up visual representation
    if (MeshComponent)
    {
        // Try to load a basic sphere mesh for visualization
        UStaticMesh* SphereMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere"));
        if (SphereMesh)
        {
            MeshComponent->SetStaticMesh(SphereMesh);
            MeshComponent->SetWorldScale3D(FVector(0.5f)); // Small sphere
        }
    }
}

void APerf_PerformanceMonitor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    TimeSinceLastUpdate += DeltaTime;

    if (TimeSinceLastUpdate >= UpdateInterval)
    {
        UpdatePerformanceMetrics();
        UpdatePerformanceLevel();
        
        if (bLogPerformanceData)
        {
            LogPerformanceData();
        }
        
        if (bShowDebugInfo)
        {
            DrawDebugInfo();
        }

        TimeSinceLastUpdate = 0.0f;
    }
}

void APerf_PerformanceMonitor::UpdatePerformanceMetrics()
{
    // Get current FPS
    if (GEngine && GEngine->GetGameViewport())
    {
        CurrentMetrics.CurrentFPS = 1.0f / GetWorld()->GetDeltaSeconds();
        
        // Add to history
        FPSHistory.Add(CurrentMetrics.CurrentFPS);
        FrameTimeHistory.Add(GetWorld()->GetDeltaSeconds() * 1000.0f); // Convert to milliseconds
        
        // Maintain history size
        if (FPSHistory.Num() > MaxHistorySize)
        {
            FPSHistory.RemoveAt(0);
            FrameTimeHistory.RemoveAt(0);
        }
        
        // Calculate averages
        CurrentMetrics.AverageFrameTime = CalculateAverageFrameTime();
    }

    // Get actor count in monitoring area
    if (UWorld* World = GetWorld())
    {
        CurrentMetrics.ActorCount = 0;
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor && Actor != this)
            {
                float Distance = FVector::Dist(GetActorLocation(), Actor->GetActorLocation());
                if (Distance <= MonitoringRadius)
                {
                    CurrentMetrics.ActorCount++;
                }
            }
        }
    }

    // Estimate memory usage (simplified)
    FPlatformMemoryStats MemoryStats = FPlatformMemory::GetStats();
    CurrentMetrics.MemoryUsageMB = MemoryStats.UsedPhysical / (1024.0f * 1024.0f);

    // Estimate render thread and GPU times (simplified)
    CurrentMetrics.GameThreadTime = GetWorld()->GetDeltaSeconds() * 1000.0f;
    CurrentMetrics.RenderThreadTime = CurrentMetrics.GameThreadTime * 0.8f; // Estimate
    CurrentMetrics.GPUTime = CurrentMetrics.GameThreadTime * 0.6f; // Estimate

    // Estimate draw calls (very rough approximation)
    CurrentMetrics.DrawCalls = CurrentMetrics.ActorCount * 2; // Rough estimate
}

void APerf_PerformanceMonitor::UpdatePerformanceLevel()
{
    float fps = CurrentMetrics.CurrentFPS;
    
    if (fps >= 60.0f)
    {
        CurrentPerformanceLevel = EPerf_PerformanceLevel::Excellent;
    }
    else if (fps >= 45.0f)
    {
        CurrentPerformanceLevel = EPerf_PerformanceLevel::Good;
    }
    else if (fps >= 30.0f)
    {
        CurrentPerformanceLevel = EPerf_PerformanceLevel::Acceptable;
    }
    else if (fps >= 15.0f)
    {
        CurrentPerformanceLevel = EPerf_PerformanceLevel::Poor;
    }
    else
    {
        CurrentPerformanceLevel = EPerf_PerformanceLevel::Critical;
    }

    // Update visual representation based on performance
    if (MeshComponent)
    {
        UMaterialInterface* Material = nullptr;
        switch (CurrentPerformanceLevel)
        {
            case EPerf_PerformanceLevel::Excellent:
                // Green material for excellent performance
                break;
            case EPerf_PerformanceLevel::Good:
                // Light green material
                break;
            case EPerf_PerformanceLevel::Acceptable:
                // Yellow material
                break;
            case EPerf_PerformanceLevel::Poor:
                // Orange material
                break;
            case EPerf_PerformanceLevel::Critical:
                // Red material
                break;
        }
    }
}

void APerf_PerformanceMonitor::LogPerformanceData()
{
    UE_LOG(LogTemp, Log, TEXT("Performance Monitor %s: FPS=%.1f, Actors=%d, Memory=%.1fMB"), 
           *GetName(), 
           CurrentMetrics.CurrentFPS, 
           CurrentMetrics.ActorCount, 
           CurrentMetrics.MemoryUsageMB);
}

void APerf_PerformanceMonitor::DrawDebugInfo()
{
    if (UWorld* World = GetWorld())
    {
        FVector Location = GetActorLocation();
        
        // Draw monitoring radius
        DrawDebugSphere(World, Location, MonitoringRadius, 32, FColor::Blue, false, UpdateInterval + 0.1f);
        
        // Draw performance level indicator
        FColor LevelColor;
        switch (CurrentPerformanceLevel)
        {
            case EPerf_PerformanceLevel::Excellent: LevelColor = FColor::Green; break;
            case EPerf_PerformanceLevel::Good: LevelColor = FColor::Cyan; break;
            case EPerf_PerformanceLevel::Acceptable: LevelColor = FColor::Yellow; break;
            case EPerf_PerformanceLevel::Poor: LevelColor = FColor::Orange; break;
            case EPerf_PerformanceLevel::Critical: LevelColor = FColor::Red; break;
            default: LevelColor = FColor::White; break;
        }
        
        DrawDebugSphere(World, Location + FVector(0, 0, 200), 50.0f, 16, LevelColor, false, UpdateInterval + 0.1f);
        
        // Draw text info
        FString DebugText = FString::Printf(TEXT("FPS: %.1f\nActors: %d\nLevel: %s"), 
                                          CurrentMetrics.CurrentFPS,
                                          CurrentMetrics.ActorCount,
                                          *UEnum::GetValueAsString(CurrentPerformanceLevel));
        
        DrawDebugString(World, Location + FVector(0, 0, 300), DebugText, nullptr, LevelColor, UpdateInterval + 0.1f);
    }
}

float APerf_PerformanceMonitor::CalculateAverageFPS()
{
    if (FPSHistory.Num() == 0) return 0.0f;
    
    float Sum = 0.0f;
    for (float FPS : FPSHistory)
    {
        Sum += FPS;
    }
    return Sum / FPSHistory.Num();
}

float APerf_PerformanceMonitor::CalculateAverageFrameTime()
{
    if (FrameTimeHistory.Num() == 0) return 0.0f;
    
    float Sum = 0.0f;
    for (float FrameTime : FrameTimeHistory)
    {
        Sum += FrameTime;
    }
    return Sum / FrameTimeHistory.Num();
}

FPerf_PerformanceMetrics APerf_PerformanceMonitor::GetCurrentMetrics() const
{
    return CurrentMetrics;
}

EPerf_PerformanceLevel APerf_PerformanceMonitor::GetPerformanceLevel() const
{
    return CurrentPerformanceLevel;
}

void APerf_PerformanceMonitor::ResetMetrics()
{
    FPSHistory.Empty();
    FrameTimeHistory.Empty();
    CurrentMetrics = FPerf_PerformanceMetrics();
    CurrentPerformanceLevel = EPerf_PerformanceLevel::Good;
    
    UE_LOG(LogTemp, Log, TEXT("Performance Monitor %s metrics reset"), *GetName());
}

TArray<float> APerf_PerformanceMonitor::GetFPSHistory() const
{
    return FPSHistory;
}

bool APerf_PerformanceMonitor::IsPerformanceAcceptable() const
{
    return CurrentPerformanceLevel != EPerf_PerformanceLevel::Critical && 
           CurrentPerformanceLevel != EPerf_PerformanceLevel::Poor;
}

void APerf_PerformanceMonitor::TestPerformanceInArea()
{
    UE_LOG(LogTemp, Warning, TEXT("Testing performance in area around %s"), *GetName());
    
    // Force immediate metrics update
    UpdatePerformanceMetrics();
    UpdatePerformanceLevel();
    
    // Log detailed results
    UE_LOG(LogTemp, Warning, TEXT("Performance Test Results:"));
    UE_LOG(LogTemp, Warning, TEXT("  Current FPS: %.2f"), CurrentMetrics.CurrentFPS);
    UE_LOG(LogTemp, Warning, TEXT("  Average Frame Time: %.2f ms"), CurrentMetrics.AverageFrameTime);
    UE_LOG(LogTemp, Warning, TEXT("  Actors in Range: %d"), CurrentMetrics.ActorCount);
    UE_LOG(LogTemp, Warning, TEXT("  Memory Usage: %.1f MB"), CurrentMetrics.MemoryUsageMB);
    UE_LOG(LogTemp, Warning, TEXT("  Performance Level: %s"), *UEnum::GetValueAsString(CurrentPerformanceLevel));
}