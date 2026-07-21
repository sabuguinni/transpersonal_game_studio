// NPCBehaviorComponent.cpp
// NPC Behavior Agent #11 — PROD_CYCLE_AUTO_20260628_011
// Implements full NPC AI state machine: daily routines, alert levels, memory, reactions

#include "NPCBehaviorComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "NavigationSystem.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

DEFINE_LOG_CATEGORY_STATIC(LogNPCBehavior, Log, All);

UNPCBehaviorComponent::UNPCBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz is sufficient for NPC logic

    // Default configuration
    PatrolRadius = 2000.0f;
    AlertDecayRate = 0.5f;
    MemoryRetentionTime = 60.0f;
    HearingRange = 1500.0f;
    SightRange = 2500.0f;
    SightAngle = 90.0f;
    FleeSpeedMultiplier = 1.8f;
    AttackRange = 250.0f;
    InvestigateRadius = 400.0f;

    CurrentRoutinePhase = ENPC_DailyRoutinePhase::Idle;
    CurrentAlertLevel = ENPC_AlertLevel::Unaware;
    AlertValue = 0.0f;
    bIsAlive = true;
    bDebugDraw = false;

    HomeLocation = FVector::ZeroVector;
    LastKnownThreatLocation = FVector::ZeroVector;
    bHasThreatMemory = false;
    ThreatMemoryAge = 0.0f;

    OwnerCharacter = nullptr;
    OwnerController = nullptr;
    OwnerBlackboard = nullptr;
}

void UNPCBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();

    // Cache owner references
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        OwnerController = Cast<AAIController>(OwnerCharacter->GetController());
        HomeLocation = OwnerCharacter->GetActorLocation();

        if (OwnerController)
        {
            UBehaviorTreeComponent* BTComp = OwnerController->FindComponentByClass<UBehaviorTreeComponent>();
            if (BTComp)
            {
                OwnerBlackboard = OwnerController->GetBlackboardComponent();
            }
        }
    }

    // Start daily routine timer — re-evaluate phase every 5 seconds
    GetWorld()->GetTimerManager().SetTimer(
        RoutineTimerHandle,
        this,
        &UNPCBehaviorComponent::EvaluateDailyRoutine,
        5.0f,
        true,
        1.0f
    );

    UE_LOG(LogNPCBehavior, Log, TEXT("NPCBehaviorComponent initialized for %s at home location %s"),
        *GetOwner()->GetName(), *HomeLocation.ToString());
}

void UNPCBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bIsAlive) return;

    // Update alert decay
    UpdateAlertLevel(DeltaTime);

    // Update threat memory age
    if (bHasThreatMemory)
    {
        ThreatMemoryAge += DeltaTime;
        if (ThreatMemoryAge >= MemoryRetentionTime)
        {
            ClearThreatMemory();
        }
    }

    // Push state to blackboard if available
    SyncToBlackboard();

    // Debug visualization
    if (bDebugDraw)
    {
        DrawDebugState();
    }
}

// ─── ALERT SYSTEM ─────────────────────────────────────────────────────────────

void UNPCBehaviorComponent::RegisterThreat(AActor* ThreatActor, float ThreatMagnitude, ENPC_ThreatType ThreatType)
{
    if (!ThreatActor || !bIsAlive) return;

    // Record threat memory
    FNPC_ThreatMemory NewMemory;
    NewMemory.ThreatActor = ThreatActor;
    NewMemory.LastKnownLocation = ThreatActor->GetActorLocation();
    NewMemory.ThreatMagnitude = ThreatMagnitude;
    NewMemory.ThreatType = ThreatType;
    NewMemory.TimeStamp = GetWorld()->GetTimeSeconds();
    NewMemory.bIsActive = true;

    // Update or add to memory
    bool bFound = false;
    for (FNPC_ThreatMemory& Mem : ThreatMemories)
    {
        if (Mem.ThreatActor == ThreatActor)
        {
            Mem = NewMemory;
            bFound = true;
            break;
        }
    }
    if (!bFound)
    {
        ThreatMemories.Add(NewMemory);
    }

    // Raise alert
    AlertValue = FMath::Clamp(AlertValue + ThreatMagnitude, 0.0f, 100.0f);
    LastKnownThreatLocation = NewMemory.LastKnownLocation;
    bHasThreatMemory = true;
    ThreatMemoryAge = 0.0f;

    // Determine new alert level
    ENPC_AlertLevel NewLevel = ComputeAlertLevel();
    if (NewLevel != CurrentAlertLevel)
    {
        SetAlertLevel(NewLevel);
    }

    UE_LOG(LogNPCBehavior, Verbose, TEXT("%s registered threat from %s — alert: %.1f"),
        *GetOwner()->GetName(), *ThreatActor->GetName(), AlertValue);
}

void UNPCBehaviorComponent::SetAlertLevel(ENPC_AlertLevel NewLevel)
{
    if (NewLevel == CurrentAlertLevel) return;

    ENPC_AlertLevel OldLevel = CurrentAlertLevel;
    CurrentAlertLevel = NewLevel;

    OnAlertLevelChanged.Broadcast(OldLevel, NewLevel);

    // Trigger phase transitions based on alert level
    switch (NewLevel)
    {
        case ENPC_AlertLevel::Unaware:
            // Return to normal routine
            break;
        case ENPC_AlertLevel::Curious:
            SetRoutinePhase(ENPC_DailyRoutinePhase::Investigating);
            break;
        case ENPC_AlertLevel::Suspicious:
            SetRoutinePhase(ENPC_DailyRoutinePhase::Investigating);
            break;
        case ENPC_AlertLevel::Alert:
            SetRoutinePhase(ENPC_DailyRoutinePhase::Patrolling);
            break;
        case ENPC_AlertLevel::Combat:
            SetRoutinePhase(ENPC_DailyRoutinePhase::Attacking);
            break;
        case ENPC_AlertLevel::Fleeing:
            SetRoutinePhase(ENPC_DailyRoutinePhase::Fleeing);
            break;
        default:
            break;
    }

    UE_LOG(LogNPCBehavior, Log, TEXT("%s alert level: %d -> %d"),
        *GetOwner()->GetName(), (int32)OldLevel, (int32)NewLevel);
}

void UNPCBehaviorComponent::UpdateAlertLevel(float DeltaTime)
{
    // Decay alert when no active threat
    bool bHasActiveThreat = false;
    for (const FNPC_ThreatMemory& Mem : ThreatMemories)
    {
        if (Mem.bIsActive)
        {
            bHasActiveThreat = true;
            break;
        }
    }

    if (!bHasActiveThreat && AlertValue > 0.0f)
    {
        AlertValue = FMath::Max(0.0f, AlertValue - AlertDecayRate * DeltaTime * 10.0f);

        ENPC_AlertLevel NewLevel = ComputeAlertLevel();
        if (NewLevel != CurrentAlertLevel)
        {
            SetAlertLevel(NewLevel);
        }
    }
}

ENPC_AlertLevel UNPCBehaviorComponent::ComputeAlertLevel() const
{
    if (AlertValue <= 0.0f)  return ENPC_AlertLevel::Unaware;
    if (AlertValue < 15.0f)  return ENPC_AlertLevel::Curious;
    if (AlertValue < 30.0f)  return ENPC_AlertLevel::Suspicious;
    if (AlertValue < 55.0f)  return ENPC_AlertLevel::Alert;
    if (AlertValue < 80.0f)  return ENPC_AlertLevel::Combat;
    return ENPC_AlertLevel::Fleeing;
}

void UNPCBehaviorComponent::ClearThreatMemory()
{
    ThreatMemories.Empty();
    bHasThreatMemory = false;
    ThreatMemoryAge = 0.0f;
    AlertValue = FMath::Max(0.0f, AlertValue - 20.0f);
    UE_LOG(LogNPCBehavior, Log, TEXT("%s threat memory cleared"), *GetOwner()->GetName());
}

// ─── DAILY ROUTINE ────────────────────────────────────────────────────────────

void UNPCBehaviorComponent::SetRoutinePhase(ENPC_DailyRoutinePhase NewPhase)
{
    if (NewPhase == CurrentRoutinePhase) return;

    ENPC_DailyRoutinePhase OldPhase = CurrentRoutinePhase;
    CurrentRoutinePhase = NewPhase;

    OnRoutinePhaseChanged.Broadcast(OldPhase, NewPhase);

    // Apply movement speed changes per phase
    if (OwnerCharacter)
    {
        UCharacterMovementComponent* MoveComp = OwnerCharacter->GetCharacterMovement();
        if (MoveComp)
        {
            switch (NewPhase)
            {
                case ENPC_DailyRoutinePhase::Sleeping:
                    MoveComp->MaxWalkSpeed = 0.0f;
                    break;
                case ENPC_DailyRoutinePhase::Idle:
                case ENPC_DailyRoutinePhase::Resting:
                    MoveComp->MaxWalkSpeed = 0.0f;
                    break;
                case ENPC_DailyRoutinePhase::Foraging:
                    MoveComp->MaxWalkSpeed = 150.0f;
                    break;
                case ENPC_DailyRoutinePhase::Patrolling:
                    MoveComp->MaxWalkSpeed = 300.0f;
                    break;
                case ENPC_DailyRoutinePhase::Investigating:
                    MoveComp->MaxWalkSpeed = 250.0f;
                    break;
                case ENPC_DailyRoutinePhase::Fleeing:
                    MoveComp->MaxWalkSpeed = 600.0f * FleeSpeedMultiplier;
                    break;
                case ENPC_DailyRoutinePhase::Attacking:
                    MoveComp->MaxWalkSpeed = 500.0f;
                    break;
                default:
                    break;
            }
        }
    }

    UE_LOG(LogNPCBehavior, Log, TEXT("%s routine phase: %d -> %d"),
        *GetOwner()->GetName(), (int32)OldPhase, (int32)NewPhase);
}

void UNPCBehaviorComponent::EvaluateDailyRoutine()
{
    if (!bIsAlive) return;

    // Don't override combat/flee states from routine evaluation
    if (CurrentAlertLevel >= ENPC_AlertLevel::Combat) return;

    // Time-of-day based routine (simplified — uses world time seconds as proxy)
    float WorldTime = GetWorld()->GetTimeSeconds();
    float DayFraction = FMath::Fmod(WorldTime / 300.0f, 1.0f); // 5-minute day cycle for testing

    ENPC_DailyRoutinePhase TargetPhase;

    if (DayFraction < 0.1f)       TargetPhase = ENPC_DailyRoutinePhase::Waking;
    else if (DayFraction < 0.35f) TargetPhase = ENPC_DailyRoutinePhase::Foraging;
    else if (DayFraction < 0.5f)  TargetPhase = ENPC_DailyRoutinePhase::Resting;
    else if (DayFraction < 0.75f) TargetPhase = ENPC_DailyRoutinePhase::Patrolling;
    else if (DayFraction < 0.85f) TargetPhase = ENPC_DailyRoutinePhase::Foraging;
    else                          TargetPhase = ENPC_DailyRoutinePhase::Sleeping;

    SetRoutinePhase(TargetPhase);
}

// ─── PATROL SYSTEM ────────────────────────────────────────────────────────────

FVector UNPCBehaviorComponent::GetNextPatrolPoint() const
{
    if (!OwnerCharacter) return HomeLocation;

    UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
    if (!NavSys) return HomeLocation;

    FNavLocation NavLoc;
    bool bFound = NavSys->GetRandomReachablePointInRadius(HomeLocation, PatrolRadius, NavLoc);
    return bFound ? NavLoc.Location : HomeLocation;
}

FVector UNPCBehaviorComponent::GetFleeDestination() const
{
    if (!OwnerCharacter) return HomeLocation;

    // Flee directly away from last known threat
    if (bHasThreatMemory)
    {
        FVector ToThreat = LastKnownThreatLocation - OwnerCharacter->GetActorLocation();
        FVector FleeDir = -ToThreat.GetSafeNormal();
        FVector FleeTarget = OwnerCharacter->GetActorLocation() + FleeDir * 3000.0f;

        UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
        if (NavSys)
        {
            FNavLocation NavLoc;
            if (NavSys->GetRandomReachablePointInRadius(FleeTarget, 500.0f, NavLoc))
            {
                return NavLoc.Location;
            }
        }
        return FleeTarget;
    }

    return HomeLocation;
}

// ─── BLACKBOARD SYNC ──────────────────────────────────────────────────────────

void UNPCBehaviorComponent::SyncToBlackboard()
{
    if (!OwnerBlackboard) return;

    // Sync alert level as integer
    OwnerBlackboard->SetValueAsInt(FName("AlertLevel"), (int32)CurrentAlertLevel);
    OwnerBlackboard->SetValueAsInt(FName("RoutinePhase"), (int32)CurrentRoutinePhase);
    OwnerBlackboard->SetValueAsBool(FName("HasThreatMemory"), bHasThreatMemory);
    OwnerBlackboard->SetValueAsFloat(FName("AlertValue"), AlertValue);

    if (bHasThreatMemory)
    {
        OwnerBlackboard->SetValueAsVector(FName("LastKnownThreatLocation"), LastKnownThreatLocation);
    }
}

// ─── PUBLIC API ───────────────────────────────────────────────────────────────

void UNPCBehaviorComponent::OnHearNoise(AActor* NoiseInstigator, float Volume)
{
    if (!NoiseInstigator || !bIsAlive) return;

    float ThreatMagnitude = Volume * 20.0f; // Scale volume to alert units
    RegisterThreat(NoiseInstigator, ThreatMagnitude, ENPC_ThreatType::Sound);
}

void UNPCBehaviorComponent::OnSeeActor(AActor* SeenActor, float Confidence)
{
    if (!SeenActor || !bIsAlive) return;

    float ThreatMagnitude = Confidence * 40.0f;
    RegisterThreat(SeenActor, ThreatMagnitude, ENPC_ThreatType::Sight);
}

void UNPCBehaviorComponent::OnTakeDamage(float DamageAmount, AActor* DamageInstigator)
{
    if (!bIsAlive) return;

    if (DamageInstigator)
    {
        RegisterThreat(DamageInstigator, DamageAmount * 2.0f, ENPC_ThreatType::Damage);
    }

    // Immediate high alert on damage
    AlertValue = FMath::Min(100.0f, AlertValue + DamageAmount);
    SetAlertLevel(ComputeAlertLevel());
}

void UNPCBehaviorComponent::SetDead()
{
    bIsAlive = false;
    CurrentRoutinePhase = ENPC_DailyRoutinePhase::Dead;
    CurrentAlertLevel = ENPC_AlertLevel::Unaware;
    AlertValue = 0.0f;

    GetWorld()->GetTimerManager().ClearTimer(RoutineTimerHandle);

    if (OwnerCharacter)
    {
        UCharacterMovementComponent* MoveComp = OwnerCharacter->GetCharacterMovement();
        if (MoveComp)
        {
            MoveComp->MaxWalkSpeed = 0.0f;
            MoveComp->DisableMovement();
        }
    }

    UE_LOG(LogNPCBehavior, Log, TEXT("%s is dead"), *GetOwner()->GetName());
}

bool UNPCBehaviorComponent::IsInCombat() const
{
    return CurrentAlertLevel == ENPC_AlertLevel::Combat;
}

bool UNPCBehaviorComponent::IsFleeing() const
{
    return CurrentAlertLevel == ENPC_AlertLevel::Fleeing ||
           CurrentRoutinePhase == ENPC_DailyRoutinePhase::Fleeing;
}

float UNPCBehaviorComponent::GetAlertValue() const
{
    return AlertValue;
}

// ─── DEBUG ────────────────────────────────────────────────────────────────────

void UNPCBehaviorComponent::DrawDebugState()
{
    if (!OwnerCharacter) return;

    FVector Loc = OwnerCharacter->GetActorLocation() + FVector(0, 0, 120.0f);

    // Alert level color
    FColor AlertColor = FColor::Green;
    switch (CurrentAlertLevel)
    {
        case ENPC_AlertLevel::Curious:    AlertColor = FColor::Cyan;    break;
        case ENPC_AlertLevel::Suspicious: AlertColor = FColor::Yellow;  break;
        case ENPC_AlertLevel::Alert:      AlertColor = FColor::Orange;  break;
        case ENPC_AlertLevel::Combat:     AlertColor = FColor::Red;     break;
        case ENPC_AlertLevel::Fleeing:    AlertColor = FColor::Purple;  break;
        default: break;
    }

    DrawDebugSphere(GetWorld(), Loc, 30.0f, 8, AlertColor, false, 0.15f);

    if (bHasThreatMemory)
    {
        DrawDebugLine(GetWorld(), OwnerCharacter->GetActorLocation(),
            LastKnownThreatLocation, FColor::Red, false, 0.15f, 0, 2.0f);
    }
}
