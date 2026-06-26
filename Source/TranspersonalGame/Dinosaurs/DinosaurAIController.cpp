// DinosaurAIController.cpp — Core Systems Programmer #03 — Cycle 009
// Prehistoric survival game: AI controller implementation for dinosaur pawns.

#include "DinosaurAIController.h"
#include "DinosaurBase.h"
#include "NavigationSystem.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────────────────

ADinosaurAIController::ADinosaurAIController()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.0f; // We manage our own interval

    SetupPerceptionSystem();
}

// ─────────────────────────────────────────────────────────────────────────────
// Lifecycle
// ─────────────────────────────────────────────────────────────────────────────

void ADinosaurAIController::BeginPlay()
{
    Super::BeginPlay();

    // Bind perception callback
    if (PerceptionComp)
    {
        PerceptionComp->OnTargetPerceptionUpdated.AddDynamic(
            this, &ADinosaurAIController::OnPerceptionUpdated);
    }
}

void ADinosaurAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Throttle AI evaluation to AITickInterval
    AITickTimer += DeltaTime;
    if (AITickTimer < AITickInterval)
    {
        return;
    }
    AITickTimer = 0.f;

    EvaluateAndExecuteState(DeltaTime);
}

void ADinosaurAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    ADinosaurBase* Dino = Cast<ADinosaurBase>(InPawn);
    if (Dino && Dino->IsAlive())
    {
        // Start roaming immediately on possession
        StartRoaming();
    }
}

void ADinosaurAIController::OnUnPossess()
{
    Super::OnUnPossess();
    CurrentThreat = nullptr;
    StopMovement();
}

// ─────────────────────────────────────────────────────────────────────────────
// Navigation
// ─────────────────────────────────────────────────────────────────────────────

void ADinosaurAIController::StartRoaming()
{
    ADinosaurBase* Dino = GetDinoPawn();
    if (!Dino || !Dino->IsAlive())
    {
        return;
    }

    FVector Destination = PickRoamDestination();
    if (Destination == FVector::ZeroVector)
    {
        // No valid nav point found — wait and retry
        bIsWaitingAtDestination = true;
        RoamWaitTimer = RoamWaitTime;
        return;
    }

    CurrentRoamDestination = Destination;
    bIsWaitingAtDestination = false;

    FAIMoveRequest MoveReq;
    MoveReq.SetGoalLocation(Destination);
    MoveReq.SetAcceptanceRadius(AcceptanceRadius);
    MoveReq.SetUsePathfinding(true);

    MoveTo(MoveReq);
}

void ADinosaurAIController::MoveToTarget(AActor* Target)
{
    if (!Target)
    {
        return;
    }

    FAIMoveRequest MoveReq;
    MoveReq.SetGoalActor(Target);
    MoveReq.SetAcceptanceRadius(AcceptanceRadius * 0.5f);
    MoveReq.SetUsePathfinding(true);

    MoveTo(MoveReq);
}

void ADinosaurAIController::FleeFromTarget(AActor* Threat)
{
    if (!Threat)
    {
        return;
    }

    APawn* MyPawn = GetPawn();
    if (!MyPawn)
    {
        return;
    }

    // Move in the direction AWAY from the threat
    FVector ThreatLoc = Threat->GetActorLocation();
    FVector MyLoc = MyPawn->GetActorLocation();
    FVector FleeDir = (MyLoc - ThreatLoc).GetSafeNormal();
    FVector FleeTarget = MyLoc + FleeDir * RoamRadius;

    // Project onto nav mesh
    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
    if (NavSys)
    {
        FNavLocation NavLoc;
        if (NavSys->GetRandomReachablePointInRadius(FleeTarget, 500.f, NavLoc))
        {
            FleeTarget = NavLoc.Location;
        }
    }

    FAIMoveRequest MoveReq;
    MoveReq.SetGoalLocation(FleeTarget);
    MoveReq.SetAcceptanceRadius(AcceptanceRadius);
    MoveReq.SetUsePathfinding(true);

    MoveTo(MoveReq);
}

void ADinosaurAIController::StopMovement()
{
    StopMovement();
}

// ─────────────────────────────────────────────────────────────────────────────
// Perception
// ─────────────────────────────────────────────────────────────────────────────

void ADinosaurAIController::OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (!Actor)
    {
        return;
    }

    ADinosaurBase* Dino = GetDinoPawn();
    if (!Dino || !Dino->IsAlive())
    {
        return;
    }

    // Only react to the player character
    ACharacter* PlayerChar = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (Actor != PlayerChar)
    {
        return;
    }

    if (Stimulus.WasSuccessfullySensed())
    {
        // Player detected — escalate based on aggression
        CurrentThreat = Actor;

        if (Dino->Stats.bIsCarnivore && Dino->Stats.AggressionLevel > 0.3f)
        {
            Dino->SetDinoState(ECore_DinoState::Hunting);
        }
        else
        {
            Dino->SetDinoState(ECore_DinoState::Alert);
        }
    }
    else
    {
        // Lost sight of player — return to roaming
        CurrentThreat = nullptr;
        Dino->SetDinoState(ECore_DinoState::Idle);
        StartRoaming();
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// State Queries
// ─────────────────────────────────────────────────────────────────────────────

bool ADinosaurAIController::HasReachedDestination() const
{
    APawn* MyPawn = GetPawn();
    if (!MyPawn)
    {
        return false;
    }

    FVector MyLoc = MyPawn->GetActorLocation();
    float DistSq = FVector::DistSquared2D(MyLoc, CurrentRoamDestination);
    return DistSq < (AcceptanceRadius * AcceptanceRadius);
}

bool ADinosaurAIController::IsInAttackRange() const
{
    ADinosaurBase* Dino = GetDinoPawn();
    if (!Dino || !CurrentThreat)
    {
        return false;
    }

    float Dist = FVector::Dist(Dino->GetActorLocation(), CurrentThreat->GetActorLocation());
    return Dist <= Dino->Stats.AttackRange;
}

// ─────────────────────────────────────────────────────────────────────────────
// Internal Helpers
// ─────────────────────────────────────────────────────────────────────────────

ADinosaurBase* ADinosaurAIController::GetDinoPawn() const
{
    return Cast<ADinosaurBase>(GetPawn());
}

void ADinosaurAIController::EvaluateAndExecuteState(float DeltaTime)
{
    ADinosaurBase* Dino = GetDinoPawn();
    if (!Dino || !Dino->IsAlive())
    {
        return;
    }

    ECore_DinoState State = Dino->GetDinoState();

    switch (State)
    {
    case ECore_DinoState::Idle:
        // After waiting, start roaming
        if (bIsWaitingAtDestination)
        {
            RoamWaitTimer -= AITickInterval;
            if (RoamWaitTimer <= 0.f)
            {
                bIsWaitingAtDestination = false;
                Dino->SetDinoState(ECore_DinoState::Roaming);
                StartRoaming();
            }
        }
        else
        {
            Dino->SetDinoState(ECore_DinoState::Roaming);
            StartRoaming();
        }
        break;

    case ECore_DinoState::Roaming:
        // Check if we've arrived
        if (HasReachedDestination())
        {
            bIsWaitingAtDestination = true;
            RoamWaitTimer = RoamWaitTime;
            Dino->SetDinoState(ECore_DinoState::Idle);
            AAIController::StopMovement();
        }
        break;

    case ECore_DinoState::Alert:
        // Look toward threat, then decide
        if (CurrentThreat)
        {
            // If carnivore and close enough, switch to hunting
            float Dist = FVector::Dist(Dino->GetActorLocation(), CurrentThreat->GetActorLocation());
            if (Dino->Stats.bIsCarnivore && Dist < Dino->Stats.DetectionRange * 0.7f)
            {
                Dino->SetDinoState(ECore_DinoState::Hunting);
            }
            else if (!Dino->Stats.bIsCarnivore)
            {
                // Herbivores flee
                Dino->SetDinoState(ECore_DinoState::Fleeing);
                FleeFromTarget(CurrentThreat);
            }
        }
        else
        {
            Dino->SetDinoState(ECore_DinoState::Idle);
        }
        break;

    case ECore_DinoState::Hunting:
        if (CurrentThreat)
        {
            if (IsInAttackRange())
            {
                Dino->SetDinoState(ECore_DinoState::Attacking);
                Dino->ApplyMeleeDamage(CurrentThreat);
            }
            else
            {
                MoveToTarget(CurrentThreat);
            }
        }
        else
        {
            Dino->SetDinoState(ECore_DinoState::Idle);
        }
        break;

    case ECore_DinoState::Attacking:
        if (CurrentThreat)
        {
            if (!IsInAttackRange())
            {
                // Target moved out of range — chase again
                Dino->SetDinoState(ECore_DinoState::Hunting);
                MoveToTarget(CurrentThreat);
            }
            else
            {
                // Apply damage at cooldown rate
                Dino->ApplyMeleeDamage(CurrentThreat);
            }
        }
        else
        {
            Dino->SetDinoState(ECore_DinoState::Idle);
        }
        break;

    case ECore_DinoState::Fleeing:
        if (CurrentThreat)
        {
            float Dist = FVector::Dist(Dino->GetActorLocation(), CurrentThreat->GetActorLocation());
            if (Dist > Dino->Stats.DetectionRange * 1.5f)
            {
                // Safe distance reached — calm down
                CurrentThreat = nullptr;
                Dino->SetDinoState(ECore_DinoState::Idle);
            }
            else if (HasReachedDestination())
            {
                // Keep fleeing
                FleeFromTarget(CurrentThreat);
            }
        }
        else
        {
            Dino->SetDinoState(ECore_DinoState::Idle);
        }
        break;

    case ECore_DinoState::Dead:
        // Stop all activity
        AAIController::StopMovement();
        break;

    default:
        break;
    }
}

FVector ADinosaurAIController::PickRoamDestination() const
{
    APawn* MyPawn = GetPawn();
    if (!MyPawn)
    {
        return FVector::ZeroVector;
    }

    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
    if (!NavSys)
    {
        return FVector::ZeroVector;
    }

    FNavLocation NavLoc;
    bool bFound = NavSys->GetRandomReachablePointInRadius(
        MyPawn->GetActorLocation(), RoamRadius, NavLoc);

    return bFound ? NavLoc.Location : FVector::ZeroVector;
}

void ADinosaurAIController::SetupPerceptionSystem()
{
    // Create perception component
    PerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComp"));

    // Sight configuration
    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = 2000.f;
    SightConfig->LoseSightRadius = 2500.f;
    SightConfig->PeripheralVisionAngleDegrees = 90.f;
    SightConfig->SetMaxAge(5.f);
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = false;

    // Hearing configuration
    HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    HearingConfig->HearingRange = 1500.f;
    HearingConfig->SetMaxAge(3.f);
    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
    HearingConfig->DetectionByAffiliation.bDetectFriendlies = false;

    PerceptionComp->ConfigureSense(*SightConfig);
    PerceptionComp->ConfigureSense(*HearingConfig);
    PerceptionComp->SetDominantSense(SightConfig->GetSenseImplementation());
}
