#include "DinosaurBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

ADinosaurBase::ADinosaurBase()
{
    PrimaryActorTick.bCanEverTick = true;

    // Default movement config — subclasses override per species
    if (UCharacterMovementComponent* Move = GetCharacterMovement())
    {
        Move->MaxWalkSpeed          = Stats.MoveSpeed;
        Move->MaxAcceleration       = 800.f;
        Move->BrakingDecelerationWalking = 600.f;
        Move->RotationRate          = FRotator(0.f, 180.f, 0.f);
        Move->bOrientRotationToMovement = true;
        Move->GravityScale          = 1.2f;
    }

    bUseControllerRotationYaw   = false;
    bUseControllerRotationPitch = false;
    bUseControllerRotationRoll  = false;
}

void ADinosaurBase::BeginPlay()
{
    Super::BeginPlay();
    HomeLocation = GetActorLocation();
    SetDinoState(EEng_DinoState::Idle);
}

void ADinosaurBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!IsAlive()) return;

    StateTimer += DeltaTime;

    switch (CurrentState)
    {
        case EEng_DinoState::Idle:    UpdateIdleState(DeltaTime);    break;
        case EEng_DinoState::Patrol:  UpdatePatrolState(DeltaTime);  break;
        case EEng_DinoState::Alert:   UpdateAlertState(DeltaTime);   break;
        case EEng_DinoState::Chasing: UpdateChaseState(DeltaTime);   break;
        case EEng_DinoState::Attacking: UpdateAttackState(DeltaTime); break;
        case EEng_DinoState::Fleeing: UpdateFleeState(DeltaTime);    break;
        default: break;
    }

    // Periodic player scan every 0.5s
    if (FMath::Fmod(StateTimer, 0.5f) < DeltaTime)
    {
        ScanForPlayer();
    }
}

// ── Combat ───────────────────────────────────────────────────────────────

float ADinosaurBase::TakeDamageFromAttack(float DamageAmount, AActor* DamageCauser)
{
    if (!IsAlive()) return 0.f;

    const float ActualDamage = FMath::Max(0.f, DamageAmount);
    Stats.CurrentHealth = FMath::Max(0.f, Stats.CurrentHealth - ActualDamage);

    if (!IsAlive())
    {
        SetDinoState(EEng_DinoState::Dead);
        GetCharacterMovement()->DisableMovement();
    }
    else if (bIsAggressive && DamageCauser)
    {
        CurrentTarget = DamageCauser;
        SetDinoState(EEng_DinoState::Chasing);
    }
    else if (!bIsAggressive)
    {
        SetDinoState(EEng_DinoState::Fleeing);
    }

    return ActualDamage;
}

void ADinosaurBase::PerformAttack()
{
    if (!CurrentTarget || !IsAlive()) return;

    const float Dist = FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation());
    if (Dist <= Stats.AttackRange)
    {
        // Apply damage via UE5 damage system
        UGameplayStatics::ApplyDamage(
            CurrentTarget,
            Stats.AttackDamage,
            GetController(),
            this,
            UDamageType::StaticClass()
        );
    }
}

// ── State Machine ────────────────────────────────────────────────────────

void ADinosaurBase::SetDinoState(EEng_DinoState NewState)
{
    if (CurrentState == NewState) return;
    CurrentState = NewState;
    StateTimer = 0.f;

    // Adjust speed per state
    if (UCharacterMovementComponent* Move = GetCharacterMovement())
    {
        switch (NewState)
        {
            case EEng_DinoState::Chasing:
            case EEng_DinoState::Fleeing:
                Move->MaxWalkSpeed = Stats.SprintSpeed;
                break;
            default:
                Move->MaxWalkSpeed = Stats.MoveSpeed;
                break;
        }
    }
}

void ADinosaurBase::UpdateIdleState(float DeltaTime)
{
    // After 5s idle, begin patrol
    if (StateTimer > 5.f)
    {
        SetDinoState(EEng_DinoState::Patrol);
    }
}

void ADinosaurBase::UpdatePatrolState(float DeltaTime)
{
    // Simple patrol: move toward a random point near home
    if (StateTimer < 0.1f)
    {
        const FVector Offset = FVector(
            FMath::RandRange(-PatrolRadius, PatrolRadius),
            FMath::RandRange(-PatrolRadius, PatrolRadius),
            0.f
        );
        const FVector Target = HomeLocation + Offset;
        AddMovementInput((Target - GetActorLocation()).GetSafeNormal(), 1.f);
    }

    // Return to idle after 15s patrol
    if (StateTimer > 15.f)
    {
        SetDinoState(EEng_DinoState::Idle);
    }
}

void ADinosaurBase::UpdateAlertState(float DeltaTime)
{
    if (!CurrentTarget) { SetDinoState(EEng_DinoState::Idle); return; }

    // Face the threat
    const FRotator LookAt = UKismetMathLibrary::FindLookAtRotation(
        GetActorLocation(), CurrentTarget->GetActorLocation());
    SetActorRotation(FRotator(0.f, LookAt.Yaw, 0.f));

    // Carnivores chase after 2s alert; herbivores flee
    if (StateTimer > 2.f)
    {
        SetDinoState(bIsAggressive ? EEng_DinoState::Chasing : EEng_DinoState::Fleeing);
    }
}

void ADinosaurBase::UpdateChaseState(float DeltaTime)
{
    if (!CurrentTarget || !IsPlayerInRange(Stats.DetectionRange * 1.5f))
    {
        OnLosePlayer();
        return;
    }

    const FVector Dir = (CurrentTarget->GetActorLocation() - GetActorLocation()).GetSafeNormal();
    AddMovementInput(Dir, 1.f);

    if (IsPlayerInRange(Stats.AttackRange))
    {
        SetDinoState(EEng_DinoState::Attacking);
    }
}

void ADinosaurBase::UpdateAttackState(float DeltaTime)
{
    if (!CurrentTarget) { SetDinoState(EEng_DinoState::Idle); return; }

    if (!IsPlayerInRange(Stats.AttackRange * 1.2f))
    {
        SetDinoState(EEng_DinoState::Chasing);
        return;
    }

    // Attack every 1.5s
    if (StateTimer >= 1.5f)
    {
        PerformAttack();
        StateTimer = 0.f;
    }
}

void ADinosaurBase::UpdateFleeState(float DeltaTime)
{
    if (!CurrentTarget) { SetDinoState(EEng_DinoState::Idle); return; }

    // Run away from threat
    const FVector AwayDir = (GetActorLocation() - CurrentTarget->GetActorLocation()).GetSafeNormal();
    AddMovementInput(AwayDir, 1.f);

    // Stop fleeing after 10s or if far enough
    if (StateTimer > 10.f || !IsPlayerInRange(Stats.DetectionRange * 2.f))
    {
        SetDinoState(EEng_DinoState::Idle);
        CurrentTarget = nullptr;
    }
}

// ── Detection ────────────────────────────────────────────────────────────

void ADinosaurBase::ScanForPlayer()
{
    if (!IsAlive()) return;

    APawn* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!Player) return;

    const float Dist = FVector::Dist(GetActorLocation(), Player->GetActorLocation());
    if (Dist <= Stats.DetectionRange)
    {
        OnDetectPlayer(Player);
    }
}

bool ADinosaurBase::IsPlayerInRange(float Range) const
{
    APawn* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!Player) return false;
    return FVector::Dist(GetActorLocation(), Player->GetActorLocation()) <= Range;
}

float ADinosaurBase::DistanceToPlayer() const
{
    APawn* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!Player) return TNumericLimits<float>::Max();
    return FVector::Dist(GetActorLocation(), Player->GetActorLocation());
}

void ADinosaurBase::OnDetectPlayer(AActor* Player)
{
    if (CurrentState == EEng_DinoState::Dead) return;
    if (CurrentState == EEng_DinoState::Chasing || CurrentState == EEng_DinoState::Attacking) return;

    CurrentTarget = Player;
    SetDinoState(EEng_DinoState::Alert);
}

void ADinosaurBase::OnLosePlayer()
{
    CurrentTarget = nullptr;
    SetDinoState(EEng_DinoState::Idle);
}

float ADinosaurBase::GetHealthPercent() const
{
    if (Stats.MaxHealth <= 0.f) return 0.f;
    return Stats.CurrentHealth / Stats.MaxHealth;
}
