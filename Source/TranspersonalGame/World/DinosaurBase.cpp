// DinosaurBase.cpp
// Core Systems Programmer #03 — Cycle PROD_CYCLE_AUTO_20260623_007
// Base dinosaur pawn: movement, survival stats, biome-aware territory, basic AI sensing

#include "DinosaurBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NavigationSystem.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "TimerManager.h"

ADinosaurBase::ADinosaurBase()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.05f; // 20Hz — sufficient for AI

    // --- Capsule ---
    GetCapsuleComponent()->SetCapsuleHalfHeight(88.0f);
    GetCapsuleComponent()->SetCapsuleRadius(34.0f);
    GetCapsuleComponent()->SetCollisionProfileName(TEXT("Pawn"));

    // --- Mesh ---
    GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -88.0f));
    GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));

    // --- Movement ---
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    MoveComp->MaxWalkSpeed = 400.0f;
    MoveComp->MaxAcceleration = 800.0f;
    MoveComp->BrakingDecelerationWalking = 600.0f;
    MoveComp->RotationRate = FRotator(0.0f, 360.0f, 0.0f);
    MoveComp->bOrientRotationToMovement = true;
    MoveComp->bUseControllerDesiredRotation = false;
    MoveComp->GravityScale = 1.0f;
    MoveComp->JumpZVelocity = 420.0f;
    MoveComp->NavAgentProps.bCanJump = false;
    MoveComp->NavAgentProps.bCanWalk = true;
    MoveComp->NavAgentProps.AgentRadius = 34.0f;
    MoveComp->NavAgentProps.AgentHeight = 176.0f;

    // --- AI Perception ---
    PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComp"));

    UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = 2000.0f;
    SightConfig->LoseSightRadius = 2500.0f;
    SightConfig->PeripheralVisionAngleDegrees = 90.0f;
    SightConfig->SetMaxAge(5.0f);
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
    PerceptionComponent->ConfigureSense(*SightConfig);

    UAISenseConfig_Hearing* HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    HearingConfig->HearingRange = 1500.0f;
    HearingConfig->SetMaxAge(3.0f);
    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
    PerceptionComponent->ConfigureSense(*HearingConfig);

    PerceptionComponent->SetDominantSense(*SightConfig->GetSenseImplementation());

    // --- Default stats (overridden by subclasses) ---
    DinoSpecies = EDinoSpecies::Unknown;
    DinoState = EDinoState::Idle;
    MaxHealth = 100.0f;
    CurrentHealth = 100.0f;
    MaxStamina = 100.0f;
    CurrentStamina = 100.0f;
    Hunger = 50.0f;
    HungerDecayRate = 1.0f;
    AttackDamage = 20.0f;
    AttackRange = 150.0f;
    AttackCooldown = 2.0f;
    bIsAlive = true;
    bIsPredator = false;
    TerritoryRadius = 3000.0f;
    PatrolRadius = 1500.0f;
    HomeLocation = FVector::ZeroVector;
    bHomeLocked = false;
    CurrentBiome = EDinoBiome::Grassland;
    LastAttackTime = -999.0f;
    bIsAggressive = false;
    ThreatLevel = 0.0f;
}

void ADinosaurBase::BeginPlay()
{
    Super::BeginPlay();

    // Lock home location on spawn
    HomeLocation = GetActorLocation();
    bHomeLocked = true;

    // Bind perception
    if (PerceptionComponent)
    {
        PerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ADinosaurBase::OnPerceptionUpdated);
    }

    // Start hunger decay timer
    GetWorldTimerManager().SetTimer(
        HungerTimerHandle,
        this,
        &ADinosaurBase::TickHunger,
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

    // Determine biome from elevation (mirrors BiomeManager logic)
    UpdateCurrentBiome();
}

void ADinosaurBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bIsAlive) return;

    // Update threat decay
    if (ThreatLevel > 0.0f)
    {
        ThreatLevel = FMath::Max(0.0f, ThreatLevel - DeltaTime * 5.0f);
        if (ThreatLevel <= 0.0f)
        {
            bIsAggressive = false;
        }
    }

#if WITH_EDITOR
    if (bDrawDebugInfo)
    {
        DrawDebugSphere(GetWorld(), GetActorLocation(), TerritoryRadius, 16, FColor::Yellow, false, -1.0f, 0, 2.0f);
        DrawDebugSphere(GetWorld(), HomeLocation, 100.0f, 8, FColor::Green, false, -1.0f, 0, 3.0f);
        DrawDebugString(GetWorld(), GetActorLocation() + FVector(0,0,200), 
            FString::Printf(TEXT("%s | HP:%.0f | State:%d | Biome:%d"), 
                *GetName(), CurrentHealth, (int32)DinoState, (int32)CurrentBiome),
            nullptr, FColor::White, 0.0f, true);
    }
#endif
}

// ─── Combat ───────────────────────────────────────────────────────────────────

float ADinosaurBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    if (!bIsAlive) return 0.0f;

    const float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
    CurrentHealth = FMath::Clamp(CurrentHealth - ActualDamage, 0.0f, MaxHealth);

    // Increase threat when hit
    ThreatLevel = FMath::Min(100.0f, ThreatLevel + ActualDamage * 0.5f);
    if (ThreatLevel > 30.0f)
    {
        bIsAggressive = true;
    }

    OnDinoHit(ActualDamage, DamageCauser);

    if (CurrentHealth <= 0.0f)
    {
        Die();
    }

    return ActualDamage;
}

void ADinosaurBase::PerformAttack(AActor* Target)
{
    if (!bIsAlive || !Target) return;

    const float Now = GetWorld()->GetTimeSeconds();
    if (Now - LastAttackTime < AttackCooldown) return;

    const float DistSq = FVector::DistSquared(GetActorLocation(), Target->GetActorLocation());
    if (DistSq > FMath::Square(AttackRange)) return;

    LastAttackTime = Now;

    // Apply damage
    FDamageEvent DmgEvent;
    Target->TakeDamage(AttackDamage, DmgEvent, GetController(), this);

    // Drain stamina
    CurrentStamina = FMath::Max(0.0f, CurrentStamina - 15.0f);

    OnAttackPerformed(Target);
}

void ADinosaurBase::Die()
{
    if (!bIsAlive) return;
    bIsAlive = false;
    DinoState = EDinoState::Dead;

    GetCharacterMovement()->DisableMovement();
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Clear timers
    GetWorldTimerManager().ClearTimer(HungerTimerHandle);
    GetWorldTimerManager().ClearTimer(StaminaTimerHandle);

    OnDinoDied();

    // Auto-destroy after 30s
    SetLifeSpan(30.0f);
}

// ─── Survival ─────────────────────────────────────────────────────────────────

void ADinosaurBase::TickHunger()
{
    if (!bIsAlive) return;
    Hunger = FMath::Max(0.0f, Hunger - HungerDecayRate);

    // Starving causes health drain
    if (Hunger <= 0.0f)
    {
        CurrentHealth = FMath::Max(0.0f, CurrentHealth - 2.0f);
        if (CurrentHealth <= 0.0f) Die();
    }
}

void ADinosaurBase::RegenStamina()
{
    if (!bIsAlive) return;
    if (DinoState != EDinoState::Sprinting)
    {
        CurrentStamina = FMath::Min(MaxStamina, CurrentStamina + 5.0f);
    }
}

void ADinosaurBase::Feed(float Amount)
{
    Hunger = FMath::Min(100.0f, Hunger + Amount);
    // Feeding also restores some health
    CurrentHealth = FMath::Min(MaxHealth, CurrentHealth + Amount * 0.2f);
}

// ─── Biome ────────────────────────────────────────────────────────────────────

void ADinosaurBase::UpdateCurrentBiome()
{
    const float Z = GetActorLocation().Z;
    if (Z > 5000.0f)
        CurrentBiome = EDinoBiome::Volcanic;
    else if (Z > 2000.0f)
        CurrentBiome = EDinoBiome::Tundra;
    else
    {
        // Horizontal position heuristic — matches BiomeManager grid
        const FVector Loc = GetActorLocation();
        const float AbsX = FMath::Abs(Loc.X);
        const float AbsY = FMath::Abs(Loc.Y);
        if (AbsX > 40000.0f)
            CurrentBiome = EDinoBiome::Desert;
        else if (AbsY > 40000.0f)
            CurrentBiome = EDinoBiome::Swamp;
        else if (AbsX > 20000.0f || AbsY > 20000.0f)
            CurrentBiome = EDinoBiome::Forest;
        else
            CurrentBiome = EDinoBiome::Grassland;
    }
}

bool ADinosaurBase::IsInPreferredBiome() const
{
    return CurrentBiome == PreferredBiome;
}

bool ADinosaurBase::IsInTerritory(const FVector& Location) const
{
    return FVector::DistSquared(Location, HomeLocation) <= FMath::Square(TerritoryRadius);
}

// ─── Perception ───────────────────────────────────────────────────────────────

void ADinosaurBase::OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (!bIsAlive || !Actor) return;

    if (Stimulus.WasSuccessfullySensed())
    {
        ThreatLevel = FMath::Min(100.0f, ThreatLevel + 20.0f);
        if (bIsPredator && ThreatLevel > 20.0f)
        {
            bIsAggressive = true;
            DinoState = EDinoState::Hunting;
        }
        else if (!bIsPredator)
        {
            DinoState = EDinoState::Fleeing;
        }
        OnThreatDetected(Actor);
    }
    else
    {
        // Lost sight — return to patrol if not still threatened
        if (ThreatLevel < 20.0f)
        {
            DinoState = EDinoState::Patrolling;
            bIsAggressive = false;
        }
    }
}

// ─── Blueprint-callable helpers ───────────────────────────────────────────────

void ADinosaurBase::SetDinoState(EDinoState NewState)
{
    if (DinoState == EDinoState::Dead) return;
    DinoState = NewState;
}

float ADinosaurBase::GetHealthPercent() const
{
    return MaxHealth > 0.0f ? CurrentHealth / MaxHealth : 0.0f;
}

float ADinosaurBase::GetStaminaPercent() const
{
    return MaxStamina > 0.0f ? CurrentStamina / MaxStamina : 0.0f;
}

// ─── Overridable events (empty base implementations) ─────────────────────────

void ADinosaurBase::OnDinoHit_Implementation(float DamageAmount, AActor* DamageCauser) {}
void ADinosaurBase::OnDinoDied_Implementation() {}
void ADinosaurBase::OnAttackPerformed_Implementation(AActor* Target) {}
void ADinosaurBase::OnThreatDetected_Implementation(AActor* ThreatActor) {}
