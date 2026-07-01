// CombatAIComponent.cpp
// Agent #12 — Combat & Enemy AI Agent
// Implements tactical combat AI for dinosaur enemies: state machine, threat detection, flanking, species profiles

#include "CombatAIComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

UCombatAIComponent::UCombatAIComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10 Hz tick for performance

    // Default species profile — T-Rex
    CurrentState = ECombat_AIState::Idle;
    CurrentSpecies = ECombat_DinoSpecies::TRex;
    ThreatDetectionRadius = 1500.0f;
    AttackRange = 250.0f;
    FleeHealthThreshold = 0.15f;
    bCanFlanking = false;
    bIsPackHunter = false;
    CurrentHealth = 1000.0f;
    MaxHealth = 1000.0f;
    AttackDamage = 120.0f;
    AttackCooldown = 2.5f;
    bAttackOnCooldown = false;
    PatrolRadius = 800.0f;
    ChaseSpeedMultiplier = 1.4f;
    AlertDuration = 5.0f;
}

void UCombatAIComponent::BeginPlay()
{
    Super::BeginPlay();
    ApplySpeciesProfile(CurrentSpecies);
    SetState(ECombat_AIState::Patrol);

    // Start patrol timer
    GetWorld()->GetTimerManager().SetTimer(
        PatrolTimerHandle,
        this,
        &UCombatAIComponent::OnPatrolTimerExpired,
        3.0f,
        true
    );
}

void UCombatAIComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    UpdateThreatList();
    UpdateStateMachine(DeltaTime);

#if WITH_EDITOR
    // Debug visualization in editor
    if (CurrentState != ECombat_AIState::Idle && CurrentState != ECombat_AIState::Dead)
    {
        AActor* Owner = GetOwner();
        if (Owner)
        {
            DrawDebugSphere(GetWorld(), Owner->GetActorLocation(), ThreatDetectionRadius, 16,
                CurrentState == ECombat_AIState::Attack ? FColor::Red : FColor::Yellow, false, 0.15f);
        }
    }
#endif
}

void UCombatAIComponent::ApplySpeciesProfile(ECombat_DinoSpecies Species)
{
    CurrentSpecies = Species;

    switch (Species)
    {
    case ECombat_DinoSpecies::TRex:
        ThreatDetectionRadius = 2000.0f;
        AttackRange = 300.0f;
        AttackDamage = 150.0f;
        AttackCooldown = 3.0f;
        MaxHealth = 1500.0f;
        CurrentHealth = MaxHealth;
        bCanFlanking = false;
        bIsPackHunter = false;
        ChaseSpeedMultiplier = 1.2f;
        FleeHealthThreshold = 0.05f; // T-Rex almost never flees
        break;

    case ECombat_DinoSpecies::Velociraptor:
        ThreatDetectionRadius = 1200.0f;
        AttackRange = 150.0f;
        AttackDamage = 45.0f;
        AttackCooldown = 0.8f;
        MaxHealth = 280.0f;
        CurrentHealth = MaxHealth;
        bCanFlanking = true;
        bIsPackHunter = true;
        ChaseSpeedMultiplier = 1.8f;
        FleeHealthThreshold = 0.25f;
        break;

    case ECombat_DinoSpecies::Triceratops:
        ThreatDetectionRadius = 800.0f;
        AttackRange = 200.0f;
        AttackDamage = 80.0f;
        AttackCooldown = 2.0f;
        MaxHealth = 900.0f;
        CurrentHealth = MaxHealth;
        bCanFlanking = false;
        bIsPackHunter = false;
        ChaseSpeedMultiplier = 1.1f;
        FleeHealthThreshold = 0.10f;
        break;

    case ECombat_DinoSpecies::Spinosaurus:
        ThreatDetectionRadius = 1800.0f;
        AttackRange = 280.0f;
        AttackDamage = 130.0f;
        AttackCooldown = 2.8f;
        MaxHealth = 1200.0f;
        CurrentHealth = MaxHealth;
        bCanFlanking = false;
        bIsPackHunter = false;
        ChaseSpeedMultiplier = 1.3f;
        FleeHealthThreshold = 0.08f;
        break;

    case ECombat_DinoSpecies::Pterodactyl:
        ThreatDetectionRadius = 2500.0f; // Aerial detection
        AttackRange = 180.0f;
        AttackDamage = 55.0f;
        AttackCooldown = 1.5f;
        MaxHealth = 320.0f;
        CurrentHealth = MaxHealth;
        bCanFlanking = true;
        bIsPackHunter = false;
        ChaseSpeedMultiplier = 2.0f;
        FleeHealthThreshold = 0.30f;
        break;
    }
}

void UCombatAIComponent::SetState(ECombat_AIState NewState)
{
    if (CurrentState == NewState) return;
    if (CurrentState == ECombat_AIState::Dead) return;

    ECombat_AIState OldState = CurrentState;
    CurrentState = NewState;

    OnStateChanged.Broadcast(OldState, NewState);
    UE_LOG(LogTemp, Log, TEXT("CombatAI [%s]: %d -> %d"),
        *GetOwner()->GetName(), (int32)OldState, (int32)NewState);
}

void UCombatAIComponent::UpdateThreatList()
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    FVector OwnerLoc = Owner->GetActorLocation();
    float Now = GetWorld()->GetTimeSeconds();

    // Remove stale threats (not seen in 10 seconds)
    ThreatList.RemoveAll([Now](const FCombat_ThreatEntry& Entry) {
        return (Now - Entry.LastSeenTime) > 10.0f;
    });

    // Scan for new threats (player and other pawns)
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), FoundActors);

    for (AActor* Actor : FoundActors)
    {
        if (Actor == Owner) continue;

        float Dist = FVector::Dist(OwnerLoc, Actor->GetActorLocation());
        if (Dist <= ThreatDetectionRadius)
        {
            // Update or add threat
            bool bFound = false;
            for (FCombat_ThreatEntry& Entry : ThreatList)
            {
                if (Entry.ThreatActor == Actor)
                {
                    Entry.LastSeenTime = Now;
                    Entry.LastKnownLocation = Actor->GetActorLocation();
                    Entry.ThreatLevel = FMath::Clamp(1.0f - (Dist / ThreatDetectionRadius), 0.0f, 1.0f);
                    bFound = true;
                    break;
                }
            }

            if (!bFound)
            {
                FCombat_ThreatEntry NewThreat;
                NewThreat.ThreatActor = Actor;
                NewThreat.ThreatLevel = FMath::Clamp(1.0f - (Dist / ThreatDetectionRadius), 0.0f, 1.0f);
                NewThreat.LastSeenTime = Now;
                NewThreat.LastKnownLocation = Actor->GetActorLocation();
                ThreatList.Add(NewThreat);
                OnThreatDetected.Broadcast(Actor, NewThreat.ThreatLevel);
            }
        }
    }
}

void UCombatAIComponent::UpdateStateMachine(float DeltaTime)
{
    if (CurrentState == ECombat_AIState::Dead) return;

    // Health-based flee check
    float HealthPct = CurrentHealth / MaxHealth;
    if (HealthPct <= FleeHealthThreshold && CurrentState != ECombat_AIState::Retreat)
    {
        SetState(ECombat_AIState::Retreat);
        return;
    }

    // Get highest threat
    FCombat_ThreatEntry* HighestThreat = GetHighestThreat();

    switch (CurrentState)
    {
    case ECombat_AIState::Idle:
    case ECombat_AIState::Patrol:
        if (HighestThreat)
        {
            SetState(ECombat_AIState::Alert);
            AlertTimer = AlertDuration;
        }
        break;

    case ECombat_AIState::Alert:
        AlertTimer -= DeltaTime;
        if (HighestThreat)
        {
            float Dist = HighestThreat->ThreatActor.IsValid() ?
                FVector::Dist(GetOwner()->GetActorLocation(), HighestThreat->ThreatActor->GetActorLocation()) : 9999.f;
            if (Dist <= AttackRange)
                SetState(ECombat_AIState::Attack);
            else
                SetState(ECombat_AIState::Chase);
        }
        else if (AlertTimer <= 0.f)
        {
            SetState(ECombat_AIState::Patrol);
        }
        break;

    case ECombat_AIState::Chase:
        if (!HighestThreat)
        {
            SetState(ECombat_AIState::Alert);
            AlertTimer = AlertDuration;
        }
        else
        {
            float Dist = HighestThreat->ThreatActor.IsValid() ?
                FVector::Dist(GetOwner()->GetActorLocation(), HighestThreat->ThreatActor->GetActorLocation()) : 9999.f;
            if (Dist <= AttackRange)
            {
                if (bCanFlanking && bIsPackHunter)
                    SetState(ECombat_AIState::Flanking);
                else
                    SetState(ECombat_AIState::Attack);
            }
        }
        break;

    case ECombat_AIState::Flanking:
        if (!HighestThreat)
        {
            SetState(ECombat_AIState::Patrol);
        }
        else
        {
            float Dist = HighestThreat->ThreatActor.IsValid() ?
                FVector::Dist(GetOwner()->GetActorLocation(), HighestThreat->ThreatActor->GetActorLocation()) : 9999.f;
            if (Dist <= AttackRange * 0.8f)
                SetState(ECombat_AIState::Attack);
        }
        break;

    case ECombat_AIState::Attack:
        if (!bAttackOnCooldown && HighestThreat && HighestThreat->ThreatActor.IsValid())
        {
            ExecuteAttack(HighestThreat->ThreatActor.Get());
        }
        if (!HighestThreat)
        {
            SetState(ECombat_AIState::Patrol);
        }
        break;

    case ECombat_AIState::Retreat:
        // Move away from all threats — handled by AI controller
        if (!HighestThreat && HealthPct > FleeHealthThreshold + 0.1f)
        {
            SetState(ECombat_AIState::Patrol);
        }
        break;

    default:
        break;
    }
}

void UCombatAIComponent::ExecuteAttack(AActor* Target)
{
    if (!Target || bAttackOnCooldown) return;

    // Apply damage
    UGameplayStatics::ApplyDamage(Target, AttackDamage, nullptr, GetOwner(), nullptr);
    OnAttackExecuted.Broadcast(Target, AttackDamage);

    UE_LOG(LogTemp, Log, TEXT("CombatAI [%s]: Attack on %s for %.1f dmg"),
        *GetOwner()->GetName(), *Target->GetName(), AttackDamage);

    // Start cooldown
    bAttackOnCooldown = true;
    GetWorld()->GetTimerManager().SetTimer(
        AttackCooldownHandle,
        [this]() { bAttackOnCooldown = false; },
        AttackCooldown,
        false
    );
}

void UCombatAIComponent::TakeCombatDamage(float Damage, AActor* DamageSource)
{
    if (CurrentState == ECombat_AIState::Dead) return;

    CurrentHealth = FMath::Clamp(CurrentHealth - Damage, 0.0f, MaxHealth);
    OnDamageTaken.Broadcast(Damage, DamageSource);

    if (CurrentHealth <= 0.0f)
    {
        SetState(ECombat_AIState::Dead);
        OnDeath.Broadcast();
        UE_LOG(LogTemp, Log, TEXT("CombatAI [%s]: DEAD"), *GetOwner()->GetName());
    }
    else if (CurrentHealth / MaxHealth <= FleeHealthThreshold)
    {
        SetState(ECombat_AIState::Retreat);
    }
    else if (DamageSource && CurrentState == ECombat_AIState::Patrol)
    {
        // Immediately alert on taking damage
        SetState(ECombat_AIState::Alert);
        AlertTimer = AlertDuration;
    }
}

FCombat_ThreatEntry* UCombatAIComponent::GetHighestThreat()
{
    FCombat_ThreatEntry* Best = nullptr;
    float BestLevel = 0.0f;

    for (FCombat_ThreatEntry& Entry : ThreatList)
    {
        if (Entry.ThreatActor.IsValid() && Entry.ThreatLevel > BestLevel)
        {
            BestLevel = Entry.ThreatLevel;
            Best = &Entry;
        }
    }
    return Best;
}

void UCombatAIComponent::OnPatrolTimerExpired()
{
    if (CurrentState != ECombat_AIState::Patrol) return;

    AActor* Owner = GetOwner();
    if (!Owner) return;

    // Pick random patrol point within radius
    FVector Origin = Owner->GetActorLocation();
    FVector RandomDir = FMath::VRand();
    RandomDir.Z = 0.0f;
    RandomDir.Normalize();
    FVector PatrolTarget = Origin + RandomDir * FMath::RandRange(200.0f, PatrolRadius);

    // Move via AI controller
    APawn* OwnerPawn = Cast<APawn>(Owner);
    if (OwnerPawn)
    {
        AAIController* AIC = Cast<AAIController>(OwnerPawn->GetController());
        if (AIC)
        {
            AIC->MoveToLocation(PatrolTarget, 50.0f);
        }
    }
}

float UCombatAIComponent::GetHealthPercent() const
{
    return MaxHealth > 0.0f ? CurrentHealth / MaxHealth : 0.0f;
}

bool UCombatAIComponent::IsInCombat() const
{
    return CurrentState == ECombat_AIState::Attack
        || CurrentState == ECombat_AIState::Chase
        || CurrentState == ECombat_AIState::Flanking
        || CurrentState == ECombat_AIState::Alert;
}
