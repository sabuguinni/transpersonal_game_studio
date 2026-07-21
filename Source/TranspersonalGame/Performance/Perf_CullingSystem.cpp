#include "Perf_CullingSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/StaticMeshActor.h"
#include "Animation/SkeletalMeshActor.h"

UPerf_CullingSystem::UPerf_CullingSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Check every 100ms
    
    // Default culling settings
    MaxCullingDistance = 50000.0f; // 500 meters
    LODDistanceMultiplier = 1.0f;
    bEnableFrustumCulling = true;
    bEnableDistanceCulling = true;
    bEnableOcclusionCulling = true;
    
    CulledActors.Reserve(1000);
}

void UPerf_CullingSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Get player camera manager for frustum culling
    if (UWorld* World = GetWorld())
    {
        if (APlayerCameraManager* CameraManager = World->GetFirstPlayerController()->PlayerCameraManager)
        {
            PlayerCameraManager = CameraManager;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Performance Culling System initialized - MaxDistance: %.1f"), MaxCullingDistance);
}

void UPerf_CullingSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!PlayerCameraManager)
    {
        return;
    }
    
    PerformCullingPass();
}

void UPerf_CullingSystem::PerformCullingPass()
{
    if (!GetWorld())
    {
        return;
    }
    
    FVector CameraLocation = PlayerCameraManager->GetCameraLocation();
    FRotator CameraRotation = PlayerCameraManager->GetCameraRotation();
    
    int32 CulledCount = 0;
    int32 VisibleCount = 0;
    
    // Iterate through all actors in the world
    for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor || Actor->IsPendingKill())
        {
            continue;
        }
        
        // Skip certain actor types
        if (ShouldSkipActor(Actor))
        {
            continue;
        }
        
        bool bShouldCull = false;
        
        // Distance culling
        if (bEnableDistanceCulling)
        {
            float Distance = FVector::Dist(CameraLocation, Actor->GetActorLocation());
            if (Distance > MaxCullingDistance)
            {
                bShouldCull = true;
            }
        }
        
        // Frustum culling (simplified)
        if (!bShouldCull && bEnableFrustumCulling)
        {
            if (!IsActorInCameraFrustum(Actor, CameraLocation, CameraRotation))
            {
                bShouldCull = true;
            }
        }
        
        // Apply culling
        if (bShouldCull)
        {
            CullActor(Actor);
            CulledCount++;
        }
        else
        {
            UnCullActor(Actor);
            VisibleCount++;
        }
    }
    
    // Log performance stats occasionally
    static float LogTimer = 0.0f;
    LogTimer += GetWorld()->GetDeltaSeconds();
    if (LogTimer > 5.0f)
    {
        UE_LOG(LogTemp, Log, TEXT("Culling Stats - Visible: %d, Culled: %d"), VisibleCount, CulledCount);
        LogTimer = 0.0f;
    }
}

bool UPerf_CullingSystem::ShouldSkipActor(AActor* Actor) const
{
    if (!Actor)
    {
        return true;
    }
    
    // Skip player, controllers, and essential game objects
    if (Actor->IsA<APawn>() || Actor->IsA<AController>() || Actor->IsA<APlayerStart>())
    {
        return true;
    }
    
    // Skip actors with specific tags
    if (Actor->Tags.Contains(FName("NoCulling")) || Actor->Tags.Contains(FName("Essential")))
    {
        return true;
    }
    
    return false;
}

bool UPerf_CullingSystem::IsActorInCameraFrustum(AActor* Actor, const FVector& CameraLocation, const FRotator& CameraRotation) const
{
    if (!Actor)
    {
        return false;
    }
    
    // Simple dot product check for frustum culling
    FVector ActorDirection = (Actor->GetActorLocation() - CameraLocation).GetSafeNormal();
    FVector CameraForward = CameraRotation.Vector();
    
    float DotProduct = FVector::DotProduct(ActorDirection, CameraForward);
    
    // If dot product is negative, actor is behind camera
    if (DotProduct < 0.0f)
    {
        return false;
    }
    
    // Simple FOV check (assuming 90 degree FOV)
    return DotProduct > 0.5f; // Roughly 60 degree cone
}

void UPerf_CullingSystem::CullActor(AActor* Actor)
{
    if (!Actor || CulledActors.Contains(Actor))
    {
        return;
    }
    
    // Hide all mesh components
    TArray<UMeshComponent*> MeshComponents;
    Actor->GetComponents<UMeshComponent>(MeshComponents);
    
    for (UMeshComponent* MeshComp : MeshComponents)
    {
        if (MeshComp)
        {
            MeshComp->SetVisibility(false);
            MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        }
    }
    
    // Disable tick if possible
    Actor->SetActorTickEnabled(false);
    
    CulledActors.Add(Actor);
}

void UPerf_CullingSystem::UnCullActor(AActor* Actor)
{
    if (!Actor || !CulledActors.Contains(Actor))
    {
        return;
    }
    
    // Show all mesh components
    TArray<UMeshComponent*> MeshComponents;
    Actor->GetComponents<UMeshComponent>(MeshComponents);
    
    for (UMeshComponent* MeshComp : MeshComponents)
    {
        if (MeshComp)
        {
            MeshComp->SetVisibility(true);
            MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        }
    }
    
    // Re-enable tick
    Actor->SetActorTickEnabled(true);
    
    CulledActors.Remove(Actor);
}

void UPerf_CullingSystem::SetCullingDistance(float NewDistance)
{
    MaxCullingDistance = FMath::Clamp(NewDistance, 1000.0f, 100000.0f);
    UE_LOG(LogTemp, Warning, TEXT("Culling distance set to: %.1f"), MaxCullingDistance);
}

int32 UPerf_CullingSystem::GetCulledActorCount() const
{
    return CulledActors.Num();
}

void UPerf_CullingSystem::ForceRefreshCulling()
{
    // Un-cull all actors first
    for (AActor* Actor : CulledActors)
    {
        if (Actor)
        {
            UnCullActor(Actor);
        }
    }
    
    CulledActors.Empty();
    
    // Perform immediate culling pass
    PerformCullingPass();
}