// DinosaurBase.cpp
// Transpersonal Game Studio — Engine Architect #02
// Base class for all dinosaur species in the prehistoric survival game

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

ADinosaurBase::ADinosaurBase()
{
    PrimaryActorTick.bCanEverTick = true;

    // --- Capsule ---
    GetCapsuleComponent()->InitCapsuleSize(50.0f, 100.0f);

    // --- Movement defaults (overridden per species) ---
    GetCharacterMovement()->MaxWalkSpeed = 400.0f;
    GetCharacterMovement()->MaxWalkSpeedCrouched = 150.0f;
    GetCharacterMovement()->JumpZVelocity = 500.0f;
    GetCharacterMovement()->GravityScale = 1.2f;
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 360.0f, 0.0f);
    bUseControllerRotationYaw = false;

    // --- Perception stimuli source ---
    PerceptionStimuliSource = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>(TEXT("PerceptionStimuliSource"));
    if (PerceptionStimuliSource)
    {
        PerceptionStimuliSource->RegisterForSense(TSubclassOf<UAISense>(UAISense_Sight::StaticClass()));
        PerceptionStimuliSource->RegisterForSense(TSubclassOf<UAISense>(UAISense_Hearing::StaticClass()));
        PerceptionStimuliSource->bAutoRegister = true;
    }

    // --- Default species stats ---
    DinosaurSpecies = EDinosaurSpecies::TRex;
    BehaviorState = EDinosaurBehavior::Idle;
    MaxHealth = 1000.0f;
    CurrentHealth = 1000.0f;
    AttackDamage = 150.0f;
    AttackRange = 200.0f;
    DetectionRadius = 2000.0f;
    AggroRadius = 800.0f;
    bIsAlpha = false;
    PackID = -1;
    HungerLevel = 0.5f;
    TerritoryCenter = FVector::ZeroVector;
    TerritoryRadius = 3000.0f;
}

void ADinosaurBase::BeginPlay()
{
    Super::BeginPlay();
    TerritoryCenter = GetActorLocation();
    CurrentHealth = MaxHealth;
    UE_LOG(LogTemp, Log, TEXT("[DinosaurBase] %s spawned — Species: %d, Health: %.0f, Territory: %s"),
        *GetName(), (int32)DinosaurSpecies, CurrentHealth, *TerritoryCenter.ToString());
}

void ADinosaurBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Passive hunger drain
    HungerLevel = FMath::Clamp(HungerLevel - (DeltaTime * 0.001f), 0.0f, 1.0f);

    // If very hungry, increase aggression radius
    if (HungerLevel < 0.2f && BehaviorState == EDinosaurBehavior::Idle)
    {
        BehaviorState = EDinosaurBehavior::Foraging;
    }
}

float ADinosaurBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    CurrentHealth = FMath::Clamp(CurrentHealth - ActualDamage, 0.0f, MaxHealth);
    UE_LOG(LogTemp, Log, TEXT("[DinosaurBase] %s took %.1f damage — Health: %.1f/%.1f"),
        *GetName(), ActualDamage, CurrentHealth, MaxHealth);

    if (CurrentHealth <= 0.0f)
    {
        OnDinosaurDeath();
    }
    else if (ActualDamage > 0.0f)
    {
        // React to being hit — switch to aggressive state
        BehaviorState = EDinosaurBehavior::Aggressive;
    }

    return ActualDamage;
}

void ADinosaurBase::OnDinosaurDeath()
{
    BehaviorState = EDinosaurBehavior::Dead;
    UE_LOG(LogTemp, Log, TEXT("[DinosaurBase] %s has died"), *GetName());

    // Disable collision and AI
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    GetCharacterMovement()->DisableMovement();

    // Broadcast death event for quest/ecology systems
    OnDinosaurDeathDelegate.Broadcast(this);

    // Ragdoll death
    GetMesh()->SetSimulatePhysics(true);
    GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}

void ADinosaurBase::SetBehaviorState(EDinosaurBehavior NewState)
{
    if (BehaviorState != NewState)
    {
        EDinosaurBehavior OldState = BehaviorState;
        BehaviorState = NewState;
        UE_LOG(LogTemp, Verbose, TEXT("[DinosaurBase] %s behavior: %d -> %d"),
            *GetName(), (int32)OldState, (int32)NewState);
    }
}

bool ADinosaurBase::IsInTerritory(const FVector& Location) const
{
    return FVector::Dist(Location, TerritoryCenter) <= TerritoryRadius;
}

float ADinosaurBase::GetHealthPercent() const
{
    return (MaxHealth > 0.0f) ? (CurrentHealth / MaxHealth) : 0.0f;
}

bool ADinosaurBase::IsAlive() const
{
    return CurrentHealth > 0.0f && BehaviorState != EDinosaurBehavior::Dead;
}

void ADinosaurBase::InitializeSpecies(EDinosaurSpecies Species)
{
    DinosaurSpecies = Species;

    // Apply species-specific stats
    switch (Species)
    {
    case EDinosaurSpecies::TRex:
        MaxHealth = 2500.0f;
        AttackDamage = 300.0f;
        AttackRange = 250.0f;
        DetectionRadius = 3000.0f;
        AggroRadius = 1500.0f;
        GetCharacterMovement()->MaxWalkSpeed = 600.0f;
        GetCapsuleComponent()->InitCapsuleSize(120.0f, 250.0f);
        break;

    case EDinosaurSpecies::Raptor:
        MaxHealth = 400.0f;
        AttackDamage = 80.0f;
        AttackRange = 120.0f;
        DetectionRadius = 2500.0f;
        AggroRadius = 1200.0f;
        GetCharacterMovement()->MaxWalkSpeed = 900.0f;
        GetCapsuleComponent()->InitCapsuleSize(40.0f, 80.0f);
        break;

    case EDinosaurSpecies::Brachiosaurus:
        MaxHealth = 5000.0f;
        AttackDamage = 50.0f;  // Herbivore — low aggression
        AttackRange = 300.0f;
        DetectionRadius = 1500.0f;
        AggroRadius = 400.0f;  // Only attacks if cornered
        GetCharacterMovement()->MaxWalkSpeed = 350.0f;
        GetCapsuleComponent()->InitCapsuleSize(150.0f, 400.0f);
        break;

    case EDinosaurSpecies::Triceratops:
        MaxHealth = 1800.0f;
        AttackDamage = 200.0f;
        AttackRange = 180.0f;
        DetectionRadius = 1800.0f;
        AggroRadius = 600.0f;
        GetCharacterMovement()->MaxWalkSpeed = 500.0f;
        GetCapsuleComponent()->InitCapsuleSize(100.0f, 180.0f);
        break;

    case EDinosaurSpecies::Pterodactyl:
        MaxHealth = 300.0f;
        AttackDamage = 60.0f;
        AttackRange = 100.0f;
        DetectionRadius = 4000.0f;  // High aerial visibility
        AggroRadius = 500.0f;
        GetCharacterMovement()->MaxWalkSpeed = 200.0f;  // Slow on ground
        GetCharacterMovement()->MaxFlySpeed = 1200.0f;
        GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying);
        GetCapsuleComponent()->InitCapsuleSize(30.0f, 50.0f);
        break;

    default:
        break;
    }

    CurrentHealth = MaxHealth;
    TerritoryRadius = DetectionRadius * 1.5f;
    UE_LOG(LogTemp, Log, TEXT("[DinosaurBase] Species initialized: %d — HP:%.0f ATK:%.0f SPD:%.0f"),
        (int32)Species, MaxHealth, AttackDamage, GetCharacterMovement()->MaxWalkSpeed);
}
