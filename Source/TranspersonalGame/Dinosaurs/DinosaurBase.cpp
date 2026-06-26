// DinosaurBase.cpp
// Engine Architect #02 — Transpersonal Game Studio
// Base class for all dinosaur actors. Provides skeletal mesh, movement,
// survival stats (health, hunger, thirst), and basic AI state machine.

#include "DinosaurBase.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

ADinosaurBase::ADinosaurBase()
{
    PrimaryActorTick.bCanEverTick = true;

    // Capsule collision (root)
    GetCapsuleComponent()->InitCapsuleSize(60.0f, 120.0f);
    GetCapsuleComponent()->SetCollisionProfileName(TEXT("Pawn"));

    // Skeletal mesh
    GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -120.0f));
    GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));

    // Movement defaults
    GetCharacterMovement()->MaxWalkSpeed = 400.0f;
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 360.0f, 0.0f);
    GetCharacterMovement()->GravityScale = 1.0f;
    GetCharacterMovement()->JumpZVelocity = 600.0f;

    // Default stats
    MaxHealth = 500.0f;
    CurrentHealth = 500.0f;
    MaxHunger = 100.0f;
    CurrentHunger = 100.0f;
    MaxThirst = 100.0f;
    CurrentThirst = 100.0f;
    MoveSpeed = 400.0f;
    AttackDamage = 50.0f;
    AttackRange = 200.0f;
    DetectionRadius = 2000.0f;
    bIsCarnivore = true;
    bIsAlive = true;
    CurrentAIState = EDinoAIState::Idle;
    DinoSpecies = EDinoSpecies::TRex;
}

void ADinosaurBase::BeginPlay()
{
    Super::BeginPlay();

    // Start metabolism tick every 5 seconds
    GetWorldTimerManager().SetTimer(
        MetabolismTimerHandle,
        this,
        &ADinosaurBase::TickMetabolism,
        5.0f,
        true
    );

    // Start AI state evaluation every 2 seconds
    GetWorldTimerManager().SetTimer(
        AIStateTimerHandle,
        this,
        &ADinosaurBase::EvaluateAIState,
        2.0f,
        true
    );
}

void ADinosaurBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Execute current AI state behaviour
    switch (CurrentAIState)
    {
        case EDinoAIState::Idle:
            // Stand still, play idle animation
            break;
        case EDinoAIState::Roaming:
            // Handled by BehaviorTree / AI controller
            break;
        case EDinoAIState::Hunting:
            // Chase target
            break;
        case EDinoAIState::Fleeing:
            // Run from threat
            break;
        case EDinoAIState::Eating:
            // Consume food, restore hunger
            CurrentHunger = FMath::Min(CurrentHunger + DeltaTime * 10.0f, MaxHunger);
            break;
        case EDinoAIState::Drinking:
            // Consume water, restore thirst
            CurrentThirst = FMath::Min(CurrentThirst + DeltaTime * 10.0f, MaxThirst);
            break;
        case EDinoAIState::Dead:
            // No behaviour
            break;
        default:
            break;
    }
}

float ADinosaurBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    if (!bIsAlive) return 0.0f;

    CurrentHealth = FMath::Max(CurrentHealth - ActualDamage, 0.0f);

    if (CurrentHealth <= 0.0f)
    {
        Die();
    }
    else if (CurrentAIState != EDinoAIState::Hunting)
    {
        // Attacked — switch to hunting/fleeing
        if (bIsCarnivore)
        {
            SetAIState(EDinoAIState::Hunting);
        }
        else
        {
            SetAIState(EDinoAIState::Fleeing);
        }
    }

    return ActualDamage;
}

void ADinosaurBase::Die()
{
    bIsAlive = false;
    SetAIState(EDinoAIState::Dead);

    // Disable collision
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Disable movement
    GetCharacterMovement()->DisableMovement();

    // Clear timers
    GetWorldTimerManager().ClearTimer(MetabolismTimerHandle);
    GetWorldTimerManager().ClearTimer(AIStateTimerHandle);

    // Broadcast death event for quest/AI systems
    OnDinosaurDied.Broadcast(this);
}

void ADinosaurBase::SetAIState(EDinoAIState NewState)
{
    if (CurrentAIState == NewState) return;
    CurrentAIState = NewState;
    OnAIStateChanged.Broadcast(NewState);
}

void ADinosaurBase::TickMetabolism()
{
    if (!bIsAlive) return;

    // Drain hunger and thirst over time
    CurrentHunger = FMath::Max(CurrentHunger - 2.0f, 0.0f);
    CurrentThirst = FMath::Max(CurrentThirst - 3.0f, 0.0f);

    // Starvation damage
    if (CurrentHunger <= 0.0f)
    {
        CurrentHealth = FMath::Max(CurrentHealth - 5.0f, 0.0f);
        if (CurrentHealth <= 0.0f) Die();
    }

    // Dehydration damage
    if (CurrentThirst <= 0.0f)
    {
        CurrentHealth = FMath::Max(CurrentHealth - 8.0f, 0.0f);
        if (CurrentHealth <= 0.0f) Die();
    }
}

void ADinosaurBase::EvaluateAIState()
{
    if (!bIsAlive) return;
    if (CurrentAIState == EDinoAIState::Dead) return;

    // Priority: hunger/thirst > hunting > roaming
    if (CurrentHunger < 30.0f && bIsCarnivore)
    {
        SetAIState(EDinoAIState::Hunting);
        return;
    }

    if (CurrentThirst < 20.0f)
    {
        SetAIState(EDinoAIState::Drinking);
        return;
    }

    if (CurrentAIState == EDinoAIState::Idle)
    {
        // Occasionally start roaming
        if (FMath::RandBool())
        {
            SetAIState(EDinoAIState::Roaming);
        }
    }
    else if (CurrentAIState == EDinoAIState::Roaming)
    {
        // Occasionally rest
        if (FMath::RandRange(0, 4) == 0)
        {
            SetAIState(EDinoAIState::Idle);
        }
    }
}

float ADinosaurBase::GetHealthPercent() const
{
    return (MaxHealth > 0.0f) ? (CurrentHealth / MaxHealth) : 0.0f;
}

float ADinosaurBase::GetHungerPercent() const
{
    return (MaxHunger > 0.0f) ? (CurrentHunger / MaxHunger) : 0.0f;
}

float ADinosaurBase::GetThirstPercent() const
{
    return (MaxThirst > 0.0f) ? (CurrentThirst / MaxThirst) : 0.0f;
}
