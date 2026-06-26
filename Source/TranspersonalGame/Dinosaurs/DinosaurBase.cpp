// DinosaurBase.cpp — Engine Architect #02 — Cycle AUTO_009
// Base class for all dinosaur actors in TranspersonalGame.
// Provides survival stats, behavior state machine, detection range,
// and aggression logic. All species-specific actors inherit from this.

#include "DinosaurBase.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

ADinosaurBase::ADinosaurBase()
{
    PrimaryActorTick.bCanEverTick = true;

    // Capsule defaults — overridden per species in child classes
    GetCapsuleComponent()->InitCapsuleSize(88.0f, 96.0f);

    // Skeletal mesh — asset assigned per species in child BP/class
    GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -96.0f));
    GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));

    // Movement defaults
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 360.0f, 0.0f);
    GetCharacterMovement()->MaxWalkSpeed = 400.0f;
    GetCharacterMovement()->JumpZVelocity = 0.0f;  // Dinosaurs don't jump by default

    // Dinosaur data defaults
    DinoData.Species = EDinosaurSpecies::Compsognathus;
    DinoData.BehaviorState = EDinosaurBehaviorState::Idle;
    DinoData.AggressionLevel = 50.0f;
    DinoData.DetectionRange = 800.0f;
    DinoData.bIsPackHunter = false;

    // Survival stats
    MaxHealth = 100.0f;
    CurrentHealth = 100.0f;
    bIsAlive = true;

    // Behavior tick interval (seconds between AI updates)
    BehaviorTickInterval = 0.5f;
    TimeSinceLastBehaviorTick = 0.0f;

    // Detection
    DetectionRange = 800.0f;
    AttackRange = 150.0f;
    AggressionLevel = 50.0f;
}

void ADinosaurBase::BeginPlay()
{
    Super::BeginPlay();
    CurrentHealth = MaxHealth;
    bIsAlive = true;
    DinoData.BehaviorState = EDinosaurBehaviorState::Idle;
}

void ADinosaurBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bIsAlive) return;

    TimeSinceLastBehaviorTick += DeltaTime;
    if (TimeSinceLastBehaviorTick >= BehaviorTickInterval)
    {
        TimeSinceLastBehaviorTick = 0.0f;
        UpdateBehaviorState();
    }
}

void ADinosaurBase::UpdateBehaviorState()
{
    // Simple state machine — override in species-specific subclasses
    switch (DinoData.BehaviorState)
    {
        case EDinosaurBehaviorState::Idle:
            OnIdle();
            break;
        case EDinosaurBehaviorState::Patrol:
            OnPatrol();
            break;
        case EDinosaurBehaviorState::Hunt:
            OnHunt();
            break;
        case EDinosaurBehaviorState::Flee:
            OnFlee();
            break;
        case EDinosaurBehaviorState::Feed:
            OnFeed();
            break;
        case EDinosaurBehaviorState::Aggressive:
            OnAggressive();
            break;
        default:
            break;
    }
}

void ADinosaurBase::OnIdle()
{
    // Base idle — do nothing; child classes can add idle animations
}

void ADinosaurBase::OnPatrol()
{
    // Base patrol — child classes implement waypoint movement
}

void ADinosaurBase::OnHunt()
{
    // Base hunt — child classes implement target pursuit
}

void ADinosaurBase::OnFlee()
{
    // Base flee — child classes implement escape movement
}

void ADinosaurBase::OnFeed()
{
    // Base feed — child classes implement feeding animation
}

void ADinosaurBase::OnAggressive()
{
    // Base aggressive — child classes implement attack logic
}

float ADinosaurBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    if (!bIsAlive) return 0.0f;

    CurrentHealth = FMath::Clamp(CurrentHealth - ActualDamage, 0.0f, MaxHealth);

    if (CurrentHealth <= 0.0f)
    {
        bIsAlive = false;
        OnDeath();
    }
    else if (ActualDamage > 0.0f && DinoData.AggressionLevel > 30.0f)
    {
        // Become aggressive when hit
        DinoData.BehaviorState = EDinosaurBehaviorState::Aggressive;
    }

    return ActualDamage;
}

void ADinosaurBase::OnDeath()
{
    // Disable collision and movement
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    GetCharacterMovement()->DisableMovement();

    // Ragdoll
    GetMesh()->SetSimulatePhysics(true);
    GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

    // Destroy after 10 seconds
    SetLifeSpan(10.0f);
}

void ADinosaurBase::SetBehaviorState(EDinosaurBehaviorState NewState)
{
    DinoData.BehaviorState = NewState;
}

EDinosaurBehaviorState ADinosaurBase::GetBehaviorState() const
{
    return DinoData.BehaviorState;
}

bool ADinosaurBase::IsPlayerInDetectionRange() const
{
    UWorld* World = GetWorld();
    if (!World) return false;

    APawn* Player = UGameplayStatics::GetPlayerPawn(World, 0);
    if (!Player) return false;

    float Distance = FVector::Dist(GetActorLocation(), Player->GetActorLocation());
    return Distance <= DetectionRange;
}

float ADinosaurBase::GetDistanceToPlayer() const
{
    UWorld* World = GetWorld();
    if (!World) return TNumericLimits<float>::Max();

    APawn* Player = UGameplayStatics::GetPlayerPawn(World, 0);
    if (!Player) return TNumericLimits<float>::Max();

    return FVector::Dist(GetActorLocation(), Player->GetActorLocation());
}
