// DinosaurBase.cpp
// Transpersonal Game Studio — Engine Architect #02
// Base class for all dinosaur pawns. Provides survival stats, AI perception,
// locomotion parameters, and damage response. All species inherit from this.

#include "DinosaurBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "AIController.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"

ADinosaurBase::ADinosaurBase()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.05f; // 20Hz tick for AI efficiency

    // --- Capsule ---
    GetCapsuleComponent()->InitCapsuleSize(50.0f, 100.0f);
    GetCapsuleComponent()->SetCollisionProfileName(TEXT("Pawn"));

    // --- Movement ---
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 360.0f, 0.0f);
    GetCharacterMovement()->MaxWalkSpeed = 400.0f;
    GetCharacterMovement()->MaxWalkSpeedCrouched = 200.0f;
    GetCharacterMovement()->JumpZVelocity = 400.0f;
    GetCharacterMovement()->GravityScale = 1.0f;
    GetCharacterMovement()->NavAgentProps.bCanCrouch = false;
    GetCharacterMovement()->NavAgentProps.bCanJump = true;
    GetCharacterMovement()->NavAgentProps.AgentRadius = 50.0f;
    GetCharacterMovement()->NavAgentProps.AgentHeight = 200.0f;

    bUseControllerRotationYaw = false;

    // --- AI Perception ---
    AIPerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComp"));

    UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = 2000.0f;
    SightConfig->LoseSightRadius = 2500.0f;
    SightConfig->PeripheralVisionAngleDegrees = 90.0f;
    SightConfig->SetMaxAge(5.0f);
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
    AIPerceptionComp->ConfigureSense(*SightConfig);

    UAISenseConfig_Hearing* HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    HearingConfig->HearingRange = 1500.0f;
    HearingConfig->SetMaxAge(3.0f);
    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
    HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
    AIPerceptionComp->ConfigureSense(*HearingConfig);
    AIPerceptionComp->SetDominantSense(SightConfig->GetSenseImplementation());

    // --- Default Species Stats ---
    SpeciesName = FName("Unknown");
    DinoClass = EDinoClass::Herbivore;
    CurrentHealth = 100.0f;
    MaxHealth = 100.0f;
    CurrentHunger = 100.0f;
    MaxHunger = 100.0f;
    CurrentThirst = 100.0f;
    MaxThirst = 100.0f;
    AttackDamage = 20.0f;
    AttackRange = 150.0f;
    DetectionRange = 2000.0f;
    FleeThreshold = 0.25f; // Flee when health < 25%
    bIsAggressive = false;
    bIsPack = false;
    PackSize = 1;
    CurrentBehaviorState = EDinoBehaviorState::Idle;
    bIsDead = false;
    HungerDecayRate = 0.5f;  // per second
    ThirstDecayRate = 0.8f;  // per second
}

void ADinosaurBase::BeginPlay()
{
    Super::BeginPlay();

    // Bind perception delegate
    if (AIPerceptionComp)
    {
        AIPerceptionComp->OnTargetPerceptionUpdated.AddDynamic(this, &ADinosaurBase::OnTargetPerceptionUpdated);
    }

    // Start survival stat decay timer
    GetWorldTimerManager().SetTimer(
        SurvivalDecayTimer,
        this,
        &ADinosaurBase::TickSurvivalStats,
        1.0f,
        true
    );

    // Initialize behavior
    SetBehaviorState(EDinoBehaviorState::Idle);
}

void ADinosaurBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update locomotion speed based on behavior state
    UpdateLocomotionSpeed();
}

float ADinosaurBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    if (bIsDead) return 0.0f;

    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
    CurrentHealth = FMath::Clamp(CurrentHealth - ActualDamage, 0.0f, MaxHealth);

    // React to damage
    if (CurrentHealth <= 0.0f)
    {
        Die();
    }
    else if (CurrentHealth / MaxHealth <= FleeThreshold)
    {
        // Flee if health is critical and not aggressive
        if (!bIsAggressive)
        {
            SetBehaviorState(EDinoBehaviorState::Fleeing);
        }
        else
        {
            SetBehaviorState(EDinoBehaviorState::Attacking);
        }
    }
    else
    {
        // Alert state — investigate threat
        SetBehaviorState(EDinoBehaviorState::Alert);
    }

    return ActualDamage;
}

void ADinosaurBase::Die()
{
    if (bIsDead) return;
    bIsDead = true;

    SetBehaviorState(EDinoBehaviorState::Dead);

    // Stop all timers
    GetWorldTimerManager().ClearTimer(SurvivalDecayTimer);

    // Disable collision
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    GetCharacterMovement()->DisableMovement();

    // Ragdoll
    GetMesh()->SetSimulatePhysics(true);
    GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

    // Notify AI controller
    if (AController* Ctrl = GetController())
    {
        Ctrl->UnPossess();
    }

    // Destroy after 30 seconds (corpse decay)
    SetLifeSpan(30.0f);
}

void ADinosaurBase::SetBehaviorState(EDinoBehaviorState NewState)
{
    if (CurrentBehaviorState == NewState) return;

    EDinoBehaviorState OldState = CurrentBehaviorState;
    CurrentBehaviorState = NewState;

    OnBehaviorStateChanged(OldState, NewState);
}

void ADinosaurBase::OnBehaviorStateChanged(EDinoBehaviorState OldState, EDinoBehaviorState NewState)
{
    // Override in subclasses for species-specific behavior transitions
    // Base implementation: update movement speed
    UpdateLocomotionSpeed();
}

void ADinosaurBase::UpdateLocomotionSpeed()
{
    if (!GetCharacterMovement()) return;

    float TargetSpeed = 0.0f;
    switch (CurrentBehaviorState)
    {
        case EDinoBehaviorState::Idle:
            TargetSpeed = 0.0f;
            break;
        case EDinoBehaviorState::Wandering:
            TargetSpeed = GetCharacterMovement()->MaxWalkSpeed * 0.4f;
            break;
        case EDinoBehaviorState::Foraging:
            TargetSpeed = GetCharacterMovement()->MaxWalkSpeed * 0.3f;
            break;
        case EDinoBehaviorState::Alert:
            TargetSpeed = GetCharacterMovement()->MaxWalkSpeed * 0.6f;
            break;
        case EDinoBehaviorState::Chasing:
            TargetSpeed = GetCharacterMovement()->MaxWalkSpeed;
            break;
        case EDinoBehaviorState::Attacking:
            TargetSpeed = GetCharacterMovement()->MaxWalkSpeed * 0.8f;
            break;
        case EDinoBehaviorState::Fleeing:
            TargetSpeed = GetCharacterMovement()->MaxWalkSpeed * 1.2f;
            break;
        case EDinoBehaviorState::Resting:
            TargetSpeed = 0.0f;
            break;
        case EDinoBehaviorState::Dead:
            TargetSpeed = 0.0f;
            break;
    }

    GetCharacterMovement()->MaxWalkSpeed = FMath::Clamp(TargetSpeed, 0.0f, 1200.0f);
}

void ADinosaurBase::TickSurvivalStats()
{
    if (bIsDead) return;

    // Decay hunger and thirst over time
    CurrentHunger = FMath::Clamp(CurrentHunger - HungerDecayRate, 0.0f, MaxHunger);
    CurrentThirst = FMath::Clamp(CurrentThirst - ThirstDecayRate, 0.0f, MaxThirst);

    // Starvation / dehydration damage
    if (CurrentHunger <= 0.0f || CurrentThirst <= 0.0f)
    {
        TakeDamage(2.0f, FDamageEvent(), nullptr, nullptr);
    }

    // Trigger foraging behavior when hungry
    if (CurrentHunger < MaxHunger * 0.3f && CurrentBehaviorState == EDinoBehaviorState::Idle)
    {
        SetBehaviorState(EDinoBehaviorState::Foraging);
    }
}

void ADinosaurBase::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (!Actor || bIsDead) return;

    // Determine if the perceived actor is a threat (player or predator)
    bool bIsThreat = false;
    bool bIsPrey = false;

    // Check if it's the player character
    if (Actor->ActorHasTag(FName("Player")))
    {
        bIsThreat = (DinoClass == EDinoClass::Herbivore);
        bIsPrey = (DinoClass == EDinoClass::Carnivore || DinoClass == EDinoClass::Omnivore);
    }

    if (Stimulus.WasSuccessfullySensed())
    {
        if (bIsThreat && !bIsAggressive)
        {
            SetBehaviorState(EDinoBehaviorState::Fleeing);
        }
        else if (bIsPrey || bIsAggressive)
        {
            SetBehaviorState(EDinoBehaviorState::Chasing);
        }
        else
        {
            SetBehaviorState(EDinoBehaviorState::Alert);
        }
    }
    else
    {
        // Lost sight/sound — return to wandering
        if (CurrentBehaviorState == EDinoBehaviorState::Chasing ||
            CurrentBehaviorState == EDinoBehaviorState::Alert)
        {
            SetBehaviorState(EDinoBehaviorState::Wandering);
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
