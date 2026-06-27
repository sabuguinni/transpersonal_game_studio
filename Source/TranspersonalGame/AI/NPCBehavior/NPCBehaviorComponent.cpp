// NPCBehaviorComponent.cpp
// Agent #11 — NPC Behavior Agent | PROD_CYCLE_AUTO_20260627_012
// Full implementation of UNPCBehaviorComponent — daily routines, alert system, memory

#include "NPCBehaviorComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "TimerManager.h"

UNPCBehaviorComponent::UNPCBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz — sufficient for NPC logic

    // Default daily schedule
    CurrentPhase = ENPC_DailyPhase::Dawn;
    AlertLevel = ENPC_AlertLevel::Unaware;
    CurrentBehaviorState = ENPC_BehaviorState::Idle;

    // Default personality
    Personality.Aggression = 0.3f;
    Personality.Curiosity = 0.5f;
    Personality.Cowardice = 0.4f;
    Personality.Sociability = 0.6f;
    Personality.TerritorialRadius = 1500.0f;

    // Default schedule
    DailySchedule.DawnActivity = ENPC_BehaviorState::Patrolling;
    DailySchedule.MorningActivity = ENPC_BehaviorState::Foraging;
    DailySchedule.MiddayActivity = ENPC_BehaviorState::Resting;
    DailySchedule.AfternoonActivity = ENPC_BehaviorState::Socializing;
    DailySchedule.DuskActivity = ENPC_BehaviorState::Patrolling;
    DailySchedule.NightActivity = ENPC_BehaviorState::Sleeping;
    DailySchedule.DeepNightActivity = ENPC_BehaviorState::Sleeping;

    // Perception defaults
    SightRange = 2500.0f;
    HearingRange = 1200.0f;
    SightAngleDegrees = 120.0f;
    AlertDecayRate = 0.05f;
    AlertBuildRate = 0.15f;
    AlertLevel_Float = 0.0f;

    // Memory
    MaxMemoryEntries = 16;
    MemoryRetentionTime = 120.0f;

    // Movement
    WalkSpeed = 200.0f;
    RunSpeed = 450.0f;
    FleeSpeed = 550.0f;

    bDebugDraw = false;
    WorldTimeHours = 6.0f; // Start at dawn
}

void UNPCBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();

    OwnerActor = GetOwner();
    if (!OwnerActor)
    {
        UE_LOG(LogTemp, Warning, TEXT("NPCBehaviorComponent: No owner actor!"));
        return;
    }

    OwnerCharacter = Cast<ACharacter>(OwnerActor);

    // Set initial phase from world time
    UpdateDailyPhase(WorldTimeHours);

    // Start behavior update timer (separate from tick for heavy logic)
    GetWorld()->GetTimerManager().SetTimer(
        BehaviorUpdateTimer,
        this,
        &UNPCBehaviorComponent::UpdateBehaviorState,
        1.0f,  // Every second
        true
    );

    UE_LOG(LogTemp, Log, TEXT("NPCBehaviorComponent initialized on %s | Phase: %d | State: %d"),
        *OwnerActor->GetName(),
        (int32)CurrentPhase,
        (int32)CurrentBehaviorState);
}

void UNPCBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!OwnerActor) return;

    // Update alert level decay/build
    UpdateAlertLevel(DeltaTime);

    // Run perception scan every tick (lightweight)
    ScanForThreats();

    // Debug visualization
    if (bDebugDraw)
    {
        DrawDebugSphere(GetWorld(), OwnerActor->GetActorLocation(), SightRange, 16,
            AlertLevel == ENPC_AlertLevel::Unaware ? FColor::Green :
            AlertLevel == ENPC_AlertLevel::Suspicious ? FColor::Yellow :
            AlertLevel == ENPC_AlertLevel::Alert ? FColor::Orange : FColor::Red,
            false, 0.15f);
    }
}

void UNPCBehaviorComponent::UpdateDailyPhase(float TimeOfDayHours)
{
    WorldTimeHours = TimeOfDayHours;

    ENPC_DailyPhase NewPhase;

    if (TimeOfDayHours >= 5.0f && TimeOfDayHours < 7.0f)
        NewPhase = ENPC_DailyPhase::Dawn;
    else if (TimeOfDayHours >= 7.0f && TimeOfDayHours < 12.0f)
        NewPhase = ENPC_DailyPhase::Morning;
    else if (TimeOfDayHours >= 12.0f && TimeOfDayHours < 14.0f)
        NewPhase = ENPC_DailyPhase::Midday;
    else if (TimeOfDayHours >= 14.0f && TimeOfDayHours < 18.0f)
        NewPhase = ENPC_DailyPhase::Afternoon;
    else if (TimeOfDayHours >= 18.0f && TimeOfDayHours < 20.0f)
        NewPhase = ENPC_DailyPhase::Dusk;
    else if (TimeOfDayHours >= 20.0f && TimeOfDayHours < 23.0f)
        NewPhase = ENPC_DailyPhase::Night;
    else
        NewPhase = ENPC_DailyPhase::DeepNight;

    if (NewPhase != CurrentPhase)
    {
        CurrentPhase = NewPhase;
        OnPhaseChanged(NewPhase);
    }
}

void UNPCBehaviorComponent::OnPhaseChanged(ENPC_DailyPhase NewPhase)
{
    // Only change behavior state if not in a high-alert situation
    if (AlertLevel == ENPC_AlertLevel::Unaware || AlertLevel == ENPC_AlertLevel::Suspicious)
    {
        ENPC_BehaviorState ScheduledState = GetScheduledActivityForPhase(NewPhase);
        TransitionToBehaviorState(ScheduledState);
    }

    UE_LOG(LogTemp, Log, TEXT("NPC %s phase changed to %d -> scheduled state: %d"),
        OwnerActor ? *OwnerActor->GetName() : TEXT("Unknown"),
        (int32)NewPhase,
        (int32)CurrentBehaviorState);
}

ENPC_BehaviorState UNPCBehaviorComponent::GetScheduledActivityForPhase(ENPC_DailyPhase Phase) const
{
    switch (Phase)
    {
    case ENPC_DailyPhase::Dawn:      return DailySchedule.DawnActivity;
    case ENPC_DailyPhase::Morning:   return DailySchedule.MorningActivity;
    case ENPC_DailyPhase::Midday:    return DailySchedule.MiddayActivity;
    case ENPC_DailyPhase::Afternoon: return DailySchedule.AfternoonActivity;
    case ENPC_DailyPhase::Dusk:      return DailySchedule.DuskActivity;
    case ENPC_DailyPhase::Night:     return DailySchedule.NightActivity;
    case ENPC_DailyPhase::DeepNight: return DailySchedule.DeepNightActivity;
    default:                          return ENPC_BehaviorState::Idle;
    }
}

void UNPCBehaviorComponent::TransitionToBehaviorState(ENPC_BehaviorState NewState)
{
    if (NewState == CurrentBehaviorState) return;

    ENPC_BehaviorState OldState = CurrentBehaviorState;
    CurrentBehaviorState = NewState;

    // Adjust movement speed based on state
    if (OwnerCharacter)
    {
        UCharacterMovementComponent* MovComp = OwnerCharacter->GetCharacterMovement();
        if (MovComp)
        {
            switch (NewState)
            {
            case ENPC_BehaviorState::Fleeing:
                MovComp->MaxWalkSpeed = FleeSpeed;
                break;
            case ENPC_BehaviorState::Chasing:
            case ENPC_BehaviorState::Patrolling:
                MovComp->MaxWalkSpeed = RunSpeed;
                break;
            case ENPC_BehaviorState::Sleeping:
            case ENPC_BehaviorState::Resting:
                MovComp->MaxWalkSpeed = 0.0f;
                break;
            default:
                MovComp->MaxWalkSpeed = WalkSpeed;
                break;
            }
        }
    }

    OnBehaviorStateChanged.Broadcast(OldState, NewState);

    UE_LOG(LogTemp, Verbose, TEXT("NPC %s state: %d -> %d"),
        OwnerActor ? *OwnerActor->GetName() : TEXT("Unknown"),
        (int32)OldState, (int32)NewState);
}

void UNPCBehaviorComponent::UpdateBehaviorState()
{
    if (!OwnerActor) return;

    // High-priority state overrides based on alert level
    switch (AlertLevel)
    {
    case ENPC_AlertLevel::Panicking:
        if (CurrentBehaviorState != ENPC_BehaviorState::Fleeing)
            TransitionToBehaviorState(ENPC_BehaviorState::Fleeing);
        break;

    case ENPC_AlertLevel::Combat:
        // Personality determines fight or flight
        if (Personality.Cowardice > Personality.Aggression)
        {
            if (CurrentBehaviorState != ENPC_BehaviorState::Fleeing)
                TransitionToBehaviorState(ENPC_BehaviorState::Fleeing);
        }
        else
        {
            if (CurrentBehaviorState != ENPC_BehaviorState::Chasing)
                TransitionToBehaviorState(ENPC_BehaviorState::Chasing);
        }
        break;

    case ENPC_AlertLevel::Alert:
        if (CurrentBehaviorState != ENPC_BehaviorState::Investigating)
            TransitionToBehaviorState(ENPC_BehaviorState::Investigating);
        break;

    case ENPC_AlertLevel::Suspicious:
        // Stay in current scheduled activity but slow down
        break;

    case ENPC_AlertLevel::Unaware:
        // Follow daily schedule — already handled by phase changes
        break;
    }
}

void UNPCBehaviorComponent::UpdateAlertLevel(float DeltaTime)
{
    // Decay alert over time when no threats visible
    if (!bThreatVisible)
    {
        AlertLevel_Float = FMath::Max(0.0f, AlertLevel_Float - AlertDecayRate * DeltaTime);
    }

    // Map float to enum
    ENPC_AlertLevel NewAlertLevel;
    if (AlertLevel_Float < 0.1f)
        NewAlertLevel = ENPC_AlertLevel::Unaware;
    else if (AlertLevel_Float < 0.35f)
        NewAlertLevel = ENPC_AlertLevel::Suspicious;
    else if (AlertLevel_Float < 0.65f)
        NewAlertLevel = ENPC_AlertLevel::Alert;
    else if (AlertLevel_Float < 0.85f)
        NewAlertLevel = ENPC_AlertLevel::Panicking;
    else
        NewAlertLevel = ENPC_AlertLevel::Combat;

    if (NewAlertLevel != AlertLevel)
    {
        AlertLevel = NewAlertLevel;
        OnAlertLevelChanged.Broadcast(AlertLevel);
    }
}

void UNPCBehaviorComponent::ScanForThreats()
{
    if (!OwnerActor || !GetWorld()) return;

    bThreatVisible = false;

    // Get player character as primary threat
    ACharacter* PlayerChar = Cast<ACharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
    if (!PlayerChar) return;

    FVector OwnerLoc = OwnerActor->GetActorLocation();
    FVector PlayerLoc = PlayerChar->GetActorLocation();
    float DistToPlayer = FVector::Dist(OwnerLoc, PlayerLoc);

    // Hearing check (omnidirectional)
    if (DistToPlayer <= HearingRange)
    {
        float HearingAlertBuild = (1.0f - (DistToPlayer / HearingRange)) * 0.3f;
        AlertLevel_Float = FMath::Min(1.0f, AlertLevel_Float + HearingAlertBuild * 0.016f);
    }

    // Sight check (cone-based)
    if (DistToPlayer <= SightRange)
    {
        FVector ToPlayer = (PlayerLoc - OwnerLoc).GetSafeNormal();
        FVector OwnerForward = OwnerActor->GetActorForwardVector();
        float DotProduct = FVector::DotProduct(OwnerForward, ToPlayer);
        float AngleDeg = FMath::RadiansToDegrees(FMath::Acos(DotProduct));

        if (AngleDeg <= SightAngleDegrees * 0.5f)
        {
            // Line of sight check
            FHitResult HitResult;
            FCollisionQueryParams QueryParams;
            QueryParams.AddIgnoredActor(OwnerActor);

            bool bBlocked = GetWorld()->LineTraceSingleByChannel(
                HitResult, OwnerLoc + FVector(0, 0, 80), PlayerLoc + FVector(0, 0, 80),
                ECC_Visibility, QueryParams);

            if (!bBlocked || HitResult.GetActor() == PlayerChar)
            {
                bThreatVisible = true;
                float SightAlertBuild = (1.0f - (DistToPlayer / SightRange)) * AlertBuildRate;
                AlertLevel_Float = FMath::Min(1.0f, AlertLevel_Float + SightAlertBuild * 0.016f);

                // Add to memory
                AddMemoryEntry(PlayerChar, PlayerLoc);
            }
        }
    }
}

void UNPCBehaviorComponent::AddMemoryEntry(AActor* Subject, FVector LastKnownLocation)
{
    if (!Subject) return;

    // Check if we already have an entry for this subject
    for (FNPC_MemoryEntry& Entry : MemoryEntries)
    {
        if (Entry.Subject == Subject)
        {
            Entry.LastKnownLocation = LastKnownLocation;
            Entry.TimeStamp = GetWorld()->GetTimeSeconds();
            Entry.ThreatLevel = AlertLevel_Float;
            return;
        }
    }

    // Add new entry
    if (MemoryEntries.Num() >= MaxMemoryEntries)
    {
        // Remove oldest entry
        MemoryEntries.RemoveAt(0);
    }

    FNPC_MemoryEntry NewEntry;
    NewEntry.Subject = Subject;
    NewEntry.LastKnownLocation = LastKnownLocation;
    NewEntry.TimeStamp = GetWorld()->GetTimeSeconds();
    NewEntry.ThreatLevel = AlertLevel_Float;
    MemoryEntries.Add(NewEntry);
}

void UNPCBehaviorComponent::PurgeOldMemories()
{
    if (!GetWorld()) return;

    float CurrentTime = GetWorld()->GetTimeSeconds();
    MemoryEntries.RemoveAll([&](const FNPC_MemoryEntry& Entry) {
        return (CurrentTime - Entry.TimeStamp) > MemoryRetentionTime;
    });
}

FVector UNPCBehaviorComponent::GetLastKnownThreatLocation() const
{
    if (MemoryEntries.Num() == 0) return FVector::ZeroVector;

    // Return most recent memory entry location
    const FNPC_MemoryEntry& Latest = MemoryEntries.Last();
    return Latest.LastKnownLocation;
}

bool UNPCBehaviorComponent::HasMemoryOf(AActor* Subject) const
{
    for (const FNPC_MemoryEntry& Entry : MemoryEntries)
    {
        if (Entry.Subject == Subject) return true;
    }
    return false;
}

void UNPCBehaviorComponent::ReactToSound(FVector SoundLocation, float SoundRadius, float ThreatMultiplier)
{
    if (!OwnerActor) return;

    float DistToSound = FVector::Dist(OwnerActor->GetActorLocation(), SoundLocation);
    if (DistToSound <= SoundRadius)
    {
        float AlertIncrease = (1.0f - (DistToSound / SoundRadius)) * ThreatMultiplier * AlertBuildRate;
        AlertLevel_Float = FMath::Min(1.0f, AlertLevel_Float + AlertIncrease);
        UE_LOG(LogTemp, Verbose, TEXT("NPC %s reacted to sound at distance %.0f, alert now %.2f"),
            *OwnerActor->GetName(), DistToSound, AlertLevel_Float);
    }
}

void UNPCBehaviorComponent::ForceAlertLevel(ENPC_AlertLevel NewLevel)
{
    AlertLevel = NewLevel;
    switch (NewLevel)
    {
    case ENPC_AlertLevel::Unaware:    AlertLevel_Float = 0.0f;  break;
    case ENPC_AlertLevel::Suspicious: AlertLevel_Float = 0.25f; break;
    case ENPC_AlertLevel::Alert:      AlertLevel_Float = 0.5f;  break;
    case ENPC_AlertLevel::Panicking:  AlertLevel_Float = 0.75f; break;
    case ENPC_AlertLevel::Combat:     AlertLevel_Float = 1.0f;  break;
    }
    OnAlertLevelChanged.Broadcast(AlertLevel);
}
