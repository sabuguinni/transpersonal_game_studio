// QuestManager.cpp — Tutorial Quest System Implementation
// Agent #4 — Performance Optimizer | Cycle PROD_CYCLE_AUTO_20260627_005
// Prehistoric survival tutorial: Find Water → Find Food → Craft Tool → Find Shelter

#include "QuestManager.h"
#include "Engine/World.h"
#include "TimerManager.h"

// ── Static tutorial sequence ──────────────────────────────────────────────
const TArray<FString> AQuestManager::TutorialSequence = {
    TEXT("tutorial_find_water"),
    TEXT("tutorial_find_food"),
    TEXT("tutorial_craft_tool"),
    TEXT("tutorial_find_shelter")
};

// ── Constructor ───────────────────────────────────────────────────────────
AQuestManager::AQuestManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 1.0f;  // Check quest timers every second — not every frame

    bAutoStartTutorial = true;
    TutorialStepIndex = 0;
}

// ── BeginPlay ─────────────────────────────────────────────────────────────
void AQuestManager::BeginPlay()
{
    Super::BeginPlay();

    if (bAutoStartTutorial)
    {
        // Pre-load all tutorial quests into the registry (inactive)
        ActiveQuests.Add(BuildTutorialFindWater());
        ActiveQuests.Add(BuildTutorialFindFood());
        ActiveQuests.Add(BuildTutorialCraftTool());
        ActiveQuests.Add(BuildTutorialFindShelter());

        // Start the first tutorial quest
        StartQuest(TEXT("tutorial_find_water"));
    }
}

// ── Tick — update quest timers ────────────────────────────────────────────
void AQuestManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    for (FQuest_Data& Quest : ActiveQuests)
    {
        if (Quest.QuestState != EQuest_State::Active)
            continue;

        // Update elapsed time
        Quest.ElapsedTime += DeltaTime;

        // Check time limit
        if (Quest.TimeLimit > 0.0f && Quest.ElapsedTime >= Quest.TimeLimit)
        {
            FailQuest(Quest.QuestID);
        }
    }
}

// ── Quest Management ──────────────────────────────────────────────────────
void AQuestManager::StartQuest(const FString& QuestID)
{
    int32 Idx = FindQuestIndex(ActiveQuests, QuestID);
    if (Idx == INDEX_NONE)
    {
        UE_LOG(LogTemp, Warning, TEXT("QuestManager: Quest '%s' not found in registry"), *QuestID);
        return;
    }

    ActiveQuests[Idx].QuestState = EQuest_State::Active;
    ActiveQuests[Idx].ElapsedTime = 0.0f;

    UE_LOG(LogTemp, Log, TEXT("QuestManager: Quest STARTED — '%s': %s"),
        *ActiveQuests[Idx].QuestTitle,
        *ActiveQuests[Idx].QuestDescription);
}

void AQuestManager::CompleteObjective(const FString& QuestID, const FString& ObjectiveID, float Value)
{
    int32 QIdx = FindQuestIndex(ActiveQuests, QuestID);
    if (QIdx == INDEX_NONE) return;

    FQuest_Data& Quest = ActiveQuests[QIdx];
    if (Quest.QuestState != EQuest_State::Active) return;

    for (FQuest_Objective& Obj : Quest.Objectives)
    {
        if (Obj.ObjectiveID == ObjectiveID && !Obj.bCompleted)
        {
            Obj.CurrentValue = FMath::Min(Obj.CurrentValue + Value, Obj.TargetValue);
            if (Obj.CurrentValue >= Obj.TargetValue)
            {
                Obj.bCompleted = true;
                UE_LOG(LogTemp, Log, TEXT("QuestManager: Objective COMPLETE — [%s] %s"),
                    *QuestID, *Obj.Description);
            }
            break;
        }
    }

    CheckQuestCompletion(Quest);
}

void AQuestManager::FailQuest(const FString& QuestID)
{
    int32 Idx = FindQuestIndex(ActiveQuests, QuestID);
    if (Idx == INDEX_NONE) return;

    ActiveQuests[Idx].QuestState = EQuest_State::Failed;
    UE_LOG(LogTemp, Warning, TEXT("QuestManager: Quest FAILED — '%s'"), *ActiveQuests[Idx].QuestTitle);
}

bool AQuestManager::IsQuestActive(const FString& QuestID) const
{
    int32 Idx = FindQuestIndex(ActiveQuests, QuestID);
    return Idx != INDEX_NONE && ActiveQuests[Idx].QuestState == EQuest_State::Active;
}

bool AQuestManager::IsQuestCompleted(const FString& QuestID) const
{
    return FindQuestIndex(CompletedQuests, QuestID) != INDEX_NONE;
}

FQuest_Data AQuestManager::GetQuestData(const FString& QuestID) const
{
    int32 Idx = FindQuestIndex(ActiveQuests, QuestID);
    if (Idx != INDEX_NONE) return ActiveQuests[Idx];
    Idx = FindQuestIndex(CompletedQuests, QuestID);
    if (Idx != INDEX_NONE) return CompletedQuests[Idx];
    return FQuest_Data();
}

FString AQuestManager::GetActiveQuestObjectiveText() const
{
    for (const FQuest_Data& Quest : ActiveQuests)
    {
        if (Quest.QuestState != EQuest_State::Active) continue;
        for (const FQuest_Objective& Obj : Quest.Objectives)
        {
            if (!Obj.bCompleted)
            {
                return FString::Printf(TEXT("[%s] %s (%.0f/%.0f)"),
                    *Quest.QuestTitle, *Obj.Description,
                    Obj.CurrentValue, Obj.TargetValue);
            }
        }
    }
    return TEXT("No active objective");
}

// ── Tutorial Quest Triggers ───────────────────────────────────────────────
void AQuestManager::OnPlayerReachedWaterSource()
{
    CompleteObjective(TEXT("tutorial_find_water"), TEXT("reach_water"), 1.0f);
}

void AQuestManager::OnPlayerDrank(float Amount)
{
    CompleteObjective(TEXT("tutorial_find_water"), TEXT("drink_water"), Amount);
}

void AQuestManager::OnPlayerHungerLow(float HungerValue)
{
    // Auto-activate food quest when hunger drops below 40
    if (HungerValue < 40.0f && IsQuestCompleted(TEXT("tutorial_find_water")))
    {
        if (!IsQuestActive(TEXT("tutorial_find_food")))
        {
            StartQuest(TEXT("tutorial_find_food"));
        }
    }
}

void AQuestManager::OnPlayerCraftedTool()
{
    CompleteObjective(TEXT("tutorial_craft_tool"), TEXT("craft_stone_tool"), 1.0f);
}

// ── Survival Stat Hooks ───────────────────────────────────────────────────
void AQuestManager::OnThirstChanged(float NewThirst)
{
    // Trigger Find Water quest if thirst drops below 30 and quest not yet started
    if (NewThirst < 30.0f && !IsQuestActive(TEXT("tutorial_find_water"))
        && !IsQuestCompleted(TEXT("tutorial_find_water")))
    {
        StartQuest(TEXT("tutorial_find_water"));
    }
}

void AQuestManager::OnHungerChanged(float NewHunger)
{
    OnPlayerHungerLow(NewHunger);
}

void AQuestManager::OnPlayerTookDamage(float DamageAmount)
{
    // Future: trigger "Find Shelter" quest if player is taking repeated damage
    if (DamageAmount > 20.0f && IsQuestCompleted(TEXT("tutorial_craft_tool")))
    {
        if (!IsQuestActive(TEXT("tutorial_find_shelter"))
            && !IsQuestCompleted(TEXT("tutorial_find_shelter")))
        {
            StartQuest(TEXT("tutorial_find_shelter"));
        }
    }
}

// ── Internal Quest Builders ───────────────────────────────────────────────
FQuest_Data AQuestManager::BuildTutorialFindWater() const
{
    FQuest_Data Q;
    Q.QuestID = TEXT("tutorial_find_water");
    Q.QuestTitle = TEXT("Find Water");
    Q.QuestDescription = TEXT("You are dehydrating. Find a water source before you collapse.");
    Q.QuestType = EQuest_Type::Tutorial;
    Q.QuestState = EQuest_State::Inactive;
    Q.TimeLimit = 0.0f;  // No time limit — player must learn naturally

    FQuest_Objective ObjReach;
    ObjReach.ObjectiveID = TEXT("reach_water");
    ObjReach.Description = TEXT("Reach the water source");
    ObjReach.TargetValue = 1.0f;
    ObjReach.CurrentValue = 0.0f;

    FQuest_Objective ObjDrink;
    ObjDrink.ObjectiveID = TEXT("drink_water");
    ObjDrink.Description = TEXT("Drink water (Thirst +50)");
    ObjDrink.TargetValue = 50.0f;
    ObjDrink.CurrentValue = 0.0f;

    Q.Objectives.Add(ObjReach);
    Q.Objectives.Add(ObjDrink);
    return Q;
}

FQuest_Data AQuestManager::BuildTutorialFindFood() const
{
    FQuest_Data Q;
    Q.QuestID = TEXT("tutorial_find_food");
    Q.QuestTitle = TEXT("Find Food");
    Q.QuestDescription = TEXT("Hunger weakens you. Find something to eat — berries, roots, or hunt small prey.");
    Q.QuestType = EQuest_Type::Tutorial;
    Q.QuestState = EQuest_State::Inactive;
    Q.TimeLimit = 0.0f;

    FQuest_Objective ObjEat;
    ObjEat.ObjectiveID = TEXT("eat_food");
    ObjEat.Description = TEXT("Eat food (Hunger +30)");
    ObjEat.TargetValue = 30.0f;
    ObjEat.CurrentValue = 0.0f;

    Q.Objectives.Add(ObjEat);
    return Q;
}

FQuest_Data AQuestManager::BuildTutorialCraftTool() const
{
    FQuest_Data Q;
    Q.QuestID = TEXT("tutorial_craft_tool");
    Q.QuestTitle = TEXT("Craft a Stone Tool");
    Q.QuestDescription = TEXT("Bare hands won't keep you alive. Find flint and craft a basic stone tool.");
    Q.QuestType = EQuest_Type::Craft;
    Q.QuestState = EQuest_State::Inactive;
    Q.TimeLimit = 0.0f;

    FQuest_Objective ObjGather;
    ObjGather.ObjectiveID = TEXT("gather_flint");
    ObjGather.Description = TEXT("Gather flint stones (0/3)");
    ObjGather.TargetValue = 3.0f;
    ObjGather.CurrentValue = 0.0f;

    FQuest_Objective ObjCraft;
    ObjCraft.ObjectiveID = TEXT("craft_stone_tool");
    ObjCraft.Description = TEXT("Craft a stone knife");
    ObjCraft.TargetValue = 1.0f;
    ObjCraft.CurrentValue = 0.0f;

    Q.Objectives.Add(ObjGather);
    Q.Objectives.Add(ObjCraft);
    return Q;
}

FQuest_Data AQuestManager::BuildTutorialFindShelter() const
{
    FQuest_Data Q;
    Q.QuestID = TEXT("tutorial_find_shelter");
    Q.QuestTitle = TEXT("Find Shelter Before Dark");
    Q.QuestDescription = TEXT("Night is coming. Predators are more active in the dark. Find a cave or build a shelter.");
    Q.QuestType = EQuest_Type::Survival;
    Q.QuestState = EQuest_State::Inactive;
    Q.TimeLimit = 0.0f;

    FQuest_Objective ObjShelter;
    ObjShelter.ObjectiveID = TEXT("reach_shelter");
    ObjShelter.Description = TEXT("Reach a shelter or cave");
    ObjShelter.TargetValue = 1.0f;
    ObjShelter.CurrentValue = 0.0f;

    Q.Objectives.Add(ObjShelter);
    return Q;
}

// ── Internal Helpers ──────────────────────────────────────────────────────
void AQuestManager::CheckQuestCompletion(FQuest_Data& Quest)
{
    if (Quest.QuestState != EQuest_State::Active) return;

    bool bAllComplete = true;
    for (const FQuest_Objective& Obj : Quest.Objectives)
    {
        if (!Obj.bCompleted)
        {
            bAllComplete = false;
            break;
        }
    }

    if (bAllComplete)
    {
        Quest.QuestState = EQuest_State::Completed;
        OnQuestCompleted(Quest);
    }
}

void AQuestManager::OnQuestCompleted(const FQuest_Data& Quest)
{
    UE_LOG(LogTemp, Log, TEXT("QuestManager: Quest COMPLETED — '%s'"), *Quest.QuestTitle);

    // Move to completed list
    CompletedQuests.Add(Quest);

    // Remove from active list
    int32 Idx = FindQuestIndex(ActiveQuests, Quest.QuestID);
    if (Idx != INDEX_NONE)
    {
        ActiveQuests.RemoveAt(Idx);
    }

    // Auto-advance tutorial sequence
    TutorialStepIndex++;
    if (TutorialStepIndex < TutorialSequence.Num())
    {
        const FString& NextQuestID = TutorialSequence[TutorialStepIndex];
        // Re-add next quest to active list if not already there
        int32 NextIdx = FindQuestIndex(ActiveQuests, NextQuestID);
        if (NextIdx == INDEX_NONE)
        {
            // Build and add the next quest
            if (NextQuestID == TEXT("tutorial_find_food"))
                ActiveQuests.Add(BuildTutorialFindFood());
            else if (NextQuestID == TEXT("tutorial_craft_tool"))
                ActiveQuests.Add(BuildTutorialCraftTool());
            else if (NextQuestID == TEXT("tutorial_find_shelter"))
                ActiveQuests.Add(BuildTutorialFindShelter());
        }
        StartQuest(NextQuestID);
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("QuestManager: Tutorial sequence COMPLETE — player is ready to survive"));
    }
}

int32 AQuestManager::FindQuestIndex(const TArray<FQuest_Data>& QuestList, const FString& QuestID) const
{
    for (int32 i = 0; i < QuestList.Num(); ++i)
    {
        if (QuestList[i].QuestID == QuestID)
            return i;
    }
    return INDEX_NONE;
}
