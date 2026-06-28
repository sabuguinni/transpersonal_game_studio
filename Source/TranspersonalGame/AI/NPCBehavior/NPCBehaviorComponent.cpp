// NPCBehaviorComponent.cpp
// Agent #11 — NPC Behavior Agent
// Cycle: PROD_CYCLE_AUTO_20260628_009
//
// Implements NPC daily routine, alert state machine, memory system,
// and Behavior Tree integration for all non-dinosaur NPCs.

#include "NPCBehaviorComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "TimerManager.h"

// ─────────────────────────────────────────────────────────────────────────────
//  Construction / Lifecycle
// ─────────────────────────────────────────────────────────────────────────────

UNPCBehaviorComponent::UNPCBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10 Hz is sufficient for NPC AI

    // Defaults
    CurrentRoutinePhase = ENPC_DailyRoutinePhase::Patrolling;
    AlertLevel          = ENPC_AlertLevel::Unaware;
    bIsAlive            = true;
    bShowDebugState     = false;

    DetectionRadius     = 1500.0f;
    HearingRadius       = 800.0f;
    AttackRadius        = 250.0f;
    FleeHealthThreshold = 0.25f;

    PatrolSpeed         = 200.0f;
    ChaseSpeed          = 500.0f;
    FleeSpeed           = 600.0f;

    MemoryDecayTime     = 30.0f;
    AlertCooldownTime   = 15.0f;

    OwnerCharacter      = nullptr;
    AIController        = nullptr;
    BlackboardComp      = nullptr;
}

void UNPCBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();

    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (!OwnerCharacter)
    {
        UE_LOG(LogTemp, Warning, TEXT("NPCBehaviorComponent: Owner is not an ACharacter — component will be inactive."));
        SetComponentTickEnabled(false);
        return;
    }

    AIController = Cast<AAIController>(OwnerCharacter->GetController());

    if (AIController)
    {
        BlackboardComp = AIController->GetBlackboardComponent();
    }

    // Schedule first routine phase evaluation
    GetWorld()->GetTimerManager().SetTimer(
        RoutinePhaseTimerHandle,
        this,
        &UNPCBehaviorComponent::EvaluateRoutinePhase,
        5.0f,
        true
    );

    UE_LOG(LogTemp, Log, TEXT("NPCBehaviorComponent: Initialized on %s"), *GetOwner()->GetName());
}

void UNPCBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bIsAlive || !OwnerCharacter) return;

    // Decay memory entries
    TickMemoryDecay(DeltaTime);

    // Update alert level based on current stimuli
    UpdateAlertState(DeltaTime);

    // Debug visualization
    if (bShowDebugState)
    {
        DrawDebugSphere(GetWorld(), OwnerCharacter->GetActorLocation(), DetectionRadius, 16, FColor::Yellow, false, 0.15f);
        DrawDebugSphere(GetWorld(), OwnerCharacter->GetActorLocation(), HearingRadius, 12, FColor::Cyan, false, 0.15f);
        DrawDebugSphere(GetWorld(), OwnerCharacter->GetActorLocation(), AttackRadius, 8, FColor::Red, false, 0.15f);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  Alert State Machine
// ─────────────────────────────────────────────────────────────────────────────

void UNPCBehaviorComponent::SetAlertLevel(ENPC_AlertLevel NewLevel)
{
    if (NewLevel == AlertLevel) return;

    ENPC_AlertLevel PreviousLevel = AlertLevel;
    AlertLevel = NewLevel;

    // Propagate to Blackboard
    if (BlackboardComp)
    {
        BlackboardComp->SetValueAsEnum(FName("AlertLevel"), static_cast<uint8>(AlertLevel));
    }

    // Adjust movement speed based on alert level
    if (OwnerCharacter)
    {
        UCharacterMovementComponent* MoveComp = OwnerCharacter->GetCharacterMovement();
        if (MoveComp)
        {
            switch (AlertLevel)
            {
                case ENPC_AlertLevel::Unaware:
                case ENPC_AlertLevel::Curious:
                    MoveComp->MaxWalkSpeed = PatrolSpeed;
                    break;
                case ENPC_AlertLevel::Suspicious:
                case ENPC_AlertLevel::Alert:
                    MoveComp->MaxWalkSpeed = PatrolSpeed * 1.5f;
                    break;
                case ENPC_AlertLevel::Combat:
                    MoveComp->MaxWalkSpeed = ChaseSpeed;
                    break;
                case ENPC_AlertLevel::Fleeing:
                    MoveComp->MaxWalkSpeed = FleeSpeed;
                    break;
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("NPCBehaviorComponent [%s]: AlertLevel %d -> %d"),
        *GetOwner()->GetName(),
        static_cast<int32>(PreviousLevel),
        static_cast<int32>(AlertLevel));
}

void UNPCBehaviorComponent::UpdateAlertState(float DeltaTime)
{
    // Natural cooldown — alert level decreases over time if no stimuli
    if (AlertLevel > ENPC_AlertLevel::Unaware && MemoryEntries.Num() == 0)
    {
        AlertCooldownAccumulator += DeltaTime;
        if (AlertCooldownAccumulator >= AlertCooldownTime)
        {
            AlertCooldownAccumulator = 0.0f;
            int32 CurrentLevel = static_cast<int32>(AlertLevel);
            if (CurrentLevel > 0)
            {
                SetAlertLevel(static_cast<ENPC_AlertLevel>(CurrentLevel - 1));
            }
        }
    }
    else
    {
        AlertCooldownAccumulator = 0.0f;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  Memory System
// ─────────────────────────────────────────────────────────────────────────────

void UNPCBehaviorComponent::AddMemoryEntry(AActor* PerceivedActor, ENPC_MemoryType MemType, FVector LastKnownLocation)
{
    if (!PerceivedActor) return;

    // Check if we already have a memory for this actor
    for (FNPC_MemoryEntry& Entry : MemoryEntries)
    {
        if (Entry.PerceivedActor == PerceivedActor && Entry.MemoryType == MemType)
        {
            // Refresh existing entry
            Entry.LastKnownLocation = LastKnownLocation;
            Entry.TimeStamp         = GetWorld()->GetTimeSeconds();
            Entry.Confidence        = 1.0f;
            return;
        }
    }

    // Add new memory
    FNPC_MemoryEntry NewEntry;
    NewEntry.PerceivedActor     = PerceivedActor;
    NewEntry.MemoryType         = MemType;
    NewEntry.LastKnownLocation  = LastKnownLocation;
    NewEntry.TimeStamp          = GetWorld()->GetTimeSeconds();
    NewEntry.Confidence         = 1.0f;
    MemoryEntries.Add(NewEntry);

    // Escalate alert based on memory type
    switch (MemType)
    {
        case ENPC_MemoryType::SeenPlayer:
            SetAlertLevel(ENPC_AlertLevel::Alert);
            break;
        case ENPC_MemoryType::HeardSound:
            if (AlertLevel < ENPC_AlertLevel::Suspicious)
                SetAlertLevel(ENPC_AlertLevel::Suspicious);
            break;
        case ENPC_MemoryType::SmeltBlood:
            if (AlertLevel < ENPC_AlertLevel::Curious)
                SetAlertLevel(ENPC_AlertLevel::Curious);
            break;
        case ENPC_MemoryType::WitnessedDeath:
            SetAlertLevel(ENPC_AlertLevel::Fleeing);
            break;
        case ENPC_MemoryType::TookDamage:
            SetAlertLevel(ENPC_AlertLevel::Combat);
            break;
        default:
            break;
    }

    // Write last known threat location to Blackboard
    if (BlackboardComp)
    {
        BlackboardComp->SetValueAsVector(FName("LastKnownThreatLocation"), LastKnownLocation);
    }
}

void UNPCBehaviorComponent::TickMemoryDecay(float DeltaTime)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();

    for (int32 i = MemoryEntries.Num() - 1; i >= 0; --i)
    {
        float Age = CurrentTime - MemoryEntries[i].TimeStamp;
        // Confidence decays linearly over MemoryDecayTime
        MemoryEntries[i].Confidence = FMath::Clamp(1.0f - (Age / MemoryDecayTime), 0.0f, 1.0f);

        if (MemoryEntries[i].Confidence <= 0.0f)
        {
            MemoryEntries.RemoveAt(i);
        }
    }
}

bool UNPCBehaviorComponent::HasMemoryOf(AActor* Actor, ENPC_MemoryType MemType) const
{
    for (const FNPC_MemoryEntry& Entry : MemoryEntries)
    {
        if (Entry.PerceivedActor == Actor && Entry.MemoryType == MemType && Entry.Confidence > 0.0f)
        {
            return true;
        }
    }
    return false;
}

FVector UNPCBehaviorComponent::GetLastKnownLocation(AActor* Actor) const
{
    for (const FNPC_MemoryEntry& Entry : MemoryEntries)
    {
        if (Entry.PerceivedActor == Actor)
        {
            return Entry.LastKnownLocation;
        }
    }
    return FVector::ZeroVector;
}

void UNPCBehaviorComponent::ClearMemory()
{
    MemoryEntries.Empty();
    AlertCooldownAccumulator = 0.0f;
    SetAlertLevel(ENPC_AlertLevel::Unaware);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Daily Routine
// ─────────────────────────────────────────────────────────────────────────────

void UNPCBehaviorComponent::SetRoutinePhase(ENPC_DailyRoutinePhase NewPhase)
{
    if (NewPhase == CurrentRoutinePhase) return;

    CurrentRoutinePhase = NewPhase;

    if (BlackboardComp)
    {
        BlackboardComp->SetValueAsEnum(FName("RoutinePhase"), static_cast<uint8>(CurrentRoutinePhase));
    }

    UE_LOG(LogTemp, Log, TEXT("NPCBehaviorComponent [%s]: RoutinePhase -> %d"),
        *GetOwner()->GetName(),
        static_cast<int32>(CurrentRoutinePhase));
}

void UNPCBehaviorComponent::EvaluateRoutinePhase()
{
    // During combat/flee states, routine is suspended
    if (AlertLevel >= ENPC_AlertLevel::Combat) return;

    // Simple time-of-day simulation (placeholder — will integrate with day/night cycle)
    float GameTime = GetWorld()->GetTimeSeconds();
    float DayProgress = FMath::Fmod(GameTime / 600.0f, 1.0f); // 10-minute day cycle

    ENPC_DailyRoutinePhase NewPhase;

    if (DayProgress < 0.05f)       NewPhase = ENPC_DailyRoutinePhase::Waking;
    else if (DayProgress < 0.25f)  NewPhase = ENPC_DailyRoutinePhase::Foraging;
    else if (DayProgress < 0.40f)  NewPhase = ENPC_DailyRoutinePhase::Patrolling;
    else if (DayProgress < 0.55f)  NewPhase = ENPC_DailyRoutinePhase::Resting;
    else if (DayProgress < 0.75f)  NewPhase = ENPC_DailyRoutinePhase::Foraging;
    else if (DayProgress < 0.90f)  NewPhase = ENPC_DailyRoutinePhase::Patrolling;
    else                           NewPhase = ENPC_DailyRoutinePhase::Sleeping;

    SetRoutinePhase(NewPhase);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Death
// ─────────────────────────────────────────────────────────────────────────────

void UNPCBehaviorComponent::OnNPCDeath()
{
    if (!bIsAlive) return;

    bIsAlive = false;
    SetRoutinePhase(ENPC_DailyRoutinePhase::Dead);
    SetAlertLevel(ENPC_AlertLevel::Unaware);
    ClearMemory();
    SetComponentTickEnabled(false);

    GetWorld()->GetTimerManager().ClearTimer(RoutinePhaseTimerHandle);

    if (BlackboardComp)
    {
        BlackboardComp->SetValueAsBool(FName("IsDead"), true);
    }

    UE_LOG(LogTemp, Log, TEXT("NPCBehaviorComponent [%s]: NPC died."), *GetOwner()->GetName());
}
