#include "DinosaurCombatAIController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "NavigationSystem.h"
#include "Navigation/PathFollowingComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

ADinosaurCombatAIController::ADinosaurCombatAIController()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.05f; // 20Hz AI tick
}

void ADinosaurCombatAIController::BeginPlay()
{
    Super::BeginPlay();
    ApplySpeciesDefaults();
    CurrentHealth = MaxHealth;
    CurrentPhase = ECombat_TacticalPhase::Idle;
}

void ADinosaurCombatAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
    ControlledPawn = InPawn;

    // Auto-generate patrol points if none provided
    if (PatrolPoints.Num() == 0 && InPawn)
    {
        FVector Origin = InPawn->GetActorLocation();
        for (int32 i = 0; i < 4; ++i)
        {
            float Angle = (float)i * 90.0f;
            FCombat_PatrolPoint PP;
            PP.Location = Origin + FVector(
                FMath::Cos(FMath::DegreesToRadians(Angle)) * PatrolRadius,
                FMath::Sin(FMath::DegreesToRadians(Angle)) * PatrolRadius,
                0.0f
            );
            PP.WaitTime = 2.0f;
            PatrolPoints.Add(PP);
        }
    }

    SetTacticalPhase(ECombat_TacticalPhase::Patrol);
}

void ADinosaurCombatAIController::OnUnPossess()
{
    Super::OnUnPossess();
    ControlledPawn = nullptr;
    CurrentTarget = nullptr;
}

void ADinosaurCombatAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!ControlledPawn || CurrentPhase == ECombat_TacticalPhase::Dead)
        return;

    AttackCooldownRemaining = FMath::Max(0.0f, AttackCooldownRemaining - DeltaTime);
    PhaseTimer += DeltaTime;

    // Periodic scan for player (every 0.1s)
    ScanTimer += DeltaTime;
    if (ScanTimer >= 0.1f)
    {
        ScanTimer = 0.0f;
        AActor* Found = ScanForPlayer();
        if (Found && CurrentPhase != ECombat_TacticalPhase::Flee)
        {
            CurrentTarget = Found;
            if (CurrentPhase == ECombat_TacticalPhase::Patrol ||
                CurrentPhase == ECombat_TacticalPhase::Idle)
            {
                SetTacticalPhase(ECombat_TacticalPhase::Alert);
                AlertPackMembers(Found);
            }
        }
        else if (!Found && CurrentTarget)
        {
            float Dist = GetDistanceToTarget();
            if (Dist > LoseTargetRange)
            {
                CurrentTarget = nullptr;
                SetTacticalPhase(ECombat_TacticalPhase::Patrol);
            }
        }
    }

    // Dispatch to phase tick
    switch (CurrentPhase)
    {
        case ECombat_TacticalPhase::Idle:    TickIdle(DeltaTime);    break;
        case ECombat_TacticalPhase::Patrol:  TickPatrol(DeltaTime);  break;
        case ECombat_TacticalPhase::Alert:   TickAlert(DeltaTime);   break;
        case ECombat_TacticalPhase::Stalk:   TickStalk(DeltaTime);   break;
        case ECombat_TacticalPhase::Charge:  TickCharge(DeltaTime);  break;
        case ECombat_TacticalPhase::Attack:  TickAttack(DeltaTime);  break;
        case ECombat_TacticalPhase::Recover: TickRecover(DeltaTime); break;
        case ECombat_TacticalPhase::Flee:    TickFlee(DeltaTime);    break;
        default: break;
    }
}

// ── Phase Transitions ────────────────────────────────────────────────────────

void ADinosaurCombatAIController::SetTacticalPhase(ECombat_TacticalPhase NewPhase)
{
    if (CurrentPhase == NewPhase) return;
    CurrentPhase = NewPhase;
    PhaseTimer = 0.0f;

    // Adjust movement speed per phase
    ACharacter* Char = Cast<ACharacter>(ControlledPawn);
    if (!Char) return;

    UCharacterMovementComponent* Move = Char->GetCharacterMovement();
    if (!Move) return;

    switch (NewPhase)
    {
        case ECombat_TacticalPhase::Patrol:
            Move->MaxWalkSpeed = 200.0f;
            break;
        case ECombat_TacticalPhase::Alert:
        case ECombat_TacticalPhase::Stalk:
            Move->MaxWalkSpeed = 350.0f;
            break;
        case ECombat_TacticalPhase::Charge:
            Move->MaxWalkSpeed = AttackData.ChargeSpeed;
            break;
        case ECombat_TacticalPhase::Attack:
            Move->MaxWalkSpeed = 600.0f;
            break;
        case ECombat_TacticalPhase::Flee:
            Move->MaxWalkSpeed = 700.0f;
            break;
        case ECombat_TacticalPhase::Recover:
            Move->MaxWalkSpeed = 100.0f;
            break;
        default:
            Move->MaxWalkSpeed = 150.0f;
            break;
    }
}

// ── Phase Tick Implementations ───────────────────────────────────────────────

void ADinosaurCombatAIController::TickIdle(float DeltaTime)
{
    // After 5 seconds idle, start patrol
    if (PhaseTimer > 5.0f)
    {
        SetTacticalPhase(ECombat_TacticalPhase::Patrol);
    }
}

void ADinosaurCombatAIController::TickPatrol(float DeltaTime)
{
    if (PatrolPoints.Num() == 0) return;

    PatrolWaitRemaining -= DeltaTime;
    if (PatrolWaitRemaining > 0.0f) return;

    MoveToPatrolPoint();

    // Check if reached patrol point
    if (GetMoveStatus() == EPathFollowingStatus::Idle)
    {
        PatrolWaitRemaining = PatrolPoints[CurrentPatrolIndex].WaitTime;
        AdvancePatrolIndex();
    }
}

void ADinosaurCombatAIController::TickAlert(float DeltaTime)
{
    if (!CurrentTarget)
    {
        SetTacticalPhase(ECombat_TacticalPhase::Patrol);
        return;
    }

    float Dist = GetDistanceToTarget();

    // Transition to charge if close enough
    if (Dist <= AlertRange)
    {
        SetTacticalPhase(ECombat_TacticalPhase::Charge);
        return;
    }

    // Move toward target cautiously
    MoveToTarget(AlertRange * 0.8f);
}

void ADinosaurCombatAIController::TickStalk(float DeltaTime)
{
    if (!CurrentTarget)
    {
        SetTacticalPhase(ECombat_TacticalPhase::Patrol);
        return;
    }

    float Dist = GetDistanceToTarget();

    if (Dist <= AttackData.AttackRange * 2.0f)
    {
        SetTacticalPhase(ECombat_TacticalPhase::Charge);
        return;
    }

    // Stalk: move to flanking position
    FVector TargetLoc = CurrentTarget->GetActorLocation();
    FVector MyLoc = ControlledPawn->GetActorLocation();
    FVector ToTarget = (TargetLoc - MyLoc).GetSafeNormal();
    FVector Perp = FVector::CrossProduct(ToTarget, FVector::UpVector);
    FVector FlankPos = TargetLoc + Perp * 600.0f;

    MoveToLocation(FlankPos, 150.0f);
}

void ADinosaurCombatAIController::TickCharge(float DeltaTime)
{
    if (!CurrentTarget)
    {
        SetTacticalPhase(ECombat_TacticalPhase::Patrol);
        return;
    }

    float Dist = GetDistanceToTarget();

    if (Dist <= AttackData.AttackRange)
    {
        SetTacticalPhase(ECombat_TacticalPhase::Attack);
        return;
    }

    MoveToTarget(AttackData.AttackRange * 0.9f);
}

void ADinosaurCombatAIController::TickAttack(float DeltaTime)
{
    if (!CurrentTarget)
    {
        SetTacticalPhase(ECombat_TacticalPhase::Recover);
        return;
    }

    float Dist = GetDistanceToTarget();

    if (Dist > AttackData.AttackRange * 1.5f)
    {
        SetTacticalPhase(ECombat_TacticalPhase::Charge);
        return;
    }

    if (AttackCooldownRemaining <= 0.0f)
    {
        ExecuteAttack();
        AttackCooldownRemaining = AttackData.AttackCooldown;

        // After attack, brief recover
        if (PhaseTimer > 3.0f)
        {
            SetTacticalPhase(ECombat_TacticalPhase::Recover);
        }
    }
}

void ADinosaurCombatAIController::TickRecover(float DeltaTime)
{
    // Short recovery window — then re-engage or flee
    if (PhaseTimer > 2.0f)
    {
        if (CurrentHealth / MaxHealth <= FleeHealthThreshold)
        {
            SetTacticalPhase(ECombat_TacticalPhase::Flee);
        }
        else if (CurrentTarget)
        {
            SetTacticalPhase(ECombat_TacticalPhase::Charge);
        }
        else
        {
            SetTacticalPhase(ECombat_TacticalPhase::Patrol);
        }
    }
}

void ADinosaurCombatAIController::TickFlee(float DeltaTime)
{
    FVector FleeDir = GetFleeDirection();
    FVector FleeTarget = ControlledPawn->GetActorLocation() + FleeDir * 3000.0f;
    MoveToLocation(FleeTarget, 200.0f);

    // After 10 seconds of fleeing, stop
    if (PhaseTimer > 10.0f)
    {
        CurrentTarget = nullptr;
        SetTacticalPhase(ECombat_TacticalPhase::Idle);
    }
}

// ── Combat Functions ─────────────────────────────────────────────────────────

void ADinosaurCombatAIController::TakeCombatDamage(float DamageAmount, AActor* DamageSource)
{
    CurrentHealth = FMath::Max(0.0f, CurrentHealth - DamageAmount);

    if (CurrentHealth <= 0.0f)
    {
        SetTacticalPhase(ECombat_TacticalPhase::Dead);
        StopMovement();
        return;
    }

    if (CurrentHealth / MaxHealth <= FleeHealthThreshold)
    {
        CurrentTarget = DamageSource;
        SetTacticalPhase(ECombat_TacticalPhase::Flee);
        return;
    }

    // If hit, become aggressive
    if (DamageSource && CurrentPhase == ECombat_TacticalPhase::Patrol)
    {
        CurrentTarget = DamageSource;
        SetTacticalPhase(ECombat_TacticalPhase::Charge);
        AlertPackMembers(DamageSource);
    }
}

void ADinosaurCombatAIController::AlertPackMembers(AActor* ThreatActor)
{
    if (!bIsPackHunter || !ControlledPawn) return;

    UWorld* World = GetWorld();
    if (!World) return;

    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, ADinosaurCombatAIController::StaticClass(), AllActors);

    FVector MyLoc = ControlledPawn->GetActorLocation();
    int32 AlertCount = 0;

    for (AActor* Actor : AllActors)
    {
        if (Actor == this) continue;
        ADinosaurCombatAIController* PackMember = Cast<ADinosaurCombatAIController>(Actor);
        if (!PackMember) continue;
        if (PackMember->Species != Species) continue;

        float Dist = FVector::Dist(MyLoc, Actor->GetActorLocation());
        if (Dist <= PackCoordinationRadius)
        {
            PackMember->CurrentTarget = ThreatActor;
            PackMember->SetTacticalPhase(ECombat_TacticalPhase::Stalk);
            AlertCount++;
            if (AlertCount >= PackSize - 1) break;
        }
    }
}

void ADinosaurCombatAIController::SetPatrolPoints(const TArray<FCombat_PatrolPoint>& NewPoints)
{
    PatrolPoints = NewPoints;
    CurrentPatrolIndex = 0;
}

// ── Helper Implementations ───────────────────────────────────────────────────

AActor* ADinosaurCombatAIController::ScanForPlayer()
{
    if (!ControlledPawn) return nullptr;

    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PC) return nullptr;

    APawn* PlayerPawn = PC->GetPawn();
    if (!PlayerPawn) return nullptr;

    float Dist = FVector::Dist(ControlledPawn->GetActorLocation(), PlayerPawn->GetActorLocation());
    if (Dist <= DetectionRange)
    {
        return PlayerPawn;
    }
    return nullptr;
}

float ADinosaurCombatAIController::GetDistanceToTarget() const
{
    if (!CurrentTarget || !ControlledPawn) return TNumericLimits<float>::Max();
    return FVector::Dist(ControlledPawn->GetActorLocation(), CurrentTarget->GetActorLocation());
}

void ADinosaurCombatAIController::ExecuteAttack()
{
    if (!CurrentTarget || !ControlledPawn) return;

    float Dist = GetDistanceToTarget();
    if (Dist > AttackData.AttackRange) return;

    // Apply damage to target
    UGameplayStatics::ApplyDamage(
        CurrentTarget,
        AttackData.BaseDamage,
        this,
        ControlledPawn,
        UDamageType::StaticClass()
    );

    // Apply knockback to character
    ACharacter* TargetChar = Cast<ACharacter>(CurrentTarget);
    if (TargetChar)
    {
        FVector KnockDir = (CurrentTarget->GetActorLocation() - ControlledPawn->GetActorLocation()).GetSafeNormal();
        KnockDir.Z = 0.4f;
        TargetChar->LaunchCharacter(KnockDir * AttackData.KnockbackForce, true, true);
    }
}

void ADinosaurCombatAIController::MoveToTarget(float AcceptanceRadius)
{
    if (!CurrentTarget) return;
    MoveToActor(CurrentTarget, AcceptanceRadius, true, true, true);
}

void ADinosaurCombatAIController::MoveToPatrolPoint()
{
    if (PatrolPoints.Num() == 0) return;
    MoveToLocation(PatrolPoints[CurrentPatrolIndex].Location, 100.0f);
}

void ADinosaurCombatAIController::AdvancePatrolIndex()
{
    CurrentPatrolIndex = (CurrentPatrolIndex + 1) % PatrolPoints.Num();
}

FVector ADinosaurCombatAIController::GetFleeDirection() const
{
    if (!CurrentTarget || !ControlledPawn) return FVector::ForwardVector;
    FVector Away = (ControlledPawn->GetActorLocation() - CurrentTarget->GetActorLocation()).GetSafeNormal();
    Away.Z = 0.0f;
    return Away;
}

float ADinosaurCombatAIController::GetHealthPercent() const
{
    if (MaxHealth <= 0.0f) return 0.0f;
    return CurrentHealth / MaxHealth;
}

bool ADinosaurCombatAIController::IsHostile() const
{
    return CurrentPhase == ECombat_TacticalPhase::Charge ||
           CurrentPhase == ECombat_TacticalPhase::Attack ||
           CurrentPhase == ECombat_TacticalPhase::Stalk  ||
           CurrentPhase == ECombat_TacticalPhase::Alert;
}

void ADinosaurCombatAIController::ApplySpeciesDefaults()
{
    switch (Species)
    {
        case ECombat_DinoSpecies::TRex:
            MaxHealth = 2000.0f;
            DetectionRange = 3000.0f;
            AttackData.BaseDamage = 120.0f;
            AttackData.AttackRange = 350.0f;
            AttackData.AttackCooldown = 3.0f;
            AttackData.ChargeSpeed = 700.0f;
            AttackData.KnockbackForce = 2000.0f;
            PatrolRadius = 2500.0f;
            bIsPackHunter = false;
            break;

        case ECombat_DinoSpecies::Raptor:
            MaxHealth = 300.0f;
            DetectionRange = 2000.0f;
            AttackData.BaseDamage = 40.0f;
            AttackData.AttackRange = 200.0f;
            AttackData.AttackCooldown = 1.2f;
            AttackData.ChargeSpeed = 900.0f;
            AttackData.KnockbackForce = 600.0f;
            PatrolRadius = 1200.0f;
            bIsPackHunter = true;
            PackSize = 3;
            PackCoordinationRadius = 1500.0f;
            break;

        case ECombat_DinoSpecies::Triceratops:
            MaxHealth = 1500.0f;
            DetectionRange = 1500.0f;
            AttackData.BaseDamage = 80.0f;
            AttackData.AttackRange = 400.0f;
            AttackData.AttackCooldown = 4.0f;
            AttackData.ChargeSpeed = 650.0f;
            AttackData.KnockbackForce = 2500.0f;
            PatrolRadius = 1800.0f;
            bIsPackHunter = false;
            break;

        case ECombat_DinoSpecies::Brachiosaurus:
            MaxHealth = 5000.0f;
            DetectionRange = 800.0f;
            AttackData.BaseDamage = 200.0f;
            AttackData.AttackRange = 600.0f;
            AttackData.AttackCooldown = 6.0f;
            AttackData.ChargeSpeed = 400.0f;
            AttackData.KnockbackForce = 5000.0f;
            PatrolRadius = 3000.0f;
            bIsPackHunter = false;
            FleeHealthThreshold = 0.1f;
            break;

        case ECombat_DinoSpecies::Pterodactyl:
            MaxHealth = 200.0f;
            DetectionRange = 3500.0f;
            AttackData.BaseDamage = 25.0f;
            AttackData.AttackRange = 150.0f;
            AttackData.AttackCooldown = 0.8f;
            AttackData.ChargeSpeed = 1200.0f;
            AttackData.KnockbackForce = 300.0f;
            PatrolRadius = 5000.0f;
            bIsPackHunter = true;
            PackSize = 5;
            PackCoordinationRadius = 3000.0f;
            break;

        default:
            break;
    }

    CurrentHealth = MaxHealth;
}
