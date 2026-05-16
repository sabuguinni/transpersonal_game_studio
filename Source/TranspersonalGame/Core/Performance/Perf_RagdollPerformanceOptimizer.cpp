#include "Perf_RagdollPerformanceOptimizer.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/StaticMeshActor.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "Kismet/GameplayStatics.h"

UPerf_RagdollPerformanceOptimizer::UPerf_RagdollPerformanceOptimizer()
{
    MaxSimultaneousRagdolls = 10;
    CullingDistance = 5000.0f;
    bEnableAutomaticOptimization = true;
    PerformanceThresholdMs = 16.67f; // 60 FPS target
}

void UPerf_RagdollPerformanceOptimizer::OptimizeRagdollPerformance(UWorld* World)
{
    if (!World || !bEnableAutomaticOptimization)
    {
        return;
    }

    // Update current metrics
    UpdateRagdollMetrics(World);

    // Check if performance optimization is needed
    if (CurrentMetrics.bPerformanceWarning || CurrentMetrics.ActiveRagdolls > MaxSimultaneousRagdolls)
    {
        UE_LOG(LogTemp, Warning, TEXT("Ragdoll performance critical - applying optimizations"));
        
        // Limit active ragdolls
        LimitActiveRagdolls(World, MaxSimultaneousRagdolls);
        
        // Disable distant ragdolls
        DisableDistantRagdolls(World, CullingDistance);
        
        // Update metrics after optimization
        UpdateRagdollMetrics(World);
    }
}

void UPerf_RagdollPerformanceOptimizer::SetRagdollLODLevel(AActor* RagdollActor, EPerf_RagdollLODLevel LODLevel)
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
    case EPerf_RagdollLODLevel::High:
        SkeletalMesh->SetForcedLOD(0);
        SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        break;
        
    case EPerf_RagdollLODLevel::Medium:
        SkeletalMesh->SetForcedLOD(1);
        SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        break;
        
    case EPerf_RagdollLODLevel::Low:
        SkeletalMesh->SetForcedLOD(2);
        SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        break;
        
    case EPerf_RagdollLODLevel::Disabled:
        SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        SkeletalMesh->SetSimulatePhysics(false);
        break;
    }
}

FPerf_RagdollMetrics UPerf_RagdollPerformanceOptimizer::GetRagdollMetrics(UWorld* World)
{
    UpdateRagdollMetrics(World);
    return CurrentMetrics;
}

void UPerf_RagdollPerformanceOptimizer::LimitActiveRagdolls(UWorld* World, int32 MaxCount)
{
    if (!World)
    {
        return;
    }

    TArray<AActor*> RagdollActors;
    
    // Find all ragdoll actors
    for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
    {
        AActor* Actor = *ActorIterator;
        if (Actor)
        {
            USkeletalMeshComponent* SkeletalMesh = Actor->FindComponentByClass<USkeletalMeshComponent>();
            if (SkeletalMesh && SkeletalMesh->IsSimulatingPhysics())
            {
                RagdollActors.Add(Actor);
            }
        }
    }

    // If we have too many ragdolls, disable the furthest ones
    if (RagdollActors.Num() > MaxCount)
    {
        // Get player location for distance calculation
        FVector PlayerLocation = FVector::ZeroVector;
        APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
        if (PC && PC->PlayerCameraManager)
        {
            PlayerLocation = PC->PlayerCameraManager->GetCameraLocation();
        }

        // Sort by distance from player
        RagdollActors.Sort([PlayerLocation](const AActor& A, const AActor& B)
        {
            float DistA = FVector::Dist(A.GetActorLocation(), PlayerLocation);
            float DistB = FVector::Dist(B.GetActorLocation(), PlayerLocation);
            return DistA < DistB;
        });

        // Disable ragdolls beyond the limit
        for (int32 i = MaxCount; i < RagdollActors.Num(); ++i)
        {
            SetRagdollLODLevel(RagdollActors[i], EPerf_RagdollLODLevel::Disabled);
        }
    }
}

void UPerf_RagdollPerformanceOptimizer::DisableDistantRagdolls(UWorld* World, float MaxDistance)
{
    if (!World)
    {
        return;
    }

    // Get player location
    FVector PlayerLocation = FVector::ZeroVector;
    APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
    if (PC && PC->PlayerCameraManager)
    {
        PlayerLocation = PC->PlayerCameraManager->GetCameraLocation();
    }

    // Check all ragdoll actors
    for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
    {
        AActor* Actor = *ActorIterator;
        if (Actor)
        {
            USkeletalMeshComponent* SkeletalMesh = Actor->FindComponentByClass<USkeletalMeshComponent>();
            if (SkeletalMesh && SkeletalMesh->IsSimulatingPhysics())
            {
                float Distance = FVector::Dist(Actor->GetActorLocation(), PlayerLocation);
                
                if (Distance > MaxDistance)
                {
                    SetRagdollLODLevel(Actor, EPerf_RagdollLODLevel::Disabled);
                }
                else
                {
                    // Apply distance-based LOD
                    ApplyLODOptimizations(Actor, Distance);
                }
            }
        }
    }
}

bool UPerf_RagdollPerformanceOptimizer::IsRagdollPerformanceCritical(UWorld* World)
{
    UpdateRagdollMetrics(World);
    return CurrentMetrics.bPerformanceWarning;
}

void UPerf_RagdollPerformanceOptimizer::EnableRagdollCulling(bool bEnable)
{
    bEnableAutomaticOptimization = bEnable;
    UE_LOG(LogTemp, Log, TEXT("Ragdoll culling %s"), bEnable ? TEXT("enabled") : TEXT("disabled"));
}

void UPerf_RagdollPerformanceOptimizer::RunRagdollPerformanceTest()
{
    UWorld* World = GEngine->GetCurrentPlayWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Warning, TEXT("No world found for ragdoll performance test"));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("Running ragdoll performance test..."));
    
    FPerf_RagdollMetrics Metrics = GetRagdollMetrics(World);
    
    UE_LOG(LogTemp, Log, TEXT("Ragdoll Performance Test Results:"));
    UE_LOG(LogTemp, Log, TEXT("- Active Ragdolls: %d/%d"), Metrics.ActiveRagdolls, Metrics.MaxRagdolls);
    UE_LOG(LogTemp, Log, TEXT("- Average Frame Time: %.2f ms"), Metrics.AverageFrameTime);
    UE_LOG(LogTemp, Log, TEXT("- Physics Time: %.2f ms"), Metrics.PhysicsTimeMs);
    UE_LOG(LogTemp, Log, TEXT("- Performance Warning: %s"), Metrics.bPerformanceWarning ? TEXT("YES") : TEXT("NO"));
    
    if (Metrics.bPerformanceWarning)
    {
        UE_LOG(LogTemp, Warning, TEXT("Performance optimization recommended!"));
        OptimizeRagdollPerformance(World);
    }
}

void UPerf_RagdollPerformanceOptimizer::UpdateRagdollMetrics(UWorld* World)
{
    if (!World)
    {
        return;
    }

    CurrentMetrics.ActiveRagdolls = 0;
    CurrentMetrics.MaxRagdolls = MaxSimultaneousRagdolls;

    // Count active ragdoll actors
    for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
    {
        AActor* Actor = *ActorIterator;
        if (Actor)
        {
            USkeletalMeshComponent* SkeletalMesh = Actor->FindComponentByClass<USkeletalMeshComponent>();
            if (SkeletalMesh && SkeletalMesh->IsSimulatingPhysics())
            {
                CurrentMetrics.ActiveRagdolls++;
            }
        }
    }

    // Get frame time metrics
    CurrentMetrics.AverageFrameTime = FApp::GetDeltaTime() * 1000.0f; // Convert to milliseconds
    CurrentMetrics.PhysicsTimeMs = CurrentMetrics.AverageFrameTime * 0.3f; // Estimate physics time

    // Check performance warning threshold
    CurrentMetrics.bPerformanceWarning = (CurrentMetrics.AverageFrameTime > PerformanceThresholdMs) || 
                                        (CurrentMetrics.ActiveRagdolls > MaxSimultaneousRagdolls);
}

void UPerf_RagdollPerformanceOptimizer::ApplyLODOptimizations(AActor* RagdollActor, float Distance)
{
    if (!RagdollActor)
    {
        return;
    }

    EPerf_RagdollLODLevel LODLevel = EPerf_RagdollLODLevel::High;

    if (Distance > CullingDistance * 0.8f)
    {
        LODLevel = EPerf_RagdollLODLevel::Low;
    }
    else if (Distance > CullingDistance * 0.5f)
    {
        LODLevel = EPerf_RagdollLODLevel::Medium;
    }

    SetRagdollLODLevel(RagdollActor, LODLevel);
}

bool UPerf_RagdollPerformanceOptimizer::ShouldCullRagdoll(AActor* RagdollActor, const FVector& ViewLocation)
{
    if (!RagdollActor)
    {
        return true;
    }

    float Distance = FVector::Dist(RagdollActor->GetActorLocation(), ViewLocation);
    return Distance > CullingDistance;
}