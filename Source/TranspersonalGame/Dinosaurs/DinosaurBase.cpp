#include "DinosaurBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

ADinosaurBase::ADinosaurBase()
{
    PrimaryActorTick.bCanEverTick = true;

    // Default movement speeds — overridden per species in child classes
    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        MoveComp->MaxWalkSpeed = Stats.WalkSpeed;
        MoveComp->JumpZVelocity = 400.0f;
        MoveComp->bOrientRotationToMovement = true;
        MoveComp->RotationRate = FRotator(0.0f, 360.0f, 0.0f);
    }

    // Dinosaurs do not use controller rotation for movement
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;
}

void ADinosaurBase::BeginPlay()
{
    Super::BeginPlay();

    // Initialise health from stats
    Stats.CurrentHealth = Stats.MaxHealth;

    // Apply movement speed from stats
    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        MoveComp->MaxWalkSpeed = Stats.WalkSpeed;
    }

    // Start in idle state
    SetDinosaurState(EEng_DinosaurState::Idle);
}

void ADinosaurBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    UpdateAI(DeltaTime);
}

// ============================================================
// Combat
// ============================================================

void ADinosaurBase::TakeDinosaurDamage(float DamageAmount, AActor* DamageInstigator)
{
    if (!IsAlive())
    {
        return;
    }

    Stats.CurrentHealth = FMath::Max(0.0f, Stats.CurrentHealth - DamageAmount);

    // React to damage — switch to hunting or fleeing
    if (IsAlive())
    {
        if (Diet == EEng_DinosaurDiet::Carnivore || Diet == EEng_DinosaurDiet::Omnivore)
        {
            CurrentTarget = DamageInstigator;
            SetDinosaurState(EEng_DinosaurState::Hunting);
        }
        else
        {
            // Herbivores flee from threats
            CurrentTarget = DamageInstigator;
            SetDinosaurState(EEng_DinosaurState::Fleeing);
        }
    }
    else
    {
        OnDeath();
    }
}

void ADinosaurBase::PerformAttack(AActor* Target)
{
    if (!Target || !IsAlive())
    {
        return;
    }

    const float DistToTarget = FVector::Dist(GetActorLocation(), Target->GetActorLocation());
    if (DistToTarget <= Stats.AttackRange)
    {
        // Apply damage to the target via UE5 damage system
        UGameplayStatics::ApplyDamage(Target, Stats.AttackDamage, GetController(), this, nullptr);
    }
}

bool ADinosaurBase::IsAlive() const
{
    return Stats.CurrentHealth > 0.0f;
}

// ============================================================
// AI State Machine
// ============================================================

void ADinosaurBase::SetDinosaurState(EEng_DinosaurState NewState)
{
    if (CurrentState == NewState)
    {
        return;
    }

    CurrentState = NewState;

    // Adjust movement speed based on state
    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        switch (NewState)
        {
        case EEng_DinosaurState::Hunting:
        case EEng_DinosaurState::Attacking:
        case EEng_DinosaurState::Fleeing:
            MoveComp->MaxWalkSpeed = Stats.RunSpeed;
            break;
        case EEng_DinosaurState::Dead:
            MoveComp->MaxWalkSpeed = 0.0f;
            MoveComp->DisableMovement();
            break;
        default:
            MoveComp->MaxWalkSpeed = Stats.WalkSpeed;
            break;
        }
    }
}

void ADinosaurBase::UpdateAI(float DeltaTime)
{
    if (!IsAlive())
    {
        return;
    }

    switch (CurrentState)
    {
    case EEng_DinosaurState::Idle:
        HandleIdleState(DeltaTime);
        break;
    case EEng_DinosaurState::Roaming:
        HandleRoamingState(DeltaTime);
        break;
    case EEng_DinosaurState::Hunting:
        HandleHuntingState(DeltaTime);
        break;
    case EEng_DinosaurState::Fleeing:
        HandleFleeingState(DeltaTime);
        break;
    case EEng_DinosaurState::Attacking:
        HandleAttackingState(DeltaTime);
        break;
    default:
        break;
    }
}

void ADinosaurBase::HandleIdleState(float DeltaTime)
{
    RoamTimer += DeltaTime;
    if (RoamTimer >= RoamInterval)
    {
        RoamTimer = 0.0f;
        SetDinosaurState(EEng_DinosaurState::Roaming);
    }

    // Check for threats
    AActor* Threat = FindNearestThreat();
    if (Threat)
    {
        CurrentTarget = Threat;
        if (Diet == EEng_DinosaurDiet::Carnivore)
        {
            SetDinosaurState(EEng_DinosaurState::Hunting);
        }
        else
        {
            SetDinosaurState(EEng_DinosaurState::Fleeing);
        }
    }
}

void ADinosaurBase::HandleRoamingState(float DeltaTime)
{
    RoamTimer += DeltaTime;
    if (RoamTimer >= RoamInterval)
    {
        RoamTimer = 0.0f;
        SetDinosaurState(EEng_DinosaurState::Idle);
    }

    // Check for threats while roaming
    AActor* Threat = FindNearestThreat();
    if (Threat)
    {
        CurrentTarget = Threat;
        if (Diet == EEng_DinosaurDiet::Carnivore)
        {
            SetDinosaurState(EEng_DinosaurState::Hunting);
        }
        else
        {
            SetDinosaurState(EEng_DinosaurState::Fleeing);
        }
    }
}

void ADinosaurBase::HandleHuntingState(float DeltaTime)
{
    if (!CurrentTarget || !IsValid(CurrentTarget))
    {
        CurrentTarget = nullptr;
        SetDinosaurState(EEng_DinosaurState::Idle);
        return;
    }

    const float DistToTarget = FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation());

    if (DistToTarget <= Stats.AttackRange)
    {
        SetDinosaurState(EEng_DinosaurState::Attacking);
    }
    else if (DistToTarget > Stats.DetectionRadius * 1.5f)
    {
        // Lost the target
        CurrentTarget = nullptr;
        SetDinosaurState(EEng_DinosaurState::Idle);
    }
}

void ADinosaurBase::HandleFleeingState(float DeltaTime)
{
    if (!CurrentTarget || !IsValid(CurrentTarget))
    {
        CurrentTarget = nullptr;
        SetDinosaurState(EEng_DinosaurState::Idle);
        return;
    }

    const float DistToThreat = FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation());
    if (DistToThreat > Stats.DetectionRadius * 2.0f)
    {
        // Safe distance reached
        CurrentTarget = nullptr;
        SetDinosaurState(EEng_DinosaurState::Idle);
    }
}

void ADinosaurBase::HandleAttackingState(float DeltaTime)
{
    if (!CurrentTarget || !IsValid(CurrentTarget))
    {
        CurrentTarget = nullptr;
        SetDinosaurState(EEng_DinosaurState::Idle);
        return;
    }

    // Attack on a cooldown (simplified: attack every 1.5s via timer accumulator)
    RoamTimer += DeltaTime;
    if (RoamTimer >= 1.5f)
    {
        RoamTimer = 0.0f;
        PerformAttack(CurrentTarget);
    }

    const float DistToTarget = FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation());
    if (DistToTarget > Stats.AttackRange * 1.5f)
    {
        SetDinosaurState(EEng_DinosaurState::Hunting);
    }
}

// ============================================================
// Detection
// ============================================================

AActor* ADinosaurBase::FindNearestThreat() const
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }

    // Find the player pawn as the primary threat for carnivores
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
    if (!PlayerPawn)
    {
        return nullptr;
    }

    const float DistToPlayer = FVector::Dist(GetActorLocation(), PlayerPawn->GetActorLocation());
    if (DistToPlayer <= Stats.DetectionRadius)
    {
        return PlayerPawn;
    }

    return nullptr;
}

bool ADinosaurBase::CanDetectActor(AActor* TargetActor) const
{
    if (!TargetActor)
    {
        return false;
    }

    const float Distance = FVector::Dist(GetActorLocation(), TargetActor->GetActorLocation());
    return Distance <= Stats.DetectionRadius;
}

// ============================================================
// Death
// ============================================================

void ADinosaurBase::OnDeath()
{
    SetDinosaurState(EEng_DinosaurState::Dead);

    // Disable collision so the player can walk through the corpse
    SetActorEnableCollision(false);

    // Destroy after 10 seconds to clean up
    SetLifeSpan(10.0f);
}
