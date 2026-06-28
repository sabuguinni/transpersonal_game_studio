// DinosaurBase.cpp
// Engine Architect #02 — Transpersonal Game Studio
// Base class for all dinosaur pawns in the prehistoric survival game.

#include "DinosaurBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "AIController.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"

ADinosaurBase::ADinosaurBase()
{
    PrimaryActorTick.bCanEverTick = true;

    // --- Capsule ---
    GetCapsuleComponent()->InitCapsuleSize(50.f, 100.f);
    GetCapsuleComponent()->SetCollisionProfileName(TEXT("Pawn"));

    // --- Mesh ---
    GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -100.f));
    GetMesh()->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));

    // --- Movement ---
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        MoveComp->bOrientRotationToMovement = true;
        MoveComp->RotationRate = FRotator(0.f, 540.f, 0.f);
        MoveComp->MaxWalkSpeed = 400.f;
        MoveComp->JumpZVelocity = 600.f;
        MoveComp->AirControl = 0.2f;
        MoveComp->NavAgentProps.bCanCrouch = false;
    }

    // --- AI Perception ---
    AIPerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComp"));

    UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = 2000.f;
    SightConfig->LoseSightRadius = 2500.f;
    SightConfig->PeripheralVisionAngleDegrees = 90.f;
    SightConfig->SetMaxAge(5.f);
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
    AIPerceptionComp->ConfigureSense(*SightConfig);

    UAISenseConfig_Hearing* HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    HearingConfig->HearingRange = 1500.f;
    HearingConfig->SetMaxAge(3.f);
    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
    AIPerceptionComp->ConfigureSense(*HearingConfig);
    AIPerceptionComp->SetDominantSense(SightConfig->GetSenseImplementation());

    // --- Default Species Stats ---
    Species = EDinoSpecies::Raptor;
    DinoHealth = 100.f;
    MaxDinoHealth = 100.f;
    DinoStamina = 100.f;
    MaxDinoStamina = 100.f;
    HungerLevel = 50.f;
    MaxHunger = 100.f;
    AttackDamage = 25.f;
    AttackRange = 150.f;
    DetectionRange = 2000.f;
    ChaseRange = 3000.f;
    WalkSpeed = 200.f;
    RunSpeed = 600.f;
    bIsAggressive = false;
    bIsPackHunter = false;
    bIsPredator = true;
    CurrentBehaviorState = EDinoBehaviorState::Idle;
    PackSize = 1;

    // --- Auto-possess by AI ---
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void ADinosaurBase::BeginPlay()
{
    Super::BeginPlay();

    // Apply species-specific stats
    ApplySpeciesStats();

    // Start hunger drain timer
    GetWorldTimerManager().SetTimer(
        HungerTimerHandle,
        this,
        &ADinosaurBase::DrainHunger,
        10.f,
        true
    );

    // Start stamina regen timer
    GetWorldTimerManager().SetTimer(
        StaminaRegenTimerHandle,
        this,
        &ADinosaurBase::RegenStamina,
        2.f,
        true
    );

    // Bind perception delegate
    if (AIPerceptionComp)
    {
        AIPerceptionComp->OnTargetPerceptionUpdated.AddDynamic(this, &ADinosaurBase::OnPerceptionUpdated);
    }
}

void ADinosaurBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update movement speed based on behavior state
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        switch (CurrentBehaviorState)
        {
        case EDinoBehaviorState::Chasing:
        case EDinoBehaviorState::Attacking:
            MoveComp->MaxWalkSpeed = RunSpeed;
            break;
        case EDinoBehaviorState::Fleeing:
            MoveComp->MaxWalkSpeed = RunSpeed * 1.2f;
            break;
        default:
            MoveComp->MaxWalkSpeed = WalkSpeed;
            break;
        }
    }
}

float ADinosaurBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    DinoHealth = FMath::Clamp(DinoHealth - ActualDamage, 0.f, MaxDinoHealth);

    if (DinoHealth <= 0.f)
    {
        OnDinosaurDeath();
    }
    else if (bIsPredator && ActualDamage > 0.f)
    {
        // Predators become aggressive when hurt
        bIsAggressive = true;
        SetBehaviorState(EDinoBehaviorState::Chasing);
    }

    return ActualDamage;
}

void ADinosaurBase::ApplySpeciesStats()
{
    switch (Species)
    {
    case EDinoSpecies::TRex:
        MaxDinoHealth = 500.f;
        DinoHealth = 500.f;
        AttackDamage = 100.f;
        AttackRange = 250.f;
        DetectionRange = 2500.f;
        ChaseRange = 4000.f;
        WalkSpeed = 250.f;
        RunSpeed = 700.f;
        bIsAggressive = true;
        bIsPredator = true;
        bIsPackHunter = false;
        PackSize = 1;
        GetCapsuleComponent()->InitCapsuleSize(100.f, 200.f);
        break;

    case EDinoSpecies::Raptor:
        MaxDinoHealth = 120.f;
        DinoHealth = 120.f;
        AttackDamage = 35.f;
        AttackRange = 120.f;
        DetectionRange = 2000.f;
        ChaseRange = 3500.f;
        WalkSpeed = 300.f;
        RunSpeed = 800.f;
        bIsAggressive = true;
        bIsPredator = true;
        bIsPackHunter = true;
        PackSize = 3;
        GetCapsuleComponent()->InitCapsuleSize(40.f, 80.f);
        break;

    case EDinoSpecies::Triceratops:
        MaxDinoHealth = 350.f;
        DinoHealth = 350.f;
        AttackDamage = 60.f;
        AttackRange = 200.f;
        DetectionRange = 1200.f;
        ChaseRange = 2000.f;
        WalkSpeed = 200.f;
        RunSpeed = 500.f;
        bIsAggressive = false;
        bIsPredator = false;
        bIsPackHunter = false;
        PackSize = 1;
        GetCapsuleComponent()->InitCapsuleSize(80.f, 120.f);
        break;

    case EDinoSpecies::Brachiosaurus:
        MaxDinoHealth = 800.f;
        DinoHealth = 800.f;
        AttackDamage = 80.f;
        AttackRange = 400.f;
        DetectionRange = 1500.f;
        ChaseRange = 1500.f;
        WalkSpeed = 150.f;
        RunSpeed = 350.f;
        bIsAggressive = false;
        bIsPredator = false;
        bIsPackHunter = false;
        PackSize = 1;
        GetCapsuleComponent()->InitCapsuleSize(150.f, 300.f);
        break;

    case EDinoSpecies::Pterodactyl:
        MaxDinoHealth = 80.f;
        DinoHealth = 80.f;
        AttackDamage = 20.f;
        AttackRange = 100.f;
        DetectionRange = 3000.f;
        ChaseRange = 5000.f;
        WalkSpeed = 100.f;
        RunSpeed = 1200.f;
        bIsAggressive = false;
        bIsPredator = true;
        bIsPackHunter = false;
        PackSize = 1;
        GetCapsuleComponent()->InitCapsuleSize(30.f, 60.f);
        break;

    case EDinoSpecies::Stegosaurus:
        MaxDinoHealth = 300.f;
        DinoHealth = 300.f;
        AttackDamage = 50.f;
        AttackRange = 180.f;
        DetectionRange = 1000.f;
        ChaseRange = 1500.f;
        WalkSpeed = 180.f;
        RunSpeed = 400.f;
        bIsAggressive = false;
        bIsPredator = false;
        bIsPackHunter = false;
        PackSize = 1;
        GetCapsuleComponent()->InitCapsuleSize(70.f, 110.f);
        break;

    default:
        break;
    }
}

void ADinosaurBase::SetBehaviorState(EDinoBehaviorState NewState)
{
    if (CurrentBehaviorState == NewState) return;

    CurrentBehaviorState = NewState;
    OnBehaviorStateChanged(NewState);
}

void ADinosaurBase::OnBehaviorStateChanged(EDinoBehaviorState NewState)
{
    // Override in Blueprint or subclass for custom behavior transitions
}

void ADinosaurBase::OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (!Actor) return;

    // If we detect the player character
    if (Actor->ActorHasTag(TEXT("Player")))
    {
        if (Stimulus.WasSuccessfullySensed())
        {
            if (bIsPredator && (bIsAggressive || HungerLevel > 70.f))
            {
                SetBehaviorState(EDinoBehaviorState::Chasing);
            }
            else if (!bIsPredator)
            {
                SetBehaviorState(EDinoBehaviorState::Fleeing);
            }
        }
        else
        {
            // Lost sight/hearing of player
            if (CurrentBehaviorState == EDinoBehaviorState::Chasing)
            {
                SetBehaviorState(EDinoBehaviorState::Patrolling);
            }
        }
    }
}

void ADinosaurBase::DrainHunger()
{
    HungerLevel = FMath::Clamp(HungerLevel + 2.f, 0.f, MaxHunger);

    // Very hungry predators become aggressive
    if (bIsPredator && HungerLevel > 80.f)
    {
        bIsAggressive = true;
    }
}

void ADinosaurBase::RegenStamina()
{
    if (CurrentBehaviorState != EDinoBehaviorState::Chasing &&
        CurrentBehaviorState != EDinoBehaviorState::Attacking)
    {
        DinoStamina = FMath::Clamp(DinoStamina + 5.f, 0.f, MaxDinoStamina);
    }
}

void ADinosaurBase::OnDinosaurDeath()
{
    // Stop timers
    GetWorldTimerManager().ClearTimer(HungerTimerHandle);
    GetWorldTimerManager().ClearTimer(StaminaRegenTimerHandle);

    SetBehaviorState(EDinoBehaviorState::Dead);

    // Disable collision and AI
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    if (GetController())
    {
        GetController()->UnPossess();
    }

    // Destroy after 30 seconds (corpse decay)
    SetLifeSpan(30.f);
}

bool ADinosaurBase::IsAlive() const
{
    return DinoHealth > 0.f;
}

float ADinosaurBase::GetHealthPercent() const
{
    return MaxDinoHealth > 0.f ? DinoHealth / MaxDinoHealth : 0.f;
}

float ADinosaurBase::GetStaminaPercent() const
{
    return MaxDinoStamina > 0.f ? DinoStamina / MaxDinoStamina : 0.f;
}
