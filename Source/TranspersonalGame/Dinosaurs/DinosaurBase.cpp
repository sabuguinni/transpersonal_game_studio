// DinosaurBase.cpp
// Engine Architect #02 — Cycle AUTO_20260629_004
// Base class for all dinosaur pawns in the prehistoric survival game.
// Implements movement, AI perception setup, health, and species traits.

#include "DinosaurBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISense_Hearing.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"

ADinosaurBase::ADinosaurBase()
{
    PrimaryActorTick.bCanEverTick = true;

    // Capsule defaults — overridden per species in child classes
    GetCapsuleComponent()->InitCapsuleSize(60.0f, 120.0f);

    // Movement defaults
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        MoveComp->MaxWalkSpeed = 400.0f;
        MoveComp->JumpZVelocity = 500.0f;
        MoveComp->GravityScale = 1.2f;
        MoveComp->bOrientRotationToMovement = true;
        MoveComp->RotationRate = FRotator(0.0f, 360.0f, 0.0f);
        MoveComp->bUseControllerDesiredRotation = false;
    }

    // Don't use controller rotation for mesh
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    // AI Perception stimuli source
    PerceptionStimuliSource = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>(TEXT("PerceptionStimuliSource"));
    if (PerceptionStimuliSource)
    {
        PerceptionStimuliSource->RegisterForSense(TSubclassOf<UAISense>(UAISense_Sight::StaticClass()));
        PerceptionStimuliSource->RegisterForSense(TSubclassOf<UAISense>(UAISense_Hearing::StaticClass()));
        PerceptionStimuliSource->bAutoRegister = true;
    }

    // Default species data
    SpeciesName = FName("Unknown");
    MaxHealth = 500.0f;
    CurrentHealth = 500.0f;
    AttackDamage = 50.0f;
    DetectionRadius = 1500.0f;
    AttackRadius = 200.0f;
    bIsCarnivore = true;
    bIsPackHunter = false;
    TerritoryRadius = 3000.0f;
    CurrentBehaviorState = EDinosaurBehaviorState::Idle;

    // Replication
    bReplicates = true;
    SetReplicatingMovement(true);
}

void ADinosaurBase::BeginPlay()
{
    Super::BeginPlay();

    CurrentHealth = MaxHealth;

    // Start idle behavior timer
    GetWorldTimerManager().SetTimer(
        BehaviorUpdateTimer,
        this,
        &ADinosaurBase::UpdateBehaviorState,
        2.0f,
        true
    );
}

void ADinosaurBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Clamp health
    CurrentHealth = FMath::Clamp(CurrentHealth, 0.0f, MaxHealth);

    // Die if health depleted
    if (CurrentHealth <= 0.0f && CurrentBehaviorState != EDinosaurBehaviorState::Dead)
    {
        Die();
    }
}

void ADinosaurBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ADinosaurBase, CurrentHealth);
    DOREPLIFETIME(ADinosaurBase, CurrentBehaviorState);
}

float ADinosaurBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    CurrentHealth -= ActualDamage;

    if (CurrentHealth <= 0.0f)
    {
        Die();
    }
    else
    {
        // React to being hit — switch to alert/aggressive
        if (CurrentBehaviorState == EDinosaurBehaviorState::Idle ||
            CurrentBehaviorState == EDinosaurBehaviorState::Roaming)
        {
            SetBehaviorState(EDinosaurBehaviorState::Aggressive);
        }
    }

    return ActualDamage;
}

void ADinosaurBase::SetBehaviorState(EDinosaurBehaviorState NewState)
{
    if (CurrentBehaviorState == NewState) return;

    CurrentBehaviorState = NewState;
    OnBehaviorStateChanged(NewState);
}

void ADinosaurBase::OnBehaviorStateChanged(EDinosaurBehaviorState NewState)
{
    // Override in child classes for species-specific behavior transitions
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (!MoveComp) return;

    switch (NewState)
    {
    case EDinosaurBehaviorState::Idle:
        MoveComp->MaxWalkSpeed = 0.0f;
        break;
    case EDinosaurBehaviorState::Roaming:
        MoveComp->MaxWalkSpeed = 200.0f;
        break;
    case EDinosaurBehaviorState::Hunting:
        MoveComp->MaxWalkSpeed = 600.0f;
        break;
    case EDinosaurBehaviorState::Aggressive:
        MoveComp->MaxWalkSpeed = 700.0f;
        break;
    case EDinosaurBehaviorState::Fleeing:
        MoveComp->MaxWalkSpeed = 800.0f;
        break;
    case EDinosaurBehaviorState::Feeding:
        MoveComp->MaxWalkSpeed = 0.0f;
        break;
    case EDinosaurBehaviorState::Dead:
        MoveComp->MaxWalkSpeed = 0.0f;
        MoveComp->DisableMovement();
        break;
    default:
        break;
    }
}

void ADinosaurBase::UpdateBehaviorState()
{
    // Base idle/roam cycle — AI controller overrides this with BT
    if (CurrentBehaviorState == EDinosaurBehaviorState::Dead) return;

    if (CurrentBehaviorState == EDinosaurBehaviorState::Idle)
    {
        // 40% chance to start roaming
        if (FMath::RandRange(0, 100) < 40)
        {
            SetBehaviorState(EDinosaurBehaviorState::Roaming);
        }
    }
    else if (CurrentBehaviorState == EDinosaurBehaviorState::Roaming)
    {
        // 20% chance to return to idle
        if (FMath::RandRange(0, 100) < 20)
        {
            SetBehaviorState(EDinosaurBehaviorState::Idle);
        }
    }
}

void ADinosaurBase::Die()
{
    SetBehaviorState(EDinosaurBehaviorState::Dead);

    // Disable collision
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Notify world (loot drops, quest events, etc.)
    OnDinosaurDied.Broadcast(this);

    // Destroy after delay (allow death animation to play)
    SetLifeSpan(10.0f);
}

bool ADinosaurBase::IsAlive() const
{
    return CurrentHealth > 0.0f && CurrentBehaviorState != EDinosaurBehaviorState::Dead;
}

float ADinosaurBase::GetHealthPercent() const
{
    if (MaxHealth <= 0.0f) return 0.0f;
    return CurrentHealth / MaxHealth;
}
