#include "CrowdHerdManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

UCrowdHerdManager::UCrowdHerdManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = CrowdTickInterval;
}

void UCrowdHerdManager::BeginPlay()
{
    Super::BeginPlay();
    PrimaryComponentTick.TickInterval = CrowdTickInterval;
}

void UCrowdHerdManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    TimeSinceLastTick += DeltaTime;
    if (TimeSinceLastTick >= CrowdTickInterval)
    {
        TimeSinceLastTick = 0.0f;
        UpdateHerdBehaviors();
        UpdateHerdPositions(DeltaTime);
    }
}

void UCrowdHerdManager::RegisterHerd(const FCrowd_HerdGroup& HerdGroup)
{
    // Remove existing herd with same ID if present
    ActiveHerds.RemoveAll([&HerdGroup](const FCrowd_HerdGroup& H)
    {
        return H.HerdID == HerdGroup.HerdID;
    });
    ActiveHerds.Add(HerdGroup);
}

void UCrowdHerdManager::TriggerFleeResponse(FVector ThreatLocation, float ThreatRadius)
{
    for (FCrowd_HerdGroup& Herd : ActiveHerds)
    {
        float DistToHerd = FVector::Dist(Herd.CenterLocation, ThreatLocation);
        if (DistToHerd <= ThreatRadius)
        {
            Herd.CurrentBehavior = ECrowd_HerdBehavior::Fleeing;

            // Move each member away from threat
            for (FCrowd_HerdMember& Member : Herd.Members)
            {
                if (Member.MeshActor)
                {
                    FVector FleeDir = GetFleeDirection(Member.MeshActor->GetActorLocation(), ThreatLocation);
                    FVector NewLocation = Member.MeshActor->GetActorLocation() + FleeDir * 600.0f;
                    Member.MeshActor->SetActorLocation(NewLocation, true);
                }
            }
        }
    }
}

void UCrowdHerdManager::SetHerdBehavior(const FString& HerdID, ECrowd_HerdBehavior NewBehavior)
{
    for (FCrowd_HerdGroup& Herd : ActiveHerds)
    {
        if (Herd.HerdID == HerdID)
        {
            Herd.CurrentBehavior = NewBehavior;
            return;
        }
    }
}

ECrowd_HerdBehavior UCrowdHerdManager::GetHerdBehavior(const FString& HerdID) const
{
    for (const FCrowd_HerdGroup& Herd : ActiveHerds)
    {
        if (Herd.HerdID == HerdID)
        {
            return Herd.CurrentBehavior;
        }
    }
    return ECrowd_HerdBehavior::Grazing;
}

void UCrowdHerdManager::UpdateHerdBehaviors()
{
    for (FCrowd_HerdGroup& Herd : ActiveHerds)
    {
        // Fleeing herds gradually calm down and return to grazing
        if (Herd.CurrentBehavior == ECrowd_HerdBehavior::Fleeing)
        {
            // After flee, transition to alerting then grazing
            Herd.CurrentBehavior = ECrowd_HerdBehavior::Alerting;
        }
        else if (Herd.CurrentBehavior == ECrowd_HerdBehavior::Alerting)
        {
            Herd.CurrentBehavior = ECrowd_HerdBehavior::Grazing;
        }

        // Update herd center location
        if (Herd.Members.Num() > 0)
        {
            FVector Center = FVector::ZeroVector;
            int32 ValidCount = 0;
            for (const FCrowd_HerdMember& Member : Herd.Members)
            {
                if (Member.MeshActor)
                {
                    Center += Member.MeshActor->GetActorLocation();
                    ValidCount++;
                }
            }
            if (ValidCount > 0)
            {
                Herd.CenterLocation = Center / ValidCount;
            }
        }
    }
}

void UCrowdHerdManager::UpdateHerdPositions(float DeltaTime)
{
    // Grazing: gentle wander around home location
    for (FCrowd_HerdGroup& Herd : ActiveHerds)
    {
        if (Herd.CurrentBehavior != ECrowd_HerdBehavior::Grazing)
        {
            continue;
        }

        for (FCrowd_HerdMember& Member : Herd.Members)
        {
            if (!Member.MeshActor)
            {
                continue;
            }

            FVector CurrentLoc = Member.MeshActor->GetActorLocation();
            float DistFromHome = FVector::Dist2D(CurrentLoc, Member.HomeLocation);

            // If too far from home, nudge back
            if (DistFromHome > Member.WanderRadius)
            {
                FVector ToHome = (Member.HomeLocation - CurrentLoc).GetSafeNormal2D();
                FVector NewLoc = CurrentLoc + ToHome * 50.0f;
                Member.MeshActor->SetActorLocation(NewLoc, true);
            }
        }
    }
}

FVector UCrowdHerdManager::GetFleeDirection(FVector MemberLocation, FVector ThreatLocation) const
{
    FVector AwayFromThreat = (MemberLocation - ThreatLocation).GetSafeNormal2D();
    // Add slight random offset to prevent all members fleeing in exact same direction
    float RandomAngle = FMath::RandRange(-30.0f, 30.0f);
    FQuat RandomRot = FQuat(FVector::UpVector, FMath::DegreesToRadians(RandomAngle));
    return RandomRot.RotateVector(AwayFromThreat);
}
