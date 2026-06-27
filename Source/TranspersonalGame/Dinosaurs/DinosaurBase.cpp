#include "DinosaurBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"

ADinosaurBase::ADinosaurBase()
{
    PrimaryActorTick.bCanEverTick = true;

    // Default movement setup
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed = Stats.MoveSpeed;
        GetCharacterMovement()->bOrientRotationToMovement = true;
        GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
        GetCharacterMovement()->JumpZVelocity = 400.0f;
        GetCharacterMovement()->AirControl = 0.2f;
    }

    // Default capsule size (overridden per species)
    GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);

    // Don't use controller rotation — let movement component handle it
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;
}

void ADinosaurBase::BeginPlay()
{
    Super::BeginPlay();

    // Set territory center to spawn location
    if (!bHasTerritoryCenter)
    {
        TerritoryCenter = GetActorLocation();
        bHasTerritoryCenter = true;
    }

    // Apply initial move speed
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed = Stats.MoveSpeed;
    }
}

void ADinosaurBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!IsAlive()) return;

    UpdateHunger(DeltaTime);

    // Update attack cooldown
    if (TimeSinceLastAttack < AttackCooldown)
    {
        TimeSinceLastAttack += DeltaTime;
    }
}

float ADinosaurBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    Stats.CurrentHealth = FMath::Clamp(Stats.CurrentHealth - DamageAmount, 0.0f, Stats.MaxHealth);

    if (Stats.CurrentHealth <= 0.0f)
    {
        Die();
    }
    else if (DamageCauser && BehaviorState != EEng_DinosaurBehaviorState::Attacking)
    {
        // React to being hit — switch to hunting/attacking
        CurrentTarget = DamageCauser;
        SetBehaviorState(EEng_DinosaurBehaviorState::Hunting);
    }

    return DamageAmount;
}

void ADinosaurBase::Attack(AActor* Target)
{
    if (!Target || !IsAlive()) return;
    if (TimeSinceLastAttack < AttackCooldown) return;

    float Distance = FVector::Dist(GetActorLocation(), Target->GetActorLocation());
    if (Distance <= Stats.AttackRange)
    {
        // Apply damage to target
        UGameplayStatics::ApplyDamage(Target, Stats.AttackDamage, GetController(), this, nullptr);
        TimeSinceLastAttack = 0.0f;
        OnAttackStart(Target);
    }
}

bool ADinosaurBase::IsAlive() const
{
    return Stats.CurrentHealth > 0.0f && BehaviorState != EEng_DinosaurBehaviorState::Dead;
}

void ADinosaurBase::SetBehaviorState(EEng_DinosaurBehaviorState NewState)
{
    if (BehaviorState == NewState) return;

    EEng_DinosaurBehaviorState OldState = BehaviorState;
    BehaviorState = NewState;

    // Adjust movement speed based on state
    if (GetCharacterMovement())
    {
        switch (NewState)
        {
        case EEng_DinosaurBehaviorState::Hunting:
        case EEng_DinosaurBehaviorState::Attacking:
            GetCharacterMovement()->MaxWalkSpeed = Stats.SprintSpeed;
            break;
        case EEng_DinosaurBehaviorState::Fleeing:
            GetCharacterMovement()->MaxWalkSpeed = Stats.SprintSpeed * 1.2f;
            break;
        case EEng_DinosaurBehaviorState::Sleeping:
            GetCharacterMovement()->MaxWalkSpeed = 0.0f;
            break;
        default:
            GetCharacterMovement()->MaxWalkSpeed = Stats.MoveSpeed;
            break;
        }
    }

    OnStateChanged(OldState, NewState);
}

void ADinosaurBase::OnDetectPlayer(AActor* Player)
{
    if (!Player || !IsAlive()) return;

    CurrentTarget = Player;

    // Carnivores hunt, herbivores flee
    if (Diet == EEng_DinosaurDiet::Carnivore)
    {
        SetBehaviorState(EEng_DinosaurBehaviorState::Hunting);
    }
    else if (Diet == EEng_DinosaurDiet::Herbivore)
    {
        SetBehaviorState(EEng_DinosaurBehaviorState::Fleeing);
    }
}

void ADinosaurBase::OnLosePlayer()
{
    CurrentTarget = nullptr;

    if (BehaviorState == EEng_DinosaurBehaviorState::Hunting ||
        BehaviorState == EEng_DinosaurBehaviorState::Attacking)
    {
        SetBehaviorState(EEng_DinosaurBehaviorState::Patrolling);
    }
}

void ADinosaurBase::Die()
{
    SetBehaviorState(EEng_DinosaurBehaviorState::Dead);

    // Disable collision and movement
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->DisableMovement();
    }

    OnDeath();

    // Destroy after delay (allow death animation to play)
    SetLifeSpan(10.0f);
}

void ADinosaurBase::UpdateHunger(float DeltaTime)
{
    if (BehaviorState == EEng_DinosaurBehaviorState::Dead) return;

    Stats.Hunger = FMath::Clamp(Stats.Hunger - (Stats.HungerDecayRate * DeltaTime), 0.0f, 100.0f);

    // Very hungry carnivores become more aggressive
    if (Stats.Hunger < 20.0f && Diet == EEng_DinosaurDiet::Carnivore)
    {
        if (BehaviorState == EEng_DinosaurBehaviorState::Idle ||
            BehaviorState == EEng_DinosaurBehaviorState::Sleeping)
        {
            SetBehaviorState(EEng_DinosaurBehaviorState::Patrolling);
        }
    }
}
