#include "Quest_QuestManager.h"
#include "Quest_QuestGiverNPC.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

AQuest_QuestManager::AQuest_QuestManager()
{
    PrimaryActorTick.bCanEverTick = false;
    MaxActiveQuests = 5;
}

void AQuest_QuestManager::BeginPlay()
{
    Super::BeginPlay();
    InitializeDefaultQuests();
    
    UE_LOG(LogTemp, Warning, TEXT("Quest Manager initialized with %d available quests"), AvailableQuests.Num());
}

bool AQuest_QuestManager::StartQuest(const FString& QuestID)
{
    if (!CanAcceptQuest(QuestID))
    {
        return false;
    }

    // Find quest in available quests
    for (int32 i = 0; i < AvailableQuests.Num(); i++)
    {
        if (AvailableQuests[i].QuestID == QuestID)
        {
            FQuest_QuestData QuestData = AvailableQuests[i];
            QuestData.QuestStatus = EQuest_QuestStatus::Active;
            
            ActiveQuests.Add(QuestData);
            AvailableQuests.RemoveAt(i);
            
            UE_LOG(LogTemp, Warning, TEXT("Started quest: %s"), *QuestData.QuestName);
            return true;
        }
    }

    return false;
}

bool AQuest_QuestManager::CompleteQuest(const FString& QuestID)
{
    for (int32 i = 0; i < ActiveQuests.Num(); i++)
    {
        if (ActiveQuests[i].QuestID == QuestID)
        {
            FQuest_QuestData QuestData = ActiveQuests[i];
            QuestData.QuestStatus = EQuest_QuestStatus::Completed;
            
            CompletedQuests.Add(QuestData);
            ActiveQuests.RemoveAt(i);
            
            UE_LOG(LogTemp, Warning, TEXT("Completed quest: %s - Reward: %d XP"), 
                   *QuestData.QuestName, QuestData.ExperienceReward);
            return true;
        }
    }

    return false;
}

bool AQuest_QuestManager::UpdateQuestObjective(const FString& QuestID, int32 ObjectiveIndex, int32 Progress)
{
    for (FQuest_QuestData& Quest : ActiveQuests)
    {
        if (Quest.QuestID == QuestID && Quest.Objectives.IsValidIndex(ObjectiveIndex))
        {
            FQuest_QuestObjective& Objective = Quest.Objectives[ObjectiveIndex];
            Objective.CurrentProgress = FMath::Min(Objective.CurrentProgress + Progress, Objective.RequiredProgress);
            
            if (Objective.CurrentProgress >= Objective.RequiredProgress)
            {
                Objective.bIsCompleted = true;
                UE_LOG(LogTemp, Warning, TEXT("Quest objective completed: %s"), *Objective.Description);
                
                // Check if all objectives are completed
                bool bAllCompleted = true;
                for (const FQuest_QuestObjective& Obj : Quest.Objectives)
                {
                    if (!Obj.bIsCompleted)
                    {
                        bAllCompleted = false;
                        break;
                    }
                }
                
                if (bAllCompleted)
                {
                    CompleteQuest(QuestID);
                }
            }
            
            return true;
        }
    }

    return false;
}

FQuest_QuestData AQuest_QuestManager::GetQuestData(const FString& QuestID)
{
    // Search in active quests
    for (const FQuest_QuestData& Quest : ActiveQuests)
    {
        if (Quest.QuestID == QuestID)
        {
            return Quest;
        }
    }

    // Search in available quests
    for (const FQuest_QuestData& Quest : AvailableQuests)
    {
        if (Quest.QuestID == QuestID)
        {
            return Quest;
        }
    }

    // Search in completed quests
    for (const FQuest_QuestData& Quest : CompletedQuests)
    {
        if (Quest.QuestID == QuestID)
        {
            return Quest;
        }
    }

    // Return default quest data if not found
    return FQuest_QuestData();
}

TArray<FQuest_QuestData> AQuest_QuestManager::GetActiveQuests() const
{
    return ActiveQuests;
}

TArray<FQuest_QuestData> AQuest_QuestManager::GetAvailableQuests() const
{
    return AvailableQuests;
}

bool AQuest_QuestManager::CanAcceptQuest(const FString& QuestID) const
{
    // Check if already at max active quests
    if (ActiveQuests.Num() >= MaxActiveQuests)
    {
        return false;
    }

    // Check if quest exists in available quests
    for (const FQuest_QuestData& Quest : AvailableQuests)
    {
        if (Quest.QuestID == QuestID)
        {
            return true;
        }
    }

    return false;
}

void AQuest_QuestManager::InitializeDefaultQuests()
{
    AvailableQuests.Empty();
    
    CreateHuntQuests();
    CreateGatherQuests();
    CreateExploreQuests();
    CreateCraftQuests();
    CreateRescueQuests();
}

void AQuest_QuestManager::RegisterQuestGiver(const FString& QuestGiverID, AQuest_QuestGiverNPC* QuestGiver)
{
    if (QuestGiver)
    {
        QuestGivers.Add(QuestGiverID, QuestGiver);
        UE_LOG(LogTemp, Warning, TEXT("Registered quest giver: %s"), *QuestGiverID);
    }
}

void AQuest_QuestManager::CreateHuntQuests()
{
    // Hunt the Alpha Raptor
    FQuest_QuestData HuntQuest;
    HuntQuest.QuestID = TEXT("HUNT_ALPHA_RAPTOR");
    HuntQuest.QuestName = TEXT("Hunt the Alpha Raptor");
    HuntQuest.QuestDescription = TEXT("The alpha raptor threatens our settlement. Track it down and eliminate the threat.");
    HuntQuest.QuestType = EQuest_QuestType::Hunt;
    HuntQuest.ExperienceReward = 500;
    HuntQuest.QuestGiverID = TEXT("HuntMaster_Krog");
    
    FQuest_QuestObjective HuntObjective;
    HuntObjective.Description = TEXT("Kill the Alpha Raptor");
    HuntObjective.RequiredProgress = 1;
    HuntQuest.Objectives.Add(HuntObjective);
    
    AvailableQuests.Add(HuntQuest);

    // Hunt Small Game
    FQuest_QuestData SmallGameQuest;
    SmallGameQuest.QuestID = TEXT("HUNT_SMALL_GAME");
    SmallGameQuest.QuestName = TEXT("Hunt Small Game");
    SmallGameQuest.QuestDescription = TEXT("Bring back meat from small creatures to feed the tribe.");
    SmallGameQuest.QuestType = EQuest_QuestType::Hunt;
    SmallGameQuest.ExperienceReward = 200;
    SmallGameQuest.QuestGiverID = TEXT("HuntMaster_Krog");
    
    FQuest_QuestObjective SmallGameObjective;
    SmallGameObjective.Description = TEXT("Collect 5 pieces of small game meat");
    SmallGameObjective.RequiredProgress = 5;
    SmallGameQuest.Objectives.Add(SmallGameObjective);
    
    AvailableQuests.Add(SmallGameQuest);
}

void AQuest_QuestManager::CreateGatherQuests()
{
    // Gather Medicinal Herbs
    FQuest_QuestData HerbQuest;
    HerbQuest.QuestID = TEXT("GATHER_HERBS");
    HerbQuest.QuestName = TEXT("Gather Medicinal Herbs");
    HerbQuest.QuestDescription = TEXT("Collect healing herbs from the dangerous swamplands.");
    HerbQuest.QuestType = EQuest_QuestType::Gather;
    HerbQuest.ExperienceReward = 300;
    HerbQuest.QuestGiverID = TEXT("HealerAncient_Yara");
    
    FQuest_QuestObjective HerbObjective;
    HerbObjective.Description = TEXT("Collect 10 medicinal herbs");
    HerbObjective.RequiredProgress = 10;
    HerbQuest.Objectives.Add(HerbObjective);
    
    AvailableQuests.Add(HerbQuest);

    // Gather Rare Stones
    FQuest_QuestData StoneQuest;
    StoneQuest.QuestID = TEXT("GATHER_RARE_STONES");
    StoneQuest.QuestName = TEXT("Gather Rare Stones");
    StoneQuest.QuestDescription = TEXT("Find special stones needed for advanced tool crafting.");
    StoneQuest.QuestType = EQuest_QuestType::Gather;
    StoneQuest.ExperienceReward = 400;
    StoneQuest.QuestGiverID = TEXT("CrafterWise_Brok");
    
    FQuest_QuestObjective StoneObjective;
    StoneObjective.Description = TEXT("Collect 3 rare crafting stones");
    StoneObjective.RequiredProgress = 3;
    StoneQuest.Objectives.Add(StoneObjective);
    
    AvailableQuests.Add(StoneQuest);
}

void AQuest_QuestManager::CreateExploreQuests()
{
    // Explore the Ancient Cave
    FQuest_QuestData CaveQuest;
    CaveQuest.QuestID = TEXT("EXPLORE_ANCIENT_CAVE");
    CaveQuest.QuestName = TEXT("Explore the Ancient Cave");
    CaveQuest.QuestDescription = TEXT("Investigate the mysterious cave system discovered by our scouts.");
    CaveQuest.QuestType = EQuest_QuestType::Explore;
    CaveQuest.ExperienceReward = 600;
    CaveQuest.QuestGiverID = TEXT("ScoutLeader_Thak");
    
    FQuest_QuestObjective CaveObjective;
    CaveObjective.Description = TEXT("Reach the deepest chamber of the cave");
    CaveObjective.RequiredProgress = 1;
    CaveQuest.Objectives.Add(CaveObjective);
    
    AvailableQuests.Add(CaveQuest);
}

void AQuest_QuestManager::CreateCraftQuests()
{
    // Craft Advanced Tools
    FQuest_QuestData CraftQuest;
    CraftQuest.QuestID = TEXT("CRAFT_ADVANCED_TOOLS");
    CraftQuest.QuestName = TEXT("Craft Advanced Tools");
    CraftQuest.QuestDescription = TEXT("Create better tools to improve our tribe's efficiency.");
    CraftQuest.QuestType = EQuest_QuestType::Craft;
    CraftQuest.ExperienceReward = 350;
    CraftQuest.QuestGiverID = TEXT("CrafterWise_Brok");
    
    FQuest_QuestObjective CraftObjective;
    CraftObjective.Description = TEXT("Craft 3 advanced stone tools");
    CraftObjective.RequiredProgress = 3;
    CraftQuest.Objectives.Add(CraftObjective);
    
    AvailableQuests.Add(CraftQuest);
}

void AQuest_QuestManager::CreateRescueQuests()
{
    // Rescue Lost Tribesman
    FQuest_QuestData RescueQuest;
    RescueQuest.QuestID = TEXT("RESCUE_LOST_TRIBESMAN");
    RescueQuest.QuestName = TEXT("Rescue Lost Tribesman");
    RescueQuest.QuestDescription = TEXT("One of our people went missing during a gathering expedition. Find them before it's too late.");
    RescueQuest.QuestType = EQuest_QuestType::Rescue;
    RescueQuest.ExperienceReward = 450;
    RescueQuest.QuestGiverID = TEXT("GathererElder_Nala");
    
    FQuest_QuestObjective RescueObjective;
    RescueObjective.Description = TEXT("Find and rescue the lost tribesman");
    RescueObjective.RequiredProgress = 1;
    RescueQuest.Objectives.Add(RescueObjective);
    
    AvailableQuests.Add(RescueQuest);
}