// DinosaurBase.cpp — Base class for all dinosaur pawns
// Transpersonal Game Studio — Engine Architect #02
// Cycle: PROD_CYCLE_AUTO_20260628_010

#include "DinosaurBase.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Engine/World.h"
#include "TimerManager.h"

ADinosaurBase::ADinosaurBase()
{
    PrimaryActorTick.bCanEverTick = true;

    // Default species traits
    DinosaurSpecies = EEng_DinosaurSpecies::Unknown;
    DinosaurDiet = EEng_DinosaurDiet::Omnivore;
    BehaviorState = EEng_DinoBehaviorState::Idle;

    // Vital stats defaults
    MaxHealth = 100.0f;
    CurrentHealth = 100.0f;
    MaxStamina = 100.0f;
    CurrentStamina = 100.0f;
    Hunger = 50.0f;
    MaxHunger = 100.0f;
    Thirst = 50.0f;
    MaxThirst = 100.0f;

    // Movement defaults
    WalkSpeed = 300.0f;
    RunSpeed = 700.0f;
    TurnRate = 90.0f;
    bIsRunning = false;
    bIsAggressive = false;
    bIsPack = false;

    // Perception defaults
    SightRadius = 2000.0f;
    HearingRadius = 1500.0f;
    AttackRange = 200.0f;
    TerritoryRadius = 3000.0f;

    // Combat defaults
    BaseDamage = 25.0f;
    AttackCooldown = 2.0f;
    bCanAttack = true;
    bIsDead = false;

    // AI Perception component
    AIPerception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));

    // Sight config
    UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = SightRadius;
    SightConfig->LoseSightRadius = SightRadius * 1.2f;
    SightConfig->PeripheralVisionAngleDegrees = 90.0f;
    SightConfig->SetMaxAge(5.0f);
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
    AIPerception->ConfigureSense(*SightConfig);

    // Hearing config
    UAISenseConfig_Hearing* HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    HearingConfig->HearingRange = HearingRadius;
    HearingConfig->SetMaxAge(3.0f);
    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
    HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
    AIPerception->ConfigureSense(*HearingConfig);
    AIPerception->SetDominantSense(SightConfig->GetSenseImplementation());

    // Movement component setup
    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
    GetCharacterMovement()->JumpZVelocity = 400.0f;
    GetCharacterMovement()->GravityScale = 1.2f;
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, TurnRate, 0.0f);

    // Capsule defaults
    GetCapsuleComponent()->SetCapsuleHalfHeight(88.0f);
    GetCapsuleComponent()->SetCapsuleRadius(34.0f);

    bUseControllerRotationYaw = false;
    bUseControllerRotationPitch = false;
    bUseControllerRotationRoll = false;
}

void ADinosaurBase::BeginPlay()
{
    Super::BeginPlay();

    CurrentHealth = MaxHealth;
    CurrentStamina = MaxStamina;

    // Start survival tick timers
    GetWorldTimerManager().SetTimer(
        HungerTimerHandle,
        this,
        &ADinosaurBase::TickHunger,
        5.0f,
        true
    );

    GetWorldTimerManager().SetTimer(
        ThirstTimerHandle,
        this,
        &ADinosaurBase::TickThirst,
        7.0f,
        true
    );

    // Bind perception delegate
    if (AIPerception)
    {
        AIPerception->OnTargetPerceptionUpdated.AddDynamic(this, &ADinosaurBase::OnPerceptionUpdated);
    }
}

void ADinosaurBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update stamina recovery when not running
    if (!bIsRunning && CurrentStamina < MaxStamina)
    {
        CurrentStamina = FMath::Min(MaxStamina, CurrentStamina + (DeltaTime * 10.0f));
    }
    else if (bIsRunning && CurrentStamina > 0.0f)
    {
        CurrentStamina = FMath::Max(0.0f, CurrentStamina - (DeltaTime * 15.0f));
        if (CurrentStamina <= 0.0f)
        {
            StopRunning();
        }
    }
}

float ADinosaurBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    if (bIsDead) return 0.0f;

    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
    CurrentHealth = FMath::Max(0.0f, CurrentHealth - ActualDamage);

    OnDamageTaken(ActualDamage, DamageCauser);

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
    BehaviorState = EEng_DinoBehaviorState::Dead;

    GetCharacterMovement()->DisableMovement();
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    OnDeath();

    // Destroy after 30 seconds (corpse decay)
    GetWorldTimerManager().SetTimer(
        DeathTimerHandle,
        this,
        &ADinosaurBase::DestroyCorpse,
        30.0f,
        false
    );
}

void ADinosaurBase::DestroyCorpse()
{
    Destroy();
}

void ADinosaurBase::StartRunning()
{
    if (bIsDead || CurrentStamina <= 10.0f) return;
    bIsRunning = true;
    GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
}

void ADinosaurBase::StopRunning()
{
    bIsRunning = false;
    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

bool ADinosaurBase::CanAttack() const
{
    return bCanAttack && !bIsDead && CurrentHealth > 0.0f;
}

void ADinosaurBase::PerformAttack(AActor* Target)
{
    if (!CanAttack() || !Target) return;

    bCanAttack = false;

    // Apply damage to target
    UGameplayStatics::ApplyDamage(Target, BaseDamage, GetController(), this, UDamageType::StaticClass());

    OnAttackPerformed(Target);

    // Reset attack cooldown
    GetWorldTimerManager().SetTimer(
        AttackCooldownHandle,
        this,
        &ADinosaurBase::ResetAttackCooldown,
        AttackCooldown,
        false
    );
}

void ADinosaurBase::ResetAttackCooldown()
{
    bCanAttack = true;
}

void ADinosaurBase::TickHunger()
{
    Hunger = FMath::Max(0.0f, Hunger - 2.0f);
    if (Hunger <= 10.0f)
    {
        // Very hungry — increase aggression
        bIsAggressive = true;
    }
}

void ADinosaurBase::TickThirst()
{
    Thirst = FMath::Max(0.0f, Thirst - 1.5f);
}

void ADinosaurBase::OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (!Actor || bIsDead) return;

    // Base implementation — subclasses override for species-specific behavior
    if (Stimulus.WasSuccessfullySensed())
    {
        // Check if it's a player or threat
        ACharacter* AsCharacter = Cast<ACharacter>(Actor);
        if (AsCharacter && bIsAggressive)
        {
            BehaviorState = EEng_DinoBehaviorState::Attacking;
        }
        else if (AsCharacter)
        {
            BehaviorState = EEng_DinoBehaviorState::Alert;
        }
    }
    else
    {
        // Lost perception — return to patrol
        if (BehaviorState == EEng_DinoBehaviorState::Alert)
        {
            BehaviorState = EEng_DinoBehaviorState::Patrolling;
        }
    }
}

void ADinosaurBase::OnDamageTaken_Implementation(float DamageAmount, AActor* DamageCauser)
{
    // Base: become aggressive when hurt
    if (DamageAmount > 0.0f)
    {
        bIsAggressive = true;
        BehaviorState = EEng_DinoBehaviorState::Attacking;
    }
}

void ADinosaurBase::OnDeath_Implementation()
{
    // Base death behavior — subclasses override
}

void ADinosaurBase::OnAttackPerformed_Implementation(AActor* Target)
{
    // Base attack behavior — subclasses override
}
