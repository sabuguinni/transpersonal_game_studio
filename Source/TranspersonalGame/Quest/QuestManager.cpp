#include "QuestManager.h"
#include "Engine/World.h"

// ─────────────────────────────────────────────────────────────────────────────
AQuestManager::AQuestManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = QuestTickInterval;
}

// ─────────────────────────────────────────────────────────────────────────────
void AQuestManager::BeginPlay()
{
    Super::BeginPlay();
    RegisterDefaultQuests();
}

// ─────────────────────────────────────────────────────────────────────────────
void AQuestManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    // Tick is intentionally lightweight — quest state is event-driven
}

// ─────────────────────────────────────────────────────────────────────────────
void AQuestManager::RegisterDefaultQuests()
{
    // ── Q1: Protect the Camp ─────────────────────────────────────────────
    {
        FQuest_Data Q;
        Q.QuestID          = "Q_ProtectCamp";
        Q.QuestTitle       = "Protect the Camp";
        Q.QuestDescription = "A T-Rex has been spotted near the camp. Keep the predator away until the tribe is safe.";
        Q.GiverActorLabel  = "CrowdNPC_Elder_001";
        Q.RewardFood       = 30.f;
        Q.RewardReputation = 20.f;

        FQuest_Objective Obj1;
        Obj1.ObjectiveID    = "OBJ_DriveAwayTRex";
        Obj1.Description    = "Drive the T-Rex away from the camp";
        Obj1.RequiredCount  = 1;
        Q.Objectives.Add(Obj1);

        FQuest_Objective Obj2;
        Obj2.ObjectiveID    = "OBJ_KeepCrowdSafe";
        Obj2.Description    = "Keep all tribe members alive";
        Obj2.RequiredCount  = 1;
        Q.Objectives.Add(Obj2);

        AllQuests.Add(Q);
    }

    // ── Q2: Track the Raptors ────────────────────────────────────────────
    {
        FQuest_Data Q;
        Q.QuestID          = "Q_TrackRaptors";
        Q.QuestTitle       = "Track the Raptors";
        Q.QuestDescription = "A raptor pack is circling the hunting grounds. Track them and drive them away before they attack.";
        Q.GiverActorLabel  = "CrowdNPC_Hunter_001";
        Q.RewardFood       = 20.f;
        Q.RewardReputation = 15.f;

        FQuest_Objective Obj1;
        Obj1.ObjectiveID   = "OBJ_FindRaptorPack";
        Obj1.Description   = "Locate the raptor pack";
        Obj1.RequiredCount = 1;
        Q.Objectives.Add(Obj1);

        FQuest_Objective Obj2;
        Obj2.ObjectiveID   = "OBJ_DriveRaptors";
        Obj2.Description   = "Drive away or kill 3 raptors";
        Obj2.RequiredCount = 3;
        Q.Objectives.Add(Obj2);

        AllQuests.Add(Q);
    }

    // ── Q3: Gather Supplies ──────────────────────────────────────────────
    {
        FQuest_Data Q;
        Q.QuestID          = "Q_GatherSupplies";
        Q.QuestTitle       = "Gather Supplies";
        Q.QuestDescription = "The tribe is running low on food. Gather berries and hunt small game before nightfall.";
        Q.GiverActorLabel  = "CrowdNPC_Gatherer_001";
        Q.RewardFood       = 10.f;
        Q.RewardReputation = 10.f;

        FQuest_Objective Obj1;
        Obj1.ObjectiveID   = "OBJ_GatherBerries";
        Obj1.Description   = "Gather 5 berry clusters";
        Obj1.RequiredCount = 5;
        Q.Objectives.Add(Obj1);

        FQuest_Objective Obj2;
        Obj2.ObjectiveID   = "OBJ_HuntSmallGame";
        Obj2.Description   = "Hunt 2 small animals";
        Obj2.RequiredCount = 2;
        Q.Objectives.Add(Obj2);

        AllQuests.Add(Q);
    }

    // ── Q4: Scout the Territory ──────────────────────────────────────────
    {
        FQuest_Data Q;
        Q.QuestID          = "Q_ScoutTerritory";
        Q.QuestTitle       = "Scout the Territory";
        Q.QuestDescription = "We need to know what lies beyond the ridge. Scout three key locations and report back.";
        Q.GiverActorLabel  = "CrowdNPC_Scout_001";
        Q.RewardFood       = 15.f;
        Q.RewardReputation = 25.f;

        FQuest_Objective Obj1;
        Obj1.ObjectiveID   = "OBJ_ReachRidge";
        Obj1.Description   = "Reach the eastern ridge";
        Obj1.RequiredCount = 1;
        Q.Objectives.Add(Obj1);

        FQuest_Objective Obj2;
        Obj2.ObjectiveID   = "OBJ_ReachRiver";
        Obj2.Description   = "Find the river crossing";
        Obj2.RequiredCount = 1;
        Q.Objectives.Add(Obj2);

        FQuest_Objective Obj3;
        Obj3.ObjectiveID   = "OBJ_ReturnToElder";
        Obj3.Description   = "Return to the Elder with your report";
        Obj3.RequiredCount = 1;
        Q.Objectives.Add(Obj3);

        AllQuests.Add(Q);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
bool AQuestManager::ActivateQuest(const FString& QuestID)
{
    int32 Idx = FindQuestIndex(QuestID);
    if (Idx == INDEX_NONE) return false;

    FQuest_Data& Q = AllQuests[Idx];
    if (Q.State != EQuest_State::Inactive) return false;

    Q.State = EQuest_State::Active;
    ActiveQuestIDs.AddUnique(QuestID);
    UE_LOG(LogTemp, Log, TEXT("[QuestManager] Quest ACTIVATED: %s"), *Q.QuestTitle);
    return true;
}

// ─────────────────────────────────────────────────────────────────────────────
bool AQuestManager::CompleteObjective(const FString& QuestID, const FString& ObjectiveID, int32 Count)
{
    int32 Idx = FindQuestIndex(QuestID);
    if (Idx == INDEX_NONE) return false;

    FQuest_Data& Q = AllQuests[Idx];
    if (Q.State != EQuest_State::Active) return false;

    for (FQuest_Objective& Obj : Q.Objectives)
    {
        if (Obj.ObjectiveID == ObjectiveID && !Obj.bCompleted)
        {
            Obj.CurrentCount = FMath::Min(Obj.CurrentCount + Count, Obj.RequiredCount);
            if (Obj.CurrentCount >= Obj.RequiredCount)
            {
                Obj.bCompleted = true;
                UE_LOG(LogTemp, Log, TEXT("[QuestManager] Objective DONE: %s / %s"), *QuestID, *ObjectiveID);
            }
            CheckQuestCompletion(Q);
            return true;
        }
    }
    return false;
}

// ─────────────────────────────────────────────────────────────────────────────
bool AQuestManager::FailQuest(const FString& QuestID)
{
    int32 Idx = FindQuestIndex(QuestID);
    if (Idx == INDEX_NONE) return false;

    FQuest_Data& Q = AllQuests[Idx];
    if (Q.State != EQuest_State::Active) return false;

    Q.State = EQuest_State::Failed;
    ActiveQuestIDs.Remove(QuestID);
    UE_LOG(LogTemp, Warning, TEXT("[QuestManager] Quest FAILED: %s"), *Q.QuestTitle);
    return true;
}

// ─────────────────────────────────────────────────────────────────────────────
FQuest_Data AQuestManager::GetQuestData(const FString& QuestID) const
{
    int32 Idx = FindQuestIndex(QuestID);
    if (Idx != INDEX_NONE) return AllQuests[Idx];
    return FQuest_Data();
}

// ─────────────────────────────────────────────────────────────────────────────
EQuest_State AQuestManager::GetQuestState(const FString& QuestID) const
{
    int32 Idx = FindQuestIndex(QuestID);
    if (Idx != INDEX_NONE) return AllQuests[Idx].State;
    return EQuest_State::Inactive;
}

// ─────────────────────────────────────────────────────────────────────────────
TArray<FQuest_Data> AQuestManager::GetActiveQuests() const
{
    TArray<FQuest_Data> Result;
    for (const FQuest_Data& Q : AllQuests)
    {
        if (Q.State == EQuest_State::Active)
        {
            Result.Add(Q);
        }
    }
    return Result;
}

// ─────────────────────────────────────────────────────────────────────────────
void AQuestManager::OnCrowdFleeEvent(float FearLevel)
{
    // High fear (>0.8) during "Protect the Camp" = quest failure condition
    if (FearLevel > 0.8f)
    {
        int32 Idx = FindQuestIndex("Q_ProtectCamp");
        if (Idx != INDEX_NONE && AllQuests[Idx].State == EQuest_State::Active)
        {
            UE_LOG(LogTemp, Warning, TEXT("[QuestManager] Crowd panic! Fear=%.2f — Q_ProtectCamp at risk"), FearLevel);
            // Fail if fear stays critical — gameplay loop handles timing
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
void AQuestManager::OnQuestTriggerEntered(const FString& TriggerID)
{
    if (TriggerID == "QuestTrigger_ProtectCamp")
    {
        ActivateQuest("Q_ProtectCamp");
    }
    else if (TriggerID == "QuestTrigger_TrackRaptors")
    {
        ActivateQuest("Q_TrackRaptors");
        CompleteObjective("Q_TrackRaptors", "OBJ_FindRaptorPack", 1);
    }
    else if (TriggerID == "QuestTrigger_GatherSupplies")
    {
        ActivateQuest("Q_GatherSupplies");
    }
}

// ─────────────────────────────────────────────────────────────────────────────
void AQuestManager::OnDinosaurKilled(const FString& DinoSpecies)
{
    if (DinoSpecies.Contains("Raptor"))
    {
        CompleteObjective("Q_TrackRaptors", "OBJ_DriveRaptors", 1);
    }
    else if (DinoSpecies.Contains("TRex") || DinoSpecies.Contains("Trex"))
    {
        CompleteObjective("Q_ProtectCamp", "OBJ_DriveAwayTRex", 1);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
void AQuestManager::OnResourceCollected(const FString& ResourceType, int32 Amount)
{
    if (ResourceType.Contains("Berry") || ResourceType.Contains("berry"))
    {
        CompleteObjective("Q_GatherSupplies", "OBJ_GatherBerries", Amount);
    }
    else if (ResourceType.Contains("SmallGame") || ResourceType.Contains("Animal"))
    {
        CompleteObjective("Q_GatherSupplies", "OBJ_HuntSmallGame", Amount);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
void AQuestManager::OnPlayerReachedLocation(const FString& LocationTag)
{
    if (LocationTag == "EasternRidge")
    {
        CompleteObjective("Q_ScoutTerritory", "OBJ_ReachRidge", 1);
    }
    else if (LocationTag == "RiverCrossing")
    {
        CompleteObjective("Q_ScoutTerritory", "OBJ_ReachRiver", 1);
    }
    else if (LocationTag == "ElderCamp")
    {
        CompleteObjective("Q_ScoutTerritory", "OBJ_ReturnToElder", 1);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
void AQuestManager::CheckQuestCompletion(FQuest_Data& Quest)
{
    if (Quest.State != EQuest_State::Active) return;

    bool bAllDone = true;
    for (const FQuest_Objective& Obj : Quest.Objectives)
    {
        if (!Obj.bCompleted)
        {
            bAllDone = false;
            break;
        }
    }

    if (bAllDone)
    {
        Quest.State = EQuest_State::Completed;
        ActiveQuestIDs.Remove(Quest.QuestID);
        UE_LOG(LogTemp, Log, TEXT("[QuestManager] Quest COMPLETED: %s | Reward Food=%.0f Rep=%.0f"),
            *Quest.QuestTitle, Quest.RewardFood, Quest.RewardReputation);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
int32 AQuestManager::FindQuestIndex(const FString& QuestID) const
{
    for (int32 i = 0; i < AllQuests.Num(); ++i)
    {
        if (AllQuests[i].QuestID == QuestID) return i;
    }
    return INDEX_NONE;
}
