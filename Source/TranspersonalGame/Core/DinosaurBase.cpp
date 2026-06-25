// DinosaurBase.cpp — Base class for all dinosaur pawns
// Engine Architect #02 — PROD_CYCLE_AUTO_20260625_002
// Provides: movement, survival stats, territory, aggression radius

#include "DinosaurBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

ADinosaurBase::ADinosaurBase()
{
    PrimaryActorTick.bCanEverTick = true;

    // Default survival stats
    Health = 100.0f;
    MaxHealth = 100.0f;
    Hunger = 100.0f;
    MaxHunger = 100.0f;
    Stamina = 100.0f;
    MaxStamina = 100.0f;

    // Default behaviour
    AggressionRadius = 1500.0f;
    TerritoryRadius = 3000.0f;
    WalkSpeed = 300.0f;
    RunSpeed = 700.0f;
    bIsAggressive = false;
    bIsPredator = false;
    DinoSpecies = EDinoSpecies::Unknown;
    CurrentBehaviourState = EDinoBehaviourState::Idle;

    // Movement defaults
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
        GetCharacterMovement()->JumpZVelocity = 400.0f;
        GetCharacterMovement()->GravityScale = 1.0f;
        GetCharacterMovement()->bOrientRotationToMovement = true;
    }

    // Capsule defaults — overridden per species in child BP
    if (GetCapsuleComponent())
    {
        GetCapsuleComponent()->SetCapsuleHalfHeight(88.0f);
        GetCapsuleComponent()->SetCapsuleRadius(34.0f);
    }
}

void ADinosaurBase::BeginPlay()
{
    Super::BeginPlay();

    // Start passive metabolism tick every 5s
    GetWorldTimerManager().SetTimer(
        MetabolismTimerHandle,
        this,
        &ADinosaurBase::TickMetabolism,
        5.0f,
        true
    );

    // Set initial movement speed
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
    }
}

void ADinosaurBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    UpdateBehaviourState(DeltaTime);
}

float ADinosaurBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
    Health = FMath::Clamp(Health - ActualDamage, 0.0f, MaxHealth);

    if (Health <= 0.0f)
    {
        OnDeath();
    }
    else if (bIsPredator && ActualDamage > 0.0f)
    {
        // Predators become aggressive when hurt
        bIsAggressive = true;
        CurrentBehaviourState = EDinoBehaviourState::Attacking;
    }

    return ActualDamage;
}

void ADinosaurBase::SetBehaviourState(EDinoBehaviourState NewState)
{
    if (CurrentBehaviourState != NewState)
    {
        CurrentBehaviourState = NewState;
        OnBehaviourStateChanged(NewState);
    }
}

void ADinosaurBase::UpdateBehaviourState(float DeltaTime)
{
    // Base implementation — child classes (BT) override this
    // Idle → Wandering if stamina > 20%
    if (CurrentBehaviourState == EDinoBehaviourState::Idle)
    {
        if (Stamina > MaxStamina * 0.2f)
        {
            // Wander logic handled by BT in child classes
        }
    }

    // Resting recovers stamina
    if (CurrentBehaviourState == EDinoBehaviourState::Resting)
    {
        Stamina = FMath::Clamp(Stamina + (DeltaTime * 10.0f), 0.0f, MaxStamina);
    }
}

void ADinosaurBase::TickMetabolism()
{
    // Passive hunger drain
    Hunger = FMath::Clamp(Hunger - 2.0f, 0.0f, MaxHunger);

    if (Hunger < MaxHunger * 0.3f && bIsPredator)
    {
        // Hungry predators become more aggressive
        bIsAggressive = true;
        if (CurrentBehaviourState == EDinoBehaviourState::Idle ||
            CurrentBehaviourState == EDinoBehaviourState::Wandering)
        {
            CurrentBehaviourState = EDinoBehaviourState::Hunting;
        }
    }

    if (Hunger <= 0.0f)
    {
        // Starvation damage
        Health = FMath::Clamp(Health - 5.0f, 0.0f, MaxHealth);
        if (Health <= 0.0f)
        {
            OnDeath();
        }
    }
}

void ADinosaurBase::OnDeath()
{
    // Disable collision and movement
    if (GetCapsuleComponent())
    {
        GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->DisableMovement();
    }

    // Stop metabolism timer
    GetWorldTimerManager().ClearTimer(MetabolismTimerHandle);

    // Notify Blueprint
    OnDeathBP();

    // Destroy after 30s (corpse decay)
    SetLifeSpan(30.0f);
}

void ADinosaurBase::OnDeathBP_Implementation()
{
    // Blueprint override point — play death animation, spawn loot, etc.
}

void ADinosaurBase::OnBehaviourStateChanged_Implementation(EDinoBehaviourState NewState)
{
    // Blueprint override point — update animation state machine
}

bool ADinosaurBase::IsPlayerInAggressionRadius() const
{
    // Simple distance check — full AI uses BT perception in child classes
    APawn* PlayerPawn = GetWorld() ? GetWorld()->GetFirstPlayerController() ?
        GetWorld()->GetFirstPlayerController()->GetPawn() : nullptr : nullptr;

    if (!PlayerPawn) return false;

    float Dist = FVector::Dist(GetActorLocation(), PlayerPawn->GetActorLocation());
    return Dist <= AggressionRadius;
}

float ADinosaurBase::GetHealthPercent() const
{
    return (MaxHealth > 0.0f) ? (Health / MaxHealth) : 0.0f;
}

float ADinosaurBase::GetHungerPercent() const
{
    return (MaxHunger > 0.0f) ? (Hunger / MaxHunger) : 0.0f;
}

float ADinosaurBase::GetStaminaPercent() const
{
    return (MaxStamina > 0.0f) ? (Stamina / MaxStamina) : 0.0f;
}
