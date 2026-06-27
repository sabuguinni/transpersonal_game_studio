#include "DinosaurBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

ADinosaurBase::ADinosaurBase()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.0f;

    // Configure movement defaults — overridden per species in child classes
    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        MoveComp->MaxWalkSpeed = DinoStats.WalkSpeed;
        MoveComp->bOrientRotationToMovement = true;
        MoveComp->RotationRate = FRotator(0.0f, 360.0f, 0.0f);
        MoveComp->GravityScale = 1.0f;
        MoveComp->JumpZVelocity = 0.0f; // Dinosaurs don't jump by default
    }

    bUseControllerRotationYaw = false;
    bUseControllerRotationPitch = false;
    bUseControllerRotationRoll = false;
}

void ADinosaurBase::BeginPlay()
{
    Super::BeginPlay();

    // Sync movement speed to stats
    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        MoveComp->MaxWalkSpeed = DinoStats.WalkSpeed;
    }

    // Start in idle
    BehaviorState = EEng_DinoBehaviorState::Idle;
}

void ADinosaurBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!IsAlive())
    {
        return;
    }

    // Throttled behavior tick for performance
    BehaviorTickAccumulator += DeltaTime;
    if (BehaviorTickAccumulator >= BehaviorTickInterval)
    {
        BehaviorTickAccumulator = 0.0f;
        TickBehavior(BehaviorTickInterval);
    }

    UpdateHunger(DeltaTime);
}

// ── Combat ────────────────────────────────────────────────────────────────

void ADinosaurBase::TakeDamageFromPlayer(float DamageAmount, AActor* DamageInstigator)
{
    if (!IsAlive())
    {
        return;
    }

    DinoStats.CurrentHealth = FMath::Max(0.0f, DinoStats.CurrentHealth - DamageAmount);

    // React to damage — become aggressive
    if (BehaviorState != EEng_DinoBehaviorState::Aggressive && DinoStats.bIsCarnivore)
    {
        CurrentTarget = DamageInstigator;
        SetBehaviorState(EEng_DinoBehaviorState::Aggressive);
    }

    if (!IsAlive())
    {
        Die();
    }
}

void ADinosaurBase::AttackTarget(AActor* Target)
{
    if (!Target || !IsAlive())
    {
        return;
    }

    float DistToTarget = FVector::Dist(GetActorLocation(), Target->GetActorLocation());
    if (DistToTarget <= DinoStats.AttackRange)
    {
        // Apply damage to target
        UGameplayStatics::ApplyDamage(
            Target,
            DinoStats.AttackDamage,
            GetController(),
            this,
            UDamageType::StaticClass()
        );

        OnDinosaurAttack(Target);
    }
}

bool ADinosaurBase::IsAlive() const
{
    return DinoStats.CurrentHealth > 0.0f;
}

float ADinosaurBase::GetHealthPercent() const
{
    if (DinoStats.MaxHealth <= 0.0f)
    {
        return 0.0f;
    }
    return DinoStats.CurrentHealth / DinoStats.MaxHealth;
}

// ── Behavior ──────────────────────────────────────────────────────────────

void ADinosaurBase::SetBehaviorState(EEng_DinoBehaviorState NewState)
{
    if (NewState == BehaviorState)
    {
        return;
    }

    EEng_DinoBehaviorState OldState = BehaviorState;
    BehaviorState = NewState;

    // Adjust movement speed based on state
    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        bool bIsRunning = (NewState == EEng_DinoBehaviorState::Hunting ||
                           NewState == EEng_DinoBehaviorState::Fleeing ||
                           NewState == EEng_DinoBehaviorState::Aggressive);
        MoveComp->MaxWalkSpeed = bIsRunning ? DinoStats.RunSpeed : DinoStats.WalkSpeed;
    }

    OnBehaviorStateChanged(OldState, NewState);
}

void ADinosaurBase::DetectNearbyPlayer()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
    if (!PlayerPawn)
    {
        return;
    }

    float DistToPlayer = FVector::Dist(GetActorLocation(), PlayerPawn->GetActorLocation());

    if (DistToPlayer <= DinoStats.DetectionRadius)
    {
        if (DinoStats.bIsCarnivore && BehaviorState == EEng_DinoBehaviorState::Idle)
        {
            CurrentTarget = PlayerPawn;
            SetBehaviorState(EEng_DinoBehaviorState::Hunting);
        }
    }
    else if (BehaviorState == EEng_DinoBehaviorState::Hunting && CurrentTarget == PlayerPawn)
    {
        // Lost sight of player
        CurrentTarget = nullptr;
        SetBehaviorState(EEng_DinoBehaviorState::Idle);
    }
}

void ADinosaurBase::UpdateHunger(float DeltaTime)
{
    DinoStats.Hunger = FMath::Max(0.0f, DinoStats.Hunger - DinoStats.HungerDecayRate * DeltaTime);

    // Starving dinosaurs become more aggressive
    if (DinoStats.Hunger < 20.0f && DinoStats.bIsCarnivore &&
        BehaviorState == EEng_DinoBehaviorState::Idle)
    {
        SetBehaviorState(EEng_DinoBehaviorState::Patrolling);
    }
}

// ── Internal ──────────────────────────────────────────────────────────────

void ADinosaurBase::TickBehavior(float DeltaTime)
{
    switch (BehaviorState)
    {
        case EEng_DinoBehaviorState::Idle:
            HandleIdleBehavior();
            break;

        case EEng_DinoBehaviorState::Hunting:
        case EEng_DinoBehaviorState::Aggressive:
            HandleHuntingBehavior();
            break;

        case EEng_DinoBehaviorState::Fleeing:
            HandleFleeingBehavior();
            break;

        case EEng_DinoBehaviorState::Patrolling:
            DetectNearbyPlayer();
            break;

        default:
            break;
    }
}

void ADinosaurBase::HandleIdleBehavior()
{
    DetectNearbyPlayer();
}

void ADinosaurBase::HandleHuntingBehavior()
{
    if (!CurrentTarget || !IsAlive())
    {
        SetBehaviorState(EEng_DinoBehaviorState::Idle);
        return;
    }

    float DistToTarget = FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation());

    if (DistToTarget <= DinoStats.AttackRange)
    {
        AttackTarget(CurrentTarget);
    }
    else if (DistToTarget > DinoStats.DetectionRadius * 1.5f)
    {
        // Target escaped
        CurrentTarget = nullptr;
        SetBehaviorState(EEng_DinoBehaviorState::Idle);
    }
}

void ADinosaurBase::HandleFleeingBehavior()
{
    // Move away from current threat
    if (CurrentTarget)
    {
        FVector AwayDir = (GetActorLocation() - CurrentTarget->GetActorLocation()).GetSafeNormal();
        FVector FleeTarget = GetActorLocation() + AwayDir * 2000.0f;
        // AI controller handles actual movement via BT — this sets the direction intent
    }

    // Stop fleeing when health recovers or threat is gone
    if (!CurrentTarget)
    {
        SetBehaviorState(EEng_DinoBehaviorState::Idle);
    }
}

void ADinosaurBase::Die()
{
    SetBehaviorState(EEng_DinoBehaviorState::Dead);

    // Disable collision and movement
    SetActorEnableCollision(false);
    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        MoveComp->DisableMovement();
    }

    OnDinosaurDeath();

    // Auto-destroy after 10 seconds (corpse cleanup)
    SetLifeSpan(10.0f);
}
