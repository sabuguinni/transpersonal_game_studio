// DinosaurBase.cpp
// Engine Architect #02 — Transpersonal Game Studio
// Base implementation for all dinosaur species in the prehistoric survival game.

#include "DinosaurBase.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISense_Hearing.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

ADinosaurBase::ADinosaurBase()
{
    PrimaryActorTick.bCanEverTick = true;

    // Configure capsule
    GetCapsuleComponent()->SetCapsuleHalfHeight(88.0f);
    GetCapsuleComponent()->SetCapsuleRadius(34.0f);
    GetCapsuleComponent()->SetCollisionProfileName(TEXT("Pawn"));

    // Configure skeletal mesh
    GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -88.0f));
    GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));

    // Configure movement
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        MoveComp->bOrientRotationToMovement = true;
        MoveComp->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
        MoveComp->MaxWalkSpeed = WalkSpeed;
        MoveComp->JumpZVelocity = 400.0f;
        MoveComp->AirControl = 0.2f;
        MoveComp->NavAgentProps.AgentRadius = 34.0f;
        MoveComp->NavAgentProps.AgentHeight = 176.0f;
    }

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

    // Default stats
    MaxHealth = 500.0f;
    CurrentHealth = MaxHealth;
    WalkSpeed = 300.0f;
    RunSpeed = 700.0f;
    AttackDamage = 50.0f;
    AttackRange = 150.0f;
    DetectionRange = 2000.0f;
    AttackCooldown = 2.0f;
    bIsAggressive = false;
    bIsAlive = true;
    CurrentBehaviorState = EDinoBehaviorState::Idle;
    Species = EDinoSpecies::TRex;
}

void ADinosaurBase::BeginPlay()
{
    Super::BeginPlay();

    CurrentHealth = MaxHealth;
    bIsAlive = true;
    CurrentBehaviorState = EDinoBehaviorState::Idle;

    // Start idle behavior loop
    GetWorldTimerManager().SetTimer(
        BehaviorUpdateTimer,
        this,
        &ADinosaurBase::UpdateBehavior,
        2.0f,
        true,
        FMath::RandRange(0.5f, 2.0f)
    );
}

void ADinosaurBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Reduce attack cooldown
    if (CurrentAttackCooldown > 0.0f)
    {
        CurrentAttackCooldown -= DeltaTime;
    }
}

float ADinosaurBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    if (!bIsAlive) return 0.0f;

    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
    ActualDamage = FMath::Max(0.0f, ActualDamage);

    CurrentHealth -= ActualDamage;
    CurrentHealth = FMath::Clamp(CurrentHealth, 0.0f, MaxHealth);

    OnDinosaurDamaged(ActualDamage, DamageCauser);

    if (CurrentHealth <= 0.0f)
    {
        Die();
    }
    else if (!bIsAggressive && ActualDamage > 0.0f)
    {
        // Herbivores flee when hit
        SetBehaviorState(EDinoBehaviorState::Fleeing);
    }

    return ActualDamage;
}

void ADinosaurBase::Die()
{
    if (!bIsAlive) return;

    bIsAlive = false;
    CurrentBehaviorState = EDinoBehaviorState::Dead;

    GetWorldTimerManager().ClearTimer(BehaviorUpdateTimer);

    // Disable collision
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Disable movement
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->DisableMovement();
    }

    OnDinosaurDied();

    // Destroy after 30 seconds (corpse despawn)
    SetLifeSpan(30.0f);
}

void ADinosaurBase::SetBehaviorState(EDinoBehaviorState NewState)
{
    if (CurrentBehaviorState == NewState) return;

    EDinoBehaviorState OldState = CurrentBehaviorState;
    CurrentBehaviorState = NewState;

    // Update movement speed based on state
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        switch (NewState)
        {
        case EDinoBehaviorState::Idle:
        case EDinoBehaviorState::Grazing:
        case EDinoBehaviorState::Resting:
            MoveComp->MaxWalkSpeed = WalkSpeed * 0.3f;
            break;
        case EDinoBehaviorState::Wandering:
        case EDinoBehaviorState::Patrolling:
            MoveComp->MaxWalkSpeed = WalkSpeed;
            break;
        case EDinoBehaviorState::Chasing:
        case EDinoBehaviorState::Fleeing:
            MoveComp->MaxWalkSpeed = RunSpeed;
            break;
        case EDinoBehaviorState::Attacking:
            MoveComp->MaxWalkSpeed = RunSpeed * 1.1f;
            break;
        default:
            MoveComp->MaxWalkSpeed = WalkSpeed;
            break;
        }
    }

    OnBehaviorStateChanged(OldState, NewState);
}

bool ADinosaurBase::CanAttack() const
{
    return bIsAlive && bIsAggressive && CurrentAttackCooldown <= 0.0f;
}

void ADinosaurBase::PerformAttack(AActor* Target)
{
    if (!CanAttack() || !Target) return;

    float DistToTarget = FVector::Dist(GetActorLocation(), Target->GetActorLocation());
    if (DistToTarget > AttackRange) return;

    // Apply damage to target
    UGameplayStatics::ApplyDamage(Target, AttackDamage, GetController(), this, nullptr);
    CurrentAttackCooldown = AttackCooldown;

    SetBehaviorState(EDinoBehaviorState::Attacking);
    OnDinosaurAttacked(Target);
}

void ADinosaurBase::UpdateBehavior()
{
    if (!bIsAlive) return;

    // Simple idle/wander behavior — AI controller handles complex logic
    if (CurrentBehaviorState == EDinoBehaviorState::Idle)
    {
        // 30% chance to start wandering
        if (FMath::RandRange(0.0f, 1.0f) < 0.3f)
        {
            SetBehaviorState(EDinoBehaviorState::Wandering);
        }
    }
    else if (CurrentBehaviorState == EDinoBehaviorState::Wandering)
    {
        // 40% chance to return to idle
        if (FMath::RandRange(0.0f, 1.0f) < 0.4f)
        {
            SetBehaviorState(EDinoBehaviorState::Idle);
        }
    }
}

void ADinosaurBase::OnDinosaurDamaged_Implementation(float DamageAmount, AActor* DamageCauser)
{
    // Override in Blueprint for visual/audio feedback
}

void ADinosaurBase::OnDinosaurDied_Implementation()
{
    // Override in Blueprint for death animation/effects
}

void ADinosaurBase::OnDinosaurAttacked_Implementation(AActor* Target)
{
    // Override in Blueprint for attack animation/effects
}

void ADinosaurBase::OnBehaviorStateChanged_Implementation(EDinoBehaviorState OldState, EDinoBehaviorState NewState)
{
    // Override in Blueprint for state-driven animation changes
}

float ADinosaurBase::GetHealthPercent() const
{
    if (MaxHealth <= 0.0f) return 0.0f;
    return CurrentHealth / MaxHealth;
}
