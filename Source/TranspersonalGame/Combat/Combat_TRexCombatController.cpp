#include "Combat_TRexCombatController.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"

ACombat_TRexCombatController::ACombat_TRexCombatController()
{
    PrimaryActorTick.bCanEverTick = true;

    // Initialize AI Perception
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
    
    // Configure sight sense
    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = 3000.0f;
    SightConfig->LoseSightRadius = 3500.0f;
    SightConfig->PeripheralVisionAngleDegrees = 120.0f;
    SightConfig->SetMaxAge(5.0f);
    SightConfig->AutoSuccessRangeFromLastSeenLocation = 500.0f;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;

    AIPerceptionComponent->ConfigureSense(*SightConfig);
    AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());

    // Initialize combat parameters
    AttackRange = 400.0f;
    ChaseRange = 2000.0f;
    PatrolRadius = 1500.0f;
    AttackCooldown = 3.0f;
    LastAttackTime = 0.0f;

    // Initialize state
    CurrentCombatState = ECombat_AIState::Patrolling;
    TargetActor = nullptr;
}

void ACombat_TRexCombatController::BeginPlay()
{
    Super::BeginPlay();
    
    HomeLocation = GetPawn() ? GetPawn()->GetActorLocation() : FVector::ZeroVector;
    InitializeAI();
}

void ACombat_TRexCombatController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
    
    if (InPawn)
    {
        HomeLocation = InPawn->GetActorLocation();
        
        // Start behavior tree if available
        if (BehaviorTree && GetBlackboardComponent())
        {
            RunBehaviorTree(BehaviorTree);
            
            // Set initial blackboard values
            GetBlackboardComponent()->SetValueAsEnum(TEXT("CombatState"), static_cast<uint8>(CurrentCombatState));
            GetBlackboardComponent()->SetValueAsVector(TEXT("HomeLocation"), HomeLocation);
        }
    }
}

void ACombat_TRexCombatController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    UpdateCombatBehavior(DeltaTime);
}

void ACombat_TRexCombatController::InitializeAI()
{
    if (AIPerceptionComponent)
    {
        AIPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ACombat_TRexCombatController::OnPerceptionUpdated);
    }
}

void ACombat_TRexCombatController::UpdateCombatBehavior(float DeltaTime)
{
    if (!GetPawn())
        return;

    switch (CurrentCombatState)
    {
        case ECombat_AIState::Patrolling:
            PatrolBehavior();
            break;
            
        case ECombat_AIState::Chasing:
            if (TargetActor && IsPlayerInRange(ChaseRange))
            {
                MoveTowardsTarget();
                
                // Check if close enough to attack
                if (IsPlayerInRange(AttackRange) && CanAttack())
                {
                    SetCombatState(ECombat_AIState::Attacking);
                }
            }
            else
            {
                // Lost target, return to patrol
                SetCombatState(ECombat_AIState::Patrolling);
                TargetActor = nullptr;
            }
            break;
            
        case ECombat_AIState::Attacking:
            if (TargetActor && IsPlayerInRange(AttackRange))
            {
                ExecuteAttack();
                SetCombatState(ECombat_AIState::Chasing);
            }
            else
            {
                SetCombatState(ECombat_AIState::Chasing);
            }
            break;
    }
}

void ACombat_TRexCombatController::SetCombatState(ECombat_AIState NewState)
{
    if (CurrentCombatState != NewState)
    {
        CurrentCombatState = NewState;
        
        // Update blackboard
        if (GetBlackboardComponent())
        {
            GetBlackboardComponent()->SetValueAsEnum(TEXT("CombatState"), static_cast<uint8>(CurrentCombatState));
        }
    }
}

void ACombat_TRexCombatController::SetTarget(AActor* NewTarget)
{
    TargetActor = NewTarget;
    
    if (GetBlackboardComponent())
    {
        GetBlackboardComponent()->SetValueAsObject(TEXT("TargetActor"), TargetActor);
    }
}

bool ACombat_TRexCombatController::CanAttack() const
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    return (CurrentTime - LastAttackTime) >= AttackCooldown;
}

void ACombat_TRexCombatController::ExecuteAttack()
{
    if (!CanAttack() || !TargetActor)
        return;

    LastAttackTime = GetWorld()->GetTimeSeconds();
    
    // Stop movement for attack
    StopMovement();
    
    // Face the target
    if (GetPawn())
    {
        FVector Direction = (TargetActor->GetActorLocation() - GetPawn()->GetActorLocation()).GetSafeNormal();
        FRotator NewRotation = Direction.Rotation();
        GetPawn()->SetActorRotation(NewRotation);
    }
    
    // TODO: Trigger attack animation and damage
    UE_LOG(LogTemp, Warning, TEXT("T-Rex executing attack on %s"), TargetActor ? *TargetActor->GetName() : TEXT("None"));
}

void ACombat_TRexCombatController::StartChasing(AActor* Target)
{
    SetTarget(Target);
    SetCombatState(ECombat_AIState::Chasing);
}

void ACombat_TRexCombatController::ReturnToPatrol()
{
    SetTarget(nullptr);
    SetCombatState(ECombat_AIState::Patrolling);
}

void ACombat_TRexCombatController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        if (Actor && Actor->IsA<ACharacter>())
        {
            // Found a character (likely the player)
            if (CurrentCombatState == ECombat_AIState::Patrolling)
            {
                StartChasing(Actor);
                UE_LOG(LogTemp, Warning, TEXT("T-Rex detected player: %s"), *Actor->GetName());
            }
            break;
        }
    }
}

bool ACombat_TRexCombatController::IsPlayerInRange(float Range) const
{
    if (!TargetActor || !GetPawn())
        return false;

    float Distance = FVector::Dist(GetPawn()->GetActorLocation(), TargetActor->GetActorLocation());
    return Distance <= Range;
}

void ACombat_TRexCombatController::MoveTowardsTarget()
{
    if (TargetActor)
    {
        MoveToActor(TargetActor, AttackRange * 0.8f);
    }
}

void ACombat_TRexCombatController::PatrolBehavior()
{
    if (!GetPawn())
        return;

    // Simple patrol: move to random points around home location
    FVector CurrentLocation = GetPawn()->GetActorLocation();
    float DistanceFromHome = FVector::Dist(CurrentLocation, HomeLocation);
    
    if (DistanceFromHome > PatrolRadius)
    {
        // Return to home area
        MoveToLocation(HomeLocation);
    }
    else
    {
        // Move to random patrol point
        FVector RandomDirection = FMath::VRand();
        RandomDirection.Z = 0.0f; // Keep on ground level
        RandomDirection.Normalize();
        
        FVector PatrolPoint = HomeLocation + (RandomDirection * FMath::RandRange(200.0f, PatrolRadius));
        MoveToLocation(PatrolPoint);
    }
}