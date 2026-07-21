// StampedeController.cpp
// Agent #13 — Crowd & Traffic Simulation
// Stampede event controller for prehistoric herd animals

#include "StampedeController.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Actor.h"

UCrowd_StampedeController::UCrowd_StampedeController()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz update for stampede logic

    MaxActiveStampedes = 3;
    StampedeDecayRate = 0.5f;
    bDebugDrawStampedes = false;
}

void UCrowd_StampedeController::BeginPlay()
{
    Super::BeginPlay();
    ActiveStampedes.Empty();
    UE_LOG(LogTemp, Log, TEXT("[StampedeController] Initialized — max stampedes: %d"), MaxActiveStampedes);
}

void UCrowd_StampedeController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    UpdateActiveStampedes(DeltaTime);

    if (bDebugDrawStampedes)
    {
        DebugDrawStampedes();
    }
}

bool UCrowd_StampedeController::TriggerStampede(FVector TriggerLocation, FVector FleeDirection, int32 HerdID, float Radius)
{
    if (ActiveStampedes.Num() >= MaxActiveStampedes)
    {
        UE_LOG(LogTemp, Warning, TEXT("[StampedeController] Max stampedes reached (%d) — cannot trigger new stampede"), MaxActiveStampedes);
        return false;
    }

    FCrowd_StampedeEvent NewStampede;
    NewStampede.TriggerLocation = TriggerLocation;
    NewStampede.FleeDirection = FleeDirection.GetSafeNormal();
    NewStampede.TriggerRadius = Radius;
    NewStampede.AffectedHerdID = HerdID;
    NewStampede.bIsActive = true;
    NewStampede.StampedeDuration = 30.0f;
    NewStampede.StampedeSpeed = 1200.0f;

    ActiveStampedes.Add(NewStampede);

    UE_LOG(LogTemp, Log, TEXT("[StampedeController] Stampede triggered at (%.0f, %.0f, %.0f) for herd %d — radius %.0f"),
        TriggerLocation.X, TriggerLocation.Y, TriggerLocation.Z, HerdID, Radius);

    OnStampedeTriggered.Broadcast(NewStampede);
    return true;
}

void UCrowd_StampedeController::UpdateActiveStampedes(float DeltaTime)
{
    for (int32 i = ActiveStampedes.Num() - 1; i >= 0; --i)
    {
        FCrowd_StampedeEvent& Stampede = ActiveStampedes[i];

        if (!Stampede.bIsActive)
        {
            ActiveStampedes.RemoveAt(i);
            continue;
        }

        Stampede.StampedeDuration -= DeltaTime;

        if (Stampede.StampedeDuration <= 0.0f)
        {
            Stampede.bIsActive = false;
            UE_LOG(LogTemp, Log, TEXT("[StampedeController] Stampede for herd %d expired"), Stampede.AffectedHerdID);
            OnStampedeEnded.Broadcast(Stampede);
            ActiveStampedes.RemoveAt(i);
        }
    }
}

bool UCrowd_StampedeController::IsLocationInStampede(FVector Location, int32& OutHerdID) const
{
    for (const FCrowd_StampedeEvent& Stampede : ActiveStampedes)
    {
        if (!Stampede.bIsActive) continue;

        float DistSq = FVector::DistSquared(Location, Stampede.TriggerLocation);
        if (DistSq <= FMath::Square(Stampede.TriggerRadius))
        {
            OutHerdID = Stampede.AffectedHerdID;
            return true;
        }
    }
    OutHerdID = -1;
    return false;
}

FVector UCrowd_StampedeController::GetStampedeFleeDirection(FVector AgentLocation) const
{
    FVector BestFleeDir = FVector::ZeroVector;
    float ClosestDistSq = MAX_FLT;

    for (const FCrowd_StampedeEvent& Stampede : ActiveStampedes)
    {
        if (!Stampede.bIsActive) continue;

        float DistSq = FVector::DistSquared(AgentLocation, Stampede.TriggerLocation);
        if (DistSq < ClosestDistSq)
        {
            ClosestDistSq = DistSq;
            BestFleeDir = Stampede.FleeDirection;
        }
    }

    return BestFleeDir;
}

int32 UCrowd_StampedeController::GetActiveStampedeCount() const
{
    return ActiveStampedes.Num();
}

void UCrowd_StampedeController::ClearAllStampedes()
{
    for (FCrowd_StampedeEvent& Stampede : ActiveStampedes)
    {
        Stampede.bIsActive = false;
        OnStampedeEnded.Broadcast(Stampede);
    }
    ActiveStampedes.Empty();
    UE_LOG(LogTemp, Log, TEXT("[StampedeController] All stampedes cleared"));
}

void UCrowd_StampedeController::DebugDrawStampedes()
{
    UWorld* World = GetWorld();
    if (!World) return;

    for (const FCrowd_StampedeEvent& Stampede : ActiveStampedes)
    {
        if (!Stampede.bIsActive) continue;

        DrawDebugSphere(World, Stampede.TriggerLocation, Stampede.TriggerRadius, 16,
            FColor::Red, false, 0.15f, 0, 5.0f);

        DrawDebugDirectionalArrow(World,
            Stampede.TriggerLocation,
            Stampede.TriggerLocation + Stampede.FleeDirection * 500.0f,
            100.0f, FColor::Orange, false, 0.15f, 0, 8.0f);
    }
}
