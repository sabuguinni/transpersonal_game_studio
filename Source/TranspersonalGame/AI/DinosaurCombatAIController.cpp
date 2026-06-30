#include "DinosaurCombatAIController.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "DrawDebugHelpers.h"

ADinosaurCombatAIController::ADinosaurCombatAIController()
{
    PrimaryActorTick.bCanEverTick = true;

    // Setup AI Perception
    PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));
    SetPerceptionComponent(*PerceptionComponent);

    // Sight config
    UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = 3000.0f;
    SightConfig->LoseSightRadius = 3500.0f;
    SightConfig->PeripheralVisionAngleDegrees = 90.0f;
    SightConfig->SetMaxAge(5.0f);
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    PerceptionComponent->ConfigureSense(*SightConfig);

    // Hearing config
    UAISenseConfig_Hearing* HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    HearingConfig->HearingRange = 1500.0f;
    HearingConfig->SetMaxAge(3.0f);
    PerceptionComponent->ConfigureSense(*HearingConfig);

    PerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());
}

void ADinosaurCombatAIController::BeginPlay()
{
    Super::BeginPlay();

    // Record home location from possessed pawn
    if (GetPawn())
    {
        HomeLocation = GetPawn()->GetActorLocation();
    }

    // Configure species-specific parameters
    ConfigureForSpecies(DinoSpecies);

    // Bind perception delegate
    if (PerceptionComponent)
    {
        PerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(
            this, &ADinosaurCombatAIController::OnTargetPerceived);
    }

    SetCombatState(ECombat_DinoState::Patrolling);
}

void ADinosaurCombatAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    UpdateStateMachine(DeltaTime);
}

void ADinosaurCombatAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
    if (InPawn)
    {
        HomeLocation = InPawn->GetActorLocation();
    }
}

void ADinosaurCombatAIController::OnUnPossess()
{
    Super::OnUnPossess();
    CurrentTarget = nullptr;
    SetCombatState(ECombat_DinoState::Idle);
}

// ─── State Machine ─────────────────────────────────────────────────────────

void ADinosaurCombatAIController::SetCombatState(ECombat_DinoState NewState)
{
    if (CurrentState == NewState) return;

    CurrentState = NewState;
    StateEntryTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

    // Update movement speed based on state
    APawn* Pawn = GetPawn();
    if (!Pawn) return;

    ACharacter* Char = Cast<ACharacter>(Pawn);
    if (Char && Char->GetCharacterMovement())
    {
        float Speed = (NewState == ECombat_DinoState::Charging || NewState == ECombat_DinoState::Attacking)
            ? CombatParams.MoveSpeed_Chase
            : CombatParams.MoveSpeed_Patrol;
        Char->GetCharacterMovement()->MaxWalkSpeed = Speed;
    }
}

void ADinosaurCombatAIController::UpdateStateMachine(float DeltaTime)
{
    switch (CurrentState)
    {
        case ECombat_DinoState::Idle:
            HandleIdleState(DeltaTime);
            break;
        case ECombat_DinoState::Patrolling:
            HandlePatrolState(DeltaTime);
            break;
        case ECombat_DinoState::Stalking:
        case ECombat_DinoState::Charging:
            HandleChaseState(DeltaTime);
            break;
        case ECombat_DinoState::Attacking:
            HandleAttackState(DeltaTime);
            break;
        case ECombat_DinoState::Retreating:
            HandleRetreatState(DeltaTime);
            break;
        default:
            break;
    }
}

void ADinosaurCombatAIController::HandleIdleState(float DeltaTime)
{
    float TimeInState = GetWorld()->GetTimeSeconds() - StateEntryTime;
    if (TimeInState > 3.0f)
    {
        SetCombatState(ECombat_DinoState::Patrolling);
    }
}

void ADinosaurCombatAIController::HandlePatrolState(float DeltaTime)
{
    APawn* Pawn = GetPawn();
    if (!Pawn) return;

    // Check if far from territory — return home
    float DistFromHome = FVector::Dist(Pawn->GetActorLocation(), HomeLocation);
    if (DistFromHome > CombatParams.TerritoryReturnRange)
    {
        ReturnToTerritory();
        return;
    }

    // Move to next patrol point
    FVector PatrolTarget = GetNextPatrolPoint();
    MoveToLocation(PatrolTarget, 200.0f);
}

void ADinosaurCombatAIController::HandleChaseState(float DeltaTime)
{
    if (!CurrentTarget)
    {
        SetCombatState(ECombat_DinoState::Investigating);
        return;
    }

    float Dist = GetDistanceToTarget();

    // Switch to attack if close enough
    if (Dist <= CombatParams.AttackRange)
    {
        SetCombatState(ECombat_DinoState::Attacking);
        return;
    }

    // Lose target if too far
    if (Dist > CombatParams.ChaseActivationRange * 1.5f)
    {
        RecordThreatMemory(CurrentTarget);
        CurrentTarget = nullptr;
        SetCombatState(ECombat_DinoState::Investigating);
        return;
    }

    // Chase target
    MoveToActor(CurrentTarget, CombatParams.AttackRange * 0.8f);

    // Signal pack members if pack hunter
    if (CombatParams.bIsPackHunter)
    {
        float TimeInState = GetWorld()->GetTimeSeconds() - StateEntryTime;
        if (TimeInState < 0.5f) // Signal once on state entry
        {
            SignalPackMembers(CurrentTarget);
        }
    }
}

void ADinosaurCombatAIController::HandleAttackState(float DeltaTime)
{
    if (!CurrentTarget)
    {
        SetCombatState(ECombat_DinoState::Patrolling);
        return;
    }

    float Dist = GetDistanceToTarget();

    // Target escaped attack range — resume chase
    if (Dist > CombatParams.AttackRange * 1.5f)
    {
        SetCombatState(ECombat_DinoState::Charging);
        return;
    }

    // Execute attack on cooldown
    float Now = GetWorld()->GetTimeSeconds();
    if (Now - LastAttackTime >= CombatParams.AttackCooldown)
    {
        ExecuteAttack();
        LastAttackTime = Now;
    }
}

void ADinosaurCombatAIController::HandleRetreatState(float DeltaTime)
{
    ReturnToTerritory();
    float DistFromHome = FVector::Dist(GetPawn()->GetActorLocation(), HomeLocation);
    if (DistFromHome < 500.0f)
    {
        SetCombatState(ECombat_DinoState::Idle);
    }
}

// ─── Perception ────────────────────────────────────────────────────────────

void ADinosaurCombatAIController::OnTargetPerceived(AActor* Actor, FAIStimulus Stimulus)
{
    if (!Actor || Actor == GetPawn()) return;

    // Only react to player characters
    ACharacter* PlayerChar = Cast<ACharacter>(Actor);
    if (!PlayerChar) return;

    if (Stimulus.WasSuccessfullySensed())
    {
        // Detected — transition to chase
        if (CurrentState == ECombat_DinoState::Patrolling ||
            CurrentState == ECombat_DinoState::Idle ||
            CurrentState == ECombat_DinoState::Investigating)
        {
            TransitionToChase(Actor);
        }
    }
    else
    {
        // Lost sight — record last known position
        if (CurrentTarget == Actor)
        {
            RecordThreatMemory(Actor);
            SetCombatState(ECombat_DinoState::Investigating);
        }
    }
}

float ADinosaurCombatAIController::GetDistanceToTarget() const
{
    if (!CurrentTarget || !GetPawn()) return TNumericLimits<float>::Max();
    return FVector::Dist(GetPawn()->GetActorLocation(), CurrentTarget->GetActorLocation());
}

bool ADinosaurCombatAIController::IsTargetInAttackRange() const
{
    return GetDistanceToTarget() <= CombatParams.AttackRange;
}

// ─── Transitions ───────────────────────────────────────────────────────────

void ADinosaurCombatAIController::TransitionToChase(AActor* Target)
{
    CurrentTarget = Target;
    float Dist = GetDistanceToTarget();

    // Stalk first if far, charge if close
    if (Dist > CombatParams.ChaseActivationRange * 0.5f)
    {
        SetCombatState(ECombat_DinoState::Stalking);
    }
    else
    {
        SetCombatState(ECombat_DinoState::Charging);
    }
}

void ADinosaurCombatAIController::TransitionToPatrol()
{
    CurrentTarget = nullptr;
    SetCombatState(ECombat_DinoState::Patrolling);
}

// ─── Pack Hunting ──────────────────────────────────────────────────────────

void ADinosaurCombatAIController::SignalPackMembers(AActor* SharedTarget)
{
    if (!SharedTarget || !GetWorld()) return;

    // Find nearby dinosaurs of same class
    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), GetPawn()->GetClass(), NearbyActors);

    int32 FlankIndex = 0;
    for (AActor* NearbyActor : NearbyActors)
    {
        if (NearbyActor == GetPawn()) continue;

        float Dist = FVector::Dist(GetPawn()->GetActorLocation(), NearbyActor->GetActorLocation());
        if (Dist > CombatParams.PackSignalRadius) continue;

        ADinosaurCombatAIController* PackMember = Cast<ADinosaurCombatAIController>(
            Cast<APawn>(NearbyActor) ? Cast<APawn>(NearbyActor)->GetController() : nullptr);

        if (PackMember)
        {
            FVector FlankPos = CalculateFlankPosition(SharedTarget, FlankIndex);
            PackMember->ReceivePackSignal(SharedTarget, FlankPos);
            FlankIndex++;
        }
    }
}

void ADinosaurCombatAIController::ReceivePackSignal(AActor* SharedTarget, FVector FlankPosition)
{
    if (!SharedTarget) return;
    CurrentTarget = SharedTarget;
    // Move to assigned flank position first, then engage
    MoveToLocation(FlankPosition, 150.0f);
    SetCombatState(ECombat_DinoState::Stalking);
}

FVector ADinosaurCombatAIController::CalculateFlankPosition(AActor* Target, int32 FlankIndex) const
{
    if (!Target) return FVector::ZeroVector;

    FVector TargetLoc = Target->GetActorLocation();
    float AngleDeg = 90.0f + (FlankIndex * 60.0f); // 90°, 150°, 210°...
    float AngleRad = FMath::DegreesToRadians(AngleDeg);
    float FlankDist = 600.0f;

    return TargetLoc + FVector(FMath::Cos(AngleRad) * FlankDist, FMath::Sin(AngleRad) * FlankDist, 0.0f);
}

// ─── Attack ────────────────────────────────────────────────────────────────

void ADinosaurCombatAIController::ExecuteAttack()
{
    if (!CurrentTarget) return;

    if (IsTargetInAttackRange())
    {
        ApplyDamageToTarget(CurrentTarget, CombatParams.AttackDamage);
    }
}

void ADinosaurCombatAIController::ApplyDamageToTarget(AActor* Target, float Damage)
{
    if (!Target || !GetWorld()) return;

    UGameplayStatics::ApplyDamage(
        Target,
        Damage,
        this,
        GetPawn(),
        UDamageType::StaticClass()
    );
}

// ─── Patrol ────────────────────────────────────────────────────────────────

FVector ADinosaurCombatAIController::GetNextPatrolPoint() const
{
    UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
    if (!NavSys) return HomeLocation;

    FNavLocation RandomPoint;
    bool bFound = NavSys->GetRandomReachablePointInRadius(HomeLocation, CombatParams.PatrolRadius, RandomPoint);
    return bFound ? RandomPoint.Location : HomeLocation;
}

void ADinosaurCombatAIController::ReturnToTerritory()
{
    MoveToLocation(HomeLocation, 300.0f);
}

// ─── Threat Memory ─────────────────────────────────────────────────────────

void ADinosaurCombatAIController::RecordThreatMemory(AActor* ThreatActor)
{
    if (!ThreatActor || !GetWorld()) return;

    ThreatMemory.LastKnownLocation = ThreatActor->GetActorLocation();
    ThreatMemory.ThreatLevel = 1.0f;
    ThreatMemory.Timestamp = GetWorld()->GetTimeSeconds();
    ThreatMemory.bIsActive = true;
}

// ─── Species Configuration ─────────────────────────────────────────────────

void ADinosaurCombatAIController::ConfigureForSpecies(ECombat_DinoSpecies Species)
{
    switch (Species)
    {
        case ECombat_DinoSpecies::TRex:
            CombatParams.PatrolRadius = 5000.0f;
            CombatParams.ChaseActivationRange = 3000.0f;
            CombatParams.AttackRange = 350.0f;
            CombatParams.HearingRadius = 2000.0f;
            CombatParams.AttackDamage = 80.0f;
            CombatParams.AttackCooldown = 3.0f;
            CombatParams.MoveSpeed_Patrol = 180.0f;
            CombatParams.MoveSpeed_Chase = 550.0f;
            CombatParams.bIsPackHunter = false;
            break;

        case ECombat_DinoSpecies::Velociraptor:
            CombatParams.PatrolRadius = 3000.0f;
            CombatParams.ChaseActivationRange = 2500.0f;
            CombatParams.AttackRange = 200.0f;
            CombatParams.HearingRadius = 1200.0f;
            CombatParams.AttackDamage = 25.0f;
            CombatParams.AttackCooldown = 1.2f;
            CombatParams.MoveSpeed_Patrol = 300.0f;
            CombatParams.MoveSpeed_Chase = 750.0f;
            CombatParams.bIsPackHunter = true;
            CombatParams.PackSignalRadius = 2000.0f;
            break;

        case ECombat_DinoSpecies::Triceratops:
            CombatParams.PatrolRadius = 2000.0f;
            CombatParams.ChaseActivationRange = 1500.0f;
            CombatParams.AttackRange = 400.0f;
            CombatParams.HearingRadius = 800.0f;
            CombatParams.AttackDamage = 60.0f;
            CombatParams.AttackCooldown = 4.0f;
            CombatParams.MoveSpeed_Patrol = 150.0f;
            CombatParams.MoveSpeed_Chase = 400.0f;
            CombatParams.bIsPackHunter = false;
            break;

        case ECombat_DinoSpecies::Brachiosaurus:
            CombatParams.PatrolRadius = 4000.0f;
            CombatParams.ChaseActivationRange = 800.0f;
            CombatParams.AttackRange = 500.0f;
            CombatParams.HearingRadius = 1000.0f;
            CombatParams.AttackDamage = 100.0f;
            CombatParams.AttackCooldown = 5.0f;
            CombatParams.MoveSpeed_Patrol = 120.0f;
            CombatParams.MoveSpeed_Chase = 280.0f;
            CombatParams.bIsPackHunter = false;
            break;

        case ECombat_DinoSpecies::Pterodactyl:
            CombatParams.PatrolRadius = 8000.0f;
            CombatParams.ChaseActivationRange = 4000.0f;
            CombatParams.AttackRange = 250.0f;
            CombatParams.HearingRadius = 500.0f;
            CombatParams.AttackDamage = 30.0f;
            CombatParams.AttackCooldown = 2.0f;
            CombatParams.MoveSpeed_Patrol = 400.0f;
            CombatParams.MoveSpeed_Chase = 900.0f;
            CombatParams.bIsPackHunter = false;
            break;

        default:
            break;
    }
}
