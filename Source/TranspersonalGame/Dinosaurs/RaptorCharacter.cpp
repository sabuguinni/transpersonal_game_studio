// RaptorCharacter.cpp
// Core Systems Programmer #03 — Transpersonal Game Studio
// Velociraptor — pack hunter implementation

#include "RaptorCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISense_Hearing.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

ARaptorCharacter::ARaptorCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // --- Capsule — raptors are smaller than T-Rex ---
    GetCapsuleComponent()->SetCapsuleHalfHeight(80.0f);
    GetCapsuleComponent()->SetCapsuleRadius(35.0f);

    // --- Movement — fast and agile ---
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    MoveComp->MaxWalkSpeed = RaptorSprintSpeed;
    MoveComp->MaxAcceleration = 2000.0f;
    MoveComp->BrakingDecelerationWalking = 1200.0f;
    MoveComp->RotationRate = FRotator(0.0f, 360.0f, 0.0f);
    MoveComp->bOrientRotationToMovement = true;
    MoveComp->GravityScale = 1.1f;
    MoveComp->JumpZVelocity = 600.0f;
    MoveComp->AirControl = 0.3f;

    // --- Perception source ---
    PerceptionSource = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>(TEXT("PerceptionSource"));
    PerceptionSource->RegisterForSense(TSubclassOf<UAISense>(UAISense_Sight::StaticClass()));
    PerceptionSource->RegisterForSense(TSubclassOf<UAISense>(UAISense_Hearing::StaticClass()));
    PerceptionSource->bAutoRegister = true;

    // --- Default stats ---
    CurrentHealth = MaxHealth;
    CurrentHunger = MaxHunger * 0.6f;
    bIsAlive = true;
    bIsAggressive = false;  // Raptors are cautious alone, aggressive in packs
    CurrentBehaviorState = ECore_DinoState::Patrolling;

    // --- Species identity ---
    DinosaurSpecies = ECore_DinosaurSpecies::Velociraptor;
    DinosaurName = TEXT("Velociraptor");
    bIsCarnivore = true;
    TerritoryRadius = 2500.0f;
    DetectionRange = 2000.0f;
    AttackRange = 150.0f;

    // --- Pack behaviour defaults ---
    PackRole = ECore_RaptorPackRole::Scout;
    PackID = -1;  // -1 = no pack assigned yet
    bIsPackLeader = false;
    FlankOffset = FVector::ZeroVector;
}

void ARaptorCharacter::BeginPlay()
{
    Super::BeginPlay();

    HomeLocation = GetActorLocation();

    // Hunger drain
    GetWorldTimerManager().SetTimer(
        HungerTimerHandle,
        this,
        &ARaptorCharacter::DrainHunger,
        HungerDrainInterval,
        true
    );

    // Pack coordination pulse (raptors check in with pack)
    GetWorldTimerManager().SetTimer(
        PackCoordTimerHandle,
        this,
        &ARaptorCharacter::CoordinateWithPack,
        PackCoordInterval,
        true
    );
}

void ARaptorCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (!MoveComp) return;

    // Speed varies by state and pack role
    switch (CurrentBehaviorState)
    {
        case ECore_DinoState::Chasing:
            MoveComp->MaxWalkSpeed = RaptorSprintSpeed;
            break;
        case ECore_DinoState::Attacking:
            MoveComp->MaxWalkSpeed = RaptorSprintSpeed * 0.8f;
            break;
        case ECore_DinoState::Patrolling:
            MoveComp->MaxWalkSpeed = RaptorWalkSpeed;
            break;
        case ECore_DinoState::Fleeing:
            MoveComp->MaxWalkSpeed = RaptorSprintSpeed * 1.1f;  // Fear boost
            break;
        default:
            MoveComp->MaxWalkSpeed = RaptorWalkSpeed * 0.4f;
            break;
    }
}

float ARaptorCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    if (!bIsAlive) return 0.0f;

    CurrentHealth = FMath::Clamp(CurrentHealth - ActualDamage, 0.0f, MaxHealth);

    if (CurrentHealth <= 0.0f)
    {
        Die();
    }
    else if (CurrentHealth < MaxHealth * 0.4f)
    {
        // Raptors flee when badly wounded (survival instinct)
        CurrentBehaviorState = ECore_DinoState::Fleeing;
        AlertPackmates(DamageCauser ? DamageCauser->GetActorLocation() : GetActorLocation());
    }

    return ActualDamage;
}

void ARaptorCharacter::Die()
{
    bIsAlive = false;
    CurrentBehaviorState = ECore_DinoState::Dead;

    GetWorldTimerManager().ClearTimer(HungerTimerHandle);
    GetWorldTimerManager().ClearTimer(PackCoordTimerHandle);

    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    GetCharacterMovement()->DisableMovement();

    // Ragdoll death
    GetMesh()->SetSimulatePhysics(true);
    GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

    // If pack leader dies, alert pack
    if (bIsPackLeader)
    {
        AlertPackmates(GetActorLocation());
    }

    UE_LOG(LogTemp, Warning, TEXT("Raptor [%s] Pack:%d Role:%d has died."),
        *GetActorLabel(), PackID, (int32)PackRole);
}

void ARaptorCharacter::DrainHunger()
{
    if (!bIsAlive) return;

    CurrentHunger = FMath::Clamp(CurrentHunger - HungerDrainRate, 0.0f, MaxHunger);

    if (CurrentHunger < MaxHunger * 0.25f)
    {
        bIsAggressive = true;
        if (CurrentBehaviorState == ECore_DinoState::Idle ||
            CurrentBehaviorState == ECore_DinoState::Resting)
        {
            CurrentBehaviorState = ECore_DinoState::Patrolling;
        }
    }
}

void ARaptorCharacter::CoordinateWithPack()
{
    if (!bIsAlive || PackID < 0) return;

    // Find other raptors in the same pack within coordination range
    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ARaptorCharacter::StaticClass(), NearbyActors);

    PackMates.Empty();
    for (AActor* Actor : NearbyActors)
    {
        ARaptorCharacter* OtherRaptor = Cast<ARaptorCharacter>(Actor);
        if (OtherRaptor && OtherRaptor != this &&
            OtherRaptor->PackID == PackID &&
            OtherRaptor->bIsAlive)
        {
            float Dist = FVector::Dist(GetActorLocation(), OtherRaptor->GetActorLocation());
            if (Dist <= PackCoordRange)
            {
                PackMates.Add(OtherRaptor);
            }
        }
    }

    // Pack aggression scales with pack size
    if (PackMates.Num() >= 2)
    {
        bIsAggressive = true;
    }
    else
    {
        // Alone — be cautious
        if (CurrentBehaviorState == ECore_DinoState::Chasing)
        {
            CurrentBehaviorState = ECore_DinoState::Patrolling;
        }
        bIsAggressive = (CurrentHunger < MaxHunger * 0.3f);
    }
}

void ARaptorCharacter::AlertPackmates(const FVector& ThreatLocation)
{
    for (ARaptorCharacter* Mate : PackMates)
    {
        if (Mate && Mate->bIsAlive)
        {
            Mate->ReceivePackAlert(ThreatLocation, this);
        }
    }
}

void ARaptorCharacter::ReceivePackAlert(const FVector& ThreatLocation, ARaptorCharacter* AlertSource)
{
    if (!bIsAlive) return;

    // React based on role
    switch (PackRole)
    {
        case ECore_RaptorPackRole::Alpha:
            // Alpha coordinates the response
            CurrentBehaviorState = ECore_DinoState::Chasing;
            bIsAggressive = true;
            break;
        case ECore_RaptorPackRole::Flanker:
            // Flankers move to encircle
            CurrentBehaviorState = ECore_DinoState::Chasing;
            FlankOffset = FVector(
                FMath::RandRange(-500.0f, 500.0f),
                FMath::RandRange(-500.0f, 500.0f),
                0.0f
            );
            break;
        case ECore_RaptorPackRole::Scout:
            // Scouts alert others and approach cautiously
            CurrentBehaviorState = ECore_DinoState::Patrolling;
            break;
        default:
            CurrentBehaviorState = ECore_DinoState::Chasing;
            break;
    }
}

bool ARaptorCharacter::IsInTerritory(const FVector& Location) const
{
    return FVector::Dist(Location, HomeLocation) <= TerritoryRadius;
}

float ARaptorCharacter::GetHealthPercent() const
{
    return (MaxHealth > 0.0f) ? (CurrentHealth / MaxHealth) : 0.0f;
}

float ARaptorCharacter::GetHungerPercent() const
{
    return (MaxHunger > 0.0f) ? (CurrentHunger / MaxHunger) : 0.0f;
}
