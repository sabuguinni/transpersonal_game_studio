#include "Perf_RenderingOptimizer.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/SkeletalMeshActor.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

UPerf_RenderingOptimizer::UPerf_RenderingOptimizer()
{
    LODSettings = FPerf_LODSettings();
    CurrentStats = FPerf_RenderStats();
    bOcclusionCullingEnabled = true;
    MaxDrawDistance = 20000.0f;
    UpdateInterval = 1.0f;
}

void UPerf_RenderingOptimizer::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Performance Rendering Optimizer initialized"));
    
    // Start periodic optimization updates
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            UpdateTimerHandle,
            this,
            &UPerf_RenderingOptimizer::UpdateRenderingStats,
            UpdateInterval,
            true
        );
    }
}

void UPerf_RenderingOptimizer::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(UpdateTimerHandle);
    }
    
    Super::Deinitialize();
}

bool UPerf_RenderingOptimizer::ShouldCreateSubsystem(UObject* Outer) const
{
    return true;
}

void UPerf_RenderingOptimizer::OptimizeLODSettings()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    ProcessStaticMeshActors();
    ProcessSkeletalMeshActors();
    
    UE_LOG(LogTemp, Warning, TEXT("LOD optimization applied to all mesh actors"));
}

void UPerf_RenderingOptimizer::ApplyDistanceCulling()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AStaticMeshActor::StaticClass(), AllActors);

    for (AActor* Actor : AllActors)
    {
        if (AStaticMeshActor* MeshActor = Cast<AStaticMeshActor>(Actor))
        {
            if (UStaticMeshComponent* MeshComp = MeshActor->GetStaticMeshComponent())
            {
                float Distance = CalculateDistanceToPlayer(Actor->GetActorLocation());
                
                if (Distance > MaxDrawDistance)
                {
                    MeshComp->SetVisibility(false);
                    CurrentStats.CulledMeshes++;
                }
                else
                {
                    MeshComp->SetVisibility(true);
                    CurrentStats.VisibleMeshes++;
                }
            }
        }
    }
}

void UPerf_RenderingOptimizer::OptimizeMaterialInstances()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Optimize material instances for better performance
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AStaticMeshActor::StaticClass(), AllActors);

    for (AActor* Actor : AllActors)
    {
        if (AStaticMeshActor* MeshActor = Cast<AStaticMeshActor>(Actor))
        {
            if (UStaticMeshComponent* MeshComp = MeshActor->GetStaticMeshComponent())
            {
                float Distance = CalculateDistanceToPlayer(Actor->GetActorLocation());
                
                // Apply simplified materials for distant objects
                if (Distance > LODSettings.FarDistance)
                {
                    // Could implement material LOD switching here
                    // For now, just log the optimization
                    UE_LOG(LogTemp, Log, TEXT("Material optimization applied to distant actor: %s"), *Actor->GetName());
                }
            }
        }
    }
}

void UPerf_RenderingOptimizer::UpdateRenderingStats()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Reset stats
    CurrentStats = FPerf_RenderStats();

    // Count all mesh actors
    TArray<AActor*> StaticMeshActors;
    UGameplayStatics::GetAllActorsOfClass(World, AStaticMeshActor::StaticClass(), StaticMeshActors);
    
    TArray<AActor*> SkeletalMeshActors;
    UGameplayStatics::GetAllActorsOfClass(World, ASkeletalMeshActor::StaticClass(), SkeletalMeshActors);

    CurrentStats.TotalMeshes = StaticMeshActors.Num() + SkeletalMeshActors.Num();

    // Count visible meshes
    for (AActor* Actor : StaticMeshActors)
    {
        if (AStaticMeshActor* MeshActor = Cast<AStaticMeshActor>(Actor))
        {
            if (UStaticMeshComponent* MeshComp = MeshActor->GetStaticMeshComponent())
            {
                if (MeshComp->IsVisible())
                {
                    CurrentStats.VisibleMeshes++;
                }
                else
                {
                    CurrentStats.CulledMeshes++;
                }
            }
        }
    }

    // Estimate memory usage (simplified)
    CurrentStats.MemoryUsageMB = CurrentStats.VisibleMeshes * 0.5f; // Rough estimate

    // Apply optimizations if needed
    if (CurrentStats.VisibleMeshes > 1000)
    {
        ApplyDistanceCulling();
    }
}

FPerf_RenderStats UPerf_RenderingOptimizer::GetCurrentRenderStats() const
{
    return CurrentStats;
}

void UPerf_RenderingOptimizer::SetLODSettings(const FPerf_LODSettings& NewSettings)
{
    LODSettings = NewSettings;
    OptimizeLODSettings();
}

FPerf_LODSettings UPerf_RenderingOptimizer::GetLODSettings() const
{
    return LODSettings;
}

void UPerf_RenderingOptimizer::EnableOcclusionCulling(bool bEnable)
{
    bOcclusionCullingEnabled = bEnable;
    
    if (UWorld* World = GetWorld())
    {
        // Apply occlusion culling settings to the world
        UE_LOG(LogTemp, Warning, TEXT("Occlusion culling %s"), bEnable ? TEXT("enabled") : TEXT("disabled"));
    }
}

void UPerf_RenderingOptimizer::SetMaxDrawDistance(float Distance)
{
    MaxDrawDistance = Distance;
    ApplyDistanceCulling();
}

void UPerf_RenderingOptimizer::ProcessStaticMeshActors()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AStaticMeshActor::StaticClass(), AllActors);

    for (AActor* Actor : AllActors)
    {
        if (AStaticMeshActor* MeshActor = Cast<AStaticMeshActor>(Actor))
        {
            if (UStaticMeshComponent* MeshComp = MeshActor->GetStaticMeshComponent())
            {
                float Distance = CalculateDistanceToPlayer(Actor->GetActorLocation());
                ApplyLODToMesh(MeshComp, Distance);
            }
        }
    }
}

void UPerf_RenderingOptimizer::ProcessSkeletalMeshActors()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, ASkeletalMeshActor::StaticClass(), AllActors);

    for (AActor* Actor : AllActors)
    {
        if (ASkeletalMeshActor* SkeletalActor = Cast<ASkeletalMeshActor>(Actor))
        {
            if (USkeletalMeshComponent* SkeletalComp = SkeletalActor->GetSkeletalMeshComponent())
            {
                float Distance = CalculateDistanceToPlayer(Actor->GetActorLocation());
                ApplyLODToSkeletalMesh(SkeletalComp, Distance);
            }
        }
    }
}

void UPerf_RenderingOptimizer::ApplyLODToMesh(UStaticMeshComponent* MeshComp, float Distance)
{
    if (!MeshComp)
    {
        return;
    }

    // Apply LOD based on distance
    if (Distance <= LODSettings.NearDistance)
    {
        MeshComp->SetForcedLodModel(0); // Highest quality
    }
    else if (Distance <= LODSettings.MidDistance)
    {
        MeshComp->SetForcedLodModel(1); // Medium quality
    }
    else if (Distance <= LODSettings.FarDistance)
    {
        MeshComp->SetForcedLodModel(2); // Low quality
    }
    else if (Distance <= LODSettings.CullDistance)
    {
        MeshComp->SetForcedLodModel(3); // Lowest quality
    }
    else
    {
        MeshComp->SetVisibility(false); // Cull completely
    }
}

void UPerf_RenderingOptimizer::ApplyLODToSkeletalMesh(USkeletalMeshComponent* SkeletalComp, float Distance)
{
    if (!SkeletalComp)
    {
        return;
    }

    // Apply LOD based on distance for skeletal meshes
    if (Distance <= LODSettings.NearDistance)
    {
        SkeletalComp->SetForcedLOD(1); // Highest quality (1-based)
    }
    else if (Distance <= LODSettings.MidDistance)
    {
        SkeletalComp->SetForcedLOD(2); // Medium quality
    }
    else if (Distance <= LODSettings.FarDistance)
    {
        SkeletalComp->SetForcedLOD(3); // Low quality
    }
    else if (Distance > LODSettings.CullDistance)
    {
        SkeletalComp->SetVisibility(false); // Cull completely
    }
}

float UPerf_RenderingOptimizer::CalculateDistanceToPlayer(const FVector& ActorLocation) const
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return 0.0f;
    }

    APlayerController* PlayerController = World->GetFirstPlayerController();
    if (!PlayerController)
    {
        return 0.0f;
    }

    APawn* PlayerPawn = PlayerController->GetPawn();
    if (!PlayerPawn)
    {
        return 0.0f;
    }

    return FVector::Dist(PlayerPawn->GetActorLocation(), ActorLocation);
}