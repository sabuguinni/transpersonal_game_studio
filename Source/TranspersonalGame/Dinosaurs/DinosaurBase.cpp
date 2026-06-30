// DinosaurBase.cpp
// Transpersonal Game Studio — Engine Architect #02
// Base class for all dinosaur actors in the prehistoric survival game

#include "DinosaurBase.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISense_Hearing.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "AIController.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"

ADinosaurBase::ADinosaurBase()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // 10Hz tick for AI — performance friendly

    // Capsule collision
    GetCapsuleComponent()->SetCapsuleHalfHeight(88.0f);
    GetCapsuleComponent()->SetCapsuleRadius(34.0f);
    GetCapsuleComponent()->SetCollisionProfileName(TEXT("Pawn"));

    // Mesh setup
    GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -88.0f));
    GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));

    // Movement component defaults
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        MoveComp->bOrientRotationToMovement = true;
        MoveComp->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
        MoveComp->MaxWalkSpeed = 400.0f;
        MoveComp->JumpZVelocity = 400.0f;
        MoveComp->AirControl = 0.2f;
        MoveComp->bUseControllerDesiredRotation = false;
        MoveComp->NavAgentProps.AgentRadius = 34.0f;
        MoveComp->NavAgentProps.AgentHeight = 176.0f;
    }

    // AI Perception stimuli source (so AI can detect this dino)
    PerceptionStimuliSource = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>(TEXT("PerceptionStimuliSource"));
    if (PerceptionStimuliSource)
    {
        PerceptionStimuliSource->RegisterForSense(TSubclassOf<UAISense>(UAISense_Sight::StaticClass()));
        PerceptionStimuliSource->RegisterForSense(TSubclassOf<UAISense>(UAISense_Hearing::StaticClass()));
        PerceptionStimuliSource->bAutoRegister = true;
    }

    // Default stats — override in subclasses
    MaxHealth = 100.0f;
    CurrentHealth = 100.0f;
    AttackDamage = 20.0f;
    AttackRange = 150.0f;
    DetectionRadius = 1500.0f;
    AggroRadius = 800.0f;
    WalkSpeed = 200.0f;
    RunSpeed = 600.0f;
    bIsAggressive = false;
    bIsPack = false;
    DinosaurSpecies = EDinosaurSpecies::Unknown;
    CurrentBehaviorState = EDinosaurBehavior::Idle;

    // Auto-possess by AI
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
    AIControllerClass = AAIController::StaticClass();
}

void ADinosaurBase::BeginPlay()
{
    Super::BeginPlay();

    CurrentHealth = MaxHealth;

    // Apply walk speed
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        MoveComp->MaxWalkSpeed = WalkSpeed;
    }

    // Start idle behavior timer
    GetWorldTimerManager().SetTimer(
        BehaviorUpdateTimer,
        this,
        &ADinosaurBase::UpdateBehaviorState,
        2.0f,
        true
    );

    UE_LOG(LogTemp, Log, TEXT("DinosaurBase: %s spawned — Species: %d, Health: %.0f"),
        *GetName(), (int32)DinosaurSpecies, CurrentHealth);
}

void ADinosaurBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    // Behavior logic handled by AI Controller + BehaviorTree
    // Tick kept minimal for performance
}

float ADinosaurBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    CurrentHealth = FMath::Clamp(CurrentHealth - ActualDamage, 0.0f, MaxHealth);

    UE_LOG(LogTemp, Log, TEXT("DinosaurBase: %s took %.1f damage — Health: %.1f/%.1f"),
        *GetName(), ActualDamage, CurrentHealth, MaxHealth);

    // Trigger aggro on damage
    if (ActualDamage > 0.0f && !bIsAggressive)
    {
        bIsAggressive = true;
        SetBehaviorState(EDinosaurBehavior::Aggressive);
    }

    if (CurrentHealth <= 0.0f)
    {
        OnDeath();
    }

    return ActualDamage;
}

void ADinosaurBase::OnDeath()
{
    SetBehaviorState(EDinosaurBehavior::Dead);

    // Disable collision and AI
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        MoveComp->DisableMovement();
    }

    // Detach controller
    if (GetController())
    {
        GetController()->UnPossess();
    }

    // Ragdoll on death
    GetMesh()->SetSimulatePhysics(true);
    GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

    UE_LOG(LogTemp, Log, TEXT("DinosaurBase: %s died"), *GetName());

    // Destroy after 30 seconds (cleanup)
    GetWorldTimerManager().SetTimer(
        DestroyTimer,
        this,
        &ADinosaurBase::DestroyAfterDeath,
        30.0f,
        false
    );
}

void ADinosaurBase::DestroyAfterDeath()
{
    Destroy();
}

void ADinosaurBase::SetBehaviorState(EDinosaurBehavior NewState)
{
    if (CurrentBehaviorState == NewState) return;

    EDinosaurBehavior OldState = CurrentBehaviorState;
    CurrentBehaviorState = NewState;

    // Apply speed based on state
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        switch (NewState)
        {
        case EDinosaurBehavior::Idle:
        case EDinosaurBehavior::Grazing:
            MoveComp->MaxWalkSpeed = WalkSpeed;
            break;
        case EDinosaurBehavior::Patrolling:
        case EDinosaurBehavior::Fleeing:
            MoveComp->MaxWalkSpeed = WalkSpeed * 1.5f;
            break;
        case EDinosaurBehavior::Hunting:
        case EDinosaurBehavior::Aggressive:
        case EDinosaurBehavior::Chasing:
            MoveComp->MaxWalkSpeed = RunSpeed;
            break;
        case EDinosaurBehavior::Dead:
            MoveComp->DisableMovement();
            break;
        default:
            break;
        }
    }

    OnBehaviorStateChanged(OldState, NewState);
}

void ADinosaurBase::OnBehaviorStateChanged(EDinosaurBehavior OldState, EDinosaurBehavior NewState)
{
    // Override in subclasses for specific behavior transitions
    UE_LOG(LogTemp, Verbose, TEXT("DinosaurBase: %s behavior %d -> %d"),
        *GetName(), (int32)OldState, (int32)NewState);
}

void ADinosaurBase::UpdateBehaviorState()
{
    if (CurrentBehaviorState == EDinosaurBehavior::Dead) return;

    // Simple idle/patrol cycle — BehaviorTree overrides this in full AI
    if (CurrentBehaviorState == EDinosaurBehavior::Idle)
    {
        // 30% chance to start patrolling
        if (FMath::RandRange(0, 100) < 30)
        {
            SetBehaviorState(EDinosaurBehavior::Patrolling);
        }
    }
    else if (CurrentBehaviorState == EDinosaurBehavior::Patrolling)
    {
        // 20% chance to return to idle
        if (FMath::RandRange(0, 100) < 20)
        {
            SetBehaviorState(EDinosaurBehavior::Idle);
        }
    }
}

bool ADinosaurBase::CanAttack() const
{
    return CurrentBehaviorState != EDinosaurBehavior::Dead
        && CurrentHealth > 0.0f;
}

float ADinosaurBase::GetHealthPercent() const
{
    if (MaxHealth <= 0.0f) return 0.0f;
    return CurrentHealth / MaxHealth;
}
