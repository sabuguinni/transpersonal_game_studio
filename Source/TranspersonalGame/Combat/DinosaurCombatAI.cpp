// DinosaurCombatAI.cpp
// Agent #12 — Combat & Enemy AI Agent
// Cycle: PROD_CYCLE_AUTO_20260626_009
// Implements: Dinosaur combat state machine with threat assessment,
//             flanking maneuvers, pack hunting, charge attacks.

#include "DinosaurCombatAI.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"

// ============================================================
// Constructor
// ============================================================
UDinosaurCombatAIComponent::UDinosaurCombatAIComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.05f; // 20Hz — sufficient for AI

    CurrentHealth = Stats.Health;
    CachedPlayerActor = nullptr;
}

// ============================================================
// BeginPlay
// ============================================================
void UDinosaurCombatAIComponent::BeginPlay()
{
    Super::BeginPlay();

    CurrentHealth = Stats.Health;

    // Apply species-specific defaults
    switch (Species)
    {
    case ECombat_DinoSpecies::TRex:
        Stats.DetectionRadius = 2000.0f;
        Stats.AttackRadius    = 350.0f;
        Stats.ChargeSpeed     = 850.0f;
        Stats.AttackDamage    = 120.0f;
        Stats.Health          = 800.0f;
        Stats.bIsPackHunter   = false;
        Stats.ReactionTime    = 1.2f;
        break;

    case ECombat_DinoSpecies::Velociraptor:
        Stats.DetectionRadius = 1200.0f;
        Stats.AttackRadius    = 180.0f;
        Stats.ChargeSpeed     = 1100.0f;
        Stats.AttackDamage    = 45.0f;
        Stats.Health          = 150.0f;
        Stats.bIsPackHunter   = true;
        Stats.ReactionTime    = 0.4f;
        Stats.FlankingAngle   = 60.0f;
        break;

    case ECombat_DinoSpecies::Triceratops:
        Stats.DetectionRadius = 1000.0f;
        Stats.AttackRadius    = 300.0f;
        Stats.ChargeSpeed     = 700.0f;
        Stats.AttackDamage    = 90.0f;
        Stats.Health          = 600.0f;
        Stats.bIsPackHunter   = false;
        Stats.ReactionTime    = 1.5f;
        break;

    case ECombat_DinoSpecies::Ankylosaurus:
        Stats.DetectionRadius = 800.0f;
        Stats.AttackRadius    = 250.0f;
        Stats.ChargeSpeed     = 400.0f;
        Stats.AttackDamage    = 70.0f;
        Stats.Health          = 900.0f;
        Stats.bIsPackHunter   = false;
        Stats.ReactionTime    = 2.0f;
        break;

    default:
        // Brachiosaurus — herbivore, only defends
        Stats.DetectionRadius = 1500.0f;
        Stats.AttackRadius    = 400.0f;
        Stats.ChargeSpeed     = 500.0f;
        Stats.AttackDamage    = 60.0f;
        Stats.Health          = 1200.0f;
        Stats.bIsPackHunter   = false;
        Stats.ReactionTime    = 2.5f;
        break;
    }

    CurrentHealth = Stats.Health;
    SetCombatState(ECombat_DinoState::Patrol);
}

// ============================================================
// TickComponent — main state machine update
// ============================================================
void UDinosaurCombatAIComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    StateTimer += DeltaTime;
    AttackCooldown = FMath::Max(0.0f, AttackCooldown - DeltaTime);

    // Periodic detection check (performance-friendly)
    DetectionCheckTimer += DeltaTime;
    if (DetectionCheckTimer >= DETECTION_CHECK_INTERVAL)
    {
        DetectionCheckTimer = 0.0f;
        CachedPlayerActor = FindNearestPlayer();
        if (CachedPlayerActor)
        {
            AssessThreat(CachedPlayerActor);
        }
    }

    // Route to current state handler
    switch (CurrentState)
    {
    case ECombat_DinoState::Idle:
    case ECombat_DinoState::Patrol:
        UpdateIdleState(DeltaTime);
        break;
    case ECombat_DinoState::Alert:
        UpdateAlertState(DeltaTime);
        break;
    case ECombat_DinoState::Stalk:
        UpdateStalkState(DeltaTime);
        break;
    case ECombat_DinoState::Charge:
        UpdateChargeState(DeltaTime);
        break;
    case ECombat_DinoState::Attack:
        UpdateAttackState(DeltaTime);
        break;
    case ECombat_DinoState::Retreat:
        UpdateRetreatState(DeltaTime);
        break;
    case ECombat_DinoState::Flanking:
        UpdateFlankingState(DeltaTime);
        break;
    case ECombat_DinoState::PackHunt:
        UpdatePackHuntState(DeltaTime);
        break;
    }

    // Debug visualization
    if (bDebugDrawDetection && GetOwner())
    {
        UWorld* World = GetWorld();
        if (World)
        {
            FVector OwnerLoc = GetOwner()->GetActorLocation();
            DrawDebugSphere(World, OwnerLoc, Stats.DetectionRadius, 16, FColor::Yellow, false, 0.1f);
            DrawDebugSphere(World, OwnerLoc, Stats.AttackRadius, 12, FColor::Red, false, 0.1f);
        }
    }
}

// ============================================================
// State Machine — Idle / Patrol
// ============================================================
void UDinosaurCombatAIComponent::UpdateIdleState(float DeltaTime)
{
    if (!CachedPlayerActor) return;

    if (IsPlayerInDetectionRange())
    {
        bIsAlerted = true;
        SetCombatState(ECombat_DinoState::Alert);
    }
}

// ============================================================
// State Machine — Alert (spotted player, assessing)
// ============================================================
void UDinosaurCombatAIComponent::UpdateAlertState(float DeltaTime)
{
    if (!CachedPlayerActor)
    {
        SetCombatState(ECombat_DinoState::Patrol);
        return;
    }

    // After reaction time, decide: stalk, charge, or flank
    if (StateTimer >= Stats.ReactionTime)
    {
        if (Stats.bIsPackHunter)
        {
            BroadcastPackAlert();
            SetCombatState(ECombat_DinoState::Flanking);
        }
        else if (CurrentThreat.ThreatLevel > 0.7f)
        {
            SetCombatState(ECombat_DinoState::Charge);
        }
        else
        {
            SetCombatState(ECombat_DinoState::Stalk);
        }
    }
}

// ============================================================
// State Machine — Stalk (slow approach, waiting for opening)
// ============================================================
void UDinosaurCombatAIComponent::UpdateStalkState(float DeltaTime)
{
    if (!CachedPlayerActor)
    {
        SetCombatState(ECombat_DinoState::Patrol);
        return;
    }

    if (IsPlayerInAttackRange() && AttackCooldown <= 0.0f)
    {
        SetCombatState(ECombat_DinoState::Attack);
    }
    else if (CurrentThreat.ThreatLevel > 0.85f)
    {
        SetCombatState(ECombat_DinoState::Charge);
    }
}

// ============================================================
// State Machine — Charge (full speed attack run)
// ============================================================
void UDinosaurCombatAIComponent::UpdateChargeState(float DeltaTime)
{
    if (!CachedPlayerActor)
    {
        SetCombatState(ECombat_DinoState::Patrol);
        return;
    }

    if (IsPlayerInAttackRange())
    {
        ExecuteChargeAttack();
        SetCombatState(ECombat_DinoState::Attack);
    }

    // Lost sight — go alert
    if (!IsPlayerInDetectionRange())
    {
        SetCombatState(ECombat_DinoState::Alert);
    }
}

// ============================================================
// State Machine — Attack (deal damage)
// ============================================================
void UDinosaurCombatAIComponent::UpdateAttackState(float DeltaTime)
{
    if (AttackCooldown <= 0.0f && IsPlayerInAttackRange() && CachedPlayerActor)
    {
        // Apply damage to player
        UGameplayStatics::ApplyDamage(
            CachedPlayerActor,
            Stats.AttackDamage,
            nullptr,
            GetOwner(),
            nullptr
        );
        AttackCooldown = ATTACK_COOLDOWN_DURATION;
    }

    // After attack, return to stalk or retreat
    if (StateTimer > 1.5f)
    {
        if (CurrentHealth < Stats.Health * 0.25f)
        {
            SetCombatState(ECombat_DinoState::Retreat);
        }
        else
        {
            SetCombatState(ECombat_DinoState::Stalk);
        }
    }
}

// ============================================================
// State Machine — Retreat (low health)
// ============================================================
void UDinosaurCombatAIComponent::UpdateRetreatState(float DeltaTime)
{
    // Retreat for 5 seconds then re-assess
    if (StateTimer > 5.0f)
    {
        if (CurrentHealth > Stats.Health * 0.4f)
        {
            SetCombatState(ECombat_DinoState::Alert);
        }
    }
}

// ============================================================
// State Machine — Flanking (raptor pack tactic)
// ============================================================
void UDinosaurCombatAIComponent::UpdateFlankingState(float DeltaTime)
{
    if (!CachedPlayerActor) return;

    ExecuteFlankingManeuver();

    if (IsPlayerInAttackRange() && AttackCooldown <= 0.0f)
    {
        SetCombatState(ECombat_DinoState::Attack);
    }
}

// ============================================================
// State Machine — Pack Hunt (coordinated multi-raptor)
// ============================================================
void UDinosaurCombatAIComponent::UpdatePackHuntState(float DeltaTime)
{
    if (!CachedPlayerActor) return;

    // Pack hunt: alternate between flanking and direct attack
    if (StateTimer > 3.0f)
    {
        SetCombatState(ECombat_DinoState::Flanking);
    }
}

// ============================================================
// Public API
// ============================================================
void UDinosaurCombatAIComponent::SetCombatState(ECombat_DinoState NewState)
{
    if (CurrentState == NewState) return;
    CurrentState = NewState;
    StateTimer = 0.0f;
}

void UDinosaurCombatAIComponent::OnTakeDamage(float DamageAmount, AActor* DamageCauser)
{
    CurrentHealth = FMath::Max(0.0f, CurrentHealth - DamageAmount);

    if (DamageCauser)
    {
        AssessThreat(DamageCauser);
        // Being hit always triggers alert/charge
        if (CurrentState == ECombat_DinoState::Idle || CurrentState == ECombat_DinoState::Patrol)
        {
            SetCombatState(ECombat_DinoState::Alert);
        }
    }

    if (CurrentHealth <= 0.0f)
    {
        SetCombatState(ECombat_DinoState::Retreat);
    }
}

void UDinosaurCombatAIComponent::AssessThreat(AActor* PotentialThreat)
{
    if (!PotentialThreat || !GetOwner()) return;

    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), PotentialThreat->GetActorLocation());

    CurrentThreat.ThreatActor = PotentialThreat;
    CurrentThreat.DistanceToThreat = Distance;
    CurrentThreat.ThreatLevel = CalculateThreatLevel(PotentialThreat);
    CurrentThreat.LastKnownLocation = PotentialThreat->GetActorLocation();

    // Check if target is moving
    if (APawn* Pawn = Cast<APawn>(PotentialThreat))
    {
        CurrentThreat.bIsMoving = !Pawn->GetVelocity().IsNearlyZero(10.0f);
    }
}

bool UDinosaurCombatAIComponent::IsPlayerInAttackRange() const
{
    if (!CachedPlayerActor || !GetOwner()) return false;
    float Dist = FVector::Dist(GetOwner()->GetActorLocation(), CachedPlayerActor->GetActorLocation());
    return Dist <= Stats.AttackRadius;
}

bool UDinosaurCombatAIComponent::IsPlayerInDetectionRange() const
{
    if (!CachedPlayerActor || !GetOwner()) return false;
    float Dist = FVector::Dist(GetOwner()->GetActorLocation(), CachedPlayerActor->GetActorLocation());
    return Dist <= Stats.DetectionRadius;
}

void UDinosaurCombatAIComponent::ExecuteFlankingManeuver()
{
    if (!CachedPlayerActor || !GetOwner()) return;

    // Calculate flank position offset from player
    FVector FlankPos = GetFlankPosition(CachedPlayerActor, Stats.FlankingAngle);

    // Move toward flank position (AI controller handles actual movement)
    // This sets the destination — the AIController reads CurrentThreat.LastKnownLocation
    CurrentThreat.LastKnownLocation = FlankPos;
}

void UDinosaurCombatAIComponent::ExecuteChargeAttack()
{
    if (!GetOwner()) return;

    // Charge: boost movement speed temporarily
    // The AIController / CharacterMovementComponent reads ChargeSpeed from Stats
    // Actual movement is handled by the AI Controller using MoveToActor
    // Here we just flag the state for the controller
    bIsAlerted = true;
}

void UDinosaurCombatAIComponent::BroadcastPackAlert()
{
    if (!GetOwner()) return;

    UWorld* World = GetWorld();
    if (!World) return;

    // Find nearby pack members (same species, within 3000 units)
    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), NearbyActors);

    for (AActor* Actor : NearbyActors)
    {
        if (Actor == GetOwner()) continue;

        float Dist = FVector::Dist(GetOwner()->GetActorLocation(), Actor->GetActorLocation());
        if (Dist > 3000.0f) continue;

        UDinosaurCombatAIComponent* PackMember = Actor->FindComponentByClass<UDinosaurCombatAIComponent>();
        if (PackMember && PackMember->Species == Species && PackMember->CurrentState == ECombat_DinoState::Idle)
        {
            PackMember->CachedPlayerActor = CachedPlayerActor;
            PackMember->CurrentThreat = CurrentThreat;
            PackMember->SetCombatState(ECombat_DinoState::PackHunt);
        }
    }
}

// ============================================================
// Private Helpers
// ============================================================
AActor* UDinosaurCombatAIComponent::FindNearestPlayer() const
{
    UWorld* World = GetWorld();
    if (!World || !GetOwner()) return nullptr;

    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC) return nullptr;

    APawn* PlayerPawn = PC->GetPawn();
    if (!PlayerPawn) return nullptr;

    float Dist = FVector::Dist(GetOwner()->GetActorLocation(), PlayerPawn->GetActorLocation());
    if (Dist <= Stats.DetectionRadius * 1.5f) // Slightly larger search radius
    {
        return PlayerPawn;
    }

    return nullptr;
}

float UDinosaurCombatAIComponent::CalculateThreatLevel(AActor* Target) const
{
    if (!Target || !GetOwner()) return 0.0f;

    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Target->GetActorLocation());
    float NormalizedDist = FMath::Clamp(1.0f - (Distance / Stats.DetectionRadius), 0.0f, 1.0f);

    // Movement increases threat
    float MovementBonus = 0.0f;
    if (APawn* Pawn = Cast<APawn>(Target))
    {
        float Speed = Pawn->GetVelocity().Size();
        MovementBonus = FMath::Clamp(Speed / 600.0f, 0.0f, 0.3f);
    }

    return FMath::Clamp(NormalizedDist + MovementBonus, 0.0f, 1.0f);
}

FVector UDinosaurCombatAIComponent::GetFlankPosition(AActor* Target, float AngleOffset) const
{
    if (!Target || !GetOwner()) return FVector::ZeroVector;

    FVector ToTarget = (Target->GetActorLocation() - GetOwner()->GetActorLocation()).GetSafeNormal();
    FVector Right = FVector::CrossProduct(ToTarget, FVector::UpVector);

    // Offset to the side of the target
    FVector FlankOffset = Right * (Stats.AttackRadius * 2.0f);
    FVector FlankPos = Target->GetActorLocation() + FlankOffset;

    return FlankPos;
}
