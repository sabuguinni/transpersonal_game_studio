
#include "DinosaurCombatAI.h"
#include "GameFramework/Actor.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "NavigationSystem.h"
#include "AIController.h"

// ============================================================
// UCombat_DinosaurDamageType
// ============================================================

UCombat_DinosaurDamageType::UCombat_DinosaurDamageType()
{
    ArmorPenetration = 0.5f;
    bCausesBleed = true;
    BleedDamagePerSecond = 5.f;
    BleedDuration = 8.f;
    bCausesKnockback = false;
    KnockbackForce = 1000.f;
}

// ============================================================
// UDinosaurCombatAI
// ============================================================

UDinosaurCombatAI::UDinosaurCombatAI()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz tick for performance

    // Default to Velociraptor preset
    ApplySpeciesPreset(ECombat_DinoSpecies::Velociraptor);
}

void UDinosaurCombatAI::BeginPlay()
{
    Super::BeginPlay();

    if (AActor* Owner = GetOwner())
    {
        HomeLocation = Owner->GetActorLocation();
    }

    // Apply species preset on begin play to ensure correct stats
    ApplySpeciesPreset(Species);
    Stats.CurrentHealth = Stats.MaxHealth;

    UE_LOG(LogTemp, Log, TEXT("DinosaurCombatAI: %s initialized as %s (Role: %s)"),
        *GetOwner()->GetName(),
        *UEnum::GetValueAsString(Species),
        *UEnum::GetValueAsString(PackRole));
}

void UDinosaurCombatAI::TickComponent(float DeltaTime, ELevelTick TickType,
                                       FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!IsAlive()) return;

    // Update timers
    TimeSinceLastAttack += DeltaTime;
    StateTimer += DeltaTime;

    if (CurrentTarget)
    {
        TimeSinceTargetSeen += DeltaTime;

        // Give up chase if target not seen for too long
        if (TimeSinceTargetSeen > GiveUpChaseTime)
        {
            OnTargetLost();
            return;
        }
    }

    // Route to state tick
    switch (CurrentState)
    {
        case ECombat_DinoState::Idle:      TickIdle(DeltaTime);      break;
        case ECombat_DinoState::Stalking:  TickStalking(DeltaTime);  break;
        case ECombat_DinoState::Charging:  TickCharging(DeltaTime);  break;
        case ECombat_DinoState::Attacking: TickAttacking(DeltaTime); break;
        case ECombat_DinoState::Retreating:TickRetreating(DeltaTime);break;
        case ECombat_DinoState::PackHunt:  TickPackHunt(DeltaTime);  break;
        default: break;
    }

    // Flee check — override any state if health is critical
    if (ShouldFlee() && CurrentState != ECombat_DinoState::Fleeing)
    {
        SetState(ECombat_DinoState::Fleeing);
    }
}

// ============================================================
// State Machine
// ============================================================

void UDinosaurCombatAI::SetState(ECombat_DinoState NewState)
{
    if (CurrentState == NewState) return;

    UE_LOG(LogTemp, Log, TEXT("DinosaurCombatAI [%s]: %s -> %s"),
        *GetOwner()->GetName(),
        *UEnum::GetValueAsString(CurrentState),
        *UEnum::GetValueAsString(NewState));

    CurrentState = NewState;
    StateTimer = 0.f;
}

void UDinosaurCombatAI::OnTargetDetected(AActor* Target)
{
    if (!Target || !IsAlive()) return;

    CurrentTarget = Target;
    TimeSinceTargetSeen = 0.f;

    // Pack leader initiates coordinated hunt
    if (bIsPackLeader && PackMembers.Num() > 0)
    {
        InitiatePackHunt(Target);
        SetState(ECombat_DinoState::PackHunt);
    }
    else
    {
        // Solo: stalk first, then charge
        SetState(ECombat_DinoState::Stalking);
    }
}

void UDinosaurCombatAI::OnTargetLost()
{
    UE_LOG(LogTemp, Log, TEXT("DinosaurCombatAI [%s]: Target lost — returning to Idle"),
        *GetOwner()->GetName());

    CurrentTarget = nullptr;
    TimeSinceTargetSeen = 0.f;
    SetState(ECombat_DinoState::Idle);
}

void UDinosaurCombatAI::PerformAttack()
{
    if (!CurrentTarget || !IsAlive()) return;
    if (TimeSinceLastAttack < Stats.AttackCooldown) return;

    AActor* Owner = GetOwner();
    if (!Owner) return;

    float DistToTarget = FVector::Dist(Owner->GetActorLocation(),
                                        CurrentTarget->GetActorLocation());

    if (DistToTarget > Stats.AttackRange) return;

    // Apply damage via UE5 damage system
    TSubclassOf<UDamageType> DmgType = UCombat_DinosaurDamageType::StaticClass();
    float ActualDamage = UGameplayStatics::ApplyDamage(
        CurrentTarget,
        Stats.BiteDamage,
        nullptr,
        Owner,
        DmgType
    );

    TimeSinceLastAttack = 0.f;

    UE_LOG(LogTemp, Log, TEXT("DinosaurCombatAI [%s]: Attack! Dealt %.1f damage to %s"),
        *Owner->GetName(), ActualDamage, *CurrentTarget->GetName());

    // Knockback for large species
    if (Species == ECombat_DinoSpecies::TRex || Species == ECombat_DinoSpecies::Triceratops)
    {
        FVector KnockDir = (CurrentTarget->GetActorLocation() - Owner->GetActorLocation()).GetSafeNormal();
        if (UPrimitiveComponent* PrimComp = CurrentTarget->FindComponentByClass<UPrimitiveComponent>())
        {
            PrimComp->AddImpulse(KnockDir * 1000.f, NAME_None, true);
        }
    }
}

float UDinosaurCombatAI::TakeDamage(float DamageAmount)
{
    Stats.CurrentHealth = FMath::Max(0.f, Stats.CurrentHealth - DamageAmount);

    UE_LOG(LogTemp, Log, TEXT("DinosaurCombatAI [%s]: Took %.1f damage — HP: %.1f/%.1f"),
        *GetOwner()->GetName(), DamageAmount, Stats.CurrentHealth, Stats.MaxHealth);

    if (!IsAlive())
    {
        SetState(ECombat_DinoState::Fleeing);
        UE_LOG(LogTemp, Warning, TEXT("DinosaurCombatAI [%s]: DEAD"), *GetOwner()->GetName());
    }
    else if (ShouldFlee())
    {
        SetState(ECombat_DinoState::Fleeing);
    }

    return Stats.CurrentHealth;
}

// ============================================================
// Pack Coordination
// ============================================================

void UDinosaurCombatAI::InitiatePackHunt(AActor* Target)
{
    if (!Target) return;

    int32 FlankerIndex = 0;
    int32 TotalFlankers = PackMembers.Num();

    for (FCombat_PackMember& Member : PackMembers)
    {
        if (!Member.MemberActor) continue;

        // Assign flank positions around the target
        Member.AssignedFlankPosition = CalculateFlankPosition(Target, FlankerIndex, TotalFlankers);
        Member.bIsInPosition = false;
        FlankerIndex++;

        // Signal pack members to begin hunt
        if (UDinosaurCombatAI* MemberAI = Member.MemberActor->FindComponentByClass<UDinosaurCombatAI>())
        {
            MemberAI->CurrentTarget = Target;
            MemberAI->SetState(ECombat_DinoState::PackHunt);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("DinosaurCombatAI [%s]: Pack hunt initiated — %d members coordinating"),
        *GetOwner()->GetName(), TotalFlankers);
}

FVector UDinosaurCombatAI::CalculateFlankPosition(AActor* Target, int32 FlankIndex, int32 TotalFlankers)
{
    if (!Target || TotalFlankers == 0) return FVector::ZeroVector;

    // Distribute flankers evenly around the target in a circle
    float AngleStep = 360.f / FMath::Max(TotalFlankers, 1);
    float Angle = FMath::DegreesToRadians(FlankIndex * AngleStep);
    float FlankRadius = 600.f; // Distance from target to flank position

    FVector TargetLoc = Target->GetActorLocation();
    FVector FlankOffset(
        FMath::Cos(Angle) * FlankRadius,
        FMath::Sin(Angle) * FlankRadius,
        0.f
    );

    return TargetLoc + FlankOffset;
}

void UDinosaurCombatAI::BroadcastPackSignal(ECombat_DinoState SignalState)
{
    for (FCombat_PackMember& Member : PackMembers)
    {
        if (!Member.MemberActor) continue;
        if (UDinosaurCombatAI* MemberAI = Member.MemberActor->FindComponentByClass<UDinosaurCombatAI>())
        {
            MemberAI->SetState(SignalState);
        }
    }
}

// ============================================================
// Species Presets
// ============================================================

void UDinosaurCombatAI::ApplySpeciesPreset(ECombat_DinoSpecies InSpecies)
{
    Species = InSpecies;

    switch (InSpecies)
    {
        case ECombat_DinoSpecies::TRex:
            Stats.MaxHealth = 2000.f;
            Stats.BiteDamage = 150.f;
            Stats.ClawDamage = 80.f;
            Stats.AttackRange = 400.f;
            Stats.ChargeSpeed = 700.f;
            Stats.PatrolSpeed = 250.f;
            Stats.AttackCooldown = 3.0f;
            Stats.DetectionRange = 4000.f;
            Stats.HearingRange = 2500.f;
            Stats.FleeHealthThreshold = 0.1f; // T-Rex almost never flees
            GiveUpChaseTime = 20.f;
            break;

        case ECombat_DinoSpecies::Velociraptor:
            Stats.MaxHealth = 300.f;
            Stats.BiteDamage = 45.f;
            Stats.ClawDamage = 60.f;
            Stats.AttackRange = 200.f;
            Stats.ChargeSpeed = 1100.f;
            Stats.PatrolSpeed = 400.f;
            Stats.AttackCooldown = 1.2f;
            Stats.DetectionRange = 2500.f;
            Stats.HearingRange = 1800.f;
            Stats.FleeHealthThreshold = 0.3f;
            GiveUpChaseTime = 15.f;
            break;

        case ECombat_DinoSpecies::Triceratops:
            Stats.MaxHealth = 1500.f;
            Stats.BiteDamage = 0.f;
            Stats.ClawDamage = 120.f; // Horn gore damage
            Stats.AttackRange = 350.f;
            Stats.ChargeSpeed = 800.f;
            Stats.PatrolSpeed = 200.f;
            Stats.AttackCooldown = 4.0f;
            Stats.DetectionRange = 1500.f;
            Stats.HearingRange = 800.f;
            Stats.FleeHealthThreshold = 0.15f;
            GiveUpChaseTime = 8.f; // Triceratops gives up chase quickly
            break;

        case ECombat_DinoSpecies::Brachiosaurus:
            Stats.MaxHealth = 3000.f;
            Stats.BiteDamage = 0.f;
            Stats.ClawDamage = 200.f; // Stomp damage
            Stats.AttackRange = 500.f;
            Stats.ChargeSpeed = 400.f;
            Stats.PatrolSpeed = 150.f;
            Stats.AttackCooldown = 5.0f;
            Stats.DetectionRange = 1000.f;
            Stats.HearingRange = 500.f;
            Stats.FleeHealthThreshold = 0.05f; // Almost never flees
            GiveUpChaseTime = 5.f;
            break;

        case ECombat_DinoSpecies::Pterodactyl:
            Stats.MaxHealth = 200.f;
            Stats.BiteDamage = 30.f;
            Stats.ClawDamage = 35.f;
            Stats.AttackRange = 150.f;
            Stats.ChargeSpeed = 1500.f;
            Stats.PatrolSpeed = 800.f;
            Stats.AttackCooldown = 1.0f;
            Stats.DetectionRange = 5000.f;
            Stats.HearingRange = 1000.f;
            Stats.FleeHealthThreshold = 0.4f;
            GiveUpChaseTime = 10.f;
            break;

        default:
            break;
    }

    Stats.CurrentHealth = Stats.MaxHealth;
}

// ============================================================
// Pure Queries
// ============================================================

bool UDinosaurCombatAI::IsAlive() const
{
    return Stats.CurrentHealth > 0.f;
}

bool UDinosaurCombatAI::ShouldFlee() const
{
    if (Stats.MaxHealth <= 0.f) return false;
    return (Stats.CurrentHealth / Stats.MaxHealth) <= Stats.FleeHealthThreshold;
}

bool UDinosaurCombatAI::IsInAttackRange() const
{
    if (!CurrentTarget || !GetOwner()) return false;
    float Dist = FVector::Dist(GetOwner()->GetActorLocation(), CurrentTarget->GetActorLocation());
    return Dist <= Stats.AttackRange;
}

// ============================================================
// State Tick Implementations
// ============================================================

void UDinosaurCombatAI::TickIdle(float DeltaTime)
{
    // Idle: do nothing, wait for perception to trigger OnTargetDetected
}

void UDinosaurCombatAI::TickStalking(float DeltaTime)
{
    if (!CurrentTarget) { SetState(ECombat_DinoState::Idle); return; }

    AActor* Owner = GetOwner();
    if (!Owner) return;

    float Dist = FVector::Dist(Owner->GetActorLocation(), CurrentTarget->GetActorLocation());

    // Transition to charge when close enough
    if (Dist < Stats.DetectionRange * 0.5f)
    {
        SetState(ECombat_DinoState::Charging);
    }
}

void UDinosaurCombatAI::TickCharging(float DeltaTime)
{
    if (!CurrentTarget) { SetState(ECombat_DinoState::Idle); return; }

    if (IsInAttackRange())
    {
        SetState(ECombat_DinoState::Attacking);
    }
}

void UDinosaurCombatAI::TickAttacking(float DeltaTime)
{
    if (!CurrentTarget) { SetState(ECombat_DinoState::Idle); return; }

    if (IsInAttackRange())
    {
        PerformAttack();
    }
    else
    {
        // Target moved away — charge again
        SetState(ECombat_DinoState::Charging);
    }
}

void UDinosaurCombatAI::TickRetreating(float DeltaTime)
{
    // Move back toward home location
    if (StateTimer > 5.f)
    {
        SetState(ECombat_DinoState::Idle);
    }
}

void UDinosaurCombatAI::TickPackHunt(float DeltaTime)
{
    if (!CurrentTarget) { SetState(ECombat_DinoState::Idle); return; }

    // Check if all pack members are in position
    bool bAllInPosition = true;
    for (FCombat_PackMember& Member : PackMembers)
    {
        if (!Member.bIsInPosition)
        {
            bAllInPosition = false;
            break;
        }
    }

    // When all in position, switch to simultaneous attack
    if (bAllInPosition && IsInAttackRange())
    {
        BroadcastPackSignal(ECombat_DinoState::Attacking);
        SetState(ECombat_DinoState::Attacking);
    }
}
