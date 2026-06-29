// DinosaurBase.cpp
// Transpersonal Game Studio — Engine Architect #02
// Base class for all dinosaur species in the prehistoric survival game.
// All dinosaur types (TRex, Raptor, Brachiosaurus, etc.) inherit from this class.

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
#include "DrawDebugHelpers.h"

// ============================================================
// Constructor
// ============================================================

ADinosaurBase::ADinosaurBase()
{
    PrimaryActorTick.bCanEverTick = true;

    // Default survival stats
    MaxHealth = 500.0f;
    CurrentHealth = MaxHealth;
    MaxStamina = 200.0f;
    CurrentStamina = MaxStamina;
    Hunger = 0.0f;
    MaxHunger = 100.0f;
    HungerDrainRate = 1.0f;
    StaminaRegenRate = 10.0f;

    // Default movement speeds
    WalkSpeed = 300.0f;
    RunSpeed = 700.0f;
    TurnRate = 90.0f;

    // Default species data
    DinosaurSpecies = EDinosaurSpecies::Unknown;
    DietType = EDinosaurDiet::Carnivore;
    bIsAggressive = false;
    bIsAlerted = false;
    bIsDead = false;
    bIsRunning = false;
    AggressionRadius = 800.0f;
    DetectionRadius = 1200.0f;
    AttackDamage = 50.0f;
    AttackRange = 200.0f;
    AttackCooldown = 2.0f;
    LastAttackTime = -999.0f;

    // Configure capsule
    GetCapsuleComponent()->InitCapsuleSize(60.0f, 120.0f);

    // Configure movement
    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, TurnRate, 0.0f);
    GetCharacterMovement()->JumpZVelocity = 400.0f;
    GetCharacterMovement()->AirControl = 0.2f;
    GetCharacterMovement()->GravityScale = 1.0f;

    // AI Perception stimuli source
    PerceptionStimuliSource = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>(TEXT("PerceptionStimuliSource"));
    PerceptionStimuliSource->RegisterForSense(TSubclassOf<UAISense_Sight>());
    PerceptionStimuliSource->RegisterForSense(TSubclassOf<UAISense_Hearing>());
    PerceptionStimuliSource->bAutoRegister = true;

    // Auto-possess by AI
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

// ============================================================
// BeginPlay
// ============================================================

void ADinosaurBase::BeginPlay()
{
    Super::BeginPlay();

    CurrentHealth = MaxHealth;
    CurrentStamina = MaxStamina;
    Hunger = 0.0f;

    // Start hunger drain timer
    GetWorldTimerManager().SetTimer(
        HungerTimerHandle,
        this,
        &ADinosaurBase::DrainHunger,
        1.0f,
        true
    );

    // Start stamina regen timer
    GetWorldTimerManager().SetTimer(
        StaminaTimerHandle,
        this,
        &ADinosaurBase::RegenStamina,
        0.5f,
        true
    );

    UE_LOG(LogTemp, Log, TEXT("DinosaurBase BeginPlay: %s | Species: %d | Health: %.1f"),
        *GetName(), (int32)DinosaurSpecies, CurrentHealth);
}

// ============================================================
// Tick
// ============================================================

void ADinosaurBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update movement speed based on state
    if (bIsRunning)
    {
        GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
    }
    else
    {
        GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
    }
}

// ============================================================
// Health & Damage
// ============================================================

void ADinosaurBase::ApplyDamage_Dino(float DamageAmount, AActor* DamageCauser)
{
    if (bIsDead) return;

    CurrentHealth = FMath::Clamp(CurrentHealth - DamageAmount, 0.0f, MaxHealth);

    UE_LOG(LogTemp, Warning, TEXT("%s took %.1f damage. Health: %.1f/%.1f"),
        *GetName(), DamageAmount, CurrentHealth, MaxHealth);

    // Alert on damage
    if (!bIsAlerted)
    {
        bIsAlerted = true;
        OnAlerted(DamageCauser);
    }

    if (CurrentHealth <= 0.0f)
    {
        Die();
    }
}

void ADinosaurBase::Heal(float HealAmount)
{
    if (bIsDead) return;
    CurrentHealth = FMath::Clamp(CurrentHealth + HealAmount, 0.0f, MaxHealth);
}

float ADinosaurBase::GetHealthPercent() const
{
    return (MaxHealth > 0.0f) ? (CurrentHealth / MaxHealth) : 0.0f;
}

bool ADinosaurBase::IsAlive() const
{
    return !bIsDead && CurrentHealth > 0.0f;
}

// ============================================================
// Death
// ============================================================

void ADinosaurBase::Die()
{
    if (bIsDead) return;
    bIsDead = true;

    // Clear timers
    GetWorldTimerManager().ClearTimer(HungerTimerHandle);
    GetWorldTimerManager().ClearTimer(StaminaTimerHandle);

    // Disable movement
    GetCharacterMovement()->DisableMovement();
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    UE_LOG(LogTemp, Log, TEXT("%s has died."), *GetName());

    OnDeath();
}

void ADinosaurBase::OnDeath()
{
    // Override in subclasses for species-specific death behavior
    // Default: ragdoll the mesh
    if (GetMesh())
    {
        GetMesh()->SetSimulatePhysics(true);
        GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    }
}

// ============================================================
// Combat
// ============================================================

bool ADinosaurBase::CanAttack() const
{
    if (bIsDead) return false;
    float CurrentTime = GetWorld()->GetTimeSeconds();
    return (CurrentTime - LastAttackTime) >= AttackCooldown;
}

void ADinosaurBase::PerformAttack(AActor* Target)
{
    if (!CanAttack() || !Target) return;

    LastAttackTime = GetWorld()->GetTimeSeconds();

    float DistToTarget = FVector::Dist(GetActorLocation(), Target->GetActorLocation());
    if (DistToTarget <= AttackRange)
    {
        // Apply damage to target
        UGameplayStatics::ApplyDamage(Target, AttackDamage, GetController(), this, nullptr);
        UE_LOG(LogTemp, Log, TEXT("%s attacked %s for %.1f damage"),
            *GetName(), *Target->GetName(), AttackDamage);
    }
}

void ADinosaurBase::OnAlerted(AActor* Threat)
{
    // Override in subclasses for species-specific alert behavior
    UE_LOG(LogTemp, Log, TEXT("%s is ALERTED by %s"),
        *GetName(), Threat ? *Threat->GetName() : TEXT("Unknown"));
}

// ============================================================
// Hunger & Stamina
// ============================================================

void ADinosaurBase::DrainHunger()
{
    if (bIsDead) return;
    Hunger = FMath::Clamp(Hunger + HungerDrainRate, 0.0f, MaxHunger);

    // Starving causes health drain
    if (Hunger >= MaxHunger)
    {
        ApplyDamage_Dino(2.0f, nullptr);
    }
}

void ADinosaurBase::RegenStamina()
{
    if (bIsDead || bIsRunning) return;
    CurrentStamina = FMath::Clamp(CurrentStamina + StaminaRegenRate, 0.0f, MaxStamina);
}

// ============================================================
// Utility
// ============================================================

FString ADinosaurBase::GetSpeciesName() const
{
    switch (DinosaurSpecies)
    {
        case EDinosaurSpecies::TyrannosaurusRex: return TEXT("Tyrannosaurus Rex");
        case EDinosaurSpecies::Velociraptor:     return TEXT("Velociraptor");
        case EDinosaurSpecies::Brachiosaurus:    return TEXT("Brachiosaurus");
        case EDinosaurSpecies::Triceratops:      return TEXT("Triceratops");
        case EDinosaurSpecies::Stegosaurus:      return TEXT("Stegosaurus");
        case EDinosaurSpecies::Pterodactyl:      return TEXT("Pterodactyl");
        case EDinosaurSpecies::Ankylosaurus:     return TEXT("Ankylosaurus");
        case EDinosaurSpecies::Spinosaurus:      return TEXT("Spinosaurus");
        default:                                  return TEXT("Unknown Dinosaur");
    }
}

bool ADinosaurBase::IsWithinDetectionRange(AActor* OtherActor) const
{
    if (!OtherActor) return false;
    return FVector::Dist(GetActorLocation(), OtherActor->GetActorLocation()) <= DetectionRadius;
}

bool ADinosaurBase::IsWithinAggressionRange(AActor* OtherActor) const
{
    if (!OtherActor) return false;
    return FVector::Dist(GetActorLocation(), OtherActor->GetActorLocation()) <= AggressionRadius;
}
