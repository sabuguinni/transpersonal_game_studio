// DinosaurBase.cpp
// Transpersonal Game Studio — Agent #02 Engine Architect
// Base implementation for all dinosaur types in the prehistoric survival game.
// All dinosaur species inherit from this class.

#include "DinosaurBase.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISense_Hearing.h"

ADinosaurBase::ADinosaurBase()
{
    PrimaryActorTick.bCanEverTick = true;

    // Configure capsule for dinosaur collision
    GetCapsuleComponent()->InitCapsuleSize(88.0f, 96.0f);
    GetCapsuleComponent()->SetCollisionProfileName(TEXT("Pawn"));

    // Skeletal mesh setup — subclasses set the actual mesh
    GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -96.0f));
    GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));

    // Movement defaults — overridden per species
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
    GetCharacterMovement()->MaxWalkSpeed = 600.0f;
    GetCharacterMovement()->JumpZVelocity = 400.0f;
    GetCharacterMovement()->AirControl = 0.2f;
    GetCharacterMovement()->GravityScale = 1.0f;
    GetCharacterMovement()->NavAgentProps.bCanCrouch = false;

    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    // AI Perception stimuli source
    PerceptionStimuliSource = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>(TEXT("PerceptionStimuliSource"));
    PerceptionStimuliSource->RegisterForSense(TSubclassOf<UAISense_Sight>());
    PerceptionStimuliSource->RegisterForSense(TSubclassOf<UAISense_Hearing>());
    PerceptionStimuliSource->bAutoRegister = true;

    // Default survival stats
    CurrentHealth = 1000.0f;
    MaxHealth = 1000.0f;
    AttackDamage = 50.0f;
    AttackRange = 200.0f;
    DetectionRadius = 2000.0f;
    bIsAggressive = false;
    bIsPredator = false;
    CurrentBehaviorState = EDinosaurBehaviorState::Idle;
    DinosaurSpecies = EDinosaurSpecies::Unknown;
}

void ADinosaurBase::BeginPlay()
{
    Super::BeginPlay();

    // Initialize health to max on spawn
    CurrentHealth = MaxHealth;

    // Log species on spawn for debugging
    UE_LOG(LogTemp, Log, TEXT("DinosaurBase::BeginPlay — Species:%d Health:%.0f Aggressive:%s"),
        (int32)DinosaurSpecies, CurrentHealth, bIsAggressive ? TEXT("true") : TEXT("false"));
}

void ADinosaurBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    // Behavior tick is handled by the AI Controller (Agent #12)
    // This base tick is intentionally minimal
}

float ADinosaurBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    CurrentHealth = FMath::Clamp(CurrentHealth - ActualDamage, 0.0f, MaxHealth);

    if (CurrentHealth <= 0.0f)
    {
        OnDinosaurDeath();
    }
    else
    {
        // Trigger aggression on damage if not already aggressive
        if (!bIsAggressive && bIsPredator)
        {
            bIsAggressive = true;
            CurrentBehaviorState = EDinosaurBehaviorState::Attacking;
        }
    }

    return ActualDamage;
}

void ADinosaurBase::OnDinosaurDeath()
{
    CurrentBehaviorState = EDinosaurBehaviorState::Dead;

    // Disable collision on death
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    GetCharacterMovement()->DisableMovement();

    // Notify AI controller
    if (AController* Ctrl = GetController())
    {
        Ctrl->UnPossess();
    }

    UE_LOG(LogTemp, Log, TEXT("DinosaurBase::OnDinosaurDeath — %s has died"), *GetActorLabel());

    // Ragdoll on death — enable physics simulation
    GetMesh()->SetSimulatePhysics(true);
    GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
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
    if (CurrentBehaviorState != NewState)
    {
        CurrentBehaviorState = NewState;
        OnBehaviorStateChanged(NewState);
    }
}

void ADinosaurBase::OnBehaviorStateChanged(EDinosaurBehaviorState NewState)
{
    // Override in subclasses for species-specific behavior responses
    UE_LOG(LogTemp, Verbose, TEXT("DinosaurBase::OnBehaviorStateChanged — %s NewState:%d"),
        *GetActorLabel(), (int32)NewState);
}
