#include "Perf_CullingOptimizer.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Components/PrimitiveComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"

UPerf_CullingOptimizer::UPerf_CullingOptimizer()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;
    
    CullingSettings = FPerf_CullingSettings();
    CullingMode = EPerf_CullingMode::Hybrid;
    UpdateFrequency = 0.1f;
    
    TotalObjects = 0;
    VisibleObjects = 0;
    CulledObjects = 0;
    CullingEfficiency = 0.0f;
    TimeSinceLastUpdate = 0.0f;
}

void UPerf_CullingOptimizer::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize actor tracking
    GatherActorsToTrack();
    
    UE_LOG(LogTemp, Warning, TEXT("Perf_CullingOptimizer: Initialized with %d tracked actors"), TrackedActors.Num());
}

void UPerf_CullingOptimizer::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    TimeSinceLastUpdate += DeltaTime;
    
    if (TimeSinceLastUpdate >= UpdateFrequency)
    {
        UpdateCulling();
        TimeSinceLastUpdate = 0.0f;
    }
}

void UPerf_CullingOptimizer::UpdateCulling()
{
    if (!GetWorld())
    {
        return;
    }
    
    // Clean up invalid actor references
    TrackedActors.RemoveAll([](const TWeakObjectPtr<AActor>& ActorPtr) {
        return !ActorPtr.IsValid();
    });
    
    // Regather actors if we have too few
    if (TrackedActors.Num() < 10)
    {
        GatherActorsToTrack();
    }
    
    TotalObjects = TrackedActors.Num();
    VisibleObjects = 0;
    CulledObjects = 0;
    
    switch (CullingMode)
    {
        case EPerf_CullingMode::Distance:
            PerformDistanceCulling();
            break;
            
        case EPerf_CullingMode::Frustum:
            PerformFrustumCulling();
            break;
            
        case EPerf_CullingMode::Occlusion:
            PerformOcclusionCulling();
            break;
            
        case EPerf_CullingMode::Hybrid:
            PerformDistanceCulling();
            PerformFrustumCulling();
            if (CullingSettings.bEnableOcclusionCulling)
            {
                PerformOcclusionCulling();
            }
            break;
    }
    
    UpdateStatistics();
}

void UPerf_CullingOptimizer::GatherActorsToTrack()
{
    TrackedActors.Empty();
    
    if (!GetWorld())
    {
        return;
    }
    
    // Find all static mesh actors and skeletal mesh actors
    for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor || Actor->IsPendingKill())
        {
            continue;
        }
        
        // Check if actor has renderable components
        bool bHasRenderableComponent = false;
        
        TArray<UActorComponent*> Components = Actor->GetRootComponent() ? Actor->GetRootComponent()->GetAttachChildren() : TArray<UActorComponent*>();
        Components.Add(Actor->GetRootComponent());
        
        for (UActorComponent* Component : Components)
        {
            if (UStaticMeshComponent* StaticMeshComp = Cast<UStaticMeshComponent>(Component))
            {
                if (StaticMeshComp->GetStaticMesh())
                {
                    bHasRenderableComponent = true;
                    break;
                }
            }
            else if (USkeletalMeshComponent* SkeletalMeshComp = Cast<USkeletalMeshComponent>(Component))
            {
                if (SkeletalMeshComp->GetSkeletalMeshAsset())
                {
                    bHasRenderableComponent = true;
                    break;
                }
            }
        }
        
        if (bHasRenderableComponent)
        {
            TrackedActors.Add(Actor);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Perf_CullingOptimizer: Gathered %d actors to track"), TrackedActors.Num());
}

void UPerf_CullingOptimizer::PerformDistanceCulling()
{
    for (const TWeakObjectPtr<AActor>& ActorPtr : TrackedActors)
    {
        if (!ActorPtr.IsValid())
        {
            continue;
        }
        
        AActor* Actor = ActorPtr.Get();
        float Distance = GetDistanceToPlayer(Actor);
        
        bool bShouldBeVisible = Distance <= CullingSettings.MaxDrawDistance;
        
        if (bShouldBeVisible)
        {
            VisibleObjects++;
        }
        else
        {
            CulledObjects++;
            SetActorVisibility(Actor, false);
        }
    }
}

void UPerf_CullingOptimizer::PerformFrustumCulling()
{
    if (!CullingSettings.bEnableFrustumCulling)
    {
        return;
    }
    
    for (const TWeakObjectPtr<AActor>& ActorPtr : TrackedActors)
    {
        if (!ActorPtr.IsValid())
        {
            continue;
        }
        
        AActor* Actor = ActorPtr.Get();
        
        if (!IsActorInViewFrustum(Actor))
        {
            SetActorVisibility(Actor, false);
            CulledObjects++;
        }
        else
        {
            VisibleObjects++;
        }
    }
}

void UPerf_CullingOptimizer::PerformOcclusionCulling()
{
    if (!CullingSettings.bEnableOcclusionCulling)
    {
        return;
    }
    
    // Basic occlusion culling implementation
    // In a full implementation, this would use hardware occlusion queries
    for (const TWeakObjectPtr<AActor>& ActorPtr : TrackedActors)
    {
        if (!ActorPtr.IsValid())
        {
            continue;
        }
        
        AActor* Actor = ActorPtr.Get();
        
        // Simple line trace based occlusion test
        APlayerController* PC = GetWorld()->GetFirstPlayerController();
        if (PC && PC->GetPawn())
        {
            FVector PlayerLocation = PC->GetPawn()->GetActorLocation();
            FVector ActorLocation = Actor->GetActorLocation();
            
            FHitResult HitResult;
            FCollisionQueryParams QueryParams;
            QueryParams.AddIgnoredActor(PC->GetPawn());
            QueryParams.AddIgnoredActor(Actor);
            
            bool bHit = GetWorld()->LineTraceSingleByChannel(
                HitResult,
                PlayerLocation,
                ActorLocation,
                ECollisionChannel::ECC_Visibility,
                QueryParams
            );
            
            if (bHit)
            {
                // Actor is occluded
                SetActorVisibility(Actor, false);
                CulledObjects++;
            }
            else
            {
                VisibleObjects++;
            }
        }
    }
}

void UPerf_CullingOptimizer::UpdateStatistics()
{
    if (TotalObjects > 0)
    {
        CullingEfficiency = (float)CulledObjects / (float)TotalObjects * 100.0f;
    }
    else
    {
        CullingEfficiency = 0.0f;
    }
    
    // Log statistics periodically
    static float LogTimer = 0.0f;
    LogTimer += UpdateFrequency;
    
    if (LogTimer >= 5.0f) // Log every 5 seconds
    {
        UE_LOG(LogTemp, Log, TEXT("Culling Stats - Total: %d, Visible: %d, Culled: %d, Efficiency: %.1f%%"), 
               TotalObjects, VisibleObjects, CulledObjects, CullingEfficiency);
        LogTimer = 0.0f;
    }
}

bool UPerf_CullingOptimizer::IsActorInViewFrustum(AActor* Actor) const
{
    if (!Actor || !GetWorld())
    {
        return false;
    }
    
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC || !PC->GetPawn())
    {
        return true; // Assume visible if no player
    }
    
    // Get player camera location and rotation
    FVector CameraLocation;
    FRotator CameraRotation;
    PC->GetPlayerViewPoint(CameraLocation, CameraRotation);
    
    // Simple frustum check using dot product
    FVector ToActor = (Actor->GetActorLocation() - CameraLocation).GetSafeNormal();
    FVector CameraForward = CameraRotation.Vector();
    
    float DotProduct = FVector::DotProduct(ToActor, CameraForward);
    
    // Actor is in frustum if it's in front of the camera (simple check)
    return DotProduct > 0.0f;
}

float UPerf_CullingOptimizer::GetDistanceToPlayer(AActor* Actor) const
{
    if (!Actor || !GetWorld())
    {
        return 0.0f;
    }
    
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC || !PC->GetPawn())
    {
        return 0.0f;
    }
    
    return FVector::Dist(Actor->GetActorLocation(), PC->GetPawn()->GetActorLocation());
}

void UPerf_CullingOptimizer::SetActorVisibility(AActor* Actor, bool bVisible)
{
    if (!Actor)
    {
        return;
    }
    
    // Set visibility on all renderable components
    TArray<UActorComponent*> Components = Actor->GetRootComponent() ? Actor->GetRootComponent()->GetAttachChildren() : TArray<UActorComponent*>();
    Components.Add(Actor->GetRootComponent());
    
    for (UActorComponent* Component : Components)
    {
        if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Component))
        {
            PrimComp->SetVisibility(bVisible);
        }
    }
}

void UPerf_CullingOptimizer::SetCullingMode(EPerf_CullingMode NewMode)
{
    CullingMode = NewMode;
    UE_LOG(LogTemp, Log, TEXT("Perf_CullingOptimizer: Culling mode changed to %d"), (int32)NewMode);
}

void UPerf_CullingOptimizer::SetMaxDrawDistance(float Distance)
{
    CullingSettings.MaxDrawDistance = FMath::Max(Distance, 100.0f);
    UE_LOG(LogTemp, Log, TEXT("Perf_CullingOptimizer: Max draw distance set to %.1f"), CullingSettings.MaxDrawDistance);
}

void UPerf_CullingOptimizer::EnableOcclusionCulling(bool bEnable)
{
    CullingSettings.bEnableOcclusionCulling = bEnable;
    UE_LOG(LogTemp, Log, TEXT("Perf_CullingOptimizer: Occlusion culling %s"), bEnable ? TEXT("enabled") : TEXT("disabled"));
}