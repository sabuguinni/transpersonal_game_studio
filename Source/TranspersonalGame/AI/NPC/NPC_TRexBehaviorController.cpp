#include "NPC_TRexBehaviorController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardAsset.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISense_Hearing.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "NavigationSystem.h"
#include "Math/UnrealMathUtility.h"

// Blackboard key definitions
const FName ANPC_TRexBehaviorController::BB_CurrentTarget(TEXT("CurrentTarget"));
const FName ANPC_TRexBehaviorController::BB_PatrolPoint(TEXT("PatrolPoint"));
const FName ANPC_TRexBehaviorController::BB_BehaviorState(TEXT("BehaviorState"));
const FName ANPC_TRexBehaviorController::BB_TerritoryCenter(TEXT("TerritoryCenter"));
const FName ANPC_TRexBehaviorController::BB_IsInTerritory(TEXT("IsInTerritory"));

ANPC_TRexBehaviorController::ANPC_TRexBehaviorController()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Initialize AI components
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
    SightConfig = CreateDefaultSubobject<UAISightConfig>(TEXT("SightConfig"));
    HearingConfig = CreateDefaultSubobject<UAIHearingConfig>(TEXT("HearingConfig"));
    
    // Territory settings (5000 unit radius as specified)
    TerritoryRadius = 5000.0f;
    TerritoryCenter = FVector::ZeroVector;
    
    // Patrol settings
    PatrolSpeed = 300.0f;
    PatrolWaitTime = 5.0f;
    
    // Combat settings (3000 unit chase range, 300 unit attack range as specified)
    DetectionRange = 3000.0f;
    AttackRange = 300.0f;
    ChaseSpeed = 600.0f;
    AttackCooldown = 3.0f;
    
    // Initialize state
    CurrentBehaviorState = ENPC_DinosaurBehaviorState::Idle;
    CurrentTarget = nullptr;
    LastAttackTime = 0.0f;
    PatrolTimer = 0.0f;
    StateChangeTime = 0.0f;
    bIsAttacking = false;
}

void ANPC_TRexBehaviorController::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeAIPerception();
    SetupBehaviorTree();
    
    // Set territory center to current location if not set
    if (TerritoryCenter.IsZero() && GetPawn())
    {
        TerritoryCenter = GetPawn()->GetActorLocation();
    }
    
    // Start patrolling
    StartPatrolling();
}

void ANPC_TRexBehaviorController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update behavior based on current state
    switch (CurrentBehaviorState)
    {
        case ENPC_DinosaurBehaviorState::Patrolling:
            UpdatePatrolBehavior(DeltaTime);
            break;
        case ENPC_DinosaurBehaviorState::Hunting:
            UpdateHuntingBehavior(DeltaTime);
            break;
        case ENPC_DinosaurBehaviorState::Attacking:
            UpdateCombatBehavior(DeltaTime);
            break;
        default:
            break;
    }
    
    // Update blackboard with current state
    UpdateBlackboardKeys();
}

void ANPC_TRexBehaviorController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
    
    if (InPawn)
    {
        // Set territory center to spawn location
        TerritoryCenter = InPawn->GetActorLocation();
        
        // Start behavior tree if available
        if (TRexBehaviorTree && Blackboard)
        {
            RunBehaviorTree(TRexBehaviorTree);
        }
    }
}

void ANPC_TRexBehaviorController::InitializeAIPerception()
{
    if (!AIPerceptionComponent || !SightConfig || !HearingConfig)
        return;
    
    // Configure sight perception
    SightConfig->SightRadius = DetectionRange;
    SightConfig->LoseSightRadius = DetectionRange * 1.2f;
    SightConfig->PeripheralVisionAngleDegrees = 120.0f;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    
    // Configure hearing perception
    HearingConfig->HearingRange = DetectionRange * 0.5f;
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
    HearingConfig->DetectionByAffiliation.bDetectFriendlies = false;
    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
    
    // Add senses to perception component
    AIPerceptionComponent->ConfigureSense(*SightConfig);
    AIPerceptionComponent->ConfigureSense(*HearingConfig);
    AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());
    
    // Bind perception events
    AIPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ANPC_TRexBehaviorController::OnPerceptionUpdated);
}

void ANPC_TRexBehaviorController::SetupBehaviorTree()
{
    if (Blackboard)
    {
        // Initialize blackboard values
        Blackboard->SetValueAsVector(BB_TerritoryCenter, TerritoryCenter);
        Blackboard->SetValueAsEnum(BB_BehaviorState, static_cast<uint8>(CurrentBehaviorState));
        Blackboard->SetValueAsBool(BB_IsInTerritory, true);
    }
}

void ANPC_TRexBehaviorController::SetBehaviorState(ENPC_DinosaurBehaviorState NewState)
{
    if (CurrentBehaviorState != NewState)
    {
        CurrentBehaviorState = NewState;
        StateChangeTime = GetWorld()->GetTimeSeconds();
        
        UE_LOG(LogTemp, Log, TEXT("T-Rex behavior state changed to: %d"), static_cast<int32>(NewState));
    }
}

void ANPC_TRexBehaviorController::StartPatrolling()
{
    SetBehaviorState(ENPC_DinosaurBehaviorState::Patrolling);
    CurrentTarget = nullptr;
    CurrentPatrolPoint = GetRandomPatrolPoint();
    
    if (GetPawn())
    {
        MoveToLocation(CurrentPatrolPoint, 100.0f);
    }
}

void ANPC_TRexBehaviorController::StartHunting(AActor* Target)
{
    if (!Target || !IsTargetValid(Target))
        return;
    
    SetBehaviorState(ENPC_DinosaurBehaviorState::Hunting);
    CurrentTarget = Target;
    LastKnownTargetLocation = Target->GetActorLocation();
    
    if (GetPawn())
    {
        MoveToActor(Target, AttackRange);
    }
}

void ANPC_TRexBehaviorController::StartAttacking(AActor* Target)
{
    if (!Target || !CanAttackTarget(Target))
        return;
    
    SetBehaviorState(ENPC_DinosaurBehaviorState::Attacking);
    CurrentTarget = Target;
    
    // Stop movement and face target
    StopMovement();
    
    if (GetPawn())
    {
        FVector LookDirection = (Target->GetActorLocation() - GetPawn()->GetActorLocation()).GetSafeNormal();
        FRotator LookRotation = LookDirection.Rotation();
        GetPawn()->SetActorRotation(LookRotation);
    }
    
    ExecuteAttack();
}

void ANPC_TRexBehaviorController::ReturnToTerritory()
{
    if (!IsWithinTerritory(GetPawn()->GetActorLocation()))
    {
        SetBehaviorState(ENPC_DinosaurBehaviorState::Returning);
        MoveToLocation(TerritoryCenter, 200.0f);
    }
    else
    {
        StartPatrolling();
    }
}

void ANPC_TRexBehaviorController::SetTerritoryCenter(const FVector& NewCenter)
{
    TerritoryCenter = NewCenter;
    
    if (Blackboard)
    {
        Blackboard->SetValueAsVector(BB_TerritoryCenter, TerritoryCenter);
    }
}

bool ANPC_TRexBehaviorController::IsWithinTerritory(const FVector& Location) const
{
    return FVector::Dist(Location, TerritoryCenter) <= TerritoryRadius;
}

FVector ANPC_TRexBehaviorController::GetRandomPatrolPoint() const
{
    // Generate random point within territory
    float RandomAngle = FMath::RandRange(0.0f, 2.0f * PI);
    float RandomRadius = FMath::RandRange(TerritoryRadius * 0.3f, TerritoryRadius * 0.8f);
    
    FVector RandomOffset = FVector(
        FMath::Cos(RandomAngle) * RandomRadius,
        FMath::Sin(RandomAngle) * RandomRadius,
        0.0f
    );
    
    return TerritoryCenter + RandomOffset;
}

void ANPC_TRexBehaviorController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    // Find the best target from perceived actors
    AActor* BestTarget = FindBestTarget();
    
    if (BestTarget && CurrentBehaviorState == ENPC_DinosaurBehaviorState::Patrolling)
    {
        // Switch to hunting if we detect a target while patrolling
        StartHunting(BestTarget);
    }
    else if (!BestTarget && CurrentBehaviorState == ENPC_DinosaurBehaviorState::Hunting)
    {
        // Return to patrolling if we lose our target
        StartPatrolling();
    }
}

AActor* ANPC_TRexBehaviorController::FindBestTarget() const
{
    if (!AIPerceptionComponent)
        return nullptr;
    
    TArray<AActor*> PerceivedActors;
    AIPerceptionComponent->GetCurrentlyPerceivedActors(UAISense_Sight::StaticClass(), PerceivedActors);
    
    AActor* BestTarget = nullptr;
    float BestThreatLevel = 0.0f;
    
    for (AActor* Actor : PerceivedActors)
    {
        if (IsTargetValid(Actor))
        {
            float ThreatLevel = EvaluateTargetThreat(Actor);
            if (ThreatLevel > BestThreatLevel)
            {
                BestTarget = Actor;
                BestThreatLevel = ThreatLevel;
            }
        }
    }
    
    return BestTarget;
}

float ANPC_TRexBehaviorController::EvaluateTargetThreat(AActor* Target) const
{
    if (!Target || !GetPawn())
        return 0.0f;
    
    float Distance = GetDistanceToTarget(Target);
    float ThreatLevel = 1.0f;
    
    // Closer targets are higher priority
    ThreatLevel *= (1.0f - (Distance / DetectionRange));
    
    // Player characters are high priority
    if (Target->IsA<ACharacter>())
    {
        ThreatLevel *= 2.0f;
    }
    
    return ThreatLevel;
}

bool ANPC_TRexBehaviorController::CanAttackTarget(AActor* Target) const
{
    if (!Target || !GetPawn())
        return false;
    
    float Distance = GetDistanceToTarget(Target);
    float TimeSinceLastAttack = GetWorld()->GetTimeSeconds() - LastAttackTime;
    
    return Distance <= AttackRange && TimeSinceLastAttack >= AttackCooldown;
}

void ANPC_TRexBehaviorController::ExecuteAttack()
{
    if (!CurrentTarget || bIsAttacking)
        return;
    
    bIsAttacking = true;
    LastAttackTime = GetWorld()->GetTimeSeconds();
    
    UE_LOG(LogTemp, Log, TEXT("T-Rex executing attack on target: %s"), 
           CurrentTarget ? *CurrentTarget->GetName() : TEXT("None"));
    
    // Schedule attack completion
    FTimerHandle AttackTimer;
    GetWorld()->GetTimerManager().SetTimer(AttackTimer, this, 
                                         &ANPC_TRexBehaviorController::OnAttackComplete, 
                                         2.0f, false);
}

void ANPC_TRexBehaviorController::OnAttackComplete()
{
    bIsAttacking = false;
    
    // Check if target is still in range for another attack
    if (CurrentTarget && CanAttackTarget(CurrentTarget))
    {
        ExecuteAttack();
    }
    else if (CurrentTarget && GetDistanceToTarget(CurrentTarget) <= DetectionRange)
    {
        // Continue hunting if target is still in detection range
        StartHunting(CurrentTarget);
    }
    else
    {
        // Return to patrolling
        StartPatrolling();
    }
}

void ANPC_TRexBehaviorController::UpdateBlackboardKeys()
{
    if (!Blackboard)
        return;
    
    Blackboard->SetValueAsObject(BB_CurrentTarget, CurrentTarget);
    Blackboard->SetValueAsVector(BB_PatrolPoint, CurrentPatrolPoint);
    Blackboard->SetValueAsEnum(BB_BehaviorState, static_cast<uint8>(CurrentBehaviorState));
    Blackboard->SetValueAsBool(BB_IsInTerritory, IsWithinTerritory(GetPawn() ? GetPawn()->GetActorLocation() : FVector::ZeroVector));
}

void ANPC_TRexBehaviorController::UpdatePatrolBehavior(float DeltaTime)
{
    if (!GetPawn())
        return;
    
    PatrolTimer += DeltaTime;
    
    // Check if we've reached the patrol point
    float DistanceToPatrol = FVector::Dist(GetPawn()->GetActorLocation(), CurrentPatrolPoint);
    
    if (DistanceToPatrol <= 150.0f)
    {
        if (PatrolTimer >= PatrolWaitTime)
        {
            // Get new patrol point and move
            CurrentPatrolPoint = GetRandomPatrolPoint();
            MoveToLocation(CurrentPatrolPoint, 100.0f);
            PatrolTimer = 0.0f;
        }
    }
    else if (PatrolTimer > PatrolWaitTime * 2.0f)
    {
        // If we're taking too long to reach patrol point, get a new one
        CurrentPatrolPoint = GetRandomPatrolPoint();
        MoveToLocation(CurrentPatrolPoint, 100.0f);
        PatrolTimer = 0.0f;
    }
}

void ANPC_TRexBehaviorController::UpdateHuntingBehavior(float DeltaTime)
{
    if (!CurrentTarget || !GetPawn())
    {
        StartPatrolling();
        return;
    }
    
    float DistanceToTarget = GetDistanceToTarget(CurrentTarget);
    
    // Check if target is in attack range
    if (DistanceToTarget <= AttackRange)
    {
        StartAttacking(CurrentTarget);
    }
    // Check if target is still within detection range
    else if (DistanceToTarget <= DetectionRange)
    {
        // Update last known location and continue pursuit
        LastKnownTargetLocation = CurrentTarget->GetActorLocation();
        MoveToActor(CurrentTarget, AttackRange);
    }
    else
    {
        // Target escaped, return to patrolling
        StartPatrolling();
    }
}

void ANPC_TRexBehaviorController::UpdateCombatBehavior(float DeltaTime)
{
    if (!CurrentTarget || !GetPawn())
    {
        StartPatrolling();
        return;
    }
    
    float DistanceToTarget = GetDistanceToTarget(CurrentTarget);
    
    // If target moved out of attack range, resume hunting
    if (DistanceToTarget > AttackRange && !bIsAttacking)
    {
        StartHunting(CurrentTarget);
    }
}

bool ANPC_TRexBehaviorController::IsTargetValid(AActor* Target) const
{
    if (!Target || Target == GetPawn())
        return false;
    
    // Check if target is a character (player or NPC)
    if (Target->IsA<ACharacter>())
        return true;
    
    return false;
}

float ANPC_TRexBehaviorController::GetDistanceToTarget(AActor* Target) const
{
    if (!Target || !GetPawn())
        return MAX_FLT;
    
    return FVector::Dist(GetPawn()->GetActorLocation(), Target->GetActorLocation());
}

FVector ANPC_TRexBehaviorController::CalculateInterceptPoint(AActor* Target) const
{
    if (!Target || !GetPawn())
        return FVector::ZeroVector;
    
    // Simple interception calculation
    FVector TargetVelocity = Target->GetVelocity();
    FVector ToTarget = Target->GetActorLocation() - GetPawn()->GetActorLocation();
    
    float TimeToIntercept = ToTarget.Size() / ChaseSpeed;
    return Target->GetActorLocation() + (TargetVelocity * TimeToIntercept);
}