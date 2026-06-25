#include "DinosaurCombatAI.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

ADinosaurCombatAI::ADinosaurCombatAI()
{
    PrimaryActorTick.bCanEverTick = true;

    // Detection sphere — outer radius for awareness
    DetectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("DetectionSphere"));
    DetectionSphere->SetSphereRadius(2000.0f);
    DetectionSphere->SetCollisionProfileName(TEXT("Trigger"));
    RootComponent = DetectionSphere;

    // Attack sphere — inner radius for melee
    AttackSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AttackSphere"));
    AttackSphere->SetSphereRadius(300.0f);
    AttackSphere->SetCollisionProfileName(TEXT("Trigger"));
    AttackSphere->SetupAttachment(RootComponent);

    CurrentTarget = nullptr;
    AttackCooldownTimer = 0.0f;
}

void ADinosaurCombatAI::BeginPlay()
{
    Super::BeginPlay();

    PatrolOrigin = GetActorLocation();

    // Bind detection overlap
    DetectionSphere->OnComponentBeginOverlap.AddDynamic(this, &ADinosaurCombatAI::OnDetectionSphereOverlap);
    AttackSphere->OnComponentBeginOverlap.AddDynamic(this, &ADinosaurCombatAI::OnAttackSphereOverlap);

    // Apply species-specific stats
    switch (Species)
    {
    case ECombat_DinoSpecies::TRex:
        Stats.MaxHealth = 2000.0f;
        Stats.CurrentHealth = 2000.0f;
        Stats.AttackDamage = 300.0f;
        Stats.AttackRange = 400.0f;
        Stats.DetectionRadius = 3000.0f;
        Stats.MoveSpeed = 700.0f;
        Stats.ChargeSpeed = 1400.0f;
        Stats.bIsPackHunter = false;
        Stats.TerritoryRadius = 8000.0f;
        break;

    case ECombat_DinoSpecies::Velociraptor:
        Stats.MaxHealth = 400.0f;
        Stats.CurrentHealth = 400.0f;
        Stats.AttackDamage = 80.0f;
        Stats.AttackRange = 150.0f;
        Stats.DetectionRadius = 2500.0f;
        Stats.MoveSpeed = 900.0f;
        Stats.ChargeSpeed = 1600.0f;
        Stats.bIsPackHunter = true;
        Stats.TerritoryRadius = 4000.0f;
        break;

    case ECombat_DinoSpecies::Triceratops:
        Stats.MaxHealth = 1500.0f;
        Stats.CurrentHealth = 1500.0f;
        Stats.AttackDamage = 200.0f;
        Stats.AttackRange = 350.0f;
        Stats.DetectionRadius = 1500.0f;
        Stats.MoveSpeed = 500.0f;
        Stats.ChargeSpeed = 1100.0f;
        Stats.bIsPackHunter = false;
        Stats.TerritoryRadius = 3000.0f;
        break;

    case ECombat_DinoSpecies::Ankylosaurus:
        Stats.MaxHealth = 1800.0f;
        Stats.CurrentHealth = 1800.0f;
        Stats.AttackDamage = 250.0f;
        Stats.AttackRange = 250.0f;
        Stats.DetectionRadius = 1200.0f;
        Stats.MoveSpeed = 350.0f;
        Stats.ChargeSpeed = 700.0f;
        Stats.bIsPackHunter = false;
        Stats.TerritoryRadius = 2500.0f;
        break;

    case ECombat_DinoSpecies::Brachiosaurus:
        Stats.MaxHealth = 3000.0f;
        Stats.CurrentHealth = 3000.0f;
        Stats.AttackDamage = 400.0f;
        Stats.AttackRange = 500.0f;
        Stats.DetectionRadius = 2000.0f;
        Stats.MoveSpeed = 400.0f;
        Stats.ChargeSpeed = 800.0f;
        Stats.bIsPackHunter = false;
        Stats.TerritoryRadius = 6000.0f;
        break;

    default:
        break;
    }

    // Update detection sphere radius from stats
    DetectionSphere->SetSphereRadius(Stats.DetectionRadius);
    AttackSphere->SetSphereRadius(Stats.AttackRange);
}

void ADinosaurCombatAI::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!IsAlive()) return;

    // Cooldown timer
    if (AttackCooldownTimer > 0.0f)
    {
        AttackCooldownTimer -= DeltaTime;
    }

    // State machine update
    switch (CurrentState)
    {
    case ECombat_DinoState::Idle:
        UpdateIdleState(DeltaTime);
        break;
    case ECombat_DinoState::Alert:
        UpdateAlertState(DeltaTime);
        break;
    case ECombat_DinoState::Chase:
        UpdateChaseState(DeltaTime);
        break;
    case ECombat_DinoState::Attack:
        UpdateAttackState(DeltaTime);
        break;
    default:
        break;
    }
}

void ADinosaurCombatAI::UpdateIdleState(float DeltaTime)
{
    // Idle — look for player via overlap events
    // Territorial check: if player enters territory, go alert
    if (CurrentTarget && IsValid(CurrentTarget))
    {
        SetCombatState(ECombat_DinoState::Alert);
    }
}

void ADinosaurCombatAI::UpdateAlertState(float DeltaTime)
{
    if (!CurrentTarget || !IsValid(CurrentTarget))
    {
        SetCombatState(ECombat_DinoState::Idle);
        return;
    }

    float DistToTarget = FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation());

    // If close enough, charge
    if (DistToTarget < Stats.DetectionRadius * 0.6f)
    {
        SetCombatState(ECombat_DinoState::Chase);
    }
}

void ADinosaurCombatAI::UpdateChaseState(float DeltaTime)
{
    if (!CurrentTarget || !IsValid(CurrentTarget))
    {
        SetCombatState(ECombat_DinoState::Idle);
        return;
    }

    float DistToTarget = FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation());

    // Move toward target
    FVector Direction = (CurrentTarget->GetActorLocation() - GetActorLocation()).GetSafeNormal();
    FVector NewLocation = GetActorLocation() + Direction * Stats.ChargeSpeed * DeltaTime;
    SetActorLocation(NewLocation, true);

    // Face the target
    FRotator LookAt = Direction.Rotation();
    SetActorRotation(LookAt);

    // Switch to attack if in range
    if (DistToTarget <= Stats.AttackRange)
    {
        SetCombatState(ECombat_DinoState::Attack);
    }

    // Lost target — too far
    if (DistToTarget > Stats.TerritoryRadius)
    {
        CurrentTarget = nullptr;
        SetCombatState(ECombat_DinoState::Idle);
    }
}

void ADinosaurCombatAI::UpdateAttackState(float DeltaTime)
{
    if (!CurrentTarget || !IsValid(CurrentTarget))
    {
        SetCombatState(ECombat_DinoState::Idle);
        return;
    }

    float DistToTarget = FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation());

    // Execute attack if cooldown ready
    if (AttackCooldownTimer <= 0.0f)
    {
        ExecuteAttack();
    }

    // Target moved away — chase again
    if (DistToTarget > Stats.AttackRange * 1.5f)
    {
        SetCombatState(ECombat_DinoState::Chase);
    }
}

void ADinosaurCombatAI::TakeCombatDamage(float DamageAmount, AActor* DamageSource)
{
    if (!IsAlive()) return;

    Stats.CurrentHealth = FMath::Max(0.0f, Stats.CurrentHealth - DamageAmount);

    // Aggro toward damage source
    if (DamageSource && IsValid(DamageSource))
    {
        CurrentTarget = DamageSource;
        if (CurrentState == ECombat_DinoState::Idle || CurrentState == ECombat_DinoState::Patrol)
        {
            SetCombatState(ECombat_DinoState::Alert);
        }
    }

    if (!IsAlive())
    {
        SetCombatState(ECombat_DinoState::Dead);
    }
}

void ADinosaurCombatAI::SetCombatState(ECombat_DinoState NewState)
{
    CurrentState = NewState;
}

float ADinosaurCombatAI::GetHealthPercent() const
{
    if (Stats.MaxHealth <= 0.0f) return 0.0f;
    return Stats.CurrentHealth / Stats.MaxHealth;
}

bool ADinosaurCombatAI::IsAlive() const
{
    return Stats.CurrentHealth > 0.0f && CurrentState != ECombat_DinoState::Dead;
}

bool ADinosaurCombatAI::IsPlayerInAttackRange() const
{
    if (!CurrentTarget || !IsValid(CurrentTarget)) return false;
    float Dist = FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation());
    return Dist <= Stats.AttackRange;
}

void ADinosaurCombatAI::ExecuteAttack()
{
    if (!CurrentTarget || !IsValid(CurrentTarget)) return;
    if (!IsPlayerInAttackRange()) return;

    // Apply damage to target
    UGameplayStatics::ApplyDamage(
        CurrentTarget,
        Stats.AttackDamage,
        nullptr,
        this,
        nullptr
    );

    AttackCooldownTimer = Stats.AttackCooldown;
}

void ADinosaurCombatAI::OnDetectionSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor || OtherActor == this) return;

    // Check if it's a player character
    if (OtherActor->IsA(ACharacter::StaticClass()))
    {
        if (!CurrentTarget)
        {
            CurrentTarget = OtherActor;
        }
        if (CurrentState == ECombat_DinoState::Idle || CurrentState == ECombat_DinoState::Patrol)
        {
            SetCombatState(ECombat_DinoState::Alert);
        }
    }
}

void ADinosaurCombatAI::OnAttackSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor || OtherActor == this) return;

    if (OtherActor->IsA(ACharacter::StaticClass()))
    {
        CurrentTarget = OtherActor;
        SetCombatState(ECombat_DinoState::Attack);
    }
}
