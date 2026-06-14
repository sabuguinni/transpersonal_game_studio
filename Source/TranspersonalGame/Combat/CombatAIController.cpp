#include "CombatAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardData.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "GameFramework/Pawn.h"

ACombatAIController::ACombatAIController()
{
    PrimaryActorTick.bCanEverTick = true;

    // Initialize components
    BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
    BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));

    // Set blackboard component
    SetBlackboardComponent(BlackboardComponent);

    // Initialize combat state
    CurrentCombatState = ECombat_AIState::Patrol;
    CurrentTarget = nullptr;
    AttackRange = 200.0f;
    ChaseRange = 1000.0f;
    SightRange = 1500.0f;
    HearingRange = 800.0f;
    AttackCooldown = 2.0f;
    LastAttackTime = 0.0f;

    // Configure sight perception
    UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    if (SightConfig)
    {
        SightConfig->SightRadius = SightRange;
        SightConfig->LoseSightRadius = SightRange * 1.2f;
        SightConfig->PeripheralVisionAngleDegrees = 90.0f;
        SightConfig->SetMaxAge(5.0f);
        SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
        SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
        SightConfig->DetectionByAffiliation.bDetectEnemies = true;
        AIPerceptionComponent->ConfigureSense(*SightConfig);
    }

    // Configure hearing perception
    UAISenseConfig_Hearing* HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    if (HearingConfig)
    {
        HearingConfig->HearingRange = HearingRange;
        HearingConfig->SetMaxAge(3.0f);
        HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
        HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
        HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
        AIPerceptionComponent->ConfigureSense(*HearingConfig);
    }

    // Set dominant sense
    AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());

    // Bind perception events
    AIPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ACombatAIController::OnPerceptionUpdated);
    AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ACombatAIController::OnTargetPerceptionUpdated);
}

void ACombatAIController::BeginPlay()
{
    Super::BeginPlay();

    // Start behavior tree if available
    if (BehaviorTree && BlackboardAsset)
    {
        if (BlackboardComponent)
        {
            UseBlackboard(BlackboardAsset);
        }
        RunBehaviorTree(BehaviorTree);
    }
}

void ACombatAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    UpdateCombatDecisions();
}

void ACombatAIController::StartCombatBehavior(AActor* Target)
{
    if (!Target) return;

    CurrentTarget = Target;
    SetCombatState(ECombat_AIState::Chase);

    // Update blackboard
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsObject(TEXT("TargetActor"), Target);
        BlackboardComponent->SetValueAsVector(TEXT("TargetLocation"), Target->GetActorLocation());
    }

    UE_LOG(LogTemp, Warning, TEXT("CombatAI: Starting combat with target %s"), *Target->GetName());
}

void ACombatAIController::StopCombatBehavior()
{
    CurrentTarget = nullptr;
    SetCombatState(ECombat_AIState::Patrol);

    // Clear blackboard
    if (BlackboardComponent)
    {
        BlackboardComponent->ClearValue(TEXT("TargetActor"));
        BlackboardComponent->ClearValue(TEXT("TargetLocation"));
    }

    UE_LOG(LogTemp, Warning, TEXT("CombatAI: Stopping combat behavior"));
}

void ACombatAIController::SetCombatState(ECombat_AIState NewState)
{
    if (CurrentCombatState != NewState)
    {
        CurrentCombatState = NewState;

        // Update blackboard
        if (BlackboardComponent)
        {
            BlackboardComponent->SetValueAsEnum(TEXT("CombatState"), static_cast<uint8>(NewState));
        }

        UE_LOG(LogTemp, Log, TEXT("CombatAI: State changed to %d"), static_cast<int32>(NewState));
    }
}

void ACombatAIController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        if (Actor && Actor->IsA<ACharacter>())
        {
            // Check if this is a potential target (player character)
            if (Actor->GetClass()->GetName().Contains(TEXT("TranspersonalCharacter")))
            {
                if (!CurrentTarget && CanSeeTarget(Actor))
                {
                    StartCombatBehavior(Actor);
                }
            }
        }
    }
}

void ACombatAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (!Actor) return;

    if (Stimulus.WasSuccessfullySensed())
    {
        // Target detected
        if (Actor->IsA<ACharacter>() && !CurrentTarget)
        {
            StartCombatBehavior(Actor);
        }
    }
    else
    {
        // Target lost
        if (Actor == CurrentTarget)
        {
            StopCombatBehavior();
        }
    }
}

void ACombatAIController::UpdateCombatDecisions()
{
    switch (CurrentCombatState)
    {
        case ECombat_AIState::Patrol:
            HandlePatrolState();
            break;
        case ECombat_AIState::Chase:
            HandleChaseState();
            break;
        case ECombat_AIState::Attack:
            HandleAttackState();
            break;
        case ECombat_AIState::Flee:
            HandleFleeState();
            break;
    }
}

void ACombatAIController::HandlePatrolState()
{
    // Look for targets in perception range
    if (AIPerceptionComponent)
    {
        TArray<AActor*> PerceivedActors;
        AIPerceptionComponent->GetCurrentlyPerceivedActors(nullptr, PerceivedActors);

        for (AActor* Actor : PerceivedActors)
        {
            if (Actor && Actor->IsA<ACharacter>())
            {
                if (CanSeeTarget(Actor))
                {
                    StartCombatBehavior(Actor);
                    return;
                }
            }
        }
    }
}

void ACombatAIController::HandleChaseState()
{
    if (!CurrentTarget)
    {
        SetCombatState(ECombat_AIState::Patrol);
        return;
    }

    float DistanceToTarget = GetDistanceToTarget(CurrentTarget);

    if (DistanceToTarget <= AttackRange)
    {
        SetCombatState(ECombat_AIState::Attack);
    }
    else if (DistanceToTarget > ChaseRange)
    {
        StopCombatBehavior();
    }
    else
    {
        // Continue chasing - move toward target
        if (BlackboardComponent)
        {
            BlackboardComponent->SetValueAsVector(TEXT("TargetLocation"), CurrentTarget->GetActorLocation());
        }
    }
}

void ACombatAIController::HandleAttackState()
{
    if (!CurrentTarget)
    {
        SetCombatState(ECombat_AIState::Patrol);
        return;
    }

    float DistanceToTarget = GetDistanceToTarget(CurrentTarget);

    if (DistanceToTarget > AttackRange)
    {
        SetCombatState(ECombat_AIState::Chase);
        return;
    }

    // Check attack cooldown
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastAttackTime >= AttackCooldown)
    {
        // Perform attack
        LastAttackTime = CurrentTime;
        
        // Update blackboard for attack action
        if (BlackboardComponent)
        {
            BlackboardComponent->SetValueAsBool(TEXT("ShouldAttack"), true);
        }

        UE_LOG(LogTemp, Warning, TEXT("CombatAI: Attacking target %s"), *CurrentTarget->GetName());
    }
}

void ACombatAIController::HandleFleeState()
{
    if (!CurrentTarget)
    {
        SetCombatState(ECombat_AIState::Patrol);
        return;
    }

    float DistanceToTarget = GetDistanceToTarget(CurrentTarget);

    if (DistanceToTarget > ChaseRange * 2.0f)
    {
        SetCombatState(ECombat_AIState::Patrol);
    }
    else
    {
        // Continue fleeing - move away from target
        if (BlackboardComponent && GetPawn())
        {
            FVector FleeDirection = GetPawn()->GetActorLocation() - CurrentTarget->GetActorLocation();
            FleeDirection.Normalize();
            FVector FleeLocation = GetPawn()->GetActorLocation() + FleeDirection * 1000.0f;
            BlackboardComponent->SetValueAsVector(TEXT("FleeLocation"), FleeLocation);
        }
    }
}

bool ACombatAIController::CanSeeTarget(AActor* Target) const
{
    if (!Target || !GetPawn()) return false;

    FVector Start = GetPawn()->GetActorLocation();
    FVector End = Target->GetActorLocation();
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetPawn());
    QueryParams.AddIgnoredActor(Target);

    bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, QueryParams);
    return !bHit;
}

bool ACombatAIController::IsInAttackRange(AActor* Target) const
{
    return GetDistanceToTarget(Target) <= AttackRange;
}

float ACombatAIController::GetDistanceToTarget(AActor* Target) const
{
    if (!Target || !GetPawn()) return FLT_MAX;
    return FVector::Dist(GetPawn()->GetActorLocation(), Target->GetActorLocation());
}