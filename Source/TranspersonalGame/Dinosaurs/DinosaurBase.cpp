#include "DinosaurBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

// ─────────────────────────────────────────────────────────────────────────────
ADinosaurBase::ADinosaurBase()
{
    PrimaryActorTick.bCanEverTick = true;

    // Default movement settings — subclasses can override
    if (UCharacterMovementComponent* Move = GetCharacterMovement())
    {
        Move->MaxWalkSpeed        = Stats.PatrolSpeed;
        Move->bOrientRotationToMovement = true;
        Move->RotationRate        = FRotator(0.0f, 360.0f, 0.0f);
        Move->GravityScale        = 1.0f;
        Move->JumpZVelocity       = 0.0f; // Dinosaurs don't jump
    }

    bUseControllerRotationYaw   = false;
    bUseControllerRotationPitch = false;
    bUseControllerRotationRoll  = false;
}

// ─────────────────────────────────────────────────────────────────────────────
void ADinosaurBase::BeginPlay()
{
    Super::BeginPlay();
    SpawnLocation = GetActorLocation();
    PatrolTarget  = SpawnLocation;

    // Start in Idle, transition to Patrol after a short delay
    SetDinoState(EEng_DinoState::Idle);
    IdleTimer = FMath::RandRange(1.0f, 3.0f);
}

// ─────────────────────────────────────────────────────────────────────────────
void ADinosaurBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!IsAlive())
    {
        return;
    }

    // Cool down attack timer
    if (AttackCooldownRemaining > 0.0f)
    {
        AttackCooldownRemaining -= DeltaTime;
    }

    // Run the correct state tick
    switch (CurrentState)
    {
        case EEng_DinoState::Idle:    TickIdle(DeltaTime);    break;
        case EEng_DinoState::Patrol:  TickPatrol(DeltaTime);  break;
        case EEng_DinoState::Chase:   TickChase(DeltaTime);   break;
        case EEng_DinoState::Attack:  TickAttack(DeltaTime);  break;
        case EEng_DinoState::Flee:    TickFlee(DeltaTime);    break;
        default: break;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
void ADinosaurBase::SetDinoState(EEng_DinoState NewState)
{
    if (NewState == CurrentState)
    {
        return;
    }

    EEng_DinoState OldState = CurrentState;
    CurrentState = NewState;

    // Adjust movement speed based on state
    if (UCharacterMovementComponent* Move = GetCharacterMovement())
    {
        switch (NewState)
        {
            case EEng_DinoState::Patrol:
                Move->MaxWalkSpeed = Stats.PatrolSpeed;
                break;
            case EEng_DinoState::Chase:
            case EEng_DinoState::Attack:
                Move->MaxWalkSpeed = Stats.ChaseSpeed;
                break;
            case EEng_DinoState::Flee:
                Move->MaxWalkSpeed = Stats.ChaseSpeed * 1.2f;
                break;
            default:
                Move->MaxWalkSpeed = Stats.PatrolSpeed;
                break;
        }
    }

    OnDinoStateChanged(OldState, NewState);
}

// ─────────────────────────────────────────────────────────────────────────────
void ADinosaurBase::TickIdle(float DeltaTime)
{
    IdleTimer -= DeltaTime;
    if (IdleTimer <= 0.0f)
    {
        PickNewPatrolTarget();
        SetDinoState(EEng_DinoState::Patrol);
    }

    // Still check for nearby players even while idle
    float DistToPlayer = GetDistanceToPlayer();
    if (DistToPlayer < Stats.DetectionRadius)
    {
        SetDinoState(EEng_DinoState::Chase);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
void ADinosaurBase::TickPatrol(float DeltaTime)
{
    // Check for player first
    float DistToPlayer = GetDistanceToPlayer();
    if (DistToPlayer < Stats.DetectionRadius)
    {
        SetDinoState(EEng_DinoState::Chase);
        return;
    }

    // Move towards patrol target
    FVector MyLoc = GetActorLocation();
    float DistToTarget = FVector::Dist(MyLoc, PatrolTarget);

    if (DistToTarget < 150.0f)
    {
        // Reached patrol point — wait then pick new one
        PatrolWaitRemaining -= DeltaTime;
        if (PatrolWaitRemaining <= 0.0f)
        {
            PickNewPatrolTarget();
            PatrolWaitRemaining = PatrolWaitTime;
        }
    }
    else
    {
        // Move towards patrol target
        FVector Direction = (PatrolTarget - MyLoc).GetSafeNormal();
        AddMovementInput(Direction, 1.0f);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
void ADinosaurBase::TickChase(float DeltaTime)
{
    float DistToPlayer = GetDistanceToPlayer();

    // Lost the player — return to patrol
    if (DistToPlayer > Stats.DetectionRadius * 1.5f || !TargetPlayer)
    {
        TargetPlayer = nullptr;
        PickNewPatrolTarget();
        SetDinoState(EEng_DinoState::Patrol);
        return;
    }

    // Close enough to attack
    if (DistToPlayer <= Stats.AttackRange)
    {
        SetDinoState(EEng_DinoState::Attack);
        return;
    }

    // Move towards player
    MoveTowardsPlayer(DeltaTime);
}

// ─────────────────────────────────────────────────────────────────────────────
void ADinosaurBase::TickAttack(float DeltaTime)
{
    float DistToPlayer = GetDistanceToPlayer();

    // Player moved out of attack range — chase again
    if (DistToPlayer > Stats.AttackRange * 1.5f)
    {
        SetDinoState(EEng_DinoState::Chase);
        return;
    }

    // Perform attack if cooldown is ready
    if (AttackCooldownRemaining <= 0.0f)
    {
        PerformAttack();
        AttackCooldownRemaining = Stats.AttackCooldown;
    }

    // Face the player while attacking
    if (TargetPlayer)
    {
        FVector ToPlayer = (TargetPlayer->GetActorLocation() - GetActorLocation()).GetSafeNormal();
        FRotator LookAt  = ToPlayer.Rotation();
        SetActorRotation(FRotator(0.0f, LookAt.Yaw, 0.0f));
    }
}

// ─────────────────────────────────────────────────────────────────────────────
void ADinosaurBase::TickFlee(float DeltaTime)
{
    // Move away from player
    if (TargetPlayer)
    {
        FVector AwayFromPlayer = (GetActorLocation() - TargetPlayer->GetActorLocation()).GetSafeNormal();
        AddMovementInput(AwayFromPlayer, 1.0f);
    }

    // If far enough, return to patrol
    float DistToPlayer = GetDistanceToPlayer();
    if (DistToPlayer > Stats.DetectionRadius * 2.0f)
    {
        TargetPlayer = nullptr;
        SetDinoState(EEng_DinoState::Patrol);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
void ADinosaurBase::PerformAttack()
{
    if (!TargetPlayer)
    {
        return;
    }

    // Apply damage to the player
    UGameplayStatics::ApplyDamage(
        TargetPlayer,
        Stats.AttackDamage,
        GetController(),
        this,
        UDamageType::StaticClass()
    );

    UE_LOG(LogTemp, Log, TEXT("DinosaurBase [%s]: Attacked player for %.1f damage"),
        *GetActorLabel(), Stats.AttackDamage);
}

// ─────────────────────────────────────────────────────────────────────────────
void ADinosaurBase::TakeDamageFromPlayer(float DamageAmount)
{
    if (!IsAlive())
    {
        return;
    }

    Stats.CurrentHealth = FMath::Max(0.0f, Stats.CurrentHealth - DamageAmount);

    UE_LOG(LogTemp, Log, TEXT("DinosaurBase [%s]: Took %.1f damage. HP: %.1f/%.1f"),
        *GetActorLabel(), DamageAmount, Stats.CurrentHealth, Stats.MaxHealth);

    if (Stats.CurrentHealth <= 0.0f)
    {
        SetDinoState(EEng_DinoState::Dead);
        OnDinoDied();
        return;
    }

    // Low health — flee
    if (Stats.CurrentHealth < Stats.MaxHealth * 0.25f)
    {
        SetDinoState(EEng_DinoState::Flee);
        return;
    }

    // Retaliate — chase the attacker
    if (CurrentState != EEng_DinoState::Chase && CurrentState != EEng_DinoState::Attack)
    {
        SetDinoState(EEng_DinoState::Chase);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
AActor* ADinosaurBase::FindNearestPlayer() const
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }

    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC || !PC->GetPawn())
    {
        return nullptr;
    }

    return PC->GetPawn();
}

// ─────────────────────────────────────────────────────────────────────────────
float ADinosaurBase::GetDistanceToPlayer() const
{
    if (!TargetPlayer)
    {
        // Try to find the player
        const_cast<ADinosaurBase*>(this)->TargetPlayer = FindNearestPlayer();
    }

    if (!TargetPlayer)
    {
        return TNumericLimits<float>::Max();
    }

    return FVector::Dist(GetActorLocation(), TargetPlayer->GetActorLocation());
}

// ─────────────────────────────────────────────────────────────────────────────
void ADinosaurBase::MoveTowardsPlayer(float DeltaTime)
{
    if (!TargetPlayer)
    {
        return;
    }

    FVector Direction = (TargetPlayer->GetActorLocation() - GetActorLocation()).GetSafeNormal();
    AddMovementInput(Direction, 1.0f);
}

// ─────────────────────────────────────────────────────────────────────────────
void ADinosaurBase::PickNewPatrolTarget()
{
    // Random point within PatrolRadius of spawn location
    FVector RandomOffset(
        FMath::RandRange(-PatrolRadius, PatrolRadius),
        FMath::RandRange(-PatrolRadius, PatrolRadius),
        0.0f
    );
    PatrolTarget = SpawnLocation + RandomOffset;
}

// ─────────────────────────────────────────────────────────────────────────────
void ADinosaurBase::OnDinoStateChanged_Implementation(EEng_DinoState OldState, EEng_DinoState NewState)
{
    // Base implementation — subclasses override for custom behaviour
    UE_LOG(LogTemp, Verbose, TEXT("DinosaurBase [%s]: State %d -> %d"),
        *GetActorLabel(), (int32)OldState, (int32)NewState);
}

// ─────────────────────────────────────────────────────────────────────────────
void ADinosaurBase::OnDinoDied_Implementation()
{
    UE_LOG(LogTemp, Log, TEXT("DinosaurBase [%s]: Died."), *GetActorLabel());

    // Disable collision and movement
    SetActorEnableCollision(false);
    if (UCharacterMovementComponent* Move = GetCharacterMovement())
    {
        Move->DisableMovement();
    }

    // Destroy after 10 seconds (corpse lingers briefly)
    SetLifeSpan(10.0f);
}
