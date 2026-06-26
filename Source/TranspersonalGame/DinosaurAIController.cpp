// DinosaurAIController.cpp — AI Controller implementation for dinosaur pawns
// Agent #3 — Core Systems Programmer

#include "DinosaurAIController.h"
#include "DinosaurBase.h"
#include "NavigationSystem.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────────────────

ADinosaurAIController::ADinosaurAIController()
{
    PrimaryActorTick.bCanEverTick = true;

    // ── Perception component ──────────────────────────────────────────────────
    PerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComp"));
    SetPerceptionComponent(*PerceptionComp);

    // Sight sense
    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = SightRadius;
    SightConfig->LoseSightRadius = SightRadius * 1.25f;
    SightConfig->PeripheralVisionAngleDegrees = SightAngle;
    SightConfig->SetMaxAge(5.0f);
    SightConfig->DetectionByAffiliation.bDetectEnemies    = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals   = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
    PerceptionComp->ConfigureSense(*SightConfig);

    // Hearing sense
    HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    HearingConfig->HearingRange = HearingRange;
    HearingConfig->SetMaxAge(3.0f);
    HearingConfig->DetectionByAffiliation.bDetectEnemies    = true;
    HearingConfig->DetectionByAffiliation.bDetectNeutrals   = true;
    HearingConfig->DetectionByAffiliation.bDetectFriendlies = false;
    PerceptionComp->ConfigureSense(*HearingConfig);

    PerceptionComp->SetDominantSense(SightConfig->GetSenseImplementation());

    // Default state
    CurrentBehaviorState = ECore_DinosaurBehaviorState::Idle;
}

// ─────────────────────────────────────────────────────────────────────────────
// Possess / UnPossess
// ─────────────────────────────────────────────────────────────────────────────

void ADinosaurAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    ControlledDino = Cast<ADinosaurBase>(InPawn);
    if (!ControlledDino)
    {
        UE_LOG(LogTemp, Warning, TEXT("DinosaurAIController: Possessed non-DinosaurBase pawn '%s'"),
            *InPawn->GetName());
        return;
    }

    // Record home location for patrol radius
    HomeLocation = ControlledDino->GetActorLocation();

    // Bind perception delegate
    PerceptionComp->OnTargetPerceptionUpdated.AddDynamic(
        this, &ADinosaurAIController::OnTargetPerceptionUpdated);

    // Start Behavior Tree if assigned, otherwise use code-driven tick
    if (BehaviorTreeAsset)
    {
        RunBehaviorTree(BehaviorTreeAsset);
        UE_LOG(LogTemp, Log, TEXT("DinosaurAIController: BT started for '%s'"),
            *ControlledDino->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("DinosaurAIController: No BT — using code-driven tick for '%s'"),
            *ControlledDino->GetName());
        SetBehaviorState(ECore_DinosaurBehaviorState::Patrolling);
    }
}

void ADinosaurAIController::OnUnPossess()
{
    PerceptionComp->OnTargetPerceptionUpdated.RemoveDynamic(
        this, &ADinosaurAIController::OnTargetPerceptionUpdated);

    ControlledDino = nullptr;
    ThreatTarget   = nullptr;

    Super::OnUnPossess();
}

// ─────────────────────────────────────────────────────────────────────────────
// Tick — code-driven state machine (used when no BT asset is assigned)
// ─────────────────────────────────────────────────────────────────────────────

void ADinosaurAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // BT handles logic if running
    if (BehaviorTreeAsset) return;
    if (!ControlledDino)   return;

    switch (CurrentBehaviorState)
    {
        case ECore_DinosaurBehaviorState::Idle:       TickIdle(DeltaTime);       break;
        case ECore_DinosaurBehaviorState::Patrolling: TickPatrolling(DeltaTime); break;
        case ECore_DinosaurBehaviorState::Alerted:    TickAlerted(DeltaTime);    break;
        case ECore_DinosaurBehaviorState::Hunting:    TickHunting(DeltaTime);    break;
        case ECore_DinosaurBehaviorState::Fleeing:    TickFleeing(DeltaTime);    break;
        case ECore_DinosaurBehaviorState::Resting:    TickResting(DeltaTime);    break;
        case ECore_DinosaurBehaviorState::Dead:       /* no-op */                break;
        default:                                                                  break;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Perception callback
// ─────────────────────────────────────────────────────────────────────────────

void ADinosaurAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (!Actor || !ControlledDino) return;

    // Ignore other dinosaurs of the same species (no intra-species aggression for now)
    if (Actor->IsA(ADinosaurBase::StaticClass())) return;

    if (Stimulus.WasSuccessfullySensed())
    {
        // Threat detected
        ThreatTarget = Actor;

        // Aggression decision: carnivores hunt, herbivores flee
        if (ControlledDino->DinoSpecies == ECore_DinosaurSpecies::TRex ||
            ControlledDino->DinoSpecies == ECore_DinosaurSpecies::Velociraptor)
        {
            SetBehaviorState(ECore_DinosaurBehaviorState::Hunting);
        }
        else
        {
            SetBehaviorState(ECore_DinosaurBehaviorState::Fleeing);
        }
    }
    else
    {
        // Lost sight/hearing — return to patrol
        ThreatTarget = nullptr;
        SetBehaviorState(ECore_DinosaurBehaviorState::Patrolling);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Navigation helpers
// ─────────────────────────────────────────────────────────────────────────────

FVector ADinosaurAIController::GetRandomPatrolPoint() const
{
    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
    if (!NavSys) return HomeLocation;

    FNavLocation NavLoc;
    const bool bFound = NavSys->GetRandomReachablePointInRadius(HomeLocation, PatrolRadius, NavLoc);
    return bFound ? NavLoc.Location : HomeLocation;
}

void ADinosaurAIController::MoveToNextPatrolPoint()
{
    const FVector Target = GetRandomPatrolPoint();
    MoveToLocation(Target, 100.0f, true, true, false, true);
}

void ADinosaurAIController::ChaseTarget(AActor* Target)
{
    if (!Target) return;
    MoveToActor(Target, 200.0f, true, true, false);
}

void ADinosaurAIController::FleeFromTarget(AActor* Target)
{
    if (!Target || !ControlledDino) return;

    // Move in the opposite direction of the threat
    const FVector ToThreat = (Target->GetActorLocation() - ControlledDino->GetActorLocation()).GetSafeNormal();
    const FVector FleeDir  = -ToThreat;
    const FVector FleeDest = ControlledDino->GetActorLocation() + FleeDir * PatrolRadius;

    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
    if (NavSys)
    {
        FNavLocation NavLoc;
        if (NavSys->GetRandomReachablePointInRadius(FleeDest, 500.0f, NavLoc))
        {
            MoveToLocation(NavLoc.Location, 100.0f, true, true, false, true);
            return;
        }
    }
    MoveToLocation(FleeDest, 100.0f, true, true, false, true);
}

// ─────────────────────────────────────────────────────────────────────────────
// State machine
// ─────────────────────────────────────────────────────────────────────────────

void ADinosaurAIController::SetBehaviorState(ECore_DinosaurBehaviorState NewState)
{
    if (CurrentBehaviorState == NewState) return;

    CurrentBehaviorState = NewState;

    // Propagate to the pawn so it can update movement speed, animations, etc.
    if (ControlledDino)
    {
        ControlledDino->OnBehaviorStateChanged(NewState);
    }

    UE_LOG(LogTemp, Verbose, TEXT("DinosaurAIController: '%s' → state %d"),
        ControlledDino ? *ControlledDino->GetName() : TEXT("?"),
        static_cast<int32>(NewState));
}

// ─── Per-state tick helpers ───────────────────────────────────────────────────

void ADinosaurAIController::TickIdle(float DeltaTime)
{
    // After a short delay, transition to patrolling
    PatrolWaitTimer += DeltaTime;
    if (PatrolWaitTimer >= PatrolWaitTime)
    {
        PatrolWaitTimer = 0.0f;
        SetBehaviorState(ECore_DinosaurBehaviorState::Patrolling);
    }
}

void ADinosaurAIController::TickPatrolling(float DeltaTime)
{
    // If we've stopped moving, pick a new patrol point
    if (GetMoveStatus() == EPathFollowingStatus::Idle)
    {
        if (bWaitingAtPatrolPoint)
        {
            PatrolWaitTimer += DeltaTime;
            if (PatrolWaitTimer >= PatrolWaitTime)
            {
                PatrolWaitTimer       = 0.0f;
                bWaitingAtPatrolPoint = false;
                MoveToNextPatrolPoint();
            }
        }
        else
        {
            bWaitingAtPatrolPoint = true;
            PatrolWaitTimer       = 0.0f;
        }
    }
}

void ADinosaurAIController::TickAlerted(float DeltaTime)
{
    // Look around for a few seconds then decide
    PatrolWaitTimer += DeltaTime;
    if (PatrolWaitTimer >= 3.0f)
    {
        PatrolWaitTimer = 0.0f;
        if (ThreatTarget)
        {
            if (ControlledDino->DinoSpecies == ECore_DinosaurSpecies::TRex ||
                ControlledDino->DinoSpecies == ECore_DinosaurSpecies::Velociraptor)
            {
                SetBehaviorState(ECore_DinosaurBehaviorState::Hunting);
            }
            else
            {
                SetBehaviorState(ECore_DinosaurBehaviorState::Fleeing);
            }
        }
        else
        {
            SetBehaviorState(ECore_DinosaurBehaviorState::Patrolling);
        }
    }
}

void ADinosaurAIController::TickHunting(float DeltaTime)
{
    if (!ThreatTarget)
    {
        SetBehaviorState(ECore_DinosaurBehaviorState::Patrolling);
        return;
    }

    // If target is dead or too far, give up
    const float DistSq = FVector::DistSquared(
        ControlledDino->GetActorLocation(), ThreatTarget->GetActorLocation());
    const float GiveUpDist = SightRadius * 2.0f;

    if (DistSq > GiveUpDist * GiveUpDist)
    {
        ThreatTarget = nullptr;
        SetBehaviorState(ECore_DinosaurBehaviorState::Patrolling);
        return;
    }

    ChaseTarget(ThreatTarget);
}

void ADinosaurAIController::TickFleeing(float DeltaTime)
{
    if (!ThreatTarget)
    {
        SetBehaviorState(ECore_DinosaurBehaviorState::Patrolling);
        return;
    }

    // If we've put enough distance, calm down
    const float DistSq = FVector::DistSquared(
        ControlledDino->GetActorLocation(), ThreatTarget->GetActorLocation());
    const float SafeDist = SightRadius * 1.5f;

    if (DistSq > SafeDist * SafeDist)
    {
        ThreatTarget = nullptr;
        SetBehaviorState(ECore_DinosaurBehaviorState::Resting);
        return;
    }

    FleeFromTarget(ThreatTarget);
}

void ADinosaurAIController::TickResting(float DeltaTime)
{
    // Rest for PatrolWaitTime * 2 then resume patrol
    PatrolWaitTimer += DeltaTime;
    if (PatrolWaitTimer >= PatrolWaitTime * 2.0f)
    {
        PatrolWaitTimer = 0.0f;
        SetBehaviorState(ECore_DinosaurBehaviorState::Patrolling);
    }
}
