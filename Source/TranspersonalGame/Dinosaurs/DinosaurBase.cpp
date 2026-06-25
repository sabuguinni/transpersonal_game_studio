#include "DinosaurBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"

ADinosaurBase::ADinosaurBase()
{
    PrimaryActorTick.bCanEverTick = true;

    // Default capsule for large dinosaur
    GetCapsuleComponent()->InitCapsuleSize(88.0f, 96.0f);

    // Movement defaults — overridden per species
    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        MoveComp->MaxWalkSpeed = Stats.MoveSpeed;
        MoveComp->bOrientRotationToMovement = true;
        MoveComp->RotationRate = FRotator(0.0f, 180.0f, 0.0f);
        MoveComp->GravityScale = 1.0f;
        MoveComp->Mass = Stats.Mass;
    }

    // Dinosaurs do not use controller rotation by default
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;
}

void ADinosaurBase::BeginPlay()
{
    Super::BeginPlay();

    // Record territory center at spawn location
    TerritoryCenter = GetActorLocation();

    // Initialise health
    Stats.CurrentHealth = Stats.MaxHealth;

    // Set initial behavior
    SetBehaviorState(EEng_DinosaurBehaviorState::Idle);
}

void ADinosaurBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!IsAlive())
    {
        return;
    }

    BehaviorUpdateTimer += DeltaTime;
    if (BehaviorUpdateTimer >= BehaviorUpdateInterval)
    {
        BehaviorUpdateTimer = 0.0f;
        UpdateBehavior(DeltaTime);
    }
}

float ADinosaurBase::TakeDinosaurDamage(float DamageAmount, AActor* DamageCauser)
{
    if (!IsAlive() || DamageAmount <= 0.0f)
    {
        return 0.0f;
    }

    const float ActualDamage = FMath::Min(DamageAmount, Stats.CurrentHealth);
    Stats.CurrentHealth -= ActualDamage;

    // React to damage — become aggressive toward attacker
    if (DamageCauser && BehaviorState != EEng_DinosaurBehaviorState::Fleeing)
    {
        SetBehaviorState(EEng_DinosaurBehaviorState::Aggressive);
    }

    if (Stats.CurrentHealth <= 0.0f)
    {
        Stats.CurrentHealth = 0.0f;
        OnDeath();
    }

    return ActualDamage;
}

void ADinosaurBase::Attack(AActor* Target)
{
    if (!Target || !IsAlive())
    {
        return;
    }

    // Base attack — deal damage to target
    // Subclasses override for species-specific attack animations and effects
    if (ADinosaurBase* DinoTarget = Cast<ADinosaurBase>(Target))
    {
        DinoTarget->TakeDinosaurDamage(Stats.AttackDamage, this);
    }
    else
    {
        // Apply damage to any actor (player, etc.)
        Target->TakeDamage(Stats.AttackDamage, FDamageEvent(), nullptr, this);
    }
}

void ADinosaurBase::SetBehaviorState(EEng_DinosaurBehaviorState NewState)
{
    if (BehaviorState == NewState)
    {
        return;
    }

    BehaviorState = NewState;

    // Adjust movement speed based on state
    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        switch (NewState)
        {
        case EEng_DinosaurBehaviorState::Hunting:
        case EEng_DinosaurBehaviorState::Aggressive:
            MoveComp->MaxWalkSpeed = Stats.MoveSpeed * 1.5f;
            break;
        case EEng_DinosaurBehaviorState::Fleeing:
            MoveComp->MaxWalkSpeed = Stats.MoveSpeed * 2.0f;
            break;
        case EEng_DinosaurBehaviorState::Resting:
        case EEng_DinosaurBehaviorState::Feeding:
            MoveComp->MaxWalkSpeed = 0.0f;
            break;
        default:
            MoveComp->MaxWalkSpeed = Stats.MoveSpeed;
            break;
        }
    }
}

bool ADinosaurBase::IsAlive() const
{
    return Stats.CurrentHealth > 0.0f;
}

float ADinosaurBase::GetHealthPercent() const
{
    if (Stats.MaxHealth <= 0.0f)
    {
        return 0.0f;
    }
    return Stats.CurrentHealth / Stats.MaxHealth;
}

void ADinosaurBase::OnDeath()
{
    SetBehaviorState(EEng_DinosaurBehaviorState::Idle);

    // Disable collision on death
    if (UCapsuleComponent* Capsule = GetCapsuleComponent())
    {
        Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    // Disable movement
    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        MoveComp->DisableMovement();
    }

    // Ragdoll on death — enable physics on mesh
    if (USkeletalMeshComponent* Mesh = GetMesh())
    {
        Mesh->SetSimulatePhysics(true);
        Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    }
}

void ADinosaurBase::UpdateBehavior(float DeltaTime)
{
    // Base behavior tick — subclasses implement species-specific AI
    // Default: if idle and far from territory center, roam back
    if (BehaviorState == EEng_DinosaurBehaviorState::Idle)
    {
        const float DistFromTerritory = FVector::Dist(GetActorLocation(), TerritoryCenter);
        if (DistFromTerritory > TerritoryRadius)
        {
            SetBehaviorState(EEng_DinosaurBehaviorState::Roaming);
        }
    }
}
