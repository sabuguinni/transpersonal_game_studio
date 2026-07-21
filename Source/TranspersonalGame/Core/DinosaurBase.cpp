// DinosaurBase.cpp
// Engine Architect #02 — Cycle PROD_CYCLE_AUTO_20260701_010
// Base class for all dinosaur pawns in the prehistoric survival game.
// Inherits from ACharacter to leverage UE5 movement, collision, and animation.

#include "DinosaurBase.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISense_Hearing.h"

ADinosaurBase::ADinosaurBase()
{
    PrimaryActorTick.bCanEverTick = true;

    // --- Capsule ---
    GetCapsuleComponent()->InitCapsuleSize(60.0f, 120.0f);
    GetCapsuleComponent()->SetCollisionProfileName(TEXT("Pawn"));

    // --- Mesh ---
    GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -120.0f));
    GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));

    // --- Movement ---
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        MoveComp->bOrientRotationToMovement = true;
        MoveComp->RotationRate = FRotator(0.0f, 360.0f, 0.0f);
        MoveComp->MaxWalkSpeed = 600.0f;
        MoveComp->JumpZVelocity = 400.0f;
        MoveComp->GravityScale = 1.5f;
        MoveComp->bCanWalkOffLedges = true;
        MoveComp->NavAgentProps.bCanCrouch = false;
    }

    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    // --- AI Perception Stimuli Source ---
    PerceptionStimuliSource = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>(TEXT("PerceptionStimuliSource"));
    if (PerceptionStimuliSource)
    {
        PerceptionStimuliSource->RegisterForSense(TSubclassOf<UAISense>(UAISense_Sight::StaticClass()));
        PerceptionStimuliSource->RegisterForSense(TSubclassOf<UAISense>(UAISense_Hearing::StaticClass()));
        PerceptionStimuliSource->bAutoRegister = true;
    }

    // --- Default survival stats ---
    MaxHealth = 500.0f;
    CurrentHealth = 500.0f;
    AttackDamage = 50.0f;
    AttackRange = 150.0f;
    DetectionRadius = 2000.0f;
    bIsAggressive = false;
    bIsPredator = false;
    DinosaurSpecies = EDinosaurSpecies::Unknown;
    BehaviorState = EDinosaurBehavior::Idle;
    HungerLevel = 0.5f;
    ThirstLevel = 0.5f;
}

void ADinosaurBase::BeginPlay()
{
    Super::BeginPlay();

    // Apply species-specific defaults
    ApplySpeciesDefaults();

    // Start behavior tick
    GetWorldTimerManager().SetTimer(
        BehaviorTickHandle,
        this,
        &ADinosaurBase::UpdateBehaviorState,
        2.0f,   // every 2 seconds
        true    // looping
    );
}

void ADinosaurBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Passive survival stat decay
    HungerLevel = FMath::Clamp(HungerLevel + DeltaTime * 0.001f, 0.0f, 1.0f);
    ThirstLevel = FMath::Clamp(ThirstLevel + DeltaTime * 0.0015f, 0.0f, 1.0f);
}

float ADinosaurBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    CurrentHealth = FMath::Clamp(CurrentHealth - ActualDamage, 0.0f, MaxHealth);

    if (CurrentHealth <= 0.0f)
    {
        OnDeath();
    }
    else if (bIsAggressive || bIsPredator)
    {
        // Predators and aggressive dinos retaliate
        BehaviorState = EDinosaurBehavior::Attacking;
    }

    return ActualDamage;
}

void ADinosaurBase::ApplySpeciesDefaults()
{
    switch (DinosaurSpecies)
    {
    case EDinosaurSpecies::TyrannosaurusRex:
        MaxHealth = 2000.0f;
        CurrentHealth = 2000.0f;
        AttackDamage = 200.0f;
        AttackRange = 250.0f;
        DetectionRadius = 3500.0f;
        bIsAggressive = true;
        bIsPredator = true;
        GetCharacterMovement()->MaxWalkSpeed = 500.0f;
        GetCapsuleComponent()->InitCapsuleSize(120.0f, 280.0f);
        break;

    case EDinosaurSpecies::Velociraptor:
        MaxHealth = 300.0f;
        CurrentHealth = 300.0f;
        AttackDamage = 80.0f;
        AttackRange = 100.0f;
        DetectionRadius = 2500.0f;
        bIsAggressive = true;
        bIsPredator = true;
        GetCharacterMovement()->MaxWalkSpeed = 900.0f;
        GetCapsuleComponent()->InitCapsuleSize(40.0f, 80.0f);
        break;

    case EDinosaurSpecies::Brachiosaurus:
        MaxHealth = 5000.0f;
        CurrentHealth = 5000.0f;
        AttackDamage = 100.0f; // stomp
        AttackRange = 300.0f;
        DetectionRadius = 1500.0f;
        bIsAggressive = false;
        bIsPredator = false;
        GetCharacterMovement()->MaxWalkSpeed = 300.0f;
        GetCapsuleComponent()->InitCapsuleSize(200.0f, 500.0f);
        break;

    case EDinosaurSpecies::Triceratops:
        MaxHealth = 1500.0f;
        CurrentHealth = 1500.0f;
        AttackDamage = 120.0f;
        AttackRange = 200.0f;
        DetectionRadius = 1800.0f;
        bIsAggressive = false; // defensive only
        bIsPredator = false;
        GetCharacterMovement()->MaxWalkSpeed = 450.0f;
        GetCapsuleComponent()->InitCapsuleSize(100.0f, 180.0f);
        break;

    case EDinosaurSpecies::Pterodactyl:
        MaxHealth = 200.0f;
        CurrentHealth = 200.0f;
        AttackDamage = 40.0f;
        AttackRange = 80.0f;
        DetectionRadius = 4000.0f;
        bIsAggressive = false;
        bIsPredator = true;
        GetCharacterMovement()->MaxWalkSpeed = 200.0f;
        GetCharacterMovement()->MaxFlySpeed = 1200.0f;
        GetCapsuleComponent()->InitCapsuleSize(50.0f, 60.0f);
        break;

    default:
        // Unknown — keep constructor defaults
        break;
    }
}

void ADinosaurBase::UpdateBehaviorState()
{
    // Simple state machine — AI agent (#12) will override with full Behavior Trees
    if (CurrentHealth <= 0.0f)
    {
        return;
    }

    // Hunger drives feeding behavior
    if (HungerLevel > 0.8f && bIsPredator)
    {
        BehaviorState = EDinosaurBehavior::Hunting;
    }
    else if (BehaviorState == EDinosaurBehavior::Idle)
    {
        // Random wander chance
        if (FMath::RandRange(0, 3) == 0)
        {
            BehaviorState = EDinosaurBehavior::Wandering;
        }
    }
    else if (BehaviorState == EDinosaurBehavior::Wandering)
    {
        // Occasionally rest
        if (FMath::RandRange(0, 5) == 0)
        {
            BehaviorState = EDinosaurBehavior::Idle;
        }
    }
}

void ADinosaurBase::OnDeath()
{
    BehaviorState = EDinosaurBehavior::Dead;

    // Stop behavior timer
    GetWorldTimerManager().ClearTimer(BehaviorTickHandle);

    // Disable collision so player can walk over corpse
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    GetMesh()->SetSimulatePhysics(true); // ragdoll

    // Destroy after 30 seconds (cleanup)
    SetLifeSpan(30.0f);
}

bool ADinosaurBase::IsAlive() const
{
    return CurrentHealth > 0.0f;
}

float ADinosaurBase::GetHealthPercent() const
{
    if (MaxHealth <= 0.0f) return 0.0f;
    return CurrentHealth / MaxHealth;
}
