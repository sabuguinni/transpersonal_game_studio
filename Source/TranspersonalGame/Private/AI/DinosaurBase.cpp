#include "AI/DinosaurBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

ADinosaurBase::ADinosaurBase()
{
    PrimaryActorTick.bCanEverTick = true;

    // Default movement settings — will be overridden by subclasses
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed = Stats.MoveSpeed;
        GetCharacterMovement()->bOrientRotationToMovement = true;
        GetCharacterMovement()->RotationRate = FRotator(0.0f, 360.0f, 0.0f);
        GetCharacterMovement()->GravityScale = 1.0f;
        GetCharacterMovement()->JumpZVelocity = 0.0f;  // Dinosaurs don't jump
        GetCharacterMovement()->AirControl = 0.0f;
    }

    // Don't use controller rotation — let movement component handle it
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;
}

void ADinosaurBase::BeginPlay()
{
    Super::BeginPlay();

    HomeLocation = GetActorLocation();
    RoamTarget = HomeLocation;

    // Initialise health
    Stats.CurrentHealth = Stats.MaxHealth;

    // Set initial movement speed
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed = Stats.MoveSpeed;
    }

    SetState(EEng_DinoState::Roaming);
}

void ADinosaurBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!IsAlive()) return;

    // Periodic player scan
    TimeSinceLastScan += DeltaTime;
    if (TimeSinceLastScan >= ScanInterval)
    {
        TimeSinceLastScan = 0.0f;
        ScanForPlayer();
    }

    // Attack cooldown
    TimeSinceLastAttack += DeltaTime;

    // State-driven behaviour
    switch (CurrentState)
    {
        case EEng_DinoState::Roaming:
            UpdateRoaming(DeltaTime);
            break;
        case EEng_DinoState::Hunting:
            UpdateHunting(DeltaTime);
            break;
        case EEng_DinoState::Attacking:
            UpdateAttacking(DeltaTime);
            break;
        default:
            break;
    }
}

// ── State Machine ─────────────────────────────────────────────────────────────

void ADinosaurBase::SetState(EEng_DinoState NewState)
{
    if (CurrentState == NewState) return;

    EEng_DinoState OldState = CurrentState;
    CurrentState = NewState;

    // Adjust speed for state
    if (GetCharacterMovement())
    {
        switch (NewState)
        {
            case EEng_DinoState::Hunting:
            case EEng_DinoState::Fleeing:
                GetCharacterMovement()->MaxWalkSpeed = Stats.SprintSpeed;
                break;
            default:
                GetCharacterMovement()->MaxWalkSpeed = Stats.MoveSpeed;
                break;
        }
    }

    OnStateChanged(OldState, NewState);
}

void ADinosaurBase::OnStateChanged_Implementation(EEng_DinoState OldState, EEng_DinoState NewState)
{
    // Base implementation — subclasses can override for animation triggers etc.
}

// ── Combat ────────────────────────────────────────────────────────────────────

void ADinosaurBase::TakeDamageFromPlayer(float DamageAmount, AActor* DamageSource)
{
    if (!IsAlive()) return;

    Stats.CurrentHealth = FMath::Max(0.0f, Stats.CurrentHealth - DamageAmount);

    if (!IsAlive())
    {
        OnDeath();
        return;
    }

    // React to damage — become aggressive if carnivore
    if (Stats.bIsCarnivore && DamageSource)
    {
        CurrentTarget = DamageSource;
        SetState(EEng_DinoState::Hunting);
    }
    else if (!Stats.bIsCarnivore)
    {
        // Herbivores flee
        SetState(EEng_DinoState::Fleeing);
    }
}

void ADinosaurBase::AttackTarget(AActor* Target)
{
    if (!Target || !IsAlive()) return;
    if (TimeSinceLastAttack < AttackCooldown) return;

    float DistToTarget = FVector::Dist(GetActorLocation(), Target->GetActorLocation());
    if (DistToTarget <= Stats.AttackRange)
    {
        // Apply damage to target
        UGameplayStatics::ApplyDamage(Target, Stats.AttackDamage, GetController(), this, nullptr);
        TimeSinceLastAttack = 0.0f;
    }
}

bool ADinosaurBase::IsAlive() const
{
    return Stats.CurrentHealth > 0.0f;
}

float ADinosaurBase::GetHealthPercent() const
{
    if (Stats.MaxHealth <= 0.0f) return 0.0f;
    return Stats.CurrentHealth / Stats.MaxHealth;
}

void ADinosaurBase::OnDeath_Implementation()
{
    SetState(EEng_DinoState::Dead);

    // Disable collision and movement
    SetActorEnableCollision(false);
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->DisableMovement();
    }

    // Destroy after 30 seconds (corpse lingers briefly)
    SetLifeSpan(30.0f);
}

// ── AI Detection ──────────────────────────────────────────────────────────────

void ADinosaurBase::ScanForPlayer()
{
    if (CurrentState == EEng_DinoState::Dead) return;

    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PC) return;

    APawn* PlayerPawn = PC->GetPawn();
    if (!PlayerPawn) return;

    float DistToPlayer = FVector::Dist(GetActorLocation(), PlayerPawn->GetActorLocation());

    if (DistToPlayer <= Stats.DetectionRadius)
    {
        if (CurrentState != EEng_DinoState::Hunting && CurrentState != EEng_DinoState::Attacking)
        {
            OnPlayerDetected(PlayerPawn);
        }
    }
    else if (CurrentState == EEng_DinoState::Hunting || CurrentState == EEng_DinoState::Attacking)
    {
        OnPlayerLost();
    }
}

void ADinosaurBase::OnPlayerDetected(AActor* Player)
{
    if (!Player) return;

    CurrentTarget = Player;

    if (Stats.bIsCarnivore)
    {
        SetState(EEng_DinoState::Hunting);
    }
    else
    {
        SetState(EEng_DinoState::Fleeing);
    }
}

void ADinosaurBase::OnPlayerLost()
{
    CurrentTarget = nullptr;
    SetState(EEng_DinoState::Roaming);
}

// ── Roaming ───────────────────────────────────────────────────────────────────

void ADinosaurBase::UpdateRoaming(float DeltaTime)
{
    TimeSinceLastRoam += DeltaTime;

    float DistToRoamTarget = FVector::Dist2D(GetActorLocation(), RoamTarget);

    if (DistToRoamTarget < 200.0f || TimeSinceLastRoam >= RoamInterval)
    {
        PickNewRoamTarget();
        TimeSinceLastRoam = 0.0f;
    }

    // Move toward roam target
    FVector Direction = (RoamTarget - GetActorLocation()).GetSafeNormal2D();
    AddMovementInput(Direction, 0.5f);
}

void ADinosaurBase::PickNewRoamTarget()
{
    // Pick a random point within territory radius of home
    float Angle = FMath::RandRange(0.0f, 360.0f);
    float Radius = FMath::RandRange(500.0f, Stats.TerritoryRadius * 0.5f);

    RoamTarget = HomeLocation + FVector(
        FMath::Cos(FMath::DegreesToRadians(Angle)) * Radius,
        FMath::Sin(FMath::DegreesToRadians(Angle)) * Radius,
        0.0f
    );
}

void ADinosaurBase::UpdateHunting(float DeltaTime)
{
    if (!CurrentTarget || !IsAlive()) return;

    float DistToTarget = FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation());

    if (DistToTarget <= Stats.AttackRange)
    {
        SetState(EEng_DinoState::Attacking);
        return;
    }

    // Move toward target
    FVector Direction = (CurrentTarget->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();
    AddMovementInput(Direction, 1.0f);
}

void ADinosaurBase::UpdateAttacking(float DeltaTime)
{
    if (!CurrentTarget || !IsAlive()) return;

    float DistToTarget = FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation());

    if (DistToTarget > Stats.AttackRange * 1.5f)
    {
        // Target moved away — chase again
        SetState(EEng_DinoState::Hunting);
        return;
    }

    // Face target
    FVector ToTarget = (CurrentTarget->GetActorLocation() - GetActorLocation()).GetSafeNormal();
    FRotator LookRot = ToTarget.Rotation();
    SetActorRotation(FMath::RInterpTo(GetActorRotation(), LookRot, DeltaTime, 5.0f));

    // Attack
    AttackTarget(CurrentTarget);
}
