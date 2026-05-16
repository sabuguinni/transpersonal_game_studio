#include "Perf_RagdollPerformanceOptimizer.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

APerf_RagdollPerformanceOptimizer::APerf_RagdollPerformanceOptimizer()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Update 10 times per second

    // Initialize default LOD settings
    LODSettings.HighDetailDistance = 1000.0f;
    LODSettings.MediumDetailDistance = 2500.0f;
    LODSettings.LowDetailDistance = 5000.0f;
    LODSettings.CullDistance = 10000.0f;
    LODSettings.MaxActiveRagdolls = 15;
    LODSettings.UpdateFrequency = 0.5f;

    // Initialize performance settings
    TargetFrameTime = 16.67f; // 60 FPS target
    bEnableAutomaticOptimization = true;
    bEnableLODSystem = true;
    bEnableDistanceCulling = true;

    // Initialize metrics
    CurrentMetrics = FPerf_RagdollMetrics();
    LastOptimizationTime = 0.0f;
    AverageFrameTime = 16.67f;
    LastFrameTime = 0.0f;

    // Reserve space for frame time history
    FrameTimeHistory.Reserve(MaxFrameTimeHistory);
}

void APerf_RagdollPerformanceOptimizer::BeginPlay()
{
    Super::BeginPlay();

    // Start optimization timer
    if (bEnableAutomaticOptimization)
    {
        GetWorldTimerManager().SetTimer(
            OptimizationTimerHandle,
            this,
            &APerf_RagdollPerformanceOptimizer::OptimizeRagdollActors,
            LODSettings.UpdateFrequency,
            true
        );
    }

    // Start metrics update timer
    GetWorldTimerManager().SetTimer(
        MetricsUpdateTimerHandle,
        this,
        &APerf_RagdollPerformanceOptimizer::UpdatePerformanceMetrics,
        0.1f,
        true
    );

    // Initial scan for ragdoll actors
    FindRagdollActors();

    UE_LOG(LogTemp, Log, TEXT("RagdollPerformanceOptimizer: System initialized"));
}

void APerf_RagdollPerformanceOptimizer::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // Clear timers
    GetWorldTimerManager().ClearTimer(OptimizationTimerHandle);
    GetWorldTimerManager().ClearTimer(MetricsUpdateTimerHandle);

    Super::EndPlay(EndPlayReason);
}

void APerf_RagdollPerformanceOptimizer::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Track frame time
    float CurrentFrameTime = DeltaTime * 1000.0f; // Convert to milliseconds
    FrameTimeHistory.Add(CurrentFrameTime);

    if (FrameTimeHistory.Num() > MaxFrameTimeHistory)
    {
        FrameTimeHistory.RemoveAt(0);
    }

    // Calculate average frame time
    if (FrameTimeHistory.Num() > 0)
    {
        float Sum = 0.0f;
        for (float FrameTime : FrameTimeHistory)
        {
            Sum += FrameTime;
        }
        AverageFrameTime = Sum / FrameTimeHistory.Num();
    }

    LastFrameTime = CurrentFrameTime;

    // Check performance thresholds
    CheckPerformanceThresholds();
}

FPerf_RagdollMetrics APerf_RagdollPerformanceOptimizer::AnalyzeRagdollPerformance()
{
    FPerf_RagdollMetrics Metrics;

    // Find and analyze ragdoll actors
    FindRagdollActors();

    Metrics.ActiveRagdolls = TrackedRagdolls.Num();
    Metrics.EstimatedCPUCost = Metrics.ActiveRagdolls * 2.5f; // Estimated 2.5ms per ragdoll
    Metrics.EstimatedMemoryUsage = Metrics.ActiveRagdolls * 1.2f; // Estimated 1.2MB per ragdoll
    Metrics.bOptimizationNeeded = Metrics.ActiveRagdolls > LODSettings.MaxActiveRagdolls || 
                                  Metrics.EstimatedCPUCost > TargetFrameTime;

    // Calculate average distance to player
    AActor* PlayerActor = GetPlayerActor();
    if (PlayerActor && TrackedRagdolls.Num() > 0)
    {
        float TotalDistance = 0.0f;
        int32 ValidActors = 0;

        for (const TWeakObjectPtr<AActor>& WeakRagdoll : TrackedRagdolls)
        {
            if (AActor* RagdollActor = WeakRagdoll.Get())
            {
                float Distance = FVector::Dist(PlayerActor->GetActorLocation(), RagdollActor->GetActorLocation());
                TotalDistance += Distance;
                ValidActors++;
            }
        }

        if (ValidActors > 0)
        {
            Metrics.AverageDistanceToPlayer = TotalDistance / ValidActors;
        }
    }

    CurrentMetrics = Metrics;
    return Metrics;
}

void APerf_RagdollPerformanceOptimizer::OptimizeRagdollActors()
{
    if (!bEnableAutomaticOptimization)
    {
        return;
    }

    // Update metrics
    AnalyzeRagdollPerformance();

    // Apply optimizations based on performance
    if (bEnableLODSystem)
    {
        UpdateRagdollLOD();
    }

    if (bEnableDistanceCulling)
    {
        CullDistantRagdolls();
    }

    // Limit active ragdolls if needed
    if (CurrentMetrics.ActiveRagdolls > LODSettings.MaxActiveRagdolls)
    {
        LimitActiveRagdolls();
    }

    LastOptimizationTime = GetWorld()->GetTimeSeconds();

    UE_LOG(LogTemp, Verbose, TEXT("RagdollOptimizer: Optimization pass completed. Active ragdolls: %d"), 
           CurrentMetrics.ActiveRagdolls);
}

void APerf_RagdollPerformanceOptimizer::UpdateRagdollLOD()
{
    AActor* PlayerActor = GetPlayerActor();
    if (!PlayerActor)
    {
        return;
    }

    for (const TWeakObjectPtr<AActor>& WeakRagdoll : TrackedRagdolls)
    {
        if (AActor* RagdollActor = WeakRagdoll.Get())
        {
            float Distance = FVector::Dist(PlayerActor->GetActorLocation(), RagdollActor->GetActorLocation());
            EPerf_RagdollLODLevel LODLevel = CalculateRagdollLOD(RagdollActor, Distance);
            ApplyRagdollLOD(RagdollActor, LODLevel);
        }
    }
}

EPerf_RagdollLODLevel APerf_RagdollPerformanceOptimizer::CalculateRagdollLOD(AActor* RagdollActor, float DistanceToPlayer)
{
    if (!RagdollActor)
    {
        return EPerf_RagdollLODLevel::Culled;
    }

    if (DistanceToPlayer <= LODSettings.HighDetailDistance)
    {
        return EPerf_RagdollLODLevel::HighDetail;
    }
    else if (DistanceToPlayer <= LODSettings.MediumDetailDistance)
    {
        return EPerf_RagdollLODLevel::MediumDetail;
    }
    else if (DistanceToPlayer <= LODSettings.LowDetailDistance)
    {
        return EPerf_RagdollLODLevel::LowDetail;
    }
    else
    {
        return EPerf_RagdollLODLevel::Culled;
    }
}

void APerf_RagdollPerformanceOptimizer::ApplyRagdollLOD(AActor* RagdollActor, EPerf_RagdollLODLevel LODLevel)
{
    if (!RagdollActor)
    {
        return;
    }

    USkeletalMeshComponent* SkeletalMesh = RagdollActor->FindComponentByClass<USkeletalMeshComponent>();
    if (!SkeletalMesh)
    {
        return;
    }

    switch (LODLevel)
    {
        case EPerf_RagdollLODLevel::HighDetail:
            SkeletalMesh->SetVisibility(true);
            SkeletalMesh->SetComponentTickEnabled(true);
            // Full physics simulation
            break;

        case EPerf_RagdollLODLevel::MediumDetail:
            SkeletalMesh->SetVisibility(true);
            SkeletalMesh->SetComponentTickEnabled(true);
            // Reduced physics update rate could be implemented here
            break;

        case EPerf_RagdollLODLevel::LowDetail:
            SkeletalMesh->SetVisibility(true);
            SkeletalMesh->SetComponentTickEnabled(false);
            // Minimal or no physics simulation
            break;

        case EPerf_RagdollLODLevel::Culled:
            SkeletalMesh->SetVisibility(false);
            SkeletalMesh->SetComponentTickEnabled(false);
            // No simulation
            break;
    }
}

void APerf_RagdollPerformanceOptimizer::CullDistantRagdolls()
{
    AActor* PlayerActor = GetPlayerActor();
    if (!PlayerActor)
    {
        return;
    }

    for (const TWeakObjectPtr<AActor>& WeakRagdoll : TrackedRagdolls)
    {
        if (AActor* RagdollActor = WeakRagdoll.Get())
        {
            float Distance = FVector::Dist(PlayerActor->GetActorLocation(), RagdollActor->GetActorLocation());
            
            if (Distance > LODSettings.CullDistance)
            {
                ApplyRagdollLOD(RagdollActor, EPerf_RagdollLODLevel::Culled);
            }
        }
    }
}

void APerf_RagdollPerformanceOptimizer::LimitActiveRagdolls()
{
    if (TrackedRagdolls.Num() <= LODSettings.MaxActiveRagdolls)
    {
        return;
    }

    AActor* PlayerActor = GetPlayerActor();
    if (!PlayerActor)
    {
        return;
    }

    // Sort ragdolls by distance to player
    TrackedRagdolls.Sort([PlayerActor](const TWeakObjectPtr<AActor>& A, const TWeakObjectPtr<AActor>& B)
    {
        AActor* ActorA = A.Get();
        AActor* ActorB = B.Get();
        
        if (!ActorA || !ActorB)
        {
            return false;
        }

        float DistanceA = FVector::Dist(PlayerActor->GetActorLocation(), ActorA->GetActorLocation());
        float DistanceB = FVector::Dist(PlayerActor->GetActorLocation(), ActorB->GetActorLocation());
        
        return DistanceA < DistanceB;
    });

    // Cull the furthest ragdolls
    for (int32 i = LODSettings.MaxActiveRagdolls; i < TrackedRagdolls.Num(); ++i)
    {
        if (AActor* RagdollActor = TrackedRagdolls[i].Get())
        {
            ApplyRagdollLOD(RagdollActor, EPerf_RagdollLODLevel::Culled);
        }
    }
}

float APerf_RagdollPerformanceOptimizer::GetRagdollFrameTime() const
{
    return AverageFrameTime;
}

bool APerf_RagdollPerformanceOptimizer::IsPerformanceAcceptable() const
{
    return AverageFrameTime <= TargetFrameTime && 
           CurrentMetrics.ActiveRagdolls <= LODSettings.MaxActiveRagdolls;
}

void APerf_RagdollPerformanceOptimizer::DebugRagdollPerformance()
{
    if (!GEngine)
    {
        return;
    }

    FPerf_RagdollMetrics Metrics = AnalyzeRagdollPerformance();

    FString DebugText = FString::Printf(TEXT(
        "=== RAGDOLL PERFORMANCE DEBUG ===\n"
        "Active Ragdolls: %d\n"
        "Estimated CPU Cost: %.1fms\n"
        "Estimated Memory: %.1fMB\n"
        "Average Frame Time: %.2fms\n"
        "Target Frame Time: %.2fms\n"
        "Performance Acceptable: %s\n"
        "Optimization Needed: %s\n"
        "Average Distance to Player: %.1f units"
    ),
        Metrics.ActiveRagdolls,
        Metrics.EstimatedCPUCost,
        Metrics.EstimatedMemoryUsage,
        AverageFrameTime,
        TargetFrameTime,
        IsPerformanceAcceptable() ? TEXT("YES") : TEXT("NO"),
        Metrics.bOptimizationNeeded ? TEXT("YES") : TEXT("NO"),
        Metrics.AverageDistanceToPlayer
    );

    GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, DebugText);
    UE_LOG(LogTemp, Warning, TEXT("%s"), *DebugText);
}

void APerf_RagdollPerformanceOptimizer::FindRagdollActors()
{
    TrackedRagdolls.Empty();

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
    {
        AActor* Actor = *ActorIterator;
        if (!Actor)
        {
            continue;
        }

        // Check if actor has skeletal mesh with physics simulation
        USkeletalMeshComponent* SkeletalMesh = Actor->FindComponentByClass<USkeletalMeshComponent>();
        if (SkeletalMesh && SkeletalMesh->IsSimulatingPhysics())
        {
            TrackedRagdolls.Add(Actor);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("RagdollOptimizer: Found %d ragdoll actors"), TrackedRagdolls.Num());
}

void APerf_RagdollPerformanceOptimizer::UpdatePerformanceMetrics()
{
    // Clean up invalid weak pointers
    TrackedRagdolls.RemoveAll([](const TWeakObjectPtr<AActor>& WeakPtr)
    {
        return !WeakPtr.IsValid();
    });

    // Update current metrics
    AnalyzeRagdollPerformance();
}

void APerf_RagdollPerformanceOptimizer::CheckPerformanceThresholds()
{
    if (AverageFrameTime > TargetFrameTime * 1.2f) // 20% over target
    {
        if (bEnableAutomaticOptimization && GetWorld()->GetTimeSeconds() - LastOptimizationTime > 1.0f)
        {
            // Force immediate optimization
            OptimizeRagdollActors();
            
            UE_LOG(LogTemp, Warning, TEXT("RagdollOptimizer: Performance threshold exceeded (%.2fms > %.2fms), forcing optimization"), 
                   AverageFrameTime, TargetFrameTime);
        }
    }
}

AActor* APerf_RagdollPerformanceOptimizer::GetPlayerActor()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }

    APlayerController* PlayerController = World->GetFirstPlayerController();
    if (!PlayerController)
    {
        return nullptr;
    }

    return PlayerController->GetPawn();
}