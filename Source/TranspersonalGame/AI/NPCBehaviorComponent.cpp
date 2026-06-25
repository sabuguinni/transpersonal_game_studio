#include "NPCBehaviorComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "DrawDebugHelpers.h"

UNPCBehaviorComponent::UNPCBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz tick for performance
}

void UNPCBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();
    CurrentState = ENPC_BehaviorState::Idle;
    CurrentPatrolIndex = 0;
    StateTimer = 0.0f;

    // Start patrolling if patrol points are defined
    if (PatrolPoints.Num() > 0)
    {
        SetBehaviorState(ENPC_BehaviorState::Patrolling);
    }
}

void UNPCBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    StateTimer += DeltaTime;

    switch (CurrentState)
    {
    case ENPC_BehaviorState::Patrolling:
        TickPatrol(DeltaTime);
        break;
    case ENPC_BehaviorState::Fleeing:
        TickFlee(DeltaTime);
        break;
    case ENPC_BehaviorState::Investigating:
        TickInvestigate(DeltaTime);
        break;
    case ENPC_BehaviorState::Idle:
        // After 5 seconds idle, resume patrol if points exist
        if (StateTimer > 5.0f && PatrolPoints.Num() > 0)
        {
            SetBehaviorState(ENPC_BehaviorState::Patrolling);
        }
        break;
    default:
        break;
    }
}

void UNPCBehaviorComponent::SetBehaviorState(ENPC_BehaviorState NewState)
{
    if (CurrentState == NewState) return;
    CurrentState = NewState;
    StateTimer = 0.0f;

    // Adjust movement speed based on state
    AActor* Owner = GetOwner();
    if (!Owner) return;

    ACharacter* OwnerChar = Cast<ACharacter>(Owner);
    if (!OwnerChar) return;

    UCharacterMovementComponent* MovComp = OwnerChar->GetCharacterMovement();
    if (!MovComp) return;

    switch (NewState)
    {
    case ENPC_BehaviorState::Patrolling:
        MovComp->MaxWalkSpeed = PatrolSpeed;
        break;
    case ENPC_BehaviorState::Fleeing:
        MovComp->MaxWalkSpeed = FleeSpeed;
        break;
    case ENPC_BehaviorState::Investigating:
        MovComp->MaxWalkSpeed = PatrolSpeed * 0.75f;
        break;
    case ENPC_BehaviorState::Idle:
        MovComp->MaxWalkSpeed = 0.0f;
        break;
    default:
        break;
    }
}

void UNPCBehaviorComponent::AddPatrolPoint(FVector Location, float WaitTime)
{
    FNPC_PatrolPoint Point;
    Point.Location = Location;
    Point.WaitTime = WaitTime;
    PatrolPoints.Add(Point);
}

void UNPCBehaviorComponent::FleeFromThreat(AActor* Threat)
{
    if (!Threat) return;
    ThreatActor = Threat;
    SetBehaviorState(ENPC_BehaviorState::Fleeing);
}

void UNPCBehaviorComponent::InvestigateLocation(FVector Location)
{
    SetBehaviorState(ENPC_BehaviorState::Investigating);
}

void UNPCBehaviorComponent::TickPatrol(float DeltaTime)
{
    if (PatrolPoints.Num() == 0) return;

    AActor* Owner = GetOwner();
    if (!Owner) return;

    const FNPC_PatrolPoint& Target = PatrolPoints[CurrentPatrolIndex];
    FVector CurrentLoc = Owner->GetActorLocation();
    FVector TargetLoc = Target.Location;
    float DistToTarget = FVector::Dist2D(CurrentLoc, TargetLoc);

    if (DistToTarget < 100.0f)
    {
        // Reached waypoint — wait then advance
        if (StateTimer > Target.WaitTime)
        {
            CurrentPatrolIndex = (CurrentPatrolIndex + 1) % PatrolPoints.Num();
            StateTimer = 0.0f;
        }
    }
    else
    {
        // Move toward target
        FVector Direction = (TargetLoc - CurrentLoc).GetSafeNormal2D();
        Owner->AddActorWorldOffset(Direction * PatrolSpeed * DeltaTime, true);

        // Face direction of travel
        FRotator LookAt = Direction.Rotation();
        Owner->SetActorRotation(FMath::RInterpTo(Owner->GetActorRotation(), LookAt, DeltaTime, 5.0f));
    }
}

void UNPCBehaviorComponent::TickFlee(float DeltaTime)
{
    AActor* Owner = GetOwner();
    if (!Owner || !ThreatActor) return;

    FVector OwnerLoc = Owner->GetActorLocation();
    FVector ThreatLoc = ThreatActor->GetActorLocation();
    float DistToThreat = FVector::Dist2D(OwnerLoc, ThreatLoc);

    // Stop fleeing if threat is far enough
    if (DistToThreat > FleeRadius)
    {
        ThreatActor = nullptr;
        SetBehaviorState(ENPC_BehaviorState::Idle);
        return;
    }

    // Move away from threat
    FVector FleeDir = (OwnerLoc - ThreatLoc).GetSafeNormal2D();
    Owner->AddActorWorldOffset(FleeDir * FleeSpeed * DeltaTime, true);

    FRotator LookAt = FleeDir.Rotation();
    Owner->SetActorRotation(FMath::RInterpTo(Owner->GetActorRotation(), LookAt, DeltaTime, 8.0f));
}

void UNPCBehaviorComponent::TickInvestigate(float DeltaTime)
{
    // After 10 seconds investigating, return to patrol
    if (StateTimer > 10.0f)
    {
        if (PatrolPoints.Num() > 0)
        {
            SetBehaviorState(ENPC_BehaviorState::Patrolling);
        }
        else
        {
            SetBehaviorState(ENPC_BehaviorState::Idle);
        }
    }
}
