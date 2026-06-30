// DinosaurBase.cpp
// Engine Architect #02 — Transpersonal Game Studio
// Base implementation for all dinosaur types in the prehistoric survival game.
// Provides movement, perception, health, territory, and attack systems.

#include "DinosaurBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"

ADinosaurBase::ADinosaurBase()
{
    PrimaryActorTick.bCanEverTick = true;

    // --- Capsule ---
    GetCapsuleComponent()->InitCapsuleSize(60.0f, 120.0f);
    GetCapsuleComponent()->SetCollisionProfileName(TEXT("Pawn"));

    // --- Movement ---
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 360.0f, 0.0f);
    GetCharacterMovement()->MaxWalkSpeed = 400.0f;
    GetCharacterMovement()->JumpZVelocity = 600.0f;
    GetCharacterMovement()->AirControl = 0.2f;
    GetCharacterMovement()->NavAgentProps.bCanCrouch = false;
    GetCharacterMovement()->NavAgentProps.bCanJump = true;

    // --- AI Perception ---
    AIPerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));

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

    AIPerceptionComp->SetDominantSense(*SightConfig->GetSenseImplementation());

    // --- Default Stats ---
    MaxHealth = 500.0f;
    CurrentHealth = MaxHealth;
    MaxStamina = 200.0f;
    CurrentStamina = MaxStamina;
    AttackDamage = 50.0f;
    AttackRange = 200.0f;
    AttackCooldown = 1.5f;
    WalkSpeed = 300.0f;
    RunSpeed = 700.0f;
    TerritoryRadius = 3000.0f;
    DetectionRadius = 2000.0f;
    bIsAggressive = false;
    bIsAlerted = false;
    bIsDead = false;
    DinosaurSpecies = EDinosaurSpecies::Unknown;
    BehaviorState = EDinosaurBehavior::Idle;
    bCanAttack = true;
    StaminaRegenRate = 20.0f;
    StaminaDrainRate = 30.0f;

    // Auto-possess by AI
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void ADinosaurBase::BeginPlay()
{
    Super::BeginPlay();

    // Store home territory location
    TerritoryCenter = GetActorLocation();

    // Start idle behavior timer
    GetWorldTimerManager().SetTimer(
        BehaviorUpdateTimer,
        this,
        &ADinosaurBase::UpdateBehaviorState,
        2.0f,
        true
    );

    // Stamina regen timer
    GetWorldTimerManager().SetTimer(
        StaminaRegenTimer,
        this,
        &ADinosaurBase::RegenerateStamina,
        0.5f,
        true
    );

    // Bind perception delegate
    if (AIPerceptionComp)
    {
        AIPerceptionComp->OnTargetPerceptionUpdated.AddDynamic(this, &ADinosaurBase::OnPerceptionUpdated);
    }

    // Set movement speed
    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

void ADinosaurBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Drain stamina when running
    if (GetVelocity().SizeSquared() > (WalkSpeed * WalkSpeed * 1.1f))
    {
        CurrentStamina = FMath::Max(0.0f, CurrentStamina - StaminaDrainRate * DeltaTime);
        if (CurrentStamina <= 0.0f)
        {
            // Force walk when exhausted
            GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
        }
    }
}

float ADinosaurBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    if (bIsDead) return 0.0f;

    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
    CurrentHealth = FMath::Max(0.0f, CurrentHealth - ActualDamage);

    // Alert on damage
    if (!bIsAlerted)
    {
        bIsAlerted = true;
        BehaviorState = EDinosaurBehavior::Alert;
    }

    // Aggression response
    if (bIsAggressive && DamageCauser)
    {
        AttackTarget = DamageCauser;
        BehaviorState = EDinosaurBehavior::Attacking;
    }

    if (CurrentHealth <= 0.0f)
    {
        Die();
    }

    return ActualDamage;
}

void ADinosaurBase::PerformAttack()
{
    if (!bCanAttack || bIsDead) return;

    // Sweep for targets in attack range
    TArray<FOverlapResult> Overlaps;
    FCollisionShape AttackSphere = FCollisionShape::MakeSphere(AttackRange);
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);

    bool bHit = GetWorld()->OverlapMultiByChannel(
        Overlaps,
        GetActorLocation() + GetActorForwardVector() * (AttackRange * 0.5f),
        FQuat::Identity,
        ECC_Pawn,
        AttackSphere,
        QueryParams
    );

    if (bHit)
    {
        for (const FOverlapResult& Overlap : Overlaps)
        {
            AActor* HitActor = Overlap.GetActor();
            if (HitActor && HitActor != this)
            {
                FDamageEvent DmgEvent;
                HitActor->TakeDamage(AttackDamage, DmgEvent, GetController(), this);
            }
        }
    }

    // Start attack cooldown
    bCanAttack = false;
    GetWorldTimerManager().SetTimer(
        AttackCooldownTimer,
        this,
        &ADinosaurBase::ResetAttackCooldown,
        AttackCooldown,
        false
    );
}

void ADinosaurBase::Die()
{
    if (bIsDead) return;
    bIsDead = true;
    BehaviorState = EDinosaurBehavior::Dead;

    // Stop all timers
    GetWorldTimerManager().ClearTimer(BehaviorUpdateTimer);
    GetWorldTimerManager().ClearTimer(StaminaRegenTimer);
    GetWorldTimerManager().ClearTimer(AttackCooldownTimer);

    // Disable collision
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    GetCharacterMovement()->DisableMovement();

    // Ragdoll
    GetMesh()->SetSimulatePhysics(true);
    GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

    // Destroy after 30 seconds
    SetLifeSpan(30.0f);
}

void ADinosaurBase::SetAlertState(bool bAlert)
{
    bIsAlerted = bAlert;
    if (bAlert && BehaviorState == EDinosaurBehavior::Idle)
    {
        BehaviorState = EDinosaurBehavior::Alert;
    }
}

bool ADinosaurBase::IsInTerritory(const FVector& Location) const
{
    return FVector::Dist(Location, TerritoryCenter) <= TerritoryRadius;
}

void ADinosaurBase::UpdateBehaviorState()
{
    if (bIsDead) return;

    switch (BehaviorState)
    {
    case EDinosaurBehavior::Idle:
        // Randomly patrol
        if (FMath::RandBool())
        {
            BehaviorState = EDinosaurBehavior::Patrolling;
        }
        break;

    case EDinosaurBehavior::Patrolling:
        // Check if we've wandered too far from territory
        if (!IsInTerritory(GetActorLocation()))
        {
            BehaviorState = EDinosaurBehavior::Returning;
        }
        else if (FMath::RandRange(0, 4) == 0)
        {
            BehaviorState = EDinosaurBehavior::Idle;
        }
        break;

    case EDinosaurBehavior::Returning:
        // Return to territory center
        if (IsInTerritory(GetActorLocation()))
        {
            BehaviorState = EDinosaurBehavior::Idle;
        }
        break;

    case EDinosaurBehavior::Alert:
        // If aggressive, transition to chase
        if (bIsAggressive && AttackTarget)
        {
            BehaviorState = EDinosaurBehavior::Chasing;
        }
        else if (!bIsAlerted)
        {
            BehaviorState = EDinosaurBehavior::Patrolling;
        }
        break;

    case EDinosaurBehavior::Chasing:
        if (!AttackTarget || !IsValid(AttackTarget))
        {
            AttackTarget = nullptr;
            BehaviorState = EDinosaurBehavior::Alert;
        }
        else
        {
            float DistToTarget = FVector::Dist(GetActorLocation(), AttackTarget->GetActorLocation());
            if (DistToTarget <= AttackRange)
            {
                BehaviorState = EDinosaurBehavior::Attacking;
            }
        }
        break;

    case EDinosaurBehavior::Attacking:
        PerformAttack();
        if (!AttackTarget || !IsValid(AttackTarget))
        {
            AttackTarget = nullptr;
            BehaviorState = EDinosaurBehavior::Alert;
        }
        break;

    case EDinosaurBehavior::Fleeing:
        // Flee until safe distance from threat
        if (CurrentStamina < 10.0f)
        {
            BehaviorState = EDinosaurBehavior::Idle;
        }
        break;

    default:
        break;
    }
}

void ADinosaurBase::RegenerateStamina()
{
    if (bIsDead) return;

    // Only regen when not running
    if (GetVelocity().SizeSquared() <= (WalkSpeed * WalkSpeed * 1.1f))
    {
        CurrentStamina = FMath::Min(MaxStamina, CurrentStamina + StaminaRegenRate * 0.5f);
        if (CurrentStamina >= MaxStamina * 0.5f)
        {
            GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
        }
    }
}

void ADinosaurBase::ResetAttackCooldown()
{
    bCanAttack = true;
}

void ADinosaurBase::OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (!Actor || bIsDead) return;

    if (Stimulus.WasSuccessfullySensed())
    {
        // Detected something
        bIsAlerted = true;
        if (bIsAggressive)
        {
            AttackTarget = Actor;
            BehaviorState = EDinosaurBehavior::Chasing;
            GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
        }
        else
        {
            BehaviorState = EDinosaurBehavior::Alert;
        }
    }
    else
    {
        // Lost perception
        if (AttackTarget == Actor)
        {
            AttackTarget = nullptr;
            bIsAlerted = false;
            BehaviorState = EDinosaurBehavior::Returning;
            GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
        }
    }
}
