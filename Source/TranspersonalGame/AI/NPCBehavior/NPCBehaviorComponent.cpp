// NPCBehaviorComponent.cpp
// NPC Behavior Agent #11 — Transpersonal Game Studio
// Implements patrol, alert, and flee behavior states for tribal NPCs

#include "NPCBehaviorComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"

UNPCBehaviorComponent::UNPCBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    DetectionRadius = 1500.0f;
    FleeRadius = 800.0f;
    CurrentState = ENPC_BehaviorState::Idle;
    CurrentPatrolIndex = 0;
    WaitTimer = 0.0f;
    bWaiting = false;
}

void UNPCBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();
    // Start patrolling if patrol points are defined
    if (PatrolPoints.Num() > 0)
    {
        SetBehaviorState(ENPC_BehaviorState::Patrol);
    }
}

void UNPCBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    switch (CurrentState)
    {
        case ENPC_BehaviorState::Patrol:
            TickPatrol(DeltaTime);
            break;
        case ENPC_BehaviorState::Alert:
            TickAlert(DeltaTime);
            break;
        case ENPC_BehaviorState::Flee:
            TickFlee(DeltaTime);
            break;
        default:
            break;
    }
}

void UNPCBehaviorComponent::SetBehaviorState(ENPC_BehaviorState NewState)
{
    if (CurrentState == NewState) return;
    CurrentState = NewState;
    WaitTimer = 0.0f;
    bWaiting = false;
}

void UNPCBehaviorComponent::AlertNearbyNPCs(float AlertRadius)
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    UWorld* World = Owner->GetWorld();
    if (!World) return;

    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), NearbyActors);

    for (AActor* Actor : NearbyActors)
    {
        if (Actor == Owner) continue;
        float Distance = FVector::Dist(Owner->GetActorLocation(), Actor->GetActorLocation());
        if (Distance <= AlertRadius)
        {
            UNPCBehaviorComponent* OtherBehavior = Actor->FindComponentByClass<UNPCBehaviorComponent>();
            if (OtherBehavior && OtherBehavior->CurrentState == ENPC_BehaviorState::Idle)
            {
                OtherBehavior->SetBehaviorState(ENPC_BehaviorState::Alert);
            }
        }
    }
}

void UNPCBehaviorComponent::AddPatrolPoint(FVector Location, float WaitTime)
{
    FNPC_PatrolPoint Point;
    Point.Location = Location;
    Point.WaitTime = WaitTime;
    PatrolPoints.Add(Point);
}

void UNPCBehaviorComponent::TickPatrol(float DeltaTime)
{
    if (PatrolPoints.Num() == 0) return;

    AActor* Owner = GetOwner();
    if (!Owner) return;

    if (bWaiting)
    {
        WaitTimer -= DeltaTime;
        if (WaitTimer <= 0.0f)
        {
            bWaiting = false;
            CurrentPatrolIndex = (CurrentPatrolIndex + 1) % PatrolPoints.Num();
        }
        return;
    }

    const FNPC_PatrolPoint& Target = PatrolPoints[CurrentPatrolIndex];
    FVector CurrentLoc = Owner->GetActorLocation();
    FVector TargetLoc = Target.Location;
    float Distance = FVector::Dist2D(CurrentLoc, TargetLoc);

    if (Distance < 100.0f)
    {
        // Reached patrol point — wait
        bWaiting = true;
        WaitTimer = Target.WaitTime;
    }
    else
    {
        // Move toward patrol point
        FVector Direction = (TargetLoc - CurrentLoc).GetSafeNormal2D();
        float MoveSpeed = 200.0f; // cm/s walking speed
        FVector NewLoc = CurrentLoc + Direction * MoveSpeed * DeltaTime;
        Owner->SetActorLocation(NewLoc, true);

        // Face movement direction
        FRotator FaceRot = Direction.Rotation();
        Owner->SetActorRotation(FaceRot);
    }
}

void UNPCBehaviorComponent::TickAlert(float DeltaTime)
{
    // Alert state: NPC looks around for 3 seconds then returns to patrol
    WaitTimer += DeltaTime;
    if (WaitTimer >= 3.0f)
    {
        SetBehaviorState(PatrolPoints.Num() > 0 ? ENPC_BehaviorState::Patrol : ENPC_BehaviorState::Idle);
    }
}

void UNPCBehaviorComponent::TickFlee(float DeltaTime)
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    // Flee away from player
    APlayerController* PC = Owner->GetWorld()->GetFirstPlayerController();
    if (!PC || !PC->GetPawn()) return;

    FVector PlayerLoc = PC->GetPawn()->GetActorLocation();
    FVector OwnerLoc = Owner->GetActorLocation();
    float Distance = FVector::Dist(OwnerLoc, PlayerLoc);

    if (Distance > FleeRadius * 2.0f)
    {
        // Safe distance reached — return to idle
        SetBehaviorState(ENPC_BehaviorState::Idle);
        return;
    }

    // Run away from player
    FVector FleeDir = (OwnerLoc - PlayerLoc).GetSafeNormal2D();
    float FleeSpeed = 400.0f; // cm/s running speed
    FVector NewLoc = OwnerLoc + FleeDir * FleeSpeed * DeltaTime;
    Owner->SetActorLocation(NewLoc, true);
    Owner->SetActorRotation(FleeDir.Rotation());
}
