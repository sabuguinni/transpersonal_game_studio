#include "QuestSurviveStampede.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"

// ============================================================
// AQuestSurviveStampede — Implementation
// Agent #14 — Quest & Mission Designer
// Cycle: PROD_CYCLE_AUTO_20260629_001
//
// Quest: "Survive the Stampede"
// Emotional arc: Panic → Determination → Relief → Pride
// Mechanical arc: Warning → Danger → Escape → Optional Combat
// ============================================================

AQuestSurviveStampede::AQuestSurviveStampede()
{
    PrimaryActorTick.bCanEverTick = true;

    // Default config
    CurrentPhase          = EQuest_StampedePhase::NotStarted;
    QuestTriggerRadius    = 500.0f;
    HighGroundZThreshold  = 350.0f;
    StampedeTimeLimit     = 90.0f;
    ElapsedStampedeTime   = 0.0f;
    bQuestStarted         = false;
    bQuestEnded           = false;

    // High ground ridge location (north-east ridge in MinPlayableMap)
    HighGroundTargetLocation = FVector(800.0f, -500.0f, 400.0f);

    // Danger zone — the stampede corridor (south valley)
    StampedeDangerZoneMin = FVector(1500.0f, 800.0f, 0.0f);
    StampedeDangerZoneMax = FVector(2800.0f, 2800.0f, 300.0f);

    // Audio URLs from ElevenLabs TTS (generated this cycle)
    WarningVoiceLineURL  = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782693143757_TribalElder_StampedeWarning.mp3");
    CompleteVoiceLineURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782693163591_TribalElder_QuestComplete.mp3");
}

void AQuestSurviveStampede::BeginPlay()
{
    Super::BeginPlay();
    InitialiseObjectives();
}

void AQuestSurviveStampede::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bQuestStarted || bQuestEnded)
        return;

    TickStampedePhase(DeltaTime);
}

void AQuestSurviveStampede::InitialiseObjectives()
{
    Objectives.Empty();

    // Objective 1 — Mandatory: Reach high ground before stampede hits
    FQuest_StampedeObjective Obj1;
    Obj1.ObjectiveID  = TEXT("REACH_HIGH_GROUND");
    Obj1.Description  = TEXT("Climb to the ridge before the herd reaches you (90 seconds)");
    Obj1.bCompleted   = false;
    Obj1.bOptional    = false;
    Objectives.Add(Obj1);

    // Objective 2 — Mandatory: Survive the stampede (don't die in danger zone)
    FQuest_StampedeObjective Obj2;
    Obj2.ObjectiveID  = TEXT("SURVIVE_DANGER_ZONE");
    Obj2.Description  = TEXT("Do not be caught in the stampede corridor");
    Obj2.bCompleted   = false;
    Obj2.bOptional    = false;
    Objectives.Add(Obj2);

    // Objective 3 — Optional: Drive off the predator
    FQuest_StampedeObjective Obj3;
    Obj3.ObjectiveID  = TEXT("DRIVE_OFF_PREDATOR");
    Obj3.Description  = TEXT("(Optional) Drive off the TRex using fire or noise");
    Obj3.bCompleted   = false;
    Obj3.bOptional    = true;
    Objectives.Add(Obj3);
}

void AQuestSurviveStampede::StartQuest()
{
    if (bQuestStarted)
        return;

    bQuestStarted = true;
    ElapsedStampedeTime = 0.0f;

    UE_LOG(LogTemp, Warning, TEXT("[Quest] 'Survive the Stampede' STARTED"));
    AdvanceToPhase(EQuest_StampedePhase::WarningIssued);
}

void AQuestSurviveStampede::AdvanceToPhase(EQuest_StampedePhase NewPhase)
{
    CurrentPhase = NewPhase;

    switch (NewPhase)
    {
        case EQuest_StampedePhase::WarningIssued:
            UE_LOG(LogTemp, Warning, TEXT("[Quest] Phase: WARNING — Elder issues stampede warning"));
            // In full implementation: play WarningVoiceLineURL via audio component
            break;

        case EQuest_StampedePhase::StampedeActive:
            UE_LOG(LogTemp, Warning, TEXT("[Quest] Phase: STAMPEDE ACTIVE — Herd is running, danger zone hot"));
            // In full implementation: call UCrowdStampedeController::TriggerStampede()
            break;

        case EQuest_StampedePhase::ReachHighGround:
            UE_LOG(LogTemp, Warning, TEXT("[Quest] Phase: REACH HIGH GROUND — Player must climb ridge"));
            break;

        case EQuest_StampedePhase::DefeatPredator:
            UE_LOG(LogTemp, Warning, TEXT("[Quest] Phase: DEFEAT PREDATOR — Optional objective active"));
            break;

        case EQuest_StampedePhase::Completed:
            CompleteQuest();
            break;

        case EQuest_StampedePhase::Failed:
            FailQuest(TEXT("Caught in stampede corridor"));
            break;

        default:
            break;
    }
}

void AQuestSurviveStampede::CompleteObjective(const FString& ObjectiveID)
{
    for (FQuest_StampedeObjective& Obj : Objectives)
    {
        if (Obj.ObjectiveID == ObjectiveID && !Obj.bCompleted)
        {
            Obj.bCompleted = true;
            UE_LOG(LogTemp, Warning, TEXT("[Quest] Objective COMPLETE: %s"), *ObjectiveID);

            if (AreAllRequiredObjectivesComplete())
            {
                AdvanceToPhase(EQuest_StampedePhase::Completed);
            }
            return;
        }
    }
}

bool AQuestSurviveStampede::AreAllRequiredObjectivesComplete() const
{
    for (const FQuest_StampedeObjective& Obj : Objectives)
    {
        if (!Obj.bOptional && !Obj.bCompleted)
            return false;
    }
    return true;
}

void AQuestSurviveStampede::CompleteQuest()
{
    if (bQuestEnded)
        return;

    bQuestEnded = true;
    CurrentPhase = EQuest_StampedePhase::Completed;

    UE_LOG(LogTemp, Warning, TEXT("[Quest] 'Survive the Stampede' COMPLETED"));
    UE_LOG(LogTemp, Warning, TEXT("[Quest] Reward: %d BoneTokens, %d StoneAxe, %d DriedMeat, +%.0f Reputation"),
        QuestReward.BoneTokens,
        QuestReward.StoneAxeCount,
        QuestReward.DriedMeatCount,
        QuestReward.ReputationGain);
    // In full implementation: play CompleteVoiceLineURL, grant inventory items, update reputation
}

void AQuestSurviveStampede::FailQuest(const FString& Reason)
{
    if (bQuestEnded)
        return;

    bQuestEnded = true;
    CurrentPhase = EQuest_StampedePhase::Failed;

    UE_LOG(LogTemp, Warning, TEXT("[Quest] 'Survive the Stampede' FAILED — Reason: %s"), *Reason);
    // In full implementation: show failure screen, allow retry after 60s respawn
}

bool AQuestSurviveStampede::IsPlayerInDangerZone(const FVector& PlayerLocation) const
{
    return (PlayerLocation.X >= StampedeDangerZoneMin.X && PlayerLocation.X <= StampedeDangerZoneMax.X &&
            PlayerLocation.Y >= StampedeDangerZoneMin.Y && PlayerLocation.Y <= StampedeDangerZoneMax.Y &&
            PlayerLocation.Z >= StampedeDangerZoneMin.Z && PlayerLocation.Z <= StampedeDangerZoneMax.Z);
}

bool AQuestSurviveStampede::IsPlayerOnHighGround(const FVector& PlayerLocation) const
{
    return PlayerLocation.Z >= HighGroundZThreshold;
}

float AQuestSurviveStampede::GetStampedeTimeRemaining() const
{
    return FMath::Max(0.0f, StampedeTimeLimit - ElapsedStampedeTime);
}

EQuest_StampedePhase AQuestSurviveStampede::GetCurrentPhase() const
{
    return CurrentPhase;
}

void AQuestSurviveStampede::DEBUG_StartQuestNow()
{
    StartQuest();
    AdvanceToPhase(EQuest_StampedePhase::StampedeActive);
    UE_LOG(LogTemp, Warning, TEXT("[Quest DEBUG] Quest force-started and stampede activated"));
}

void AQuestSurviveStampede::TickStampedePhase(float DeltaTime)
{
    // Only tick during active stampede phase
    if (CurrentPhase != EQuest_StampedePhase::StampedeActive &&
        CurrentPhase != EQuest_StampedePhase::ReachHighGround)
        return;

    ElapsedStampedeTime += DeltaTime;

    // Get player location
    UWorld* World = GetWorld();
    if (!World)
        return;

    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC || !PC->GetPawn())
        return;

    FVector PlayerLoc = PC->GetPawn()->GetActorLocation();

    // Check if player reached high ground
    if (IsPlayerOnHighGround(PlayerLoc))
    {
        CompleteObjective(TEXT("REACH_HIGH_GROUND"));
        CompleteObjective(TEXT("SURVIVE_DANGER_ZONE"));
        AdvanceToPhase(EQuest_StampedePhase::Completed);
        return;
    }

    // Check if player is caught in danger zone during stampede
    if (CurrentPhase == EQuest_StampedePhase::StampedeActive && IsPlayerInDangerZone(PlayerLoc))
    {
        // Give a 3-second grace window before failing (handled by damage system in full impl)
        UE_LOG(LogTemp, Warning, TEXT("[Quest] DANGER: Player in stampede corridor! Z=%.0f"), PlayerLoc.Z);
    }

    // Time limit failure
    if (ElapsedStampedeTime >= StampedeTimeLimit)
    {
        FailQuest(TEXT("Time limit exceeded — stampede overran the valley"));
    }
}
