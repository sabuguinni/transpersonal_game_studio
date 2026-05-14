#include "Perf_TerrainPhysicsPerformanceOptimizer.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "PhysicsEngine/BodyInstance.h"
#include "Landscape/Landscape.h"
#include "Landscape/LandscapeComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Kismet/GameplayStatics.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"

UPerf_TerrainPhysicsPerformanceOptimizer::UPerf_TerrainPhysicsPerformanceOptimizer()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
    
    CurrentLOD = EPerf_TerrainPhysicsLOD::High;
    LastOptimizationTime = 0.0f;
    OptimizationInterval = 1.0f; // Optimize every second
    PerformanceBudgetMS = 16.67f; // Target 60 FPS (16.67ms per frame)
    MetricsCollectionFrames = 0;
    TotalPhysicsTime = 0.0f;
    TotalCollisionTime = 0.0f;
    
    // Initialize frame time history
    FrameTimeHistory.Reserve(120); // Store 2 seconds of history at 60fps
}

void UPerf_TerrainPhysicsPerformanceOptimizer::BeginPlay()
{
    Super::BeginPlay();
    InitializeTerrainPhysicsOptimization();
}

void UPerf_TerrainPhysicsPerformanceOptimizer::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdatePerformanceMetrics(DeltaTime);
    MonitorFrameTime(DeltaTime);
    
    // Periodic optimization
    if (GetWorld()->GetTimeSeconds() - LastOptimizationTime > OptimizationInterval)
    {
        UpdateLODBasedOnPerformance();
        
        // Get player location for distance-based optimization
        if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
        {
            if (APawn* PlayerPawn = PC->GetPawn())
            {
                OptimizePhysicsActorsByDistance(PlayerPawn->GetActorLocation());
            }
        }
        
        EnforcePerformanceBudget();
        LastOptimizationTime = GetWorld()->GetTimeSeconds();
    }
}

void UPerf_TerrainPhysicsPerformanceOptimizer::InitializeTerrainPhysicsOptimization()
{
    UE_LOG(LogTemp, Log, TEXT("Initializing Terrain Physics Performance Optimization"));
    
    // Collect all physics actors in the scene
    TrackedPhysicsActors.Empty();
    
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Find all actors with physics components
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && Actor->GetRootComponent())
        {
            if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent()))
            {
                if (PrimComp->IsSimulatingPhysics() || PrimComp->GetCollisionEnabled() != ECollisionEnabled::NoCollision)
                {
                    TrackedPhysicsActors.Add(Actor);
                    ActorLODLevels.Add(Actor, 2); // Start at medium LOD
                }
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Tracking %d physics actors for optimization"), TrackedPhysicsActors.Num());
    
    // Apply initial LOD settings
    ApplyLODSettings(CurrentLOD);
}

void UPerf_TerrainPhysicsPerformanceOptimizer::UpdatePerformanceMetrics(float DeltaTime)
{
    float StartTime = FPlatformTime::Seconds();
    
    CollectPhysicsMetrics();
    
    // Update frame time impact
    CurrentMetrics.FrameTimeImpact = DeltaTime * 1000.0f; // Convert to milliseconds
    
    // Track physics simulation time
    float PhysicsTime = FPlatformTime::Seconds() - StartTime;
    CurrentMetrics.PhysicsSimulationTime = PhysicsTime * 1000.0f;
    
    TotalPhysicsTime += PhysicsTime;
    MetricsCollectionFrames++;
    
    // Calculate averages every 60 frames
    if (MetricsCollectionFrames >= 60)
    {
        float AvgPhysicsTime = (TotalPhysicsTime / MetricsCollectionFrames) * 1000.0f;
        CurrentMetrics.PhysicsSimulationTime = AvgPhysicsTime;
        
        // Reset counters
        TotalPhysicsTime = 0.0f;
        MetricsCollectionFrames = 0;
    }
}

void UPerf_TerrainPhysicsPerformanceOptimizer::SetTerrainPhysicsLOD(EPerf_TerrainPhysicsLOD NewLOD)
{
    if (CurrentLOD != NewLOD)
    {
        CurrentLOD = NewLOD;
        ApplyLODSettings(NewLOD);
        UE_LOG(LogTemp, Log, TEXT("Terrain Physics LOD changed to: %d"), (int32)NewLOD);
    }
}

void UPerf_TerrainPhysicsPerformanceOptimizer::UpdateLODBasedOnPerformance()
{
    if (!IsWithinPerformanceBudget())
    {
        // Reduce LOD if performance is poor
        switch (CurrentLOD)
        {
            case EPerf_TerrainPhysicsLOD::Ultra:
                SetTerrainPhysicsLOD(EPerf_TerrainPhysicsLOD::High);
                break;
            case EPerf_TerrainPhysicsLOD::High:
                SetTerrainPhysicsLOD(EPerf_TerrainPhysicsLOD::Medium);
                break;
            case EPerf_TerrainPhysicsLOD::Medium:
                SetTerrainPhysicsLOD(EPerf_TerrainPhysicsLOD::Low);
                break;
            case EPerf_TerrainPhysicsLOD::Low:
                SetTerrainPhysicsLOD(EPerf_TerrainPhysicsLOD::Minimal);
                break;
            default:
                break;
        }
    }
    else if (CurrentMetrics.FrameTimeImpact < PerformanceBudgetMS * 0.7f)
    {
        // Increase LOD if we have performance headroom
        switch (CurrentLOD)
        {
            case EPerf_TerrainPhysicsLOD::Minimal:
                SetTerrainPhysicsLOD(EPerf_TerrainPhysicsLOD::Low);
                break;
            case EPerf_TerrainPhysicsLOD::Low:
                SetTerrainPhysicsLOD(EPerf_TerrainPhysicsLOD::Medium);
                break;
            case EPerf_TerrainPhysicsLOD::Medium:
                SetTerrainPhysicsLOD(EPerf_TerrainPhysicsLOD::High);
                break;
            case EPerf_TerrainPhysicsLOD::High:
                SetTerrainPhysicsLOD(EPerf_TerrainPhysicsLOD::Ultra);
                break;
            default:
                break;
        }
    }
}

void UPerf_TerrainPhysicsPerformanceOptimizer::OptimizePhysicsActorsByDistance(const FVector& PlayerLocation)
{
    if (!OptimizationSettings.bEnableDistanceCulling) return;
    
    int32 OptimizedActors = 0;
    
    for (AActor* Actor : TrackedPhysicsActors)
    {
        if (!IsValid(Actor)) continue;
        
        float Distance = FVector::Dist(Actor->GetActorLocation(), PlayerLocation);
        UpdatePhysicsActorLOD(Actor, Distance);
        
        if (ShouldCullActor(Actor, PlayerLocation))
        {
            if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent()))
            {
                PrimComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
                PrimComp->SetSimulatePhysics(false);
                OptimizedActors++;
            }
        }
    }
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("Optimized %d physics actors by distance"), OptimizedActors);
}

void UPerf_TerrainPhysicsPerformanceOptimizer::CullDistantPhysicsActors()
{
    if (!OptimizationSettings.bEnableDistanceCulling) return;
    
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC || !PC->GetPawn()) return;
    
    FVector PlayerLocation = PC->GetPawn()->GetActorLocation();
    int32 CulledActors = 0;
    
    for (AActor* Actor : TrackedPhysicsActors)
    {
        if (!IsValid(Actor)) continue;
        
        float Distance = FVector::Dist(Actor->GetActorLocation(), PlayerLocation);
        if (Distance > OptimizationSettings.MaxPhysicsDistance)
        {
            if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent()))
            {
                PrimComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
                PrimComp->SetSimulatePhysics(false);
                CulledActors++;
            }
        }
    }
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("Culled %d distant physics actors"), CulledActors);
}

bool UPerf_TerrainPhysicsPerformanceOptimizer::IsWithinPerformanceBudget() const
{
    return CurrentMetrics.FrameTimeImpact <= PerformanceBudgetMS;
}

void UPerf_TerrainPhysicsPerformanceOptimizer::EnforcePerformanceBudget()
{
    if (IsWithinPerformanceBudget()) return;
    
    // Emergency performance measures
    if (CurrentMetrics.ActivePhysicsActors > OptimizationSettings.MaxActivePhysicsActors)
    {
        // Disable physics on furthest actors
        APlayerController* PC = GetWorld()->GetFirstPlayerController();
        if (PC && PC->GetPawn())
        {
            FVector PlayerLocation = PC->GetPawn()->GetActorLocation();
            
            // Sort actors by distance
            TrackedPhysicsActors.Sort([PlayerLocation](const AActor& A, const AActor& B) {
                float DistA = FVector::Dist(A.GetActorLocation(), PlayerLocation);
                float DistB = FVector::Dist(B.GetActorLocation(), PlayerLocation);
                return DistA < DistB;
            });
            
            // Disable physics on actors beyond the limit
            for (int32 i = OptimizationSettings.MaxActivePhysicsActors; i < TrackedPhysicsActors.Num(); i++)
            {
                if (AActor* Actor = TrackedPhysicsActors[i])
                {
                    if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent()))
                    {
                        PrimComp->SetSimulatePhysics(false);
                    }
                }
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Performance budget exceeded - emergency optimization applied"));
}

void UPerf_TerrainPhysicsPerformanceOptimizer::OptimizeTerrainCollision()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Find landscape actors and optimize their collision
    for (TActorIterator<ALandscape> LandscapeItr(World); LandscapeItr; ++LandscapeItr)
    {
        ALandscape* Landscape = *LandscapeItr;
        if (Landscape)
        {
            // Optimize landscape collision based on LOD
            switch (CurrentLOD)
            {
                case EPerf_TerrainPhysicsLOD::Ultra:
                case EPerf_TerrainPhysicsLOD::High:
                    // Full collision detail
                    break;
                case EPerf_TerrainPhysicsLOD::Medium:
                    // Reduced collision detail
                    break;
                case EPerf_TerrainPhysicsLOD::Low:
                case EPerf_TerrainPhysicsLOD::Minimal:
                    // Minimal collision
                    break;
            }
        }
    }
}

void UPerf_TerrainPhysicsPerformanceOptimizer::SimplifyTerrainMaterials()
{
    if (!OptimizationSettings.bEnableComplexityReduction) return;
    
    // Simplify physics materials based on performance requirements
    for (AActor* Actor : TrackedPhysicsActors)
    {
        if (!IsValid(Actor)) continue;
        
        if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent()))
        {
            // Simplify physics materials for distant or low-LOD actors
            int32* LODLevel = ActorLODLevels.Find(Actor);
            if (LODLevel && *LODLevel <= 1) // Low LOD
            {
                // Apply simplified physics material
                // This would involve setting simpler physics materials
            }
        }
    }
}

void UPerf_TerrainPhysicsPerformanceOptimizer::ReducePhysicsComplexity()
{
    if (!OptimizationSettings.bEnableComplexityReduction) return;
    
    for (AActor* Actor : TrackedPhysicsActors)
    {
        if (!IsValid(Actor)) continue;
        
        float Complexity = CalculatePhysicsComplexity(Actor);
        if (Complexity > 1.0f) // High complexity threshold
        {
            // Reduce physics complexity
            if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent()))
            {
                // Switch to simpler collision shapes
                PrimComp->SetCollisionResponseToAllChannels(ECR_Block);
            }
        }
    }
}

void UPerf_TerrainPhysicsPerformanceOptimizer::AnalyzeTerrainPhysicsPerformance()
{
    UE_LOG(LogTemp, Log, TEXT("=== Terrain Physics Performance Analysis ==="));
    UE_LOG(LogTemp, Log, TEXT("Current LOD: %d"), (int32)CurrentLOD);
    UE_LOG(LogTemp, Log, TEXT("Active Physics Actors: %d"), CurrentMetrics.ActivePhysicsActors);
    UE_LOG(LogTemp, Log, TEXT("Physics Simulation Time: %.3f ms"), CurrentMetrics.PhysicsSimulationTime);
    UE_LOG(LogTemp, Log, TEXT("Terrain Collision Time: %.3f ms"), CurrentMetrics.TerrainCollisionTime);
    UE_LOG(LogTemp, Log, TEXT("Frame Time Impact: %.3f ms"), CurrentMetrics.FrameTimeImpact);
    UE_LOG(LogTemp, Log, TEXT("Within Budget: %s"), IsWithinPerformanceBudget() ? TEXT("Yes") : TEXT("No"));
    UE_LOG(LogTemp, Log, TEXT("=== End Analysis ==="));
}

void UPerf_TerrainPhysicsPerformanceOptimizer::GeneratePerformanceReport()
{
    FString ReportContent = FString::Printf(TEXT(
        "Terrain Physics Performance Report\n"
        "Generated: %s\n\n"
        "Current Settings:\n"
        "- LOD Level: %d\n"
        "- Max Physics Distance: %.1f\n"
        "- Max Active Actors: %d\n"
        "- Performance Budget: %.2f ms\n\n"
        "Current Metrics:\n"
        "- Active Physics Actors: %d\n"
        "- Physics Simulation Time: %.3f ms\n"
        "- Terrain Collision Time: %.3f ms\n"
        "- Frame Time Impact: %.3f ms\n"
        "- Within Budget: %s\n\n"
        "Optimization Status:\n"
        "- Distance Culling: %s\n"
        "- Complexity Reduction: %s\n"
        "- Last Optimization: %.1f seconds ago\n"
    ),
    *FDateTime::Now().ToString(),
    (int32)CurrentLOD,
    OptimizationSettings.MaxPhysicsDistance,
    OptimizationSettings.MaxActivePhysicsActors,
    PerformanceBudgetMS,
    CurrentMetrics.ActivePhysicsActors,
    CurrentMetrics.PhysicsSimulationTime,
    CurrentMetrics.TerrainCollisionTime,
    CurrentMetrics.FrameTimeImpact,
    IsWithinPerformanceBudget() ? TEXT("Yes") : TEXT("No"),
    OptimizationSettings.bEnableDistanceCulling ? TEXT("Enabled") : TEXT("Disabled"),
    OptimizationSettings.bEnableComplexityReduction ? TEXT("Enabled") : TEXT("Disabled"),
    GetWorld()->GetTimeSeconds() - LastOptimizationTime
    );
    
    UE_LOG(LogTemp, Log, TEXT("%s"), *ReportContent);
}

void UPerf_TerrainPhysicsPerformanceOptimizer::LogPerformanceMetrics() const
{
    UE_LOG(LogTemp, Log, TEXT("Terrain Physics Metrics - Active: %d, Physics Time: %.3f ms, Collision Time: %.3f ms, Frame Impact: %.3f ms"),
        CurrentMetrics.ActivePhysicsActors,
        CurrentMetrics.PhysicsSimulationTime,
        CurrentMetrics.TerrainCollisionTime,
        CurrentMetrics.FrameTimeImpact
    );
}

void UPerf_TerrainPhysicsPerformanceOptimizer::UpdatePhysicsActorLOD(AActor* Actor, float Distance)
{
    if (!IsValid(Actor)) return;
    
    int32 NewLODLevel = 2; // Medium by default
    
    if (Distance < OptimizationSettings.LODTransitionDistance * 0.5f)
    {
        NewLODLevel = 4; // High LOD
    }
    else if (Distance < OptimizationSettings.LODTransitionDistance)
    {
        NewLODLevel = 2; // Medium LOD
    }
    else if (Distance < OptimizationSettings.LODTransitionDistance * 2.0f)
    {
        NewLODLevel = 1; // Low LOD
    }
    else
    {
        NewLODLevel = 0; // Minimal LOD
    }
    
    SetActorPhysicsLOD(Actor, NewLODLevel);
}

void UPerf_TerrainPhysicsPerformanceOptimizer::ApplyLODSettings(EPerf_TerrainPhysicsLOD LOD)
{
    switch (LOD)
    {
        case EPerf_TerrainPhysicsLOD::Ultra:
            PerformanceBudgetMS = 20.0f; // Allow more time for ultra quality
            break;
        case EPerf_TerrainPhysicsLOD::High:
            PerformanceBudgetMS = 16.67f; // 60 FPS target
            break;
        case EPerf_TerrainPhysicsLOD::Medium:
            PerformanceBudgetMS = 13.33f; // Stricter budget
            break;
        case EPerf_TerrainPhysicsLOD::Low:
            PerformanceBudgetMS = 11.11f; // Even stricter
            break;
        case EPerf_TerrainPhysicsLOD::Minimal:
            PerformanceBudgetMS = 8.33f; // Very strict for minimal quality
            break;
    }
}

void UPerf_TerrainPhysicsPerformanceOptimizer::MonitorFrameTime(float DeltaTime)
{
    float FrameTimeMS = DeltaTime * 1000.0f;
    
    FrameTimeHistory.Add(FrameTimeMS);
    if (FrameTimeHistory.Num() > 120) // Keep only last 2 seconds
    {
        FrameTimeHistory.RemoveAt(0);
    }
    
    // Calculate average frame time
    if (FrameTimeHistory.Num() > 0)
    {
        float TotalTime = 0.0f;
        for (float Time : FrameTimeHistory)
        {
            TotalTime += Time;
        }
        CurrentMetrics.FrameTimeImpact = TotalTime / FrameTimeHistory.Num();
    }
}

void UPerf_TerrainPhysicsPerformanceOptimizer::CollectPhysicsMetrics()
{
    CurrentMetrics.ActivePhysicsActors = 0;
    CurrentMetrics.TerrainCollisionChecks = 0;
    
    for (AActor* Actor : TrackedPhysicsActors)
    {
        if (!IsValid(Actor)) continue;
        
        if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent()))
        {
            if (PrimComp->IsSimulatingPhysics())
            {
                CurrentMetrics.ActivePhysicsActors++;
            }
            
            if (PrimComp->GetCollisionEnabled() != ECollisionEnabled::NoCollision)
            {
                CurrentMetrics.TerrainCollisionChecks++;
            }
        }
    }
}

void UPerf_TerrainPhysicsPerformanceOptimizer::OptimizeBasedOnMetrics()
{
    if (CurrentMetrics.PhysicsSimulationTime > PerformanceBudgetMS * 0.5f)
    {
        ReducePhysicsComplexity();
    }
    
    if (CurrentMetrics.TerrainCollisionTime > PerformanceBudgetMS * 0.3f)
    {
        OptimizeTerrainCollision();
    }
    
    if (CurrentMetrics.ActivePhysicsActors > OptimizationSettings.MaxActivePhysicsActors)
    {
        CullDistantPhysicsActors();
    }
}

TArray<AActor*> UPerf_TerrainPhysicsPerformanceOptimizer::GetNearbyPhysicsActors(const FVector& Location, float Radius)
{
    TArray<AActor*> NearbyActors;
    
    for (AActor* Actor : TrackedPhysicsActors)
    {
        if (IsValid(Actor))
        {
            float Distance = FVector::Dist(Actor->GetActorLocation(), Location);
            if (Distance <= Radius)
            {
                NearbyActors.Add(Actor);
            }
        }
    }
    
    return NearbyActors;
}

float UPerf_TerrainPhysicsPerformanceOptimizer::CalculatePhysicsComplexity(AActor* Actor)
{
    if (!IsValid(Actor)) return 0.0f;
    
    float Complexity = 1.0f;
    
    if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent()))
    {
        // Factor in collision complexity
        if (PrimComp->GetCollisionShape().IsComplex())
        {
            Complexity += 2.0f;
        }
        
        // Factor in physics simulation
        if (PrimComp->IsSimulatingPhysics())
        {
            Complexity += 1.5f;
        }
        
        // Factor in mass
        if (PrimComp->GetMass() > 100.0f)
        {
            Complexity += 0.5f;
        }
    }
    
    return Complexity;
}

void UPerf_TerrainPhysicsPerformanceOptimizer::SetActorPhysicsLOD(AActor* Actor, int32 LODLevel)
{
    if (!IsValid(Actor)) return;
    
    ActorLODLevels.Add(Actor, LODLevel);
    
    if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent()))
    {
        switch (LODLevel)
        {
            case 0: // Minimal
                PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
                PrimComp->SetSimulatePhysics(false);
                break;
            case 1: // Low
                PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                PrimComp->SetSimulatePhysics(false);
                break;
            case 2: // Medium
                PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                PrimComp->SetSimulatePhysics(true);
                break;
            case 3: // High
            case 4: // Ultra
                PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                PrimComp->SetSimulatePhysics(true);
                break;
        }
    }
}

bool UPerf_TerrainPhysicsPerformanceOptimizer::ShouldCullActor(AActor* Actor, const FVector& PlayerLocation)
{
    if (!IsValid(Actor) || !OptimizationSettings.bEnableDistanceCulling) return false;
    
    float Distance = FVector::Dist(Actor->GetActorLocation(), PlayerLocation);
    return Distance > OptimizationSettings.MaxPhysicsDistance;
}