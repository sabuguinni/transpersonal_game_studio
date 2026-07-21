#include "Perf_PhysicsSystemIntegrator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "PhysicsEngine/BodyInstance.h"
#include "HAL/PlatformFilemanager.h"

UPerf_PhysicsSystemIntegrator::UPerf_PhysicsSystemIntegrator()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update every 100ms
    
    // Initialize performance targets
    TargetFrameTime = 16.67f; // 60 FPS
    PhysicsTimeThreshold = 5.0f; // Max 5ms for physics
    MaxPhysicsObjects = 500;
    
    // Initialize optimization settings
    bEnableDistanceCulling = true;
    bEnableComplexityReduction = true;
    bEnableAdaptiveOptimization = true;
    bEnablePerformanceLogging = true;
    
    // Distance thresholds
    CullingDistance = 5000.0f;
    LODDistance1 = 1000.0f;
    LODDistance2 = 2500.0f;
    
    // Performance counters
    CurrentPhysicsTime = 0.0f;
    AveragePhysicsTime = 0.0f;
    PeakPhysicsTime = 0.0f;
    ActivePhysicsObjects = 0;
    CulledPhysicsObjects = 0;
    OptimizationLevel = EPerf_PhysicsOptimizationLevel::Medium;
    
    // Initialize history arrays
    PhysicsTimeHistory.SetNum(60); // 6 seconds of history at 10Hz
    FrameTimeHistory.SetNum(60);
    for (int32 i = 0; i < 60; i++)
    {
        PhysicsTimeHistory[i] = 0.0f;
        FrameTimeHistory[i] = 16.67f;
    }
    HistoryIndex = 0;
}

void UPerf_PhysicsSystemIntegrator::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize performance monitoring
    LastUpdateTime = FPlatformTime::Seconds();
    
    // Find physics optimizer component
    if (AActor* Owner = GetOwner())
    {
        PhysicsOptimizer = Owner->FindComponentByClass<UPerf_PhysicsOptimizer>();
        PhysicsMonitor = Owner->FindComponentByClass<UPerf_PhysicsPerformanceMonitor>();
    }
    
    UE_LOG(LogTemp, Warning, TEXT("PhysicsSystemIntegrator: Initialized with target frame time %.2fms"), TargetFrameTime);
}

void UPerf_PhysicsSystemIntegrator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update performance metrics
    UpdatePerformanceMetrics(DeltaTime);
    
    // Perform physics optimization
    OptimizePhysicsPerformance();
    
    // Update optimization level based on performance
    UpdateOptimizationLevel();
    
    // Log performance data
    if (bEnablePerformanceLogging)
    {
        LogPerformanceData();
    }
}

void UPerf_PhysicsSystemIntegrator::UpdatePerformanceMetrics(float DeltaTime)
{
    double CurrentTime = FPlatformTime::Seconds();
    float FrameTime = (CurrentTime - LastUpdateTime) * 1000.0f; // Convert to ms
    LastUpdateTime = CurrentTime;
    
    // Update frame time history
    FrameTimeHistory[HistoryIndex] = FrameTime;
    
    // Get physics time from monitor if available
    if (PhysicsMonitor)
    {
        CurrentPhysicsTime = PhysicsMonitor->GetCurrentPhysicsTime();
        PhysicsTimeHistory[HistoryIndex] = CurrentPhysicsTime;
        
        // Update peak physics time
        if (CurrentPhysicsTime > PeakPhysicsTime)
        {
            PeakPhysicsTime = CurrentPhysicsTime;
        }
    }
    
    // Calculate average physics time
    float TotalPhysicsTime = 0.0f;
    for (float Time : PhysicsTimeHistory)
    {
        TotalPhysicsTime += Time;
    }
    AveragePhysicsTime = TotalPhysicsTime / PhysicsTimeHistory.Num();
    
    // Update history index
    HistoryIndex = (HistoryIndex + 1) % PhysicsTimeHistory.Num();
    
    // Count active physics objects
    CountPhysicsObjects();
}

void UPerf_PhysicsSystemIntegrator::OptimizePhysicsPerformance()
{
    if (!GetWorld()) return;
    
    // Apply distance-based culling
    if (bEnableDistanceCulling)
    {
        ApplyDistanceCulling();
    }
    
    // Apply complexity reduction
    if (bEnableComplexityReduction)
    {
        ApplyComplexityReduction();
    }
    
    // Apply adaptive optimization
    if (bEnableAdaptiveOptimization)
    {
        ApplyAdaptiveOptimization();
    }
}

void UPerf_PhysicsSystemIntegrator::ApplyDistanceCulling()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    APawn* PlayerPawn = World->GetFirstPlayerController() ? World->GetFirstPlayerController()->GetPawn() : nullptr;
    if (!PlayerPawn) return;
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    CulledPhysicsObjects = 0;
    
    // Iterate through all actors with physics
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor || Actor == PlayerPawn) continue;
        
        // Check if actor has physics components
        TArray<UPrimitiveComponent*> PrimitiveComponents;
        Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
        
        for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
        {
            if (!PrimComp || !PrimComp->IsSimulatingPhysics()) continue;
            
            float Distance = FVector::Dist(PlayerLocation, Actor->GetActorLocation());
            
            // Apply distance-based optimization
            if (Distance > CullingDistance)
            {
                // Disable physics simulation for distant objects
                PrimComp->SetSimulatePhysics(false);
                CulledPhysicsObjects++;
            }
            else if (Distance > LODDistance2)
            {
                // Reduce physics complexity for far objects
                if (PrimComp->GetBodyInstance())
                {
                    PrimComp->GetBodyInstance()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
                }
            }
            else if (Distance > LODDistance1)
            {
                // Medium complexity for medium distance
                if (PrimComp->GetBodyInstance())
                {
                    PrimComp->GetBodyInstance()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                }
            }
        }
    }
}

void UPerf_PhysicsSystemIntegrator::ApplyComplexityReduction()
{
    // Reduce physics complexity based on optimization level
    switch (OptimizationLevel)
    {
        case EPerf_PhysicsOptimizationLevel::Ultra:
            // Maximum performance - minimal physics
            ReducePhysicsComplexity(0.25f);
            break;
            
        case EPerf_PhysicsOptimizationLevel::High:
            // High performance - reduced physics
            ReducePhysicsComplexity(0.5f);
            break;
            
        case EPerf_PhysicsOptimizationLevel::Medium:
            // Balanced - normal physics
            ReducePhysicsComplexity(0.75f);
            break;
            
        case EPerf_PhysicsOptimizationLevel::Low:
            // Quality focused - full physics
            ReducePhysicsComplexity(1.0f);
            break;
            
        default:
            break;
    }
}

void UPerf_PhysicsSystemIntegrator::ReducePhysicsComplexity(float ComplexityFactor)
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Adjust physics sub-stepping based on complexity factor
    if (UPhysicsSettings* PhysicsSettings = UPhysicsSettings::Get())
    {
        float TargetSubsteps = FMath::Lerp(1.0f, 4.0f, ComplexityFactor);
        // Note: This would require access to physics settings modification
        // In practice, this might be done through console commands or engine modifications
    }
    
    // Reduce collision complexity for non-critical objects
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor) continue;
        
        TArray<UPrimitiveComponent*> PrimitiveComponents;
        Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
        
        for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
        {
            if (!PrimComp) continue;
            
            // Simplify collision for background objects
            if (ComplexityFactor < 0.5f)
            {
                PrimComp->SetCollisionResponseToAllChannels(ECR_Ignore);
                PrimComp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
            }
        }
    }
}

void UPerf_PhysicsSystemIntegrator::ApplyAdaptiveOptimization()
{
    // Adaptive optimization based on current performance
    float CurrentFrameTime = FrameTimeHistory[HistoryIndex];
    
    if (CurrentFrameTime > TargetFrameTime * 1.5f) // 150% of target
    {
        // Performance is poor - increase optimization
        if (OptimizationLevel < EPerf_PhysicsOptimizationLevel::Ultra)
        {
            OptimizationLevel = static_cast<EPerf_PhysicsOptimizationLevel>(
                static_cast<int32>(OptimizationLevel) + 1);
            UE_LOG(LogTemp, Warning, TEXT("PhysicsIntegrator: Increased optimization level due to poor performance"));
        }
    }
    else if (CurrentFrameTime < TargetFrameTime * 0.8f) // 80% of target
    {
        // Performance is good - can reduce optimization
        if (OptimizationLevel > EPerf_PhysicsOptimizationLevel::Disabled)
        {
            OptimizationLevel = static_cast<EPerf_PhysicsOptimizationLevel>(
                static_cast<int32>(OptimizationLevel) - 1);
            UE_LOG(LogTemp, Log, TEXT("PhysicsIntegrator: Reduced optimization level due to good performance"));
        }
    }
}

void UPerf_PhysicsSystemIntegrator::UpdateOptimizationLevel()
{
    // Calculate performance score based on multiple factors
    float PerformanceScore = CalculatePerformanceScore();
    
    // Update optimization level based on performance score
    if (PerformanceScore < 0.3f)
    {
        OptimizationLevel = EPerf_PhysicsOptimizationLevel::Ultra;
    }
    else if (PerformanceScore < 0.5f)
    {
        OptimizationLevel = EPerf_PhysicsOptimizationLevel::High;
    }
    else if (PerformanceScore < 0.7f)
    {
        OptimizationLevel = EPerf_PhysicsOptimizationLevel::Medium;
    }
    else if (PerformanceScore < 0.9f)
    {
        OptimizationLevel = EPerf_PhysicsOptimizationLevel::Low;
    }
    else
    {
        OptimizationLevel = EPerf_PhysicsOptimizationLevel::Disabled;
    }
}

float UPerf_PhysicsSystemIntegrator::CalculatePerformanceScore()
{
    // Calculate score based on frame time performance
    float FrameScore = FMath::Clamp(TargetFrameTime / FMath::Max(AveragePhysicsTime, 1.0f), 0.0f, 1.0f);
    
    // Calculate score based on physics object count
    float ObjectScore = FMath::Clamp(1.0f - (float)ActivePhysicsObjects / MaxPhysicsObjects, 0.0f, 1.0f);
    
    // Calculate score based on physics time
    float PhysicsScore = FMath::Clamp(PhysicsTimeThreshold / FMath::Max(CurrentPhysicsTime, 1.0f), 0.0f, 1.0f);
    
    // Weighted average
    return (FrameScore * 0.4f) + (ObjectScore * 0.3f) + (PhysicsScore * 0.3f);
}

void UPerf_PhysicsSystemIntegrator::CountPhysicsObjects()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    ActivePhysicsObjects = 0;
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor) continue;
        
        TArray<UPrimitiveComponent*> PrimitiveComponents;
        Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
        
        for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
        {
            if (PrimComp && PrimComp->IsSimulatingPhysics())
            {
                ActivePhysicsObjects++;
            }
        }
    }
}

void UPerf_PhysicsSystemIntegrator::LogPerformanceData()
{
    static float LogTimer = 0.0f;
    LogTimer += GetWorld()->GetDeltaSeconds();
    
    // Log every 5 seconds
    if (LogTimer >= 5.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("PhysicsIntegrator Performance: Frame=%.2fms Physics=%.2fms Objects=%d Culled=%d Level=%d"),
            FrameTimeHistory[HistoryIndex],
            CurrentPhysicsTime,
            ActivePhysicsObjects,
            CulledPhysicsObjects,
            static_cast<int32>(OptimizationLevel));
        
        LogTimer = 0.0f;
    }
}

void UPerf_PhysicsSystemIntegrator::SetOptimizationLevel(EPerf_PhysicsOptimizationLevel NewLevel)
{
    OptimizationLevel = NewLevel;
    UE_LOG(LogTemp, Warning, TEXT("PhysicsIntegrator: Optimization level set to %d"), static_cast<int32>(NewLevel));
}

void UPerf_PhysicsSystemIntegrator::SetPerformanceTargets(float NewTargetFrameTime, float NewPhysicsThreshold, int32 NewMaxObjects)
{
    TargetFrameTime = NewTargetFrameTime;
    PhysicsTimeThreshold = NewPhysicsThreshold;
    MaxPhysicsObjects = NewMaxObjects;
    
    UE_LOG(LogTemp, Warning, TEXT("PhysicsIntegrator: Performance targets updated - Frame=%.2fms Physics=%.2fms Objects=%d"),
        TargetFrameTime, PhysicsTimeThreshold, MaxPhysicsObjects);
}

FPerf_PhysicsPerformanceData UPerf_PhysicsSystemIntegrator::GetPerformanceData() const
{
    FPerf_PhysicsPerformanceData Data;
    Data.CurrentFrameTime = FrameTimeHistory[HistoryIndex];
    Data.AverageFrameTime = 0.0f;
    
    // Calculate average frame time
    for (float Time : FrameTimeHistory)
    {
        Data.AverageFrameTime += Time;
    }
    Data.AverageFrameTime /= FrameTimeHistory.Num();
    
    Data.CurrentPhysicsTime = CurrentPhysicsTime;
    Data.AveragePhysicsTime = AveragePhysicsTime;
    Data.PeakPhysicsTime = PeakPhysicsTime;
    Data.ActivePhysicsObjects = ActivePhysicsObjects;
    Data.CulledPhysicsObjects = CulledPhysicsObjects;
    Data.OptimizationLevel = OptimizationLevel;
    Data.PerformanceScore = CalculatePerformanceScore();
    
    return Data;
}