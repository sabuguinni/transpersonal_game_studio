#include "NPC_TRexBehaviorTree.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardData.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Float.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

ANPC_TRexBehaviorTree::ANPC_TRexBehaviorTree()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create behavior tree component
    BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
    
    // Create blackboard component
    BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));

    // Create AI perception component
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));

    // Initialize state
    CurrentState = ENPC_TRexState::Idle;
    CurrentPatrolIndex = 0;
    CurrentTarget = nullptr;
    LastAttackTime = 0.0f;

    // Set default territory center to spawn location
    TerritoryCenter = FVector::ZeroVector;

    // Initialize T-Rex stats
    TRexStats.Health = 1000.0f;
    TRexStats.Hunger = 50.0f;
    TRexStats.Stamina = 100.0f;
    TRexStats.Aggression = 75.0f;
    TRexStats.TerritorialRadius = 5000.0f;
    TRexStats.ChaseRadius = 3000.0f;
    TRexStats.AttackRadius = 300.0f;
}

void ANPC_TRexBehaviorTree::BeginPlay()
{
    Super::BeginPlay();

    // Set territory center to current location
    if (GetPawn())
    {
        TerritoryCenter = GetPawn()->GetActorLocation();
    }

    // Setup AI perception
    SetupAIPerception();

    // Generate initial patrol points
    GeneratePatrolPoints();

    // Start behavior tree after a short delay
    FTimerHandle StartBTTimer;
    GetWorld()->GetTimerManager().SetTimer(StartBTTimer, this, &ANPC_TRexBehaviorTree::StartBehaviorTree, 1.0f, false);
}

void ANPC_TRexBehaviorTree::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update T-Rex behavior and stats
    UpdateTRexBehavior(DeltaTime);
    UpdateHunger(DeltaTime);
    UpdateStamina(DeltaTime);

    // Update blackboard values
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsFloat(TEXT("Hunger"), TRexStats.Hunger);
        BlackboardComponent->SetValueAsFloat(TEXT("Stamina"), TRexStats.Stamina);
        BlackboardComponent->SetValueAsFloat(TEXT("Health"), TRexStats.Health);
        BlackboardComponent->SetValueAsVector(TEXT("TerritoryCenter"), TerritoryCenter);
        
        if (CurrentTarget)
        {
            BlackboardComponent->SetValueAsObject(TEXT("TargetActor"), CurrentTarget);
            BlackboardComponent->SetValueAsVector(TEXT("TargetLocation"), CurrentTarget->GetActorLocation());
        }
        else
        {
            BlackboardComponent->ClearValue(TEXT("TargetActor"));
        }
    }
}

void ANPC_TRexBehaviorTree::StartBehaviorTree()
{
    if (TRexBehaviorTree && BlackboardComponent)
    {
        // Initialize blackboard
        if (TRexBlackboard)
        {
            BlackboardComponent->InitializeBlackboard(*TRexBlackboard);
        }

        // Set initial blackboard values
        BlackboardComponent->SetValueAsVector(TEXT("TerritoryCenter"), TerritoryCenter);
        BlackboardComponent->SetValueAsFloat(TEXT("TerritorialRadius"), TRexStats.TerritorialRadius);
        BlackboardComponent->SetValueAsFloat(TEXT("ChaseRadius"), TRexStats.ChaseRadius);
        BlackboardComponent->SetValueAsFloat(TEXT("AttackRadius"), TRexStats.AttackRadius);

        // Start the behavior tree
        if (BehaviorTreeComponent)
        {
            BehaviorTreeComponent->StartTree(*TRexBehaviorTree);
            UE_LOG(LogTemp, Warning, TEXT("T-Rex Behavior Tree Started"));
        }
    }
}

void ANPC_TRexBehaviorTree::StopBehaviorTree()
{
    if (BehaviorTreeComponent)
    {
        BehaviorTreeComponent->StopTree();
        UE_LOG(LogTemp, Warning, TEXT("T-Rex Behavior Tree Stopped"));
    }
}

void ANPC_TRexBehaviorTree::SetTRexState(ENPC_TRexState NewState)
{
    if (CurrentState != NewState)
    {
        CurrentState = NewState;
        
        // Update blackboard
        if (BlackboardComponent)
        {
            BlackboardComponent->SetValueAsInt(TEXT("CurrentState"), static_cast<int32>(CurrentState));
        }

        UE_LOG(LogTemp, Log, TEXT("T-Rex State Changed to: %d"), static_cast<int32>(CurrentState));
    }
}

void ANPC_TRexBehaviorTree::SetupPatrolPoints(const TArray<FVector>& Points)
{
    PatrolPoints = Points;
    CurrentPatrolIndex = 0;

    // Update blackboard with first patrol point
    if (PatrolPoints.Num() > 0 && BlackboardComponent)
    {
        BlackboardComponent->SetValueAsVector(TEXT("PatrolTarget"), PatrolPoints[0]);
    }
}

FVector ANPC_TRexBehaviorTree::GetNextPatrolPoint()
{
    if (PatrolPoints.Num() == 0)
    {
        return TerritoryCenter;
    }

    return PatrolPoints[CurrentPatrolIndex];
}

void ANPC_TRexBehaviorTree::AdvancePatrolPoint()
{
    if (PatrolPoints.Num() > 0)
    {
        CurrentPatrolIndex = (CurrentPatrolIndex + 1) % PatrolPoints.Num();
        
        // Update blackboard
        if (BlackboardComponent)
        {
            BlackboardComponent->SetValueAsVector(TEXT("PatrolTarget"), PatrolPoints[CurrentPatrolIndex]);
        }
    }
}

void ANPC_TRexBehaviorTree::SetTarget(APawn* NewTarget)
{
    CurrentTarget = NewTarget;

    if (BlackboardComponent)
    {
        if (CurrentTarget)
        {
            BlackboardComponent->SetValueAsObject(TEXT("TargetActor"), CurrentTarget);
            BlackboardComponent->SetValueAsVector(TEXT("TargetLocation"), CurrentTarget->GetActorLocation());
        }
        else
        {
            BlackboardComponent->ClearValue(TEXT("TargetActor"));
            BlackboardComponent->ClearValue(TEXT("TargetLocation"));
        }
    }
}

float ANPC_TRexBehaviorTree::GetDistanceToTarget() const
{
    if (!CurrentTarget || !GetPawn())
    {
        return -1.0f;
    }

    return FVector::Dist(GetPawn()->GetActorLocation(), CurrentTarget->GetActorLocation());
}

bool ANPC_TRexBehaviorTree::CanAttack() const
{
    if (!CurrentTarget || !GetPawn())
    {
        return false;
    }

    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastAttackTime < AttackCooldown)
    {
        return false;
    }

    float DistanceToTarget = GetDistanceToTarget();
    return DistanceToTarget <= TRexStats.AttackRadius && TRexStats.Stamina > 20.0f;
}

void ANPC_TRexBehaviorTree::PerformAttack()
{
    if (!CanAttack())
    {
        return;
    }

    LastAttackTime = GetWorld()->GetTimeSeconds();
    TRexStats.Stamina = FMath::Max(0.0f, TRexStats.Stamina - 25.0f);

    // Play attack animation or effects here
    UE_LOG(LogTemp, Warning, TEXT("T-Rex performs attack on target!"));

    // Damage target if it's a character
    if (ACharacter* TargetCharacter = Cast<ACharacter>(CurrentTarget))
    {
        // Apply damage logic here
        UE_LOG(LogTemp, Warning, TEXT("T-Rex deals damage to %s"), *TargetCharacter->GetName());
    }

    // Update blackboard
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsFloat(TEXT("LastAttackTime"), LastAttackTime);
    }
}

bool ANPC_TRexBehaviorTree::IsInTerritory(const FVector& Location) const
{
    float Distance = FVector::Dist(Location, TerritoryCenter);
    return Distance <= TRexStats.TerritorialRadius;
}

float ANPC_TRexBehaviorTree::GetDistanceFromTerritoryCenter(const FVector& Location) const
{
    return FVector::Dist(Location, TerritoryCenter);
}

void ANPC_TRexBehaviorTree::UpdateHunger(float DeltaTime)
{
    // Hunger increases over time
    TRexStats.Hunger = FMath::Min(100.0f, TRexStats.Hunger + (DeltaTime * 0.5f));

    // If very hungry, increase aggression
    if (TRexStats.Hunger > 80.0f)
    {
        TRexStats.Aggression = FMath::Min(100.0f, TRexStats.Aggression + (DeltaTime * 2.0f));
    }
}

void ANPC_TRexBehaviorTree::UpdateStamina(float DeltaTime)
{
    // Regenerate stamina when not attacking or chasing
    if (CurrentState != ENPC_TRexState::Chasing && CurrentState != ENPC_TRexState::Attacking)
    {
        TRexStats.Stamina = FMath::Min(100.0f, TRexStats.Stamina + (DeltaTime * 10.0f));
    }
    // Drain stamina when active
    else if (CurrentState == ENPC_TRexState::Chasing)
    {
        TRexStats.Stamina = FMath::Max(0.0f, TRexStats.Stamina - (DeltaTime * 15.0f));
    }
}

void ANPC_TRexBehaviorTree::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        if (APawn* DetectedPawn = Cast<APawn>(Actor))
        {
            // Check if this is a valid prey target
            if (IsValidTarget(DetectedPawn))
            {
                float Distance = FVector::Dist(GetPawn()->GetActorLocation(), DetectedPawn->GetActorLocation());
                
                // If within chase radius and we don't have a target, or this one is closer
                if (Distance <= TRexStats.ChaseRadius)
                {
                    if (!CurrentTarget || Distance < GetDistanceToTarget())
                    {
                        SetTarget(DetectedPawn);
                        SetTRexState(ENPC_TRexState::Hunting);
                    }
                }
            }
        }
    }
}

void ANPC_TRexBehaviorTree::UpdateTRexBehavior(float DeltaTime)
{
    switch (CurrentState)
    {
        case ENPC_TRexState::Idle:
            HandleIdleState(DeltaTime);
            break;
        case ENPC_TRexState::Patrolling:
            HandlePatrollingState(DeltaTime);
            break;
        case ENPC_TRexState::Hunting:
            HandleHuntingState(DeltaTime);
            break;
        case ENPC_TRexState::Chasing:
            HandleChasingState(DeltaTime);
            break;
        case ENPC_TRexState::Attacking:
            HandleAttackingState(DeltaTime);
            break;
    }
}

void ANPC_TRexBehaviorTree::HandleIdleState(float DeltaTime)
{
    // Look for nearby prey
    APawn* NearbyPrey = FindNearestPrey();
    if (NearbyPrey)
    {
        SetTarget(NearbyPrey);
        SetTRexState(ENPC_TRexState::Hunting);
        return;
    }

    // If hungry, start patrolling to find food
    if (IsHungry())
    {
        SetTRexState(ENPC_TRexState::Patrolling);
    }
}

void ANPC_TRexBehaviorTree::HandlePatrollingState(float DeltaTime)
{
    // Look for prey while patrolling
    APawn* NearbyPrey = FindNearestPrey();
    if (NearbyPrey)
    {
        SetTarget(NearbyPrey);
        SetTRexState(ENPC_TRexState::Hunting);
        return;
    }

    // Continue patrolling if not tired
    if (!IsTired())
    {
        // Behavior tree handles actual movement to patrol points
    }
    else
    {
        SetTRexState(ENPC_TRexState::Resting);
    }
}

void ANPC_TRexBehaviorTree::HandleHuntingState(float DeltaTime)
{
    if (!CurrentTarget)
    {
        SetTRexState(ENPC_TRexState::Patrolling);
        return;
    }

    float DistanceToTarget = GetDistanceToTarget();
    
    // If target is within attack range
    if (DistanceToTarget <= TRexStats.AttackRadius)
    {
        SetTRexState(ENPC_TRexState::Attacking);
    }
    // If target is within chase range
    else if (DistanceToTarget <= TRexStats.ChaseRadius)
    {
        SetTRexState(ENPC_TRexState::Chasing);
    }
    // Target too far, give up
    else
    {
        SetTarget(nullptr);
        SetTRexState(ENPC_TRexState::Patrolling);
    }
}

void ANPC_TRexBehaviorTree::HandleChasingState(float DeltaTime)
{
    if (!CurrentTarget)
    {
        SetTRexState(ENPC_TRexState::Patrolling);
        return;
    }

    float DistanceToTarget = GetDistanceToTarget();
    
    // If close enough to attack
    if (DistanceToTarget <= TRexStats.AttackRadius)
    {
        SetTRexState(ENPC_TRexState::Attacking);
    }
    // If too tired or target too far, give up chase
    else if (IsTired() || DistanceToTarget > TRexStats.ChaseRadius * 1.5f)
    {
        SetTarget(nullptr);
        SetTRexState(ENPC_TRexState::Resting);
    }
}

void ANPC_TRexBehaviorTree::HandleAttackingState(float DeltaTime)
{
    if (!CurrentTarget)
    {
        SetTRexState(ENPC_TRexState::Patrolling);
        return;
    }

    float DistanceToTarget = GetDistanceToTarget();
    
    // If still in range and can attack
    if (DistanceToTarget <= TRexStats.AttackRadius && CanAttack())
    {
        PerformAttack();
    }
    // If target moved away, chase again
    else if (DistanceToTarget > TRexStats.AttackRadius && DistanceToTarget <= TRexStats.ChaseRadius)
    {
        SetTRexState(ENPC_TRexState::Chasing);
    }
    // Target escaped or we're too tired
    else
    {
        SetTarget(nullptr);
        SetTRexState(ENPC_TRexState::Resting);
    }
}

APawn* ANPC_TRexBehaviorTree::FindNearestPrey() const
{
    if (!GetPawn())
    {
        return nullptr;
    }

    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), FoundActors);

    APawn* NearestPrey = nullptr;
    float NearestDistance = TRexStats.ChaseRadius;

    for (AActor* Actor : FoundActors)
    {
        if (APawn* PotentialPrey = Cast<APawn>(Actor))
        {
            if (IsValidTarget(PotentialPrey))
            {
                float Distance = FVector::Dist(GetPawn()->GetActorLocation(), PotentialPrey->GetActorLocation());
                if (Distance < NearestDistance)
                {
                    NearestDistance = Distance;
                    NearestPrey = PotentialPrey;
                }
            }
        }
    }

    return NearestPrey;
}

bool ANPC_TRexBehaviorTree::IsValidTarget(APawn* Target) const
{
    if (!Target || Target == GetPawn())
    {
        return false;
    }

    // Don't target other T-Rex (could add species checking here)
    if (Target->GetClass() == GetPawn()->GetClass())
    {
        return false;
    }

    // Target must be alive (has health component or similar)
    return true;
}

void ANPC_TRexBehaviorTree::SetupAIPerception()
{
    if (!AIPerceptionComponent)
    {
        return;
    }

    // Setup sight sense
    UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    if (SightConfig)
    {
        SightConfig->SightRadius = TRexStats.ChaseRadius;
        SightConfig->LoseSightRadius = TRexStats.ChaseRadius * 1.2f;
        SightConfig->PeripheralVisionAngleDegrees = 120.0f;
        SightConfig->SetMaxAge(5.0f);
        SightConfig->AutoSuccessRangeFromLastSeenLocation = 500.0f;
        SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
        SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
        SightConfig->DetectionByAffiliation.bDetectEnemies = true;

        AIPerceptionComponent->ConfigureSense(*SightConfig);
    }

    // Setup hearing sense
    UAISenseConfig_Hearing* HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    if (HearingConfig)
    {
        HearingConfig->HearingRange = 2000.0f;
        HearingConfig->SetMaxAge(3.0f);
        HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
        HearingConfig->DetectionByAffiliation.bDetectFriendlies = false;
        HearingConfig->DetectionByAffiliation.bDetectEnemies = true;

        AIPerceptionComponent->ConfigureSense(*HearingConfig);
    }

    // Set sight as dominant sense
    AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());

    // Bind perception events
    AIPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ANPC_TRexBehaviorTree::OnPerceptionUpdated);
}

void ANPC_TRexBehaviorTree::GeneratePatrolPoints()
{
    PatrolPoints.Empty();

    // Generate 4 patrol points around territory center
    const float PatrolRadius = TRexStats.TerritorialRadius * 0.7f;
    const int32 NumPoints = 4;

    for (int32 i = 0; i < NumPoints; i++)
    {
        float Angle = (2.0f * PI * i) / NumPoints;
        FVector PatrolPoint = TerritoryCenter + FVector(
            FMath::Cos(Angle) * PatrolRadius,
            FMath::Sin(Angle) * PatrolRadius,
            0.0f
        );
        PatrolPoints.Add(PatrolPoint);
    }

    // Update blackboard with first patrol point
    if (PatrolPoints.Num() > 0 && BlackboardComponent)
    {
        BlackboardComponent->SetValueAsVector(TEXT("PatrolTarget"), PatrolPoints[0]);
    }
}