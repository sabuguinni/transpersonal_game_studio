// DinosaurBase.cpp — Base class for all dinosaur pawns
// Engine Architect #02 — Transpersonal Game Studio
// Cycle: PROD_CYCLE_AUTO_20260630_011

#include "DinosaurBase.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISense_Hearing.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"

ADinosaurBase::ADinosaurBase()
{
    PrimaryActorTick.bCanEverTick = true;

    // Capsule sizing — default for medium dinosaur (raptor-sized)
    GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);

    // Movement defaults
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
    GetCharacterMovement()->JumpZVelocity = 400.0f;
    GetCharacterMovement()->AirControl = 0.2f;
    GetCharacterMovement()->MaxWalkSpeed = 600.0f;
    GetCharacterMovement()->NavAgentProps.AgentRadius = 42.0f;
    GetCharacterMovement()->NavAgentProps.AgentHeight = 192.0f;

    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    // AI Perception stimuli source
    PerceptionStimuliSource = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>(
        TEXT("PerceptionStimuliSource"));
    PerceptionStimuliSource->RegisterForSense(TSubclassOf<UAISense_Sight>());
    PerceptionStimuliSource->RegisterForSense(TSubclassOf<UAISense_Hearing>());
    PerceptionStimuliSource->bAutoRegister = true;

    // Default species stats
    DinosaurSpecies = EDinosaurSpecies::Raptor;
    MaxHealth = 100.0f;
    CurrentHealth = 100.0f;
    AttackDamage = 25.0f;
    AttackRange = 150.0f;
    DetectionRadius = 1500.0f;
    WalkSpeed = 300.0f;
    RunSpeed = 600.0f;
    bIsAggressive = false;
    bIsAlpha = false;
    PackSize = 1;
    CurrentBehaviorState = EDinosaurBehaviorState::Idle;
    TerritoryRadius = 3000.0f;
    HungerLevel = 0.5f;
    FearLevel = 0.0f;
}

void ADinosaurBase::BeginPlay()
{
    Super::BeginPlay();

    // Apply species-specific stats
    ApplySpeciesStats();

    // Set initial walk speed
    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;

    // Start hunger tick
    GetWorld()->GetTimerManager().SetTimer(
        HungerTimerHandle,
        this,
        &ADinosaurBase::UpdateHunger,
        10.0f,  // Every 10 seconds
        true
    );

    // Start behavior update
    GetWorld()->GetTimerManager().SetTimer(
        BehaviorTimerHandle,
        this,
        &ADinosaurBase::UpdateBehaviorState,
        2.0f,  // Every 2 seconds
        true
    );

    // Set territory origin to spawn point
    TerritoryOrigin = GetActorLocation();

    UE_LOG(LogTemp, Log, TEXT("DinosaurBase: %s spawned as %s, Health=%.1f, Damage=%.1f"),
        *GetName(),
        *UEnum::GetValueAsString(DinosaurSpecies),
        CurrentHealth,
        AttackDamage);
}

void ADinosaurBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update fear decay
    if (FearLevel > 0.0f)
    {
        FearLevel = FMath::Max(0.0f, FearLevel - DeltaTime * 0.05f);
    }
}

void ADinosaurBase::ApplySpeciesStats()
{
    switch (DinosaurSpecies)
    {
    case EDinosaurSpecies::TRex:
        MaxHealth = 1000.0f;
        CurrentHealth = 1000.0f;
        AttackDamage = 150.0f;
        AttackRange = 300.0f;
        DetectionRadius = 3000.0f;
        WalkSpeed = 250.0f;
        RunSpeed = 800.0f;
        bIsAggressive = true;
        TerritoryRadius = 8000.0f;
        GetCapsuleComponent()->InitCapsuleSize(120.0f, 300.0f);
        break;

    case EDinosaurSpecies::Raptor:
        MaxHealth = 200.0f;
        CurrentHealth = 200.0f;
        AttackDamage = 40.0f;
        AttackRange = 120.0f;
        DetectionRadius = 2000.0f;
        WalkSpeed = 350.0f;
        RunSpeed = 900.0f;
        bIsAggressive = true;
        PackSize = 3;
        TerritoryRadius = 4000.0f;
        GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);
        break;

    case EDinosaurSpecies::Brachiosaurus:
        MaxHealth = 2000.0f;
        CurrentHealth = 2000.0f;
        AttackDamage = 80.0f;  // Stomp damage
        AttackRange = 400.0f;
        DetectionRadius = 1000.0f;
        WalkSpeed = 200.0f;
        RunSpeed = 400.0f;
        bIsAggressive = false;
        TerritoryRadius = 6000.0f;
        GetCapsuleComponent()->InitCapsuleSize(200.0f, 500.0f);
        break;

    case EDinosaurSpecies::Triceratops:
        MaxHealth = 800.0f;
        CurrentHealth = 800.0f;
        AttackDamage = 100.0f;
        AttackRange = 250.0f;
        DetectionRadius = 1500.0f;
        WalkSpeed = 280.0f;
        RunSpeed = 600.0f;
        bIsAggressive = false;  // Defensive only
        TerritoryRadius = 5000.0f;
        GetCapsuleComponent()->InitCapsuleSize(100.0f, 200.0f);
        break;

    case EDinosaurSpecies::Pterodactyl:
        MaxHealth = 150.0f;
        CurrentHealth = 150.0f;
        AttackDamage = 30.0f;
        AttackRange = 200.0f;
        DetectionRadius = 4000.0f;
        WalkSpeed = 150.0f;
        RunSpeed = 1200.0f;  // Flying speed
        bIsAggressive = true;
        TerritoryRadius = 10000.0f;
        GetCapsuleComponent()->InitCapsuleSize(60.0f, 80.0f);
        break;

    case EDinosaurSpecies::Stegosaurus:
        MaxHealth = 600.0f;
        CurrentHealth = 600.0f;
        AttackDamage = 60.0f;
        AttackRange = 200.0f;
        DetectionRadius = 1200.0f;
        WalkSpeed = 220.0f;
        RunSpeed = 450.0f;
        bIsAggressive = false;
        TerritoryRadius = 4000.0f;
        GetCapsuleComponent()->InitCapsuleSize(90.0f, 180.0f);
        break;

    default:
        break;
    }

    // Apply movement speed
    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

float ADinosaurBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    CurrentHealth = FMath::Max(0.0f, CurrentHealth - ActualDamage);

    // Increase fear when taking damage
    FearLevel = FMath::Min(1.0f, FearLevel + 0.2f);

    UE_LOG(LogTemp, Log, TEXT("DinosaurBase: %s took %.1f damage, Health=%.1f/%.1f"),
        *GetName(), ActualDamage, CurrentHealth, MaxHealth);

    // Trigger aggression if not already aggressive
    if (!bIsAggressive && CurrentHealth < MaxHealth * 0.5f)
    {
        bIsAggressive = true;
        CurrentBehaviorState = EDinosaurBehaviorState::Aggressive;
    }

    if (CurrentHealth <= 0.0f)
    {
        OnDeath();
    }

    return ActualDamage;
}

void ADinosaurBase::OnDeath()
{
    CurrentBehaviorState = EDinosaurBehaviorState::Dead;

    // Stop all timers
    GetWorld()->GetTimerManager().ClearTimer(HungerTimerHandle);
    GetWorld()->GetTimerManager().ClearTimer(BehaviorTimerHandle);

    // Disable movement
    GetCharacterMovement()->DisableMovement();

    // Disable collision
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    UE_LOG(LogTemp, Log, TEXT("DinosaurBase: %s has died"), *GetName());

    // Broadcast death event (for quest system, loot, etc.)
    OnDinosaurDied.Broadcast(this);

    // Destroy after delay (allow death animation to play)
    SetLifeSpan(10.0f);
}

void ADinosaurBase::UpdateHunger()
{
    HungerLevel = FMath::Min(1.0f, HungerLevel + 0.05f);

    // Hungry dinosaurs become more aggressive
    if (HungerLevel > 0.8f && !bIsAggressive)
    {
        bIsAggressive = true;
        UE_LOG(LogTemp, Log, TEXT("DinosaurBase: %s is hungry and becoming aggressive"), *GetName());
    }
}

void ADinosaurBase::UpdateBehaviorState()
{
    if (CurrentBehaviorState == EDinosaurBehaviorState::Dead)
    {
        return;
    }

    // Check if we're far from territory
    float DistFromTerritory = FVector::Dist(GetActorLocation(), TerritoryOrigin);

    if (DistFromTerritory > TerritoryRadius)
    {
        CurrentBehaviorState = EDinosaurBehaviorState::Patrolling;
        return;
    }

    // High fear = flee
    if (FearLevel > 0.7f && !bIsAlpha)
    {
        CurrentBehaviorState = EDinosaurBehaviorState::Fleeing;
        return;
    }

    // Hungry + aggressive = hunting
    if (HungerLevel > 0.6f && bIsAggressive)
    {
        CurrentBehaviorState = EDinosaurBehaviorState::Hunting;
        return;
    }

    // Default: idle or patrolling
    if (CurrentBehaviorState != EDinosaurBehaviorState::Aggressive)
    {
        CurrentBehaviorState = (FMath::RandBool()) ?
            EDinosaurBehaviorState::Idle :
            EDinosaurBehaviorState::Patrolling;
    }
}

bool ADinosaurBase::IsAlive() const
{
    return CurrentHealth > 0.0f && CurrentBehaviorState != EDinosaurBehaviorState::Dead;
}

float ADinosaurBase::GetHealthPercent() const
{
    return (MaxHealth > 0.0f) ? (CurrentHealth / MaxHealth) : 0.0f;
}

void ADinosaurBase::SetBehaviorState(EDinosaurBehaviorState NewState)
{
    if (CurrentBehaviorState == EDinosaurBehaviorState::Dead)
    {
        return;
    }

    CurrentBehaviorState = NewState;

    // Adjust speed based on state
    switch (NewState)
    {
    case EDinosaurBehaviorState::Hunting:
    case EDinosaurBehaviorState::Aggressive:
        GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
        break;
    case EDinosaurBehaviorState::Fleeing:
        GetCharacterMovement()->MaxWalkSpeed = RunSpeed * 1.2f;
        break;
    default:
        GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
        break;
    }
}

void ADinosaurBase::MakeNoise_Implementation(float Loudness, APawn* NoiseInstigator,
    FVector NoiseLocation, float MaxRange, FName Tag)
{
    // Forward to UE5 noise system
    Super::MakeNoise(Loudness, NoiseInstigator, NoiseLocation, MaxRange, Tag);
}
