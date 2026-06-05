#include "Narr_StoryProgressionManager.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

UNarr_StoryProgressionManager::UNarr_StoryProgressionManager()
{
    CurrentChapterID = TEXT("Chapter_01_Awakening");
    CurrentChapterIndex = 0;
}

void UNarr_StoryProgressionManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    InitializeStorySystem();
}

void UNarr_StoryProgressionManager::InitializeStorySystem()
{
    SetupDefaultStoryChapters();
    SetupDefaultCharacterArcs();
    
    UE_LOG(LogTemp, Warning, TEXT("Story Progression Manager initialized with %d chapters"), StoryChapters.Num());
}

void UNarr_StoryProgressionManager::SetupDefaultStoryChapters()
{
    StoryChapters.Empty();

    // Chapter 1: Awakening
    FNarr_StoryChapter Chapter1;
    Chapter1.ChapterName = TEXT("The Awakening");
    Chapter1.ChapterDescription = TEXT("You awaken in a primordial world, alone and vulnerable. Learn to survive the basics of this dangerous land.");
    Chapter1.RequiredQuestIDs = {};
    Chapter1.UnlockedQuestIDs = {TEXT("Quest_FirstTools"), TEXT("Quest_FindShelter"), TEXT("Quest_FirstHunt")};
    Chapter1.bIsCompleted = false;
    Chapter1.ChapterOrder = 1;
    StoryChapters.Add(Chapter1);

    // Chapter 2: First Contact
    FNarr_StoryChapter Chapter2;
    Chapter2.ChapterName = TEXT("First Contact");
    Chapter2.ChapterDescription = TEXT("You encounter other survivors. Will they be allies or enemies in this harsh world?");
    Chapter2.RequiredQuestIDs = {TEXT("Quest_FirstTools"), TEXT("Quest_FindShelter")};
    Chapter2.UnlockedQuestIDs = {TEXT("Quest_MeetTribal"), TEXT("Quest_ProveWorth"), TEXT("Quest_LearnLanguage")};
    Chapter2.bIsCompleted = false;
    Chapter2.ChapterOrder = 2;
    StoryChapters.Add(Chapter2);

    // Chapter 3: The Hunt
    FNarr_StoryChapter Chapter3;
    Chapter3.ChapterName = TEXT("The Great Hunt");
    Chapter3.ChapterDescription = TEXT("A massive predator threatens the tribal lands. Join the hunt or face extinction.");
    Chapter3.RequiredQuestIDs = {TEXT("Quest_MeetTribal"), TEXT("Quest_ProveWorth")};
    Chapter3.UnlockedQuestIDs = {TEXT("Quest_TrackBeast"), TEXT("Quest_GatherHunters"), TEXT("Quest_FinalConfrontation")};
    Chapter3.bIsCompleted = false;
    Chapter3.ChapterOrder = 3;
    StoryChapters.Add(Chapter3);

    // Chapter 4: New Territories
    FNarr_StoryChapter Chapter4;
    Chapter4.ChapterName = TEXT("New Territories");
    Chapter4.ChapterDescription = TEXT("With the great threat defeated, explore new lands and establish your place in this world.");
    Chapter4.RequiredQuestIDs = {TEXT("Quest_FinalConfrontation")};
    Chapter4.UnlockedQuestIDs = {TEXT("Quest_ExploreNorth"), TEXT("Quest_BuildSettlement"), TEXT("Quest_TameBeasts")};
    Chapter4.bIsCompleted = false;
    Chapter4.ChapterOrder = 4;
    StoryChapters.Add(Chapter4);
}

void UNarr_StoryProgressionManager::SetupDefaultCharacterArcs()
{
    CharacterArcs.Empty();

    // Grimjaw - Tribal Chief
    FNarr_CharacterArc GrimjawArc;
    GrimjawArc.CharacterID = TEXT("Chief_Grimjaw");
    GrimjawArc.CharacterName = TEXT("Chief Grimjaw Beastslayer");
    GrimjawArc.CompletedEvents = {};
    GrimjawArc.AvailableEvents = {TEXT("Event_FirstMeeting"), TEXT("Event_ProveWorth"), TEXT("Event_GainTrust")};
    GrimjawArc.RelationshipLevel = 0.0f;
    GrimjawArc.bIsAlive = true;
    CharacterArcs.Add(GrimjawArc);

    // Stoneheart - Elder
    FNarr_CharacterArc StoneheartArc;
    StoneheartArc.CharacterID = TEXT("Elder_Stoneheart");
    StoneheartArc.CharacterName = TEXT("Elder Stoneheart Lorekeeper");
    StoneheartArc.CompletedEvents = {};
    StoneheartArc.AvailableEvents = {TEXT("Event_LearnLore"), TEXT("Event_AncientWisdom"), TEXT("Event_SacredRites")};
    StoneheartArc.RelationshipLevel = 0.0f;
    StoneheartArc.bIsAlive = true;
    CharacterArcs.Add(StoneheartArc);

    // Vex - Scout
    FNarr_CharacterArc VexArc;
    VexArc.CharacterID = TEXT("Scout_Vex");
    VexArc.CharacterName = TEXT("Vex Pathfinder");
    VexArc.CompletedEvents = {};
    VexArc.AvailableEvents = {TEXT("Event_LearnPaths"), TEXT("Event_DangerWarning"), TEXT("Event_SecretRoute")};
    VexArc.RelationshipLevel = 0.0f;
    VexArc.bIsAlive = true;
    CharacterArcs.Add(VexArc);
}

void UNarr_StoryProgressionManager::CompleteChapter(const FString& ChapterID)
{
    for (FNarr_StoryChapter& Chapter : StoryChapters)
    {
        if (Chapter.ChapterName.Contains(ChapterID) || Chapter.ChapterOrder == CurrentChapterIndex + 1)
        {
            Chapter.bIsCompleted = true;
            UnlockNewQuests(Chapter);
            CurrentChapterIndex++;
            
            if (CurrentChapterIndex < StoryChapters.Num())
            {
                CurrentChapterID = StoryChapters[CurrentChapterIndex].ChapterName;
            }
            
            UE_LOG(LogTemp, Warning, TEXT("Chapter completed: %s"), *Chapter.ChapterName);
            SaveStoryProgress();
            break;
        }
    }
}

bool UNarr_StoryProgressionManager::IsChapterUnlocked(const FString& ChapterID) const
{
    for (const FNarr_StoryChapter& Chapter : StoryChapters)
    {
        if (Chapter.ChapterName.Contains(ChapterID))
        {
            return CheckChapterRequirements(Chapter);
        }
    }
    return false;
}

FNarr_StoryChapter UNarr_StoryProgressionManager::GetCurrentChapter() const
{
    if (CurrentChapterIndex >= 0 && CurrentChapterIndex < StoryChapters.Num())
    {
        return StoryChapters[CurrentChapterIndex];
    }
    return FNarr_StoryChapter();
}

TArray<FNarr_StoryChapter> UNarr_StoryProgressionManager::GetAvailableChapters() const
{
    TArray<FNarr_StoryChapter> AvailableChapters;
    
    for (const FNarr_StoryChapter& Chapter : StoryChapters)
    {
        if (CheckChapterRequirements(Chapter) && !Chapter.bIsCompleted)
        {
            AvailableChapters.Add(Chapter);
        }
    }
    
    return AvailableChapters;
}

void UNarr_StoryProgressionManager::UpdateCharacterRelationship(const FString& CharacterID, float DeltaRelationship)
{
    for (FNarr_CharacterArc& Arc : CharacterArcs)
    {
        if (Arc.CharacterID == CharacterID)
        {
            Arc.RelationshipLevel = FMath::Clamp(Arc.RelationshipLevel + DeltaRelationship, -100.0f, 100.0f);
            UE_LOG(LogTemp, Warning, TEXT("Character %s relationship updated to %f"), *Arc.CharacterName, Arc.RelationshipLevel);
            break;
        }
    }
}

void UNarr_StoryProgressionManager::TriggerCharacterEvent(const FString& CharacterID, const FString& EventID)
{
    for (FNarr_CharacterArc& Arc : CharacterArcs)
    {
        if (Arc.CharacterID == CharacterID)
        {
            if (Arc.AvailableEvents.Contains(EventID))
            {
                Arc.AvailableEvents.Remove(EventID);
                Arc.CompletedEvents.Add(EventID);
                UE_LOG(LogTemp, Warning, TEXT("Character event triggered: %s - %s"), *Arc.CharacterName, *EventID);
            }
            break;
        }
    }
}

FNarr_CharacterArc UNarr_StoryProgressionManager::GetCharacterArc(const FString& CharacterID) const
{
    for (const FNarr_CharacterArc& Arc : CharacterArcs)
    {
        if (Arc.CharacterID == CharacterID)
        {
            return Arc;
        }
    }
    return FNarr_CharacterArc();
}

bool UNarr_StoryProgressionManager::CheckChapterRequirements(const FNarr_StoryChapter& Chapter) const
{
    if (Chapter.RequiredQuestIDs.Num() == 0)
    {
        return true; // No requirements
    }
    
    // For now, assume all required quests are completed
    // In a full implementation, this would check against a quest manager
    return true;
}

void UNarr_StoryProgressionManager::UnlockNewQuests(const FNarr_StoryChapter& Chapter)
{
    for (const FString& QuestID : Chapter.UnlockedQuestIDs)
    {
        UE_LOG(LogTemp, Warning, TEXT("Quest unlocked: %s"), *QuestID);
        // In a full implementation, this would notify the quest manager
    }
}

void UNarr_StoryProgressionManager::SaveStoryProgress()
{
    UE_LOG(LogTemp, Warning, TEXT("Story progress saved - Current Chapter: %s"), *CurrentChapterID);
    // Implementation would save to persistent storage
}

void UNarr_StoryProgressionManager::LoadStoryProgress()
{
    UE_LOG(LogTemp, Warning, TEXT("Story progress loaded"));
    // Implementation would load from persistent storage
}