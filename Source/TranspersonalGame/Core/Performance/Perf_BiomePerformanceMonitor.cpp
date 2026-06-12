#include "Perf_BiomePerformanceMonitor.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

APerf_BiomePerformanceMonitor::APerf_BiomePerformanceMonitor()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = MonitoringInterval;

    // Create monitor mesh component
    MonitorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MonitorMesh"));
    RootComponent = MonitorMesh;

    // Set default mesh (cube for visibility)
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube"));
    if (CubeMesh.Succeeded())
    {
        MonitorMesh->SetStaticMesh(CubeMesh.Object);
        MonitorMesh->SetWorldScale3D(FVector(0.5f, 0.5f, 0.5f));
    }

    // Initialize performance metrics
    CurrentMetrics = FPerf_BiomePerformanceMetrics();
    CurrentPerformanceLevel = EPerf_PerformanceLevel::Optimal;
    
    // Initialize monitoring settings
    MonitoringInterval = 1.0f;
    bEnablePerformanceLogging = true;
    FPSWarningThreshold = 30.0f;
    PhysicsTimeWarningThreshold = 16.0f;
    
    TimeSinceLastUpdate = 0.0f;
    MaxHistorySize = 60;
}

void APerf_BiomePerformanceMonitor::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("BiomePerformanceMonitor started at location: %s"), *GetActorLocation().ToString());
    
    // Start performance monitoring
    StartPerformanceMonitoring();
}

void APerf_BiomePerformanceMonitor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    TimeSinceLastUpdate += DeltaTime;
    
    if (TimeSinceLastUpdate >= MonitoringInterval)
    {
        UpdatePerformanceMetrics();
        TimeSinceLastUpdate = 0.0f;
    }
}

void APerf_BiomePerformanceMonitor::UpdatePerformanceMetrics()
{
    if (!GetWorld())
    {
        return;
    }

    // Update FPS
    CurrentMetrics.CurrentFPS = 1.0f / GetWorld()->GetDeltaSeconds();
    AddToHistory(FPSHistory, CurrentMetrics.CurrentFPS);

    // Update physics time (approximation based on frame time)
    float FrameTime = GetWorld()->GetDeltaSeconds() * 1000.0f; // Convert to milliseconds
    CurrentMetrics.PhysicsTime = FrameTime * 0.3f; // Estimate physics takes 30% of frame time
    AddToHistory(PhysicsTimeHistory, CurrentMetrics.PhysicsTime);

    // Update render time (approximation)
    CurrentMetrics.RenderTime = FrameTime * 0.5f; // Estimate rendering takes 50% of frame time

    // Count active biome actors
    CurrentMetrics.ActiveBiomeActors = CountActiveBiomeActors();

    // Calculate biome physics overhead
    CurrentMetrics.BiomePhysicsOverhead = CalculateBiomePhysicsOverhead();

    // Estimate memory usage (basic approximation)
    CurrentMetrics.MemoryUsageMB = CurrentMetrics.ActiveBiomeActors * 0.5f; // Rough estimate

    // Update performance level
    UpdatePerformanceLevel();

    // Log performance warnings if needed
    if (bEnablePerformanceLogging)
    {
        if (CurrentMetrics.CurrentFPS < FPSWarningThreshold)
        {
            LogPerformanceWarning(FString::Printf(TEXT("Low FPS detected: %.1f (threshold: %.1f)"), 
                CurrentMetrics.CurrentFPS, FPSWarningThreshold));
        }

        if (CurrentMetrics.PhysicsTime > PhysicsTimeWarningThreshold)
        {
            LogPerformanceWarning(FString::Printf(TEXT("High physics time: %.1fms (threshold: %.1fms)"), 
                CurrentMetrics.PhysicsTime, PhysicsTimeWarningThreshold));
        }
    }
}

void APerf_BiomePerformanceMonitor::StartPerformanceMonitoring()
{
    SetActorTickEnabled(true);
    UE_LOG(LogTemp, Log, TEXT("Performance monitoring started"));
}

void APerf_BiomePerformanceMonitor::StopPerformanceMonitoring()
{
    SetActorTickEnabled(false);
    UE_LOG(LogTemp, Log, TEXT("Performance monitoring stopped"));
}

void APerf_BiomePerformanceMonitor::LogPerformanceReport()
{
    if (!bEnablePerformanceLogging)
    {
        return;
    }

    FString Report = FString::Printf(TEXT("\n=== BIOME PERFORMANCE REPORT ===\n"));
    Report += FString::Printf(TEXT("Current FPS: %.1f\n"), CurrentMetrics.CurrentFPS);
    Report += FString::Printf(TEXT("Average FPS: %.1f\n"), GetAverageFPS());
    Report += FString::Printf(TEXT("Physics Time: %.1fms\n"), CurrentMetrics.PhysicsTime);
    Report += FString::Printf(TEXT("Render Time: %.1fms\n"), CurrentMetrics.RenderTime);
    Report += FString::Printf(TEXT("Active Biome Actors: %d\n"), CurrentMetrics.ActiveBiomeActors);
    Report += FString::Printf(TEXT("Memory Usage: %.1fMB\n"), CurrentMetrics.MemoryUsageMB);
    Report += FString::Printf(TEXT("Biome Physics Overhead: %.1fms\n"), CurrentMetrics.BiomePhysicsOverhead);
    Report += FString::Printf(TEXT("Performance Level: %s\n"), 
        *UEnum::GetValueAsString(CurrentPerformanceLevel));

    UE_LOG(LogTemp, Log, TEXT("%s"), *Report);
}

float APerf_BiomePerformanceMonitor::GetAverageFPS() const
{
    if (FPSHistory.Num() == 0)
    {
        return 0.0f;
    }

    float Sum = 0.0f;
    for (float FPS : FPSHistory)
    {
        Sum += FPS;
    }

    return Sum / FPSHistory.Num();
}

void APerf_BiomePerformanceMonitor::ResetPerformanceHistory()
{
    FPSHistory.Empty();
    PhysicsTimeHistory.Empty();
    UE_LOG(LogTemp, Log, TEXT("Performance history reset"));
}

void APerf_BiomePerformanceMonitor::MonitorBiomePhysicsImpact()
{
    // Monitor the specific impact of biome physics on performance
    int32 BiomeActorCount = CountActiveBiomeActors();
    float PhysicsOverhead = CalculateBiomePhysicsOverhead();

    if (bEnablePerformanceLogging)
    {
        UE_LOG(LogTemp, Log, TEXT("Biome Physics Impact - Actors: %d, Overhead: %.1fms"), 
            BiomeActorCount, PhysicsOverhead);
    }
}

int32 APerf_BiomePerformanceMonitor::CountActiveBiomeActors()
{
    if (!GetWorld())
    {
        return 0;
    }

    int32 Count = 0;
    
    // Count actors with "Biome" or "Physics" in their class name
    for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && IsValid(Actor))
        {
            FString ClassName = Actor->GetClass()->GetName();
            if (ClassName.Contains(TEXT("Biome")) || ClassName.Contains(TEXT("Physics")))
            {
                Count++;
            }
        }
    }

    return Count;
}

float APerf_BiomePerformanceMonitor::CalculateBiomePhysicsOverhead()
{
    // Calculate estimated physics overhead based on active biome actors
    int32 BiomeActors = CurrentMetrics.ActiveBiomeActors;
    
    // Base overhead per biome actor (estimated)
    float BaseOverheadPerActor = 0.5f; // milliseconds
    
    // Additional overhead for complex biome interactions
    float ComplexityMultiplier = 1.0f + (BiomeActors * 0.1f);
    
    return BiomeActors * BaseOverheadPerActor * ComplexityMultiplier;
}

void APerf_BiomePerformanceMonitor::UpdatePerformanceLevel()
{
    // Determine performance level based on current metrics
    if (CurrentMetrics.CurrentFPS >= 55.0f && CurrentMetrics.PhysicsTime <= 8.0f)
    {
        CurrentPerformanceLevel = EPerf_PerformanceLevel::Optimal;
    }
    else if (CurrentMetrics.CurrentFPS >= 45.0f && CurrentMetrics.PhysicsTime <= 12.0f)
    {
        CurrentPerformanceLevel = EPerf_PerformanceLevel::Good;
    }
    else if (CurrentMetrics.CurrentFPS >= 30.0f && CurrentMetrics.PhysicsTime <= 16.0f)
    {
        CurrentPerformanceLevel = EPerf_PerformanceLevel::Acceptable;
    }
    else if (CurrentMetrics.CurrentFPS >= 20.0f && CurrentMetrics.PhysicsTime <= 25.0f)
    {
        CurrentPerformanceLevel = EPerf_PerformanceLevel::Poor;
    }
    else
    {
        CurrentPerformanceLevel = EPerf_PerformanceLevel::Critical;
    }
}

void APerf_BiomePerformanceMonitor::AddToHistory(TArray<float>& History, float Value)
{
    History.Add(Value);
    
    // Keep history size manageable
    if (History.Num() > MaxHistorySize)
    {
        History.RemoveAt(0);
    }
}

void APerf_BiomePerformanceMonitor::LogPerformanceWarning(const FString& Warning)
{
    UE_LOG(LogTemp, Warning, TEXT("PERFORMANCE WARNING: %s"), *Warning);
    
    // Also log to screen if in editor
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, 
            FString::Printf(TEXT("Performance Warning: %s"), *Warning));
    }
}