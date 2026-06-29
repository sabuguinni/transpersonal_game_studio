// DinosaurBase.cpp — Base implementation for all dinosaur species
// Engine Architect #02 — Transpersonal Game Studio
// Cycle: PROD_CYCLE_AUTO_20260629_012

#include "DinosaurBase.h"
#include "Components/SphereComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

ADinosaurBase::ADinosaurBase()
{
    PrimaryActorTick.bCanEverTick = true;

    // Detection sphere for threat/prey awareness
    DetectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("DetectionSphere"));
    DetectionSphere->SetupAttachment(RootComponent);
    DetectionSphere->SetSphereRadius(1500.0f);
    DetectionSphere->SetCollisionProfileName(TEXT("OverlapAll"));

    // Default movement settings
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed = Stats.WalkSpeed;
        GetCharacterMovement()->bOrientRotationToMovement = true;
        GetCharacterMovement()->RotationRate = FRotator(0.0f, 360.0f, 0.0f);
        GetCharacterMovement()->GravityScale = 1.0f;
        GetCharacterMovement()->JumpZVelocity = 0.0f; // Most dinos don't jump
    }

    bUseControllerRotationYaw = false;
    bUseControllerRotationPitch = false;
    bUseControllerRotationRoll = false;
}

void ADinosaurBase::BeginPlay()
{
    Super::BeginPlay();

    // Initialize detection sphere radius from stats
    if (DetectionSphere)
    {
        DetectionSphere->SetSphereRadius(Stats.DetectionRadius);
    }

    // Set initial walk speed
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed = Stats.WalkSpeed;
    }

    CurrentBehavior = EDinosaurBehavior::Idle;
}

void ADinosaurBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!IsAlive()) return;

    TimeSinceLastAttack += DeltaTime;

    UpdateHunger(DeltaTime);
    UpdateBehaviorTick(DeltaTime);
}

float ADinosaurBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    Stats.CurrentHealth = FMath::Clamp(Stats.CurrentHealth - DamageAmount, 0.0f, Stats.MaxHealth);

    if (!IsAlive())
    {
        OnDeath();
    }
    else if (bIsAggressive && DamageCauser && CurrentBehavior != EDinosaurBehavior::Attacking)
    {
        CurrentTarget = DamageCauser;
        SetBehavior(EDinosaurBehavior::Attacking);
    }

    return DamageAmount;
}

void ADinosaurBase::Attack(AActor* Target)
{
    if (!Target || !IsAlive()) return;
    if (TimeSinceLastAttack < AttackCooldown) return;

    float DistToTarget = FVector::Dist(GetActorLocation(), Target->GetActorLocation());
    if (DistToTarget <= Stats.AttackRange)
    {
        UGameplayStatics::ApplyDamage(Target, Stats.AttackDamage, GetController(), this, nullptr);
        TimeSinceLastAttack = 0.0f;
    }
}

void ADinosaurBase::SetBehavior(EDinosaurBehavior NewBehavior)
{
    if (CurrentBehavior == NewBehavior) return;

    CurrentBehavior = NewBehavior;

    // Adjust movement speed based on behavior
    if (GetCharacterMovement())
    {
        switch (NewBehavior)
        {
        case EDinosaurBehavior::Hunting:
        case EDinosaurBehavior::Attacking:
            GetCharacterMovement()->MaxWalkSpeed = Stats.RunSpeed;
            break;
        case EDinosaurBehavior::Fleeing:
            GetCharacterMovement()->MaxWalkSpeed = Stats.RunSpeed * 1.2f;
            break;
        case EDinosaurBehavior::Resting:
            GetCharacterMovement()->MaxWalkSpeed = 0.0f;
            break;
        default:
            GetCharacterMovement()->MaxWalkSpeed = Stats.WalkSpeed;
            break;
        }
    }
}

void ADinosaurBase::DetectThreats()
{
    if (!DetectionSphere) return;

    TArray<AActor*> OverlappingActors;
    DetectionSphere->GetOverlappingActors(OverlappingActors);

    for (AActor* Actor : OverlappingActors)
    {
        if (!Actor || Actor == this) continue;

        // Check if it's a player character
        APawn* Pawn = Cast<APawn>(Actor);
        if (Pawn && Pawn->IsPlayerControlled())
        {
            if (bIsAggressive && CurrentBehavior != EDinosaurBehavior::Attacking)
            {
                CurrentTarget = Actor;
                SetBehavior(EDinosaurBehavior::Hunting);
            }
            return;
        }
    }
}

bool ADinosaurBase::IsAlive() const
{
    return Stats.CurrentHealth > 0.0f;
}

float ADinosaurBase::GetHealthPercent() const
{
    if (Stats.MaxHealth <= 0.0f) return 0.0f;
    return Stats.CurrentHealth / Stats.MaxHealth;
}

void ADinosaurBase::HealDinosaur(float Amount)
{
    Stats.CurrentHealth = FMath::Clamp(Stats.CurrentHealth + Amount, 0.0f, Stats.MaxHealth);
}

void ADinosaurBase::OnDeath()
{
    SetBehavior(EDinosaurBehavior::Idle);

    // Disable collision and movement
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->DisableMovement();
    }

    SetActorEnableCollision(false);

    // Destroy after 10 seconds (allow ragdoll/loot interaction)
    SetLifeSpan(10.0f);
}

void ADinosaurBase::UpdateBehaviorTick(float DeltaTime)
{
    // Base behavior tick — override in derived classes for species-specific AI
    switch (CurrentBehavior)
    {
    case EDinosaurBehavior::Idle:
        // Periodically scan for threats
        DetectThreats();
        break;

    case EDinosaurBehavior::Hunting:
        if (CurrentTarget)
        {
            float Dist = FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation());
            if (Dist <= Stats.AttackRange)
            {
                SetBehavior(EDinosaurBehavior::Attacking);
            }
        }
        else
        {
            SetBehavior(EDinosaurBehavior::Wandering);
        }
        break;

    case EDinosaurBehavior::Attacking:
        if (CurrentTarget)
        {
            Attack(CurrentTarget);
        }
        else
        {
            SetBehavior(EDinosaurBehavior::Idle);
        }
        break;

    default:
        break;
    }
}

void ADinosaurBase::UpdateHunger(float DeltaTime)
{
    Stats.Hunger = FMath::Clamp(Stats.Hunger - (Stats.HungerDecayRate * DeltaTime), 0.0f, 100.0f);

    // Hungry carnivores become more aggressive
    if (Stats.Hunger < 20.0f && Diet == EDinosaurDiet::Carnivore)
    {
        bIsAggressive = true;
    }
}
