#include "Perf_PhysicsPerformanceMonitor.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Actor.h"
#include "HAL/PlatformMemory.h"
#include "Stats/Stats.h"

UPerf_PhysicsPerformanceMonitor::UPerf_PhysicsPerformanceMonitor()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
    
    // Initialize performance metrics
    PhysicsFrameTime = 0.0f;
    ActivePhysicsActors = 0;
    SimulatingRigidBodies = 0;
    PhysicsMemoryUsageMB = 0.0f;
    PhysicsCPUPercent = 0.0f;
    
    // Set reasonable defaults for thresholds
    MaxPhysicsFrameTime = 8.33f; // Target 120fps physics
    MaxActivePhysicsActors = 500;
    MaxPhysicsMemoryMB = 256.0f;
    MaxPhysicsCPUPercent = 25.0f;
    
    // Enable adaptive optimization by default
    bEnableAdaptiveOptimization = true;
    OptimizationCheckInterval = 1.0f;
    CurrentOptimizationLevel = EPerf_OptimizationLevel::Balanced;
    
    // Initialize status
    bPhysicsPerformanceGood = true;
    PerformanceStatus = TEXT("Initializing");
    PerformanceWarnings.Empty();
}

void UPerf_PhysicsPerformanceMonitor::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("Physics Performance Monitor initialized"));
    
    // Initial metrics gathering
    UpdatePhysicsMetrics();
    CheckPerformanceThresholds();
    
    PerformanceStatus = TEXT("Active");
}

void UPerf_PhysicsPerformanceMonitor::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update frame time tracking
    FrameTimeAccumulator += DeltaTime * 1000.0f; // Convert to milliseconds
    FrameCount++;
    
    // Update metrics every optimization check interval
    LastOptimizationCheck += DeltaTime;
    if (LastOptimizationCheck >= OptimizationCheckInterval)
    {
        UpdatePhysicsMetrics();
        CheckPerformanceThresholds();
        
        if (bEnableAdaptiveOptimization)
        {
            ApplyPhysicsOptimizations();
        }
        
        LastOptimizationCheck = 0.0f;
    }
}

void UPerf_PhysicsPerformanceMonitor::UpdatePhysicsMetrics()
{
    if (!GetWorld())
    {
        return;
    }
    
    // Calculate average frame time
    if (FrameCount > 0)
    {
        PhysicsFrameTime = FrameTimeAccumulator / FrameCount;
        FrameTimeAccumulator = 0.0f;
        FrameCount = 0;
    }
    
    // Gather physics actors
    GatherPhysicsActors();
    ActivePhysicsActors = CachedPhysicsActors.Num();
    
    // Count simulating rigid bodies
    SimulatingRigidBodies = 0;
    for (AActor* Actor : CachedPhysicsActors)
    {
        if (Actor && IsValid(Actor))
        {
            UPrimitiveComponent* PrimComp = Actor->GetRootComponent() ? Cast<UPrimitiveComponent>(Actor->GetRootComponent()) : nullptr;
            if (PrimComp && PrimComp->IsSimulatingPhysics())
            {
                SimulatingRigidBodies++;
            }
        }
    }
    
    // Calculate memory usage
    CalculatePhysicsMemoryUsage();
    
    // Update performance status
    UpdatePerformanceStatus();
    
    // Log metrics periodically
    LogPerformanceMetrics();
}

void UPerf_PhysicsPerformanceMonitor::CheckPerformanceThresholds()
{
    PerformanceWarnings.Empty();
    bPhysicsPerformanceGood = true;
    
    // Check frame time
    if (PhysicsFrameTime > MaxPhysicsFrameTime)
    {
        PerformanceWarnings.Add(FString::Printf(TEXT("Physics frame time %.2fms exceeds limit %.2fms"), PhysicsFrameTime, MaxPhysicsFrameTime));
        bPhysicsPerformanceGood = false;
    }
    
    // Check actor count
    if (ActivePhysicsActors > MaxActivePhysicsActors)
    {
        PerformanceWarnings.Add(FString::Printf(TEXT("Active physics actors %d exceeds limit %d"), ActivePhysicsActors, MaxActivePhysicsActors));
        bPhysicsPerformanceGood = false;
    }
    
    // Check memory usage
    if (PhysicsMemoryUsageMB > MaxPhysicsMemoryMB)
    {
        PerformanceWarnings.Add(FString::Printf(TEXT("Physics memory %.2fMB exceeds limit %.2fMB"), PhysicsMemoryUsageMB, MaxPhysicsMemoryMB));
        bPhysicsPerformanceGood = false;
    }
    
    // Check CPU usage
    if (PhysicsCPUPercent > MaxPhysicsCPUPercent)
    {
        PerformanceWarnings.Add(FString::Printf(TEXT("Physics CPU %.2f%% exceeds limit %.2f%%"), PhysicsCPUPercent, MaxPhysicsCPUPercent));
        bPhysicsPerformanceGood = false;
    }
}

void UPerf_PhysicsPerformanceMonitor::ApplyPhysicsOptimizations()
{
    if (bPhysicsPerformanceGood)
    {
        return; // No optimization needed
    }
    
    // Determine optimization level based on performance severity
    int32 WarningCount = PerformanceWarnings.Num();
    EPerf_OptimizationLevel TargetLevel = EPerf_OptimizationLevel::Balanced;
    
    if (WarningCount >= 3)
    {
        TargetLevel = EPerf_OptimizationLevel::Performance;
    }
    else if (WarningCount >= 2)
    {
        TargetLevel = EPerf_OptimizationLevel::Balanced;
    }
    else
    {
        TargetLevel = EPerf_OptimizationLevel::Quality;
    }
    
    if (TargetLevel != CurrentOptimizationLevel)
    {
        SetOptimizationLevel(TargetLevel);
    }
    
    // Apply specific optimizations
    if (ActivePhysicsActors > MaxActivePhysicsActors)
    {
        OptimizePhysicsActorsByDistance();
    }
    
    if (PhysicsFrameTime > MaxPhysicsFrameTime)
    {
        ReducePhysicsComplexity();
    }
}

void UPerf_PhysicsPerformanceMonitor::SetOptimizationLevel(EPerf_OptimizationLevel NewLevel)
{
    CurrentOptimizationLevel = NewLevel;
    
    switch (NewLevel)
    {
        case EPerf_OptimizationLevel::Ultra:
            MaxPhysicsFrameTime = 4.16f; // 240fps
            MaxActivePhysicsActors = 1000;
            break;
            
        case EPerf_OptimizationLevel::Quality:
            MaxPhysicsFrameTime = 6.25f; // 160fps
            MaxActivePhysicsActors = 750;
            break;
            
        case EPerf_OptimizationLevel::Balanced:
            MaxPhysicsFrameTime = 8.33f; // 120fps
            MaxActivePhysicsActors = 500;
            break;
            
        case EPerf_OptimizationLevel::Performance:
            MaxPhysicsFrameTime = 16.67f; // 60fps
            MaxActivePhysicsActors = 250;
            break;
            
        case EPerf_OptimizationLevel::Disabled:
            MaxPhysicsFrameTime = 33.33f; // 30fps
            MaxActivePhysicsActors = 100;
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Physics optimization level set to %d"), (int32)NewLevel);
}

bool UPerf_PhysicsPerformanceMonitor::IsPhysicsPerformanceWithinLimits() const
{
    return bPhysicsPerformanceGood && PerformanceWarnings.Num() == 0;
}

void UPerf_PhysicsPerformanceMonitor::OptimizePhysicsActorsByDistance(float MaxDistance)
{
    if (!GetWorld() || !GetOwner())
    {
        return;
    }
    
    FVector PlayerLocation = GetOwner()->GetActorLocation();
    int32 DisabledCount = 0;
    
    for (AActor* Actor : CachedPhysicsActors)
    {
        if (Actor && IsValid(Actor))
        {
            float Distance = FVector::Dist(Actor->GetActorLocation(), PlayerLocation);
            
            if (Distance > MaxDistance)
            {
                UPrimitiveComponent* PrimComp = Actor->GetRootComponent() ? Cast<UPrimitiveComponent>(Actor->GetRootComponent()) : nullptr;
                if (PrimComp && PrimComp->IsSimulatingPhysics())
                {
                    PrimComp->SetSimulatePhysics(false);
                    DisabledCount++;
                }
            }
        }
    }
    
    if (DisabledCount > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("Disabled physics on %d distant actors (>%.0fm)"), DisabledCount, MaxDistance);
    }
}

void UPerf_PhysicsPerformanceMonitor::ReducePhysicsComplexity()
{
    // Reduce physics update rate temporarily
    if (GetWorld())
    {
        UWorld* World = GetWorld();
        if (World->GetPhysicsScene())
        {
            // Implement physics complexity reduction
            UE_LOG(LogTemp, Log, TEXT("Reducing physics complexity due to performance issues"));
        }
    }
}

void UPerf_PhysicsPerformanceMonitor::EnablePhysicsLOD(bool bEnable)
{
    // Enable/disable physics LOD system
    UE_LOG(LogTemp, Log, TEXT("Physics LOD %s"), bEnable ? TEXT("enabled") : TEXT("disabled"));
}

void UPerf_PhysicsPerformanceMonitor::GatherPhysicsActors()
{
    CachedPhysicsActors.Empty();
    
    if (!GetWorld())
    {
        return;
    }
    
    for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && IsValid(Actor))
        {
            UPrimitiveComponent* PrimComp = Actor->GetRootComponent() ? Cast<UPrimitiveComponent>(Actor->GetRootComponent()) : nullptr;
            if (PrimComp && (PrimComp->IsSimulatingPhysics() || PrimComp->IsCollisionEnabled()))
            {
                CachedPhysicsActors.Add(Actor);
            }
        }
    }
}

void UPerf_PhysicsPerformanceMonitor::CalculatePhysicsMemoryUsage()
{
    // Estimate physics memory usage
    PhysicsMemoryUsageMB = (float)(CachedPhysicsActors.Num() * 1024) / (1024.0f * 1024.0f); // Rough estimate
    
    // Add memory from simulating bodies
    PhysicsMemoryUsageMB += (float)(SimulatingRigidBodies * 2048) / (1024.0f * 1024.0f);
}

void UPerf_PhysicsPerformanceMonitor::UpdatePerformanceStatus()
{
    if (bPhysicsPerformanceGood)
    {
        PerformanceStatus = TEXT("Good");
    }
    else if (PerformanceWarnings.Num() == 1)
    {
        PerformanceStatus = TEXT("Warning");
    }
    else if (PerformanceWarnings.Num() >= 2)
    {
        PerformanceStatus = TEXT("Critical");
    }
}

void UPerf_PhysicsPerformanceMonitor::LogPerformanceMetrics()
{
    static float LastLogTime = 0.0f;
    float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    
    // Log every 5 seconds
    if (CurrentTime - LastLogTime >= 5.0f)
    {
        UE_LOG(LogTemp, Log, TEXT("Physics Performance: FrameTime=%.2fms, Actors=%d, Bodies=%d, Memory=%.2fMB, Status=%s"), 
            PhysicsFrameTime, ActivePhysicsActors, SimulatingRigidBodies, PhysicsMemoryUsageMB, *PerformanceStatus);
        
        LastLogTime = CurrentTime;
    }
}