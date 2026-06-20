// DinosaurBase.cpp
// Core Systems Programmer — Agent #03
// Abstract base implementation for all dinosaur species.

#include "DinosaurBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "TimerManager.h"
#include "Engine/World.h"

ADinosaurBase::ADinosaurBase()
{
    PrimaryActorTick.bCanEverTick = true;

    // Default survival stats — overridden per species
    MaxHealth       = 200.0f;
    CurrentHealth   = MaxHealth;
    MoveSpeed       = 400.0f;
    AttackDamage    = 30.0f;
    AttackRange     = 150.0f;
    DetectionRadius = 1500.0f;
    bIsAggressive   = false;
    bIsDead         = false;
    DinosaurSpecies = EDinosaurSpecies::Unknown;
    DietType        = EDietType::Omnivore;

    // Movement
    GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
    GetCharacterMovement()->JumpZVelocity = 400.0f;
    GetCharacterMovement()->AirControl = 0.2f;

    // Capsule defaults
    GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw   = false;
    bUseControllerRotationRoll  = false;
}

void ADinosaurBase::BeginPlay()
{
    Super::BeginPlay();
    CurrentHealth = MaxHealth;
    GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
}

void ADinosaurBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

float ADinosaurBase::TakeDamageDino(float DamageAmount, AActor* DamageCauser)
{
    if (bIsDead) return 0.0f;

    CurrentHealth = FMath::Clamp(CurrentHealth - DamageAmount, 0.0f, MaxHealth);

    if (CurrentHealth <= 0.0f)
    {
        bIsDead = true;
        OnDeath();
    }
    else if (!bIsAggressive && DamageAmount > 0.0f)
    {
        // Become aggressive when hit
        SetAggressive(true);
    }

    return DamageAmount;
}

void ADinosaurBase::Attack_Implementation()
{
    // Base implementation — subclasses override for species-specific attacks
    // Plays attack animation, applies damage to overlapping actors in AttackRange
    if (bIsDead) return;

    TArray<AActor*> OverlappingActors;
    GetOverlappingActors(OverlappingActors);

    for (AActor* Target : OverlappingActors)
    {
        if (Target && Target != this)
        {
            float Dist = FVector::Dist(GetActorLocation(), Target->GetActorLocation());
            if (Dist <= AttackRange)
            {
                Target->TakeDamage(AttackDamage, FDamageEvent(), GetController(), this);
            }
        }
    }
}

void ADinosaurBase::SetAggressive(bool bAggressive)
{
    bIsAggressive = bAggressive;
    GetCharacterMovement()->MaxWalkSpeed = bAggressive ? MoveSpeed * 1.5f : MoveSpeed;
}

void ADinosaurBase::OnDeath_Implementation()
{
    // Base death — disable movement, collision, schedule destroy
    GetCharacterMovement()->DisableMovement();
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Destroy after 10 seconds (allow death animation to play)
    FTimerHandle DestroyTimer;
    GetWorldTimerManager().SetTimer(DestroyTimer, [this]()
    {
        Destroy();
    }, 10.0f, false);
}

float ADinosaurBase::GetHealthPercent() const
{
    return (MaxHealth > 0.0f) ? (CurrentHealth / MaxHealth) : 0.0f;
}
