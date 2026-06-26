// DinosaurBase.cpp
// Engine Architect #02 — Transpersonal Game Studio
// Base class for all dinosaur species in the prehistoric survival game.
// Provides skeletal mesh setup, survival AI stats, territory, and damage handling.

#include "DinosaurBase.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"

ADinosaurBase::ADinosaurBase()
{
    PrimaryActorTick.bCanEverTick = true;

    // Capsule collision — sized for medium dinosaur, subclasses override
    GetCapsuleComponent()->InitCapsuleSize(80.0f, 160.0f);
    GetCapsuleComponent()->SetCollisionProfileName(TEXT("Pawn"));

    // Skeletal mesh — no default mesh, subclasses assign species mesh
    GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -160.0f));
    GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));

    // Detection sphere — 1200 units default, subclasses tune per species
    DetectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("DetectionSphere"));
    DetectionSphere->SetupAttachment(RootComponent);
    DetectionSphere->SetSphereRadius(1200.0f);
    DetectionSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
    DetectionSphere->OnComponentBeginOverlap.AddDynamic(this, &ADinosaurBase::OnDetectionBeginOverlap);
    DetectionSphere->OnComponentEndOverlap.AddDynamic(this, &ADinosaurBase::OnDetectionEndOverlap);

    // Attack sphere — melee range, 200 units default
    AttackSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AttackSphere"));
    AttackSphere->SetupAttachment(RootComponent);
    AttackSphere->SetSphereRadius(200.0f);
    AttackSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));

    // Movement defaults — subclasses override for species-specific speed
    GetCharacterMovement()->MaxWalkSpeed = 400.0f;
    GetCharacterMovement()->MaxWalkSpeedCrouched = 200.0f;
    GetCharacterMovement()->JumpZVelocity = 400.0f;
    GetCharacterMovement()->GravityScale = 1.0f;
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 360.0f, 0.0f);

    bUseControllerRotationYaw = false;

    // Default species stats — subclasses override in their constructors
    SpeciesName = TEXT("Unknown Dinosaur");
    DinoClass = EDinoClass::Herbivore;
    MaxHealth = 300.0f;
    CurrentHealth = 300.0f;
    MaxStamina = 100.0f;
    CurrentStamina = 100.0f;
    AttackDamage = 25.0f;
    AttackCooldown = 1.5f;
    DetectionRange = 1200.0f;
    TerritoryRadius = 3000.0f;
    bIsAlerted = false;
    bIsAttacking = false;
    bIsFleeing = false;
    bIsDead = false;
    CurrentBehaviorState = EDinoBehaviorState::Idle;
    HungerLevel = 50.0f;
    ThirstLevel = 50.0f;
    FearLevel = 0.0f;
    PackSize = 1;
    bIsPackLeader = false;
}

void ADinosaurBase::BeginPlay()
{
    Super::BeginPlay();

    // Record home territory center
    TerritoryCenter = GetActorLocation();

    // Start metabolism tick — hunger/thirst drain over time
    GetWorldTimerManager().SetTimer(
        MetabolismTimerHandle,
        this,
        &ADinosaurBase::TickMetabolism,
        5.0f,   // every 5 seconds
        true    // looping
    );

    // Start behavior update tick
    GetWorldTimerManager().SetTimer(
        BehaviorTimerHandle,
        this,
        &ADinosaurBase::UpdateBehaviorState,
        0.5f,   // every 0.5 seconds
        true
    );
}

void ADinosaurBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Stamina recovery when not sprinting
    if (CurrentStamina < MaxStamina && !bIsAttacking)
    {
        CurrentStamina = FMath::Min(MaxStamina, CurrentStamina + DeltaTime * 10.0f);
    }

    // Fear decay over time
    if (FearLevel > 0.0f)
    {
        FearLevel = FMath::Max(0.0f, FearLevel - DeltaTime * 2.0f);
    }
}

float ADinosaurBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    if (bIsDead) return 0.0f;

    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
    CurrentHealth = FMath::Max(0.0f, CurrentHealth - ActualDamage);

    // Increase fear when hit
    FearLevel = FMath::Min(100.0f, FearLevel + ActualDamage * 0.5f);

    // Alert the dinosaur
    if (!bIsAlerted)
    {
        bIsAlerted = true;
        OnAlerted(DamageCauser);
    }

    if (CurrentHealth <= 0.0f)
    {
        Die();
    }

    return ActualDamage;
}

void ADinosaurBase::OnDetectionBeginOverlap(UPrimitiveComponent* OverlappedComp,
    AActor* OtherActor, UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor || OtherActor == this) return;

    // Check if it's the player character
    APawn* OtherPawn = Cast<APawn>(OtherActor);
    if (OtherPawn && OtherPawn->IsPlayerControlled())
    {
        NearbyThreats.AddUnique(OtherActor);

        // Carnivores alert immediately; herbivores alert based on fear threshold
        if (DinoClass == EDinoClass::Carnivore || DinoClass == EDinoClass::Apex)
        {
            bIsAlerted = true;
            OnAlerted(OtherActor);
        }
        else if (FearLevel > 30.0f)
        {
            // Herbivore flees when already fearful
            bIsFleeing = true;
            CurrentBehaviorState = EDinoBehaviorState::Fleeing;
        }
    }
}

void ADinosaurBase::OnDetectionEndOverlap(UPrimitiveComponent* OverlappedComp,
    AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    NearbyThreats.Remove(OtherActor);

    if (NearbyThreats.Num() == 0)
    {
        bIsAlerted = false;
        bIsFleeing = false;
        // Return to patrol after a delay
        GetWorldTimerManager().SetTimer(
            BehaviorTimerHandle,
            this,
            &ADinosaurBase::UpdateBehaviorState,
            3.0f,
            false
        );
    }
}

void ADinosaurBase::OnAlerted(AActor* Threat)
{
    bIsAlerted = true;
    CurrentBehaviorState = (DinoClass == EDinoClass::Carnivore || DinoClass == EDinoClass::Apex)
        ? EDinoBehaviorState::Hunting
        : EDinoBehaviorState::Fleeing;
}

void ADinosaurBase::UpdateBehaviorState()
{
    if (bIsDead) return;

    if (NearbyThreats.Num() == 0 && !bIsAlerted)
    {
        // Idle or patrol based on hunger
        if (HungerLevel < 30.0f)
        {
            CurrentBehaviorState = EDinoBehaviorState::Foraging;
        }
        else if (ThirstLevel < 30.0f)
        {
            CurrentBehaviorState = EDinoBehaviorState::Migrating; // seek water
        }
        else
        {
            CurrentBehaviorState = EDinoBehaviorState::Patrolling;
        }
    }
}

void ADinosaurBase::TickMetabolism()
{
    if (bIsDead) return;

    // Drain hunger and thirst slowly
    HungerLevel = FMath::Max(0.0f, HungerLevel - 2.0f);
    ThirstLevel = FMath::Max(0.0f, ThirstLevel - 1.5f);

    // Starvation damage
    if (HungerLevel <= 0.0f)
    {
        CurrentHealth = FMath::Max(0.0f, CurrentHealth - 5.0f);
        if (CurrentHealth <= 0.0f)
        {
            Die();
        }
    }
}

void ADinosaurBase::Die()
{
    if (bIsDead) return;
    bIsDead = true;
    CurrentBehaviorState = EDinoBehaviorState::Idle;

    // Stop all timers
    GetWorldTimerManager().ClearTimer(MetabolismTimerHandle);
    GetWorldTimerManager().ClearTimer(BehaviorTimerHandle);

    // Disable collision
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    DetectionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    AttackSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Disable movement
    GetCharacterMovement()->DisableMovement();

    // Play death animation (Blueprint handles the actual anim montage)
    OnDeath();
}

void ADinosaurBase::OnDeath()
{
    // Override in Blueprint or subclass to play death animation, spawn loot, etc.
    // Default: ragdoll the mesh
    GetMesh()->SetSimulatePhysics(true);
    GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

    // Destroy actor after 30 seconds (cleanup)
    SetLifeSpan(30.0f);
}

bool ADinosaurBase::IsWithinTerritory(const FVector& Location) const
{
    return FVector::Dist(Location, TerritoryCenter) <= TerritoryRadius;
}

float ADinosaurBase::GetHealthPercent() const
{
    return (MaxHealth > 0.0f) ? (CurrentHealth / MaxHealth) : 0.0f;
}

float ADinosaurBase::GetStaminaPercent() const
{
    return (MaxStamina > 0.0f) ? (CurrentStamina / MaxStamina) : 0.0f;
}
