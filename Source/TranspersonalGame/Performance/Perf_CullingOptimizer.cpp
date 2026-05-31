#include "Perf_CullingOptimizer.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "TimerManager.h"

UPerf_CullingOptimizer::UPerf_CullingOptimizer()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;

    bIsCullingActive = false;
    CulledActorCount = 0;
    VisibleActorCount = 0;
    TimeSinceLastUpdate = 0.0f;
}

void UPerf_CullingOptimizer::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("CullingOptimizer: Component started"));
    
    // Automatically start culling
    StartCulling();
}

void UPerf_CullingOptimizer::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    StopCulling();
    Super::EndPlay(EndPlayReason);
}

void UPerf_CullingOptimizer::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bIsCullingActive)
    {
        TimeSinceLastUpdate += DeltaTime;
        
        if (TimeSinceLastUpdate >= CullingSettings.CullingUpdateFrequency)
        {
            UpdateCulling();
            TimeSinceLastUpdate = 0.0f;
        }
    }
}

void UPerf_CullingOptimizer::StartCulling()
{
    if (bIsCullingActive)
    {
        return;
    }

    bIsCullingActive = true;
    
    // Collect all actors in the world to manage
    if (UWorld* World = GetWorld())
    {
        ManagedActors.Empty();
        
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor && Actor->GetRootComponent())
            {
                // Only manage actors with mesh components
                if (Actor->FindComponentByClass<UStaticMeshComponent>() || 
                    Actor->FindComponentByClass<USkeletalMeshComponent>())
                {
                    ManagedActors.Add(Actor);
                }
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("CullingOptimizer: Started managing %d actors"), ManagedActors.Num());
}

void UPerf_CullingOptimizer::StopCulling()
{
    if (!bIsCullingActive)
    {
        return;
    }

    bIsCullingActive = false;
    
    // Restore visibility to all managed actors
    for (TWeakObjectPtr<AActor> ActorPtr : ManagedActors)
    {
        if (AActor* Actor = ActorPtr.Get())
        {
            Actor->SetActorHiddenInGame(false);
        }
    }

    ManagedActors.Empty();
    CulledActorCount = 0;
    VisibleActorCount = 0;

    UE_LOG(LogTemp, Log, TEXT("CullingOptimizer: Stopped culling"));
}

void UPerf_CullingOptimizer::UpdateCulling()
{
    if (!bIsCullingActive || !GetWorld())
    {
        return;
    }

    // Get player view location and direction
    FVector ViewLocation = FVector::ZeroVector;
    FVector ViewDirection = FVector::ForwardVector;
    
    if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
    {
        if (APawn* PlayerPawn = PC->GetPawn())
        {
            ViewLocation = PlayerPawn->GetActorLocation();
            ViewDirection = PlayerPawn->GetActorForwardVector();
        }
    }

    int32 NewCulledCount = 0;
    int32 NewVisibleCount = 0;

    // Update visibility for each managed actor
    for (TWeakObjectPtr<AActor> ActorPtr : ManagedActors)
    {
        if (AActor* Actor = ActorPtr.Get())
        {
            bool bShouldCull = ShouldCullActor(Actor, ViewLocation, ViewDirection);
            
            Actor->SetActorHiddenInGame(bShouldCull);
            
            if (bShouldCull)
            {
                NewCulledCount++;
            }
            else
            {
                NewVisibleCount++;
            }
        }
    }

    CulledActorCount = NewCulledCount;
    VisibleActorCount = NewVisibleCount;
}

bool UPerf_CullingOptimizer::ShouldCullActor(AActor* Actor, const FVector& ViewLocation, const FVector& ViewDirection) const
{
    if (!Actor)
    {
        return true;
    }

    // Distance culling
    if (CullingSettings.bEnableDistanceCulling)
    {
        float Distance = GetDistanceToActor(Actor, ViewLocation);
        if (Distance > CullingSettings.MaxVisibilityDistance)
        {
            return true;
        }
    }

    // Frustum culling
    if (CullingSettings.bEnableFrustumCulling)
    {
        if (!IsActorInFrustum(Actor, ViewLocation, ViewDirection))
        {
            return true;
        }
    }

    return false;
}

bool UPerf_CullingOptimizer::IsActorInFrustum(AActor* Actor, const FVector& ViewLocation, const FVector& ViewDirection) const
{
    if (!Actor)
    {
        return false;
    }

    FVector ActorLocation = Actor->GetActorLocation();
    FVector ToActor = (ActorLocation - ViewLocation).GetSafeNormal();
    
    // Simple dot product check for frustum culling
    float DotProduct = FVector::DotProduct(ViewDirection, ToActor);
    
    // Consider actor visible if it's within a reasonable angle (roughly 120 degrees)
    return DotProduct > -0.5f;
}

float UPerf_CullingOptimizer::GetDistanceToActor(AActor* Actor, const FVector& ViewLocation) const
{
    if (!Actor)
    {
        return MAX_FLT;
    }

    return FVector::Dist(ViewLocation, Actor->GetActorLocation());
}

void UPerf_CullingOptimizer::SetCullingSettings(const FPerf_CullingSettings& NewSettings)
{
    CullingSettings = NewSettings;
    UE_LOG(LogTemp, Log, TEXT("CullingOptimizer: Settings updated"));
}