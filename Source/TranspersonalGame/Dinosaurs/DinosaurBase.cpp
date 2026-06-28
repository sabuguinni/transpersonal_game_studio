// DinosaurBase.cpp
// Transpersonal Game Studio — Engine Architect #02
// Base class for all dinosaur pawns in the prehistoric survival game.

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

ADinosaurBase::ADinosaurBase()
{
    PrimaryActorTick.bCanEverTick = true;

    // Capsule defaults — overridden per species
    GetCapsuleComponent()->InitCapsuleSize(60.0f, 120.0f);

    // Movement defaults
    GetCharacterMovement()->MaxWalkSpeed = 400.0f;
    GetCharacterMovement()->MaxWalkSpeedCrouched = 200.0f;
    GetCharacterMovement()->JumpZVelocity = 500.0f;
    GetCharacterMovement()->GravityScale = 1.2f;
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 360.0f, 0.0f);
    bUseControllerRotationYaw = false;

    // AI Perception stimuli source
    PerceptionStimuliSource = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>(TEXT("PerceptionStimuliSource"));
    PerceptionStimuliSource->RegisterForSense(TSubclassOf<UAISense>(UAISense_Sight::StaticClass()));
    PerceptionStimuliSource->RegisterForSense(TSubclassOf<UAISense>(UAISense_Hearing::StaticClass()));

    // Default species stats
    MaxHealth = 500.0f;
    CurrentHealth = MaxHealth;
    MaxStamina = 300.0f;
    CurrentStamina = MaxStamina;
    AttackDamage = 40.0f;
    AttackRange = 200.0f;
    DetectionRange = 2000.0f;
    ChaseRange = 3000.0f;
    WalkSpeed = 300.0f;
    RunSpeed = 700.0f;
    bIsCarnivore = true;
    bIsHerbivore = false;
    bIsPackHunter = false;
    bIsTerritorial = true;
    DinosaurSpecies = EEng_DinosaurSpecies::TyrannosaurusRex;
    CurrentBehaviorState = EEng_DinosaurBehavior::Idle;
    StaminaRegenRate = 20.0f;
    HungerDecayRate = 5.0f;
    MaxHunger = 100.0f;
    CurrentHunger = 50.0f;
    bIsAlerted = false;
    bIsAttacking = false;
    LastKnownPlayerLocation = FVector::ZeroVector;
}

void ADinosaurBase::BeginPlay()
{
    Super::BeginPlay();

    // Apply species-specific stats
    ApplySpeciesStats();

    // Start behavior update timer
    GetWorldTimerManager().SetTimer(
        BehaviorUpdateTimer,
        this,
        &ADinosaurBase::UpdateBehaviorState,
        0.5f,
        true
    );

    // Start hunger decay timer
    GetWorldTimerManager().SetTimer(
        HungerDecayTimer,
        this,
        &ADinosaurBase::DecayHunger,
        2.0f,
        true
    );
}

void ADinosaurBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Stamina regen when not running
    if (CurrentStamina < MaxStamina && !bIsAttacking)
    {
        CurrentStamina = FMath::Min(MaxStamina, CurrentStamina + StaminaRegenRate * DeltaTime);
    }
}

float ADinosaurBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    CurrentHealth = FMath::Max(0.0f, CurrentHealth - ActualDamage);

    if (ActualDamage > 0.0f)
    {
        OnDinosaurDamaged(ActualDamage, DamageCauser);

        // Alert the dinosaur when hit
        bIsAlerted = true;
        CurrentBehaviorState = EEng_DinosaurBehavior::Aggressive;

        if (DamageCauser)
        {
            LastKnownPlayerLocation = DamageCauser->GetActorLocation();
        }
    }

    if (CurrentHealth <= 0.0f)
    {
        OnDinosaurDeath();
    }

    return ActualDamage;
}

void ADinosaurBase::ApplySpeciesStats()
{
    switch (DinosaurSpecies)
    {
    case EEng_DinosaurSpecies::TyrannosaurusRex:
        MaxHealth = 1500.0f;
        AttackDamage = 120.0f;
        AttackRange = 250.0f;
        DetectionRange = 3000.0f;
        WalkSpeed = 250.0f;
        RunSpeed = 550.0f;
        GetCapsuleComponent()->InitCapsuleSize(80.0f, 200.0f);
        bIsPackHunter = false;
        break;

    case EEng_DinosaurSpecies::Velociraptor:
        MaxHealth = 200.0f;
        AttackDamage = 45.0f;
        AttackRange = 150.0f;
        DetectionRange = 2500.0f;
        WalkSpeed = 400.0f;
        RunSpeed = 900.0f;
        GetCapsuleComponent()->InitCapsuleSize(35.0f, 80.0f);
        bIsPackHunter = true;
        break;

    case EEng_DinosaurSpecies::Triceratops:
        MaxHealth = 1200.0f;
        AttackDamage = 80.0f;
        AttackRange = 300.0f;
        DetectionRange = 1500.0f;
        WalkSpeed = 280.0f;
        RunSpeed = 500.0f;
        GetCapsuleComponent()->InitCapsuleSize(90.0f, 160.0f);
        bIsCarnivore = false;
        bIsHerbivore = true;
        bIsTerritorial = true;
        break;

    case EEng_DinosaurSpecies::Brachiosaurus:
        MaxHealth = 3000.0f;
        AttackDamage = 60.0f;
        AttackRange = 400.0f;
        DetectionRange = 1000.0f;
        WalkSpeed = 200.0f;
        RunSpeed = 380.0f;
        GetCapsuleComponent()->InitCapsuleSize(120.0f, 300.0f);
        bIsCarnivore = false;
        bIsHerbivore = true;
        bIsTerritorial = false;
        break;

    case EEng_DinosaurSpecies::Pterodactyl:
        MaxHealth = 150.0f;
        AttackDamage = 30.0f;
        AttackRange = 200.0f;
        DetectionRange = 4000.0f;
        WalkSpeed = 200.0f;
        RunSpeed = 1200.0f;
        GetCapsuleComponent()->InitCapsuleSize(40.0f, 60.0f);
        bIsPackHunter = false;
        break;

    case EEng_DinosaurSpecies::Stegosaurus:
        MaxHealth = 900.0f;
        AttackDamage = 55.0f;
        AttackRange = 250.0f;
        DetectionRange = 1200.0f;
        WalkSpeed = 220.0f;
        RunSpeed = 420.0f;
        GetCapsuleComponent()->InitCapsuleSize(75.0f, 140.0f);
        bIsCarnivore = false;
        bIsHerbivore = true;
        break;

    case EEng_DinosaurSpecies::Spinosaurus:
        MaxHealth = 1800.0f;
        AttackDamage = 100.0f;
        AttackRange = 280.0f;
        DetectionRange = 2800.0f;
        WalkSpeed = 300.0f;
        RunSpeed = 600.0f;
        GetCapsuleComponent()->InitCapsuleSize(90.0f, 220.0f);
        break;

    case EEng_DinosaurSpecies::Ankylosaurus:
        MaxHealth = 1100.0f;
        AttackDamage = 70.0f;
        AttackRange = 200.0f;
        DetectionRange = 1000.0f;
        WalkSpeed = 180.0f;
        RunSpeed = 350.0f;
        GetCapsuleComponent()->InitCapsuleSize(85.0f, 130.0f);
        bIsCarnivore = false;
        bIsHerbivore = true;
        break;

    default:
        break;
    }

    // Apply movement speeds
    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
    CurrentHealth = MaxHealth;
    CurrentStamina = MaxStamina;
}

void ADinosaurBase::UpdateBehaviorState()
{
    // Basic state machine — full BehaviorTree integration handled by AI Controller
    if (CurrentHealth <= 0.0f)
    {
        CurrentBehaviorState = EEng_DinosaurBehavior::Dead;
        return;
    }

    if (CurrentHunger < 20.0f && bIsCarnivore)
    {
        CurrentBehaviorState = EEng_DinosaurBehavior::Hunting;
        return;
    }

    if (bIsAlerted && CurrentBehaviorState != EEng_DinosaurBehavior::Aggressive)
    {
        CurrentBehaviorState = EEng_DinosaurBehavior::Alert;
    }
}

void ADinosaurBase::DecayHunger()
{
    CurrentHunger = FMath::Max(0.0f, CurrentHunger - HungerDecayRate * 2.0f);
}

void ADinosaurBase::OnDinosaurDamaged_Implementation(float DamageAmount, AActor* DamageCauser)
{
    // Default: alert and face attacker
    bIsAlerted = true;
    if (DamageCauser)
    {
        FVector ToAttacker = DamageCauser->GetActorLocation() - GetActorLocation();
        ToAttacker.Z = 0.0f;
        FRotator LookAt = ToAttacker.Rotation();
        SetActorRotation(LookAt);
    }
}

void ADinosaurBase::OnDinosaurDeath_Implementation()
{
    // Default death: disable collision, stop movement, set dead state
    CurrentBehaviorState = EEng_DinosaurBehavior::Dead;
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    GetCharacterMovement()->DisableMovement();
    bIsAlerted = false;
    bIsAttacking = false;

    // Detach AI controller
    if (GetController())
    {
        GetController()->UnPossess();
    }
}

void ADinosaurBase::SetBehaviorState(EEng_DinosaurBehavior NewState)
{
    if (CurrentBehaviorState != NewState)
    {
        CurrentBehaviorState = NewState;
        OnBehaviorStateChanged(NewState);
    }
}

void ADinosaurBase::OnBehaviorStateChanged_Implementation(EEng_DinosaurBehavior NewState)
{
    // Apply movement speed based on state
    switch (NewState)
    {
    case EEng_DinosaurBehavior::Idle:
    case EEng_DinosaurBehavior::Grazing:
    case EEng_DinosaurBehavior::Patrolling:
        GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
        break;

    case EEng_DinosaurBehavior::Alert:
    case EEng_DinosaurBehavior::Hunting:
        GetCharacterMovement()->MaxWalkSpeed = (WalkSpeed + RunSpeed) * 0.5f;
        break;

    case EEng_DinosaurBehavior::Aggressive:
    case EEng_DinosaurBehavior::Fleeing:
        GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
        break;

    default:
        break;
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

float ADinosaurBase::GetStaminaPercent() const
{
    return (MaxStamina > 0.0f) ? (CurrentStamina / MaxStamina) : 0.0f;
}
