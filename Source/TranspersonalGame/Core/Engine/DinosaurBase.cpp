#include "DinosaurBase.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "Engine/Engine.h"

ADinosaurBase::ADinosaurBase()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create capsule component
    CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComponent"));
    RootComponent = CapsuleComponent;
    CapsuleComponent->SetCapsuleHalfHeight(88.0f);
    CapsuleComponent->SetCapsuleRadius(34.0f);

    // Create mesh component
    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    MeshComponent->SetupAttachment(RootComponent);

    // Create movement component
    MovementComponent = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("MovementComponent"));
    MovementComponent->MaxSpeed = 300.0f;
    MovementComponent->Acceleration = 1000.0f;
    MovementComponent->Deceleration = 1000.0f;

    // Initialize default stats
    MaxHealth = 100.0f;
    CurrentHealth = MaxHealth;
    MovementSpeed = 300.0f;
    AttackDamage = 25.0f;
    DetectionRange = 500.0f;
    
    // Initialize behavior
    bIsAggressive = false;
    bIsHerbivore = true;
    HungerLevel = 50.0f;
    FearLevel = 0.0f;
}

void ADinosaurBase::BeginPlay()
{
    Super::BeginPlay();
    
    // Set initial health
    CurrentHealth = MaxHealth;
    
    // Configure movement speed
    if (MovementComponent)
    {
        MovementComponent->MaxSpeed = MovementSpeed;
    }
}

void ADinosaurBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update dinosaur behavior
    UpdateBehavior(DeltaTime);
    UpdateHunger(DeltaTime);
}

void ADinosaurBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
    // Dinosaurs don't need player input by default
}

void ADinosaurBase::TakeDamage(float DamageAmount)
{
    if (DamageAmount <= 0.0f || !IsAlive())
    {
        return;
    }

    CurrentHealth = FMath::Clamp(CurrentHealth - DamageAmount, 0.0f, MaxHealth);
    
    // Increase fear when taking damage
    FearLevel = FMath::Clamp(FearLevel + (DamageAmount * 0.5f), 0.0f, 100.0f);
    
    // Trigger health changed event
    OnHealthChanged(CurrentHealth, MaxHealth);
    
    // Check if dinosaur died
    if (CurrentHealth <= 0.0f)
    {
        OnDeath();
    }
}

void ADinosaurBase::Heal(float HealAmount)
{
    if (HealAmount <= 0.0f || !IsAlive())
    {
        return;
    }

    CurrentHealth = FMath::Clamp(CurrentHealth + HealAmount, 0.0f, MaxHealth);
    OnHealthChanged(CurrentHealth, MaxHealth);
}

bool ADinosaurBase::IsAlive() const
{
    return CurrentHealth > 0.0f;
}

void ADinosaurBase::SetAggressionState(bool bAggressive)
{
    bIsAggressive = bAggressive;
    
    if (bIsAggressive)
    {
        // Increase movement speed when aggressive
        if (MovementComponent)
        {
            MovementComponent->MaxSpeed = MovementSpeed * 1.5f;
        }
    }
    else
    {
        // Reset to normal speed
        if (MovementComponent)
        {
            MovementComponent->MaxSpeed = MovementSpeed;
        }
    }
}

void ADinosaurBase::UpdateBehavior(float DeltaTime)
{
    if (!IsAlive())
    {
        return;
    }

    // Gradually reduce fear over time
    if (FearLevel > 0.0f)
    {
        FearLevel = FMath::Clamp(FearLevel - (DeltaTime * 10.0f), 0.0f, 100.0f);
    }

    // Become aggressive if health is low and not herbivore
    if (!bIsHerbivore && CurrentHealth < (MaxHealth * 0.3f))
    {
        SetAggressionState(true);
    }
    else if (FearLevel < 20.0f && CurrentHealth > (MaxHealth * 0.7f))
    {
        SetAggressionState(false);
    }
}

void ADinosaurBase::UpdateHunger(float DeltaTime)
{
    // Increase hunger over time
    HungerLevel = FMath::Clamp(HungerLevel + (DeltaTime * 2.0f), 0.0f, 100.0f);
    
    // High hunger affects behavior
    if (HungerLevel > 80.0f)
    {
        // Hungry dinosaurs become more aggressive (even herbivores)
        SetAggressionState(true);
    }
}