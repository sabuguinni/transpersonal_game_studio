#include "Quest_ObjectiveSystem.h"
#include "Engine/Engine.h"

void UQuest_ObjectiveSystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    InitializeDefaultQuests();
    
    UE_LOG(LogTemp, Log, TEXT("Quest Objective System initialized with %d quests"), AllQuests.Num());
}

void UQuest_ObjectiveSystem::InitializeDefaultQuests()
{
    // Quest 1: Survival Basics
    FQuest_QuestData SurvivalQuest;
    SurvivalQuest.QuestID = TEXT("QUEST_SURVIVAL_BASICS");
    SurvivalQuest.QuestName = TEXT("First Steps to Survival");
    SurvivalQuest.Description = TEXT("Learn the basics of survival in the prehistoric world. Collect resources and craft your first tools.");
    SurvivalQuest.NPCGiver = TEXT("Elder_Krog");
    SurvivalQuest.ExperienceReward = 100;

    FQuest_ObjectiveData CollectSticks;
    CollectSticks.ObjectiveID = TEXT("COLLECT_STICKS");
    CollectSticks.Description = TEXT("Collect 5 sticks from the ground");
    CollectSticks.Type = EQuest_ObjectiveType::CollectItem;
    CollectSticks.TargetID = TEXT("Stick");
    CollectSticks.RequiredCount = 5;

    FQuest_ObjectiveData CollectRocks;
    CollectRocks.ObjectiveID = TEXT("COLLECT_ROCKS");
    CollectRocks.Description = TEXT("Collect 3 rocks for tool making");
    CollectRocks.Type = EQuest_ObjectiveType::CollectItem;
    CollectRocks.TargetID = TEXT("Rock");
    CollectRocks.RequiredCount = 3;

    FQuest_ObjectiveData CraftAxe;
    CraftAxe.ObjectiveID = TEXT("CRAFT_AXE");
    CraftAxe.Description = TEXT("Craft your first stone axe");
    CraftAxe.Type = EQuest_ObjectiveType::Craft;
    CraftAxe.TargetID = TEXT("StoneAxe");
    CraftAxe.RequiredCount = 1;

    SurvivalQuest.Objectives.Add(CollectSticks);
    SurvivalQuest.Objectives.Add(CollectRocks);
    SurvivalQuest.Objectives.Add(CraftAxe);
    AllQuests.Add(SurvivalQuest.QuestID, SurvivalQuest);

    // Quest 2: First Hunt
    FQuest_QuestData HuntQuest;
    HuntQuest.QuestID = TEXT("QUEST_FIRST_HUNT");
    HuntQuest.QuestName = TEXT("The Hunter's Trial");
    HuntQuest.Description = TEXT("Prove your hunting skills by taking down dangerous prey. Bring back trophies as proof of your prowess.");
    HuntQuest.NPCGiver = TEXT("Warrior_Thok");
    HuntQuest.ExperienceReward = 250;

    FQuest_ObjectiveData KillRaptors;
    KillRaptors.ObjectiveID = TEXT("KILL_RAPTORS");
    KillRaptors.Description = TEXT("Hunt 3 Velociraptors in the hunting grounds");
    KillRaptors.Type = EQuest_ObjectiveType::KillTarget;
    KillRaptors.TargetID = TEXT("Velociraptor");
    KillRaptors.RequiredCount = 3;

    FQuest_ObjectiveData ReachHuntZone;
    ReachHuntZone.ObjectiveID = TEXT("REACH_HUNT_ZONE");
    ReachHuntZone.Description = TEXT("Travel to the designated hunting grounds");
    ReachHuntZone.Type = EQuest_ObjectiveType::ReachLocation;
    ReachHuntZone.TargetLocation = FVector(4000, 2000, 100);
    ReachHuntZone.CompletionRadius = 1000.0f;

    HuntQuest.Objectives.Add(ReachHuntZone);
    HuntQuest.Objectives.Add(KillRaptors);
    AllQuests.Add(HuntQuest.QuestID, HuntQuest);

    // Quest 3: Valley Exploration
    FQuest_QuestData ExploreQuest;
    ExploreQuest.QuestID = TEXT("QUEST_VALLEY_EXPLORATION");
    ExploreQuest.QuestName = TEXT("Secrets of the Valley");
    ExploreQuest.Description = TEXT("Explore the mysterious valley and discover its hidden secrets. Find the ancient cave and the sacred water source.");
    ExploreQuest.NPCGiver = TEXT("Scout_Vera");
    ExploreQuest.ExperienceReward = 200;

    FQuest_ObjectiveData FindCave;
    FindCave.ObjectiveID = TEXT("FIND_CAVE");
    FindCave.Description = TEXT("Discover the entrance to the ancient cave");
    FindCave.Type = EQuest_ObjectiveType::ReachLocation;
    FindCave.TargetLocation = FVector(-3000, -1000, 200);
    FindCave.CompletionRadius = 500.0f;

    FQuest_ObjectiveData FindWater;
    FindWater.ObjectiveID = TEXT("FIND_WATER");
    FindWater.Description = TEXT("Locate the fresh water source");
    FindWater.Type = EQuest_ObjectiveType::ReachLocation;
    FindWater.TargetLocation = FVector(1000, 3000, 150);
    FindWater.CompletionRadius = 800.0f;

    FQuest_ObjectiveData TalkToHealer;
    TalkToHealer.ObjectiveID = TEXT("TALK_HEALER");
    TalkToHealer.Description = TEXT("Report your findings to Healer Mira");
    TalkToHealer.Type = EQuest_ObjectiveType::TalkToNPC;
    TalkToHealer.TargetID = TEXT("Healer_Mira");

    ExploreQuest.Objectives.Add(FindCave);
    ExploreQuest.Objectives.Add(FindWater);
    ExploreQuest.Objectives.Add(TalkToHealer);
    AllQuests.Add(ExploreQuest.QuestID, ExploreQuest);
}

void UQuest_ObjectiveSystem::StartQuest(const FString& QuestID)
{
    if (FQuest_QuestData* Quest = AllQuests.Find(QuestID))
    {
        if (!Quest->bActive && !Quest->bCompleted)
        {
            Quest->bActive = true;
            ActiveQuestIDs.AddUnique(QuestID);
            
            UE_LOG(LogTemp, Log, TEXT("Started quest: %s"), *Quest->QuestName);
            
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
                    FString::Printf(TEXT("Quest Started: %s"), *Quest->QuestName));
            }
        }
    }
}

void UQuest_ObjectiveSystem::CompleteQuest(const FString& QuestID)
{
    if (FQuest_QuestData* Quest = AllQuests.Find(QuestID))
    {
        Quest->bCompleted = true;
        Quest->bActive = false;
        ActiveQuestIDs.Remove(QuestID);
        
        UE_LOG(LogTemp, Log, TEXT("Completed quest: %s"), *Quest->QuestName);
        
        OnQuestCompleted.Broadcast(QuestID);
        
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Gold, 
                FString::Printf(TEXT("Quest Completed: %s (+%d XP)"), *Quest->QuestName, Quest->ExperienceReward));
        }
    }
}

void UQuest_ObjectiveSystem::UpdateObjective(const FString& QuestID, const FString& ObjectiveID, int32 Progress)
{
    if (FQuest_QuestData* Quest = AllQuests.Find(QuestID))
    {
        if (Quest->bActive)
        {
            for (FQuest_ObjectiveData& Objective : Quest->Objectives)
            {
                if (Objective.ObjectiveID == ObjectiveID && !Objective.bCompleted)
                {
                    Objective.CurrentCount += Progress;
                    
                    CheckObjectiveCompletion(*Quest, Objective);
                    CheckQuestCompletion(*Quest);
                    
                    OnObjectiveUpdated.Broadcast(QuestID, ObjectiveID);
                    break;
                }
            }
        }
    }
}

void UQuest_ObjectiveSystem::CheckLocationObjectives(const FVector& PlayerLocation)
{
    for (const FString& QuestID : ActiveQuestIDs)
    {
        if (FQuest_QuestData* Quest = AllQuests.Find(QuestID))
        {
            for (FQuest_ObjectiveData& Objective : Quest->Objectives)
            {
                if (Objective.Type == EQuest_ObjectiveType::ReachLocation && !Objective.bCompleted)
                {
                    float Distance = FVector::Dist(PlayerLocation, Objective.TargetLocation);
                    if (Distance <= Objective.CompletionRadius)
                    {
                        Objective.CurrentCount = 1;
                        CheckObjectiveCompletion(*Quest, Objective);
                        CheckQuestCompletion(*Quest);
                        
                        OnObjectiveUpdated.Broadcast(QuestID, Objective.ObjectiveID);
                    }
                }
            }
        }
    }
}

void UQuest_ObjectiveSystem::OnDinosaurKilled(const FString& DinosaurType)
{
    for (const FString& QuestID : ActiveQuestIDs)
    {
        if (FQuest_QuestData* Quest = AllQuests.Find(QuestID))
        {
            for (FQuest_ObjectiveData& Objective : Quest->Objectives)
            {
                if (Objective.Type == EQuest_ObjectiveType::KillTarget && 
                    Objective.TargetID == DinosaurType && !Objective.bCompleted)
                {
                    UpdateObjective(QuestID, Objective.ObjectiveID, 1);
                }
            }
        }
    }
}

void UQuest_ObjectiveSystem::OnItemCollected(const FString& ItemType, int32 Count)
{
    for (const FString& QuestID : ActiveQuestIDs)
    {
        if (FQuest_QuestData* Quest = AllQuests.Find(QuestID))
        {
            for (FQuest_ObjectiveData& Objective : Quest->Objectives)
            {
                if (Objective.Type == EQuest_ObjectiveType::CollectItem && 
                    Objective.TargetID == ItemType && !Objective.bCompleted)
                {
                    UpdateObjective(QuestID, Objective.ObjectiveID, Count);
                }
            }
        }
    }
}

void UQuest_ObjectiveSystem::OnNPCTalkedTo(const FString& NPCID)
{
    for (const FString& QuestID : ActiveQuestIDs)
    {
        if (FQuest_QuestData* Quest = AllQuests.Find(QuestID))
        {
            for (FQuest_ObjectiveData& Objective : Quest->Objectives)
            {
                if (Objective.Type == EQuest_ObjectiveType::TalkToNPC && 
                    Objective.TargetID == NPCID && !Objective.bCompleted)
                {
                    UpdateObjective(QuestID, Objective.ObjectiveID, 1);
                }
            }
        }
    }
}

void UQuest_ObjectiveSystem::CheckObjectiveCompletion(FQuest_QuestData& Quest, FQuest_ObjectiveData& Objective)
{
    if (Objective.CurrentCount >= Objective.RequiredCount)
    {
        Objective.bCompleted = true;
        
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, 
                FString::Printf(TEXT("Objective Complete: %s"), *Objective.Description));
        }
    }
}

void UQuest_ObjectiveSystem::CheckQuestCompletion(FQuest_QuestData& Quest)
{
    bool bAllObjectivesComplete = true;
    for (const FQuest_ObjectiveData& Objective : Quest.Objectives)
    {
        if (!Objective.bCompleted)
        {
            bAllObjectivesComplete = false;
            break;
        }
    }
    
    if (bAllObjectivesComplete && !Quest.bCompleted)
    {
        CompleteQuest(Quest.QuestID);
    }
}

bool UQuest_ObjectiveSystem::IsQuestActive(const FString& QuestID) const
{
    if (const FQuest_QuestData* Quest = AllQuests.Find(QuestID))
    {
        return Quest->bActive;
    }
    return false;
}

bool UQuest_ObjectiveSystem::IsQuestCompleted(const FString& QuestID) const
{
    if (const FQuest_QuestData* Quest = AllQuests.Find(QuestID))
    {
        return Quest->bCompleted;
    }
    return false;
}

TArray<FQuest_QuestData> UQuest_ObjectiveSystem::GetActiveQuests() const
{
    TArray<FQuest_QuestData> ActiveQuests;
    for (const FString& QuestID : ActiveQuestIDs)
    {
        if (const FQuest_QuestData* Quest = AllQuests.Find(QuestID))
        {
            ActiveQuests.Add(*Quest);
        }
    }
    return ActiveQuests;
}

FQuest_QuestData UQuest_ObjectiveSystem::GetQuestData(const FString& QuestID) const
{
    if (const FQuest_QuestData* Quest = AllQuests.Find(QuestID))
    {
        return *Quest;
    }
    return FQuest_QuestData();
}