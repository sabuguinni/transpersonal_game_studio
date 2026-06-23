// QuestManager.cpp — Agent #14 Quest & Mission Designer
// Transpersonal Game Studio — Dinosaur Survival Game
// Quest lifecycle, objective tracking, crafting recipes

#include "QuestManager.h"

// ============================================================
// Constructor
// ============================================================

UQuestManager::UQuestManager()
{
}

// ============================================================
// Subsystem Lifecycle
// ============================================================

void UQuestManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    RegisterDefaultQuests();
    RegisterCraftingRecipes();
    UE_LOG(LogTemp, Log, TEXT("[QuestManager] Initialized — %d quests, %d recipes"), AllQuests.Num(), CraftingRecipes.Num());
}

void UQuestManager::Deinitialize()
{
    AllQuests.Empty();
    CraftingRecipes.Empty();
    ResourceInventory.Empty();
    Super::Deinitialize();
}

// ============================================================
// Quest Registration
// ============================================================

void UQuestManager::RegisterDefaultQuests()
{
    // Quest 1: Follow the Herd — reach the herd migration start, follow to water
    {
        FQuest_Definition Q;
        Q.QuestID = EQuest_ID::FollowTheHerd;
        Q.QuestTitle = TEXT("Follow the Herd");
        Q.QuestDescription = TEXT("A massive herd of herbivores is migrating north. Follow them to find a reliable water source before the dry season.");
        Q.bIsMainQuest = false;
        Q.TimeLimit = 0.0f;

        FQuest_Objective Obj1;
        Obj1.ObjectiveType = EQuest_ObjectiveType::Reach;
        Obj1.Description = TEXT("Find the herd migration corridor");
        Obj1.TargetLocation = FVector(-2000.0f, -3000.0f, 50.0f);
        Obj1.AcceptanceRadius = 500.0f;
        Q.Objectives.Add(Obj1);

        FQuest_Objective Obj2;
        Obj2.ObjectiveType = EQuest_ObjectiveType::Follow;
        Obj2.Description = TEXT("Follow the herd to the water source");
        Obj2.RequiredCount = 1;
        Obj2.TargetLocation = FVector(-500.0f, 1500.0f, 50.0f);
        Obj2.AcceptanceRadius = 400.0f;
        Q.Objectives.Add(Obj2);

        AllQuests.Add(Q);
    }

    // Quest 2: Raptor Territory — retrieve meat cache from raptor patrol zone
    {
        FQuest_Definition Q;
        Q.QuestID = EQuest_ID::RaptorTerritory;
        Q.QuestTitle = TEXT("Raptor Territory");
        Q.QuestDescription = TEXT("The raptor pack has claimed the eastern pass. Retrieve the meat cache from their patrol zone and return to camp.");
        Q.bIsMainQuest = false;
        Q.TimeLimit = 300.0f; // 5 minutes

        FQuest_Objective Obj1;
        Obj1.ObjectiveType = EQuest_ObjectiveType::Reach;
        Obj1.Description = TEXT("Enter the raptor patrol zone");
        Obj1.TargetLocation = FVector(500.0f, 500.0f, 50.0f);
        Obj1.AcceptanceRadius = 300.0f;
        Q.Objectives.Add(Obj1);

        FQuest_Objective Obj2;
        Obj2.ObjectiveType = EQuest_ObjectiveType::Collect;
        Obj2.Description = TEXT("Retrieve the meat cache");
        Obj2.RequiredCount = 1;
        Q.Objectives.Add(Obj2);

        FQuest_Objective Obj3;
        Obj3.ObjectiveType = EQuest_ObjectiveType::Reach;
        Obj3.Description = TEXT("Return to the tribe camp");
        Obj3.TargetLocation = FVector(800.0f, 400.0f, 50.0f);
        Obj3.AcceptanceRadius = 300.0f;
        Q.Objectives.Add(Obj3);

        AllQuests.Add(Q);
    }

    // Quest 3: Tribe Camp Defense — survive raptor attack on camp
    {
        FQuest_Definition Q;
        Q.QuestID = EQuest_ID::TribeCampDefense;
        Q.QuestTitle = TEXT("Defend the Camp");
        Q.QuestDescription = TEXT("A raptor pack is closing in on the tribe camp. Defend the camp until the tribe members reach safety.");
        Q.bIsMainQuest = false;
        Q.TimeLimit = 180.0f; // 3 minutes

        FQuest_Objective Obj1;
        Obj1.ObjectiveType = EQuest_ObjectiveType::Survive;
        Obj1.Description = TEXT("Survive the raptor attack for 3 minutes");
        Obj1.RequiredCount = 180;
        Q.Objectives.Add(Obj1);

        FQuest_Objective Obj2;
        Obj2.ObjectiveType = EQuest_ObjectiveType::Defend;
        Obj2.Description = TEXT("Keep the tribe camp intact");
        Obj2.TargetLocation = FVector(800.0f, 400.0f, 50.0f);
        Obj2.AcceptanceRadius = 600.0f;
        Q.Objectives.Add(Obj2);

        AllQuests.Add(Q);
    }

    // Quest 4: Find Water Source
    {
        FQuest_Definition Q;
        Q.QuestID = EQuest_ID::FindWaterSource;
        Q.QuestTitle = TEXT("Find Water");
        Q.QuestDescription = TEXT("The tribe's water supply is running low. Scout the valley to find a clean water source before dehydration sets in.");
        Q.bIsMainQuest = true;
        Q.TimeLimit = 0.0f;

        FQuest_Objective Obj1;
        Obj1.ObjectiveType = EQuest_ObjectiveType::Reach;
        Obj1.Description = TEXT("Reach the water source in the valley");
        Obj1.TargetLocation = FVector(-500.0f, 1500.0f, 50.0f);
        Obj1.AcceptanceRadius = 400.0f;
        Q.Objectives.Add(Obj1);

        AllQuests.Add(Q);
    }

    // Quest 5: Cave Exploration
    {
        FQuest_Definition Q;
        Q.QuestID = EQuest_ID::CaveExploration;
        Q.QuestTitle = TEXT("Explore the Cave");
        Q.QuestDescription = TEXT("A cave entrance has been spotted to the east. It may contain flint and obsidian for better tools — if the predators inside don't find you first.");
        Q.bIsMainQuest = false;
        Q.TimeLimit = 0.0f;

        FQuest_Objective Obj1;
        Obj1.ObjectiveType = EQuest_ObjectiveType::Reach;
        Obj1.Description = TEXT("Find the cave entrance");
        Obj1.TargetLocation = FVector(1200.0f, -800.0f, 50.0f);
        Obj1.AcceptanceRadius = 350.0f;
        Q.Objectives.Add(Obj1);

        FQuest_Objective Obj2;
        Obj2.ObjectiveType = EQuest_ObjectiveType::Collect;
        Obj2.Description = TEXT("Collect flint from inside the cave");
        Obj2.RequiredCount = 3;
        Q.Objectives.Add(Obj2);

        AllQuests.Add(Q);
    }

    // Quest 6: Craft Stone Axe
    {
        FQuest_Definition Q;
        Q.QuestID = EQuest_ID::CraftStoneAxe;
        Q.QuestTitle = TEXT("Craft a Stone Axe");
        Q.QuestDescription = TEXT("Gather rocks and a sturdy stick to craft a stone axe. It will make hunting and chopping wood far more efficient.");
        Q.bIsMainQuest = false;
        Q.TimeLimit = 0.0f;

        FQuest_Objective Obj1;
        Obj1.ObjectiveType = EQuest_ObjectiveType::Collect;
        Obj1.Description = TEXT("Collect 2 rocks");
        Obj1.RequiredCount = 2;
        Q.Objectives.Add(Obj1);

        FQuest_Objective Obj2;
        Obj2.ObjectiveType = EQuest_ObjectiveType::Collect;
        Obj2.Description = TEXT("Collect 1 stick");
        Obj2.RequiredCount = 1;
        Q.Objectives.Add(Obj2);

        FQuest_Objective Obj3;
        Obj3.ObjectiveType = EQuest_ObjectiveType::Craft;
        Obj3.Description = TEXT("Craft the Stone Axe");
        Obj3.RequiredCount = 1;
        Q.Objectives.Add(Obj3);

        AllQuests.Add(Q);
    }

    // Quest 7: Craft Campfire
    {
        FQuest_Definition Q;
        Q.QuestID = EQuest_ID::CraftCampfire;
        Q.QuestTitle = TEXT("Build a Campfire");
        Q.QuestDescription = TEXT("Night is coming and the temperature is dropping. Gather sticks and build a campfire to survive the cold and keep predators at bay.");
        Q.bIsMainQuest = true;
        Q.TimeLimit = 0.0f;

        FQuest_Objective Obj1;
        Obj1.ObjectiveType = EQuest_ObjectiveType::Collect;
        Obj1.Description = TEXT("Collect 3 sticks");
        Obj1.RequiredCount = 3;
        Q.Objectives.Add(Obj1);

        FQuest_Objective Obj2;
        Obj2.ObjectiveType = EQuest_ObjectiveType::Craft;
        Obj2.Description = TEXT("Build the campfire");
        Obj2.RequiredCount = 1;
        Q.Objectives.Add(Obj2);

        AllQuests.Add(Q);
    }
}

void UQuestManager::RegisterCraftingRecipes()
{
    // Recipe 1: Stone Axe — 2 rocks + 1 stick
    {
        FQuest_CraftingRecipe R;
        R.RecipeName = TEXT("Stone Axe");
        R.RequiredResources.Add(TEXT("Rock"));
        R.RequiredCounts.Add(2);
        R.RequiredResources.Add(TEXT("Stick"));
        R.RequiredCounts.Add(1);
        R.OutputItem = TEXT("Stone Axe");
        R.bUnlocked = true;
        CraftingRecipes.Add(R);
    }

    // Recipe 2: Campfire — 3 sticks
    {
        FQuest_CraftingRecipe R;
        R.RecipeName = TEXT("Campfire");
        R.RequiredResources.Add(TEXT("Stick"));
        R.RequiredCounts.Add(3);
        R.OutputItem = TEXT("Campfire");
        R.bUnlocked = true;
        CraftingRecipes.Add(R);
    }

    // Recipe 3: Water Container — 1 rock + 1 leaf
    {
        FQuest_CraftingRecipe R;
        R.RecipeName = TEXT("Water Container");
        R.RequiredResources.Add(TEXT("Rock"));
        R.RequiredCounts.Add(1);
        R.RequiredResources.Add(TEXT("Leaf"));
        R.RequiredCounts.Add(1);
        R.OutputItem = TEXT("Water Container");
        R.bUnlocked = true;
        CraftingRecipes.Add(R);
    }
}

// ============================================================
// Quest Lifecycle
// ============================================================

bool UQuestManager::StartQuest(EQuest_ID QuestID)
{
    FQuest_Definition* Q = FindQuest(QuestID);
    if (!Q) return false;
    if (Q->Status == EQuest_Status::Active) return false;

    Q->Status = EQuest_Status::Active;
    Q->ElapsedTime = 0.0f;
    for (FQuest_Objective& Obj : Q->Objectives)
    {
        Obj.CurrentCount = 0;
        Obj.bCompleted = false;
    }
    UE_LOG(LogTemp, Log, TEXT("[QuestManager] Started quest: %s"), *Q->QuestTitle);
    return true;
}

bool UQuestManager::CompleteQuest(EQuest_ID QuestID)
{
    FQuest_Definition* Q = FindQuest(QuestID);
    if (!Q) return false;
    Q->Status = EQuest_Status::Completed;
    UE_LOG(LogTemp, Log, TEXT("[QuestManager] Completed quest: %s"), *Q->QuestTitle);
    return true;
}

bool UQuestManager::FailQuest(EQuest_ID QuestID)
{
    FQuest_Definition* Q = FindQuest(QuestID);
    if (!Q) return false;
    Q->Status = EQuest_Status::Failed;
    UE_LOG(LogTemp, Log, TEXT("[QuestManager] Failed quest: %s"), *Q->QuestTitle);
    return true;
}

EQuest_Status UQuestManager::GetQuestStatus(EQuest_ID QuestID) const
{
    const FQuest_Definition* Q = FindQuestConst(QuestID);
    if (!Q) return EQuest_Status::Inactive;
    return Q->Status;
}

bool UQuestManager::IsQuestActive(EQuest_ID QuestID) const
{
    return GetQuestStatus(QuestID) == EQuest_Status::Active;
}

int32 UQuestManager::GetActiveQuestCount() const
{
    int32 Count = 0;
    for (const FQuest_Definition& Q : AllQuests)
    {
        if (Q.Status == EQuest_Status::Active) ++Count;
    }
    return Count;
}

// ============================================================
// Objective Progress
// ============================================================

void UQuestManager::AdvanceObjective(EQuest_ID QuestID, int32 ObjectiveIndex, int32 Amount)
{
    FQuest_Definition* Q = FindQuest(QuestID);
    if (!Q || Q->Status != EQuest_Status::Active) return;
    if (!Q->Objectives.IsValidIndex(ObjectiveIndex)) return;

    FQuest_Objective& Obj = Q->Objectives[ObjectiveIndex];
    if (Obj.bCompleted) return;

    Obj.CurrentCount = FMath::Min(Obj.CurrentCount + Amount, Obj.RequiredCount);
    if (Obj.CurrentCount >= Obj.RequiredCount)
    {
        Obj.bCompleted = true;
        UE_LOG(LogTemp, Log, TEXT("[QuestManager] Objective complete: %s / %s"), *Q->QuestTitle, *Obj.Description);

        // Check if all objectives done
        bool bAllDone = true;
        for (const FQuest_Objective& O : Q->Objectives)
        {
            if (!O.bCompleted) { bAllDone = false; break; }
        }
        if (bAllDone)
        {
            CompleteQuest(QuestID);
        }
    }
}

bool UQuestManager::CheckObjectiveComplete(EQuest_ID QuestID, int32 ObjectiveIndex) const
{
    const FQuest_Definition* Q = FindQuestConst(QuestID);
    if (!Q || !Q->Objectives.IsValidIndex(ObjectiveIndex)) return false;
    return Q->Objectives[ObjectiveIndex].bCompleted;
}

// ============================================================
// Resource & Crafting
// ============================================================

bool UQuestManager::AddResource(const FString& ResourceName, int32 Count)
{
    if (ResourceName.IsEmpty() || Count <= 0) return false;
    int32& Current = ResourceInventory.FindOrAdd(ResourceName);
    Current += Count;
    UE_LOG(LogTemp, Log, TEXT("[QuestManager] Resource added: %s x%d (total: %d)"), *ResourceName, Count, Current);
    return true;
}

int32 UQuestManager::GetResourceCount(const FString& ResourceName) const
{
    const int32* Found = ResourceInventory.Find(ResourceName);
    return Found ? *Found : 0;
}

bool UQuestManager::CanCraft(const FString& RecipeName) const
{
    for (const FQuest_CraftingRecipe& R : CraftingRecipes)
    {
        if (R.RecipeName == RecipeName && R.bUnlocked)
        {
            for (int32 i = 0; i < R.RequiredResources.Num(); ++i)
            {
                int32 Have = GetResourceCount(R.RequiredResources[i]);
                int32 Need = R.RequiredCounts.IsValidIndex(i) ? R.RequiredCounts[i] : 1;
                if (Have < Need) return false;
            }
            return true;
        }
    }
    return false;
}

bool UQuestManager::CraftItem(const FString& RecipeName)
{
    if (!CanCraft(RecipeName)) return false;

    for (FQuest_CraftingRecipe& R : CraftingRecipes)
    {
        if (R.RecipeName == RecipeName)
        {
            // Consume resources
            for (int32 i = 0; i < R.RequiredResources.Num(); ++i)
            {
                int32 Need = R.RequiredCounts.IsValidIndex(i) ? R.RequiredCounts[i] : 1;
                int32& Current = ResourceInventory.FindOrAdd(R.RequiredResources[i]);
                Current = FMath::Max(0, Current - Need);
            }
            // Add output
            AddResource(R.OutputItem, 1);
            UE_LOG(LogTemp, Log, TEXT("[QuestManager] Crafted: %s"), *R.OutputItem);
            return true;
        }
    }
    return false;
}

TArray<FQuest_CraftingRecipe> UQuestManager::GetAvailableRecipes() const
{
    TArray<FQuest_CraftingRecipe> Available;
    for (const FQuest_CraftingRecipe& R : CraftingRecipes)
    {
        if (R.bUnlocked) Available.Add(R);
    }
    return Available;
}

// ============================================================
// Tick
// ============================================================

void UQuestManager::TickQuests(float DeltaTime)
{
    for (FQuest_Definition& Q : AllQuests)
    {
        if (Q.Status != EQuest_Status::Active) continue;

        if (Q.TimeLimit > 0.0f)
        {
            Q.ElapsedTime += DeltaTime;
            if (Q.ElapsedTime >= Q.TimeLimit)
            {
                FailQuest(Q.QuestID);
            }
        }
    }
}

// ============================================================
// Internal Helpers
// ============================================================

FQuest_Definition* UQuestManager::FindQuest(EQuest_ID QuestID)
{
    for (FQuest_Definition& Q : AllQuests)
    {
        if (Q.QuestID == QuestID) return &Q;
    }
    return nullptr;
}

const FQuest_Definition* UQuestManager::FindQuestConst(EQuest_ID QuestID) const
{
    for (const FQuest_Definition& Q : AllQuests)
    {
        if (Q.QuestID == QuestID) return &Q;
    }
    return nullptr;
}
