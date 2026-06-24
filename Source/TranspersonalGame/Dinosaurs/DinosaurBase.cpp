// DinosaurBase.cpp
// Engine Architect #02 — Transpersonal Game Studio
// Base implementation for all dinosaur types in the prehistoric survival game.
// All dinosaur species (TRex, Raptor, Triceratops, Brachiosaurus, etc.) inherit from this class.

#include "DinosaurBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISense_Hearing.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "AIController.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

ADinosaurBase::ADinosaurBase()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.05f; // 20Hz tick — sufficient for AI

    // --- Capsule ---
    GetCapsuleComponent()->InitCapsuleSize(60.0f, 120.0f);
    GetCapsuleComponent()->SetCollisionProfileName(TEXT("Pawn"));

    // --- Mesh ---
    GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -120.0f));
    GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));

    // --- Movement ---
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    MoveComp->bOrientRotationToMovement = true;
    MoveComp->RotationRate = FRotator(0.0f, 360.0f, 0.0f);
    MoveComp->MaxWalkSpeed = 400.0f;
    MoveComp->MaxAcceleration = 1200.0f;
    MoveComp->BrakingDecelerationWalking = 800.0f;
    MoveComp->JumpZVelocity = 500.0f;
    MoveComp->GravityScale = 1.5f;
    MoveComp->NavAgentProps.bCanCrouch = false;
    MoveComp->NavAgentProps.bCanJump = true;

    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    // --- AI Perception Stimuli Source ---
    PerceptionStimuliSource = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>(TEXT("PerceptionStimuliSource"));
    PerceptionStimuliSource->RegisterForSense(TSubclassOf<UAISense>(UAISense_Sight::StaticClass()));
    PerceptionStimuliSource->RegisterForSense(TSubclassOf<UAISense>(UAISense_Hearing::StaticClass()));
    PerceptionStimuliSource->bAutoRegister = true;

    // --- Default Species Stats ---
    Species = EDinoSpecies::Unknown;
    BehaviorState = EDinoBehaviorState::Idle;

    MaxHealth = 500.0f;
    CurrentHealth = 500.0f;
    AttackDamage = 50.0f;
    AttackRange = 200.0f;
    AttackCooldown = 2.0f;
    DetectionRadius = 2000.0f;
    AggroRadius = 800.0f;
    bIsAggressive = false;
    bIsHerbivore = false;
    bIsPackHunter = false;
    PackSize = 1;

    HungerLevel = 100.0f;
    ThirstLevel = 100.0f;
    HungerDecayRate = 2.0f;  // per minute
    ThirstDecayRate = 3.0f;  // per minute

    TerritoryRadius = 3000.0f;
    HomeLocation = FVector::ZeroVector;
    bHasTerritoryHome = false;

    bCanBeRidden = false;
    bIsDead = false;
    bIsAttacking = false;

    LastAttackTime = -999.0f;
}

void ADinosaurBase::BeginPlay()
{
    Super::BeginPlay();

    // Record home territory
    HomeLocation = GetActorLocation();
    bHasTerritoryHome = true;

    // Start survival tick timers
    GetWorldTimerManager().SetTimer(
        HungerTimerHandle,
        this,
        &ADinosaurBase::TickHunger,
        60.0f,   // every 60 seconds = 1 game minute
        true
    );

    GetWorldTimerManager().SetTimer(
        ThirstTimerHandle,
        this,
        &ADinosaurBase::TickThirst,
        60.0f,
        true
    );

    // Initialize species-specific stats
    InitializeSpeciesStats();

    UE_LOG(LogTemp, Log, TEXT("DinosaurBase[%s] spawned — Species:%d Health:%.0f Aggressive:%d"),
        *GetActorLabel(), (int32)Species, CurrentHealth, bIsAggressive ? 1 : 0);
}

void ADinosaurBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Basic behavior state machine tick
    UpdateBehaviorState(DeltaTime);
}

void ADinosaurBase::InitializeSpeciesStats()
{
    // Override in subclasses or set via Blueprint defaults.
    // Base implementation provides sensible defaults per species enum.
    switch (Species)
    {
    case EDinoSpecies::TyrannosaurusRex:
        MaxHealth = 2000.0f;
        CurrentHealth = 2000.0f;
        AttackDamage = 200.0f;
        AttackRange = 300.0f;
        AttackCooldown = 3.0f;
        DetectionRadius = 4000.0f;
        AggroRadius = 2000.0f;
        bIsAggressive = true;
        bIsHerbivore = false;
        bIsPackHunter = false;
        PackSize = 1;
        TerritoryRadius = 8000.0f;
        GetCharacterMovement()->MaxWalkSpeed = 600.0f;
        break;

    case EDinoSpecies::Velociraptor:
        MaxHealth = 300.0f;
        CurrentHealth = 300.0f;
        AttackDamage = 80.0f;
        AttackRange = 150.0f;
        AttackCooldown = 0.8f;
        DetectionRadius = 3000.0f;
        AggroRadius = 1500.0f;
        bIsAggressive = true;
        bIsHerbivore = false;
        bIsPackHunter = true;
        PackSize = 4;
        TerritoryRadius = 5000.0f;
        GetCharacterMovement()->MaxWalkSpeed = 800.0f;
        break;

    case EDinoSpecies::Triceratops:
        MaxHealth = 1500.0f;
        CurrentHealth = 1500.0f;
        AttackDamage = 120.0f;
        AttackRange = 250.0f;
        AttackCooldown = 2.5f;
        DetectionRadius = 2000.0f;
        AggroRadius = 600.0f;  // Only aggro when threatened
        bIsAggressive = false;
        bIsHerbivore = true;
        bIsPackHunter = false;
        PackSize = 3;
        TerritoryRadius = 4000.0f;
        GetCharacterMovement()->MaxWalkSpeed = 450.0f;
        break;

    case EDinoSpecies::Brachiosaurus:
        MaxHealth = 3000.0f;
        CurrentHealth = 3000.0f;
        AttackDamage = 150.0f;  // Stomp damage
        AttackRange = 400.0f;
        AttackCooldown = 4.0f;
        DetectionRadius = 1500.0f;
        AggroRadius = 400.0f;  // Very passive
        bIsAggressive = false;
        bIsHerbivore = true;
        bIsPackHunter = false;
        PackSize = 2;
        TerritoryRadius = 6000.0f;
        GetCharacterMovement()->MaxWalkSpeed = 300.0f;
        break;

    case EDinoSpecies::Pteranodon:
        MaxHealth = 200.0f;
        CurrentHealth = 200.0f;
        AttackDamage = 40.0f;
        AttackRange = 200.0f;
        AttackCooldown = 1.5f;
        DetectionRadius = 5000.0f;  // High aerial vision
        AggroRadius = 1000.0f;
        bIsAggressive = true;
        bIsHerbivore = false;
        bIsPackHunter = false;
        PackSize = 1;
        TerritoryRadius = 10000.0f;
        GetCharacterMovement()->MaxWalkSpeed = 200.0f;  // Slow on ground
        break;

    case EDinoSpecies::Stegosaurus:
        MaxHealth = 1200.0f;
        CurrentHealth = 1200.0f;
        AttackDamage = 90.0f;  // Tail spike
        AttackRange = 300.0f;
        AttackCooldown = 2.0f;
        DetectionRadius = 1800.0f;
        AggroRadius = 500.0f;
        bIsAggressive = false;
        bIsHerbivore = true;
        bIsPackHunter = false;
        PackSize = 2;
        TerritoryRadius = 3500.0f;
        GetCharacterMovement()->MaxWalkSpeed = 350.0f;
        break;

    default:
        // Unknown — keep constructor defaults
        break;
    }
}

void ADinosaurBase::UpdateBehaviorState(float DeltaTime)
{
    if (bIsDead) return;

    // Simple state machine — full BehaviorTree integration done by Agent #12
    switch (BehaviorState)
    {
    case EDinoBehaviorState::Idle:
        // Transition to Wandering after a short idle period
        // (BehaviorTree will override this in full implementation)
        break;

    case EDinoBehaviorState::Wandering:
        // Random patrol within territory — handled by BT
        break;

    case EDinoBehaviorState::Hunting:
        // Pursuit logic — handled by BT
        break;

    case EDinoBehaviorState::Fleeing:
        // Flee from threat — handled by BT
        break;

    case EDinoBehaviorState::Attacking:
        // Attack target — handled by BT
        break;

    case EDinoBehaviorState::Feeding:
        // Eating — restore hunger
        HungerLevel = FMath::Min(100.0f, HungerLevel + DeltaTime * 10.0f);
        break;

    case EDinoBehaviorState::Resting:
        // Resting — restore health slowly
        CurrentHealth = FMath::Min(MaxHealth, CurrentHealth + DeltaTime * 5.0f);
        break;

    default:
        break;
    }
}

float ADinosaurBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    if (bIsDead) return 0.0f;

    const float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
    CurrentHealth -= ActualDamage;

    UE_LOG(LogTemp, Log, TEXT("DinosaurBase[%s] took %.0f damage — Health: %.0f/%.0f"),
        *GetActorLabel(), ActualDamage, CurrentHealth, MaxHealth);

    // Aggro on damage — even herbivores fight back
    if (DamageCauser && !bIsDead)
    {
        BehaviorState = EDinoBehaviorState::Attacking;
    }

    if (CurrentHealth <= 0.0f)
    {
        Die();
    }

    return ActualDamage;
}

void ADinosaurBase::Die()
{
    if (bIsDead) return;
    bIsDead = true;
    BehaviorState = EDinoBehaviorState::Dead;

    // Stop movement
    GetCharacterMovement()->StopMovementImmediately();
    GetCharacterMovement()->DisableMovement();

    // Disable collision
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Clear timers
    GetWorldTimerManager().ClearTimer(HungerTimerHandle);
    GetWorldTimerManager().ClearTimer(ThirstTimerHandle);

    // Notify AI controller
    if (AController* Ctrl = GetController())
    {
        Ctrl->UnPossess();
    }

    UE_LOG(LogTemp, Log, TEXT("DinosaurBase[%s] DIED"), *GetActorLabel());

    // Blueprint event for death VFX/sound (Agent #17 / #16)
    OnDinosaurDied();
}

void ADinosaurBase::OnDinosaurDied_Implementation()
{
    // Default: ragdoll on death
    GetMesh()->SetSimulatePhysics(true);
    GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

    // Destroy after 30 seconds (corpse cleanup)
    SetLifeSpan(30.0f);
}

bool ADinosaurBase::CanAttack() const
{
    if (bIsDead || bIsAttacking) return false;
    const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    return (Now - LastAttackTime) >= AttackCooldown;
}

void ADinosaurBase::PerformAttack(AActor* Target)
{
    if (!Target || !CanAttack()) return;

    bIsAttacking = true;
    LastAttackTime = GetWorld()->GetTimeSeconds();

    // Apply damage
    const float Distance = FVector::Dist(GetActorLocation(), Target->GetActorLocation());
    if (Distance <= AttackRange)
    {
        FDamageEvent DmgEvent;
        Target->TakeDamage(AttackDamage, DmgEvent, GetController(), this);
        UE_LOG(LogTemp, Log, TEXT("DinosaurBase[%s] attacked [%s] for %.0f damage"),
            *GetActorLabel(), *Target->GetActorLabel(), AttackDamage);
    }

    // Reset attack flag after cooldown
    FTimerHandle AttackResetTimer;
    GetWorldTimerManager().SetTimer(AttackResetTimer, [this]()
    {
        bIsAttacking = false;
    }, AttackCooldown * 0.5f, false);
}

void ADinosaurBase::TickHunger()
{
    HungerLevel = FMath::Max(0.0f, HungerLevel - HungerDecayRate);
    if (HungerLevel <= 20.0f && !bIsHerbivore)
    {
        // Carnivores become more aggressive when hungry
        bIsAggressive = true;
        AggroRadius = DetectionRadius * 0.8f;
    }
    if (HungerLevel <= 0.0f)
    {
        // Starvation damage
        TakeDamage(10.0f, FDamageEvent(), nullptr, nullptr);
    }
}

void ADinosaurBase::TickThirst()
{
    ThirstLevel = FMath::Max(0.0f, ThirstLevel - ThirstDecayRate);
    if (ThirstLevel <= 0.0f)
    {
        // Dehydration damage
        TakeDamage(5.0f, FDamageEvent(), nullptr, nullptr);
    }
}

bool ADinosaurBase::IsWithinTerritory(const FVector& Location) const
{
    if (!bHasTerritoryHome) return true;
    return FVector::Dist(HomeLocation, Location) <= TerritoryRadius;
}

void ADinosaurBase::SetBehaviorState(EDinoBehaviorState NewState)
{
    if (BehaviorState == NewState) return;
    BehaviorState = NewState;
    UE_LOG(LogTemp, Verbose, TEXT("DinosaurBase[%s] state: %d -> %d"),
        *GetActorLabel(), (int32)BehaviorState, (int32)NewState);
}

float ADinosaurBase::GetHealthPercent() const
{
    if (MaxHealth <= 0.0f) return 0.0f;
    return CurrentHealth / MaxHealth;
}
