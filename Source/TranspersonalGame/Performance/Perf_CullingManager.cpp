#include "Perf_CullingManager.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Engine/Engine.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"

UPerf_CullingManager::UPerf_CullingManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;
    
    CullingSettings.MaxDrawDistance = 10000.0f;
    CullingSettings.FrustumCullingMargin = 100.0f;
    CullingSettings.bEnableOcclusionCulling = true;
    CullingSettings.MaxVisibleActors = 5000;
    CullingSettings.PerformanceCullingThreshold = 30.0f;
    
    bEnableCulling = true;
    CullingUpdateInterval = 0.1f;
}

void UPerf_CullingManager::BeginPlay()
{
    Super::BeginPlay();
    
    LastUpdateTime = GetWorld()->GetTimeSeconds();
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
            TEXT("Performance Culling Manager Initialized"));
    }
}

void UPerf_CullingManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bEnableCulling)
    {
        return;
    }
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastUpdateTime >= CullingUpdateInterval)
    {
        UpdateCulling();
        LastUpdateTime = CurrentTime;
    }
}

void UPerf_CullingManager::UpdateCulling()
{
    if (!GetWorld())
    {
        return;
    }
    
    float StartTime = FPlatformTime::Seconds();
    
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
    
    TotalActors = AllActors.Num();
    CulledActorsList.Empty();
    VisibleActorsList.Empty();
    
    int32 ProcessedActors = 0;
    
    for (AActor* Actor : AllActors)
    {
        if (!Actor || Actor == GetOwner())
        {
            continue;
        }
        
        bool bShouldCull = false;
        EPerf_CullingType CullingReason = EPerf_CullingType::Distance;
        
        // Distance culling
        if (ShouldCullByDistance(Actor))
        {
            bShouldCull = true;
            CullingReason = EPerf_CullingType::Distance;
        }
        // Frustum culling
        else if (ShouldCullByFrustum(Actor))
        {
            bShouldCull = true;
            CullingReason = EPerf_CullingType::Frustum;
        }
        // Performance culling
        else if (ShouldCullByPerformance(Actor))
        {
            bShouldCull = true;
            CullingReason = EPerf_CullingType::Performance;
        }
        
        if (bShouldCull)
        {
            CullActor(Actor, CullingReason);
        }
        else
        {
            UnCullActor(Actor);
        }
        
        ProcessedActors++;
        
        // Limit processing time per frame
        if (ProcessedActors % 100 == 0)
        {
            float ElapsedTime = FPlatformTime::Seconds() - StartTime;
            if (ElapsedTime > 0.005f) // 5ms limit
            {
                break;
            }
        }
    }
    
    CulledActors = CulledActorsList.Num();
    VisibleActors = VisibleActorsList.Num();
    LastCullingTime = FPlatformTime::Seconds() - StartTime;
}

void UPerf_CullingManager::SetCullingEnabled(bool bEnabled)
{
    bEnableCulling = bEnabled;
    
    if (!bEnabled)
    {
        // Restore all culled actors
        for (AActor* Actor : CulledActorsList)
        {
            UnCullActor(Actor);
        }
    }
}

bool UPerf_CullingManager::IsActorCulled(AActor* Actor) const
{
    return CulledActorsList.Contains(Actor);
}

void UPerf_CullingManager::CullActor(AActor* Actor, EPerf_CullingType CullingType)
{
    if (!Actor || IsActorCulled(Actor))
    {
        return;
    }
    
    UpdateActorVisibility(Actor, false);
    CulledActorsList.AddUnique(Actor);
    VisibleActorsList.Remove(Actor);
}

void UPerf_CullingManager::UnCullActor(AActor* Actor)
{
    if (!Actor || !IsActorCulled(Actor))
    {
        return;
    }
    
    UpdateActorVisibility(Actor, true);
    CulledActorsList.Remove(Actor);
    VisibleActorsList.AddUnique(Actor);
}

TArray<AActor*> UPerf_CullingManager::GetCulledActors() const
{
    return CulledActorsList;
}

TArray<AActor*> UPerf_CullingManager::GetVisibleActors() const
{
    return VisibleActorsList;
}

float UPerf_CullingManager::GetCullingEfficiency() const
{
    if (TotalActors == 0)
    {
        return 0.0f;
    }
    
    return (float)CulledActors / (float)TotalActors * 100.0f;
}

bool UPerf_CullingManager::ShouldCullByDistance(AActor* Actor) const
{
    FVector PlayerLocation = GetPlayerLocation();
    FVector ActorLocation = Actor->GetActorLocation();
    
    float Distance = FVector::Dist(PlayerLocation, ActorLocation);
    return Distance > CullingSettings.MaxDrawDistance;
}

bool UPerf_CullingManager::ShouldCullByFrustum(AActor* Actor) const
{
    // Simplified frustum culling - would need camera frustum in full implementation
    FVector PlayerLocation = GetPlayerLocation();
    FVector ActorLocation = Actor->GetActorLocation();
    
    // Basic behind-player check
    if (GetWorld() && GetWorld()->GetFirstPlayerController())
    {
        APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
        if (PlayerPawn)
        {
            FVector ForwardVector = PlayerPawn->GetActorForwardVector();
            FVector ToActor = (ActorLocation - PlayerLocation).GetSafeNormal();
            
            float DotProduct = FVector::DotProduct(ForwardVector, ToActor);
            return DotProduct < -0.5f; // Behind player
        }
    }
    
    return false;
}

bool UPerf_CullingManager::ShouldCullByOcclusion(AActor* Actor) const
{
    // Simplified occlusion check - would need proper occlusion queries in full implementation
    return false;
}

bool UPerf_CullingManager::ShouldCullByPerformance(AActor* Actor) const
{
    // Cull based on current FPS
    if (GEngine)
    {
        float CurrentFPS = 1.0f / GEngine->GetDeltaSeconds();
        if (CurrentFPS < CullingSettings.PerformanceCullingThreshold)
        {
            // Cull actors that are far and not essential
            FVector PlayerLocation = GetPlayerLocation();
            FVector ActorLocation = Actor->GetActorLocation();
            float Distance = FVector::Dist(PlayerLocation, ActorLocation);
            
            return Distance > CullingSettings.MaxDrawDistance * 0.5f;
        }
    }
    
    return false;
}

void UPerf_CullingManager::UpdateActorVisibility(AActor* Actor, bool bVisible)
{
    if (!Actor)
    {
        return;
    }
    
    // Hide/show static mesh components
    TArray<UStaticMeshComponent*> StaticMeshComponents;
    Actor->GetComponents<UStaticMeshComponent>(StaticMeshComponents);
    for (UStaticMeshComponent* MeshComp : StaticMeshComponents)
    {
        if (MeshComp)
        {
            MeshComp->SetVisibility(bVisible);
        }
    }
    
    // Hide/show skeletal mesh components
    TArray<USkeletalMeshComponent*> SkeletalMeshComponents;
    Actor->GetComponents<USkeletalMeshComponent>(SkeletalMeshComponents);
    for (USkeletalMeshComponent* MeshComp : SkeletalMeshComponents)
    {
        if (MeshComp)
        {
            MeshComp->SetVisibility(bVisible);
        }
    }
    
    // Disable/enable tick for performance
    Actor->SetActorTickEnabled(bVisible);
}

FVector UPerf_CullingManager::GetPlayerLocation() const
{
    if (GetWorld() && GetWorld()->GetFirstPlayerController())
    {
        APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
        if (PlayerPawn)
        {
            return PlayerPawn->GetActorLocation();
        }
    }
    
    return FVector::ZeroVector;
}