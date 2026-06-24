// TRexCharacter.cpp
// Core Systems Programmer #03 — Transpersonal Game Studio
// Tyrannosaurus Rex — apex predator implementation

#include "TRexCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISense_Hearing.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"

ATRexCharacter::ATRexCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // --- Capsule ---
    GetCapsuleComponent()->SetCapsuleHalfHeight(200.0f);
    GetCapsuleComponent()->SetCapsuleRadius(80.0f);

    // --- Movement ---
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    MoveComp->MaxWalkSpeed = TRexWalkSpeed;
    MoveComp->MaxAcceleration = 800.0f;
    MoveComp->BrakingDecelerationWalking = 600.0f;
    MoveComp->RotationRate = FRotator(0.0f, 180.0f, 0.0f);
    MoveComp->bOrientRotationToMovement = true;
    MoveComp->GravityScale = 1.2f;

    // --- Perception source (AI sees/hears this actor) ---
    PerceptionSource = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>(TEXT("PerceptionSource"));
    PerceptionSource->RegisterForSense(TSubclassOf<UAISense>(UAISense_Sight::StaticClass()));
    PerceptionSource->RegisterForSense(TSubclassOf<UAISense>(UAISense_Hearing::StaticClass()));
    PerceptionSource->bAutoRegister = true;

    // --- Default stats ---
    CurrentHealth = MaxHealth;
    CurrentHunger = MaxHunger * 0.5f;
    bIsAlive = true;
    bIsAggressive = true;
    CurrentBehaviorState = ECore_DinoState::Idle;

    // --- Species identity ---
    DinosaurSpecies = ECore_DinosaurSpecies::TyrannosaurusRex;
    DinosaurName = TEXT("Tyrannosaurus Rex");
    bIsCarnivore = true;
    TerritoryRadius = 5000.0f;
    DetectionRange = 3000.0f;
    AttackRange = 350.0f;
}

void ATRexCharacter::BeginPlay()
{
    Super::BeginPlay();

    // Cache home location for territory patrol
    HomeLocation = GetActorLocation();

    // Start hunger drain timer
    GetWorldTimerManager().SetTimer(
        HungerTimerHandle,
        this,
        &ATRexCharacter::DrainHunger,
        HungerDrainInterval,
        true
    );

    // Start roar timer (ambient behaviour)
    GetWorldTimerManager().SetTimer(
        RoarTimerHandle,
        this,
        &ATRexCharacter::PlayRoar,
        FMath::RandRange(30.0f, 90.0f),
        false
    );
}

void ATRexCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update movement speed based on state
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        switch (CurrentBehaviorState)
        {
            case ECore_DinoState::Chasing:
            case ECore_DinoState::Attacking:
                MoveComp->MaxWalkSpeed = TRexChargeSpeed;
                break;
            case ECore_DinoState::Patrolling:
                MoveComp->MaxWalkSpeed = TRexWalkSpeed;
                break;
            default:
                MoveComp->MaxWalkSpeed = TRexWalkSpeed * 0.5f;
                break;
        }
    }
}

float ATRexCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    if (!bIsAlive) return 0.0f;

    CurrentHealth = FMath::Clamp(CurrentHealth - ActualDamage, 0.0f, MaxHealth);

    if (CurrentHealth <= 0.0f)
    {
        Die();
    }
    else if (CurrentHealth < MaxHealth * 0.3f)
    {
        // Wounded — become more aggressive
        bIsAggressive = true;
        CurrentBehaviorState = ECore_DinoState::Fleeing;
    }

    return ActualDamage;
}

void ATRexCharacter::Die()
{
    bIsAlive = false;
    CurrentBehaviorState = ECore_DinoState::Dead;

    // Stop timers
    GetWorldTimerManager().ClearTimer(HungerTimerHandle);
    GetWorldTimerManager().ClearTimer(RoarTimerHandle);

    // Disable collision
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    GetCharacterMovement()->DisableMovement();

    // Ragdoll
    GetMesh()->SetSimulatePhysics(true);
    GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

    UE_LOG(LogTemp, Warning, TEXT("TRex [%s] has died."), *GetActorLabel());
}

void ATRexCharacter::DrainHunger()
{
    if (!bIsAlive) return;

    CurrentHunger = FMath::Clamp(CurrentHunger - HungerDrainRate, 0.0f, MaxHunger);

    if (CurrentHunger < MaxHunger * 0.2f)
    {
        // Starving — actively hunt
        bIsAggressive = true;
        if (CurrentBehaviorState == ECore_DinoState::Idle ||
            CurrentBehaviorState == ECore_DinoState::Resting)
        {
            CurrentBehaviorState = ECore_DinoState::Patrolling;
        }
    }
}

void ATRexCharacter::PlayRoar()
{
    if (!bIsAlive) return;

    UE_LOG(LogTemp, Log, TEXT("TRex [%s] ROARS at location %s"),
        *GetActorLabel(), *GetActorLocation().ToString());

    // Schedule next roar
    GetWorldTimerManager().SetTimer(
        RoarTimerHandle,
        this,
        &ATRexCharacter::PlayRoar,
        FMath::RandRange(45.0f, 120.0f),
        false
    );
}

bool ATRexCharacter::IsInTerritory(const FVector& Location) const
{
    return FVector::Dist(Location, HomeLocation) <= TerritoryRadius;
}

float ATRexCharacter::GetHealthPercent() const
{
    return (MaxHealth > 0.0f) ? (CurrentHealth / MaxHealth) : 0.0f;
}

float ATRexCharacter::GetHungerPercent() const
{
    return (MaxHunger > 0.0f) ? (CurrentHunger / MaxHunger) : 0.0f;
}
