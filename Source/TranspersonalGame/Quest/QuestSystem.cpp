#include "QuestSystem.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

// ============================================================
// Constructor
// ============================================================

AQuestSystemManager::AQuestSystemManager()
{
    PrimaryActorTick.bCanEverTick = true;
    MaxActiveQuests = 5;
}

// ============================================================
// BeginPlay — register default quests
// ============================================================

void AQuestSystemManager::BeginPlay()
{
    Super::BeginPlay();
    RegisterDefaultQuests();
    UE_LOG(LogTemp, Log, TEXT("[QuestSystem] Initialized — %d default quests registered"), ActiveQuestMap.Num());
}

// ============================================================
// Tick — time-limited quest countdown
// ============================================================

void AQuestSystemManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    TickTimeLimitedQuests(DeltaTime);
}

// ============================================================
// Quest Lifecycle
// ============================================================

bool AQuestSystemManager::StartQuest(FName QuestID)
{
    if (ActiveQuestMap.Contains(QuestID))
    {
        UE_LOG(LogTemp, Warning, TEXT("[QuestSystem] Quest already active: %s"), *QuestID.ToString());
        return false;
    }

    if (ActiveQuestMap.Num() >= MaxActiveQuests)
    {
        UE_LOG(LogTemp, Warning, TEXT("[QuestSystem] Max active quests reached (%d)"), MaxActiveQuests);
        return false;
    }

    // Find quest in available pool
    FQuest_ActiveEntry* ExistingEntry = ActiveQuestMap.Find(QuestID);
    if (!ExistingEntry)
    {
        UE_LOG(LogTemp, Warning, TEXT("[QuestSystem] Quest not found in registry: %s"), *QuestID.ToString());
        return false;
    }

    ExistingEntry->State = EQuest_State::Active;
    ExistingEntry->TimeStarted = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
    if (ExistingEntry->Data.bHasTimeLimit)
    {
        ExistingEntry->TimeRemaining = ExistingEntry->Data.TimeLimit;
    }

    OnQuestStarted.Broadcast(QuestID, ExistingEntry->Data);
    UE_LOG(LogTemp, Log, TEXT("[QuestSystem] Quest started: %s"), *QuestID.ToString());
    return true;
}

bool AQuestSystemManager::CompleteQuest(FName QuestID)
{
    FQuest_ActiveEntry* Entry = ActiveQuestMap.Find(QuestID);
    if (!Entry || Entry->State != EQuest_State::Active)
    {
        return false;
    }

    Entry->State = EQuest_State::Completed;
    FQuest_ActiveEntry Completed = *Entry;
    CompletedQuestList.Add(Completed);
    ActiveQuestMap.Remove(QuestID);

    OnQuestCompleted.Broadcast(QuestID, Completed.Data.Reward);
    UE_LOG(LogTemp, Log, TEXT("[QuestSystem] Quest completed: %s | XP: %d"), *QuestID.ToString(), Completed.Data.Reward.ExperiencePoints);
    return true;
}

bool AQuestSystemManager::FailQuest(FName QuestID)
{
    FQuest_ActiveEntry* Entry = ActiveQuestMap.Find(QuestID);
    if (!Entry)
    {
        return false;
    }

    Entry->State = EQuest_State::Failed;
    FailedQuestIDs.Add(QuestID);
    ActiveQuestMap.Remove(QuestID);

    OnQuestFailed.Broadcast(QuestID);
    UE_LOG(LogTemp, Log, TEXT("[QuestSystem] Quest failed: %s"), *QuestID.ToString());
    return true;
}

bool AQuestSystemManager::AbandonQuest(FName QuestID)
{
    if (!ActiveQuestMap.Contains(QuestID))
    {
        return false;
    }
    ActiveQuestMap.Remove(QuestID);
    UE_LOG(LogTemp, Log, TEXT("[QuestSystem] Quest abandoned: %s"), *QuestID.ToString());
    return true;
}

// ============================================================
// Objective Tracking
// ============================================================

void AQuestSystemManager::ReportObjectiveProgress(FName QuestID, FName ObjectiveID, int32 ProgressAmount)
{
    FQuest_ActiveEntry* Entry = ActiveQuestMap.Find(QuestID);
    if (!Entry || Entry->State != EQuest_State::Active)
    {
        return;
    }

    for (FQuest_Objective& Obj : Entry->Data.Objectives)
    {
        if (Obj.ObjectiveID == ObjectiveID && !Obj.bIsCompleted)
        {
            Obj.CurrentCount = FMath::Min(Obj.CurrentCount + ProgressAmount, Obj.RequiredCount);
            if (Obj.CurrentCount >= Obj.RequiredCount)
            {
                Obj.bIsCompleted = true;
                UE_LOG(LogTemp, Log, TEXT("[QuestSystem] Objective completed: %s / %s"), *QuestID.ToString(), *ObjectiveID.ToString());
            }
            OnObjectiveUpdated.Broadcast(QuestID, Obj);
            CheckObjectiveCompletion(QuestID);
            return;
        }
    }
}

void AQuestSystemManager::ReportLocationReached(FVector PlayerLocation)
{
    for (auto& Pair : ActiveQuestMap)
    {
        if (Pair.Value.State != EQuest_State::Active) continue;

        for (FQuest_Objective& Obj : Pair.Value.Data.Objectives)
        {
            if (Obj.ObjectiveType == EQuest_ObjectiveType::ReachLocation && !Obj.bIsCompleted)
            {
                float Dist = FVector::Dist(PlayerLocation, Obj.TargetLocation);
                if (Dist <= Obj.LocationRadius)
                {
                    ReportObjectiveProgress(Pair.Key, Obj.ObjectiveID, 1);
                }
            }
        }
    }
}

void AQuestSystemManager::ReportKill(FName TargetTag)
{
    for (auto& Pair : ActiveQuestMap)
    {
        if (Pair.Value.State != EQuest_State::Active) continue;

        for (FQuest_Objective& Obj : Pair.Value.Data.Objectives)
        {
            if (Obj.ObjectiveType == EQuest_ObjectiveType::KillTarget
                && Obj.TargetTag == TargetTag
                && !Obj.bIsCompleted)
            {
                ReportObjectiveProgress(Pair.Key, Obj.ObjectiveID, 1);
            }
        }
    }
}

void AQuestSystemManager::ReportItemCollected(FName ItemTag)
{
    for (auto& Pair : ActiveQuestMap)
    {
        if (Pair.Value.State != EQuest_State::Active) continue;

        for (FQuest_Objective& Obj : Pair.Value.Data.Objectives)
        {
            if (Obj.ObjectiveType == EQuest_ObjectiveType::CollectResource
                && Obj.TargetTag == ItemTag
                && !Obj.bIsCompleted)
            {
                ReportObjectiveProgress(Pair.Key, Obj.ObjectiveID, 1);
            }
        }
    }
}

void AQuestSystemManager::ReportItemCrafted(FName RecipeID)
{
    for (auto& Pair : ActiveQuestMap)
    {
        if (Pair.Value.State != EQuest_State::Active) continue;

        for (FQuest_Objective& Obj : Pair.Value.Data.Objectives)
        {
            if (Obj.ObjectiveType == EQuest_ObjectiveType::CraftItem
                && Obj.TargetTag == RecipeID
                && !Obj.bIsCompleted)
            {
                ReportObjectiveProgress(Pair.Key, Obj.ObjectiveID, 1);
            }
        }
    }
}

// ============================================================
// Query
// ============================================================

EQuest_State AQuestSystemManager::GetQuestState(FName QuestID) const
{
    const FQuest_ActiveEntry* Entry = ActiveQuestMap.Find(QuestID);
    if (Entry) return Entry->State;
    if (FailedQuestIDs.Contains(QuestID)) return EQuest_State::Failed;
    for (const FQuest_ActiveEntry& Completed : CompletedQuestList)
    {
        if (Completed.Data.QuestID == QuestID) return EQuest_State::Completed;
    }
    return EQuest_State::Locked;
}

bool AQuestSystemManager::IsQuestActive(FName QuestID) const
{
    const FQuest_ActiveEntry* Entry = ActiveQuestMap.Find(QuestID);
    return Entry && Entry->State == EQuest_State::Active;
}

TArray<FQuest_ActiveEntry> AQuestSystemManager::GetActiveQuests() const
{
    TArray<FQuest_ActiveEntry> Result;
    for (const auto& Pair : ActiveQuestMap)
    {
        if (Pair.Value.State == EQuest_State::Active)
        {
            Result.Add(Pair.Value);
        }
    }
    return Result;
}

TArray<FQuest_ActiveEntry> AQuestSystemManager::GetCompletedQuests() const
{
    return CompletedQuestList;
}

int32 AQuestSystemManager::GetActiveQuestCount() const
{
    int32 Count = 0;
    for (const auto& Pair : ActiveQuestMap)
    {
        if (Pair.Value.State == EQuest_State::Active) Count++;
    }
    return Count;
}

// ============================================================
// Crowd Integration Events
// ============================================================

void AQuestSystemManager::OnStampedeTriggered(FVector StampedeOrigin, float StampedeRadius)
{
    // Trigger "Survive the Stampede" quest if available
    const FName StampedeQuestID = FName("Quest_SurviveStampede");
    if (ActiveQuestMap.Contains(StampedeQuestID))
    {
        FQuest_ActiveEntry* Entry = ActiveQuestMap.Find(StampedeQuestID);
        if (Entry && Entry->State == EQuest_State::Available)
        {
            StartQuest(StampedeQuestID);
            UE_LOG(LogTemp, Log, TEXT("[QuestSystem] Stampede event triggered Quest_SurviveStampede"));
        }
    }
}

void AQuestSystemManager::OnPredatorDetected(FVector PredatorLocation, FName PredatorSpecies)
{
    // Report predator kill objectives
    ReportKill(PredatorSpecies);
    UE_LOG(LogTemp, Log, TEXT("[QuestSystem] Predator detected: %s at (%.0f, %.0f, %.0f)"),
        *PredatorSpecies.ToString(), PredatorLocation.X, PredatorLocation.Y, PredatorLocation.Z);
}

// ============================================================
// Internal Helpers
// ============================================================

void AQuestSystemManager::TickTimeLimitedQuests(float DeltaTime)
{
    TArray<FName> ToFail;
    for (auto& Pair : ActiveQuestMap)
    {
        if (Pair.Value.State == EQuest_State::Active && Pair.Value.Data.bHasTimeLimit)
        {
            Pair.Value.TimeRemaining -= DeltaTime;
            if (Pair.Value.TimeRemaining <= 0.f)
            {
                ToFail.Add(Pair.Key);
            }
        }
    }
    for (FName QID : ToFail)
    {
        FailQuest(QID);
    }
}

void AQuestSystemManager::CheckObjectiveCompletion(FName QuestID)
{
    FQuest_ActiveEntry* Entry = ActiveQuestMap.Find(QuestID);
    if (!Entry || Entry->State != EQuest_State::Active) return;

    bool bAllMandatoryDone = true;
    for (const FQuest_Objective& Obj : Entry->Data.Objectives)
    {
        if (!Obj.bIsOptional && !Obj.bIsCompleted)
        {
            bAllMandatoryDone = false;
            break;
        }
    }

    if (bAllMandatoryDone)
    {
        CompleteQuest(QuestID);
    }
}

bool AQuestSystemManager::ArePrerequisitesMet(const FQuest_Data& QuestData) const
{
    for (const FName& PrereqID : QuestData.PrerequisiteQuestIDs)
    {
        bool bFound = false;
        for (const FQuest_ActiveEntry& Completed : CompletedQuestList)
        {
            if (Completed.Data.QuestID == PrereqID)
            {
                bFound = true;
                break;
            }
        }
        if (!bFound) return false;
    }
    return true;
}

void AQuestSystemManager::RegisterDefaultQuests()
{
    // Quest 1: Rescue the Hunters
    {
        FQuest_Data Q = BuildRescueHuntersQuest();
        FQuest_ActiveEntry Entry;
        Entry.Data = Q;
        Entry.State = EQuest_State::Available;
        ActiveQuestMap.Add(Q.QuestID, Entry);
    }

    // Quest 2: Survive the Stampede
    {
        FQuest_Data Q = BuildSurviveStampedeQuest();
        FQuest_ActiveEntry Entry;
        Entry.Data = Q;
        Entry.State = EQuest_State::Available;
        ActiveQuestMap.Add(Q.QuestID, Entry);
    }

    // Quest 3: Craft a Stone Axe
    {
        FQuest_Data Q = BuildCraftStoneAxeQuest();
        FQuest_ActiveEntry Entry;
        Entry.Data = Q;
        Entry.State = EQuest_State::Available;
        ActiveQuestMap.Add(Q.QuestID, Entry);
    }
}

FQuest_Data AQuestSystemManager::BuildRescueHuntersQuest() const
{
    FQuest_Data Q;
    Q.QuestID = FName("Quest_RescueHunters");
    Q.Title = FText::FromString("Rescue the Hunters");
    Q.Description = FText::FromString("Three hunters are trapped near the river crossing. Reach them before the raptor pack closes in.");
    Q.Category = EQuest_Category::Rescue;
    Q.Difficulty = EQuest_Difficulty::Hard;
    Q.bHasTimeLimit = true;
    Q.TimeLimit = 180.f;
    Q.GiverNPCTag = FName("NPC_Elder");

    FQuest_Objective ObjReach;
    ObjReach.ObjectiveID = FName("Obj_ReachRiverCrossing");
    ObjReach.Description = FText::FromString("Reach the river crossing");
    ObjReach.ObjectiveType = EQuest_ObjectiveType::ReachLocation;
    ObjReach.TargetLocation = FVector(3200.f, -800.f, 50.f);
    ObjReach.LocationRadius = 400.f;
    ObjReach.RequiredCount = 1;
    ObjReach.bIsOptional = false;

    FQuest_Objective ObjKill;
    ObjKill.ObjectiveID = FName("Obj_KillRaptors");
    ObjKill.Description = FText::FromString("Drive off the raptors (kill 3)");
    ObjKill.ObjectiveType = EQuest_ObjectiveType::KillTarget;
    ObjKill.TargetTag = FName("Raptor");
    ObjKill.RequiredCount = 3;
    ObjKill.bIsOptional = false;

    Q.Objectives.Add(ObjReach);
    Q.Objectives.Add(ObjKill);

    Q.Reward.ExperiencePoints = 250;
    Q.Reward.ItemRewards.Add(FName("Item_BoneTip"));
    Q.Reward.ItemRewards.Add(FName("Item_HideStrip"));

    return Q;
}

FQuest_Data AQuestSystemManager::BuildSurviveStampedeQuest() const
{
    FQuest_Data Q;
    Q.QuestID = FName("Quest_SurviveStampede");
    Q.Title = FText::FromString("Survive the Stampede");
    Q.Description = FText::FromString("A Gallimimus herd has been spooked. Survive the stampede without being trampled.");
    Q.Category = EQuest_Category::Survival;
    Q.Difficulty = EQuest_Difficulty::Medium;
    Q.bHasTimeLimit = true;
    Q.TimeLimit = 60.f;
    Q.GiverNPCTag = FName("NPC_Scout");

    FQuest_Objective ObjSurvive;
    ObjSurvive.ObjectiveID = FName("Obj_SurviveStampede");
    ObjSurvive.Description = FText::FromString("Survive the stampede for 60 seconds");
    ObjSurvive.ObjectiveType = EQuest_ObjectiveType::SurviveDuration;
    ObjSurvive.RequiredCount = 60;
    ObjSurvive.bIsOptional = false;

    FQuest_Objective ObjEscape;
    ObjEscape.ObjectiveID = FName("Obj_EscapeStampedeZone");
    ObjEscape.Description = FText::FromString("Escape the stampede zone");
    ObjEscape.ObjectiveType = EQuest_ObjectiveType::EscapeArea;
    ObjEscape.TargetLocation = FVector(1500.f, 0.f, 0.f);
    ObjEscape.LocationRadius = 2500.f;
    ObjEscape.RequiredCount = 1;
    ObjEscape.bIsOptional = true;

    Q.Objectives.Add(ObjSurvive);
    Q.Objectives.Add(ObjEscape);

    Q.Reward.ExperiencePoints = 150;
    Q.Reward.ItemRewards.Add(FName("Item_Flint"));

    return Q;
}

FQuest_Data AQuestSystemManager::BuildCraftStoneAxeQuest() const
{
    FQuest_Data Q;
    Q.QuestID = FName("Quest_CraftStoneAxe");
    Q.Title = FText::FromString("Forge Your First Axe");
    Q.Description = FText::FromString("Collect flint and a hardwood branch to craft a stone axe. You will need it to survive.");
    Q.Category = EQuest_Category::Crafting;
    Q.Difficulty = EQuest_Difficulty::Easy;
    Q.bHasTimeLimit = false;
    Q.GiverNPCTag = FName("NPC_Scout");

    FQuest_Objective ObjFlint;
    ObjFlint.ObjectiveID = FName("Obj_CollectFlint");
    ObjFlint.Description = FText::FromString("Collect 2 flint stones");
    ObjFlint.ObjectiveType = EQuest_ObjectiveType::CollectResource;
    ObjFlint.TargetTag = FName("Item_Flint");
    ObjFlint.RequiredCount = 2;
    ObjFlint.bIsOptional = false;

    FQuest_Objective ObjStick;
    ObjStick.ObjectiveID = FName("Obj_CollectStick");
    ObjStick.Description = FText::FromString("Collect 1 hardwood branch");
    ObjStick.ObjectiveType = EQuest_ObjectiveType::CollectResource;
    ObjStick.TargetTag = FName("Item_HardwoodBranch");
    ObjStick.RequiredCount = 1;
    ObjStick.bIsOptional = false;

    FQuest_Objective ObjCraft;
    ObjCraft.ObjectiveID = FName("Obj_CraftAxe");
    ObjCraft.Description = FText::FromString("Craft the stone axe at a crafting spot");
    ObjCraft.ObjectiveType = EQuest_ObjectiveType::CraftItem;
    ObjCraft.TargetTag = FName("Recipe_StoneAxe");
    ObjCraft.RequiredCount = 1;
    ObjCraft.bIsOptional = false;

    Q.Objectives.Add(ObjFlint);
    Q.Objectives.Add(ObjStick);
    Q.Objectives.Add(ObjCraft);

    Q.Reward.ExperiencePoints = 100;
    Q.Reward.UnlockRecipeID = FName("Recipe_SpearTip");
    Q.Reward.ItemRewards.Add(FName("Item_StoneAxe"));

    return Q;
}
