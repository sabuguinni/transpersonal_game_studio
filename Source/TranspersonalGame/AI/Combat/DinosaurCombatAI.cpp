// DinosaurCombatAI.cpp
// Agent #12 — Combat & Enemy AI
// Full implementation of UDinosaurCombatAI component

#include "DinosaurCombatAI.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"
#include "NavigationSystem.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────────────────

UDinosaurCombatAI::UDinosaurCombatAI()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz tick for performance

    // Default species: Raptor
    Species = ECombat_DinoSpecies::Velociraptor;
    CurrentState = ECombat_DinoState::Idle;
    bIsAlerted = false;
    bIsPackLeader = false;
    CurrentHealth = 100.f;
    MaxHealth = 100.f;
    CurrentStamina = 100.f;
    MaxStamina = 100.f;
    ThreatLevel = 0.f;
    LastAttackTime = -999.f;
    FlankAngle = 0.f;

    // Apply default species traits
    ApplySpeciesTraits(Species);
}

// ─────────────────────────────────────────────────────────────────────────────
// BeginPlay
// ─────────────────────────────────────────────────────────────────────────────

void UDinosaurCombatAI::BeginPlay()
{
    Super::BeginPlay();

    ApplySpeciesTraits(Species);
    CurrentHealth = SpeciesTraits.MaxHealth;
    MaxHealth = SpeciesTraits.MaxHealth;
    CurrentStamina = 100.f;

    // Start idle patrol loop
    GetWorld()->GetTimerManager().SetTimer(
        StateUpdateTimer,
        this,
        &UDinosaurCombatAI::UpdateCombatState,
        0.25f,
        true
    );
}

// ─────────────────────────────────────────────────────────────────────────────
// TickComponent
// ─────────────────────────────────────────────────────────────────────────────

void UDinosaurCombatAI::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Stamina recovery when not charging
    if (CurrentState != ECombat_DinoState::Charging && CurrentState != ECombat_DinoState::Attacking)
    {
        CurrentStamina = FMath::Min(MaxStamina, CurrentStamina + (15.f * DeltaTime));
    }

    // Threat decay over time
    ThreatLevel = FMath::Max(0.f, ThreatLevel - (5.f * DeltaTime));

    // Update threat table — decay old entries
    for (auto It = ThreatTable.CreateIterator(); It; ++It)
    {
        It->Value.ThreatScore = FMath::Max(0.f, It->Value.ThreatScore - (2.f * DeltaTime));
        if (It->Value.ThreatScore <= 0.f)
        {
            It.RemoveCurrent();
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// ApplySpeciesTraits
// ─────────────────────────────────────────────────────────────────────────────

void UDinosaurCombatAI::ApplySpeciesTraits(ECombat_DinoSpecies InSpecies)
{
    switch (InSpecies)
    {
    case ECombat_DinoSpecies::TyrannosaurusRex:
        SpeciesTraits.MaxHealth = 2000.f;
        SpeciesTraits.MoveSpeed = 600.f;
        SpeciesTraits.ChargeSpeed = 900.f;
        SpeciesTraits.AttackDamage = 250.f;
        SpeciesTraits.AttackRange = 300.f;
        SpeciesTraits.DetectionRange = 3000.f;
        SpeciesTraits.AggroRange = 1500.f;
        SpeciesTraits.FleeHealthThreshold = 0.f;   // Never flees
        SpeciesTraits.bIsPack = false;
        SpeciesTraits.PackSize = 1;
        SpeciesTraits.bCanRoar = true;
        SpeciesTraits.RoarRadius = 2000.f;
        SpeciesTraits.AttackCooldown = 2.5f;
        break;

    case ECombat_DinoSpecies::Velociraptor:
        SpeciesTraits.MaxHealth = 150.f;
        SpeciesTraits.MoveSpeed = 900.f;
        SpeciesTraits.ChargeSpeed = 1400.f;
        SpeciesTraits.AttackDamage = 45.f;
        SpeciesTraits.AttackRange = 120.f;
        SpeciesTraits.DetectionRange = 1800.f;
        SpeciesTraits.AggroRange = 800.f;
        SpeciesTraits.FleeHealthThreshold = 0.15f; // Flees below 15% HP
        SpeciesTraits.bIsPack = true;
        SpeciesTraits.PackSize = 4;
        SpeciesTraits.bCanRoar = false;
        SpeciesTraits.RoarRadius = 600.f;
        SpeciesTraits.AttackCooldown = 0.8f;
        break;

    case ECombat_DinoSpecies::Triceratops:
        SpeciesTraits.MaxHealth = 800.f;
        SpeciesTraits.MoveSpeed = 500.f;
        SpeciesTraits.ChargeSpeed = 850.f;
        SpeciesTraits.AttackDamage = 120.f;
        SpeciesTraits.AttackRange = 200.f;
        SpeciesTraits.DetectionRange = 1200.f;
        SpeciesTraits.AggroRange = 400.f;           // Only aggros if threatened
        SpeciesTraits.FleeHealthThreshold = 0.1f;
        SpeciesTraits.bIsPack = false;
        SpeciesTraits.PackSize = 1;
        SpeciesTraits.bCanRoar = false;
        SpeciesTraits.RoarRadius = 0.f;
        SpeciesTraits.AttackCooldown = 3.0f;
        break;

    case ECombat_DinoSpecies::Brachiosaurus:
        SpeciesTraits.MaxHealth = 3000.f;
        SpeciesTraits.MoveSpeed = 350.f;
        SpeciesTraits.ChargeSpeed = 500.f;
        SpeciesTraits.AttackDamage = 180.f;         // Stomp damage
        SpeciesTraits.AttackRange = 400.f;
        SpeciesTraits.DetectionRange = 800.f;
        SpeciesTraits.AggroRange = 200.f;           // Rarely aggros
        SpeciesTraits.FleeHealthThreshold = 0.05f;
        SpeciesTraits.bIsPack = false;
        SpeciesTraits.PackSize = 1;
        SpeciesTraits.bCanRoar = false;
        SpeciesTraits.RoarRadius = 0.f;
        SpeciesTraits.AttackCooldown = 4.0f;
        break;

    case ECombat_DinoSpecies::Pterodactyl:
        SpeciesTraits.MaxHealth = 120.f;
        SpeciesTraits.MoveSpeed = 1200.f;           // Flying speed
        SpeciesTraits.ChargeSpeed = 1800.f;
        SpeciesTraits.AttackDamage = 35.f;
        SpeciesTraits.AttackRange = 150.f;
        SpeciesTraits.DetectionRange = 2500.f;
        SpeciesTraits.AggroRange = 600.f;
        SpeciesTraits.FleeHealthThreshold = 0.3f;  // Flees early
        SpeciesTraits.bIsPack = true;
        SpeciesTraits.PackSize = 3;
        SpeciesTraits.bCanRoar = false;
        SpeciesTraits.RoarRadius = 0.f;
        SpeciesTraits.AttackCooldown = 1.2f;
        break;

    case ECombat_DinoSpecies::Ankylosaurus:
        SpeciesTraits.MaxHealth = 1200.f;
        SpeciesTraits.MoveSpeed = 300.f;
        SpeciesTraits.ChargeSpeed = 450.f;
        SpeciesTraits.AttackDamage = 200.f;         // Tail club
        SpeciesTraits.AttackRange = 250.f;
        SpeciesTraits.DetectionRange = 900.f;
        SpeciesTraits.AggroRange = 300.f;
        SpeciesTraits.FleeHealthThreshold = 0.0f;  // Never flees
        SpeciesTraits.bIsPack = false;
        SpeciesTraits.PackSize = 1;
        SpeciesTraits.bCanRoar = false;
        SpeciesTraits.RoarRadius = 0.f;
        SpeciesTraits.AttackCooldown = 3.5f;
        break;

    default:
        // Generic fallback
        SpeciesTraits.MaxHealth = 200.f;
        SpeciesTraits.MoveSpeed = 500.f;
        SpeciesTraits.ChargeSpeed = 700.f;
        SpeciesTraits.AttackDamage = 50.f;
        SpeciesTraits.AttackRange = 150.f;
        SpeciesTraits.DetectionRange = 1200.f;
        SpeciesTraits.AggroRange = 500.f;
        SpeciesTraits.FleeHealthThreshold = 0.1f;
        SpeciesTraits.bIsPack = false;
        SpeciesTraits.PackSize = 1;
        SpeciesTraits.bCanRoar = false;
        SpeciesTraits.RoarRadius = 0.f;
        SpeciesTraits.AttackCooldown = 1.5f;
        break;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// UpdateCombatState — main state machine (called every 0.25s)
// ─────────────────────────────────────────────────────────────────────────────

void UDinosaurCombatAI::UpdateCombatState()
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    // Health-based flee check
    float HealthPct = CurrentHealth / FMath::Max(1.f, MaxHealth);
    if (HealthPct <= SpeciesTraits.FleeHealthThreshold && SpeciesTraits.FleeHealthThreshold > 0.f)
    {
        SetCombatState(ECombat_DinoState::Fleeing);
        return;
    }

    // Find highest-threat target
    AActor* HighestThreatTarget = nullptr;
    float HighestThreat = 0.f;
    for (auto& Pair : ThreatTable)
    {
        if (Pair.Value.ThreatScore > HighestThreat && IsValid(Pair.Key))
        {
            HighestThreat = Pair.Value.ThreatScore;
            HighestThreatTarget = Pair.Key;
        }
    }

    if (HighestThreatTarget)
    {
        float Dist = FVector::Dist(Owner->GetActorLocation(), HighestThreatTarget->GetActorLocation());

        if (Dist <= SpeciesTraits.AttackRange)
        {
            SetCombatState(ECombat_DinoState::Attacking);
            ExecuteAttack(HighestThreatTarget);
        }
        else if (Dist <= SpeciesTraits.AggroRange * 1.5f)
        {
            // Pack flanking logic
            if (SpeciesTraits.bIsPack && !bIsPackLeader)
            {
                SetCombatState(ECombat_DinoState::Flanking);
            }
            else
            {
                SetCombatState(ECombat_DinoState::Charging);
            }
        }
        else
        {
            SetCombatState(ECombat_DinoState::Investigating);
        }
    }
    else
    {
        // No threats — patrol or idle
        if (CurrentState != ECombat_DinoState::Idle && CurrentState != ECombat_DinoState::Patrolling)
        {
            SetCombatState(ECombat_DinoState::Patrolling);
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// SetCombatState
// ─────────────────────────────────────────────────────────────────────────────

void UDinosaurCombatAI::SetCombatState(ECombat_DinoState NewState)
{
    if (CurrentState == NewState) return;

    ECombat_DinoState OldState = CurrentState;
    CurrentState = NewState;

    OnCombatStateChanged.Broadcast(OldState, NewState);

    // Update AI controller blackboard if available
    AActor* Owner = GetOwner();
    if (!Owner) return;

    APawn* Pawn = Cast<APawn>(Owner);
    if (!Pawn) return;

    AAIController* AIC = Cast<AAIController>(Pawn->GetController());
    if (!AIC) return;

    UBlackboardComponent* BB = AIC->GetBlackboardComponent();
    if (!BB) return;

    BB->SetValueAsEnum(FName("CombatState"), (uint8)NewState);
}

// ─────────────────────────────────────────────────────────────────────────────
// RegisterThreat
// ─────────────────────────────────────────────────────────────────────────────

void UDinosaurCombatAI::RegisterThreat(AActor* ThreatSource, float ThreatAmount)
{
    if (!IsValid(ThreatSource)) return;

    if (ThreatTable.Contains(ThreatSource))
    {
        ThreatTable[ThreatSource].ThreatScore = FMath::Min(100.f, ThreatTable[ThreatSource].ThreatScore + ThreatAmount);
        ThreatTable[ThreatSource].LastSeenTime = GetWorld()->GetTimeSeconds();
        ThreatTable[ThreatSource].LastSeenLocation = ThreatSource->GetActorLocation();
    }
    else
    {
        FCombat_ThreatEntry NewEntry;
        NewEntry.ThreatActor = ThreatSource;
        NewEntry.ThreatScore = FMath::Min(100.f, ThreatAmount);
        NewEntry.LastSeenTime = GetWorld()->GetTimeSeconds();
        NewEntry.LastSeenLocation = ThreatSource->GetActorLocation();
        NewEntry.bIsVisible = true;
        ThreatTable.Add(ThreatSource, NewEntry);
    }

    ThreatLevel = FMath::Min(100.f, ThreatLevel + ThreatAmount * 0.5f);
    bIsAlerted = (ThreatLevel > 20.f);

    // Alert pack members if pack leader
    if (bIsPackLeader && SpeciesTraits.bIsPack)
    {
        AlertPackMembers(ThreatSource);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// ExecuteAttack
// ─────────────────────────────────────────────────────────────────────────────

void UDinosaurCombatAI::ExecuteAttack(AActor* Target)
{
    if (!IsValid(Target)) return;

    float Now = GetWorld()->GetTimeSeconds();
    if (Now - LastAttackTime < SpeciesTraits.AttackCooldown) return;

    LastAttackTime = Now;

    float Dist = FVector::Dist(GetOwner()->GetActorLocation(), Target->GetActorLocation());
    if (Dist > SpeciesTraits.AttackRange) return;

    // Apply damage via UE5 damage system
    UGameplayStatics::ApplyDamage(
        Target,
        SpeciesTraits.AttackDamage,
        nullptr,
        GetOwner(),
        nullptr
    );

    OnAttackExecuted.Broadcast(Target, SpeciesTraits.AttackDamage);

    // Stamina cost
    CurrentStamina = FMath::Max(0.f, CurrentStamina - 15.f);
}

// ─────────────────────────────────────────────────────────────────────────────
// TakeDamage (called externally)
// ─────────────────────────────────────────────────────────────────────────────

void UDinosaurCombatAI::TakeCombatDamage(float DamageAmount, AActor* DamageSource)
{
    CurrentHealth = FMath::Max(0.f, CurrentHealth - DamageAmount);

    if (IsValid(DamageSource))
    {
        // Damage from a source = high threat registration
        RegisterThreat(DamageSource, DamageAmount * 2.f);
    }

    if (CurrentHealth <= 0.f)
    {
        SetCombatState(ECombat_DinoState::Dead);
        OnDinosaurDied.Broadcast(GetOwner());
    }
    else
    {
        float HealthPct = CurrentHealth / MaxHealth;
        if (HealthPct <= SpeciesTraits.FleeHealthThreshold && SpeciesTraits.FleeHealthThreshold > 0.f)
        {
            SetCombatState(ECombat_DinoState::Fleeing);
        }
        else if (CurrentState == ECombat_DinoState::Idle || CurrentState == ECombat_DinoState::Patrolling)
        {
            SetCombatState(ECombat_DinoState::Investigating);
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// AlertPackMembers
// ─────────────────────────────────────────────────────────────────────────────

void UDinosaurCombatAI::AlertPackMembers(AActor* ThreatTarget)
{
    if (!IsValid(ThreatTarget)) return;

    AActor* Owner = GetOwner();
    if (!Owner) return;

    // Find nearby actors of same class within roar radius
    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), Owner->GetClass(), NearbyActors);

    for (AActor* PackMember : NearbyActors)
    {
        if (PackMember == Owner) continue;

        float Dist = FVector::Dist(Owner->GetActorLocation(), PackMember->GetActorLocation());
        if (Dist > SpeciesTraits.RoarRadius) continue;

        UDinosaurCombatAI* MemberAI = PackMember->FindComponentByClass<UDinosaurCombatAI>();
        if (MemberAI)
        {
            MemberAI->RegisterThreat(ThreatTarget, 50.f);
            // Assign flanking angles to pack members
            MemberAI->FlankAngle = FMath::RandRange(-120.f, 120.f);
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// GetFlankPosition
// ─────────────────────────────────────────────────────────────────────────────

FVector UDinosaurCombatAI::GetFlankPosition(AActor* Target) const
{
    if (!IsValid(Target)) return FVector::ZeroVector;

    FVector TargetLoc = Target->GetActorLocation();
    FVector TargetFwd = Target->GetActorForwardVector();

    // Rotate around target by flank angle at attack range distance
    FVector FlankDir = TargetFwd.RotateAngleAxis(FlankAngle, FVector::UpVector);
    return TargetLoc + FlankDir * (SpeciesTraits.AttackRange * 1.5f);
}

// ─────────────────────────────────────────────────────────────────────────────
// GetCurrentStateAsString (debug utility)
// ─────────────────────────────────────────────────────────────────────────────

FString UDinosaurCombatAI::GetCurrentStateAsString() const
{
    switch (CurrentState)
    {
    case ECombat_DinoState::Idle:          return TEXT("Idle");
    case ECombat_DinoState::Patrolling:    return TEXT("Patrolling");
    case ECombat_DinoState::Investigating: return TEXT("Investigating");
    case ECombat_DinoState::Alerted:       return TEXT("Alerted");
    case ECombat_DinoState::Stalking:      return TEXT("Stalking");
    case ECombat_DinoState::Charging:      return TEXT("Charging");
    case ECombat_DinoState::Attacking:     return TEXT("Attacking");
    case ECombat_DinoState::Flanking:      return TEXT("Flanking");
    case ECombat_DinoState::Fleeing:       return TEXT("Fleeing");
    case ECombat_DinoState::Dead:          return TEXT("Dead");
    default:                               return TEXT("Unknown");
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// IsTargetInRange
// ─────────────────────────────────────────────────────────────────────────────

bool UDinosaurCombatAI::IsTargetInRange(AActor* Target, float Range) const
{
    if (!IsValid(Target) || !GetOwner()) return false;
    return FVector::Dist(GetOwner()->GetActorLocation(), Target->GetActorLocation()) <= Range;
}

// ─────────────────────────────────────────────────────────────────────────────
// GetHighestThreatTarget
// ─────────────────────────────────────────────────────────────────────────────

AActor* UDinosaurCombatAI::GetHighestThreatTarget() const
{
    AActor* Best = nullptr;
    float BestScore = 0.f;

    for (auto& Pair : ThreatTable)
    {
        if (Pair.Value.ThreatScore > BestScore && IsValid(Pair.Key))
        {
            BestScore = Pair.Value.ThreatScore;
            Best = Pair.Key;
        }
    }

    return Best;
}

// ─────────────────────────────────────────────────────────────────────────────
// ClearAllThreats
// ─────────────────────────────────────────────────────────────────────────────

void UDinosaurCombatAI::ClearAllThreats()
{
    ThreatTable.Empty();
    ThreatLevel = 0.f;
    bIsAlerted = false;
    SetCombatState(ECombat_DinoState::Idle);
}
