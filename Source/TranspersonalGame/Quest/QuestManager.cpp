// QuestManager.cpp
// Agent #14 — Quest & Mission Designer
// PROD_CYCLE_AUTO_20260628_007
// Full implementation of quest lifecycle, objective tracking, and stampede-triggered quests

#include "Quest/QuestManager.h"
#include "TranspersonalCharacter.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

AQuestManager::AQuestManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f;

    ObjectiveCheckInterval = 0.5f;
    StampedeWitnessRadius = 2000.0f;
    PredatorTrackRadius = 800.0f;
    MaxActiveQuests = 5;
}

void AQuestManager::BeginPlay()
{
    Super::BeginPlay();

    // Find player character
    AActor* PlayerActor = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (PlayerActor)
    {
        PlayerCharacter = Cast<ATranspersonalCharacter>(PlayerActor);
    }

    // Build the quest library with pre-defined quests
    BuildQuestLibrary();

    // Start periodic objective check timer
    GetWorldTimerManager().SetTimer(
        ObjectiveCheckTimer,
        this,
        &AQuestManager::PeriodicObjectiveCheck,
        ObjectiveCheckInterval,
        true
    );

    UE_LOG(LogTemp, Log, TEXT("QuestManager initialized — %d quests in library"), QuestLibrary.Num());
}

void AQuestManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

// ─── Quest Lifecycle ───────────────────────────────────────────────────────────

bool AQuestManager::StartQuest(FName QuestID)
{
    if (ActiveQuests.Num() >= MaxActiveQuests)
    {
        UE_LOG(LogTemp, Warning, TEXT("QuestManager: Cannot start quest %s — max active quests reached"), *QuestID.ToString());
        return false;
    }

    if (ActiveQuests.Contains(QuestID))
    {
        UE_LOG(LogTemp, Warning, TEXT("QuestManager: Quest %s already active"), *QuestID.ToString());
        return false;
    }

    if (CompletedQuestIDs.Contains(QuestID))
    {
        UE_LOG(LogTemp, Warning, TEXT("QuestManager: Quest %s already completed"), *QuestID.ToString());
        return false;
    }

    FQuest_QuestData* FoundQuest = FindQuestInLibrary(QuestID);
    if (!FoundQuest)
    {
        UE_LOG(LogTemp, Error, TEXT("QuestManager: Quest %s not found in library"), *QuestID.ToString());
        return false;
    }

    // Copy quest data into active map
    FQuest_QuestData ActiveQuest = *FoundQuest;
    ActiveQuest.Status = EQuest_Status::Active;
    ActiveQuests.Add(QuestID, ActiveQuest);

    // Initialize objective progress
    for (const FQuest_ObjectiveData& Obj : ActiveQuest.Objectives)
    {
        FName ProgressKey = MakeProgressKey(QuestID, Obj.ObjectiveID);
        ObjectiveProgressCache.Add(ProgressKey, 0);
    }

    UE_LOG(LogTemp, Log, TEXT("QuestManager: Quest STARTED — %s"), *ActiveQuest.Title.ToString());
    OnQuestStarted.Broadcast(ActiveQuest);
    return true;
}

bool AQuestManager::AbandonQuest(FName QuestID)
{
    if (!ActiveQuests.Contains(QuestID))
    {
        return false;
    }

    FQuest_QuestData& Quest = ActiveQuests[QuestID];
    Quest.Status = EQuest_Status::Abandoned;
    ActiveQuests.Remove(QuestID);

    UE_LOG(LogTemp, Log, TEXT("QuestManager: Quest ABANDONED — %s"), *QuestID.ToString());
    OnQuestCompleted.Broadcast(QuestID, false);
    return true;
}

bool AQuestManager::CompleteQuest(FName QuestID, bool bSuccess)
{
    if (!ActiveQuests.Contains(QuestID))
    {
        return false;
    }

    FQuest_QuestData& Quest = ActiveQuests[QuestID];
    Quest.Status = bSuccess ? EQuest_Status::Completed : EQuest_Status::Failed;

    CompletedQuestIDs.Add(QuestID);
    ActiveQuests.Remove(QuestID);

    UE_LOG(LogTemp, Log, TEXT("QuestManager: Quest %s — %s"), *QuestID.ToString(), bSuccess ? TEXT("COMPLETED") : TEXT("FAILED"));
    OnQuestCompleted.Broadcast(QuestID, bSuccess);
    return true;
}

// ─── Objective Tracking ────────────────────────────────────────────────────────

void AQuestManager::UpdateObjectiveProgress(FName QuestID, FName ObjectiveID, int32 ProgressDelta)
{
    if (!ActiveQuests.Contains(QuestID))
    {
        return;
    }

    FName ProgressKey = MakeProgressKey(QuestID, ObjectiveID);
    int32& CurrentProgress = ObjectiveProgressCache.FindOrAdd(ProgressKey);
    CurrentProgress += ProgressDelta;

    FQuest_QuestData& Quest = ActiveQuests[QuestID];
    FQuest_ObjectiveData* Objective = FindObjectiveInQuest(Quest, ObjectiveID);
    if (!Objective)
    {
        return;
    }

    Objective->CurrentCount = CurrentProgress;

    if (CurrentProgress >= Objective->RequiredCount && !Objective->bIsComplete)
    {
        MarkObjectiveComplete(QuestID, ObjectiveID);
    }
    else
    {
        OnQuestObjectiveUpdated.Broadcast(QuestID, *Objective);
    }
}

void AQuestManager::MarkObjectiveComplete(FName QuestID, FName ObjectiveID)
{
    if (!ActiveQuests.Contains(QuestID))
    {
        return;
    }

    FQuest_QuestData& Quest = ActiveQuests[QuestID];
    FQuest_ObjectiveData* Objective = FindObjectiveInQuest(Quest, ObjectiveID);
    if (!Objective || Objective->bIsComplete)
    {
        return;
    }

    Objective->bIsComplete = true;
    Objective->CurrentCount = Objective->RequiredCount;

    UE_LOG(LogTemp, Log, TEXT("QuestManager: Objective COMPLETE — %s in quest %s"), *ObjectiveID.ToString(), *QuestID.ToString());
    OnQuestObjectiveUpdated.Broadcast(QuestID, *Objective);

    // Check if all required objectives are done
    CheckAllObjectivesForQuest(QuestID);
}

bool AQuestManager::IsObjectiveComplete(FName QuestID, FName ObjectiveID) const
{
    if (!ActiveQuests.Contains(QuestID))
    {
        return false;
    }

    const FQuest_QuestData& Quest = ActiveQuests[QuestID];
    for (const FQuest_ObjectiveData& Obj : Quest.Objectives)
    {
        if (Obj.ObjectiveID == ObjectiveID)
        {
            return Obj.bIsComplete;
        }
    }
    return false;
}

// ─── Query ─────────────────────────────────────────────────────────────────────

bool AQuestManager::IsQuestActive(FName QuestID) const
{
    return ActiveQuests.Contains(QuestID);
}

EQuest_Status AQuestManager::GetQuestStatus(FName QuestID) const
{
    if (ActiveQuests.Contains(QuestID))
    {
        return ActiveQuests[QuestID].Status;
    }
    if (CompletedQuestIDs.Contains(QuestID))
    {
        return EQuest_Status::Completed;
    }
    return EQuest_Status::Inactive;
}

TArray<FQuest_QuestData> AQuestManager::GetActiveQuests() const
{
    TArray<FQuest_QuestData> Result;
    for (const auto& Pair : ActiveQuests)
    {
        Result.Add(Pair.Value);
    }
    return Result;
}

int32 AQuestManager::GetActiveQuestCount() const
{
    return ActiveQuests.Num();
}

// ─── Stampede Integration ──────────────────────────────────────────────────────

void AQuestManager::OnStampedeWitnessed(FVector StampedeLocation, FVector ThreatLocation)
{
    UE_LOG(LogTemp, Log, TEXT("QuestManager: Stampede witnessed at %s — threat at %s"),
        *StampedeLocation.ToString(), *ThreatLocation.ToString());

    // Auto-start escape quest if not already active
    if (!IsQuestActive(FName("Quest_EscapeStampede")))
    {
        StartQuest(FName("Quest_EscapeStampede"));
    }

    // Auto-start predator tracking quest
    if (!IsQuestActive(FName("Quest_TrackThePredator")) &&
        !CompletedQuestIDs.Contains(FName("Quest_TrackThePredator")))
    {
        StartQuest(FName("Quest_TrackThePredator"));
    }
}

void AQuestManager::OnPlayerEscapedStampedePath()
{
    if (IsQuestActive(FName("Quest_EscapeStampede")))
    {
        MarkObjectiveComplete(FName("Quest_EscapeStampede"), FName("Obj_ExitDangerZone"));
    }
}

void AQuestManager::OnPredatorTracked(FVector PredatorLocation)
{
    if (IsQuestActive(FName("Quest_TrackThePredator")))
    {
        UpdateObjectiveProgress(FName("Quest_TrackThePredator"), FName("Obj_TrackPredator"), 1);
    }
}

// ─── Internal Helpers ──────────────────────────────────────────────────────────

FQuest_QuestData* AQuestManager::FindQuestInLibrary(FName QuestID)
{
    for (FQuest_QuestData& Quest : QuestLibrary)
    {
        if (Quest.QuestID == QuestID)
        {
            return &Quest;
        }
    }
    return nullptr;
}

FQuest_ObjectiveData* AQuestManager::FindObjectiveInQuest(FQuest_QuestData& Quest, FName ObjectiveID)
{
    for (FQuest_ObjectiveData& Obj : Quest.Objectives)
    {
        if (Obj.ObjectiveID == ObjectiveID)
        {
            return &Obj;
        }
    }
    return nullptr;
}

void AQuestManager::CheckAllObjectivesForQuest(FName QuestID)
{
    if (!ActiveQuests.Contains(QuestID))
    {
        return;
    }

    const FQuest_QuestData& Quest = ActiveQuests[QuestID];
    if (AreAllRequiredObjectivesComplete(Quest))
    {
        CompleteQuest(QuestID, true);
    }
}

void AQuestManager::PeriodicObjectiveCheck()
{
    // Periodic check for location-based objectives
    if (!PlayerCharacter)
    {
        return;
    }

    FVector PlayerPos = PlayerCharacter->GetActorLocation();

    for (auto& Pair : ActiveQuests)
    {
        FName QuestID = Pair.Key;
        FQuest_QuestData& Quest = Pair.Value;

        for (FQuest_ObjectiveData& Obj : Quest.Objectives)
        {
            if (Obj.bIsComplete)
            {
                continue;
            }

            if (Obj.ObjectiveType == EQuest_ObjectiveType::ReachLocation)
            {
                float Distance = FVector::Dist(PlayerPos, Obj.TargetLocation);
                if (Distance <= Obj.InteractionRadius)
                {
                    MarkObjectiveComplete(QuestID, Obj.ObjectiveID);
                }
            }
        }
    }
}

bool AQuestManager::AreAllRequiredObjectivesComplete(const FQuest_QuestData& Quest) const
{
    for (const FQuest_ObjectiveData& Obj : Quest.Objectives)
    {
        if (!Obj.bIsOptional && !Obj.bIsComplete)
        {
            return false;
        }
    }
    return true;
}

FName AQuestManager::MakeProgressKey(FName QuestID, FName ObjectiveID) const
{
    return FName(*(QuestID.ToString() + TEXT("_") + ObjectiveID.ToString()));
}

// ─── Quest Library Builder ─────────────────────────────────────────────────────

void AQuestManager::BuildQuestLibrary()
{
    QuestLibrary.Empty();
    QuestLibrary.Add(BuildQuest_EscapeStampede());
    QuestLibrary.Add(BuildQuest_TrackThePredator());
    QuestLibrary.Add(BuildQuest_RescueMissingHunters());
    QuestLibrary.Add(BuildQuest_SalvageSupplies());

    UE_LOG(LogTemp, Log, TEXT("QuestManager: Built %d quests in library"), QuestLibrary.Num());
}

FQuest_QuestData AQuestManager::BuildQuest_EscapeStampede()
{
    FQuest_QuestData Quest;
    Quest.QuestID = FName("Quest_EscapeStampede");
    Quest.Title = FText::FromString(TEXT("Escape the Stampede"));
    Quest.Description = FText::FromString(TEXT("A massive herd is in full panic. Get out of the stampede path before you are trampled."));
    Quest.Status = EQuest_Status::Inactive;
    Quest.QuestType = EQuest_Type::Survival;
    Quest.Priority = EQuest_Priority::Critical;
    Quest.bIsMainQuest = false;
    Quest.bHasTimeLimit = true;
    Quest.TimeLimitSeconds = 60.0f;

    // Objective 1: Exit the danger zone
    FQuest_ObjectiveData ObjExit;
    ObjExit.ObjectiveID = FName("Obj_ExitDangerZone");
    ObjExit.Description = FText::FromString(TEXT("Escape the stampede path"));
    ObjExit.ObjectiveType = EQuest_ObjectiveType::ReachLocation;
    ObjExit.TargetLocation = FVector(0.0f, -3000.0f, 0.0f); // Safe zone west of stampede
    ObjExit.InteractionRadius = 500.0f;
    ObjExit.RequiredCount = 1;
    ObjExit.CurrentCount = 0;
    ObjExit.bIsComplete = false;
    ObjExit.bIsOptional = false;
    Quest.Objectives.Add(ObjExit);

    // Objective 2: Survive without taking lethal damage (optional bonus)
    FQuest_ObjectiveData ObjSurvive;
    ObjSurvive.ObjectiveID = FName("Obj_SurviveUnharmed");
    ObjSurvive.Description = FText::FromString(TEXT("Escape without being trampled (bonus)"));
    ObjSurvive.ObjectiveType = EQuest_ObjectiveType::Survive;
    ObjSurvive.RequiredCount = 1;
    ObjSurvive.CurrentCount = 0;
    ObjSurvive.bIsComplete = false;
    ObjSurvive.bIsOptional = true;
    Quest.Objectives.Add(ObjSurvive);

    return Quest;
}

FQuest_QuestData AQuestManager::BuildQuest_TrackThePredator()
{
    FQuest_QuestData Quest;
    Quest.QuestID = FName("Quest_TrackThePredator");
    Quest.Title = FText::FromString(TEXT("Track the Predator"));
    Quest.Description = FText::FromString(TEXT("The great predator that caused the stampede still hunts these lands. Track it to its territory and drive it away from camp."));
    Quest.Status = EQuest_Status::Inactive;
    Quest.QuestType = EQuest_Type::Hunt;
    Quest.Priority = EQuest_Priority::High;
    Quest.bIsMainQuest = false;
    Quest.bHasTimeLimit = false;

    // Objective 1: Follow the trail east
    FQuest_ObjectiveData ObjTrail;
    ObjTrail.ObjectiveID = FName("Obj_FollowTrail");
    ObjTrail.Description = FText::FromString(TEXT("Follow the crushed vegetation trail east"));
    ObjTrail.ObjectiveType = EQuest_ObjectiveType::ReachLocation;
    ObjTrail.TargetLocation = FVector(2500.0f, 0.0f, 0.0f); // Midpoint east
    ObjTrail.InteractionRadius = 400.0f;
    ObjTrail.RequiredCount = 1;
    ObjTrail.CurrentCount = 0;
    ObjTrail.bIsComplete = false;
    ObjTrail.bIsOptional = false;
    Quest.Objectives.Add(ObjTrail);

    // Objective 2: Track the predator (approach TRex marker at 5000,0,0)
    FQuest_ObjectiveData ObjTrack;
    ObjTrack.ObjectiveID = FName("Obj_TrackPredator");
    ObjTrack.Description = FText::FromString(TEXT("Locate the predator's territory"));
    ObjTrack.ObjectiveType = EQuest_ObjectiveType::ReachLocation;
    ObjTrack.TargetLocation = FVector(5000.0f, 0.0f, 0.0f); // TRex threat marker from cycle 007
    ObjTrack.InteractionRadius = 600.0f;
    ObjTrack.RequiredCount = 1;
    ObjTrack.CurrentCount = 0;
    ObjTrack.bIsComplete = false;
    ObjTrack.bIsOptional = false;
    Quest.Objectives.Add(ObjTrack);

    // Objective 3: Drive it away (use fire or noise — interact with campfire trigger)
    FQuest_ObjectiveData ObjDrive;
    ObjDrive.ObjectiveID = FName("Obj_DriveAwayPredator");
    ObjDrive.Description = FText::FromString(TEXT("Drive the predator away from camp territory"));
    ObjDrive.ObjectiveType = EQuest_ObjectiveType::UseItem;
    ObjDrive.RequiredCount = 1;
    ObjDrive.CurrentCount = 0;
    ObjDrive.bIsComplete = false;
    ObjDrive.bIsOptional = false;
    Quest.Objectives.Add(ObjDrive);

    return Quest;
}

FQuest_QuestData AQuestManager::BuildQuest_RescueMissingHunters()
{
    FQuest_QuestData Quest;
    Quest.QuestID = FName("Quest_RescueMissingHunters");
    Quest.Title = FText::FromString(TEXT("Find the Missing Hunters"));
    Quest.Description = FText::FromString(TEXT("Three hunters were last seen near the valley floor when the herd broke through. Find them before the predators do."));
    Quest.Status = EQuest_Status::Inactive;
    Quest.QuestType = EQuest_Type::Rescue;
    Quest.Priority = EQuest_Priority::High;
    Quest.bIsMainQuest = false;
    Quest.bHasTimeLimit = true;
    Quest.TimeLimitSeconds = 300.0f; // 5 minutes

    // Objective: Find hunter 1
    FQuest_ObjectiveData ObjHunter1;
    ObjHunter1.ObjectiveID = FName("Obj_FindHunter1");
    ObjHunter1.Description = FText::FromString(TEXT("Find Kael near the river crossing"));
    ObjHunter1.ObjectiveType = EQuest_ObjectiveType::ReachLocation;
    ObjHunter1.TargetLocation = FVector(-800.0f, 1200.0f, 0.0f);
    ObjHunter1.InteractionRadius = 300.0f;
    ObjHunter1.RequiredCount = 1;
    ObjHunter1.CurrentCount = 0;
    ObjHunter1.bIsComplete = false;
    ObjHunter1.bIsOptional = false;
    Quest.Objectives.Add(ObjHunter1);

    // Objective: Find hunter 2
    FQuest_ObjectiveData ObjHunter2;
    ObjHunter2.ObjectiveID = FName("Obj_FindHunter2");
    ObjHunter2.Description = FText::FromString(TEXT("Find Mira trapped under debris"));
    ObjHunter2.ObjectiveType = EQuest_ObjectiveType::ReachLocation;
    ObjHunter2.TargetLocation = FVector(-1200.0f, -400.0f, 0.0f);
    ObjHunter2.InteractionRadius = 300.0f;
    ObjHunter2.RequiredCount = 1;
    ObjHunter2.CurrentCount = 0;
    ObjHunter2.bIsComplete = false;
    ObjHunter2.bIsOptional = false;
    Quest.Objectives.Add(ObjHunter2);

    // Objective: Find hunter 3
    FQuest_ObjectiveData ObjHunter3;
    ObjHunter3.ObjectiveID = FName("Obj_FindHunter3");
    ObjHunter3.Description = FText::FromString(TEXT("Find Darro hiding in the rock formation"));
    ObjHunter3.ObjectiveType = EQuest_ObjectiveType::ReachLocation;
    ObjHunter3.TargetLocation = FVector(-500.0f, 800.0f, 0.0f);
    ObjHunter3.InteractionRadius = 300.0f;
    ObjHunter3.RequiredCount = 1;
    ObjHunter3.CurrentCount = 0;
    ObjHunter3.bIsComplete = false;
    ObjHunter3.bIsOptional = false;
    Quest.Objectives.Add(ObjHunter3);

    return Quest;
}

FQuest_QuestData AQuestManager::BuildQuest_SalvageSupplies()
{
    FQuest_QuestData Quest;
    Quest.QuestID = FName("Quest_SalvageSupplies");
    Quest.Title = FText::FromString(TEXT("Salvage the Food Stores"));
    Quest.Description = FText::FromString(TEXT("The stampede destroyed our food stores. Salvage whatever you can from the wreckage before scavengers take it."));
    Quest.Status = EQuest_Status::Inactive;
    Quest.QuestType = EQuest_Type::Gather;
    Quest.Priority = EQuest_Priority::Medium;
    Quest.bIsMainQuest = false;
    Quest.bHasTimeLimit = true;
    Quest.TimeLimitSeconds = 180.0f; // 3 minutes

    // Objective: Collect meat
    FQuest_ObjectiveData ObjMeat;
    ObjMeat.ObjectiveID = FName("Obj_CollectMeat");
    ObjMeat.Description = FText::FromString(TEXT("Collect dried meat from the wreckage (0/5)"));
    ObjMeat.ObjectiveType = EQuest_ObjectiveType::CollectItem;
    ObjMeat.RequiredCount = 5;
    ObjMeat.CurrentCount = 0;
    ObjMeat.bIsComplete = false;
    ObjMeat.bIsOptional = false;
    Quest.Objectives.Add(ObjMeat);

    // Objective: Collect water containers
    FQuest_ObjectiveData ObjWater;
    ObjWater.ObjectiveID = FName("Obj_CollectWater");
    ObjWater.Description = FText::FromString(TEXT("Recover water containers (0/3)"));
    ObjWater.ObjectiveType = EQuest_ObjectiveType::CollectItem;
    ObjWater.RequiredCount = 3;
    ObjWater.CurrentCount = 0;
    ObjWater.bIsComplete = false;
    ObjWater.bIsOptional = false;
    Quest.Objectives.Add(ObjWater);

    // Objective: Collect tools (optional)
    FQuest_ObjectiveData ObjTools;
    ObjTools.ObjectiveID = FName("Obj_CollectTools");
    ObjTools.Description = FText::FromString(TEXT("Recover stone tools if possible (bonus)"));
    ObjTools.ObjectiveType = EQuest_ObjectiveType::CollectItem;
    ObjTools.RequiredCount = 2;
    ObjTools.CurrentCount = 0;
    ObjTools.bIsComplete = false;
    ObjTools.bIsOptional = true;
    Quest.Objectives.Add(ObjTools);

    return Quest;
}
