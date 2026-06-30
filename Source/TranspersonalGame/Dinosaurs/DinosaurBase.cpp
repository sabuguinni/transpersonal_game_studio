// DinosaurBase.cpp
// Engine Architect #02 — Transpersonal Game Studio
// Full implementation of the dinosaur base class for all species

#include "DinosaurBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISense_Hearing.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"

ADinosaurBase::ADinosaurBase()
{
    PrimaryActorTick.bCanEverTick = true;

    // Default survival stats
    MaxHealth = 500.0f;
    CurrentHealth = 500.0f;
    MaxStamina = 200.0f;
    CurrentStamina = 200.0f;
    MaxHunger = 100.0f;
    CurrentHunger = 100.0f;

    // Default movement speeds (overridden by species subclasses)
    WalkSpeed = 300.0f;
    RunSpeed = 700.0f;
    AttackRange = 200.0f;
    DetectionRange = 1500.0f;
    AttackDamage = 50.0f;
    AttackCooldown = 2.0f;

    // Default species traits
    DinosaurSpecies = EDinosaurSpecies::TRex;
    CurrentBehaviorState = EDinosaurBehaviorState::Idle;
    bIsAlpha = false;
    bIsJuvenile = false;
    PackLeader = nullptr;

    // Movement component defaults
    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 180.0f, 0.0f);
    GetCharacterMovement()->GravityScale = 1.2f;
    GetCharacterMovement()->JumpZVelocity = 400.0f;

    // Capsule defaults — overridden per species
    GetCapsuleComponent()->SetCapsuleHalfHeight(90.0f);
    GetCapsuleComponent()->SetCapsuleRadius(40.0f);

    // AI Perception stimuli source
    StimuliSource = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>(TEXT("StimuliSource"));
    StimuliSource->RegisterForSense(TSubclassOf<UAISense_Sight>());
    StimuliSource->RegisterForSense(TSubclassOf<UAISense_Hearing>());
    StimuliSource->bAutoRegister = true;

    // Internal state
    bCanAttack = true;
    TimeSinceLastAttack = 0.0f;
    CurrentTarget = nullptr;
    HomeLocation = FVector::ZeroVector;
    bHasHomeLocation = false;
}

void ADinosaurBase::BeginPlay()
{
    Super::BeginPlay();

    // Record home location for territory behavior
    HomeLocation = GetActorLocation();
    bHasHomeLocation = true;

    // Start behavior tick timer (every 0.5s for AI decisions)
    GetWorld()->GetTimerManager().SetTimer(
        BehaviorTickHandle,
        this,
        &ADinosaurBase::UpdateBehaviorState,
        0.5f,
        true
    );

    // Start hunger depletion (every 10s)
    GetWorld()->GetTimerManager().SetTimer(
        HungerTickHandle,
        this,
        &ADinosaurBase::TickHunger,
        10.0f,
        true
    );

    // Apply species-specific configuration
    ApplySpeciesTraits();
}

void ADinosaurBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Attack cooldown tracking
    if (!bCanAttack)
    {
        TimeSinceLastAttack += DeltaTime;
        if (TimeSinceLastAttack >= AttackCooldown)
        {
            bCanAttack = true;
            TimeSinceLastAttack = 0.0f;
        }
    }

    // Stamina regeneration when not running
    if (CurrentBehaviorState != EDinosaurBehaviorState::Chasing &&
        CurrentBehaviorState != EDinosaurBehaviorState::Fleeing)
    {
        CurrentStamina = FMath::Min(MaxStamina, CurrentStamina + DeltaTime * 10.0f);
    }
}

void ADinosaurBase::ApplySpeciesTraits()
{
    switch (DinosaurSpecies)
    {
        case EDinosaurSpecies::TRex:
            MaxHealth = 2000.0f;
            CurrentHealth = 2000.0f;
            WalkSpeed = 250.0f;
            RunSpeed = 600.0f;
            AttackDamage = 200.0f;
            AttackRange = 300.0f;
            DetectionRange = 2000.0f;
            AttackCooldown = 3.0f;
            GetCapsuleComponent()->SetCapsuleHalfHeight(200.0f);
            GetCapsuleComponent()->SetCapsuleRadius(80.0f);
            break;

        case EDinosaurSpecies::Raptor:
            MaxHealth = 300.0f;
            CurrentHealth = 300.0f;
            WalkSpeed = 400.0f;
            RunSpeed = 900.0f;
            AttackDamage = 60.0f;
            AttackRange = 150.0f;
            DetectionRange = 1800.0f;
            AttackCooldown = 1.0f;
            GetCapsuleComponent()->SetCapsuleHalfHeight(70.0f);
            GetCapsuleComponent()->SetCapsuleRadius(30.0f);
            break;

        case EDinosaurSpecies::Brachiosaurus:
            MaxHealth = 5000.0f;
            CurrentHealth = 5000.0f;
            WalkSpeed = 200.0f;
            RunSpeed = 400.0f;
            AttackDamage = 100.0f; // Stomp
            AttackRange = 400.0f;
            DetectionRange = 1000.0f;
            AttackCooldown = 4.0f;
            GetCapsuleComponent()->SetCapsuleHalfHeight(350.0f);
            GetCapsuleComponent()->SetCapsuleRadius(120.0f);
            break;

        case EDinosaurSpecies::Triceratops:
            MaxHealth = 1500.0f;
            CurrentHealth = 1500.0f;
            WalkSpeed = 280.0f;
            RunSpeed = 550.0f;
            AttackDamage = 120.0f;
            AttackRange = 250.0f;
            DetectionRange = 1200.0f;
            AttackCooldown = 2.5f;
            GetCapsuleComponent()->SetCapsuleHalfHeight(130.0f);
            GetCapsuleComponent()->SetCapsuleRadius(70.0f);
            break;

        case EDinosaurSpecies::Pterodactyl:
            MaxHealth = 200.0f;
            CurrentHealth = 200.0f;
            WalkSpeed = 0.0f;
            RunSpeed = 1200.0f; // Flying speed
            AttackDamage = 40.0f;
            AttackRange = 100.0f;
            DetectionRange = 2500.0f;
            AttackCooldown = 1.5f;
            GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying);
            break;

        case EDinosaurSpecies::Ankylosaurus:
            MaxHealth = 1800.0f;
            CurrentHealth = 1800.0f;
            WalkSpeed = 180.0f;
            RunSpeed = 350.0f;
            AttackDamage = 150.0f; // Tail club
            AttackRange = 220.0f;
            DetectionRange = 800.0f;
            AttackCooldown = 3.0f;
            GetCapsuleComponent()->SetCapsuleHalfHeight(110.0f);
            GetCapsuleComponent()->SetCapsuleRadius(80.0f);
            break;

        default:
            // Generic herbivore defaults
            MaxHealth = 600.0f;
            CurrentHealth = 600.0f;
            WalkSpeed = 300.0f;
            RunSpeed = 650.0f;
            AttackDamage = 30.0f;
            AttackRange = 150.0f;
            DetectionRange = 1200.0f;
            break;
    }

    // Apply movement speed
    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

void ADinosaurBase::UpdateBehaviorState()
{
    // Hunger-driven behavior override
    if (CurrentHunger < 20.0f)
    {
        // Desperate hunger — aggressive hunting
        if (CurrentBehaviorState != EDinosaurBehaviorState::Hunting)
        {
            SetBehaviorState(EDinosaurBehaviorState::Hunting);
        }
        return;
    }

    // Low health — flee
    if (CurrentHealth < MaxHealth * 0.2f)
    {
        if (CurrentBehaviorState != EDinosaurBehaviorState::Fleeing)
        {
            SetBehaviorState(EDinosaurBehaviorState::Fleeing);
        }
        return;
    }

    // Default idle/patrol cycle
    if (CurrentBehaviorState == EDinosaurBehaviorState::Idle)
    {
        // 30% chance to start patrolling
        if (FMath::RandRange(0, 100) < 30)
        {
            SetBehaviorState(EDinosaurBehaviorState::Patrolling);
        }
    }
    else if (CurrentBehaviorState == EDinosaurBehaviorState::Patrolling)
    {
        // Check territory bounds — return home if too far
        float DistFromHome = FVector::Dist(GetActorLocation(), HomeLocation);
        if (bHasHomeLocation && DistFromHome > DetectionRange * 1.5f)
        {
            SetBehaviorState(EDinosaurBehaviorState::Idle);
        }
    }
}

void ADinosaurBase::SetBehaviorState(EDinosaurBehaviorState NewState)
{
    if (CurrentBehaviorState == NewState) return;

    CurrentBehaviorState = NewState;

    // Adjust movement speed based on state
    switch (NewState)
    {
        case EDinosaurBehaviorState::Idle:
        case EDinosaurBehaviorState::Resting:
            GetCharacterMovement()->MaxWalkSpeed = WalkSpeed * 0.3f;
            break;

        case EDinosaurBehaviorState::Patrolling:
        case EDinosaurBehaviorState::Grazing:
            GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
            break;

        case EDinosaurBehaviorState::Hunting:
        case EDinosaurBehaviorState::Chasing:
        case EDinosaurBehaviorState::Fleeing:
            GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
            break;

        case EDinosaurBehaviorState::Attacking:
            GetCharacterMovement()->MaxWalkSpeed = 0.0f;
            break;

        default:
            GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
            break;
    }

    OnBehaviorStateChanged(NewState);
}

void ADinosaurBase::OnBehaviorStateChanged(EDinosaurBehaviorState NewState)
{
    // Override in subclasses for species-specific reactions
}

float ADinosaurBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    CurrentHealth = FMath::Max(0.0f, CurrentHealth - ActualDamage);

    // React to being hit
    if (CurrentHealth > 0.0f)
    {
        // Become aggressive toward attacker
        if (DamageCauser && CurrentBehaviorState != EDinosaurBehaviorState::Fleeing)
        {
            CurrentTarget = DamageCauser;
            SetBehaviorState(EDinosaurBehaviorState::Attacking);
        }
    }
    else
    {
        // Death
        OnDinosaurDeath();
    }

    return ActualDamage;
}

void ADinosaurBase::OnDinosaurDeath()
{
    // Stop all timers
    GetWorld()->GetTimerManager().ClearTimer(BehaviorTickHandle);
    GetWorld()->GetTimerManager().ClearTimer(HungerTickHandle);

    // Disable collision and movement
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    GetCharacterMovement()->DisableMovement();

    // Ragdoll
    GetMesh()->SetSimulatePhysics(true);
    GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

    // Destroy after 30 seconds (cleanup)
    SetLifeSpan(30.0f);
}

void ADinosaurBase::PerformAttack()
{
    if (!bCanAttack || !CurrentTarget) return;

    float DistToTarget = FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation());
    if (DistToTarget > AttackRange) return;

    // Apply damage to target
    UGameplayStatics::ApplyDamage(
        CurrentTarget,
        AttackDamage,
        GetController(),
        this,
        UDamageType::StaticClass()
    );

    bCanAttack = false;
    TimeSinceLastAttack = 0.0f;

    SetBehaviorState(EDinosaurBehaviorState::Attacking);
}

void ADinosaurBase::TickHunger()
{
    // Hunger depletes over time
    float HungerDepletion = 2.0f;

    // Carnivores deplete faster
    if (DinosaurSpecies == EDinosaurSpecies::TRex ||
        DinosaurSpecies == EDinosaurSpecies::Raptor ||
        DinosaurSpecies == EDinosaurSpecies::Pterodactyl)
    {
        HungerDepletion = 3.0f;
    }

    CurrentHunger = FMath::Max(0.0f, CurrentHunger - HungerDepletion);

    // Starvation damage
    if (CurrentHunger <= 0.0f)
    {
        CurrentHealth = FMath::Max(0.0f, CurrentHealth - 10.0f);
        if (CurrentHealth <= 0.0f)
        {
            OnDinosaurDeath();
        }
    }
}

bool ADinosaurBase::IsAlive() const
{
    return CurrentHealth > 0.0f;
}

float ADinosaurBase::GetHealthPercent() const
{
    return (MaxHealth > 0.0f) ? (CurrentHealth / MaxHealth) : 0.0f;
}

float ADinosaurBase::GetHungerPercent() const
{
    return (MaxHunger > 0.0f) ? (CurrentHunger / MaxHunger) : 0.0f;
}

void ADinosaurBase::SetPackLeader(ADinosaurBase* Leader)
{
    PackLeader = Leader;
}

ADinosaurBase* ADinosaurBase::GetPackLeader() const
{
    return PackLeader;
}
