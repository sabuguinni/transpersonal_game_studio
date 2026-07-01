#include "AI/TRexBehavior.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "NavigationSystem.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"

ATRexAIController::ATRexAIController()
{
    PrimaryActorTick.bCanEverTick = true;

    // Set up AI Perception
    PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));
    SetPerceptionComponent(*PerceptionComponent);

    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = Config.SightRadius;
    SightConfig->LoseSightRadius = Config.LoseSightRadius;
    SightConfig->PeripheralVisionAngleDegrees = Config.PeripheralVisionAngle;
    SightConfig->SetMaxAge(5.0f);
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = false;

    HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    HearingConfig->HearingRange = Config.HearingRange;
    HearingConfig->SetMaxAge(3.0f);
    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;

    PerceptionComponent->ConfigureSense(*SightConfig);
    PerceptionComponent->ConfigureSense(*HearingConfig);
    PerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());

    // Initial state
    CurrentState = ENPC_TRexState::Patrolling;
    PreviousState = ENPC_TRexState::Resting;
    CurrentTarget = nullptr;
    StateTimer = 0.0f;
    AttackCooldownTimer = 0.0f;
    InvestigateTimer = 0.0f;
    bHasRoaredThisChase = false;
    CurrentWaypointIndex = 0;
}

void ATRexAIController::BeginPlay()
{
    Super::BeginPlay();

    // Bind perception delegate
    if (PerceptionComponent)
    {
        PerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(
            this, &ATRexAIController::OnTargetPerceptionUpdated);
    }
}

void ATRexAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    // Record spawn location as patrol origin
    if (InPawn)
    {
        SpawnLocation = InPawn->GetActorLocation();
    }

    // Generate patrol waypoints around spawn
    PatrolWaypoints.Empty();
    const int32 NumWaypoints = 6;
    for (int32 i = 0; i < NumWaypoints; ++i)
    {
        float Angle = (360.0f / NumWaypoints) * i;
        float Rad = FMath::DegreesToRadians(Angle);
        float Dist = Config.PatrolRadius * FMath::RandRange(0.4f, 1.0f);
        FVector WP = SpawnLocation + FVector(FMath::Cos(Rad) * Dist, FMath::Sin(Rad) * Dist, 0.0f);

        // Project onto nav mesh
        FNavLocation NavLoc;
        UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
        if (NavSys && NavSys->ProjectPointToNavigation(WP, NavLoc, FVector(500.0f, 500.0f, 500.0f)))
        {
            PatrolWaypoints.Add(NavLoc.Location);
        }
        else
        {
            PatrolWaypoints.Add(WP);
        }
    }

    CurrentWaypointIndex = 0;
    TransitionTo(ENPC_TRexState::Patrolling);
    MoveToPatrolPoint();
}

void ATRexAIController::OnUnPossess()
{
    Super::OnUnPossess();
    CurrentTarget = nullptr;
}

void ATRexAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    UpdateState(DeltaTime);
}

void ATRexAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (!Actor) return;

    // Only react to the player
    ACharacter* PlayerChar = Cast<ACharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
    if (Actor != PlayerChar) return;

    if (Stimulus.WasSuccessfullySensed())
    {
        CurrentTarget = Actor;

        // If we were patrolling or resting, investigate first
        if (CurrentState == ENPC_TRexState::Patrolling || CurrentState == ENPC_TRexState::Resting)
        {
            InvestigationPoint = Actor->GetActorLocation();
            TransitionTo(ENPC_TRexState::Investigating);
        }
        else if (CurrentState == ENPC_TRexState::Investigating)
        {
            // Escalate directly to chase
            bHasRoaredThisChase = false;
            TransitionTo(ENPC_TRexState::Chasing);
        }
    }
    else
    {
        // Lost sight — record last known position
        if (CurrentState == ENPC_TRexState::Chasing || CurrentState == ENPC_TRexState::Attacking)
        {
            InvestigationPoint = Stimulus.StimulusLocation;
            TransitionTo(ENPC_TRexState::Investigating);
        }
    }
}

void ATRexAIController::TransitionTo(ENPC_TRexState NewState)
{
    if (CurrentState == NewState) return;

    PreviousState = CurrentState;
    CurrentState = NewState;
    StateTimer = 0.0f;

    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn) return;

    ACharacter* TRexChar = Cast<ACharacter>(ControlledPawn);

    switch (NewState)
    {
    case ENPC_TRexState::Patrolling:
        if (TRexChar && TRexChar->GetCharacterMovement())
            TRexChar->GetCharacterMovement()->MaxWalkSpeed = Config.PatrolSpeed;
        MoveToPatrolPoint();
        break;

    case ENPC_TRexState::Investigating:
        if (TRexChar && TRexChar->GetCharacterMovement())
            TRexChar->GetCharacterMovement()->MaxWalkSpeed = Config.PatrolSpeed * 1.5f;
        MoveToLocation(InvestigationPoint);
        break;

    case ENPC_TRexState::Chasing:
        if (TRexChar && TRexChar->GetCharacterMovement())
            TRexChar->GetCharacterMovement()->MaxWalkSpeed = Config.ChaseSpeed;
        bHasRoaredThisChase = false;
        break;

    case ENPC_TRexState::Roaring:
        StopMovement();
        if (TRexChar && TRexChar->GetCharacterMovement())
            TRexChar->GetCharacterMovement()->MaxWalkSpeed = 0.0f;
        break;

    case ENPC_TRexState::Attacking:
        if (TRexChar && TRexChar->GetCharacterMovement())
            TRexChar->GetCharacterMovement()->MaxWalkSpeed = Config.AttackSpeed;
        break;

    case ENPC_TRexState::Resting:
        StopMovement();
        if (TRexChar && TRexChar->GetCharacterMovement())
            TRexChar->GetCharacterMovement()->MaxWalkSpeed = 0.0f;
        break;
    }
}

void ATRexAIController::UpdateState(float DeltaTime)
{
    StateTimer += DeltaTime;
    AttackCooldownTimer = FMath::Max(0.0f, AttackCooldownTimer - DeltaTime);

    switch (CurrentState)
    {
    case ENPC_TRexState::Patrolling:    TickPatrolling(DeltaTime);    break;
    case ENPC_TRexState::Investigating: TickInvestigating(DeltaTime); break;
    case ENPC_TRexState::Chasing:       TickChasing(DeltaTime);       break;
    case ENPC_TRexState::Attacking:     TickAttacking(DeltaTime);     break;
    case ENPC_TRexState::Roaring:       TickRoaring(DeltaTime);       break;
    case ENPC_TRexState::Resting:       TickResting(DeltaTime);       break;
    }
}

void ATRexAIController::TickPatrolling(float DeltaTime)
{
    // Check if we've reached the current waypoint
    EPathFollowingStatus::Type Status = GetMoveStatus();
    if (Status == EPathFollowingStatus::Idle || Status == EPathFollowingStatus::Waiting)
    {
        SelectNextPatrolWaypoint();
        MoveToPatrolPoint();
    }

    // Occasionally rest
    if (StateTimer > 120.0f && FMath::RandBool())
    {
        TransitionTo(ENPC_TRexState::Resting);
    }
}

void ATRexAIController::TickInvestigating(float DeltaTime)
{
    InvestigateTimer += DeltaTime;

    float DistToPlayer = GetDistanceToPlayer();

    // If player is close enough and we can see them, start chasing
    if (DistToPlayer < Config.ChaseRange && HasLineOfSightToPlayer())
    {
        bHasRoaredThisChase = false;
        TransitionTo(ENPC_TRexState::Chasing);
        return;
    }

    // Give up investigation after duration
    if (InvestigateTimer >= Config.InvestigateDuration)
    {
        InvestigateTimer = 0.0f;
        CurrentTarget = nullptr;
        TransitionTo(ENPC_TRexState::Patrolling);
    }
}

void ATRexAIController::TickChasing(float DeltaTime)
{
    if (!CurrentTarget)
    {
        TransitionTo(ENPC_TRexState::Patrolling);
        return;
    }

    float DistToPlayer = GetDistanceToPlayer();

    // Player escaped — give up
    if (DistToPlayer > Config.GiveUpRange)
    {
        CurrentTarget = nullptr;
        TransitionTo(ENPC_TRexState::Patrolling);
        return;
    }

    // Close enough to attack
    if (DistToPlayer <= Config.AttackRange)
    {
        TransitionTo(ENPC_TRexState::Attacking);
        return;
    }

    // Roar before first chase
    if (!bHasRoaredThisChase && DistToPlayer < Config.ChaseRange * 0.8f)
    {
        bHasRoaredThisChase = true;
        TransitionTo(ENPC_TRexState::Roaring);
        return;
    }

    // Move toward player
    MoveToActor(CurrentTarget, Config.AttackRange * 0.8f);
}

void ATRexAIController::TickAttacking(float DeltaTime)
{
    if (!CurrentTarget)
    {
        TransitionTo(ENPC_TRexState::Patrolling);
        return;
    }

    float DistToPlayer = GetDistanceToPlayer();

    // Player moved away — chase again
    if (DistToPlayer > Config.AttackRange * 1.5f)
    {
        TransitionTo(ENPC_TRexState::Chasing);
        return;
    }

    // Execute attack if cooldown is ready
    if (AttackCooldownTimer <= 0.0f)
    {
        ExecuteAttack();
        AttackCooldownTimer = Config.AttackCooldown;
    }

    // Keep moving toward player
    MoveToActor(CurrentTarget, Config.AttackRange * 0.5f);
}

void ATRexAIController::TickRoaring(float DeltaTime)
{
    // After roar duration, transition to chasing
    if (StateTimer >= Config.RoarDuration)
    {
        TransitionTo(ENPC_TRexState::Chasing);
    }
}

void ATRexAIController::TickResting(float DeltaTime)
{
    // After rest duration, resume patrol
    if (StateTimer >= Config.RestDuration)
    {
        TransitionTo(ENPC_TRexState::Patrolling);
    }

    // Emergency: player gets very close while resting
    float DistToPlayer = GetDistanceToPlayer();
    if (DistToPlayer < Config.ChaseRange * 0.5f)
    {
        bHasRoaredThisChase = false;
        TransitionTo(ENPC_TRexState::Chasing);
    }
}

float ATRexAIController::GetDistanceToPlayer() const
{
    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn) return TNumericLimits<float>::Max();

    ACharacter* PlayerChar = Cast<ACharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
    if (!PlayerChar) return TNumericLimits<float>::Max();

    return FVector::Dist(ControlledPawn->GetActorLocation(), PlayerChar->GetActorLocation());
}

bool ATRexAIController::HasLineOfSightToPlayer() const
{
    ACharacter* PlayerChar = Cast<ACharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
    if (!PlayerChar) return false;

    return LineOfSightTo(PlayerChar);
}

FVector ATRexAIController::GetRandomPatrolPoint() const
{
    float Angle = FMath::RandRange(0.0f, 360.0f);
    float Rad = FMath::DegreesToRadians(Angle);
    float Dist = FMath::RandRange(Config.PatrolRadius * 0.3f, Config.PatrolRadius);
    return SpawnLocation + FVector(FMath::Cos(Rad) * Dist, FMath::Sin(Rad) * Dist, 0.0f);
}

void ATRexAIController::MoveToPatrolPoint()
{
    if (PatrolWaypoints.IsValidIndex(CurrentWaypointIndex))
    {
        MoveToLocation(PatrolWaypoints[CurrentWaypointIndex], 150.0f);
    }
    else
    {
        FVector RandomPoint = GetRandomPatrolPoint();
        MoveToLocation(RandomPoint, 150.0f);
    }
}

void ATRexAIController::SelectNextPatrolWaypoint()
{
    if (PatrolWaypoints.Num() == 0) return;
    CurrentWaypointIndex = (CurrentWaypointIndex + 1) % PatrolWaypoints.Num();
}

void ATRexAIController::ExecuteAttack()
{
    if (!CurrentTarget) return;

    // Apply damage to the target
    UGameplayStatics::ApplyDamage(
        CurrentTarget,
        Config.AttackDamage,
        this,
        GetPawn(),
        UDamageType::StaticClass()
    );

    UE_LOG(LogTemp, Log, TEXT("TRex attacked %s for %.1f damage"), *CurrentTarget->GetName(), Config.AttackDamage);
}

void ATRexAIController::ForceState(ENPC_TRexState NewState)
{
    TransitionTo(NewState);
}
