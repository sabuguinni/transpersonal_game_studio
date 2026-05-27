#include "Perf_RenderOptimizer.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/StaticMeshActor.h"

UPerf_RenderOptimizer::UPerf_RenderOptimizer()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
    
    // Initialize default settings
    LODSettings = FPerf_LODSettings();
    TargetFrameRate = 60.0f;
    MinFrameRate = 30.0f;
    bAutoAdjustLOD = true;
    bEnableRenderOptimization = true;
    
    // Initialize stats
    RenderStats = FPerf_RenderStats();
    
    // Initialize performance tracking
    LastFrameRate = 60.0f;
    for (int32 i = 0; i < 10; i++)
    {
        FrameRateHistory[i] = 60.0f;
    }
    FrameHistoryIndex = 0;
    bOptimizationActive = false;
    LastOptimizationTime = 0.0f;
    OptimizationInterval = 1.0f;
}

void UPerf_RenderOptimizer::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Render Optimizer initialized for %s"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
    
    // Start optimization
    bOptimizationActive = true;
}

void UPerf_RenderOptimizer::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bEnableRenderOptimization || !bOptimizationActive)
    {
        return;
    }
    
    // Update frame rate tracking
    float CurrentFrameRate = 1.0f / DeltaTime;
    FrameRateHistory[FrameHistoryIndex] = CurrentFrameRate;
    FrameHistoryIndex = (FrameHistoryIndex + 1) % 10;
    
    // Calculate average frame rate
    float AverageFrameRate = 0.0f;
    for (int32 i = 0; i < 10; i++)
    {
        AverageFrameRate += FrameRateHistory[i];
    }
    AverageFrameRate /= 10.0f;
    LastFrameRate = AverageFrameRate;
    
    // Auto-adjust LOD if enabled
    if (bAutoAdjustLOD)
    {
        AutoAdjustLODSettings(AverageFrameRate);
    }
    
    // Periodic optimization
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastOptimizationTime >= OptimizationInterval)
    {
        UpdateRenderStats();
        LastOptimizationTime = CurrentTime;
    }
    
    // Optimize owner actor if it exists
    if (GetOwner())
    {
        OptimizeRenderingForActor(GetOwner());
    }
}

void UPerf_RenderOptimizer::OptimizeRenderingForActor(AActor* Actor)
{
    if (!Actor || !bEnableRenderOptimization)
    {
        return;
    }
    
    float Distance = GetDistanceToPlayer(Actor);
    int32 OptimalLOD = GetOptimalLODLevel(Distance);
    
    // Apply LOD to static meshes
    TArray<UStaticMeshComponent*> StaticMeshes;
    Actor->GetComponents<UStaticMeshComponent>(StaticMeshes);
    for (UStaticMeshComponent* MeshComp : StaticMeshes)
    {
        OptimizeStaticMesh(MeshComp, Distance);
    }
    
    // Apply LOD to skeletal meshes
    TArray<USkeletalMeshComponent*> SkeletalMeshes;
    Actor->GetComponents<USkeletalMeshComponent>(SkeletalMeshes);
    for (USkeletalMeshComponent* MeshComp : SkeletalMeshes)
    {
        OptimizeSkeletalMesh(MeshComp, Distance);
    }
    
    // Update visibility based on distance
    UpdateActorVisibility(Actor, Distance);
}

void UPerf_RenderOptimizer::SetLODForActor(AActor* Actor, int32 LODLevel)
{
    if (!Actor)
    {
        return;
    }
    
    // Set LOD for static meshes
    TArray<UStaticMeshComponent*> StaticMeshes;
    Actor->GetComponents<UStaticMeshComponent>(StaticMeshes);
    for (UStaticMeshComponent* MeshComp : StaticMeshes)
    {
        if (MeshComp && MeshComp->GetStaticMesh())
        {
            MeshComp->SetForcedLodModel(LODLevel + 1); // UE5 uses 1-based LOD indexing
        }
    }
    
    // Set LOD for skeletal meshes
    TArray<USkeletalMeshComponent*> SkeletalMeshes;
    Actor->GetComponents<USkeletalMeshComponent>(SkeletalMeshes);
    for (USkeletalMeshComponent* MeshComp : SkeletalMeshes)
    {
        if (MeshComp && MeshComp->GetSkeletalMeshAsset())
        {
            MeshComp->SetForcedLOD(LODLevel + 1); // UE5 uses 1-based LOD indexing
        }
    }
}

void UPerf_RenderOptimizer::CullActorByDistance(AActor* Actor, bool bShouldCull)
{
    if (!Actor)
    {
        return;
    }
    
    Actor->SetActorHiddenInGame(bShouldCull);
    Actor->SetActorEnableCollision(!bShouldCull);
    Actor->SetActorTickEnabled(!bShouldCull);
}

void UPerf_RenderOptimizer::UpdateRenderStats()
{
    if (!GetWorld())
    {
        return;
    }
    
    // Reset stats
    RenderStats.VisibleActors = 0;
    RenderStats.CulledActors = 0;
    RenderStats.LOD0Actors = 0;
    RenderStats.LOD1Actors = 0;
    RenderStats.LOD2Actors = 0;
    RenderStats.AverageDrawDistance = 0.0f;
    
    float TotalDistance = 0.0f;
    int32 TotalActors = 0;
    
    // Iterate through all actors in the world
    for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor || Actor->IsA<APawn>())
        {
            continue;
        }
        
        float Distance = GetDistanceToPlayer(Actor);
        TotalDistance += Distance;
        TotalActors++;
        
        // Check visibility
        if (Actor->IsHidden())
        {
            RenderStats.CulledActors++;
        }
        else
        {
            RenderStats.VisibleActors++;
            
            // Determine LOD level
            int32 LODLevel = GetOptimalLODLevel(Distance);
            switch (LODLevel)
            {
                case 0:
                    RenderStats.LOD0Actors++;
                    break;
                case 1:
                    RenderStats.LOD1Actors++;
                    break;
                case 2:
                    RenderStats.LOD2Actors++;
                    break;
            }
        }
    }
    
    if (TotalActors > 0)
    {
        RenderStats.AverageDrawDistance = TotalDistance / TotalActors;
    }
    
    RenderStats.RenderThreadTime = LastFrameRate > 0.0f ? (1000.0f / LastFrameRate) : 0.0f;
}

void UPerf_RenderOptimizer::AutoAdjustLODSettings(float CurrentFrameRate)
{
    if (CurrentFrameRate < MinFrameRate)
    {
        // Performance is poor, increase culling distances
        LODSettings.LOD0Distance *= 0.9f;
        LODSettings.LOD1Distance *= 0.9f;
        LODSettings.LOD2Distance *= 0.9f;
        LODSettings.CullDistance *= 0.8f;
        
        UE_LOG(LogTemp, Warning, TEXT("Performance poor (%.1f fps), reducing LOD distances"), CurrentFrameRate);
    }
    else if (CurrentFrameRate > TargetFrameRate * 1.1f)
    {
        // Performance is good, we can increase quality
        LODSettings.LOD0Distance *= 1.05f;
        LODSettings.LOD1Distance *= 1.05f;
        LODSettings.LOD2Distance *= 1.05f;
        LODSettings.CullDistance *= 1.1f;
        
        // Clamp to reasonable maximums
        LODSettings.LOD0Distance = FMath::Min(LODSettings.LOD0Distance, 2000.0f);
        LODSettings.LOD1Distance = FMath::Min(LODSettings.LOD1Distance, 5000.0f);
        LODSettings.LOD2Distance = FMath::Min(LODSettings.LOD2Distance, 10000.0f);
        LODSettings.CullDistance = FMath::Min(LODSettings.CullDistance, 20000.0f);
    }
}

float UPerf_RenderOptimizer::GetDistanceToPlayer(AActor* Actor) const
{
    if (!Actor || !GetWorld())
    {
        return 0.0f;
    }
    
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        return 0.0f;
    }
    
    return FVector::Dist(Actor->GetActorLocation(), PlayerPawn->GetActorLocation());
}

int32 UPerf_RenderOptimizer::GetOptimalLODLevel(float Distance) const
{
    if (Distance <= LODSettings.LOD0Distance)
    {
        return 0;
    }
    else if (Distance <= LODSettings.LOD1Distance)
    {
        return 1;
    }
    else if (Distance <= LODSettings.LOD2Distance)
    {
        return 2;
    }
    else
    {
        return 3; // Maximum LOD or cull
    }
}

void UPerf_RenderOptimizer::ApplyOptimizationToAllActors()
{
    if (!GetWorld())
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot apply optimization - no world context"));
        return;
    }
    
    int32 OptimizedCount = 0;
    
    for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && !Actor->IsA<APawn>())
        {
            OptimizeRenderingForActor(Actor);
            OptimizedCount++;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Applied render optimization to %d actors"), OptimizedCount);
    UpdateRenderStats();
}

void UPerf_RenderOptimizer::OptimizeStaticMesh(UStaticMeshComponent* MeshComp, float Distance)
{
    if (!MeshComp || !MeshComp->GetStaticMesh())
    {
        return;
    }
    
    int32 LODLevel = GetOptimalLODLevel(Distance);
    
    if (LODLevel >= 3 && LODSettings.bEnableDistanceCulling)
    {
        // Cull the component
        MeshComp->SetVisibility(false);
        MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
    else
    {
        // Set appropriate LOD
        MeshComp->SetVisibility(true);
        MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        MeshComp->SetForcedLodModel(LODLevel + 1);
    }
}

void UPerf_RenderOptimizer::OptimizeSkeletalMesh(USkeletalMeshComponent* MeshComp, float Distance)
{
    if (!MeshComp || !MeshComp->GetSkeletalMeshAsset())
    {
        return;
    }
    
    int32 LODLevel = GetOptimalLODLevel(Distance);
    
    if (LODLevel >= 3 && LODSettings.bEnableDistanceCulling)
    {
        // Cull the component
        MeshComp->SetVisibility(false);
        MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
    else
    {
        // Set appropriate LOD
        MeshComp->SetVisibility(true);
        MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        MeshComp->SetForcedLOD(LODLevel + 1);
    }
}

void UPerf_RenderOptimizer::UpdateActorVisibility(AActor* Actor, float Distance)
{
    if (!Actor)
    {
        return;
    }
    
    bool bShouldCull = Distance > LODSettings.CullDistance && LODSettings.bEnableDistanceCulling;
    CullActorByDistance(Actor, bShouldCull);
}