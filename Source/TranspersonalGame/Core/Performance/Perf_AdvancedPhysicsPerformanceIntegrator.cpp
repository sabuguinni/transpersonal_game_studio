#include "Perf_AdvancedPhysicsPerformanceIntegrator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Kismet/GameplayStatics.h"

APerf_AdvancedPhysicsPerformanceIntegrator::APerf_AdvancedPhysicsPerformanceIntegrator()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // 10 FPS monitoring

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Initialize default values
    bIsMonitoringActive = false;
    OptimizationStrategy = EPerf_PhysicsOptimizationStrategy::Balanced;
    PerformanceLevel = EPerf_PhysicsPerformanceLevel::High;
    
    // Performance thresholds
    MaxAcceptablePhysicsFrameTime = 16.67f; // 60 FPS target
    MaxActivePhysicsBodies = 500;
    PhysicsMemoryThreshold = 100.0f; // MB
    
    // Monitoring intervals
    MonitoringUpdateInterval = 0.1f;
    OptimizationCheckInterval = 1.0f;
    
    // Internal state
    LastMonitoringUpdate = 0.0f;
    LastOptimizationCheck = 0.0f;
    bNeedsOptimization = false;
    CurrentOptimizationFactor = 1.0f;
    
    // Reserve space for history tracking
    PhysicsFrameTimeHistory.Reserve(100);
    PhysicsBodiesHistory.Reserve(100);
}

void APerf_AdvancedPhysicsPerformanceIntegrator::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("AdvancedPhysicsPerformanceIntegrator: Starting physics performance monitoring"));
    
    // Register with subsystem
    if (UPerf_PhysicsPerformanceSubsystem* Subsystem = GetWorld()->GetSubsystem<UPerf_PhysicsPerformanceSubsystem>())
    {
        Subsystem->RegisterPhysicsPerformanceIntegrator(this);
    }
    
    // Start monitoring automatically
    StartPhysicsPerformanceMonitoring();
}

void APerf_AdvancedPhysicsPerformanceIntegrator::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (!bIsMonitoringActive)
    {
        return;
    }
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Update metrics at specified interval
    if (CurrentTime - LastMonitoringUpdate >= MonitoringUpdateInterval)
    {
        UpdatePhysicsMetrics();
        LastMonitoringUpdate = CurrentTime;
    }
    
    // Check for optimization needs
    if (CurrentTime - LastOptimizationCheck >= OptimizationCheckInterval)
    {
        CheckPerformanceThresholds();
        if (bNeedsOptimization)
        {
            ApplyOptimizationStrategy();
        }
        LastOptimizationCheck = CurrentTime;
    }
}

void APerf_AdvancedPhysicsPerformanceIntegrator::StartPhysicsPerformanceMonitoring()
{
    bIsMonitoringActive = true;
    LastMonitoringUpdate = GetWorld()->GetTimeSeconds();
    LastOptimizationCheck = GetWorld()->GetTimeSeconds();
    
    UE_LOG(LogTemp, Warning, TEXT("Physics performance monitoring started"));
}

void APerf_AdvancedPhysicsPerformanceIntegrator::StopPhysicsPerformanceMonitoring()
{
    bIsMonitoringActive = false;
    UE_LOG(LogTemp, Warning, TEXT("Physics performance monitoring stopped"));
}

FPerf_PhysicsPerformanceMetrics APerf_AdvancedPhysicsPerformanceIntegrator::GetCurrentPhysicsMetrics() const
{
    return CurrentMetrics;
}

void APerf_AdvancedPhysicsPerformanceIntegrator::SetPhysicsOptimizationStrategy(EPerf_PhysicsOptimizationStrategy Strategy)
{
    OptimizationStrategy = Strategy;
    
    // Apply strategy immediately
    ApplyOptimizationStrategy();
    
    UE_LOG(LogTemp, Warning, TEXT("Physics optimization strategy changed to: %d"), (int32)Strategy);
}

void APerf_AdvancedPhysicsPerformanceIntegrator::SetPhysicsPerformanceLevel(EPerf_PhysicsPerformanceLevel Level)
{
    PerformanceLevel = Level;
    UpdatePhysicsQuality();
    
    UE_LOG(LogTemp, Warning, TEXT("Physics performance level set to: %d"), (int32)Level);
}

void APerf_AdvancedPhysicsPerformanceIntegrator::OptimizeTerrainPhysicsIntegration()
{
    UE_LOG(LogTemp, Warning, TEXT("Optimizing terrain physics integration"));
    
    // Get all static mesh actors in the world
    TArray<AActor*> StaticMeshActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AStaticMeshActor::StaticClass(), StaticMeshActors);
    
    int32 OptimizedActors = 0;
    
    for (AActor* Actor : StaticMeshActors)
    {
        if (AStaticMeshActor* MeshActor = Cast<AStaticMeshActor>(Actor))
        {
            UStaticMeshComponent* MeshComp = MeshActor->GetStaticMeshComponent();
            if (MeshComp && MeshComp->IsSimulatingPhysics())
            {
                // Optimize collision complexity based on distance and size
                float DistanceToPlayer = 1000.0f; // Simplified - would get actual player location
                
                if (DistanceToPlayer > 2000.0f)
                {
                    // Use simple collision for distant objects
                    MeshComp->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
                    OptimizedActors++;
                }
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Terrain physics optimization complete. Optimized %d actors"), OptimizedActors);
}

void APerf_AdvancedPhysicsPerformanceIntegrator::AnalyzePhysicsBottlenecks()
{
    UE_LOG(LogTemp, Warning, TEXT("Analyzing physics bottlenecks"));
    
    // Analyze frame time history
    if (PhysicsFrameTimeHistory.Num() > 10)
    {
        float AverageFrameTime = 0.0f;
        float MaxFrameTime = 0.0f;
        
        for (float FrameTime : PhysicsFrameTimeHistory)
        {
            AverageFrameTime += FrameTime;
            MaxFrameTime = FMath::Max(MaxFrameTime, FrameTime);
        }
        
        AverageFrameTime /= PhysicsFrameTimeHistory.Num();
        
        UE_LOG(LogTemp, Warning, TEXT("Physics Performance Analysis:"));
        UE_LOG(LogTemp, Warning, TEXT("  Average Frame Time: %.2f ms"), AverageFrameTime);
        UE_LOG(LogTemp, Warning, TEXT("  Max Frame Time: %.2f ms"), MaxFrameTime);
        UE_LOG(LogTemp, Warning, TEXT("  Active Physics Bodies: %d"), CurrentMetrics.ActivePhysicsBodies);
        
        // Identify bottlenecks
        if (AverageFrameTime > MaxAcceptablePhysicsFrameTime)
        {
            UE_LOG(LogTemp, Warning, TEXT("BOTTLENECK: Physics frame time exceeds target"));
            bNeedsOptimization = true;
        }
        
        if (CurrentMetrics.ActivePhysicsBodies > MaxActivePhysicsBodies)
        {
            UE_LOG(LogTemp, Warning, TEXT("BOTTLENECK: Too many active physics bodies"));
            bNeedsOptimization = true;
        }
    }
}

void APerf_AdvancedPhysicsPerformanceIntegrator::AdjustPhysicsQualityForFrameRate(float TargetFrameRate)
{
    float TargetFrameTime = 1000.0f / TargetFrameRate; // Convert to milliseconds
    
    if (CurrentMetrics.PhysicsFrameTime > TargetFrameTime * 1.2f) // 20% tolerance
    {
        // Reduce physics quality
        switch (PerformanceLevel)
        {
            case EPerf_PhysicsPerformanceLevel::Ultra:
                SetPhysicsPerformanceLevel(EPerf_PhysicsPerformanceLevel::High);
                break;
            case EPerf_PhysicsPerformanceLevel::High:
                SetPhysicsPerformanceLevel(EPerf_PhysicsPerformanceLevel::Medium);
                break;
            case EPerf_PhysicsPerformanceLevel::Medium:
                SetPhysicsPerformanceLevel(EPerf_PhysicsPerformanceLevel::Low);
                break;
            case EPerf_PhysicsPerformanceLevel::Low:
                SetPhysicsPerformanceLevel(EPerf_PhysicsPerformanceLevel::Minimal);
                break;
            default:
                break;
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Physics quality reduced to maintain %.1f FPS"), TargetFrameRate);
    }
    else if (CurrentMetrics.PhysicsFrameTime < TargetFrameTime * 0.8f) // Can increase quality
    {
        // Increase physics quality
        switch (PerformanceLevel)
        {
            case EPerf_PhysicsPerformanceLevel::Minimal:
                SetPhysicsPerformanceLevel(EPerf_PhysicsPerformanceLevel::Low);
                break;
            case EPerf_PhysicsPerformanceLevel::Low:
                SetPhysicsPerformanceLevel(EPerf_PhysicsPerformanceLevel::Medium);
                break;
            case EPerf_PhysicsPerformanceLevel::Medium:
                SetPhysicsPerformanceLevel(EPerf_PhysicsPerformanceLevel::High);
                break;
            case EPerf_PhysicsPerformanceLevel::High:
                SetPhysicsPerformanceLevel(EPerf_PhysicsPerformanceLevel::Ultra);
                break;
            default:
                break;
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Physics quality increased while maintaining %.1f FPS"), TargetFrameRate);
    }
}

bool APerf_AdvancedPhysicsPerformanceIntegrator::IsPhysicsPerformanceOptimal() const
{
    return CurrentMetrics.PhysicsFrameTime <= MaxAcceptablePhysicsFrameTime &&
           CurrentMetrics.ActivePhysicsBodies <= MaxActivePhysicsBodies &&
           CurrentMetrics.PhysicsMemoryUsage <= PhysicsMemoryThreshold;
}

void APerf_AdvancedPhysicsPerformanceIntegrator::UpdatePhysicsMetrics()
{
    // Get current frame time (simplified)
    CurrentMetrics.PhysicsFrameTime = FPlatformTime::ToMilliseconds(FPlatformTime::Cycles());
    
    // Count active physics bodies
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
    
    CurrentMetrics.ActivePhysicsBodies = 0;
    CurrentMetrics.PhysicsCollisionChecks = 0;
    
    for (AActor* Actor : AllActors)
    {
        if (UPrimitiveComponent* PrimComp = Actor->FindComponentByClass<UPrimitiveComponent>())
        {
            if (PrimComp->IsSimulatingPhysics())
            {
                CurrentMetrics.ActivePhysicsBodies++;
            }
            
            if (PrimComp->GetCollisionEnabled() != ECollisionEnabled::NoCollision)
            {
                CurrentMetrics.PhysicsCollisionChecks++;
            }
        }
    }
    
    // Estimate memory usage (simplified)
    CurrentMetrics.PhysicsMemoryUsage = CurrentMetrics.ActivePhysicsBodies * 0.1f; // Rough estimate
    
    // Calculate terrain complexity (simplified)
    CurrentMetrics.TerrainPhysicsComplexity = CurrentMetrics.PhysicsCollisionChecks * 0.01f;
    
    // Update optimization level
    CurrentMetrics.PhysicsOptimizationLevel = CurrentOptimizationFactor;
    
    // Add to history
    PhysicsFrameTimeHistory.Add(CurrentMetrics.PhysicsFrameTime);
    PhysicsBodiesHistory.Add(CurrentMetrics.ActivePhysicsBodies);
    
    // Limit history size
    if (PhysicsFrameTimeHistory.Num() > 100)
    {
        PhysicsFrameTimeHistory.RemoveAt(0);
    }
    if (PhysicsBodiesHistory.Num() > 100)
    {
        PhysicsBodiesHistory.RemoveAt(0);
    }
}

void APerf_AdvancedPhysicsPerformanceIntegrator::CheckPerformanceThresholds()
{
    bNeedsOptimization = false;
    
    if (CurrentMetrics.PhysicsFrameTime > MaxAcceptablePhysicsFrameTime)
    {
        bNeedsOptimization = true;
        UE_LOG(LogTemp, Warning, TEXT("Physics frame time threshold exceeded: %.2f ms"), CurrentMetrics.PhysicsFrameTime);
    }
    
    if (CurrentMetrics.ActivePhysicsBodies > MaxActivePhysicsBodies)
    {
        bNeedsOptimization = true;
        UE_LOG(LogTemp, Warning, TEXT("Active physics bodies threshold exceeded: %d"), CurrentMetrics.ActivePhysicsBodies);
    }
    
    if (CurrentMetrics.PhysicsMemoryUsage > PhysicsMemoryThreshold)
    {
        bNeedsOptimization = true;
        UE_LOG(LogTemp, Warning, TEXT("Physics memory threshold exceeded: %.2f MB"), CurrentMetrics.PhysicsMemoryUsage);
    }
}

void APerf_AdvancedPhysicsPerformanceIntegrator::ApplyOptimizationStrategy()
{
    switch (OptimizationStrategy)
    {
        case EPerf_PhysicsOptimizationStrategy::Conservative:
            CurrentOptimizationFactor = FMath::Max(0.9f, CurrentOptimizationFactor - 0.05f);
            break;
            
        case EPerf_PhysicsOptimizationStrategy::Balanced:
            CurrentOptimizationFactor = FMath::Max(0.7f, CurrentOptimizationFactor - 0.1f);
            break;
            
        case EPerf_PhysicsOptimizationStrategy::Aggressive:
            CurrentOptimizationFactor = FMath::Max(0.5f, CurrentOptimizationFactor - 0.2f);
            break;
            
        case EPerf_PhysicsOptimizationStrategy::UltraPerformance:
            CurrentOptimizationFactor = FMath::Max(0.3f, CurrentOptimizationFactor - 0.3f);
            break;
    }
    
    UpdatePhysicsQuality();
    
    UE_LOG(LogTemp, Warning, TEXT("Applied optimization strategy. New factor: %.2f"), CurrentOptimizationFactor);
}

void APerf_AdvancedPhysicsPerformanceIntegrator::UpdatePhysicsQuality()
{
    // Apply performance level settings to physics subsystem
    if (UPhysicsSettings* PhysicsSettings = GetMutableDefault<UPhysicsSettings>())
    {
        switch (PerformanceLevel)
        {
            case EPerf_PhysicsPerformanceLevel::Ultra:
                // High quality settings
                break;
                
            case EPerf_PhysicsPerformanceLevel::High:
                // Good quality settings
                break;
                
            case EPerf_PhysicsPerformanceLevel::Medium:
                // Balanced settings
                break;
                
            case EPerf_PhysicsPerformanceLevel::Low:
                // Performance-focused settings
                break;
                
            case EPerf_PhysicsPerformanceLevel::Minimal:
                // Minimum quality for maximum performance
                break;
        }
    }
}

void APerf_AdvancedPhysicsPerformanceIntegrator::LogPerformanceMetrics() const
{
    UE_LOG(LogTemp, Log, TEXT("=== Physics Performance Metrics ==="));
    UE_LOG(LogTemp, Log, TEXT("Frame Time: %.2f ms"), CurrentMetrics.PhysicsFrameTime);
    UE_LOG(LogTemp, Log, TEXT("Active Bodies: %d"), CurrentMetrics.ActivePhysicsBodies);
    UE_LOG(LogTemp, Log, TEXT("Collision Checks: %d"), CurrentMetrics.PhysicsCollisionChecks);
    UE_LOG(LogTemp, Log, TEXT("Memory Usage: %.2f MB"), CurrentMetrics.PhysicsMemoryUsage);
    UE_LOG(LogTemp, Log, TEXT("Optimization Level: %.2f"), CurrentMetrics.PhysicsOptimizationLevel);
}

// Subsystem Implementation
void UPerf_PhysicsPerformanceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    UE_LOG(LogTemp, Warning, TEXT("Physics Performance Subsystem initialized"));
}

void UPerf_PhysicsPerformanceSubsystem::Deinitialize()
{
    RegisteredIntegrators.Empty();
    Super::Deinitialize();
    UE_LOG(LogTemp, Warning, TEXT("Physics Performance Subsystem deinitialized"));
}

void UPerf_PhysicsPerformanceSubsystem::RegisterPhysicsPerformanceIntegrator(APerf_AdvancedPhysicsPerformanceIntegrator* Integrator)
{
    if (Integrator && !RegisteredIntegrators.Contains(Integrator))
    {
        RegisteredIntegrators.Add(Integrator);
        UE_LOG(LogTemp, Warning, TEXT("Physics Performance Integrator registered"));
    }
}

void UPerf_PhysicsPerformanceSubsystem::UnregisterPhysicsPerformanceIntegrator(APerf_AdvancedPhysicsPerformanceIntegrator* Integrator)
{
    RegisteredIntegrators.Remove(Integrator);
    UE_LOG(LogTemp, Warning, TEXT("Physics Performance Integrator unregistered"));
}

FPerf_PhysicsPerformanceMetrics UPerf_PhysicsPerformanceSubsystem::GetGlobalPhysicsMetrics() const
{
    UpdateGlobalMetrics();
    return GlobalMetrics;
}

void UPerf_PhysicsPerformanceSubsystem::OptimizeGlobalPhysicsPerformance()
{
    UE_LOG(LogTemp, Warning, TEXT("Optimizing global physics performance"));
    
    for (APerf_AdvancedPhysicsPerformanceIntegrator* Integrator : RegisteredIntegrators)
    {
        if (IsValid(Integrator))
        {
            Integrator->AnalyzePhysicsBottlenecks();
            Integrator->OptimizeTerrainPhysicsIntegration();
        }
    }
    
    ApplyGlobalOptimizations();
}

void UPerf_PhysicsPerformanceSubsystem::UpdateGlobalMetrics() const
{
    // Aggregate metrics from all registered integrators
    if (RegisteredIntegrators.Num() > 0)
    {
        float TotalFrameTime = 0.0f;
        int32 TotalBodies = 0;
        int32 TotalChecks = 0;
        float TotalMemory = 0.0f;
        
        for (APerf_AdvancedPhysicsPerformanceIntegrator* Integrator : RegisteredIntegrators)
        {
            if (IsValid(Integrator))
            {
                FPerf_PhysicsPerformanceMetrics Metrics = Integrator->GetCurrentPhysicsMetrics();
                TotalFrameTime += Metrics.PhysicsFrameTime;
                TotalBodies += Metrics.ActivePhysicsBodies;
                TotalChecks += Metrics.PhysicsCollisionChecks;
                TotalMemory += Metrics.PhysicsMemoryUsage;
            }
        }
        
        // Calculate averages
        const_cast<UPerf_PhysicsPerformanceSubsystem*>(this)->GlobalMetrics.PhysicsFrameTime = TotalFrameTime / RegisteredIntegrators.Num();
        const_cast<UPerf_PhysicsPerformanceSubsystem*>(this)->GlobalMetrics.ActivePhysicsBodies = TotalBodies;
        const_cast<UPerf_PhysicsPerformanceSubsystem*>(this)->GlobalMetrics.PhysicsCollisionChecks = TotalChecks;
        const_cast<UPerf_PhysicsPerformanceSubsystem*>(this)->GlobalMetrics.PhysicsMemoryUsage = TotalMemory;
    }
}

void UPerf_PhysicsPerformanceSubsystem::ApplyGlobalOptimizations()
{
    // Apply world-level physics optimizations
    UE_LOG(LogTemp, Warning, TEXT("Applying global physics optimizations"));
    
    // This would contain global optimization logic
    // such as adjusting world physics settings, culling distant objects, etc.
}