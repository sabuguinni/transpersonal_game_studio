#include "DinosaurCombatAI.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UDinosaurCombatAI::UDinosaurCombatAI()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz update — performance friendly
}

void UDinosaurCombatAI::BeginPlay()
{
    Super::BeginPlay();

    // Apply species-specific stat presets
    switch (Species)
    {
    case ECombat_DinoSpecies::TRex:
        Stats.MaxHealth = 500.f;
        Stats.Health = 500.f;
        Stats.AttackDamage = 80.f;
        Stats.AttackRange = 350.f;
        Stats.DetectionRange = 2500.f;
        Stats.MoveSpeed = 350.f;
        Stats.ChargeSpeed = 700.f;
        Stats.bIsPackHunter = false;
        break;

    case ECombat_DinoSpecies::Velociraptor:
        Stats.MaxHealth = 80.f;
        Stats.Health = 80.f;
        Stats.AttackDamage = 20.f;
        Stats.AttackRange = 150.f;
        Stats.DetectionRange = 1800.f;
        Stats.MoveSpeed = 600.f;
        Stats.ChargeSpeed = 900.f;
        Stats.bIsPackHunter = true;
        Stats.PackSize = 3;
        break;

    case ECombat_DinoSpecies::Triceratops:
        Stats.MaxHealth = 300.f;
        Stats.Health = 300.f;
        Stats.AttackDamage = 50.f;
        Stats.AttackRange = 250.f;
        Stats.DetectionRange = 1000.f;
        Stats.MoveSpeed = 300.f;
        Stats.ChargeSpeed = 600.f;
        Stats.bIsPackHunter = false;
        bIsAggressive = false; // Only attacks when threatened
        break;

    case ECombat_DinoSpecies::Brachiosaurus:
        Stats.MaxHealth = 800.f;
        Stats.Health = 800.f;
        Stats.AttackDamage = 30.f; // Stomp damage
        Stats.AttackRange = 400.f;
        Stats.DetectionRange = 800.f;
        Stats.MoveSpeed = 200.f;
        Stats.ChargeSpeed = 300.f;
        Stats.bIsPackHunter = false;
        bIsAggressive = false;
        FleeHealthThreshold = 0.1f;
        break;

    case ECombat_DinoSpecies::Pterodactyl:
        Stats.MaxHealth = 60.f;
        Stats.Health = 60.f;
        Stats.AttackDamage = 15.f;
        Stats.AttackRange = 200.f;
        Stats.DetectionRange = 3000.f;
        Stats.MoveSpeed = 700.f;
        Stats.ChargeSpeed = 1100.f;
        Stats.bIsPackHunter = false;
        break;

    default:
        break;
    }
}

void UDinosaurCombatAI::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    UpdateCombatState(DeltaTime);
}

void UDinosaurCombatAI::UpdateCombatState(float DeltaTime)
{
    // Cooldown tick
    if (AttackCooldownTimer > 0.f)
    {
        AttackCooldownTimer -= DeltaTime;
    }

    // Flee check — override everything if critically wounded
    if (Stats.Health / Stats.MaxHealth <= FleeHealthThreshold && CurrentState != ECombat_DinoState::Fleeing)
    {
        CurrentState = ECombat_DinoState::Fleeing;
        return;
    }

    // Find target if we don't have one
    if (!CombatTarget || !IsValid(CombatTarget))
    {
        CombatTarget = FindNearestPlayer();
    }

    switch (CurrentState)
    {
    case ECombat_DinoState::Idle:
        HandleIdleState(DeltaTime);
        break;
    case ECombat_DinoState::Stalking:
        HandleStalkingState(DeltaTime);
        break;
    case ECombat_DinoState::Charging:
        HandleChargingState(DeltaTime);
        break;
    case ECombat_DinoState::Attacking:
        HandleAttackingState(DeltaTime);
        break;
    case ECombat_DinoState::Retreating:
        HandleRetreatingState(DeltaTime);
        break;
    default:
        break;
    }
}

void UDinosaurCombatAI::HandleIdleState(float DeltaTime)
{
    if (!CombatTarget) return;

    float Dist = GetDistanceToTarget();
    if (Dist <= Stats.DetectionRange && CanSeeTarget())
    {
        if (bIsAggressive)
        {
            CurrentState = ECombat_DinoState::Stalking;
            StalkTimer = StalkDuration;

            if (Stats.bIsPackHunter)
            {
                AlertPackMembers();
            }
        }
    }
}

void UDinosaurCombatAI::HandleStalkingState(float DeltaTime)
{
    StalkTimer -= DeltaTime;

    if (!CombatTarget) { CurrentState = ECombat_DinoState::Idle; return; }

    float Dist = GetDistanceToTarget();

    // Close enough to attack directly
    if (Dist <= Stats.AttackRange)
    {
        CurrentState = ECombat_DinoState::Attacking;
        return;
    }

    // Stalk timer expired — charge!
    if (StalkTimer <= 0.f)
    {
        StartCharge(CombatTarget->GetActorLocation());
        return;
    }

    // Lost sight — return to idle
    if (!CanSeeTarget() && Dist > Stats.DetectionRange * 1.5f)
    {
        CurrentState = ECombat_DinoState::Idle;
        CombatTarget = nullptr;
    }
}

void UDinosaurCombatAI::HandleChargingState(float DeltaTime)
{
    if (!CombatTarget) { CurrentState = ECombat_DinoState::Idle; return; }

    float Dist = GetDistanceToTarget();

    if (Dist <= Stats.AttackRange)
    {
        CurrentState = ECombat_DinoState::Attacking;
        bCharging = false;
    }
}

void UDinosaurCombatAI::HandleAttackingState(float DeltaTime)
{
    if (!CombatTarget) { CurrentState = ECombat_DinoState::Idle; return; }

    float Dist = GetDistanceToTarget();

    if (Dist > Stats.AttackRange * 1.5f)
    {
        // Target escaped — re-stalk
        CurrentState = ECombat_DinoState::Stalking;
        StalkTimer = StalkDuration * 0.5f;
        return;
    }

    if (IsInAttackRange() && AttackCooldownTimer <= 0.f)
    {
        ExecuteAttack();
    }
}

void UDinosaurCombatAI::HandleRetreatingState(float DeltaTime)
{
    // After retreating for a moment, go back to idle
    StalkTimer -= DeltaTime;
    if (StalkTimer <= 0.f)
    {
        CurrentState = ECombat_DinoState::Idle;
    }
}

void UDinosaurCombatAI::SetCombatTarget(AActor* NewTarget)
{
    CombatTarget = NewTarget;
    if (NewTarget && CurrentState == ECombat_DinoState::Idle)
    {
        CurrentState = ECombat_DinoState::Stalking;
        StalkTimer = StalkDuration;
    }
}

void UDinosaurCombatAI::ExecuteAttack()
{
    if (!CombatTarget || !IsValid(CombatTarget)) return;

    AttackCooldownTimer = Stats.AttackCooldown;

    // Apply damage to target
    UGameplayStatics::ApplyDamage(
        CombatTarget,
        Stats.AttackDamage,
        nullptr,
        GetOwner(),
        nullptr
    );

    UE_LOG(LogTemp, Log, TEXT("DinosaurCombatAI: %s attacked %s for %.1f damage"),
        *GetOwner()->GetName(), *CombatTarget->GetName(), Stats.AttackDamage);
}

void UDinosaurCombatAI::TakeDamage_Combat(float DamageAmount, AActor* DamageSource)
{
    Stats.Health = FMath::Max(0.f, Stats.Health - DamageAmount);

    UE_LOG(LogTemp, Log, TEXT("DinosaurCombatAI: %s took %.1f damage — HP: %.1f/%.1f"),
        *GetOwner()->GetName(), DamageAmount, Stats.Health, Stats.MaxHealth);

    // Retaliate if not already in combat
    if (DamageSource && CurrentState == ECombat_DinoState::Idle)
    {
        SetCombatTarget(DamageSource);
    }

    // Retreat if health is low but not at flee threshold
    float HealthPct = Stats.Health / Stats.MaxHealth;
    if (HealthPct <= 0.4f && HealthPct > FleeHealthThreshold)
    {
        CurrentState = ECombat_DinoState::Retreating;
        StalkTimer = 3.f;
    }
}

void UDinosaurCombatAI::StartCharge(FVector TargetLocation)
{
    bCharging = true;
    ChargeTargetLocation = TargetLocation;
    CurrentState = ECombat_DinoState::Charging;

    UE_LOG(LogTemp, Log, TEXT("DinosaurCombatAI: %s CHARGING at target!"), *GetOwner()->GetName());
}

void UDinosaurCombatAI::AlertPackMembers()
{
    if (!GetOwner()) return;

    UWorld* World = GetOwner()->GetWorld();
    if (!World) return;

    // Find nearby pack members within 1000 units
    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), NearbyActors);

    int32 AlertCount = 0;
    for (AActor* Actor : NearbyActors)
    {
        if (Actor == GetOwner()) continue;

        UDinosaurCombatAI* PackMember = Actor->FindComponentByClass<UDinosaurCombatAI>();
        if (PackMember && PackMember->Species == Species)
        {
            float Dist = FVector::Dist(GetOwner()->GetActorLocation(), Actor->GetActorLocation());
            if (Dist <= 1000.f && CombatTarget)
            {
                PackMember->SetCombatTarget(CombatTarget);
                AlertCount++;
                if (AlertCount >= Stats.PackSize - 1) break;
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("DinosaurCombatAI: Pack alert sent to %d members"), AlertCount);
}

bool UDinosaurCombatAI::CanSeeTarget() const
{
    if (!CombatTarget || !GetOwner()) return false;

    UWorld* World = GetOwner()->GetWorld();
    if (!World) return false;

    FVector Start = GetOwner()->GetActorLocation() + FVector(0, 0, 50.f);
    FVector End = CombatTarget->GetActorLocation() + FVector(0, 0, 50.f);

    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(GetOwner());

    bool bHit = World->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params);

    // Can see if no blocking hit, or hit the target directly
    return !bHit || Hit.GetActor() == CombatTarget;
}

bool UDinosaurCombatAI::IsInAttackRange() const
{
    return GetDistanceToTarget() <= Stats.AttackRange;
}

float UDinosaurCombatAI::GetHealthPercent() const
{
    if (Stats.MaxHealth <= 0.f) return 0.f;
    return Stats.Health / Stats.MaxHealth;
}

AActor* UDinosaurCombatAI::FindNearestPlayer() const
{
    if (!GetOwner()) return nullptr;

    UWorld* World = GetOwner()->GetWorld();
    if (!World) return nullptr;

    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC) return nullptr;

    APawn* PlayerPawn = PC->GetPawn();
    if (!PlayerPawn) return nullptr;

    float Dist = FVector::Dist(GetOwner()->GetActorLocation(), PlayerPawn->GetActorLocation());
    if (Dist <= Stats.DetectionRange)
    {
        return PlayerPawn;
    }

    return nullptr;
}

float UDinosaurCombatAI::GetDistanceToTarget() const
{
    if (!CombatTarget || !GetOwner()) return TNumericLimits<float>::Max();
    return FVector::Dist(GetOwner()->GetActorLocation(), CombatTarget->GetActorLocation());
}
