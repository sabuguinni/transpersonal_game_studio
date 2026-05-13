#include "Quest_PrehistoricStorylineManager.h"
#include "Quest_SurvivalMissionManager.h"
#include "../Character/TranspersonalCharacter.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

UQuest_PrehistoricStorylineManager::UQuest_PrehistoricStorylineManager()
{
    CurrentChapterIndex = 0;
    SurvivalMissionManager = nullptr;
}

void UQuest_PrehistoricStorylineManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    // Get survival mission manager reference
    SurvivalMissionManager = GetGameInstance()->GetSubsystem<UQuest_SurvivalMissionManager>();
    
    InitializeStoryline();
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_PrehistoricStorylineManager initialized with %d chapters"), StorylineChapters.Num());
}

void UQuest_PrehistoricStorylineManager::Deinitialize()
{
    StorylineChapters.Empty();
    CompletedStoryBeats.Empty();
    UnlockedMissions.Empty();
    
    Super::Deinitialize();
}

void UQuest_PrehistoricStorylineManager::InitializeStoryline()
{
    StorylineChapters.Empty();
    CompletedStoryBeats.Empty();
    UnlockedMissions.Empty();
    CurrentChapterIndex = 0;
    
    CreatePrehistoricStoryline();
    
    // Unlock first chapter
    if (StorylineChapters.Num() > 0)
    {
        StorylineChapters[0].bIsUnlocked = true;
    }
}

void UQuest_PrehistoricStorylineManager::CreatePrehistoricStoryline()
{
    CreateChapter1_FirstSteps();
    CreateChapter2_TribalFormation();
    CreateChapter3_TerritoryExpansion();
    CreateChapter4_DinosaurMastery();
    CreateChapter5_CivilizationDawn();
}

void UQuest_PrehistoricStorylineManager::CreateChapter1_FirstSteps()
{
    FQuest_PrehistoricChapter Chapter1;
    Chapter1.ChapterID = TEXT("Chapter1_FirstSteps");
    Chapter1.ChapterTitle = TEXT("First Steps in the Prehistoric World");
    Chapter1.ChapterDescription = TEXT("Learn the basics of survival in a dangerous prehistoric world filled with massive predators.");
    Chapter1.bIsUnlocked = true;
    
    // Story Beat 1: Awakening
    FQuest_StoryBeat Beat1;
    Beat1.BeatID = TEXT("Beat1_Awakening");
    Beat1.BeatTitle = TEXT("Awakening");
    Beat1.BeatDescription = TEXT("You wake up in an unfamiliar prehistoric landscape. Learn to move and observe your surroundings.");
    Beat1.RequiredSurvivalLevel = 1;
    Beat1.UnlockedMissions.Add(TEXT("Tutorial_Movement"));
    Beat1.UnlockedMissions.Add(TEXT("Tutorial_BasicSurvival"));
    
    // Story Beat 2: First Hunt
    FQuest_StoryBeat Beat2;
    Beat2.BeatID = TEXT("Beat1_FirstHunt");
    Beat2.BeatTitle = TEXT("First Hunt");
    Beat2.BeatDescription = TEXT("Hunger drives you to hunt small prey. Craft your first weapon and learn to track animals.");
    Beat2.RequiredCompletedBeats.Add(TEXT("Beat1_Awakening"));
    Beat2.RequiredSurvivalLevel = 2;
    Beat2.UnlockedMissions.Add(TEXT("Hunt_SmallPrey"));
    Beat2.UnlockedMissions.Add(TEXT("Craft_StoneKnife"));
    
    // Story Beat 3: Shelter Building
    FQuest_StoryBeat Beat3;
    Beat3.BeatID = TEXT("Beat1_FirstShelter");
    Beat3.BeatTitle = TEXT("First Shelter");
    Beat3.BeatDescription = TEXT("Night approaches and predators emerge. Build your first shelter to survive the darkness.");
    Beat3.RequiredCompletedBeats.Add(TEXT("Beat1_FirstHunt"));
    Beat3.RequiredSurvivalLevel = 3;
    Beat3.UnlockedMissions.Add(TEXT("Build_BasicShelter"));
    Beat3.UnlockedMissions.Add(TEXT("Gather_ShelterMaterials"));
    
    Chapter1.StoryBeats.Add(Beat1);
    Chapter1.StoryBeats.Add(Beat2);
    Chapter1.StoryBeats.Add(Beat3);
    
    StorylineChapters.Add(Chapter1);
}

void UQuest_PrehistoricStorylineManager::CreateChapter2_TribalFormation()
{
    FQuest_PrehistoricChapter Chapter2;
    Chapter2.ChapterID = TEXT("Chapter2_TribalFormation");
    Chapter2.ChapterTitle = TEXT("Forming the First Tribe");
    Chapter2.ChapterDescription = TEXT("Discover other survivors and learn the power of cooperation in the harsh prehistoric world.");
    
    // Story Beat 1: First Contact
    FQuest_StoryBeat Beat1;
    Beat1.BeatID = TEXT("Beat2_FirstContact");
    Beat1.BeatTitle = TEXT("First Contact");
    Beat1.BeatDescription = TEXT("Encounter other human survivors. Decide whether to approach peacefully or remain hidden.");
    Beat1.RequiredCompletedBeats.Add(TEXT("Beat1_FirstShelter"));
    Beat1.RequiredSurvivalLevel = 5;
    Beat1.UnlockedMissions.Add(TEXT("Encounter_OtherSurvivors"));
    Beat1.UnlockedMissions.Add(TEXT("Establish_Communication"));
    
    // Story Beat 2: Tribal Alliance
    FQuest_StoryBeat Beat2;
    Beat2.BeatID = TEXT("Beat2_TribalAlliance");
    Beat2.BeatTitle = TEXT("Tribal Alliance");
    Beat2.BeatDescription = TEXT("Form your first tribal alliance. Share knowledge and resources for mutual survival.");
    Beat2.RequiredCompletedBeats.Add(TEXT("Beat2_FirstContact"));
    Beat2.RequiredSurvivalLevel = 7;
    Beat2.UnlockedMissions.Add(TEXT("Form_TribalAlliance"));
    Beat2.UnlockedMissions.Add(TEXT("Share_SurvivalKnowledge"));
    
    Chapter2.StoryBeats.Add(Beat1);
    Chapter2.StoryBeats.Add(Beat2);
    
    StorylineChapters.Add(Chapter2);
}

void UQuest_PrehistoricStorylineManager::CreateChapter3_TerritoryExpansion()
{
    FQuest_PrehistoricChapter Chapter3;
    Chapter3.ChapterID = TEXT("Chapter3_TerritoryExpansion");
    Chapter3.ChapterTitle = TEXT("Expanding Territory");
    Chapter3.ChapterDescription = TEXT("Explore new biomes and establish territorial control over resource-rich areas.");
    
    // Story Beat 1: Biome Discovery
    FQuest_StoryBeat Beat1;
    Beat1.BeatID = TEXT("Beat3_BiomeDiscovery");
    Beat1.BeatTitle = TEXT("New Lands");
    Beat1.BeatDescription = TEXT("Discover different biomes with unique resources and dangers. Map the prehistoric landscape.");
    Beat1.RequiredCompletedBeats.Add(TEXT("Beat2_TribalAlliance"));
    Beat1.RequiredSurvivalLevel = 10;
    Beat1.UnlockedMissions.Add(TEXT("Explore_NewBiomes"));
    Beat1.UnlockedMissions.Add(TEXT("Map_Territory"));
    
    Chapter3.StoryBeats.Add(Beat1);
    StorylineChapters.Add(Chapter3);
}

void UQuest_PrehistoricStorylineManager::CreateChapter4_DinosaurMastery()
{
    FQuest_PrehistoricChapter Chapter4;
    Chapter4.ChapterID = TEXT("Chapter4_DinosaurMastery");
    Chapter4.ChapterTitle = TEXT("Mastering the Ancient Beasts");
    Chapter4.ChapterDescription = TEXT("Learn to coexist with, hunt, and even domesticate the mighty dinosaurs that rule this world.");
    
    // Story Beat 1: First Dinosaur Encounter
    FQuest_StoryBeat Beat1;
    Beat1.BeatID = TEXT("Beat4_FirstDinosaurEncounter");
    Beat1.BeatTitle = TEXT("Giants of the Past");
    Beat1.BeatDescription = TEXT("Face your first massive dinosaur. Learn their behavior patterns and survival strategies.");
    Beat1.RequiredCompletedBeats.Add(TEXT("Beat3_BiomeDiscovery"));
    Beat1.RequiredSurvivalLevel = 15;
    Beat1.UnlockedMissions.Add(TEXT("Study_DinosaurBehavior"));
    Beat1.UnlockedMissions.Add(TEXT("Survive_DinosaurEncounter"));
    
    Chapter4.StoryBeats.Add(Beat1);
    StorylineChapters.Add(Chapter4);
}

void UQuest_PrehistoricStorylineManager::CreateChapter5_CivilizationDawn()
{
    FQuest_PrehistoricChapter Chapter5;
    Chapter5.ChapterID = TEXT("Chapter5_CivilizationDawn");
    Chapter5.ChapterTitle = TEXT("Dawn of Civilization");
    Chapter5.ChapterDescription = TEXT("Establish the foundations of early human civilization in the prehistoric world.");
    
    // Story Beat 1: Settlement Foundation
    FQuest_StoryBeat Beat1;
    Beat1.BeatID = TEXT("Beat5_SettlementFoundation");
    Beat1.BeatTitle = TEXT("The First Settlement");
    Beat1.BeatDescription = TEXT("Establish a permanent settlement that can house multiple families and withstand dinosaur attacks.");
    Beat1.RequiredCompletedBeats.Add(TEXT("Beat4_FirstDinosaurEncounter"));
    Beat1.RequiredSurvivalLevel = 20;
    Beat1.UnlockedMissions.Add(TEXT("Build_PermanentSettlement"));
    Beat1.UnlockedMissions.Add(TEXT("Establish_DefensiveMeasures"));
    
    Chapter5.StoryBeats.Add(Beat1);
    StorylineChapters.Add(Chapter5);
}

void UQuest_PrehistoricStorylineManager::UpdateStorylineProgress()
{
    // Check if current chapter is completed
    if (CurrentChapterIndex < StorylineChapters.Num())
    {
        FQuest_PrehistoricChapter& CurrentChapter = StorylineChapters[CurrentChapterIndex];
        bool bAllBeatsCompleted = true;
        
        for (const FQuest_StoryBeat& Beat : CurrentChapter.StoryBeats)
        {
            if (!Beat.bIsCompleted)
            {
                bAllBeatsCompleted = false;
                break;
            }
        }
        
        if (bAllBeatsCompleted && !CurrentChapter.bIsCompleted)
        {
            CurrentChapter.bIsCompleted = true;
            UnlockNextChapter();
            UE_LOG(LogTemp, Warning, TEXT("Chapter completed: %s"), *CurrentChapter.ChapterTitle);
        }
    }
}

bool UQuest_PrehistoricStorylineManager::CompleteStoryBeat(const FString& BeatID)
{
    for (FQuest_PrehistoricChapter& Chapter : StorylineChapters)
    {
        for (FQuest_StoryBeat& Beat : Chapter.StoryBeats)
        {
            if (Beat.BeatID == BeatID && !Beat.bIsCompleted)
            {
                if (CheckStoryBeatRequirements(Beat))
                {
                    Beat.bIsCompleted = true;
                    CompletedStoryBeats.AddUnique(BeatID);
                    ProcessStoryBeatCompletion(Beat);
                    UpdateStorylineProgress();
                    
                    UE_LOG(LogTemp, Warning, TEXT("Story beat completed: %s"), *Beat.BeatTitle);
                    return true;
                }
            }
        }
    }
    
    return false;
}

bool UQuest_PrehistoricStorylineManager::CheckStoryBeatRequirements(const FQuest_StoryBeat& StoryBeat) const
{
    // Check if all required beats are completed
    for (const FString& RequiredBeat : StoryBeat.RequiredCompletedBeats)
    {
        if (!CompletedStoryBeats.Contains(RequiredBeat))
        {
            return false;
        }
    }
    
    // Check survival level requirement
    ATranspersonalCharacter* PlayerCharacter = GetPlayerCharacter();
    if (PlayerCharacter)
    {
        // Assuming survival level is based on some character stat
        // This would need to be implemented in TranspersonalCharacter
        return true; // Placeholder for now
    }
    
    return true;
}

void UQuest_PrehistoricStorylineManager::ProcessStoryBeatCompletion(const FQuest_StoryBeat& CompletedBeat)
{
    // Unlock missions associated with this story beat
    for (const FString& MissionID : CompletedBeat.UnlockedMissions)
    {
        UnlockedMissions.AddUnique(MissionID);
        
        // Notify survival mission manager to activate these missions
        if (SurvivalMissionManager)
        {
            // This would trigger the survival mission manager to make these missions available
            UE_LOG(LogTemp, Warning, TEXT("Unlocked mission: %s"), *MissionID);
        }
    }
}

TArray<FQuest_StoryBeat> UQuest_PrehistoricStorylineManager::GetAvailableStoryBeats() const
{
    TArray<FQuest_StoryBeat> AvailableBeats;
    
    for (const FQuest_PrehistoricChapter& Chapter : StorylineChapters)
    {
        if (Chapter.bIsUnlocked)
        {
            for (const FQuest_StoryBeat& Beat : Chapter.StoryBeats)
            {
                if (!Beat.bIsCompleted && CheckStoryBeatRequirements(Beat))
                {
                    AvailableBeats.Add(Beat);
                }
            }
        }
    }
    
    return AvailableBeats;
}

FQuest_PrehistoricChapter UQuest_PrehistoricStorylineManager::GetCurrentChapter() const
{
    if (CurrentChapterIndex < StorylineChapters.Num())
    {
        return StorylineChapters[CurrentChapterIndex];
    }
    
    return FQuest_PrehistoricChapter();
}

bool UQuest_PrehistoricStorylineManager::IsStoryBeatUnlocked(const FString& BeatID) const
{
    for (const FQuest_PrehistoricChapter& Chapter : StorylineChapters)
    {
        if (Chapter.bIsUnlocked)
        {
            for (const FQuest_StoryBeat& Beat : Chapter.StoryBeats)
            {
                if (Beat.BeatID == BeatID)
                {
                    return CheckStoryBeatRequirements(Beat);
                }
            }
        }
    }
    
    return false;
}

void UQuest_PrehistoricStorylineManager::UnlockNextChapter()
{
    int32 NextChapterIndex = CurrentChapterIndex + 1;
    if (NextChapterIndex < StorylineChapters.Num())
    {
        StorylineChapters[NextChapterIndex].bIsUnlocked = true;
        CurrentChapterIndex = NextChapterIndex;
        
        UE_LOG(LogTemp, Warning, TEXT("Unlocked next chapter: %s"), *StorylineChapters[NextChapterIndex].ChapterTitle);
    }
}

float UQuest_PrehistoricStorylineManager::GetOverallStoryProgress() const
{
    if (StorylineChapters.Num() == 0)
    {
        return 0.0f;
    }
    
    int32 TotalBeats = 0;
    int32 CompletedBeats = 0;
    
    for (const FQuest_PrehistoricChapter& Chapter : StorylineChapters)
    {
        for (const FQuest_StoryBeat& Beat : Chapter.StoryBeats)
        {
            TotalBeats++;
            if (Beat.bIsCompleted)
            {
                CompletedBeats++;
            }
        }
    }
    
    return TotalBeats > 0 ? (float)CompletedBeats / (float)TotalBeats : 0.0f;
}

void UQuest_PrehistoricStorylineManager::TriggerStoryEvent(const FString& EventID)
{
    // Handle story events that can trigger story beat completion
    UE_LOG(LogTemp, Warning, TEXT("Story event triggered: %s"), *EventID);
    
    // Example: if player completes certain actions, complete related story beats
    if (EventID == TEXT("PlayerBuiltFirstShelter"))
    {
        CompleteStoryBeat(TEXT("Beat1_FirstShelter"));
    }
    else if (EventID == TEXT("PlayerHuntedFirstAnimal"))
    {
        CompleteStoryBeat(TEXT("Beat1_FirstHunt"));
    }
    else if (EventID == TEXT("PlayerMetOtherSurvivor"))
    {
        CompleteStoryBeat(TEXT("Beat2_FirstContact"));
    }
}

TArray<FString> UQuest_PrehistoricStorylineManager::GetUnlockedMissionsForBeat(const FString& BeatID) const
{
    for (const FQuest_PrehistoricChapter& Chapter : StorylineChapters)
    {
        for (const FQuest_StoryBeat& Beat : Chapter.StoryBeats)
        {
            if (Beat.BeatID == BeatID)
            {
                return Beat.UnlockedMissions;
            }
        }
    }
    
    return TArray<FString>();
}

ATranspersonalCharacter* UQuest_PrehistoricStorylineManager::GetPlayerCharacter() const
{
    if (UWorld* World = GetWorld())
    {
        return Cast<ATranspersonalCharacter>(UGameplayStatics::GetPlayerCharacter(World, 0));
    }
    
    return nullptr;
}