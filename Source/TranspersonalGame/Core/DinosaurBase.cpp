// DinosaurBase.cpp
// Engine Architect #02 — Cycle PROD_CYCLE_AUTO_20260625_004
// Base class for all dinosaur pawns in the prehistoric survival game.

#include "DinosaurBase.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"

ADinosaurBase::ADinosaurBase()
{
    PrimaryActorTick.bCanEverTick = true;

    // Default survival stats
    MaxHealth = 500.0f;
    CurrentHealth = MaxHealth;
    MaxStamina = 200.0f;
    CurrentStamina = MaxStamina;
    WalkSpeed = 300.0f;
    RunSpeed = 700.0f;
    AttackDamage = 50.0f;
    DetectionRadius = 1500.0f;
    bIsAggressive = false;
    bIsAlive = true;
    DinosaurSpecies = EEng_DinosaurSpecies::TRex;

    // Movement component defaults
    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
    GetCharacterMovement()->JumpZVelocity = 0.0f; // Dinosaurs don't jump by default
    GetCharacterMovement()->GravityScale = 1.0f;
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 180.0f, 0.0f);

    // Capsule defaults
    GetCapsuleComponent()->SetCapsuleHalfHeight(88.0f);
    GetCapsuleComponent()->SetCapsuleRadius(34.0f);

    // Mesh defaults
    GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -88.0f));
    GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
}

void ADinosaurBase::BeginPlay()
{
    Super::BeginPlay();
    CurrentHealth = MaxHealth;
    CurrentStamina = MaxStamina;
}

void ADinosaurBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Stamina regeneration when not running
    if (CurrentStamina < MaxStamina)
    {
        CurrentStamina = FMath::Min(MaxStamina, CurrentStamina + DeltaTime * 20.0f);
    }
}

float ADinosaurBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    if (!bIsAlive) return 0.0f;

    CurrentHealth = FMath::Max(0.0f, CurrentHealth - ActualDamage);

    if (CurrentHealth <= 0.0f)
    {
        Die();
    }

    return ActualDamage;
}

void ADinosaurBase::Die()
{
    if (!bIsAlive) return;
    bIsAlive = false;

    // Disable movement
    GetCharacterMovement()->DisableMovement();
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Enable ragdoll on death
    GetMesh()->SetSimulatePhysics(true);
    GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

    // Destroy after 10 seconds
    SetLifeSpan(10.0f);
}

void ADinosaurBase::SetRunning(bool bRun)
{
    if (bRun && CurrentStamina > 10.0f)
    {
        GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
    }
    else
    {
        GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
    }
}

float ADinosaurBase::GetHealthPercent() const
{
    return (MaxHealth > 0.0f) ? (CurrentHealth / MaxHealth) : 0.0f;
}

float ADinosaurBase::GetStaminaPercent() const
{
    return (MaxStamina > 0.0f) ? (CurrentStamina / MaxStamina) : 0.0f;
}
