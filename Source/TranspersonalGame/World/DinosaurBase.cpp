// DinosaurBase.cpp
// Engine Architect #02 — P2 Dinosaur AI Foundation
// Base class for all dinosaur pawns. Provides survival stats, biome registration,
// and stub hooks for Combat AI (#12) and NPC Behavior (#11).

#include "DinosaurBase.h"
#include "BiomeManager.h"
#include "TranspersonalGame/SharedTypes.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"

// -------------------------------------------------------
// Constructor
// -------------------------------------------------------
ADinosaurBase::ADinosaurBase()
{
    PrimaryActorTick.bCanEverTick = true;

    // Default survival stats
    MaxHealth           = 100.0f;
    CurrentHealth       = 100.0f;
    MaxStamina          = 100.0f;
    CurrentStamina      = 100.0f;
    MaxHunger           = 100.0f;
    CurrentHunger       = 80.0f;
    WalkSpeed           = 300.0f;
    RunSpeed            = 700.0f;
    AttackDamage        = 25.0f;
    AttackRange         = 150.0f;
    DetectionRadius     = 1500.0f;
    bIsAggressive       = false;
    bIsPredator         = false;
    DinosaurSpecies     = EDinosaurSpecies::Raptor;
    CurrentBiome        = EBiomeType::Plains;
    bIsAlive            = true;
    bIsRegisteredInBiome = false;

    // Capsule defaults — subclasses override for their body size
    if (GetCapsuleComponent())
    {
        GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);
    }

    // Movement defaults
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
        GetCharacterMovement()->JumpZVelocity = 400.0f;
        GetCharacterMovement()->GravityScale = 1.0f;
    }
}

// -------------------------------------------------------
// BeginPlay — register with BiomeManager
// -------------------------------------------------------
void ADinosaurBase::BeginPlay()
{
    Super::BeginPlay();

    RegisterWithBiomeManager(true);
}

// -------------------------------------------------------
// EndPlay — deregister from BiomeManager
// -------------------------------------------------------
void ADinosaurBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    RegisterWithBiomeManager(false);
    Super::EndPlay(EndPlayReason);
}

// -------------------------------------------------------
// Tick
// -------------------------------------------------------
void ADinosaurBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bIsAlive)
    {
        return;
    }

    // Drain hunger slowly over time (1 unit per 10 seconds)
    CurrentHunger = FMath::Max(0.0f, CurrentHunger - DeltaTime * 0.1f);

    // Stamina regeneration when not running
    const float CurrentSpeed = GetVelocity().Size();
    if (CurrentSpeed < WalkSpeed * 0.5f)
    {
        CurrentStamina = FMath::Min(MaxStamina, CurrentStamina + DeltaTime * 5.0f);
    }
}

// -------------------------------------------------------
// TakeDamage override
// -------------------------------------------------------
float ADinosaurBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
                                 AController* EventInstigator, AActor* DamageCauser)
{
    if (!bIsAlive)
    {
        return 0.0f;
    }

    const float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
    CurrentHealth = FMath::Max(0.0f, CurrentHealth - ActualDamage);

    OnDinosaurDamaged.Broadcast(ActualDamage, CurrentHealth);

    if (CurrentHealth <= 0.0f)
    {
        Die();
    }

    return ActualDamage;
}

// -------------------------------------------------------
// Die
// -------------------------------------------------------
void ADinosaurBase::Die()
{
    if (!bIsAlive)
    {
        return;
    }

    bIsAlive = false;
    RegisterWithBiomeManager(false);

    OnDinosaurDied.Broadcast(this);

    // Disable movement and collision
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->DisableMovement();
    }
    if (GetCapsuleComponent())
    {
        GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    // Destroy after 30 seconds (corpse despawn)
    SetLifeSpan(30.0f);
}

// -------------------------------------------------------
// IsAlive
// -------------------------------------------------------
bool ADinosaurBase::IsAlive() const
{
    return bIsAlive;
}

// -------------------------------------------------------
// GetHealthPercent
// -------------------------------------------------------
float ADinosaurBase::GetHealthPercent() const
{
    if (MaxHealth <= 0.0f) { return 0.0f; }
    return CurrentHealth / MaxHealth;
}

// -------------------------------------------------------
// SetAggressive
// -------------------------------------------------------
void ADinosaurBase::SetAggressive(bool bAggressive)
{
    bIsAggressive = bAggressive;
}

// -------------------------------------------------------
// RegisterWithBiomeManager — internal helper
// -------------------------------------------------------
void ADinosaurBase::RegisterWithBiomeManager(bool bRegistering)
{
    if (!GetWorld())
    {
        return;
    }

    // Find the BiomeManager in the world
    ABiomeManager* BiomeMgr = nullptr;
    for (TActorIterator<ABiomeManager> It(GetWorld()); It; ++It)
    {
        BiomeMgr = *It;
        break;
    }

    if (!BiomeMgr)
    {
        return;
    }

    // Determine which biome we are in
    CurrentBiome = BiomeMgr->GetBiomeAtLocation(GetActorLocation());
    BiomeMgr->RegisterDinosaurInBiome(CurrentBiome, bRegistering);
    bIsRegisteredInBiome = bRegistering;
}
