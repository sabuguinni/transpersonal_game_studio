// CombatAIComponent.cpp
// Combat & Enemy AI Agent #12 — Transpersonal Game Studio
// Implements dinosaur combat AI: threat detection, state machine, attack execution

#include "CombatAIComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "TimerManager.h"

UCombatAIComponent::UCombatAIComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz AI tick

    // Default combat parameters
    CurrentState = ECombat_AIState::Idle;
    Species = ECombat_DinoSpecies::Raptor;
    DetectionRadius = 1500.0f;
    AttackRadius = 200.0f;
    ChaseSpeed = 600.0f;
    PatrolSpeed = 200.0f;
    MaxHealth = 300.0f;
    CurrentHealth = 300.0f;
    AttackDamage = 35.0f;
    AttackCooldown = 1.5f;
    bIsPackHunter = false;
    PackFlankingAngle = 60.0f;
    ThreatMemoryDuration = 30.0f;
    bCanRetreat = true;
    RetreatHealthThreshold = 0.25f;
    LastAttackTime = -999.0f;
}

void UCombatAIComponent::BeginPlay()
{
    Super::BeginPlay();

    // Apply species-specific defaults
    ApplySpeciesDefaults();

    // Start patrol loop
    GetWorld()->GetTimerManager().SetTimer(
        PatrolTimerHandle,
        this,
        &UCombatAIComponent::UpdatePatrolPoint,
        5.0f,
        true
    );

    // Start threat scan loop
    GetWorld()->GetTimerManager().SetTimer(
        ThreatScanTimerHandle,
        this,
        &UCombatAIComponent::ScanForThreats,
        0.5f,
        true
    );
}

void UCombatAIComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    UpdateThreatMemory(DeltaTime);
    UpdateStateMachine(DeltaTime);
}

void UCombatAIComponent::ApplySpeciesDefaults()
{
    switch (Species)
    {
        case ECombat_DinoSpecies::TRex:
            DetectionRadius = 2500.0f;
            AttackRadius = 350.0f;
            ChaseSpeed = 750.0f;
            PatrolSpeed = 150.0f;
            MaxHealth = 1500.0f;
            CurrentHealth = 1500.0f;
            AttackDamage = 120.0f;
            AttackCooldown = 2.5f;
            bIsPackHunter = false;
            bCanRetreat = false; // T-Rex never retreats
            break;

        case ECombat_DinoSpecies::Raptor:
            DetectionRadius = 1200.0f;
            AttackRadius = 180.0f;
            ChaseSpeed = 800.0f;
            PatrolSpeed = 300.0f;
            MaxHealth = 200.0f;
            CurrentHealth = 200.0f;
            AttackDamage = 30.0f;
            AttackCooldown = 0.8f;
            bIsPackHunter = true;
            PackFlankingAngle = 65.0f;
            bCanRetreat = true;
            RetreatHealthThreshold = 0.3f;
            break;

        case ECombat_DinoSpecies::Triceratops:
            DetectionRadius = 800.0f;
            AttackRadius = 280.0f;
            ChaseSpeed = 500.0f;
            PatrolSpeed = 120.0f;
            MaxHealth = 800.0f;
            CurrentHealth = 800.0f;
            AttackDamage = 60.0f;
            AttackCooldown = 3.0f;
            bIsPackHunter = false;
            bCanRetreat = false; // Territorial — stands ground
            break;

        case ECombat_DinoSpecies::Pterodactyl:
            DetectionRadius = 3000.0f;
            AttackRadius = 150.0f;
            ChaseSpeed = 1200.0f;
            PatrolSpeed = 400.0f;
            MaxHealth = 120.0f;
            CurrentHealth = 120.0f;
            AttackDamage = 20.0f;
            AttackCooldown = 1.2f;
            bIsPackHunter = false;
            bCanRetreat = true;
            RetreatHealthThreshold = 0.4f;
            break;

        case ECombat_DinoSpecies::Brachiosaurus:
            DetectionRadius = 600.0f;
            AttackRadius = 400.0f;
            ChaseSpeed = 300.0f;
            PatrolSpeed = 80.0f;
            MaxHealth = 3000.0f;
            CurrentHealth = 3000.0f;
            AttackDamage = 80.0f;
            AttackCooldown = 4.0f;
            bIsPackHunter = false;
            bCanRetreat = false;
            break;
    }
}

void UCombatAIComponent::ScanForThreats()
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    UWorld* World = GetWorld();
    if (!World) return;

    // Get all pawns in detection radius
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, APawn::StaticClass(), FoundActors);

    for (AActor* Actor : FoundActors)
    {
        if (Actor == Owner) continue;

        float Distance = FVector::Dist(Owner->GetActorLocation(), Actor->GetActorLocation());
        if (Distance <= DetectionRadius)
        {
            float ThreatLevel = CalculateThreatLevel(Actor, Distance);
            RegisterThreat(Actor, ThreatLevel);
        }
    }
}

float UCombatAIComponent::CalculateThreatLevel(AActor* ThreatActor, float Distance) const
{
    if (!ThreatActor) return 0.0f;

    // Base threat from distance (closer = higher threat)
    float DistanceFactor = 1.0f - FMath::Clamp(Distance / DetectionRadius, 0.0f, 1.0f);
    float BaseThreat = DistanceFactor * 100.0f;

    // Increase threat if actor is moving toward us
    AActor* Owner = GetOwner();
    if (Owner)
    {
        FVector ToThreat = (Owner->GetActorLocation() - ThreatActor->GetActorLocation()).GetSafeNormal();
        FVector ThreatVelocity = ThreatActor->GetVelocity().GetSafeNormal();
        float ApproachDot = FVector::DotProduct(ThreatVelocity, ToThreat);
        if (ApproachDot > 0.5f)
        {
            BaseThreat *= 1.5f; // 50% more threatening if approaching
        }
    }

    return FMath::Clamp(BaseThreat, 0.0f, 100.0f);
}

void UCombatAIComponent::RegisterThreat(AActor* ThreatActor, float ThreatLevel)
{
    if (!ThreatActor) return;

    // Check if already in threat list
    for (FCombat_ThreatEntry& Entry : ThreatList)
    {
        if (Entry.ThreatActor == ThreatActor)
        {
            Entry.ThreatLevel = ThreatLevel;
            Entry.LastSeenTime = GetWorld()->GetTimeSeconds();
            Entry.LastKnownLocation = ThreatActor->GetActorLocation();
            return;
        }
    }

    // New threat
    FCombat_ThreatEntry NewEntry;
    NewEntry.ThreatActor = ThreatActor;
    NewEntry.ThreatLevel = ThreatLevel;
    NewEntry.LastSeenTime = GetWorld()->GetTimeSeconds();
    NewEntry.LastKnownLocation = ThreatActor->GetActorLocation();
    ThreatList.Add(NewEntry);

    // Transition to alert if idle/patrolling
    if (CurrentState == ECombat_AIState::Idle || CurrentState == ECombat_AIState::Patrol)
    {
        TransitionToState(ECombat_AIState::Alert);
    }
}

void UCombatAIComponent::UpdateThreatMemory(float DeltaTime)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();

    // Remove expired threats
    ThreatList.RemoveAll([&](const FCombat_ThreatEntry& Entry) {
        return (CurrentTime - Entry.LastSeenTime) > ThreatMemoryDuration;
    });

    // If no threats remain, return to patrol
    if (ThreatList.Num() == 0 && CurrentState == ECombat_AIState::Alert)
    {
        TransitionToState(ECombat_AIState::Patrol);
    }
}

AActor* UCombatAIComponent::GetHighestThreat() const
{
    AActor* HighestThreatActor = nullptr;
    float HighestLevel = 0.0f;

    for (const FCombat_ThreatEntry& Entry : ThreatList)
    {
        if (Entry.ThreatLevel > HighestLevel && Entry.ThreatActor)
        {
            HighestLevel = Entry.ThreatLevel;
            HighestThreatActor = Entry.ThreatActor;
        }
    }

    return HighestThreatActor;
}

void UCombatAIComponent::UpdateStateMachine(float DeltaTime)
{
    AActor* PrimaryTarget = GetHighestThreat();

    switch (CurrentState)
    {
        case ECombat_AIState::Idle:
            // Do nothing — wait for ScanForThreats to trigger
            break;

        case ECombat_AIState::Patrol:
            ExecutePatrol(DeltaTime);
            if (PrimaryTarget)
            {
                float Dist = FVector::Dist(GetOwner()->GetActorLocation(), PrimaryTarget->GetActorLocation());
                if (Dist < DetectionRadius * 0.6f)
                {
                    TransitionToState(ECombat_AIState::Chase);
                }
            }
            break;

        case ECombat_AIState::Alert:
            // Look around, sniff — short alert window before chase
            if (PrimaryTarget)
            {
                TransitionToState(ECombat_AIState::Chase);
            }
            break;

        case ECombat_AIState::Chase:
            ExecuteChase(DeltaTime, PrimaryTarget);
            if (PrimaryTarget)
            {
                float Dist = FVector::Dist(GetOwner()->GetActorLocation(), PrimaryTarget->GetActorLocation());
                if (Dist <= AttackRadius)
                {
                    TransitionToState(ECombat_AIState::Attack);
                }
            }
            else
            {
                TransitionToState(ECombat_AIState::Patrol);
            }
            break;

        case ECombat_AIState::Attack:
            ExecuteAttack(DeltaTime, PrimaryTarget);
            if (!PrimaryTarget)
            {
                TransitionToState(ECombat_AIState::Patrol);
            }
            else if (bCanRetreat && (CurrentHealth / MaxHealth) < RetreatHealthThreshold)
            {
                TransitionToState(ECombat_AIState::Retreat);
            }
            break;

        case ECombat_AIState::Flanking:
            ExecuteFlanking(DeltaTime, PrimaryTarget);
            break;

        case ECombat_AIState::Retreat:
            ExecuteRetreat(DeltaTime);
            break;

        case ECombat_AIState::Dead:
            // No updates
            break;
    }
}

void UCombatAIComponent::ExecutePatrol(float DeltaTime)
{
    // Movement handled by AI Controller — we just set the destination
    // Actual movement via AIController->MoveToLocation in Blueprint
}

void UCombatAIComponent::ExecuteChase(float DeltaTime, AActor* Target)
{
    if (!Target) return;
    // Chase target — AI controller handles pathfinding
    // We broadcast the target location for the AI controller to pick up
    OnCombatStateChanged.Broadcast(ECombat_AIState::Chase);
}

void UCombatAIComponent::ExecuteAttack(float DeltaTime, AActor* Target)
{
    if (!Target) return;

    float CurrentTime = GetWorld()->GetTimeSeconds();
    if ((CurrentTime - LastAttackTime) < AttackCooldown) return;

    LastAttackTime = CurrentTime;

    // Calculate attack data
    FCombat_AttackData AttackData;
    AttackData.AttackType = ECombat_AttackType::Bite;
    AttackData.Damage = AttackDamage;
    AttackData.AttackRadius = AttackRadius;
    AttackData.ImpactLocation = Target->GetActorLocation();
    AttackData.bIsLethal = false;
    AttackData.KnockbackForce = 500.0f;

    // Apply damage
    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Target->GetActorLocation());
    if (Distance <= AttackRadius)
    {
        UGameplayStatics::ApplyDamage(Target, AttackData.Damage, nullptr, GetOwner(), nullptr);
        OnAttackExecuted.Broadcast(AttackData);
    }
}

void UCombatAIComponent::ExecuteFlanking(float DeltaTime, AActor* Target)
{
    if (!Target || !bIsPackHunter) return;

    // Flanking position is offset from target by PackFlankingAngle
    // This is set externally by the pack coordinator
    OnCombatStateChanged.Broadcast(ECombat_AIState::Flanking);
}

void UCombatAIComponent::ExecuteRetreat(float DeltaTime)
{
    // Move away from all threats
    OnCombatStateChanged.Broadcast(ECombat_AIState::Retreat);
}

void UCombatAIComponent::TransitionToState(ECombat_AIState NewState)
{
    if (CurrentState == NewState) return;
    if (CurrentState == ECombat_AIState::Dead) return;

    ECombat_AIState OldState = CurrentState;
    CurrentState = NewState;

    OnCombatStateChanged.Broadcast(NewState);

    UE_LOG(LogTemp, Log, TEXT("[CombatAI] %s: %d -> %d"),
        *GetOwner()->GetName(),
        (int32)OldState,
        (int32)NewState);
}

void UCombatAIComponent::TakeCombatDamage(float DamageAmount, AActor* DamageSource)
{
    CurrentHealth = FMath::Max(0.0f, CurrentHealth - DamageAmount);

    if (DamageSource)
    {
        RegisterThreat(DamageSource, 100.0f); // Max threat from attacker
    }

    if (CurrentHealth <= 0.0f)
    {
        TransitionToState(ECombat_AIState::Dead);
    }
    else if (bCanRetreat && (CurrentHealth / MaxHealth) < RetreatHealthThreshold)
    {
        TransitionToState(ECombat_AIState::Retreat);
    }
}

void UCombatAIComponent::UpdatePatrolPoint()
{
    if (CurrentState != ECombat_AIState::Patrol && CurrentState != ECombat_AIState::Idle) return;

    AActor* Owner = GetOwner();
    if (!Owner) return;

    // Generate random patrol point within 1000 units
    FVector Origin = Owner->GetActorLocation();
    FVector RandomOffset = FVector(
        FMath::RandRange(-1000.0f, 1000.0f),
        FMath::RandRange(-1000.0f, 1000.0f),
        0.0f
    );

    CurrentPatrolTarget = Origin + RandomOffset;
}

FVector UCombatAIComponent::CalculateFlankingPosition(AActor* Target, int32 FlankIndex, int32 TotalFlankers) const
{
    if (!Target) return FVector::ZeroVector;

    AActor* Owner = GetOwner();
    if (!Owner) return FVector::ZeroVector;

    // Distribute flankers evenly around the target
    float AngleStep = 360.0f / FMath::Max(TotalFlankers, 1);
    float MyAngle = FMath::DegreesToRadians(FlankIndex * AngleStep);

    FVector TargetLoc = Target->GetActorLocation();
    float FlankRadius = AttackRadius * 2.5f;

    return TargetLoc + FVector(
        FMath::Cos(MyAngle) * FlankRadius,
        FMath::Sin(MyAngle) * FlankRadius,
        0.0f
    );
}

ECombat_AIState UCombatAIComponent::GetCurrentState() const
{
    return CurrentState;
}

float UCombatAIComponent::GetHealthPercent() const
{
    if (MaxHealth <= 0.0f) return 0.0f;
    return CurrentHealth / MaxHealth;
}

bool UCombatAIComponent::IsHostile() const
{
    return CurrentState != ECombat_AIState::Idle &&
           CurrentState != ECombat_AIState::Patrol &&
           CurrentState != ECombat_AIState::Dead;
}
