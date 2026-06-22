// DinosaurCombatAIController.cpp
// Combat & Enemy AI Agent #12 — PROD_CYCLE_AUTO_20260622_006
// Full implementation of dinosaur combat AI state machine.

#include "DinosaurCombatAIController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "NavigationSystem.h"
#include "Navigation/PathFollowingComponent.h"

ADinosaurCombatAIController::ADinosaurCombatAIController()
{
    PrimaryActorTick.bCanEverTick = true;
    bAttachToPawn = true;
}

void ADinosaurCombatAIController::BeginPlay()
{
    Super::BeginPlay();
    ApplySpeciesPreset(DinoSpecies);
    SetCombatState(ECombat_DinoAIState::Idle);
}

void ADinosaurCombatAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    UpdateCombatAI(DeltaTime);
}

void ADinosaurCombatAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
    ApplySpeciesPreset(DinoSpecies);
    SetCombatState(ECombat_DinoAIState::Patrol);
}

void ADinosaurCombatAIController::OnUnPossess()
{
    Super::OnUnPossess();
    StopMovement();
}

// ── State Machine ──────────────────────────────────────────────────────────

void ADinosaurCombatAIController::SetCombatState(ECombat_DinoAIState NewState)
{
    if (CurrentCombatState == NewState) return;
    CurrentCombatState = NewState;

    switch (NewState)
    {
        case ECombat_DinoAIState::Chase:
            SetMovementSpeed(DinoStats.ChaseSpeed);
            break;
        case ECombat_DinoAIState::Flee:
            SetMovementSpeed(DinoStats.FleeSpeed);
            break;
        case ECombat_DinoAIState::Patrol:
        case ECombat_DinoAIState::Idle:
            SetMovementSpeed(DinoStats.WalkSpeed);
            break;
        default:
            break;
    }
}

void ADinosaurCombatAIController::UpdateCombatAI(float DeltaTime)
{
    if (CurrentCombatState == ECombat_DinoAIState::Dead) return;

    TimeSinceLastAttack += DeltaTime;
    TimeSinceLastRoar += DeltaTime;

    switch (CurrentCombatState)
    {
        case ECombat_DinoAIState::Idle:    State_Idle(DeltaTime);    break;
        case ECombat_DinoAIState::Patrol:  State_Patrol(DeltaTime);  break;
        case ECombat_DinoAIState::Alert:   State_Alert(DeltaTime);   break;
        case ECombat_DinoAIState::Chase:   State_Chase(DeltaTime);   break;
        case ECombat_DinoAIState::Attack:  State_Attack(DeltaTime);  break;
        case ECombat_DinoAIState::Flee:    State_Flee(DeltaTime);    break;
        case ECombat_DinoAIState::Roar:    State_Roar(DeltaTime);    break;
        default: break;
    }
}

// ── Individual States ──────────────────────────────────────────────────────

void ADinosaurCombatAIController::State_Idle(float DeltaTime)
{
    AActor* Player = ScanForPlayer();
    if (Player)
    {
        CurrentTarget = Player;
        SetCombatState(ECombat_DinoAIState::Alert);
        return;
    }
    // After 3s idle, start patrol
    PatrolWaitTimer += DeltaTime;
    if (PatrolWaitTimer > 3.0f)
    {
        PatrolWaitTimer = 0.0f;
        SetCombatState(ECombat_DinoAIState::Patrol);
    }
}

void ADinosaurCombatAIController::State_Patrol(float DeltaTime)
{
    AActor* Player = ScanForPlayer();
    if (Player)
    {
        CurrentTarget = Player;
        // Roar before chasing if cooldown allows
        if (TimeSinceLastRoar >= DinoStats.RoarCooldown)
        {
            SetCombatState(ECombat_DinoAIState::Roar);
        }
        else
        {
            SetCombatState(ECombat_DinoAIState::Alert);
        }
        return;
    }

    // Move to next patrol point if we've arrived or not moving
    if (bWaitingAtPatrolPoint)
    {
        PatrolWaitTimer += DeltaTime;
        if (PatrolWaitTimer > 2.0f)
        {
            bWaitingAtPatrolPoint = false;
            PatrolWaitTimer = 0.0f;
            MoveToNextPatrolPoint();
        }
        return;
    }

    EPathFollowingStatus::Type MoveStatus = GetMoveStatus();
    if (MoveStatus == EPathFollowingStatus::Idle || MoveStatus == EPathFollowingStatus::Waiting)
    {
        bWaitingAtPatrolPoint = true;
    }
}

void ADinosaurCombatAIController::State_Alert(float DeltaTime)
{
    if (!CurrentTarget)
    {
        SetCombatState(ECombat_DinoAIState::Patrol);
        return;
    }

    float Dist = GetDistanceToTarget(CurrentTarget);

    if (Dist <= AttackData.AttackRange)
    {
        SetCombatState(ECombat_DinoAIState::Attack);
    }
    else if (Dist <= DinoStats.ChaseRange)
    {
        SetCombatState(ECombat_DinoAIState::Chase);
    }
    else
    {
        // Lost target
        CurrentTarget = nullptr;
        SetCombatState(ECombat_DinoAIState::Patrol);
    }
}

void ADinosaurCombatAIController::State_Chase(float DeltaTime)
{
    if (!CurrentTarget)
    {
        SetCombatState(ECombat_DinoAIState::Patrol);
        return;
    }

    float Dist = GetDistanceToTarget(CurrentTarget);

    if (Dist > DinoStats.ChaseRange * 1.2f)
    {
        // Lost target — give up chase
        CurrentTarget = nullptr;
        SetCombatState(ECombat_DinoAIState::Patrol);
        return;
    }

    if (Dist <= AttackData.AttackRange)
    {
        SetCombatState(ECombat_DinoAIState::Attack);
        return;
    }

    // Keep chasing
    MoveToActor(CurrentTarget, AttackData.AttackRange * 0.8f);
}

void ADinosaurCombatAIController::State_Attack(float DeltaTime)
{
    if (!CurrentTarget)
    {
        SetCombatState(ECombat_DinoAIState::Patrol);
        return;
    }

    float Dist = GetDistanceToTarget(CurrentTarget);

    if (Dist > AttackData.AttackRange * 1.5f)
    {
        // Target escaped attack range — chase again
        SetCombatState(ECombat_DinoAIState::Chase);
        return;
    }

    if (CanAttackTarget(CurrentTarget))
    {
        ExecuteAttack(CurrentTarget);
    }
}

void ADinosaurCombatAIController::State_Flee(float DeltaTime)
{
    // Flee away from threat source
    if (!CurrentTarget) return;

    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn) return;

    FVector MyLoc = ControlledPawn->GetActorLocation();
    FVector ThreatLoc = CurrentTarget->GetActorLocation();
    FVector FleeDir = (MyLoc - ThreatLoc).GetSafeNormal();
    FVector FleeTarget = MyLoc + FleeDir * 3000.0f;

    MoveToLocation(FleeTarget, 100.0f);

    // Stop fleeing after 8 seconds
    PatrolWaitTimer += DeltaTime;
    if (PatrolWaitTimer > 8.0f)
    {
        PatrolWaitTimer = 0.0f;
        CurrentTarget = nullptr;
        SetCombatState(ECombat_DinoAIState::Patrol);
    }
}

void ADinosaurCombatAIController::State_Roar(float DeltaTime)
{
    RoarTimer += DeltaTime;
    bIsRoaring = true;

    // Alert pack members during roar
    if (bPackHunter)
    {
        AlertNearbyPackMembers();
    }

    // Roar lasts 2 seconds then transition to chase
    if (RoarTimer >= 2.0f)
    {
        RoarTimer = 0.0f;
        bIsRoaring = false;
        TimeSinceLastRoar = 0.0f;
        SetCombatState(ECombat_DinoAIState::Chase);
    }
}

// ── Attack Logic ───────────────────────────────────────────────────────────

bool ADinosaurCombatAIController::CanAttackTarget(AActor* Target) const
{
    if (!Target) return false;
    if (TimeSinceLastAttack < AttackData.AttackCooldown) return false;
    return GetDistanceToTarget(Target) <= AttackData.AttackRange;
}

void ADinosaurCombatAIController::ExecuteAttack(AActor* Target)
{
    if (!Target) return;

    TimeSinceLastAttack = 0.0f;

    // Apply damage via UE5 damage system
    UGameplayStatics::ApplyDamage(
        Target,
        AttackData.Damage,
        this,
        GetPawn(),
        nullptr
    );

    // Knockback — push target away from attacker
    ACharacter* TargetChar = Cast<ACharacter>(Target);
    if (TargetChar && TargetChar->GetCharacterMovement())
    {
        APawn* MyPawn = GetPawn();
        if (MyPawn)
        {
            FVector KnockDir = (Target->GetActorLocation() - MyPawn->GetActorLocation()).GetSafeNormal();
            KnockDir.Z = 0.4f; // slight upward component
            TargetChar->GetCharacterMovement()->AddImpulse(KnockDir * AttackData.KnockbackForce, true);
        }
    }
}

void ADinosaurCombatAIController::TriggerRoar()
{
    if (TimeSinceLastRoar >= DinoStats.RoarCooldown)
    {
        SetCombatState(ECombat_DinoAIState::Roar);
    }
}

void ADinosaurCombatAIController::TakeDamage_Combat(float DamageAmount, AActor* DamageSource)
{
    DinoStats.CurrentHealth = FMath::Max(0.0f, DinoStats.CurrentHealth - DamageAmount);

    if (DinoStats.CurrentHealth <= 0.0f)
    {
        SetCombatState(ECombat_DinoAIState::Dead);
        StopMovement();
        return;
    }

    // Low health — flee if health < 25%
    float HealthPct = DinoStats.CurrentHealth / DinoStats.MaxHealth;
    if (HealthPct < 0.25f && CurrentCombatState != ECombat_DinoAIState::Flee)
    {
        CurrentTarget = DamageSource;
        SetCombatState(ECombat_DinoAIState::Flee);
        return;
    }

    // Otherwise fight back
    if (DamageSource && CurrentCombatState != ECombat_DinoAIState::Attack)
    {
        CurrentTarget = DamageSource;
        SetCombatState(ECombat_DinoAIState::Chase);
    }
}

// ── Perception ─────────────────────────────────────────────────────────────

AActor* ADinosaurCombatAIController::ScanForPlayer() const
{
    UWorld* World = GetWorld();
    if (!World) return nullptr;

    APawn* MyPawn = GetPawn();
    if (!MyPawn) return nullptr;

    APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
    if (!PC) return nullptr;

    APawn* PlayerPawn = PC->GetPawn();
    if (!PlayerPawn) return nullptr;

    float Dist = FVector::Dist(MyPawn->GetActorLocation(), PlayerPawn->GetActorLocation());
    if (Dist <= DinoStats.SightRange)
    {
        return PlayerPawn;
    }

    return nullptr;
}

float ADinosaurCombatAIController::GetDistanceToTarget(AActor* Target) const
{
    APawn* MyPawn = GetPawn();
    if (!MyPawn || !Target) return TNumericLimits<float>::Max();
    return FVector::Dist(MyPawn->GetActorLocation(), Target->GetActorLocation());
}

void ADinosaurCombatAIController::SetPatrolPoints(const TArray<AActor*>& Points)
{
    PatrolPoints = Points;
    CurrentPatrolIndex = 0;
    MoveToNextPatrolPoint();
}

// ── Species Presets ────────────────────────────────────────────────────────

void ADinosaurCombatAIController::ApplySpeciesPreset(ECombat_DinoSpecies Species)
{
    switch (Species)
    {
        case ECombat_DinoSpecies::TyrannosaurusRex:
            DinoStats.MaxHealth       = 800.0f;
            DinoStats.CurrentHealth   = 800.0f;
            DinoStats.WalkSpeed       = 350.0f;
            DinoStats.ChaseSpeed      = 850.0f;
            DinoStats.FleeSpeed       = 950.0f;
            DinoStats.ChaseRange      = 3000.0f;
            DinoStats.SightRange      = 4500.0f;
            DinoStats.HearingRange    = 2500.0f;
            DinoStats.RoarCooldown    = 20.0f;
            AttackData.Damage         = 75.0f;
            AttackData.AttackRange    = 350.0f;
            AttackData.AttackCooldown = 3.0f;
            AttackData.KnockbackForce = 1200.0f;
            AttackData.bIsAreaAttack  = false;
            bPackHunter               = false;
            break;

        case ECombat_DinoSpecies::Velociraptor:
            DinoStats.MaxHealth       = 150.0f;
            DinoStats.CurrentHealth   = 150.0f;
            DinoStats.WalkSpeed       = 500.0f;
            DinoStats.ChaseSpeed      = 1200.0f;
            DinoStats.FleeSpeed       = 1400.0f;
            DinoStats.ChaseRange      = 2500.0f;
            DinoStats.SightRange      = 3000.0f;
            DinoStats.HearingRange    = 1800.0f;
            DinoStats.RoarCooldown    = 10.0f;
            AttackData.Damage         = 30.0f;
            AttackData.AttackRange    = 200.0f;
            AttackData.AttackCooldown = 1.2f;
            AttackData.KnockbackForce = 400.0f;
            AttackData.bIsAreaAttack  = false;
            bPackHunter               = true;
            PackAlertRadius           = 2000.0f;
            break;

        case ECombat_DinoSpecies::Brachiosaurus:
            DinoStats.MaxHealth       = 2000.0f;
            DinoStats.CurrentHealth   = 2000.0f;
            DinoStats.WalkSpeed       = 250.0f;
            DinoStats.ChaseSpeed      = 400.0f;
            DinoStats.FleeSpeed       = 500.0f;
            DinoStats.ChaseRange      = 1000.0f;
            DinoStats.SightRange      = 2000.0f;
            DinoStats.HearingRange    = 1500.0f;
            DinoStats.RoarCooldown    = 30.0f;
            AttackData.Damage         = 50.0f;
            AttackData.AttackRange    = 500.0f;
            AttackData.AttackCooldown = 4.0f;
            AttackData.KnockbackForce = 1500.0f;
            AttackData.bIsAreaAttack  = true;
            AttackData.AreaAttackRadius = 300.0f;
            bPackHunter               = false;
            break;

        case ECombat_DinoSpecies::Triceratops:
            DinoStats.MaxHealth       = 600.0f;
            DinoStats.CurrentHealth   = 600.0f;
            DinoStats.WalkSpeed       = 300.0f;
            DinoStats.ChaseSpeed      = 700.0f;
            DinoStats.FleeSpeed       = 750.0f;
            DinoStats.ChaseRange      = 1500.0f;
            DinoStats.SightRange      = 2500.0f;
            DinoStats.HearingRange    = 1200.0f;
            DinoStats.RoarCooldown    = 15.0f;
            AttackData.Damage         = 60.0f;
            AttackData.AttackRange    = 400.0f;
            AttackData.AttackCooldown = 2.5f;
            AttackData.KnockbackForce = 1800.0f;
            AttackData.bIsAreaAttack  = false;
            bPackHunter               = false;
            break;

        default:
            // Generic defaults already set in struct
            break;
    }
}

// ── Private Helpers ────────────────────────────────────────────────────────

void ADinosaurCombatAIController::AlertNearbyPackMembers()
{
    UWorld* World = GetWorld();
    if (!World) return;

    APawn* MyPawn = GetPawn();
    if (!MyPawn) return;

    // Find all other DinosaurCombatAIControllers within pack alert radius
    TArray<AActor*> FoundControllers;
    UGameplayStatics::GetAllActorsOfClass(World, ADinosaurCombatAIController::StaticClass(), FoundControllers);

    for (AActor* CtrlActor : FoundControllers)
    {
        ADinosaurCombatAIController* PackMember = Cast<ADinosaurCombatAIController>(CtrlActor);
        if (!PackMember || PackMember == this) continue;

        float Dist = FVector::Dist(MyPawn->GetActorLocation(), PackMember->GetPawn() ? PackMember->GetPawn()->GetActorLocation() : FVector::ZeroVector);
        if (Dist <= PackAlertRadius && CurrentTarget)
        {
            PackMember->CurrentTarget = CurrentTarget;
            if (PackMember->CurrentCombatState == ECombat_DinoAIState::Idle ||
                PackMember->CurrentCombatState == ECombat_DinoAIState::Patrol)
            {
                PackMember->SetCombatState(ECombat_DinoAIState::Chase);
            }
        }
    }
}

void ADinosaurCombatAIController::SetMovementSpeed(float Speed)
{
    APawn* MyPawn = GetPawn();
    if (!MyPawn) return;

    ACharacter* MyChar = Cast<ACharacter>(MyPawn);
    if (MyChar && MyChar->GetCharacterMovement())
    {
        MyChar->GetCharacterMovement()->MaxWalkSpeed = Speed;
    }
}

void ADinosaurCombatAIController::MoveToNextPatrolPoint()
{
    if (PatrolPoints.Num() == 0) return;

    CurrentPatrolIndex = (CurrentPatrolIndex + 1) % PatrolPoints.Num();
    AActor* NextPoint = PatrolPoints[CurrentPatrolIndex];
    if (NextPoint)
    {
        MoveToActor(NextPoint, 100.0f);
    }
}
