#include "NPC_TRexBehaviorController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISightConfig.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

// Blackboard Keys
const FName ANPC_TRexBehaviorController::TargetActorKey = TEXT("TargetActor");
const FName ANPC_TRexBehaviorController::PatrolLocationKey = TEXT("PatrolLocation");
const FName ANPC_TRexBehaviorController::StateKey = TEXT("State");
const FName ANPC_TRexBehaviorController::HomeLocationKey = TEXT("HomeLocation");

ANPC_TRexBehaviorController::ANPC_TRexBehaviorController()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create AI Perception Component
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
    
    // Create Sight Configuration
    SightConfig = CreateDefaultSubobject<UAISightConfig>(TEXT("SightConfig"));
    if (SightConfig)
    {
        SightConfig->SightRadius = 3000.0f;
        SightConfig->LoseSightRadius = 3500.0f;
        SightConfig->PeripheralVisionAngleDegrees = 120.0f;
        SightConfig->SetMaxAge(5.0f);
        SightConfig->AutoSuccessRangeFromLastSeenLocation = 500.0f;
        SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
        SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
        SightConfig->DetectionByAffiliation.bDetectEnemies = true;

        if (AIPerceptionComponent)
        {
            AIPerceptionComponent->SetDominantSense(*SightConfig->GetSenseImplementation());
            AIPerceptionComponent->ConfigureSense(*SightConfig);
        }
    }

    // Create Behavior Tree Component
    BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
    
    // Create Blackboard Component
    BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));

    // Initialize state
    CurrentState = ENPC_TRexState::Idle;
    CurrentTarget = nullptr;
    StateTimer = 0.0f;
    LastAttackTime = 0.0f;

    // Initialize behavior data with default values
    BehaviorData = FNPC_TRexBehaviorData();
}

void ANPC_TRexBehaviorController::BeginPlay()
{
    Super::BeginPlay();

    // Set home location
    if (GetPawn())
    {
        HomeLocation = GetPawn()->GetActorLocation();
        CurrentPatrolTarget = HomeLocation;
    }

    // Bind perception events
    if (AIPerceptionComponent)
    {
        AIPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ANPC_TRexBehaviorController::OnPerceptionUpdated);
    }

    // Initialize blackboard
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsVector(HomeLocationKey, HomeLocation);
        BlackboardComponent->SetValueAsEnum(StateKey, static_cast<uint8>(CurrentState));
    }

    // Start initial behavior
    SetState(ENPC_TRexState::Patrolling);
}

void ANPC_TRexBehaviorController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    StateTimer += DeltaTime;
    UpdateBehavior(DeltaTime);
}

void ANPC_TRexBehaviorController::SetState(ENPC_TRexState NewState)
{
    if (CurrentState != NewState)
    {
        CurrentState = NewState;
        StateTimer = 0.0f;

        // Update blackboard
        if (BlackboardComponent)
        {
            BlackboardComponent->SetValueAsEnum(StateKey, static_cast<uint8>(CurrentState));
        }

        // Handle state transitions
        switch (NewState)
        {
            case ENPC_TRexState::Patrolling:
                StartPatrol();
                break;
            case ENPC_TRexState::Hunting:
                if (CurrentTarget)
                {
                    StartHunting(CurrentTarget);
                }
                break;
            case ENPC_TRexState::Attacking:
                StartAttack();
                break;
            default:
                break;
        }
    }
}

void ANPC_TRexBehaviorController::StartPatrol()
{
    SetMovementSpeed(BehaviorData.MovementSpeed);
    CurrentPatrolTarget = GetRandomPatrolPoint();
    
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsVector(PatrolLocationKey, CurrentPatrolTarget);
    }

    // Move to patrol point
    MoveToLocation(CurrentPatrolTarget, 100.0f);
}

void ANPC_TRexBehaviorController::StartHunting(AActor* Target)
{
    if (!Target) return;

    CurrentTarget = Target;
    SetMovementSpeed(BehaviorData.HuntingSpeed);

    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsObject(TargetActorKey, Target);
    }

    // Move to target
    MoveToActor(Target, BehaviorData.AttackRange);
}

void ANPC_TRexBehaviorController::StartAttack()
{
    if (!CurrentTarget) return;

    // Stop movement during attack
    StopMovement();
    
    // Record attack time
    LastAttackTime = GetWorld()->GetTimeSeconds();

    // Face target
    if (GetPawn())
    {
        FVector Direction = CurrentTarget->GetActorLocation() - GetPawn()->GetActorLocation();
        Direction.Z = 0.0f;
        Direction.Normalize();
        
        FRotator TargetRotation = Direction.Rotation();
        GetPawn()->SetActorRotation(TargetRotation);
    }
}

bool ANPC_TRexBehaviorController::IsInAttackRange(AActor* Target) const
{
    if (!Target || !GetPawn()) return false;
    
    float Distance = GetDistanceToTarget(Target);
    return Distance <= BehaviorData.AttackRange;
}

bool ANPC_TRexBehaviorController::IsInDetectionRange(AActor* Target) const
{
    if (!Target || !GetPawn()) return false;
    
    float Distance = GetDistanceToTarget(Target);
    return Distance <= BehaviorData.DetectionRange;
}

FVector ANPC_TRexBehaviorController::GetRandomPatrolPoint() const
{
    // Generate random point within patrol radius
    FVector RandomDirection = FMath::VRand();
    RandomDirection.Z = 0.0f; // Keep on ground level
    RandomDirection.Normalize();
    
    float RandomDistance = FMath::RandRange(500.0f, BehaviorData.PatrolRadius);
    FVector PatrolPoint = HomeLocation + (RandomDirection * RandomDistance);
    
    return PatrolPoint;
}

void ANPC_TRexBehaviorController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        if (!Actor) continue;

        // Check if it's a player character
        ACharacter* Character = Cast<ACharacter>(Actor);
        if (Character && Character->IsPlayerControlled())
        {
            float Distance = GetDistanceToTarget(Actor);
            
            // If player is within detection range and we're not already hunting
            if (Distance <= BehaviorData.DetectionRange && CurrentState != ENPC_TRexState::Hunting && CurrentState != ENPC_TRexState::Attacking)
            {
                CurrentTarget = Actor;
                SetState(ENPC_TRexState::Hunting);
                break;
            }
        }
    }
}

void ANPC_TRexBehaviorController::UpdateBehavior(float DeltaTime)
{
    switch (CurrentState)
    {
        case ENPC_TRexState::Patrolling:
            UpdatePatrol(DeltaTime);
            break;
        case ENPC_TRexState::Hunting:
            UpdateHunting(DeltaTime);
            break;
        case ENPC_TRexState::Attacking:
            UpdateAttacking(DeltaTime);
            break;
        case ENPC_TRexState::Feeding:
            UpdateFeeding(DeltaTime);
            break;
        default:
            break;
    }
}

void ANPC_TRexBehaviorController::UpdatePatrol(float DeltaTime)
{
    if (!GetPawn()) return;

    // Check if we've reached the patrol point
    float DistanceToPatrol = FVector::Dist(GetPawn()->GetActorLocation(), CurrentPatrolTarget);
    if (DistanceToPatrol < 200.0f)
    {
        // Wait at patrol point for a moment, then get new patrol point
        if (StateTimer > 3.0f)
        {
            CurrentPatrolTarget = GetRandomPatrolPoint();
            MoveToLocation(CurrentPatrolTarget, 100.0f);
            StateTimer = 0.0f;
        }
    }
}

void ANPC_TRexBehaviorController::UpdateHunting(float DeltaTime)
{
    if (!CurrentTarget)
    {
        SetState(ENPC_TRexState::Patrolling);
        return;
    }

    float DistanceToTarget = GetDistanceToTarget(CurrentTarget);

    // If target is in attack range, start attacking
    if (DistanceToTarget <= BehaviorData.AttackRange)
    {
        SetState(ENPC_TRexState::Attacking);
    }
    // If target is too far, give up hunt and return to patrol
    else if (DistanceToTarget > BehaviorData.DetectionRange * 1.5f)
    {
        CurrentTarget = nullptr;
        SetState(ENPC_TRexState::Patrolling);
    }
    else
    {
        // Continue hunting - move to target
        MoveToActor(CurrentTarget, BehaviorData.AttackRange);
    }
}

void ANPC_TRexBehaviorController::UpdateAttacking(float DeltaTime)
{
    if (!CurrentTarget)
    {
        SetState(ENPC_TRexState::Patrolling);
        return;
    }

    float DistanceToTarget = GetDistanceToTarget(CurrentTarget);

    // If target moved out of attack range, resume hunting
    if (DistanceToTarget > BehaviorData.AttackRange * 1.2f)
    {
        SetState(ENPC_TRexState::Hunting);
    }
    // Attack duration - after 2 seconds, return to hunting
    else if (StateTimer > 2.0f)
    {
        SetState(ENPC_TRexState::Hunting);
    }
}

void ANPC_TRexBehaviorController::UpdateFeeding(float DeltaTime)
{
    // Feeding behavior - stay in place for a while
    if (StateTimer > 10.0f)
    {
        SetState(ENPC_TRexState::Patrolling);
    }
}

float ANPC_TRexBehaviorController::GetDistanceToTarget(AActor* Target) const
{
    if (!Target || !GetPawn()) return FLT_MAX;
    
    return FVector::Dist(GetPawn()->GetActorLocation(), Target->GetActorLocation());
}

bool ANPC_TRexBehaviorController::CanSeeTarget(AActor* Target) const
{
    if (!Target || !GetPawn() || !AIPerceptionComponent) return false;
    
    FActorPerceptionBlueprintInfo Info;
    return AIPerceptionComponent->GetActorsPerception(Target, Info) && Info.LastSensedStimuli.Num() > 0;
}

void ANPC_TRexBehaviorController::SetMovementSpeed(float Speed)
{
    if (GetPawn())
    {
        ACharacter* Character = Cast<ACharacter>(GetPawn());
        if (Character && Character->GetCharacterMovement())
        {
            Character->GetCharacterMovement()->MaxWalkSpeed = Speed;
        }
    }
}