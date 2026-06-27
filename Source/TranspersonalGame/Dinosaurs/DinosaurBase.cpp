// DinosaurBase.cpp
// Engine Architect #02 — Transpersonal Game Studio
// Base class for all dinosaur actors in the prehistoric survival game.

#include "DinosaurBase.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "AIController.h"
#include "Net/UnrealNetwork.h"

ADinosaurBase::ADinosaurBase()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;

    // ── Capsule ──────────────────────────────────────────────────────────────
    GetCapsuleComponent()->InitCapsuleSize(80.f, 120.f);
    GetCapsuleComponent()->SetCollisionProfileName(TEXT("Pawn"));

    // ── Skeletal Mesh ────────────────────────────────────────────────────────
    GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -120.f));
    GetMesh()->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));

    // ── Movement ─────────────────────────────────────────────────────────────
    GetCharacterMovement()->MaxWalkSpeed          = 600.f;
    GetCharacterMovement()->MaxAcceleration       = 2048.f;
    GetCharacterMovement()->BrakingDecelerationWalking = 2048.f;
    GetCharacterMovement()->RotationRate          = FRotator(0.f, 540.f, 0.f);
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->GravityScale          = 1.0f;
    GetCharacterMovement()->JumpZVelocity         = 600.f;

    // ── AI Perception ────────────────────────────────────────────────────────
    AIPerception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));

    UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius               = 2000.f;
    SightConfig->LoseSightRadius           = 2500.f;
    SightConfig->PeripheralVisionAngleDegrees = 90.f;
    SightConfig->SetMaxAge(5.f);
    SightConfig->DetectionByAffiliation.bDetectEnemies   = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals  = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
    AIPerception->ConfigureSense(*SightConfig);

    UAISenseConfig_Hearing* HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    HearingConfig->HearingRange = 1500.f;
    HearingConfig->SetMaxAge(3.f);
    HearingConfig->DetectionByAffiliation.bDetectEnemies   = true;
    HearingConfig->DetectionByAffiliation.bDetectNeutrals  = true;
    HearingConfig->DetectionByAffiliation.bDetectFriendlies = false;
    AIPerception->ConfigureSense(*HearingConfig);

    AIPerception->SetDominantSense(SightConfig->GetSenseImplementation());

    // ── Default Stats ────────────────────────────────────────────────────────
    MaxHealth       = 500.f;
    CurrentHealth   = 500.f;
    AttackDamage    = 50.f;
    AttackRange     = 200.f;
    DetectionRadius = 2000.f;
    Species         = EDinoSpecies::Unknown;
    BehaviorState   = EDinoBehaviorState::Idle;
    bIsAggressive   = false;
    bIsPack         = false;
    PackRadius      = 1000.f;
    TerritoryRadius = 3000.f;

    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void ADinosaurBase::BeginPlay()
{
    Super::BeginPlay();

    CurrentHealth = MaxHealth;

    // Bind perception delegate
    if (AIPerception)
    {
        AIPerception->OnTargetPerceptionUpdated.AddDynamic(this, &ADinosaurBase::OnTargetPerceptionUpdated);
    }
}

void ADinosaurBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update behavior state machine
    UpdateBehaviorState(DeltaTime);
}

void ADinosaurBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ADinosaurBase, CurrentHealth);
    DOREPLIFETIME(ADinosaurBase, BehaviorState);
    DOREPLIFETIME(ADinosaurBase, bIsAggressive);
}

// ── Combat ───────────────────────────────────────────────────────────────────

float ADinosaurBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    const float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    CurrentHealth = FMath::Clamp(CurrentHealth - ActualDamage, 0.f, MaxHealth);

    if (CurrentHealth <= 0.f)
    {
        OnDeath();
    }
    else
    {
        // Become aggressive when hit
        if (!bIsAggressive && DamageCauser)
        {
            bIsAggressive = true;
            TargetActor   = DamageCauser;
            BehaviorState = EDinoBehaviorState::Attacking;
        }
    }

    return ActualDamage;
}

void ADinosaurBase::PerformAttack()
{
    if (!TargetActor) return;

    const float Dist = FVector::Dist(GetActorLocation(), TargetActor->GetActorLocation());
    if (Dist <= AttackRange)
    {
        FDamageEvent DmgEvent;
        TargetActor->TakeDamage(AttackDamage, DmgEvent, GetController(), this);
    }
}

// ── AI ───────────────────────────────────────────────────────────────────────

void ADinosaurBase::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (!Actor) return;

    if (Stimulus.WasSuccessfullySensed())
    {
        // Only react to player pawns
        if (Actor->ActorHasTag(TEXT("Player")))
        {
            TargetActor = Actor;
            if (bIsAggressive)
            {
                BehaviorState = EDinoBehaviorState::Attacking;
            }
            else
            {
                BehaviorState = EDinoBehaviorState::Fleeing;
            }
        }
    }
    else
    {
        // Lost sight — return to patrol
        if (TargetActor == Actor)
        {
            TargetActor   = nullptr;
            BehaviorState = EDinoBehaviorState::Patrolling;
        }
    }
}

void ADinosaurBase::UpdateBehaviorState(float DeltaTime)
{
    switch (BehaviorState)
    {
    case EDinoBehaviorState::Idle:
        // Idle: do nothing, wait for stimulus
        break;

    case EDinoBehaviorState::Patrolling:
        // Handled by Behavior Tree
        break;

    case EDinoBehaviorState::Chasing:
        if (TargetActor)
        {
            const float Dist = FVector::Dist(GetActorLocation(), TargetActor->GetActorLocation());
            if (Dist <= AttackRange)
            {
                BehaviorState = EDinoBehaviorState::Attacking;
            }
        }
        else
        {
            BehaviorState = EDinoBehaviorState::Patrolling;
        }
        break;

    case EDinoBehaviorState::Attacking:
        PerformAttack();
        break;

    case EDinoBehaviorState::Fleeing:
        // Move away from target — handled by BT
        break;

    case EDinoBehaviorState::Feeding:
        // Feeding animation — handled by BT
        break;

    case EDinoBehaviorState::Sleeping:
        // Sleep state — no movement
        break;

    default:
        break;
    }
}

void ADinosaurBase::OnDeath()
{
    BehaviorState = EDinoBehaviorState::Dead;

    // Disable collision and movement
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    GetCharacterMovement()->DisableMovement();

    // Ragdoll
    GetMesh()->SetSimulatePhysics(true);
    GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

    // Destroy after 30 seconds
    SetLifeSpan(30.f);
}

// ── Utility ──────────────────────────────────────────────────────────────────

float ADinosaurBase::GetHealthPercent() const
{
    return (MaxHealth > 0.f) ? (CurrentHealth / MaxHealth) : 0.f;
}

bool ADinosaurBase::IsAlive() const
{
    return CurrentHealth > 0.f;
}

void ADinosaurBase::SetAggressive(bool bAggressive)
{
    bIsAggressive = bAggressive;
    if (bIsAggressive && TargetActor)
    {
        BehaviorState = EDinoBehaviorState::Chasing;
    }
}
