// TRexBehavior.cpp
// T-Rex AI Controller — patrol (5000u radius), chase (3000u), attack (300u)
// Agent #11 — NPC Behavior Agent | PROD_CYCLE_AUTO_20260622_011

#include "TRexBehavior.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "NavigationSystem.h"
#include "Navigation/PathFollowingComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"

ATRexBehaviorController::ATRexBehaviorController()
{
    PrimaryActorTick.bCanEverTick = true;

    // Setup AI Perception
    PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("TRexPerception"));
    SetPerceptionComponent(*PerceptionComponent);

    // Sight config
    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    if (SightConfig)
    {
        SightConfig->SightRadius = 3000.0f;
        SightConfig->LoseSightRadius = 3500.0f;
        SightConfig->PeripheralVisionAngleDegrees = 90.0f;
        SightConfig->SetMaxAge(5.0f);
        SightConfig->AutoSuccessRangeFromLastSeenLocation = 900.0f;
        SightConfig->DetectionByAffiliation.bDetectEnemies = true;
        SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
        SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
        PerceptionComponent->ConfigureSense(*SightConfig);
    }

    // Hearing config
    HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    if (HearingConfig)
    {
        HearingConfig->HearingRange = 1500.0f;
        HearingConfig->SetMaxAge(3.0f);
        HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
        HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
        HearingConfig->DetectionByAffiliation.bDetectFriendlies = false;
        PerceptionComponent->ConfigureSense(*HearingConfig);
    }

    PerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());
}

void ATRexBehaviorController::BeginPlay()
{
    Super::BeginPlay();

    if (PerceptionComponent)
    {
        PerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ATRexBehaviorController::OnPerceptionUpdated);
    }

    // Record patrol center from spawn location
    if (GetPawn())
    {
        PatrolCenter = GetPawn()->GetActorLocation();
    }

    GeneratePatrolWaypoints();
    SetBehaviorState(ENPC_TRexState::Patrolling);
}

void ATRexBehaviorController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    TimeSinceLastAttack += DeltaTime;
    PerceptionData.TimeSinceLastDetection += DeltaTime;

    UpdateBehaviorLogic(DeltaTime);
}

void ATRexBehaviorController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    PatrolCenter = InPawn->GetActorLocation();
    GeneratePatrolWaypoints();

    // Apply patrol movement speed
    ACharacter* DinoChar = Cast<ACharacter>(InPawn);
    if (DinoChar && DinoChar->GetCharacterMovement())
    {
        DinoChar->GetCharacterMovement()->MaxWalkSpeed = PatrolSpeed;
    }
}

void ATRexBehaviorController::OnUnPossess()
{
    Super::OnUnPossess();
    PlayerPawn = nullptr;
}

void ATRexBehaviorController::SetBehaviorState(ENPC_TRexState NewState)
{
    if (CurrentState == NewState) return;

    CurrentState = NewState;

    APawn* ControlledPawn = GetPawn();
    ACharacter* DinoChar = ControlledPawn ? Cast<ACharacter>(ControlledPawn) : nullptr;

    switch (NewState)
    {
    case ENPC_TRexState::Patrolling:
        if (DinoChar && DinoChar->GetCharacterMovement())
            DinoChar->GetCharacterMovement()->MaxWalkSpeed = PatrolSpeed;
        MoveToNextWaypoint();
        break;

    case ENPC_TRexState::Chasing:
        if (DinoChar && DinoChar->GetCharacterMovement())
            DinoChar->GetCharacterMovement()->MaxWalkSpeed = ChaseSpeed;
        break;

    case ENPC_TRexState::Attacking:
        StopMovement();
        ExecuteAttack();
        break;

    case ENPC_TRexState::Idle:
        StopMovement();
        break;

    case ENPC_TRexState::Resting:
        StopMovement();
        break;

    default:
        break;
    }
}

void ATRexBehaviorController::StartPatrol()
{
    GeneratePatrolWaypoints();
    SetBehaviorState(ENPC_TRexState::Patrolling);
}

void ATRexBehaviorController::MoveToNextWaypoint()
{
    if (PatrolWaypoints.Num() == 0) return;

    CurrentWaypointIndex = (CurrentWaypointIndex + 1) % PatrolWaypoints.Num();
    FVector Target = PatrolWaypoints[CurrentWaypointIndex];

    EPathFollowingRequestResult::Type Result = MoveToLocation(Target, 150.0f, true, true, true, false);
    if (Result == EPathFollowingRequestResult::Failed)
    {
        // Regenerate waypoints if navigation fails
        GeneratePatrolWaypoints();
    }
}

void ATRexBehaviorController::SetPatrolCenter(FVector Center, float Radius)
{
    PatrolCenter = Center;
    PatrolRadius = Radius;
    GeneratePatrolWaypoints();
}

bool ATRexBehaviorController::IsPlayerInChaseRange() const
{
    return GetDistanceToPlayer() <= ChaseRange;
}

bool ATRexBehaviorController::IsPlayerInAttackRange() const
{
    return GetDistanceToPlayer() <= AttackRange;
}

float ATRexBehaviorController::GetDistanceToPlayer() const
{
    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn) return TNumericLimits<float>::Max();

    APawn* Player = FindPlayerPawn();
    if (!Player) return TNumericLimits<float>::Max();

    return FVector::Dist(ControlledPawn->GetActorLocation(), Player->GetActorLocation());
}

void ATRexBehaviorController::ExecuteAttack()
{
    if (TimeSinceLastAttack < AttackCooldown) return;

    APawn* Player = FindPlayerPawn();
    if (!Player) return;

    if (IsPlayerInAttackRange())
    {
        TimeSinceLastAttack = 0.0f;

        // Apply damage via UE5 damage system
        UGameplayStatics::ApplyDamage(
            Player,
            AttackDamage,
            this,
            GetPawn(),
            UDamageType::StaticClass()
        );
    }
}

void ATRexBehaviorController::ChasePlayer()
{
    APawn* Player = FindPlayerPawn();
    if (!Player) return;

    MoveToActor(Player, AttackRange * 0.8f, true, true, true);
}

void ATRexBehaviorController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        APawn* PawnActor = Cast<APawn>(Actor);
        if (!PawnActor) continue;

        // Check if this is the player
        APlayerController* PC = Cast<APlayerController>(PawnActor->GetController());
        if (!PC) continue;

        FActorPerceptionBlueprintInfo Info;
        PerceptionComponent->GetActorsPerception(Actor, Info);

        bool bCurrentlySensed = false;
        for (const FAIStimulus& Stimulus : Info.LastSensedStimuli)
        {
            if (Stimulus.WasSuccessfullySensed())
            {
                bCurrentlySensed = true;
                PerceptionData.LastKnownPlayerLocation = Actor->GetActorLocation();
                PerceptionData.TimeSinceLastDetection = 0.0f;
                PerceptionData.ThreatLevel = FMath::Min(PerceptionData.ThreatLevel + 0.3f, 1.0f);
                break;
            }
        }

        PerceptionData.bPlayerDetectedBySight = bCurrentlySensed;
        PlayerPawn = bCurrentlySensed ? PawnActor : nullptr;
    }
}

// --- Private ---

void ATRexBehaviorController::GeneratePatrolWaypoints()
{
    PatrolWaypoints.Empty();

    // Generate 6 waypoints in a circle around PatrolCenter
    const int32 NumWaypoints = 6;
    for (int32 i = 0; i < NumWaypoints; ++i)
    {
        float Angle = (360.0f / NumWaypoints) * i;
        float RadAngle = FMath::DegreesToRadians(Angle);
        float Variation = FMath::RandRange(PatrolRadius * 0.6f, PatrolRadius);

        FVector WaypointPos = PatrolCenter + FVector(
            FMath::Cos(RadAngle) * Variation,
            FMath::Sin(RadAngle) * Variation,
            0.0f
        );

        // Project to navigation mesh
        UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
        if (NavSys)
        {
            FNavLocation NavLoc;
            if (NavSys->ProjectPointToNavigation(WaypointPos, NavLoc, FVector(500, 500, 500)))
            {
                PatrolWaypoints.Add(NavLoc.Location);
            }
            else
            {
                PatrolWaypoints.Add(WaypointPos);
            }
        }
        else
        {
            PatrolWaypoints.Add(WaypointPos);
        }
    }
}

void ATRexBehaviorController::UpdateBehaviorLogic(float DeltaTime)
{
    // Refresh player reference
    if (!PlayerPawn)
    {
        PlayerPawn = FindPlayerPawn();
    }

    float DistToPlayer = GetDistanceToPlayer();

    switch (CurrentState)
    {
    case ENPC_TRexState::Patrolling:
        // Check if player is close enough to trigger chase
        if (DistToPlayer <= ChaseRange && PlayerPawn)
        {
            PerceptionData.ThreatLevel = 0.8f;
            SetBehaviorState(ENPC_TRexState::Chasing);
        }
        else if (GetMoveStatus() == EPathFollowingStatus::Idle)
        {
            // Reached waypoint — move to next
            MoveToNextWaypoint();
        }
        break;

    case ENPC_TRexState::Chasing:
        if (!PlayerPawn || DistToPlayer > ChaseRange * 1.2f)
        {
            // Lost player — return to patrol
            PerceptionData.ThreatLevel = FMath::Max(0.0f, PerceptionData.ThreatLevel - DeltaTime * 0.2f);
            SetBehaviorState(ENPC_TRexState::Patrolling);
        }
        else if (DistToPlayer <= AttackRange)
        {
            SetBehaviorState(ENPC_TRexState::Attacking);
        }
        else
        {
            ChasePlayer();
        }
        break;

    case ENPC_TRexState::Attacking:
        if (!PlayerPawn || DistToPlayer > AttackRange * 1.5f)
        {
            SetBehaviorState(ENPC_TRexState::Chasing);
        }
        else if (TimeSinceLastAttack >= AttackCooldown)
        {
            ExecuteAttack();
        }
        break;

    case ENPC_TRexState::Idle:
        // After 5 seconds idle, resume patrol
        if (PerceptionData.TimeSinceLastDetection > 5.0f)
        {
            SetBehaviorState(ENPC_TRexState::Patrolling);
        }
        break;

    default:
        break;
    }
}

APawn* ATRexBehaviorController::FindPlayerPawn() const
{
    UWorld* World = GetWorld();
    if (!World) return nullptr;

    APlayerController* PC = World->GetFirstPlayerController();
    return PC ? PC->GetPawn() : nullptr;
}
