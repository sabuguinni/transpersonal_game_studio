// DinosaurBase.cpp
// Core Systems Programmer — Agent #03
// Prehistoric survival game — realistic dinosaur base class implementation

#include "DinosaurBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

ADinosaurBase::ADinosaurBase()
{
    PrimaryActorTick.bCanEverTick = true;

    // Capsule defaults — overridden per species in child classes
    GetCapsuleComponent()->InitCapsuleSize(60.0f, 120.0f);

    // Movement defaults
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    MoveComp->MaxWalkSpeed = 400.0f;
    MoveComp->MaxWalkSpeedCrouched = 200.0f;
    MoveComp->JumpZVelocity = 500.0f;
    MoveComp->GravityScale = 1.2f;
    MoveComp->bOrientRotationToMovement = true;
    MoveComp->RotationRate = FRotator(0.0f, 360.0f, 0.0f);
    bUseControllerRotationYaw = false;

    // AI Perception
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

    // Species defaults (overridden per species)
    SpeciesName = TEXT("Unknown");
    DinosaurType = EDinosaurType::Herbivore;
    CurrentBehaviorState = EDinosaurBehaviorState::Idle;

    MaxHealth = 500.0f;
    CurrentHealth = MaxHealth;
    AttackDamage = 50.0f;
    AttackRange = 200.0f;
    AttackCooldown = 2.0f;
    bCanAttack = true;

    PatrolRadius = 1500.0f;
    ChaseSpeed = 700.0f;
    PatrolSpeed = 200.0f;
    WalkSpeed = 300.0f;

    HungerMax = 100.0f;
    CurrentHunger = HungerMax;
    HungerDrainRate = 0.5f;
    ThirstMax = 100.0f;
    CurrentThirst = ThirstMax;
    ThirstDrainRate = 0.3f;

    TerritoryRadius = 3000.0f;
    bIsTerritory = false;
    PackLeader = nullptr;
    bIsPackLeader = false;
    PackRadius = 800.0f;

    bIsAlerted = false;
    AlertLevel = 0.0f;
    AlertDecayRate = 5.0f;

    LastKnownPlayerLocation = FVector::ZeroVector;
    bHasLineOfSightToPlayer = false;
    TimeSinceLastSeenPlayer = 0.0f;
}

void ADinosaurBase::BeginPlay()
{
    Super::BeginPlay();

    CurrentHealth = MaxHealth;
    HomeLocation = GetActorLocation();

    // Bind perception delegate
    if (AIPerceptionComp)
    {
        AIPerceptionComp->OnTargetPerceptionUpdated.AddDynamic(this, &ADinosaurBase::OnTargetPerceptionUpdated);
    }

    // Start survival tick (hunger/thirst drain)
    GetWorldTimerManager().SetTimer(
        SurvivalTickHandle,
        this,
        &ADinosaurBase::UpdateSurvivalNeeds,
        1.0f,
        true
    );

    // Start behavior update tick
    GetWorldTimerManager().SetTimer(
        BehaviorUpdateHandle,
        this,
        &ADinosaurBase::UpdateBehaviorState,
        0.5f,
        true
    );
}

void ADinosaurBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Alert level decay
    if (AlertLevel > 0.0f)
    {
        AlertLevel = FMath::Max(0.0f, AlertLevel - AlertDecayRate * DeltaTime);
    }

    // Track time since last seen player
    if (!bHasLineOfSightToPlayer)
    {
        TimeSinceLastSeenPlayer += DeltaTime;
    }
}

float ADinosaurBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    CurrentHealth = FMath::Max(0.0f, CurrentHealth - ActualDamage);
    OnDamageTaken(ActualDamage, DamageCauser);

    // Alert on damage
    AlertLevel = FMath::Min(100.0f, AlertLevel + 40.0f);
    bIsAlerted = true;

    if (DamageCauser)
    {
        LastKnownPlayerLocation = DamageCauser->GetActorLocation();
    }

    if (CurrentHealth <= 0.0f)
    {
        Die();
    }

    return ActualDamage;
}

void ADinosaurBase::Die()
{
    // Stop timers
    GetWorldTimerManager().ClearTimer(SurvivalTickHandle);
    GetWorldTimerManager().ClearTimer(BehaviorUpdateHandle);
    GetWorldTimerManager().ClearTimer(AttackCooldownHandle);

    // Disable collision and movement
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    GetCharacterMovement()->DisableMovement();

    OnDeath();
    OnDinosaurDied.Broadcast(this);
}

void ADinosaurBase::PerformAttack(AActor* Target)
{
    if (!Target || !bCanAttack) return;

    float DistToTarget = FVector::Dist(GetActorLocation(), Target->GetActorLocation());
    if (DistToTarget > AttackRange) return;

    // Apply damage
    FDamageEvent DamageEvent;
    Target->TakeDamage(AttackDamage, DamageEvent, GetController(), this);

    OnAttackPerformed(Target);
    OnDinosaurAttacked.Broadcast(this, Target);

    // Start cooldown
    bCanAttack = false;
    GetWorldTimerManager().SetTimer(
        AttackCooldownHandle,
        [this]() { bCanAttack = true; },
        AttackCooldown,
        false
    );
}

bool ADinosaurBase::IsPlayerInRange(float Range) const
{
    APawn* Player = GetWorld() ? GetWorld()->GetFirstPlayerController() ? 
        GetWorld()->GetFirstPlayerController()->GetPawn() : nullptr : nullptr;
    if (!Player) return false;
    return FVector::Dist(GetActorLocation(), Player->GetActorLocation()) <= Range;
}

float ADinosaurBase::GetDistanceToPlayer() const
{
    APawn* Player = GetWorld() ? GetWorld()->GetFirstPlayerController() ?
        GetWorld()->GetFirstPlayerController()->GetPawn() : nullptr : nullptr;
    if (!Player) return TNumericLimits<float>::Max();
    return FVector::Dist(GetActorLocation(), Player->GetActorLocation());
}

void ADinosaurBase::SetBehaviorState(EDinosaurBehaviorState NewState)
{
    if (CurrentBehaviorState == NewState) return;

    EDinosaurBehaviorState OldState = CurrentBehaviorState;
    CurrentBehaviorState = NewState;

    // Update movement speed based on state
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    switch (NewState)
    {
    case EDinosaurBehaviorState::Idle:
    case EDinosaurBehaviorState::Resting:
        MoveComp->MaxWalkSpeed = PatrolSpeed;
        break;
    case EDinosaurBehaviorState::Patrolling:
    case EDinosaurBehaviorState::Grazing:
        MoveComp->MaxWalkSpeed = WalkSpeed;
        break;
    case EDinosaurBehaviorState::Chasing:
    case EDinosaurBehaviorState::Fleeing:
        MoveComp->MaxWalkSpeed = ChaseSpeed;
        break;
    case EDinosaurBehaviorState::Attacking:
        MoveComp->MaxWalkSpeed = ChaseSpeed * 0.5f;
        break;
    default:
        MoveComp->MaxWalkSpeed = WalkSpeed;
        break;
    }

    OnBehaviorStateChanged(OldState, NewState);
    OnBehaviorStateChangedDelegate.Broadcast(this, NewState);
}

void ADinosaurBase::UpdateBehaviorState()
{
    if (CurrentHealth <= 0.0f) return;

    float DistToPlayer = GetDistanceToPlayer();

    // Carnivore logic
    if (DinosaurType == EDinosaurType::Carnivore || DinosaurType == EDinosaurType::Apex)
    {
        if (bIsAlerted && DistToPlayer < 3000.0f)
        {
            if (DistToPlayer <= AttackRange)
            {
                SetBehaviorState(EDinosaurBehaviorState::Attacking);
            }
            else
            {
                SetBehaviorState(EDinosaurBehaviorState::Chasing);
            }
        }
        else if (CurrentBehaviorState == EDinosaurBehaviorState::Chasing ||
                 CurrentBehaviorState == EDinosaurBehaviorState::Attacking)
        {
            // Lost the player
            if (TimeSinceLastSeenPlayer > 10.0f)
            {
                SetBehaviorState(EDinosaurBehaviorState::Patrolling);
                bIsAlerted = false;
            }
        }
        else
        {
            SetBehaviorState(EDinosaurBehaviorState::Patrolling);
        }
    }
    // Herbivore logic
    else
    {
        if (bIsAlerted && DistToPlayer < 2000.0f)
        {
            SetBehaviorState(EDinosaurBehaviorState::Fleeing);
        }
        else if (CurrentBehaviorState == EDinosaurBehaviorState::Fleeing)
        {
            if (DistToPlayer > 3000.0f || TimeSinceLastSeenPlayer > 15.0f)
            {
                SetBehaviorState(EDinosaurBehaviorState::Grazing);
                bIsAlerted = false;
            }
        }
        else
        {
            // Alternate between grazing and patrolling
            if (CurrentBehaviorState == EDinosaurBehaviorState::Idle)
            {
                SetBehaviorState(EDinosaurBehaviorState::Grazing);
            }
        }
    }
}

void ADinosaurBase::UpdateSurvivalNeeds()
{
    // Hunger drain
    CurrentHunger = FMath::Max(0.0f, CurrentHunger - HungerDrainRate);
    if (CurrentHunger <= 0.0f && DinosaurType == EDinosaurType::Carnivore)
    {
        // Starving carnivore becomes more aggressive
        AlertLevel = FMath::Min(100.0f, AlertLevel + 10.0f);
    }

    // Thirst drain
    CurrentThirst = FMath::Max(0.0f, CurrentThirst - ThirstDrainRate);
}

void ADinosaurBase::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (!Actor) return;

    // Check if it's the player
    APawn* PlayerPawn = GetWorld()->GetFirstPlayerController() ?
        GetWorld()->GetFirstPlayerController()->GetPawn() : nullptr;

    if (Actor == PlayerPawn)
    {
        if (Stimulus.WasSuccessfullySensed())
        {
            bHasLineOfSightToPlayer = true;
            TimeSinceLastSeenPlayer = 0.0f;
            LastKnownPlayerLocation = Actor->GetActorLocation();

            // Alert on first sight
            if (!bIsAlerted)
            {
                bIsAlerted = true;
                AlertLevel = 60.0f;
                OnPlayerSpotted(Actor);
                OnDinosaurSpottedPlayer.Broadcast(this, Actor);
            }
            else
            {
                AlertLevel = FMath::Min(100.0f, AlertLevel + 20.0f);
            }
        }
        else
        {
            bHasLineOfSightToPlayer = false;
        }
    }
}

// Blueprint-overridable event stubs
void ADinosaurBase::OnDamageTaken_Implementation(float DamageAmount, AActor* DamageCauser) {}
void ADinosaurBase::OnDeath_Implementation() {}
void ADinosaurBase::OnAttackPerformed_Implementation(AActor* Target) {}
void ADinosaurBase::OnPlayerSpotted_Implementation(AActor* Player) {}
void ADinosaurBase::OnBehaviorStateChanged_Implementation(EDinosaurBehaviorState OldState, EDinosaurBehaviorState NewState) {}

float ADinosaurBase::GetHealthPercent() const
{
    return MaxHealth > 0.0f ? CurrentHealth / MaxHealth : 0.0f;
}

float ADinosaurBase::GetHungerPercent() const
{
    return HungerMax > 0.0f ? CurrentHunger / HungerMax : 0.0f;
}

float ADinosaurBase::GetThirstPercent() const
{
    return ThirstMax > 0.0f ? CurrentThirst / ThirstMax : 0.0f;
}

bool ADinosaurBase::IsDead() const
{
    return CurrentHealth <= 0.0f;
}
