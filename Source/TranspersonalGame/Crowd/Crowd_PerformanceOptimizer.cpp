#include "Crowd_PerformanceOptimizer.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SphereComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

ACrowd_PerformanceOptimizer::ACrowd_PerformanceOptimizer()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create visualization mesh component
    VisualizationMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualizationMesh"));
    RootComponent = VisualizationMesh;

    // Set default values
    PerformanceUpdateInterval = 0.5f;
    MaxTotalCrowdAgents = 5000;
    TargetFrameRate = 60.0f;
    bEnableAdaptiveOptimization = true;
    StreamingRadius = 3000.0f;
    StreamingPriority = 1;

    LastPerformanceUpdate = 0.0f;
    CurrentFrameRate = 60.0f;
    CurrentCrowdCount = 0;

    // Initialize default performance zones
    FCrowd_PerformanceZone HighPerfZone;
    HighPerfZone.ZoneCenter = FVector::ZeroVector;
    HighPerfZone.ZoneRadius = 500.0f;
    HighPerfZone.OptimizationLevel = ECrowd_OptimizationLevel::Ultra_High;
    HighPerfZone.MaxCrowdAgents = 200;
    HighPerfZone.UpdateFrequency = 0.05f;
    PerformanceZones.Add(HighPerfZone);

    FCrowd_PerformanceZone MediumPerfZone;
    MediumPerfZone.ZoneCenter = FVector::ZeroVector;
    MediumPerfZone.ZoneRadius = 1500.0f;
    MediumPerfZone.OptimizationLevel = ECrowd_OptimizationLevel::Medium;
    MediumPerfZone.MaxCrowdAgents = 100;
    MediumPerfZone.UpdateFrequency = 0.1f;
    PerformanceZones.Add(MediumPerfZone);

    FCrowd_PerformanceZone LowPerfZone;
    LowPerfZone.ZoneCenter = FVector::ZeroVector;
    LowPerfZone.ZoneRadius = 3000.0f;
    LowPerfZone.OptimizationLevel = ECrowd_OptimizationLevel::Low;
    LowPerfZone.MaxCrowdAgents = 50;
    LowPerfZone.UpdateFrequency = 0.2f;
    PerformanceZones.Add(LowPerfZone);

    // Initialize LOD settings
    LODSettings.HighDetailDistance = 500.0f;
    LODSettings.MediumDetailDistance = 1000.0f;
    LODSettings.LowDetailDistance = 2000.0f;
    LODSettings.CullingDistance = 5000.0f;
    LODSettings.bEnableOcclusionCulling = true;
}

void ACrowd_PerformanceOptimizer::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd Performance Optimizer initialized"));
    SetupPerformanceZones();
}

void ACrowd_PerformanceOptimizer::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    LastPerformanceUpdate += DeltaTime;
    
    if (LastPerformanceUpdate >= PerformanceUpdateInterval)
    {
        OptimizeCrowdPerformance();
        UpdateLODLevels();
        UpdateStreamingZones();
        LastPerformanceUpdate = 0.0f;
    }
}

void ACrowd_PerformanceOptimizer::OptimizeCrowdPerformance()
{
    if (!GetWorld())
    {
        return;
    }

    // Calculate current frame rate
    CurrentFrameRate = 1.0f / GetWorld()->GetDeltaSeconds();
    
    // Update crowd count
    CurrentCrowdCount = ManagedCrowdActors.Num();

    // Adaptive optimization based on performance
    if (bEnableAdaptiveOptimization)
    {
        if (CurrentFrameRate < TargetFrameRate * 0.8f) // Performance is poor
        {
            // Reduce crowd density
            for (FCrowd_PerformanceZone& Zone : PerformanceZones)
            {
                Zone.MaxCrowdAgents = FMath::Max(Zone.MaxCrowdAgents - 10, 10);
                Zone.UpdateFrequency = FMath::Min(Zone.UpdateFrequency + 0.05f, 1.0f);
            }
            
            // Increase culling distance
            LODSettings.CullingDistance = FMath::Max(LODSettings.CullingDistance - 500.0f, 1000.0f);
        }
        else if (CurrentFrameRate > TargetFrameRate * 1.1f) // Performance is good
        {
            // Increase crowd density gradually
            for (FCrowd_PerformanceZone& Zone : PerformanceZones)
            {
                Zone.MaxCrowdAgents = FMath::Min(Zone.MaxCrowdAgents + 5, 500);
                Zone.UpdateFrequency = FMath::Max(Zone.UpdateFrequency - 0.01f, 0.05f);
            }
            
            // Decrease culling distance
            LODSettings.CullingDistance = FMath::Min(LODSettings.CullingDistance + 200.0f, 8000.0f);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Performance Optimizer: FPS=%.1f, Crowd Count=%d"), CurrentFrameRate, CurrentCrowdCount);
}

void ACrowd_PerformanceOptimizer::UpdateLODLevels()
{
    if (!GetWorld())
    {
        return;
    }

    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC || !PC->GetPawn())
    {
        return;
    }

    FVector PlayerLocation = PC->GetPawn()->GetActorLocation();

    // Update LOD for all managed crowd actors
    for (AActor* CrowdActor : ManagedCrowdActors)
    {
        if (!IsValid(CrowdActor))
        {
            continue;
        }

        float Distance = FVector::Dist(PlayerLocation, CrowdActor->GetActorLocation());
        
        // Determine LOD level based on distance
        int32 LODLevel = 3; // Default to lowest detail
        
        if (Distance <= LODSettings.HighDetailDistance)
        {
            LODLevel = 0; // Highest detail
        }
        else if (Distance <= LODSettings.MediumDetailDistance)
        {
            LODLevel = 1; // Medium detail
        }
        else if (Distance <= LODSettings.LowDetailDistance)
        {
            LODLevel = 2; // Low detail
        }

        // Apply occlusion culling if enabled
        if (LODSettings.bEnableOcclusionCulling && Distance > LODSettings.CullingDistance)
        {
            CrowdActor->SetActorHiddenInGame(true);
        }
        else
        {
            CrowdActor->SetActorHiddenInGame(false);
        }
    }
}

void ACrowd_PerformanceOptimizer::RegisterCrowdActor(AActor* CrowdActor)
{
    if (IsValid(CrowdActor) && !ManagedCrowdActors.Contains(CrowdActor))
    {
        ManagedCrowdActors.Add(CrowdActor);
        UE_LOG(LogTemp, Log, TEXT("Registered crowd actor: %s"), *CrowdActor->GetName());
    }
}

void ACrowd_PerformanceOptimizer::UnregisterCrowdActor(AActor* CrowdActor)
{
    if (ManagedCrowdActors.Contains(CrowdActor))
    {
        ManagedCrowdActors.Remove(CrowdActor);
        UE_LOG(LogTemp, Log, TEXT("Unregistered crowd actor: %s"), CrowdActor ? *CrowdActor->GetName() : TEXT("NULL"));
    }
}

ECrowd_OptimizationLevel ACrowd_PerformanceOptimizer::GetOptimizationLevelForLocation(const FVector& Location)
{
    float MinDistance = FLT_MAX;
    ECrowd_OptimizationLevel BestLevel = ECrowd_OptimizationLevel::Minimal;

    for (const FCrowd_PerformanceZone& Zone : PerformanceZones)
    {
        float Distance = FVector::Dist(Location, Zone.ZoneCenter);
        if (Distance <= Zone.ZoneRadius && Distance < MinDistance)
        {
            MinDistance = Distance;
            BestLevel = Zone.OptimizationLevel;
        }
    }

    return BestLevel;
}

bool ACrowd_PerformanceOptimizer::ShouldCullCrowdActor(const FVector& ActorLocation, const FVector& ViewerLocation)
{
    float Distance = FVector::Dist(ActorLocation, ViewerLocation);
    
    if (Distance > LODSettings.CullingDistance)
    {
        return true;
    }

    // Additional occlusion culling logic could be added here
    if (LODSettings.bEnableOcclusionCulling)
    {
        // Simple frustum culling check could be implemented
        return false;
    }

    return false;
}

void ACrowd_PerformanceOptimizer::UpdateStreamingZones()
{
    if (!GetWorld())
    {
        return;
    }

    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC || !PC->GetPawn())
    {
        return;
    }

    FVector PlayerLocation = PC->GetPawn()->GetActorLocation();

    // Update streaming zones based on player location
    for (FCrowd_PerformanceZone& Zone : PerformanceZones)
    {
        float DistanceToPlayer = FVector::Dist(PlayerLocation, Zone.ZoneCenter);
        
        // Adjust zone priority based on distance to player
        if (DistanceToPlayer <= StreamingRadius * 0.5f)
        {
            Zone.OptimizationLevel = ECrowd_OptimizationLevel::Ultra_High;
        }
        else if (DistanceToPlayer <= StreamingRadius)
        {
            Zone.OptimizationLevel = ECrowd_OptimizationLevel::High;
        }
        else if (DistanceToPlayer <= StreamingRadius * 2.0f)
        {
            Zone.OptimizationLevel = ECrowd_OptimizationLevel::Medium;
        }
        else
        {
            Zone.OptimizationLevel = ECrowd_OptimizationLevel::Low;
        }
    }
}

float ACrowd_PerformanceOptimizer::GetCurrentPerformanceMetric()
{
    // Return a normalized performance metric (0.0 = poor, 1.0 = excellent)
    float FrameRateRatio = FMath::Clamp(CurrentFrameRate / TargetFrameRate, 0.0f, 2.0f);
    float CrowdLoadRatio = FMath::Clamp(1.0f - (float(CurrentCrowdCount) / float(MaxTotalCrowdAgents)), 0.0f, 1.0f);
    
    return (FrameRateRatio + CrowdLoadRatio) * 0.5f;
}

void ACrowd_PerformanceOptimizer::SetupPerformanceZones()
{
    UE_LOG(LogTemp, Warning, TEXT("Setting up %d performance zones"), PerformanceZones.Num());
    
    for (int32 i = 0; i < PerformanceZones.Num(); i++)
    {
        const FCrowd_PerformanceZone& Zone = PerformanceZones[i];
        UE_LOG(LogTemp, Log, TEXT("Zone %d: Center=(%s), Radius=%.1f, Level=%d"), 
            i, *Zone.ZoneCenter.ToString(), Zone.ZoneRadius, (int32)Zone.OptimizationLevel);
    }
}

void ACrowd_PerformanceOptimizer::VisualizePerformanceZones()
{
    if (!GetWorld())
    {
        return;
    }

    // Draw debug spheres for each performance zone
    for (int32 i = 0; i < PerformanceZones.Num(); i++)
    {
        const FCrowd_PerformanceZone& Zone = PerformanceZones[i];
        
        FColor ZoneColor = FColor::Green;
        switch (Zone.OptimizationLevel)
        {
            case ECrowd_OptimizationLevel::Ultra_High:
                ZoneColor = FColor::Red;
                break;
            case ECrowd_OptimizationLevel::High:
                ZoneColor = FColor::Orange;
                break;
            case ECrowd_OptimizationLevel::Medium:
                ZoneColor = FColor::Yellow;
                break;
            case ECrowd_OptimizationLevel::Low:
                ZoneColor = FColor::Green;
                break;
            case ECrowd_OptimizationLevel::Minimal:
                ZoneColor = FColor::Blue;
                break;
        }

        DrawDebugSphere(GetWorld(), Zone.ZoneCenter, Zone.ZoneRadius, 32, ZoneColor, false, 5.0f, 0, 2.0f);
    }

    UE_LOG(LogTemp, Warning, TEXT("Performance zones visualization updated"));
}