#include "QuestManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Components/ActorComponent.h"

UQuestManager::UQuestManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Check quest progress every second
    
    bAutoActivate = true;
    bWantsInitializeComponent = true;
    
    // Initialize default prehistoric survival quests
    InitializeDefaultQuests();
}

void UQuestManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("QuestManager: BeginPlay - Initializing quest system"));
    
    // Register quest NPCs
    RegisterQuestNPCs();
    
    // Start with first available quest
    if (AvailableQuests.Num() > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("QuestManager: %d quests available"), AvailableQuests.Num());
    }
}

void UQuestManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update active quest progress
    UpdateActiveQuests(DeltaTime);
    
    // Check for quest completion
    CheckQuestCompletion();
}

void UQuestManager::InitializeDefaultQuests()
{
    UE_LOG(LogTemp, Warning, TEXT("QuestManager: Initializing default prehistoric survival quests"));
    
    // Quest 1: Hunt the Raptor Pack
    FQuest_Data HuntQuest;
    HuntQuest.QuestID = TEXT("HUNT_001");
    HuntQuest.QuestName = TEXT("Hunt the Raptor Pack");
    HuntQuest.Description = TEXT("A pack of raptors threatens our hunting grounds. Eliminate 3 raptors to secure the area.");
    HuntQuest.QuestType = EQuest_Type::Hunt;
    HuntQuest.Difficulty = EQuest_Difficulty::Hard;
    HuntQuest.QuestGiverName = TEXT("Elder Grok");
    HuntQuest.MinLevel = 3;
    
    FQuest_Objective HuntObj;
    HuntObj.Type = EQuest_ObjectiveType::KillTarget;
    HuntObj.Description = TEXT("Kill Raptors");
    HuntObj.TargetName = TEXT("Raptor");
    HuntObj.RequiredCount = 3;
    HuntObj.CurrentCount = 0;
    HuntQuest.Objectives.Add(HuntObj);
    
    FQuest_Reward HuntReward;
    HuntReward.ItemName = TEXT("Raptor Claw");
    HuntReward.Quantity = 3;
    HuntReward.ExperiencePoints = 150;
    HuntReward.Description = TEXT("Sharp claws for crafting advanced weapons");
    HuntQuest.Rewards.Add(HuntReward);
    
    AvailableQuests.Add(HuntQuest.QuestID, HuntQuest);
    
    // Quest 2: Gather Crystal Shards
    FQuest_Data GatherQuest;
    GatherQuest.QuestID = TEXT("GATHER_001");
    GatherQuest.QuestName = TEXT("Crystal Cave Expedition");
    GatherQuest.Description = TEXT("The healer needs rare crystal shards from the dangerous caves. Gather 5 crystal shards.");
    GatherQuest.QuestType = EQuest_Type::Gather;
    GatherQuest.Difficulty = EQuest_Difficulty::Medium;
    GatherQuest.QuestGiverName = TEXT("Healer Nala");
    GatherQuest.MinLevel = 2;
    
    FQuest_Objective GatherObj;
    GatherObj.Type = EQuest_ObjectiveType::CollectItem;
    GatherObj.Description = TEXT("Collect Crystal Shards");
    GatherObj.TargetName = TEXT("Crystal Shard");
    GatherObj.RequiredCount = 5;
    GatherObj.CurrentCount = 0;
    GatherQuest.Objectives.Add(GatherObj);
    
    FQuest_Reward GatherReward;
    GatherReward.ItemName = TEXT("Healing Potion");
    GatherReward.Quantity = 2;
    GatherReward.ExperiencePoints = 75;
    GatherReward.Description = TEXT("Potent healing medicine");
    GatherQuest.Rewards.Add(GatherReward);
    
    AvailableQuests.Add(GatherQuest.QuestID, GatherQuest);
    
    // Quest 3: Explore High Ground
    FQuest_Data ExploreQuest;
    ExploreQuest.QuestID = TEXT("EXPLORE_001");
    ExploreQuest.QuestName = TEXT("Scout the High Ground");
    ExploreQuest.Description = TEXT("Scout Kira needs eyes on the distant mountains. Reach the observation post and report back.");
    ExploreQuest.QuestType = EQuest_Type::Explore;
    ExploreQuest.Difficulty = EQuest_Difficulty::Easy;
    ExploreQuest.QuestGiverName = TEXT("Scout Kira");
    ExploreQuest.MinLevel = 1;
    
    FQuest_Objective ExploreObj;
    ExploreObj.Type = EQuest_ObjectiveType::ReachLocation;
    ExploreObj.Description = TEXT("Reach the observation post");
    ExploreObj.TargetLocation = FVector(1000, -1000, 300);
    ExploreObj.Radius = 200.0f;
    ExploreObj.RequiredCount = 1;
    ExploreObj.CurrentCount = 0;
    ExploreQuest.Objectives.Add(ExploreObj);
    
    FQuest_Reward ExploreReward;
    ExploreReward.ItemName = TEXT("Map Fragment");
    ExploreReward.Quantity = 1;
    ExploreReward.ExperiencePoints = 50;
    ExploreReward.Description = TEXT("Reveals new areas to explore");
    ExploreQuest.Rewards.Add(ExploreReward);
    
    AvailableQuests.Add(ExploreQuest.QuestID, ExploreQuest);
    
    // Quest 4: Craft Survival Tools
    FQuest_Data CraftQuest;
    CraftQuest.QuestID = TEXT("CRAFT_001");
    CraftQuest.QuestName = TEXT("Tools for Survival");
    CraftQuest.Description = TEXT("Crafter Thane needs help making essential tools. Craft a stone axe and spear.");
    CraftQuest.QuestType = EQuest_Type::Craft;
    CraftQuest.Difficulty = EQuest_Difficulty::Easy;
    CraftQuest.QuestGiverName = TEXT("Crafter Thane");
    CraftQuest.MinLevel = 1;
    
    FQuest_Objective CraftObj1;
    CraftObj1.Type = EQuest_ObjectiveType::CraftItem;
    CraftObj1.Description = TEXT("Craft Stone Axe");
    CraftObj1.TargetName = TEXT("Stone Axe");
    CraftObj1.RequiredCount = 1;
    CraftObj1.CurrentCount = 0;
    CraftQuest.Objectives.Add(CraftObj1);
    
    FQuest_Objective CraftObj2;
    CraftObj2.Type = EQuest_ObjectiveType::CraftItem;
    CraftObj2.Description = TEXT("Craft Stone Spear");
    CraftObj2.TargetName = TEXT("Stone Spear");
    CraftObj2.RequiredCount = 1;
    CraftObj2.CurrentCount = 0;
    CraftQuest.Objectives.Add(CraftObj2);
    
    FQuest_Reward CraftReward;
    CraftReward.ItemName = TEXT("Crafting Knowledge");
    CraftReward.Quantity = 1;
    CraftReward.ExperiencePoints = 40;
    CraftReward.Description = TEXT("Learn advanced crafting techniques");
    CraftQuest.Rewards.Add(CraftReward);
    
    AvailableQuests.Add(CraftQuest.QuestID, CraftQuest);
    
    UE_LOG(LogTemp, Warning, TEXT("QuestManager: Initialized %d default quests"), AvailableQuests.Num());
}

void UQuestManager::RegisterQuestNPCs()
{
    // Register quest giver NPCs
    FQuest_NPCData ElderGrok;
    ElderGrok.NPCName = TEXT("Elder Grok");
    ElderGrok.Role = TEXT("Tribal Elder");
    ElderGrok.Location = FVector(500, 0, 150);
    ElderGrok.AvailableQuests.Add(TEXT("HUNT_001"));
    ElderGrok.Greeting = TEXT("Greetings, young hunter. The tribe needs your strength.");
    
    FQuest_NPCData HealerNala;
    HealerNala.NPCName = TEXT("Healer Nala");
    HealerNala.Role = TEXT("Medicine Woman");
    HealerNala.Location = FVector(-600, -200, 130);
    HealerNala.AvailableQuests.Add(TEXT("GATHER_001"));
    HealerNala.Greeting = TEXT("The spirits guide us to healing. Will you help gather what we need?");
    
    FQuest_NPCData ScoutKira;
    ScoutKira.NPCName = TEXT("Scout Kira");
    ScoutKira.Role = TEXT("Scout");
    ScoutKira.Location = FVector(-300, 400, 120);
    ScoutKira.AvailableQuests.Add(TEXT("EXPLORE_001"));
    ScoutKira.Greeting = TEXT("The lands beyond hold secrets. Are you brave enough to discover them?");
    
    FQuest_NPCData CrafterThane;
    CrafterThane.NPCName = TEXT("Crafter Thane");
    CrafterThane.Role = TEXT("Tool Maker");
    CrafterThane.Location = FVector(200, -500, 100);
    CrafterThane.AvailableQuests.Add(TEXT("CRAFT_001"));
    CrafterThane.Greeting = TEXT("Good tools mean survival. Let me teach you the old ways.");
    
    QuestNPCs.Add(ElderGrok.NPCName, ElderGrok);
    QuestNPCs.Add(HealerNala.NPCName, HealerNala);
    QuestNPCs.Add(ScoutKira.NPCName, ScoutKira);
    QuestNPCs.Add(CrafterThane.NPCName, CrafterThane);
    
    UE_LOG(LogTemp, Warning, TEXT("QuestManager: Registered %d quest NPCs"), QuestNPCs.Num());
}

bool UQuestManager::StartQuest(const FString& QuestID)
{
    if (!AvailableQuests.Contains(QuestID))
    {
        UE_LOG(LogTemp, Error, TEXT("QuestManager: Quest %s not found"), *QuestID);
        return false;
    }
    
    if (ActiveQuests.Contains(QuestID))
    {
        UE_LOG(LogTemp, Warning, TEXT("QuestManager: Quest %s already active"), *QuestID);
        return false;
    }
    
    FQuest_Data QuestData = AvailableQuests[QuestID];
    QuestData.Status = EQuest_Status::Active;
    
    ActiveQuests.Add(QuestID, QuestData);
    
    UE_LOG(LogTemp, Warning, TEXT("QuestManager: Started quest '%s'"), *QuestData.QuestName);
    
    // Broadcast quest started event
    OnQuestStarted.Broadcast(QuestData);
    
    return true;
}

bool UQuestManager::CompleteQuest(const FString& QuestID)
{
    if (!ActiveQuests.Contains(QuestID))
    {
        UE_LOG(LogTemp, Error, TEXT("QuestManager: Active quest %s not found"), *QuestID);
        return false;
    }
    
    FQuest_Data& QuestData = ActiveQuests[QuestID];
    QuestData.Status = EQuest_Status::Completed;
    
    // Move to completed quests
    CompletedQuests.Add(QuestID, QuestData);
    ActiveQuests.Remove(QuestID);
    
    UE_LOG(LogTemp, Warning, TEXT("QuestManager: Completed quest '%s'"), *QuestData.QuestName);
    
    // Grant rewards
    for (const FQuest_Reward& Reward : QuestData.Rewards)
    {
        UE_LOG(LogTemp, Warning, TEXT("QuestManager: Granted reward: %s x%d"), *Reward.ItemName, Reward.Quantity);
    }
    
    // Broadcast quest completed event
    OnQuestCompleted.Broadcast(QuestData);
    
    return true;
}

void UQuestManager::UpdateActiveQuests(float DeltaTime)
{
    for (auto& QuestPair : ActiveQuests)
    {
        FQuest_Data& QuestData = QuestPair.Value;
        
        // Update time-limited quests
        if (QuestData.bHasTimeLimit)
        {
            QuestData.TimeLimit -= DeltaTime;
            if (QuestData.TimeLimit <= 0.0f)
            {
                FailQuest(QuestData.QuestID);
                continue;
            }
        }
        
        // Update objective progress based on quest type
        UpdateQuestObjectives(QuestData);
    }
}

void UQuestManager::UpdateQuestObjectives(FQuest_Data& QuestData)
{
    // This would be expanded to check actual game state
    // For now, just log the update
    for (FQuest_Objective& Objective : QuestData.Objectives)
    {
        if (!Objective.bIsCompleted)
        {
            // Here we would check game state for objective completion
            // Example: check if player is in target location, has killed targets, etc.
        }
    }
}

void UQuestManager::CheckQuestCompletion()
{
    TArray<FString> QuestsToComplete;
    
    for (auto& QuestPair : ActiveQuests)
    {
        FQuest_Data& QuestData = QuestPair.Value;
        bool bAllObjectivesComplete = true;
        
        for (const FQuest_Objective& Objective : QuestData.Objectives)
        {
            if (!Objective.bIsCompleted)
            {
                bAllObjectivesComplete = false;
                break;
            }
        }
        
        if (bAllObjectivesComplete)
        {
            QuestsToComplete.Add(QuestData.QuestID);
        }
    }
    
    // Complete all finished quests
    for (const FString& QuestID : QuestsToComplete)
    {
        CompleteQuest(QuestID);
    }
}

bool UQuestManager::FailQuest(const FString& QuestID)
{
    if (!ActiveQuests.Contains(QuestID))
    {
        return false;
    }
    
    FQuest_Data QuestData = ActiveQuests[QuestID];
    QuestData.Status = EQuest_Status::Failed;
    
    FailedQuests.Add(QuestID, QuestData);
    ActiveQuests.Remove(QuestID);
    
    UE_LOG(LogTemp, Warning, TEXT("QuestManager: Failed quest '%s'"), *QuestData.QuestName);
    
    OnQuestFailed.Broadcast(QuestData);
    
    return true;
}

TArray<FQuest_Data> UQuestManager::GetAvailableQuestsForNPC(const FString& NPCName) const
{
    TArray<FQuest_Data> NPCQuests;
    
    if (QuestNPCs.Contains(NPCName))
    {
        const FQuest_NPCData& NPCData = QuestNPCs[NPCName];
        
        for (const FString& QuestID : NPCData.AvailableQuests)
        {
            if (AvailableQuests.Contains(QuestID) && !ActiveQuests.Contains(QuestID) && !CompletedQuests.Contains(QuestID))
            {
                NPCQuests.Add(AvailableQuests[QuestID]);
            }
        }
    }
    
    return NPCQuests;
}

FString UQuestManager::GetNPCGreeting(const FString& NPCName) const
{
    if (QuestNPCs.Contains(NPCName))
    {
        return QuestNPCs[NPCName].Greeting;
    }
    
    return TEXT("Hello, traveler.");
}

void UQuestManager::UpdateObjectiveProgress(const FString& QuestID, int32 ObjectiveIndex, int32 Progress)
{
    if (ActiveQuests.Contains(QuestID))
    {
        FQuest_Data& QuestData = ActiveQuests[QuestID];
        
        if (QuestData.Objectives.IsValidIndex(ObjectiveIndex))
        {
            FQuest_Objective& Objective = QuestData.Objectives[ObjectiveIndex];
            Objective.CurrentCount = FMath::Min(Objective.CurrentCount + Progress, Objective.RequiredCount);
            
            if (Objective.CurrentCount >= Objective.RequiredCount)
            {
                Objective.bIsCompleted = true;
                UE_LOG(LogTemp, Warning, TEXT("QuestManager: Objective completed: %s"), *Objective.Description);
                
                OnObjectiveCompleted.Broadcast(QuestData, Objective);
            }
        }
    }
}