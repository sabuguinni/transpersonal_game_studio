// QuestStampedeSystem.cpp
// Agent #14 — Quest & Mission Designer
// "Survive the Stampede" — quest that fires when CrowdStampedeController triggers a panic event.
// Integrates with OnAgentPanicked / OnStampedeEnded delegates from CrowdStampedeController.

#include "QuestStampedeSystem.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"

// ============================================================
// Constructor
// ============================================================
AQuest_StampedeManager::AQuest_StampedeManager()
{
    PrimaryActorTick.bCanEverTick = true;

    CurrentObjective      = EQuest_StampedeObjective::None;
    bQuestActive          = false;
    bQuestCompleted       = false;
    bOnHighGround         = false;
    SurviveTimer          = 0.0f;

    HighGroundMinZ        = 300.0f;   // Z threshold for "high ground" in cm
    HerdObserveRadius     = 1500.0f;  // Must be within 1500 units of herd
    StampedeWaveSurviveTime = 20.0f; // Must survive 20 seconds on high ground
    HerdObserveLocation   = FVector(3000.0f, 2000.0f, 0.0f); // Matches herd spawn from Agent #13
}

// ============================================================
// BeginPlay
// ============================================================
void AQuest_StampedeManager::BeginPlay()
{
    Super::BeginPlay();

    // Auto-start quest on level load
    StartQuest();
}

// ============================================================
// Tick — check high ground condition during SurviveWave objective
// ============================================================
void AQuest_StampedeManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bQuestActive || bQuestCompleted)
    {
        return;
    }

    if (CurrentObjective == EQuest_StampedeObjective::SurviveWave)
    {
        CheckPlayerOnHighGround();

        if (bOnHighGround)
        {
            SurviveTimer += DeltaTime;
            if (SurviveTimer >= StampedeWaveSurviveTime)
            {
                OnStampedeEnded(true);
            }
        }
        else
        {
            // Player fell off high ground — reset timer
            SurviveTimer = FMath::Max(0.0f, SurviveTimer - DeltaTime * 2.0f);
        }
    }

    // Check herd proximity for ObserveHerd objective
    if (CurrentObjective == EQuest_StampedeObjective::ObserveHerd)
    {
        if (IsPlayerNearHerd())
        {
            AdvanceObjective(EQuest_StampedeObjective::DetectThreat);
        }
    }
}

// ============================================================
// StartQuest
// ============================================================
void AQuest_StampedeManager::StartQuest()
{
    if (bQuestActive)
    {
        return;
    }

    bQuestActive    = false; // Will be set true when player enters herd area
    bQuestCompleted = false;
    SurviveTimer    = 0.0f;

    // Quest begins in "ObserveHerd" state — player must find the herd first
    CurrentObjective = EQuest_StampedeObjective::ObserveHerd;
    bQuestActive     = true;

    OnObjectiveChanged.Broadcast(CurrentObjective);

    UE_LOG(LogTemp, Log, TEXT("QuestStampede: Quest started — Objective: ObserveHerd at (3000, 2000)"));
}

// ============================================================
// AdvanceObjective
// ============================================================
void AQuest_StampedeManager::AdvanceObjective(EQuest_StampedeObjective NewObjective)
{
    if (!bQuestActive || bQuestCompleted)
    {
        return;
    }

    CurrentObjective = NewObjective;
    OnObjectiveChanged.Broadcast(CurrentObjective);

    UE_LOG(LogTemp, Log, TEXT("QuestStampede: Objective advanced to %d"), (int32)NewObjective);
}

// ============================================================
// OnStampedeTriggered — called when CrowdStampedeController fires
// ============================================================
void AQuest_StampedeManager::OnStampedeTriggered(EQuest_StampedeCause Cause, FVector Location, float PanicLevel)
{
    if (!bQuestActive || bQuestCompleted)
    {
        return;
    }

    // Record the event
    LastStampedeRecord.Cause          = Cause;
    LastStampedeRecord.TriggerLocation = Location;
    LastStampedeRecord.PanicPeakLevel  = PanicLevel;
    LastStampedeRecord.bPlayerSurvived = false;
    LastStampedeRecord.TimeToReachSafety = 0.0f;

    // If player has observed the herd, advance to SurviveWave
    if (CurrentObjective == EQuest_StampedeObjective::DetectThreat ||
        CurrentObjective == EQuest_StampedeObjective::ObserveHerd)
    {
        SurviveTimer = 0.0f;
        AdvanceObjective(EQuest_StampedeObjective::ReachHighGround);

        // Give player 10 seconds to reach high ground before wave hits
        FTimerHandle HighGroundTimer;
        GetWorldTimerManager().SetTimer(HighGroundTimer, [this]()
        {
            if (CurrentObjective == EQuest_StampedeObjective::ReachHighGround)
            {
                AdvanceObjective(EQuest_StampedeObjective::SurviveWave);
            }
        }, 10.0f, false);
    }

    UE_LOG(LogTemp, Log, TEXT("QuestStampede: Stampede triggered — Cause=%d PanicLevel=%.2f"),
        (int32)Cause, PanicLevel);
}

// ============================================================
// OnStampedeEnded — called when stampede wave subsides
// ============================================================
void AQuest_StampedeManager::OnStampedeEnded(bool bPlayerSurvived)
{
    if (!bQuestActive || bQuestCompleted)
    {
        return;
    }

    LastStampedeRecord.bPlayerSurvived    = bPlayerSurvived;
    LastStampedeRecord.TimeToReachSafety  = SurviveTimer;

    if (bPlayerSurvived)
    {
        CompleteQuest();
    }
    else
    {
        FailQuest();
    }
}

// ============================================================
// CheckPlayerOnHighGround
// ============================================================
void AQuest_StampedeManager::CheckPlayerOnHighGround()
{
    ACharacter* Player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (!Player)
    {
        bOnHighGround = false;
        return;
    }

    float PlayerZ = Player->GetActorLocation().Z;
    bOnHighGround = (PlayerZ >= HighGroundMinZ);

    if (bOnHighGround && CurrentObjective == EQuest_StampedeObjective::ReachHighGround)
    {
        AdvanceObjective(EQuest_StampedeObjective::SurviveWave);
    }
}

// ============================================================
// IsPlayerNearHerd
// ============================================================
bool AQuest_StampedeManager::IsPlayerNearHerd() const
{
    ACharacter* Player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (!Player)
    {
        return false;
    }

    float DistSq = FVector::DistSquared(Player->GetActorLocation(), HerdObserveLocation);
    return DistSq <= (HerdObserveRadius * HerdObserveRadius);
}

// ============================================================
// GetObjectiveText — returns HUD-ready objective string
// ============================================================
FString AQuest_StampedeManager::GetObjectiveText() const
{
    switch (CurrentObjective)
    {
        case EQuest_StampedeObjective::ObserveHerd:
            return TEXT("Find the herbivore herd in the southern valley");
        case EQuest_StampedeObjective::DetectThreat:
            return TEXT("Watch the herd — something is disturbing them");
        case EQuest_StampedeObjective::ReachHighGround:
            return TEXT("STAMPEDE! Reach high ground before the herd reaches you!");
        case EQuest_StampedeObjective::SurviveWave:
            return TEXT("Stay on high ground — survive the stampede wave");
        case EQuest_StampedeObjective::Completed:
            return TEXT("You survived the stampede");
        default:
            return TEXT("");
    }
}

// ============================================================
// GetQuestProgress — 0.0 to 1.0
// ============================================================
float AQuest_StampedeManager::GetQuestProgress() const
{
    switch (CurrentObjective)
    {
        case EQuest_StampedeObjective::None:            return 0.0f;
        case EQuest_StampedeObjective::ObserveHerd:     return 0.1f;
        case EQuest_StampedeObjective::DetectThreat:    return 0.35f;
        case EQuest_StampedeObjective::ReachHighGround: return 0.60f;
        case EQuest_StampedeObjective::SurviveWave:
        {
            float WaveProgress = FMath::Clamp(SurviveTimer / StampedeWaveSurviveTime, 0.0f, 1.0f);
            return 0.60f + WaveProgress * 0.40f;
        }
        case EQuest_StampedeObjective::Completed:       return 1.0f;
        default:                                         return 0.0f;
    }
}

// ============================================================
// CompleteQuest
// ============================================================
void AQuest_StampedeManager::CompleteQuest()
{
    bQuestCompleted  = true;
    CurrentObjective = EQuest_StampedeObjective::Completed;

    OnObjectiveChanged.Broadcast(CurrentObjective);
    OnQuestCompleted.Broadcast(LastStampedeRecord);

    UE_LOG(LogTemp, Log, TEXT("QuestStampede: QUEST COMPLETE — Player survived in %.1f seconds"),
        LastStampedeRecord.TimeToReachSafety);
}

// ============================================================
// FailQuest
// ============================================================
void AQuest_StampedeManager::FailQuest()
{
    bQuestActive = false;

    UE_LOG(LogTemp, Warning, TEXT("QuestStampede: Quest FAILED — player did not reach high ground in time"));

    // Restart after 5 seconds
    FTimerHandle RestartTimer;
    GetWorldTimerManager().SetTimer(RestartTimer, [this]()
    {
        bQuestActive     = false;
        bQuestCompleted  = false;
        CurrentObjective = EQuest_StampedeObjective::None;
        StartQuest();
    }, 5.0f, false);
}
