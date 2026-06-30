// DinosaurBase.cpp
// Engine Architect #02 — Cycle AUTO_20260630_003
// Base class for all dinosaur pawns in the prehistoric survival game

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

    // Default species data
    SpeciesName = FName("UnknownDinosaur");
    MaxHealth = 500.0f;
    CurrentHealth = 500.0f;
    MaxSpeed = 600.0f;
    AttackDamage = 50.0f;
    DetectionRadius = 1500.0f;
    bIsHerbivore = false;
    bIsPredator = true;
    bIsInPack = false;
    PackSize = 1;
    TerritoryRadius = 3000.0f;
    CurrentBehaviorState = EDinosaurBehavior::Idle;

    // Configure capsule
    GetCapsuleComponent()->InitCapsuleSize(80.0f, 120.0f);

    // Configure movement
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        MoveComp->MaxWalkSpeed = MaxSpeed;
        MoveComp->bOrientRotationToMovement = true;
        MoveComp->RotationRate = FRotator(0.0f, 360.0f, 0.0f);
        MoveComp->JumpZVelocity = 400.0f;
        MoveComp->AirControl = 0.1f;
        MoveComp->GravityScale = 1.5f;
    }

    // AI Perception stimuli source
    AIPerceptionStimuliSource = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>(
        TEXT("AIPerceptionStimuliSource"));
    if (AIPerceptionStimuliSource)
    {
        AIPerceptionStimuliSource->RegisterForSense(TSubclassOf<UAISense>(UAISense_Sight::StaticClass()));
        AIPerceptionStimuliSource->RegisterForSense(TSubclassOf<UAISense>(UAISense_Hearing::StaticClass()));
        AIPerceptionStimuliSource->bAutoRegister = true;
    }

    // Don't use controller rotation — let movement component handle it
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;
}

void ADinosaurBase::BeginPlay()
{
    Super::BeginPlay();

    CurrentHealth = MaxHealth;
    HomeLocation = GetActorLocation();

    UE_LOG(LogTemp, Log, TEXT("DinosaurBase: %s spawned at %s"),
        *SpeciesName.ToString(),
        *GetActorLocation().ToString());
}

void ADinosaurBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Basic behavior tick — AI controller handles detailed logic
    UpdateBehaviorState(DeltaTime);
}

void ADinosaurBase::UpdateBehaviorState(float DeltaTime)
{
    // Stub: AI controller (BehaviorTree) overrides this in derived classes
    // Base implementation: return to home if wandered too far
    float DistFromHome = FVector::Dist(GetActorLocation(), HomeLocation);
    if (DistFromHome > TerritoryRadius && CurrentBehaviorState != EDinosaurBehavior::Fleeing)
    {
        CurrentBehaviorState = EDinosaurBehavior::Patrolling;
    }
}

float ADinosaurBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    CurrentHealth = FMath::Clamp(CurrentHealth - ActualDamage, 0.0f, MaxHealth);

    UE_LOG(LogTemp, Log, TEXT("DinosaurBase: %s took %.1f damage, health: %.1f/%.1f"),
        *SpeciesName.ToString(), ActualDamage, CurrentHealth, MaxHealth);

    if (CurrentHealth <= 0.0f)
    {
        OnDeath();
    }
    else if (CurrentHealth < MaxHealth * 0.3f)
    {
        // Low health — flee behavior
        CurrentBehaviorState = EDinosaurBehavior::Fleeing;
    }

    return ActualDamage;
}

void ADinosaurBase::OnDeath()
{
    CurrentBehaviorState = EDinosaurBehavior::Dead;
    UE_LOG(LogTemp, Log, TEXT("DinosaurBase: %s has died"), *SpeciesName.ToString());

    // Disable collision and movement
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        MoveComp->DisableMovement();
    }

    // Destroy after delay (allow death animation to play)
    SetLifeSpan(10.0f);
}

bool ADinosaurBase::IsAlive() const
{
    return CurrentHealth > 0.0f && CurrentBehaviorState != EDinosaurBehavior::Dead;
}

void ADinosaurBase::SetBehaviorState(EDinosaurBehavior NewState)
{
    if (CurrentBehaviorState != NewState)
    {
        CurrentBehaviorState = NewState;
        UE_LOG(LogTemp, Verbose, TEXT("DinosaurBase: %s behavior -> %d"),
            *SpeciesName.ToString(), (int32)NewState);
    }
}

FVector ADinosaurBase::GetHomeLocation() const
{
    return HomeLocation;
}

void ADinosaurBase::SetHomeLocation(FVector NewHome)
{
    HomeLocation = NewHome;
}
