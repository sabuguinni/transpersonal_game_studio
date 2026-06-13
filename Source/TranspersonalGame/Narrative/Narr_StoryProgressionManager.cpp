#include "Narr_StoryProgressionManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

void UNarr_StoryProgressionManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    PlayerMorality = 0.0f; // Neutral starting morality
    TotalQuestsCompleted = 0;
    CurrentChapterID = "chapter_01_arrival";
    
    InitializeStoryChapters();
}

void UNarr_StoryProgressionManager::InitializeStoryChapters()
{
    StoryChapters.Empty();
    SetupDefaultChapters();
}

void UNarr_StoryProgressionManager::SetupDefaultChapters()
{
    // Chapter 1: Arrival
    FNarr_StoryChapter Chapter1;
    Chapter1.ChapterID = "chapter_01_arrival";
    Chapter1.ChapterTitle = "First Steps";
    Chapter1.ChapterDescription = "You awaken in a strange prehistoric world. Survival is your first priority.";
    Chapter1.ChapterOrder = 1;
    Chapter1.NarrativeText = "The sun beats down mercilessly as you struggle to your feet. This world is not your own - massive creatures roam the landscape, and danger lurks behind every rock and tree.";
    Chapter1.UnlockedQuestIDs.Add("quest_find_water");
    Chapter1.UnlockedQuestIDs.Add("quest_basic_shelter");
    Chapter1.bIsCompleted = false;
    StoryChapters.Add(Chapter1);
    
    // Chapter 2: First Contact
    FNarr_StoryChapter Chapter2;
    Chapter2.ChapterID = "chapter_02_first_contact";
    Chapter2.ChapterTitle = "Voices in the Wilderness";
    Chapter2.ChapterDescription = "You encounter other survivors. Trust must be earned in this harsh world.";
    Chapter2.ChapterOrder = 2;
    Chapter2.RequiredQuestIDs.Add("quest_find_water");
    Chapter2.NarrativeText = "Smoke rises in the distance - a sign of other humans. But in this world, not all survivors can be trusted.";
    Chapter2.UnlockedQuestIDs.Add("quest_meet_elder");
    Chapter2.UnlockedQuestIDs.Add("quest_prove_worth");
    Chapter2.bIsCompleted = false;
    StoryChapters.Add(Chapter2);
    
    // Chapter 3: The Hunt Begins
    FNarr_StoryChapter Chapter3;
    Chapter3.ChapterID = "chapter_03_hunt_begins";
    Chapter3.ChapterTitle = "Predator and Prey";
    Chapter3.ChapterDescription = "Learn to hunt or become the hunted. The food chain is unforgiving.";
    Chapter3.ChapterOrder = 3;
    Chapter3.RequiredQuestIDs.Add("quest_meet_elder");
    Chapter3.NarrativeText = "The elder's weathered hands gesture toward the hunting grounds. 'Out there, you are either predator or prey. Choose wisely.'";
    Chapter3.UnlockedQuestIDs.Add("quest_first_hunt");
    Chapter3.UnlockedQuestIDs.Add("quest_craft_weapons");
    Chapter3.bIsCompleted = false;
    StoryChapters.Add(Chapter3);
    
    // Chapter 4: Territory Wars
    FNarr_StoryChapter Chapter4;
    Chapter4.ChapterID = "chapter_04_territory_wars";
    Chapter4.ChapterTitle = "Claiming Ground";
    Chapter4.ChapterDescription = "Establish your place in this world. Territory means survival.";
    Chapter4.ChapterOrder = 4;
    Chapter4.RequiredQuestIDs.Add("quest_first_hunt");
    Chapter4.NarrativeText = "The pack of raptors watches from the ridge. This valley could sustain your growing tribe, but it won't be given freely.";
    Chapter4.UnlockedQuestIDs.Add("quest_defend_territory");
    Chapter4.UnlockedQuestIDs.Add("quest_expand_settlement");
    Chapter4.bIsCompleted = false;
    StoryChapters.Add(Chapter4);
    
    // Chapter 5: The Great Migration
    FNarr_StoryChapter Chapter5;
    Chapter5.ChapterID = "chapter_05_great_migration";
    Chapter5.ChapterTitle = "Following the Herds";
    Chapter5.ChapterDescription = "The seasons change, and with them, the great beasts migrate. Adapt or perish.";
    Chapter5.ChapterOrder = 5;
    Chapter5.RequiredQuestIDs.Add("quest_defend_territory");
    Chapter5.NarrativeText = "The ground trembles as thousands of massive herbivores begin their ancient journey. Your survival depends on following the herds.";
    Chapter5.UnlockedQuestIDs.Add("quest_track_migration");
    Chapter5.UnlockedQuestIDs.Add("quest_survive_winter");
    Chapter5.bIsCompleted = false;
    StoryChapters.Add(Chapter5);
}

bool UNarr_StoryProgressionManager::AdvanceToNextChapter()
{
    FNarr_StoryChapter* CurrentChapter = nullptr;
    for (FNarr_StoryChapter& Chapter : StoryChapters)
    {
        if (Chapter.ChapterID == CurrentChapterID)
        {
            CurrentChapter = &Chapter;
            break;
        }
    }
    
    if (!CurrentChapter)
    {
        return false;
    }
    
    // Find next chapter
    for (FNarr_StoryChapter& Chapter : StoryChapters)
    {
        if (Chapter.ChapterOrder == CurrentChapter->ChapterOrder + 1)
        {
            if (CheckChapterRequirements(Chapter))
            {
                CurrentChapterID = Chapter.ChapterID;
                TriggerChapterEvents(Chapter.ChapterID);
                return true;
            }
            break;
        }
    }
    
    return false;
}

FNarr_StoryChapter UNarr_StoryProgressionManager::GetCurrentChapter()
{
    for (const FNarr_StoryChapter& Chapter : StoryChapters)
    {
        if (Chapter.ChapterID == CurrentChapterID)
        {
            return Chapter;
        }
    }
    
    // Return default chapter if not found
    FNarr_StoryChapter DefaultChapter;
    DefaultChapter.ChapterID = "default";
    DefaultChapter.ChapterTitle = "Unknown Chapter";
    DefaultChapter.ChapterDescription = "Chapter data not found.";
    return DefaultChapter;
}

bool UNarr_StoryProgressionManager::IsChapterUnlocked(const FString& ChapterID)
{
    for (const FNarr_StoryChapter& Chapter : StoryChapters)
    {
        if (Chapter.ChapterID == ChapterID)
        {
            return CheckChapterRequirements(Chapter);
        }
    }
    return false;
}

void UNarr_StoryProgressionManager::CompleteChapter(const FString& ChapterID)
{
    for (FNarr_StoryChapter& Chapter : StoryChapters)
    {
        if (Chapter.ChapterID == ChapterID)
        {
            Chapter.bIsCompleted = true;
            
            // Unlock regions associated with this chapter
            if (ChapterID == "chapter_02_first_contact")
            {
                UnlockRegion("Village");
            }
            else if (ChapterID == "chapter_03_hunt_begins")
            {
                UnlockRegion("HuntingGrounds");
            }
            else if (ChapterID == "chapter_04_territory_wars")
            {
                UnlockRegion("NorthernValley");
            }
            
            break;
        }
    }
}

void UNarr_StoryProgressionManager::RecordPlayerChoice(const FNarr_PlayerChoice& Choice)
{
    PlayerChoiceHistory.Add(Choice);
    ModifyPlayerMorality(Choice.MoralityImpact);
    
    // Update NPC relationships based on choice
    for (const FString& NPCID : Choice.AffectedNPCs)
    {
        int32 RelationshipDelta = Choice.MoralityImpact > 0 ? 1 : -1;
        UpdateNPCRelationship(NPCID, RelationshipDelta);
    }
}

float UNarr_StoryProgressionManager::GetPlayerMorality()
{
    return PlayerMorality;
}

void UNarr_StoryProgressionManager::ModifyPlayerMorality(float Delta)
{
    PlayerMorality = FMath::Clamp(PlayerMorality + Delta, -100.0f, 100.0f);
}

TArray<FString> UNarr_StoryProgressionManager::GetUnlockedQuests()
{
    TArray<FString> UnlockedQuests;
    
    for (const FNarr_StoryChapter& Chapter : StoryChapters)
    {
        if (IsChapterUnlocked(Chapter.ChapterID))
        {
            UnlockedQuests.Append(Chapter.UnlockedQuestIDs);
        }
    }
    
    return UnlockedQuests;
}

void UNarr_StoryProgressionManager::UnlockRegion(const FString& RegionName)
{
    UnlockedRegions.AddUnique(RegionName);
}

bool UNarr_StoryProgressionManager::IsRegionUnlocked(const FString& RegionName)
{
    return UnlockedRegions.Contains(RegionName);
}

void UNarr_StoryProgressionManager::UpdateNPCRelationship(const FString& NPCID, int32 Delta)
{
    if (NPCRelationshipLevels.Contains(NPCID))
    {
        NPCRelationshipLevels[NPCID] += Delta;
    }
    else
    {
        NPCRelationshipLevels.Add(NPCID, Delta);
    }
    
    // Clamp relationship levels
    NPCRelationshipLevels[NPCID] = FMath::Clamp(NPCRelationshipLevels[NPCID], -100, 100);
}

int32 UNarr_StoryProgressionManager::GetNPCRelationshipLevel(const FString& NPCID)
{
    if (NPCRelationshipLevels.Contains(NPCID))
    {
        return NPCRelationshipLevels[NPCID];
    }
    return 0; // Neutral relationship
}

TArray<FString> UNarr_StoryProgressionManager::GetStoryMilestones()
{
    TArray<FString> Milestones;
    
    for (const FNarr_StoryChapter& Chapter : StoryChapters)
    {
        if (Chapter.bIsCompleted)
        {
            Milestones.Add(Chapter.ChapterTitle);
        }
    }
    
    return Milestones;
}

bool UNarr_StoryProgressionManager::CheckChapterRequirements(const FNarr_StoryChapter& Chapter)
{
    // First chapter is always unlocked
    if (Chapter.ChapterOrder == 1)
    {
        return true;
    }
    
    // Check if all required quests are completed
    for (const FString& RequiredQuestID : Chapter.RequiredQuestIDs)
    {
        // This would need integration with quest system
        // For now, assume requirements are met if previous chapter is completed
        bool bPreviousChapterCompleted = false;
        for (const FNarr_StoryChapter& PrevChapter : StoryChapters)
        {
            if (PrevChapter.ChapterOrder == Chapter.ChapterOrder - 1)
            {
                bPreviousChapterCompleted = PrevChapter.bIsCompleted;
                break;
            }
        }
        
        if (!bPreviousChapterCompleted)
        {
            return false;
        }
    }
    
    return true;
}

void UNarr_StoryProgressionManager::TriggerChapterEvents(const FString& ChapterID)
{
    // Trigger events specific to each chapter
    if (ChapterID == "chapter_02_first_contact")
    {
        // Spawn NPCs in the world
        UnlockRegion("Village");
    }
    else if (ChapterID == "chapter_03_hunt_begins")
    {
        // Enable hunting mechanics
        UnlockRegion("HuntingGrounds");
    }
}