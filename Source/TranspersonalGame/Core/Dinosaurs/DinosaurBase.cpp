// DinosaurBase.cpp
// Core Systems Programmer — Agent #3
// Implements base dinosaur behaviour: patrol movement, state machine, combat, health.
// All dinosaur species (TRex, Raptor, Brachiosaurus, etc.) inherit from this class.

#include "DinosaurBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"

// ============================================================
// Constructor
// ============================================================
ADinosaurBase::ADinosaurBase()
{
    PrimaryActorTick.bCanEverTick = true;

    // Default movement settings — subclasses override via Stats
    if (UCharacterMovementComponent* Move = GetCharacterMovement())
    {
        Move->MaxWalkSpeed = Stats.WalkSpeed;
        Move->bOrientRotationToMovement = true;
        Move->RotationRate = FRotator(0.f, 360.f, 0.f);
        Move->bUseControllerDesiredRotation = false;
        Move->NavAgentProps.bCanCrouch = false;
        Move->NavAgentProps.bCanJump = false;
    }

    // No player-controlled rotation
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw   = false;
    bUseControllerRotationRoll  = false;
}

// ============================================================
// BeginPlay
// ============================================================
void ADinosaurBase::BeginPlay()
{
    Super::BeginPlay();

    // Cache AI controller (spawned automatically by GameMode for APawn subclasses)
    CachedAIController = Cast<AAIController>(GetController());

    // Apply stats to movement component
    if (UCharacterMovementComponent* Move = GetCharacterMovement())
    {
        Move->MaxWalkSpeed = Stats.WalkSpeed;
    }

    // Auto-start patrol if route is defined
    if (PatrolRoute.Num() > 0)
    {
        StartPatrol();
    }
    else
    {
        SetDinoState(ECore_DinoState::Idle);
    }
}

// ============================================================
// Tick
// ============================================================
void ADinosaurBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!IsAlive()) return;

    switch (CurrentState)
    {
        case ECore_DinoState::Patrol:
            TickPatrol(DeltaTime);
            break;
        case ECore_DinoState::Idle:
        case ECore_DinoState::Resting:
            TickHunger(DeltaTime);
            break;
        default:
            break;
    }
}

// ============================================================
// TakeDamage
// ============================================================
float ADinosaurBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
                                 AController* EventInstigator, AActor* DamageCauser)
{
    const float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    Stats.Health = FMath::Clamp(Stats.Health - ActualDamage, 0.f, Stats.MaxHealth);

    if (!IsAlive())
    {
        OnDinoDeath();
    }
    else if (CurrentState == ECore_DinoState::Idle || CurrentState == ECore_DinoState::Patrol)
    {
        // Become alert when hit
        SetDinoState(ECore_DinoState::Alert);
        if (DamageCauser)
        {
            OnTargetDetected(DamageCauser);
        }
    }

    return ActualDamage;
}

// ============================================================
// State Machine
// ============================================================
void ADinosaurBase::SetDinoState(ECore_DinoState NewState)
{
    if (NewState == CurrentState) return;

    const ECore_DinoState OldState = CurrentState;
    CurrentState = NewState;

    // Adjust movement speed based on state
    if (UCharacterMovementComponent* Move = GetCharacterMovement())
    {
        switch (NewState)
        {
            case ECore_DinoState::Chase:
            case ECore_DinoState::Attack:
            case ECore_DinoState::Flee:
                Move->MaxWalkSpeed = Stats.RunSpeed;
                break;
            default:
                Move->MaxWalkSpeed = Stats.WalkSpeed;
                break;
        }
    }

    OnDinoStateChanged(OldState, NewState);
}

// ============================================================
// Patrol
// ============================================================
void ADinosaurBase::StartPatrol()
{
    if (PatrolRoute.Num() == 0) return;

    bPatrolActive = true;
    CurrentPatrolIndex = 0;
    bWaitingAtPatrolPoint = false;
    PatrolWaitTimer = 0.f;

    SetDinoState(ECore_DinoState::Patrol);
    MoveToPatrolPoint(PatrolRoute[CurrentPatrolIndex]);
}

void ADinosaurBase::StopPatrol()
{
    bPatrolActive = false;

    if (CachedAIController)
    {
        CachedAIController->StopMovement();
    }

    SetDinoState(ECore_DinoState::Idle);
}

void ADinosaurBase::TickPatrol(float DeltaTime)
{
    if (!bPatrolActive || PatrolRoute.Num() == 0) return;

    if (bWaitingAtPatrolPoint)
    {
        PatrolWaitTimer -= DeltaTime;
        if (PatrolWaitTimer <= 0.f)
        {
            bWaitingAtPatrolPoint = false;
            AdvanceToNextPatrolPoint();
        }
        return;
    }

    // Check if we've reached the current patrol point
    if (CachedAIController)
    {
        const FVector TargetLoc = PatrolRoute[CurrentPatrolIndex].Location;
        const float DistSq = FVector::DistSquared(GetActorLocation(), TargetLoc);
        const float AcceptanceRadius = 150.f; // cm

        if (DistSq <= FMath::Square(AcceptanceRadius))
        {
            // Arrived — wait then advance
            const float WaitTime = PatrolRoute[CurrentPatrolIndex].WaitTimeSeconds;
            if (WaitTime > 0.f)
            {
                bWaitingAtPatrolPoint = true;
                PatrolWaitTimer = WaitTime;
                CachedAIController->StopMovement();
            }
            else
            {
                AdvanceToNextPatrolPoint();
            }
        }
    }
}

void ADinosaurBase::AdvanceToNextPatrolPoint()
{
    if (PatrolRoute.Num() == 0) return;

    CurrentPatrolIndex++;

    if (CurrentPatrolIndex >= PatrolRoute.Num())
    {
        if (bLoopPatrol)
        {
            CurrentPatrolIndex = 0;
        }
        else
        {
            StopPatrol();
            return;
        }
    }

    MoveToPatrolPoint(PatrolRoute[CurrentPatrolIndex]);
}

void ADinosaurBase::MoveToPatrolPoint(const FCore_PatrolPoint& Point)
{
    if (!CachedAIController) return;

    CachedAIController->MoveToLocation(
        Point.Location,
        150.f,   // AcceptanceRadius
        true,    // bStopOnOverlap
        true,    // bUsePathfinding
        false,   // bProjectDestinationToNavigation
        true,    // bCanStrafe
        nullptr, // FilterClass
        false    // bAllowPartialPath
    );
}

// ============================================================
// Hunger (passive drain during idle/rest)
// ============================================================
void ADinosaurBase::TickHunger(float DeltaTime)
{
    // Drain 1 unit/second when idle; dinosaur becomes aggressive when starving
    const float DrainRate = 1.f;
    Stats.Hunger = FMath::Clamp(Stats.Hunger - DrainRate * DeltaTime, 0.f, 100.f);
}

// ============================================================
// Combat
// ============================================================
void ADinosaurBase::ApplyMeleeDamage(AActor* Target)
{
    if (!Target || !IsAlive()) return;

    UGameplayStatics::ApplyDamage(
        Target,
        Stats.AttackDamage,
        GetController(),
        this,
        UDamageType::StaticClass()
    );
}

bool ADinosaurBase::IsTargetInAttackRange(AActor* Target) const
{
    if (!Target) return false;
    return FVector::Dist(GetActorLocation(), Target->GetActorLocation()) <= Stats.AttackRadius;
}

bool ADinosaurBase::IsTargetInDetectionRange(AActor* Target) const
{
    if (!Target) return false;
    return FVector::Dist(GetActorLocation(), Target->GetActorLocation()) <= Stats.DetectionRadius;
}

// ============================================================
// Health
// ============================================================
void ADinosaurBase::HealDinosaur(float Amount)
{
    Stats.Health = FMath::Clamp(Stats.Health + Amount, 0.f, Stats.MaxHealth);
}

float ADinosaurBase::GetHealthPercent() const
{
    if (Stats.MaxHealth <= 0.f) return 0.f;
    return Stats.Health / Stats.MaxHealth;
}

// ============================================================
// BlueprintNativeEvent implementations
// ============================================================
void ADinosaurBase::OnDinoStateChanged_Implementation(ECore_DinoState OldState, ECore_DinoState NewState)
{
    // Default: log state change. Subclasses override for animations, sounds, etc.
    UE_LOG(LogTemp, Verbose, TEXT("[DinosaurBase] %s state: %d -> %d"),
        *GetActorLabel(), (int32)OldState, (int32)NewState);
}

void ADinosaurBase::OnDinoDeath_Implementation()
{
    // Default: stop movement, disable collision, begin ragdoll timer
    StopPatrol();

    if (UCharacterMovementComponent* Move = GetCharacterMovement())
    {
        Move->DisableMovement();
    }

    SetActorEnableCollision(false);

    UE_LOG(LogTemp, Log, TEXT("[DinosaurBase] %s died."), *GetActorLabel());
}

void ADinosaurBase::OnTargetDetected_Implementation(AActor* DetectedTarget)
{
    // Default: transition to Alert state
    if (CurrentState == ECore_DinoState::Idle || CurrentState == ECore_DinoState::Patrol)
    {
        SetDinoState(ECore_DinoState::Alert);
    }

    UE_LOG(LogTemp, Log, TEXT("[DinosaurBase] %s detected target: %s"),
        *GetActorLabel(),
        DetectedTarget ? *DetectedTarget->GetActorLabel() : TEXT("None"));
}
