// DinosaurBase.cpp
// Core Systems Programmer — Agent #3
// Full implementation of ADinosaurBase — health, territory, aggro state machine, attack system.

#include "DinosaurBase.h"
#include "Components/SphereComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "GameFramework/DamageType.h"

ADinosaurBase::ADinosaurBase()
{
    PrimaryActorTick.bCanEverTick = true;

    // --- Territory Volume ---
    TerritoryVolume = CreateDefaultSubobject<USphereComponent>(TEXT("TerritoryVolume"));
    TerritoryVolume->SetupAttachment(RootComponent);
    TerritoryVolume->SetSphereRadius(1500.0f);
    TerritoryVolume->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
    TerritoryVolume->SetGenerateOverlapEvents(false); // Territory is checked by distance, not overlap

    // --- Aggro Volume ---
    AggroVolume = CreateDefaultSubobject<USphereComponent>(TEXT("AggroVolume"));
    AggroVolume->SetupAttachment(RootComponent);
    AggroVolume->SetSphereRadius(800.0f);
    AggroVolume->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
    AggroVolume->SetGenerateOverlapEvents(true);

    // Bind aggro overlap events
    AggroVolume->OnComponentBeginOverlap.AddDynamic(this, &ADinosaurBase::OnAggroVolumeBeginOverlap);
    AggroVolume->OnComponentEndOverlap.AddDynamic(this, &ADinosaurBase::OnAggroVolumeEndOverlap);

    // --- Default State ---
    CurrentHealth = 100.0f;
    AggroState = ECore_DinoAggroState::Idle;
    CurrentTarget = nullptr;
    bAttackOnCooldown = false;

    // --- Movement ---
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed = DinoStats.WalkSpeed;
        GetCharacterMovement()->bOrientRotationToMovement = true;
        GetCharacterMovement()->RotationRate = FRotator(0.0f, 360.0f, 0.0f);
    }

    // Dinosaurs don't use controller rotation for mesh orientation
    bUseControllerRotationYaw = false;
    bUseControllerRotationPitch = false;
    bUseControllerRotationRoll = false;
}

void ADinosaurBase::BeginPlay()
{
    Super::BeginPlay();

    // Record home territory center from spawn location
    TerritoryCenter = GetActorLocation();

    // Initialise health from stats
    CurrentHealth = DinoStats.MaxHealth;

    // Sync sphere radii with stats
    if (TerritoryVolume)
    {
        TerritoryVolume->SetSphereRadius(DinoStats.TerritoryRadius);
    }
    if (AggroVolume)
    {
        AggroVolume->SetSphereRadius(DinoStats.AggroRadius);
    }

    // Sync movement speed
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed = DinoStats.WalkSpeed;
    }

    SetAggroState(ECore_DinoAggroState::Patrolling);
}

void ADinosaurBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // If chasing, check if target is still alive and in range
    if (AggroState == ECore_DinoAggroState::Chasing && CurrentTarget)
    {
        float DistToTarget = FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation());

        // If target is within attack range, switch to attacking
        if (DistToTarget <= DinoStats.AttackRange)
        {
            SetAggroState(ECore_DinoAggroState::Attacking);
            PerformAttack();
        }
        // If target has fled beyond 2× aggro radius, give up chase
        else if (DistToTarget > DinoStats.AggroRadius * 2.0f)
        {
            CurrentTarget = nullptr;
            SetAggroState(ECore_DinoAggroState::Patrolling);
        }
    }

    // If attacking but target moved out of range, resume chase
    if (AggroState == ECore_DinoAggroState::Attacking && CurrentTarget)
    {
        float DistToTarget = FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation());
        if (DistToTarget > DinoStats.AttackRange * 1.5f)
        {
            SetAggroState(ECore_DinoAggroState::Chasing);
        }
    }
}

// --- Health System ---

float ADinosaurBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
                                 AController* EventInstigator, AActor* DamageCauser)
{
    if (!IsAlive()) return 0.0f;

    const float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
    if (ActualDamage <= 0.0f) return 0.0f;

    CurrentHealth = FMath::Max(0.0f, CurrentHealth - ActualDamage);

    // Aggro on attacker
    if (DamageCauser && AggroState != ECore_DinoAggroState::Dead)
    {
        CurrentTarget = DamageCauser;
        SetAggroState(ECore_DinoAggroState::Chasing);
    }

    if (CurrentHealth <= 0.0f)
    {
        OnDeath(DamageCauser);
    }

    return ActualDamage;
}

void ADinosaurBase::HealDinosaur(float HealAmount)
{
    if (!IsAlive()) return;
    CurrentHealth = FMath::Min(DinoStats.MaxHealth, CurrentHealth + HealAmount);
}

bool ADinosaurBase::IsAlive() const
{
    return AggroState != ECore_DinoAggroState::Dead && CurrentHealth > 0.0f;
}

// --- Species ---

FString ADinosaurBase::GetSpeciesName() const
{
    return TEXT("Unknown Dinosaur");
}

// --- Aggro State Machine ---

void ADinosaurBase::SetAggroState(ECore_DinoAggroState NewState)
{
    if (AggroState == NewState) return;

    ECore_DinoAggroState OldState = AggroState;
    AggroState = NewState;

    // Adjust movement speed based on state
    if (GetCharacterMovement())
    {
        switch (NewState)
        {
            case ECore_DinoAggroState::Chasing:
            case ECore_DinoAggroState::Attacking:
                GetCharacterMovement()->MaxWalkSpeed = DinoStats.RunSpeed;
                break;
            case ECore_DinoAggroState::Fleeing:
                GetCharacterMovement()->MaxWalkSpeed = DinoStats.RunSpeed * 1.2f;
                break;
            default:
                GetCharacterMovement()->MaxWalkSpeed = DinoStats.WalkSpeed;
                break;
        }
    }

    OnAggroStateChanged(OldState, NewState);
}

// --- Attack System ---

void ADinosaurBase::PerformAttack()
{
    if (!CanAttack() || !CurrentTarget || !IsAlive()) return;

    // Apply damage to target
    UGameplayStatics::ApplyDamage(CurrentTarget, DinoStats.AttackDamage,
                                   GetController(), this, UDamageType::StaticClass());

    // Start cooldown
    bAttackOnCooldown = true;
    GetWorldTimerManager().SetTimer(AttackCooldownTimer, this,
                                    &ADinosaurBase::ResetAttackCooldown,
                                    DinoStats.AttackCooldown, false);
}

bool ADinosaurBase::CanAttack() const
{
    return !bAttackOnCooldown && IsAlive();
}

void ADinosaurBase::ResetAttackCooldown()
{
    bAttackOnCooldown = false;
}

// --- Overlap Callbacks ---

void ADinosaurBase::OnAggroVolumeBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                                               UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                                               bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor || OtherActor == this) return;
    if (!IsAlive()) return;

    // Only aggro on player characters (ACharacter subclasses that are not dinosaurs)
    ACharacter* OtherChar = Cast<ACharacter>(OtherActor);
    if (!OtherChar) return;

    // Don't aggro on other dinosaurs
    if (Cast<ADinosaurBase>(OtherActor)) return;

    // Carnivores and omnivores aggro on sight
    if (DinoStats.Diet != ECore_DinoDiet::Herbivore)
    {
        CurrentTarget = OtherActor;
        SetAggroState(ECore_DinoAggroState::Chasing);
    }
    else
    {
        // Herbivores become alerted but don't chase unless attacked
        if (AggroState == ECore_DinoAggroState::Patrolling || AggroState == ECore_DinoAggroState::Idle)
        {
            SetAggroState(ECore_DinoAggroState::Alerted);
        }
    }
}

void ADinosaurBase::OnAggroVolumeEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                                             UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (OtherActor == CurrentTarget)
    {
        // Target escaped aggro radius — return to patrol
        CurrentTarget = nullptr;
        if (AggroState == ECore_DinoAggroState::Chasing || AggroState == ECore_DinoAggroState::Alerted)
        {
            SetAggroState(ECore_DinoAggroState::Patrolling);
        }
    }
}

// --- Blueprint Native Events ---

void ADinosaurBase::OnDeath_Implementation(AActor* Killer)
{
    SetAggroState(ECore_DinoAggroState::Dead);

    // Enable ragdoll physics on the mesh
    if (GetMesh())
    {
        GetMesh()->SetSimulatePhysics(true);
        GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    }

    // Disable movement
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->DisableMovement();
    }

    // Disable capsule collision so player can walk over the corpse
    if (GetCapsuleComponent())
    {
        GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
}

void ADinosaurBase::OnAggroStateChanged_Implementation(ECore_DinoAggroState OldState, ECore_DinoAggroState NewState)
{
    // Base implementation is a no-op.
    // Override in Blueprint to play roar sounds, trigger animations, etc.
}
