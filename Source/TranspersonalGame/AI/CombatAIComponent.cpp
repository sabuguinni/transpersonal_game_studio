// CombatAIComponent.cpp
// Combat & Enemy AI Agent #12 — Transpersonal Game Studio
// Implements tactical AI state machine for dinosaur enemies

#include "CombatAIComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"

UCombatAIComponent::UCombatAIComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz tick for AI — performance friendly
}

void UCombatAIComponent::BeginPlay()
{
    Super::BeginPlay();
    ApplySpeciesDefaults();
    SetAIState(ECombat_AIState::Patrol);
}

void UCombatAIComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    StateTimer += DeltaTime;
    ScanForThreats();

    switch (CurrentState)
    {
        case ECombat_AIState::Idle:      UpdateIdleState(DeltaTime);     break;
        case ECombat_AIState::Patrol:    UpdatePatrolState(DeltaTime);   break;
        case ECombat_AIState::Alert:     UpdateAlertState(DeltaTime);    break;
        case ECombat_AIState::Chase:     UpdateChaseState(DeltaTime);    break;
        case ECombat_AIState::Attack:    UpdateAttackState(DeltaTime);   break;
        case ECombat_AIState::Flanking:  UpdateFlankingState(DeltaTime); break;
        case ECombat_AIState::Retreat:   UpdateRetreatState(DeltaTime);  break;
        default: break;
    }
}

void UCombatAIComponent::SetAIState(ECombat_AIState NewState)
{
    if (CurrentState == NewState) return;
    CurrentState = NewState;
    StateTimer = 0.0f;
    UE_LOG(LogTemp, Log, TEXT("CombatAI [%s]: State -> %d"), *GetOwner()->GetName(), (int32)NewState);
}

void UCombatAIComponent::RegisterThreat(AActor* ThreatActor, float ThreatLevel)
{
    if (!ThreatActor) return;

    for (FCombat_ThreatEntry& Entry : ThreatList)
    {
        if (Entry.ThreatActor == ThreatActor)
        {
            Entry.ThreatLevel = FMath::Max(Entry.ThreatLevel, ThreatLevel);
            Entry.LastSeenTime = GetWorld()->GetTimeSeconds();
            Entry.LastKnownLocation = ThreatActor->GetActorLocation();
            return;
        }
    }

    FCombat_ThreatEntry NewEntry;
    NewEntry.ThreatActor = ThreatActor;
    NewEntry.ThreatLevel = ThreatLevel;
    NewEntry.LastSeenTime = GetWorld()->GetTimeSeconds();
    NewEntry.LastKnownLocation = ThreatActor->GetActorLocation();
    ThreatList.Add(NewEntry);

    if (CurrentState == ECombat_AIState::Idle || CurrentState == ECombat_AIState::Patrol)
    {
        SetAIState(ECombat_AIState::Alert);
    }
}

void UCombatAIComponent::ClearThreat(AActor* ThreatActor)
{
    ThreatList.RemoveAll([ThreatActor](const FCombat_ThreatEntry& E) {
        return E.ThreatActor == ThreatActor;
    });
}

AActor* UCombatAIComponent::GetHighestThreat() const
{
    AActor* Best = nullptr;
    float BestLevel = -1.0f;
    for (const FCombat_ThreatEntry& Entry : ThreatList)
    {
        if (Entry.ThreatLevel > BestLevel && Entry.ThreatActor != nullptr)
        {
            BestLevel = Entry.ThreatLevel;
            Best = Entry.ThreatActor;
        }
    }
    return Best;
}

bool UCombatAIComponent::CanAttack() const
{
    float Now = GetWorld()->GetTimeSeconds();
    return (Now - LastAttackTime) >= AttackData.AttackCooldown;
}

void UCombatAIComponent::ExecuteAttack(AActor* Target)
{
    if (!Target || !CanAttack()) return;

    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Target->GetActorLocation());
    if (Distance > AttackData.AttackRange) return;

    LastAttackTime = GetWorld()->GetTimeSeconds();

    // Apply damage via UE5 damage system
    UGameplayStatics::ApplyDamage(
        Target,
        AttackData.BaseDamage * AggressionLevel,
        nullptr,
        GetOwner(),
        UDamageType::StaticClass()
    );

    UE_LOG(LogTemp, Log, TEXT("CombatAI [%s]: Attack on %s — %.1f damage"),
        *GetOwner()->GetName(), *Target->GetName(), AttackData.BaseDamage * AggressionLevel);
}

void UCombatAIComponent::SetFlankingTarget(AActor* Target, int32 FlankIndex)
{
    CurrentTarget = Target;
    PackFlankIndex = FlankIndex;
    bIsPackMember = true;
    SetAIState(ECombat_AIState::Flanking);
}

FVector UCombatAIComponent::CalculateFlankPosition(AActor* Target, int32 FlankIndex) const
{
    if (!Target) return FVector::ZeroVector;

    FVector TargetLoc = Target->GetActorLocation();
    FVector TargetFwd = Target->GetActorForwardVector();
    FVector TargetRight = Target->GetActorRightVector();

    // 3-raptor flanking pattern: front, left-rear, right-rear
    const float FlankRadius = 400.0f;
    switch (FlankIndex % 3)
    {
        case 0: return TargetLoc + TargetFwd * FlankRadius;                          // Drive from front
        case 1: return TargetLoc - TargetFwd * FlankRadius * 0.5f - TargetRight * FlankRadius; // Left rear
        case 2: return TargetLoc - TargetFwd * FlankRadius * 0.5f + TargetRight * FlankRadius; // Right rear
        default: return TargetLoc;
    }
}

bool UCombatAIComponent::CanSeeActor(AActor* Target) const
{
    if (!Target || !GetWorld()) return false;

    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Target->GetActorLocation());
    if (Distance > DetectionRadius) return false;

    // Line of sight check
    FHitResult HitResult;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(GetOwner());

    bool bBlocked = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        GetOwner()->GetActorLocation() + FVector(0, 0, 50),
        Target->GetActorLocation() + FVector(0, 0, 50),
        ECC_Visibility,
        Params
    );

    return !bBlocked || HitResult.GetActor() == Target;
}

bool UCombatAIComponent::CanHearActor(AActor* Target) const
{
    if (!Target) return false;
    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Target->GetActorLocation());
    return Distance <= HearingRadius;
}

// --- Private State Handlers ---

void UCombatAIComponent::UpdateIdleState(float DeltaTime)
{
    if (StateTimer > 5.0f)
    {
        SetAIState(ECombat_AIState::Patrol);
    }
}

void UCombatAIComponent::UpdatePatrolState(float DeltaTime)
{
    // Patrol logic handled by BehaviorTree — this just monitors for threats
    AActor* Threat = GetHighestThreat();
    if (Threat)
    {
        CurrentTarget = Threat;
        SetAIState(ECombat_AIState::Alert);
    }
}

void UCombatAIComponent::UpdateAlertState(float DeltaTime)
{
    if (!CurrentTarget) { SetAIState(ECombat_AIState::Patrol); return; }

    if (CanSeeActor(CurrentTarget))
    {
        if (AggressionLevel >= 0.7f)
            SetAIState(bIsPackMember ? ECombat_AIState::Flanking : ECombat_AIState::Chase);
        else
            SetAIState(ECombat_AIState::Chase);
    }
    else if (StateTimer > 10.0f)
    {
        SetAIState(ECombat_AIState::Patrol);
    }
}

void UCombatAIComponent::UpdateChaseState(float DeltaTime)
{
    if (!CurrentTarget) { SetAIState(ECombat_AIState::Patrol); return; }

    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), CurrentTarget->GetActorLocation());

    if (Distance <= AttackData.AttackRange)
    {
        SetAIState(ECombat_AIState::Attack);
    }
    else if (Distance > DetectionRadius * 1.5f)
    {
        // Lost the target
        CurrentTarget = nullptr;
        SetAIState(ECombat_AIState::Patrol);
    }
}

void UCombatAIComponent::UpdateAttackState(float DeltaTime)
{
    if (!CurrentTarget) { SetAIState(ECombat_AIState::Patrol); return; }

    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), CurrentTarget->GetActorLocation());

    if (Distance > AttackData.AttackRange * 1.2f)
    {
        SetAIState(ECombat_AIState::Chase);
        return;
    }

    ExecuteAttack(CurrentTarget);
}

void UCombatAIComponent::UpdateFlankingState(float DeltaTime)
{
    if (!CurrentTarget) { SetAIState(ECombat_AIState::Patrol); return; }

    // Move toward flank position — actual movement handled by AIController/BT
    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), CurrentTarget->GetActorLocation());
    if (Distance <= AttackData.AttackRange)
    {
        SetAIState(ECombat_AIState::Attack);
    }
}

void UCombatAIComponent::UpdateRetreatState(float DeltaTime)
{
    if (StateTimer > 8.0f)
    {
        AggressionLevel = FMath::Min(AggressionLevel + 0.1f, 1.0f); // Recover aggression
        SetAIState(ECombat_AIState::Patrol);
    }
}

void UCombatAIComponent::ScanForThreats()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Find player character
    ACharacter* Player = UGameplayStatics::GetPlayerCharacter(World, 0);
    if (!Player) return;

    if (CanSeeActor(Player))
    {
        RegisterThreat(Player, 1.0f);
    }
    else if (CanHearActor(Player))
    {
        RegisterThreat(Player, 0.5f);
    }

    // Expire old threats (not seen for 30 seconds)
    float Now = World->GetTimeSeconds();
    ThreatList.RemoveAll([Now](const FCombat_ThreatEntry& E) {
        return (Now - E.LastSeenTime) > 30.0f;
    });
}

void UCombatAIComponent::ApplySpeciesDefaults()
{
    switch (Species)
    {
        case ECombat_DinoSpecies::TRex:
            DetectionRadius = 2500.0f;
            HearingRadius = 3000.0f;
            ChaseSpeed = 700.0f;
            PatrolSpeed = 150.0f;
            AttackData.BaseDamage = 80.0f;
            AttackData.AttackRange = 350.0f;
            AttackData.AttackCooldown = 3.0f;
            AttackData.KnockbackForce = 1200.0f;
            AggressionLevel = 0.9f;
            bIsPackMember = false;
            break;

        case ECombat_DinoSpecies::Raptor:
            DetectionRadius = 1500.0f;
            HearingRadius = 2000.0f;
            ChaseSpeed = 800.0f;
            PatrolSpeed = 300.0f;
            AttackData.BaseDamage = 30.0f;
            AttackData.AttackRange = 150.0f;
            AttackData.AttackCooldown = 1.2f;
            AttackData.KnockbackForce = 400.0f;
            AggressionLevel = 0.75f;
            bIsPackMember = true;
            break;

        case ECombat_DinoSpecies::Triceratops:
            DetectionRadius = 1000.0f;
            HearingRadius = 1200.0f;
            ChaseSpeed = 500.0f;
            PatrolSpeed = 180.0f;
            AttackData.BaseDamage = 60.0f;
            AttackData.AttackRange = 280.0f;
            AttackData.AttackCooldown = 2.5f;
            AttackData.KnockbackForce = 900.0f;
            AggressionLevel = 0.4f; // Defensive, not aggressive by default
            bIsPackMember = false;
            break;

        case ECombat_DinoSpecies::Brachiosaurus:
            DetectionRadius = 800.0f;
            HearingRadius = 1000.0f;
            ChaseSpeed = 300.0f;
            PatrolSpeed = 100.0f;
            AttackData.BaseDamage = 40.0f;
            AttackData.AttackRange = 400.0f; // Tail sweep
            AttackData.AttackCooldown = 4.0f;
            AttackData.KnockbackForce = 1500.0f;
            AggressionLevel = 0.2f; // Passive unless provoked
            bIsPackMember = false;
            break;

        case ECombat_DinoSpecies::Pterodactyl:
            DetectionRadius = 3000.0f;
            HearingRadius = 1500.0f;
            ChaseSpeed = 1200.0f;
            PatrolSpeed = 600.0f;
            AttackData.BaseDamage = 20.0f;
            AttackData.AttackRange = 200.0f;
            AttackData.AttackCooldown = 1.5f;
            AttackData.KnockbackForce = 200.0f;
            AggressionLevel = 0.6f;
            bIsPackMember = false;
            break;

        default:
            break;
    }
}
