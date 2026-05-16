#include "NarrativeManager.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

void UNarrativeManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    CurrentStoryPhase = TEXT("Arrival");
    DaysSurvived = 0;
    
    InitializeStoryEvents();
    InitializeCharacters();
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeManager initialized"));
}

void UNarrativeManager::InitializeStoryEvents()
{
    // Phase 1: Arrival and First Survival
    FNarr_StoryEvent ArrivalEvent;
    ArrivalEvent.EventID = TEXT("arrival_awakening");
    ArrivalEvent.EventName = TEXT("Awakening in the Valley");
    ArrivalEvent.EventDescription = TEXT("You awaken in a strange prehistoric valley, disoriented and alone.");
    ArrivalEvent.bIsCompleted = false;
    ArrivalEvent.UnlockedEvents.Add(TEXT("first_encounter"));
    ArrivalEvent.AssociatedDialogue = TEXT("SurvivalTutorial");
    StoryEvents.Add(ArrivalEvent);

    FNarr_StoryEvent FirstEncounter;
    FirstEncounter.EventID = TEXT("first_encounter");
    FirstEncounter.EventName = TEXT("First Dinosaur Encounter");
    FirstEncounter.EventDescription = TEXT("Your first sighting of the magnificent and terrifying creatures that rule this land.");
    FirstEncounter.Prerequisites.Add(TEXT("arrival_awakening"));
    FirstEncounter.UnlockedEvents.Add(TEXT("learn_survival"));
    FirstEncounter.bIsCompleted = false;
    StoryEvents.Add(FirstEncounter);

    FNarr_StoryEvent LearnSurvival;
    LearnSurvival.EventID = TEXT("learn_survival");
    LearnSurvival.EventName = TEXT("Learning to Survive");
    LearnSurvival.EventDescription = TEXT("You meet a wise tribal elder who teaches you the basics of survival.");
    LearnSurvival.Prerequisites.Add(TEXT("first_encounter"));
    LearnSurvival.UnlockedEvents.Add(TEXT("first_hunt"));
    LearnSurvival.AssociatedDialogue = TEXT("SurvivalTutorial");
    LearnSurvival.bIsCompleted = false;
    StoryEvents.Add(LearnSurvival);

    // Phase 2: Becoming a Hunter
    FNarr_StoryEvent FirstHunt;
    FirstHunt.EventID = TEXT("first_hunt");
    FirstHunt.EventName = TEXT("The First Hunt");
    FirstHunt.EventDescription = TEXT("You attempt your first hunt for food, learning the ways of the predator.");
    FirstHunt.Prerequisites.Add(TEXT("learn_survival"));
    FirstHunt.UnlockedEvents.Add(TEXT("craft_tools"));
    FirstHunt.AssociatedDialogue = TEXT("HuntingTips");
    FirstHunt.bIsCompleted = false;
    StoryEvents.Add(FirstHunt);

    FNarr_StoryEvent CraftTools;
    CraftTools.EventID = TEXT("craft_tools");
    CraftTools.EventName = TEXT("Crafting Your First Tools");
    CraftTools.EventDescription = TEXT("You learn to craft basic tools and weapons from stone and wood.");
    CraftTools.Prerequisites.Add(TEXT("first_hunt"));
    CraftTools.UnlockedEvents.Add(TEXT("territory_exploration"));
    CraftTools.bIsCompleted = false;
    StoryEvents.Add(CraftTools);

    // Phase 3: Exploration and Territory
    FNarr_StoryEvent TerritoryExploration;
    TerritoryExploration.EventID = TEXT("territory_exploration");
    TerritoryExploration.EventName = TEXT("Exploring New Territories");
    TerritoryExploration.EventDescription = TEXT("You venture into different biomes, each with unique challenges and opportunities.");
    TerritoryExploration.Prerequisites.Add(TEXT("craft_tools"));
    TerritoryExploration.UnlockedEvents.Add(TEXT("apex_predator_encounter"));
    TerritoryExploration.bIsCompleted = false;
    StoryEvents.Add(TerritoryExploration);

    // Phase 4: Apex Predator Challenge
    FNarr_StoryEvent ApexEncounter;
    ApexEncounter.EventID = TEXT("apex_predator_encounter");
    ApexEncounter.EventName = TEXT("Face of the Thunder Lizard");
    ApexEncounter.EventDescription = TEXT("You encounter the mighty T-Rex, the apex predator of this ancient world.");
    ApexEncounter.Prerequisites.Add(TEXT("territory_exploration"));
    ApexEncounter.UnlockedEvents.Add(TEXT("tribal_leadership"));
    ApexEncounter.bIsCompleted = false;
    StoryEvents.Add(ApexEncounter);

    // Phase 5: Leadership and Community
    FNarr_StoryEvent TribalLeadership;
    TribalLeadership.EventID = TEXT("tribal_leadership");
    TribalLeadership.EventName = TEXT("Becoming a Leader");
    TribalLeadership.EventDescription = TEXT("Your survival skills and courage earn you respect among other survivors.");
    TribalLeadership.Prerequisites.Add(TEXT("apex_predator_encounter"));
    TribalLeadership.bIsCompleted = false;
    StoryEvents.Add(TribalLeadership);
}

void UNarrativeManager::InitializeCharacters()
{
    // Tribal Elder - Survival Guide
    FNarr_CharacterProfile TribalElder;
    TribalElder.CharacterID = TEXT("tribal_elder");
    TribalElder.CharacterName = TEXT("Kael the Wise");
    TribalElder.CharacterRole = TEXT("Survival Mentor");
    TribalElder.CharacterBackground = TEXT("An aged survivor who has lived in the valley for decades, possessing vast knowledge of the land and its dangers.");
    TribalElder.AvailableDialogues.Add(TEXT("SurvivalTutorial"));
    TribalElder.SpawnLocation = FVector(0, 0, 100);
    TribalElder.bIsAlive = true;
    Characters.Add(TribalElder);

    // Wise Hunter - Combat and Hunting Guide
    FNarr_CharacterProfile WiseHunter;
    WiseHunter.CharacterID = TEXT("wise_hunter");
    WiseHunter.CharacterName = TEXT("Nara the Tracker");
    WiseHunter.CharacterRole = TEXT("Hunting Instructor");
    WiseHunter.CharacterBackground = TEXT("A skilled hunter who knows the patterns of prey and predator alike.");
    WiseHunter.AvailableDialogues.Add(TEXT("HuntingTips"));
    WiseHunter.SpawnLocation = FVector(-45000, 40000, 100);
    WiseHunter.bIsAlive = true;
    Characters.Add(WiseHunter);

    // Scout - Territory Guide
    FNarr_CharacterProfile Scout;
    Scout.CharacterID = TEXT("territory_scout");
    Scout.CharacterName = TEXT("Jax the Swift");
    Scout.CharacterRole = TEXT("Territory Guide");
    Scout.CharacterBackground = TEXT("A young but experienced scout who knows the safest paths through dangerous territories.");
    Scout.AvailableDialogues.Add(TEXT("SurvivalTutorial"));
    Scout.SpawnLocation = FVector(55000, 0, 100);
    Scout.bIsAlive = true;
    Characters.Add(Scout);

    // Mountain Sage - Advanced Survival
    FNarr_CharacterProfile MountainSage;
    MountainSage.CharacterID = TEXT("mountain_sage");
    MountainSage.CharacterName = TEXT("Thane of the Peaks");
    MountainSage.CharacterRole = TEXT("Advanced Survival Expert");
    MountainSage.CharacterBackground = TEXT("A hermit who lives in the dangerous mountain regions, master of advanced survival techniques.");
    MountainSage.AvailableDialogues.Add(TEXT("HuntingTips"));
    MountainSage.SpawnLocation = FVector(40000, 50000, 100);
    MountainSage.bIsAlive = true;
    Characters.Add(MountainSage);
}

void UNarrativeManager::TriggerStoryEvent(const FString& EventID)
{
    for (FNarr_StoryEvent& Event : StoryEvents)
    {
        if (Event.EventID == EventID && !Event.bIsCompleted)
        {
            if (CheckEventPrerequisites(Event))
            {
                Event.bIsCompleted = true;
                CompletedEvents.Add(EventID);
                
                UE_LOG(LogTemp, Log, TEXT("Story event completed: %s"), *Event.EventName);
                
                // Unlock subsequent events
                for (const FString& UnlockedEventID : Event.UnlockedEvents)
                {
                    UE_LOG(LogTemp, Log, TEXT("Unlocked event: %s"), *UnlockedEventID);
                }
                
                return;
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("Prerequisites not met for event: %s"), *EventID);
            }
        }
    }
}

bool UNarrativeManager::IsEventCompleted(const FString& EventID)
{
    return CompletedEvents.Contains(EventID);
}

TArray<FNarr_StoryEvent> UNarrativeManager::GetAvailableEvents()
{
    TArray<FNarr_StoryEvent> AvailableEvents;
    
    for (const FNarr_StoryEvent& Event : StoryEvents)
    {
        if (!Event.bIsCompleted && CheckEventPrerequisites(Event))
        {
            AvailableEvents.Add(Event);
        }
    }
    
    return AvailableEvents;
}

void UNarrativeManager::AdvanceStoryPhase(const FString& NewPhase)
{
    CurrentStoryPhase = NewPhase;
    UE_LOG(LogTemp, Log, TEXT("Story phase advanced to: %s"), *NewPhase);
}

FNarr_CharacterProfile UNarrativeManager::GetCharacterProfile(const FString& CharacterID)
{
    for (const FNarr_CharacterProfile& Character : Characters)
    {
        if (Character.CharacterID == CharacterID)
        {
            return Character;
        }
    }
    
    return FNarr_CharacterProfile();
}

void UNarrativeManager::RegisterCharacter(const FNarr_CharacterProfile& NewCharacter)
{
    Characters.Add(NewCharacter);
    UE_LOG(LogTemp, Log, TEXT("Registered character: %s"), *NewCharacter.CharacterName);
}

TArray<FString> UNarrativeManager::GetCharacterDialogues(const FString& CharacterID)
{
    FNarr_CharacterProfile Character = GetCharacterProfile(CharacterID);
    return Character.AvailableDialogues;
}

void UNarrativeManager::IncrementDaysSurvived()
{
    DaysSurvived++;
    UE_LOG(LogTemp, Log, TEXT("Days survived: %d"), DaysSurvived);
    
    // Trigger story events based on survival milestones
    if (DaysSurvived == 1 && !IsEventCompleted(TEXT("arrival_awakening")))
    {
        TriggerStoryEvent(TEXT("arrival_awakening"));
    }
    else if (DaysSurvived == 3 && !IsEventCompleted(TEXT("first_encounter")))
    {
        TriggerStoryEvent(TEXT("first_encounter"));
    }
    else if (DaysSurvived == 7 && !IsEventCompleted(TEXT("learn_survival")))
    {
        TriggerStoryEvent(TEXT("learn_survival"));
    }
}

void UNarrativeManager::SaveNarrativeProgress()
{
    // Implementation for saving narrative progress
    UE_LOG(LogTemp, Log, TEXT("Saving narrative progress"));
}

void UNarrativeManager::LoadNarrativeProgress()
{
    // Implementation for loading narrative progress
    UE_LOG(LogTemp, Log, TEXT("Loading narrative progress"));
}

bool UNarrativeManager::CheckEventPrerequisites(const FNarr_StoryEvent& Event)
{
    for (const FString& Prerequisite : Event.Prerequisites)
    {
        if (!IsEventCompleted(Prerequisite))
        {
            return false;
        }
    }
    return true;
}