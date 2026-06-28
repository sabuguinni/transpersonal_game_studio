// DinosaurBase.cpp
// Engine Architect #02 — Transpersonal Game Studio
// Cycle: PROD_CYCLE_AUTO_20260628_003
// Base class for all dinosaur pawns in the prehistoric survival game.

#include "DinosaurBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "AIController.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"

ADinosaurBase::ADinosaurBase()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.05f; // 20Hz tick for AI performance

    // --- Capsule ---
    GetCapsuleComponent()->InitCapsuleSize(42.f, 96.f);
    GetCapsuleComponent()->SetCollisionProfileName(TEXT("Pawn"));

    // --- Skeletal Mesh ---
    GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -96.f));
    GetMesh()->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));

    // --- Movement ---
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    MoveComp->bOrientRotationToMovement = true;
    MoveComp->RotationRate = FRotator(0.f, 540.f, 0.f);
    MoveComp->MaxWalkSpeed = 600.f;
    MoveComp->JumpZVelocity = 400.f;
    MoveComp->AirControl = 0.2f;
    MoveComp->NavAgentProps.bCanJump = true;
    MoveComp->NavAgentProps.bCanWalk = true;
    MoveComp->NavAgentProps.bCanSwim = false;

    bUseControllerRotationYaw = false;
    bUseControllerRotationPitch = false;
    bUseControllerRotationRoll = false;

    // --- AI Perception ---
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));

    UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = 2000.f;
    SightConfig->LoseSightRadius = 2500.f;
    SightConfig->PeripheralVisionAngleDegrees = 90.f;
    SightConfig->SetMaxAge(5.f);
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
    AIPerceptionComponent->ConfigureSense(*SightConfig);

    UAISenseConfig_Hearing* HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    HearingConfig->HearingRange = 1500.f;
    HearingConfig->SetMaxAge(3.f);
    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
    AIPerceptionComponent->ConfigureSense(*HearingConfig);
    AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());

    // --- Default Stats ---
    MaxHealth = 500.f;
    CurrentHealth = 500.f;
    MaxStamina = 200.f;
    CurrentStamina = 200.f;
    AttackDamage = 50.f;
    AttackRange = 200.f;
    DetectionRadius = 2000.f;
    WalkSpeed = 300.f;
    RunSpeed = 700.f;
    bIsAggressive = false;
    bIsAlive = true;
    DinosaurSpecies = EDinosaurSpecies::Unknown;
    CurrentBehaviorState = EDinosaurBehaviorState::Idle;
    Diet = EDinosaurDiet::Carnivore;

    // --- Replication ---
    bReplicates = true;
    SetReplicateMovement(true);
}

void ADinosaurBase::BeginPlay()
{
    Super::BeginPlay();

    CurrentHealth = MaxHealth;
    CurrentStamina = MaxStamina;

    // Apply species-specific movement speed
    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        MoveComp->MaxWalkSpeed = WalkSpeed;
    }

    // Start idle behavior timer
    GetWorldTimerManager().SetTimer(
        BehaviorUpdateTimer,
        this,
        &ADinosaurBase::UpdateBehaviorState,
        1.0f,
        true
    );

    // Bind perception delegate
    if (AIPerceptionComponent)
    {
        AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(
            this,
            &ADinosaurBase::OnPerceptionUpdated
        );
    }
}

void ADinosaurBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Stamina regeneration when not running
    if (CurrentStamina < MaxStamina && CurrentBehaviorState != EDinosaurBehaviorState::Fleeing)
    {
        CurrentStamina = FMath::Min(MaxStamina, CurrentStamina + (DeltaTime * 10.f));
    }
}

void ADinosaurBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ADinosaurBase, CurrentHealth);
    DOREPLIFETIME(ADinosaurBase, CurrentStamina);
    DOREPLIFETIME(ADinosaurBase, bIsAlive);
    DOREPLIFETIME(ADinosaurBase, CurrentBehaviorState);
}

float ADinosaurBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    if (!bIsAlive) return 0.f;

    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
    CurrentHealth = FMath::Max(0.f, CurrentHealth - ActualDamage);

    if (CurrentHealth <= 0.f)
    {
        Die();
    }
    else if (bIsAggressive && DamageCauser)
    {
        // Become aggressive toward attacker
        CurrentBehaviorState = EDinosaurBehaviorState::Attacking;
        TargetActor = DamageCauser;
    }

    return ActualDamage;
}

void ADinosaurBase::Die()
{
    if (!bIsAlive) return;

    bIsAlive = false;
    CurrentBehaviorState = EDinosaurBehaviorState::Dead;

    // Disable collision and movement
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    GetCharacterMovement()->DisableMovement();

    // Notify Blueprint for death animation
    OnDeath();

    // Destroy after delay (allow death animation to play)
    SetLifeSpan(10.f);
}

void ADinosaurBase::Attack(AActor* Target)
{
    if (!bIsAlive || !Target) return;
    if (CurrentStamina < 20.f) return;

    float DistToTarget = FVector::Dist(GetActorLocation(), Target->GetActorLocation());
    if (DistToTarget <= AttackRange)
    {
        // Apply damage to target
        UGameplayStatics::ApplyDamage(Target, AttackDamage, GetController(), this, nullptr);
        CurrentStamina = FMath::Max(0.f, CurrentStamina - 20.f);
        OnAttack(Target);
    }
}

void ADinosaurBase::SetBehaviorState(EDinosaurBehaviorState NewState)
{
    if (CurrentBehaviorState == NewState) return;
    CurrentBehaviorState = NewState;

    // Adjust movement speed based on state
    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        switch (NewState)
        {
        case EDinosaurBehaviorState::Idle:
        case EDinosaurBehaviorState::Grazing:
            MoveComp->MaxWalkSpeed = WalkSpeed * 0.3f;
            break;
        case EDinosaurBehaviorState::Patrolling:
        case EDinosaurBehaviorState::Wandering:
            MoveComp->MaxWalkSpeed = WalkSpeed;
            break;
        case EDinosaurBehaviorState::Chasing:
        case EDinosaurBehaviorState::Fleeing:
        case EDinosaurBehaviorState::Attacking:
            MoveComp->MaxWalkSpeed = RunSpeed;
            break;
        default:
            MoveComp->MaxWalkSpeed = WalkSpeed;
            break;
        }
    }
}

void ADinosaurBase::UpdateBehaviorState()
{
    if (!bIsAlive) return;

    // Simple state machine — override in subclasses for species-specific behavior
    switch (CurrentBehaviorState)
    {
    case EDinosaurBehaviorState::Idle:
        // Randomly transition to wandering
        if (FMath::RandBool())
        {
            SetBehaviorState(EDinosaurBehaviorState::Wandering);
        }
        break;
    case EDinosaurBehaviorState::Wandering:
        // Randomly transition back to idle or grazing
        if (FMath::RandRange(0, 3) == 0)
        {
            SetBehaviorState(Diet == EDinosaurDiet::Herbivore
                ? EDinosaurBehaviorState::Grazing
                : EDinosaurBehaviorState::Idle);
        }
        break;
    case EDinosaurBehaviorState::Attacking:
        if (!TargetActor || !TargetActor->IsValidLowLevel())
        {
            SetBehaviorState(EDinosaurBehaviorState::Idle);
            TargetActor = nullptr;
        }
        break;
    default:
        break;
    }
}

void ADinosaurBase::OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (!bIsAlive || !Actor) return;

    // Carnivores become aggressive toward player/humans
    if (bIsAggressive && Stimulus.WasSuccessfullySensed())
    {
        if (Actor->ActorHasTag(TEXT("Player")) || Actor->ActorHasTag(TEXT("Human")))
        {
            TargetActor = Actor;
            SetBehaviorState(EDinosaurBehaviorState::Chasing);
        }
    }
    else if (!Stimulus.WasSuccessfullySensed() && TargetActor == Actor)
    {
        // Lost sight of target
        SetBehaviorState(EDinosaurBehaviorState::Patrolling);
        TargetActor = nullptr;
    }
}

void ADinosaurBase::OnDeath_Implementation()
{
    // Default: play ragdoll
    GetMesh()->SetSimulatePhysics(true);
    GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}

void ADinosaurBase::OnAttack_Implementation(AActor* Target)
{
    // Override in Blueprint for attack animation/sound
}
