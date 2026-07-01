#include "TRexBehavior.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────────────────

ATRexAIController::ATRexAIController()
{
    PrimaryActorTick.bCanEverTick = true;

    // Perception setup
    PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("TRexPerception"));
    SetPerceptionComponent(*PerceptionComponent);

    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = 3000.0f;
    SightConfig->LoseSightRadius = 3500.0f;
    SightConfig->PeripheralVisionAngleDegrees = 90.0f;
    SightConfig->SetMaxAge(5.0f);
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = false;

    HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    HearingConfig->HearingRange = 1500.0f;
    HearingConfig->SetMaxAge(3.0f);
    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;

    PerceptionComponent->ConfigureSense(*SightConfig);
    PerceptionComponent->ConfigureSense(*HearingConfig);
    PerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());

    // Internal defaults
    CurrentState = ENPC_TRexState::Patrolling;
    TargetActor = nullptr;
    TimeSinceLastAttack = 0.0f;
    TimeSinceTargetSeen = 0.0f;
    RoarTimer = 0.0f;
}

// ─────────────────────────────────────────────────────────────────────────────
// Lifecycle
// ─────────────────────────────────────────────────────────────────────────────

void ATRexAIController::BeginPlay()
{
    Super::BeginPlay();

    if (PerceptionComponent)
    {
        PerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(
            this, &ATRexAIController::OnPerceptionUpdated);
    }
}

void ATRexAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    if (InPawn)
    {
        HomeLocation = InPawn->GetActorLocation();
        UE_LOG(LogTemp, Log, TEXT("ATRexAIController: Possessed %s at %s"),
               *InPawn->GetName(), *HomeLocation.ToString());
    }

    // Start patrol immediately
    SetState(ENPC_TRexState::Patrolling);
    PickNewPatrolPoint();
}

void ATRexAIController::OnUnPossess()
{
    Super::OnUnPossess();
    TargetActor = nullptr;
}

// ─────────────────────────────────────────────────────────────────────────────
// Tick — State Machine Dispatch
// ─────────────────────────────────────────────────────────────────────────────

void ATRexAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    TimeSinceLastAttack += DeltaTime;

    switch (CurrentState)
    {
    case ENPC_TRexState::Patrolling:
        TickPatrol(DeltaTime);
        break;
    case ENPC_TRexState::Investigating:
        TickChase(DeltaTime);
        break;
    case ENPC_TRexState::Chasing:
        TickChase(DeltaTime);
        break;
    case ENPC_TRexState::Attacking:
        TickAttack(DeltaTime);
        break;
    case ENPC_TRexState::Roaring:
        TickRoar(DeltaTime);
        break;
    case ENPC_TRexState::Resting:
        TickResting(DeltaTime);
        break;
    default:
        break;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// State Handlers
// ─────────────────────────────────────────────────────────────────────────────

void ATRexAIController::TickPatrol(float DeltaTime)
{
    // Check if we can see the player
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (PC && PC->GetPawn())
    {
        float Dist = FVector::Dist(GetPawn()->GetActorLocation(),
                                   PC->GetPawn()->GetActorLocation());
        if (Dist <= Config.ChaseDetectionRange)
        {
            TargetActor = PC->GetPawn();
            SetState(ENPC_TRexState::Roaring); // Roar before chasing
            return;
        }
    }

    // Check if we reached the patrol point
    if (GetMoveStatus() == EPathFollowingStatus::Idle)
    {
        PickNewPatrolPoint();
    }
}

void ATRexAIController::TickChase(float DeltaTime)
{
    if (!TargetActor)
    {
        SetState(ENPC_TRexState::Patrolling);
        return;
    }

    float Dist = DistanceToTarget();

    // Close enough to attack
    if (Dist <= Config.AttackRange)
    {
        SetState(ENPC_TRexState::Attacking);
        return;
    }

    // Lost sight — give up after timeout
    if (!CanSeeTarget())
    {
        TimeSinceTargetSeen += DeltaTime;
        if (TimeSinceTargetSeen >= Config.ChaseGiveUpTime)
        {
            UE_LOG(LogTemp, Log, TEXT("ATRexAIController: Lost target — returning to patrol"));
            TargetActor = nullptr;
            TimeSinceTargetSeen = 0.0f;
            SetState(ENPC_TRexState::Patrolling);
            PickNewPatrolPoint();
            return;
        }
    }
    else
    {
        TimeSinceTargetSeen = 0.0f;
    }

    // Move toward target
    ApplyMoveSpeed(Config.ChaseSpeed);
    MoveToActor(TargetActor, Config.AttackRange * 0.8f);
}

void ATRexAIController::TickAttack(float DeltaTime)
{
    if (!TargetActor)
    {
        SetState(ENPC_TRexState::Patrolling);
        return;
    }

    float Dist = DistanceToTarget();

    // Target escaped — resume chase
    if (Dist > Config.AttackRange * 1.5f)
    {
        SetState(ENPC_TRexState::Chasing);
        return;
    }

    // Attack on cooldown
    if (TimeSinceLastAttack >= Config.AttackCooldown)
    {
        TimeSinceLastAttack = 0.0f;

        // Apply damage via UE5 damage system
        if (APawn* TargetPawn = Cast<APawn>(TargetActor))
        {
            UGameplayStatics::ApplyDamage(
                TargetActor,
                Config.AttackDamage,
                this,
                GetPawn(),
                UDamageType::StaticClass());

            UE_LOG(LogTemp, Log, TEXT("ATRexAIController: ATTACK — dealt %.0f damage to %s"),
                   Config.AttackDamage, *TargetActor->GetName());
        }
    }
}

void ATRexAIController::TickRoar(float DeltaTime)
{
    RoarTimer += DeltaTime;
    StopMovement();

    // After 2 seconds of roaring, begin chase
    if (RoarTimer >= 2.0f)
    {
        RoarTimer = 0.0f;
        SetState(ENPC_TRexState::Chasing);
    }
}

void ATRexAIController::TickResting(float DeltaTime)
{
    // Resting is passive — just wait
    // Could be extended with a rest timer
    StopMovement();
}

// ─────────────────────────────────────────────────────────────────────────────
// Helpers
// ─────────────────────────────────────────────────────────────────────────────

void ATRexAIController::SetState(ENPC_TRexState NewState)
{
    if (CurrentState == NewState) return;

    UE_LOG(LogTemp, Log, TEXT("ATRexAIController: State %d → %d"),
           (int32)CurrentState, (int32)NewState);

    CurrentState = NewState;

    // Apply appropriate move speed on state entry
    switch (NewState)
    {
    case ENPC_TRexState::Patrolling:
        ApplyMoveSpeed(Config.PatrolSpeed);
        break;
    case ENPC_TRexState::Chasing:
    case ENPC_TRexState::Investigating:
        ApplyMoveSpeed(Config.ChaseSpeed);
        break;
    default:
        break;
    }
}

void ATRexAIController::ForceRoar()
{
    RoarTimer = 0.0f;
    SetState(ENPC_TRexState::Roaring);
}

void ATRexAIController::PickNewPatrolPoint()
{
    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
    if (!NavSys || !GetPawn()) return;

    FNavLocation NavLoc;
    bool bFound = NavSys->GetRandomReachablePointInRadius(HomeLocation, Config.PatrolRadius, NavLoc);

    if (bFound)
    {
        ApplyMoveSpeed(Config.PatrolSpeed);
        MoveToLocation(NavLoc.Location, 50.0f);
        UE_LOG(LogTemp, Verbose, TEXT("ATRexAIController: New patrol point %s"), *NavLoc.Location.ToString());
    }
}

bool ATRexAIController::CanSeeTarget() const
{
    if (!TargetActor || !GetPawn()) return false;

    FVector Start = GetPawn()->GetActorLocation() + FVector(0, 0, 150.0f);
    FVector End = TargetActor->GetActorLocation() + FVector(0, 0, 100.0f);

    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(GetPawn());

    bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params);
    return !bHit || Hit.GetActor() == TargetActor;
}

float ATRexAIController::DistanceToTarget() const
{
    if (!TargetActor || !GetPawn()) return TNumericLimits<float>::Max();
    return FVector::Dist(GetPawn()->GetActorLocation(), TargetActor->GetActorLocation());
}

void ATRexAIController::ApplyMoveSpeed(float Speed)
{
    APawn* Pawn = GetPawn();
    if (!Pawn) return;

    if (ACharacter* Char = Cast<ACharacter>(Pawn))
    {
        if (UCharacterMovementComponent* Move = Char->GetCharacterMovement())
        {
            Move->MaxWalkSpeed = Speed;
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Perception Callback
// ─────────────────────────────────────────────────────────────────────────────

void ATRexAIController::OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (!Actor) return;

    // Only react to the player
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PC || Actor != PC->GetPawn()) return;

    if (Stimulus.WasSuccessfullySensed())
    {
        if (CurrentState == ENPC_TRexState::Patrolling ||
            CurrentState == ENPC_TRexState::Resting)
        {
            TargetActor = Actor;
            SetState(ENPC_TRexState::Roaring);
            UE_LOG(LogTemp, Warning, TEXT("ATRexAIController: Player detected! ROAR → CHASE"));
        }
    }
    else
    {
        // Stimulus lost — start give-up timer (handled in TickChase)
        UE_LOG(LogTemp, Log, TEXT("ATRexAIController: Lost perception of player"));
    }
}
