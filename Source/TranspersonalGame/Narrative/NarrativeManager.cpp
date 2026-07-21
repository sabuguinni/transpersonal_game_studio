#include "NarrativeManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

void UNarrativeManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    LoadStoryData();
    InitializeMainStoryBeats();
    UE_LOG(LogTemp, Log, TEXT("NarrativeManager initialized"));
}

void UNarrativeManager::Deinitialize()
{
    StoryBeats.Empty();
    DialogueDatabase.Empty();
    StoryFlags.Empty();
    Super::Deinitialize();
}

void UNarrativeManager::StartStoryBeat(const FString& BeatID)
{
    for (FNarr_StoryBeat& Beat : StoryBeats)
    {
        if (Beat.BeatID == BeatID)
        {
            // Check if all required conditions are met
            bool bCanStart = true;
            for (const FString& Condition : Beat.RequiredConditions)
            {
                if (!GetStoryFlag(Condition))
                {
                    bCanStart = false;
                    break;
                }
            }

            if (bCanStart && !Beat.bIsCompleted)
            {
                Beat.bIsActive = true;
                UE_LOG(LogTemp, Log, TEXT("Started story beat: %s"), *BeatID);
            }
            break;
        }
    }
}

void UNarrativeManager::CompleteStoryBeat(const FString& BeatID)
{
    for (FNarr_StoryBeat& Beat : StoryBeats)
    {
        if (Beat.BeatID == BeatID && Beat.bIsActive)
        {
            Beat.bIsActive = false;
            Beat.bIsCompleted = true;

            // Set completion flags
            for (const FString& Flag : Beat.CompletionFlags)
            {
                SetStoryFlag(Flag, true);
            }

            UE_LOG(LogTemp, Log, TEXT("Completed story beat: %s"), *BeatID);
            break;
        }
    }
}

bool UNarrativeManager::IsStoryBeatActive(const FString& BeatID) const
{
    for (const FNarr_StoryBeat& Beat : StoryBeats)
    {
        if (Beat.BeatID == BeatID)
        {
            return Beat.bIsActive;
        }
    }
    return false;
}

bool UNarrativeManager::IsStoryBeatCompleted(const FString& BeatID) const
{
    for (const FNarr_StoryBeat& Beat : StoryBeats)
    {
        if (Beat.BeatID == BeatID)
        {
            return Beat.bIsCompleted;
        }
    }
    return false;
}

void UNarrativeManager::TriggerDialogue(const FString& DialogueID, AActor* Speaker)
{
    if (DialogueDatabase.Contains(DialogueID))
    {
        const FNarr_DialogueLine& Line = DialogueDatabase[DialogueID];
        UE_LOG(LogTemp, Log, TEXT("Dialogue triggered: %s says: %s"), *Line.SpeakerName, *Line.DialogueText);
        
        // Here you would integrate with UI system to display dialogue
        // For now, just log the dialogue
    }
}

void UNarrativeManager::SetStoryFlag(const FString& FlagName, bool bValue)
{
    StoryFlags.Add(FlagName, bValue);
    UE_LOG(LogTemp, Log, TEXT("Story flag set: %s = %s"), *FlagName, bValue ? TEXT("true") : TEXT("false"));
}

bool UNarrativeManager::GetStoryFlag(const FString& FlagName) const
{
    if (StoryFlags.Contains(FlagName))
    {
        return StoryFlags[FlagName];
    }
    return false;
}

TArray<FNarr_StoryBeat> UNarrativeManager::GetActiveStoryBeats() const
{
    TArray<FNarr_StoryBeat> ActiveBeats;
    for (const FNarr_StoryBeat& Beat : StoryBeats)
    {
        if (Beat.bIsActive)
        {
            ActiveBeats.Add(Beat);
        }
    }
    return ActiveBeats;
}

void UNarrativeManager::LoadStoryData()
{
    // Initialize dialogue database with survival-focused lines
    FNarr_DialogueLine HunterGreeting;
    HunterGreeting.SpeakerName = TEXT("Tribal Hunter");
    HunterGreeting.DialogueText = TEXT("The beasts grow restless. We must hunt before they hunt us.");
    HunterGreeting.Duration = 4.0f;
    DialogueDatabase.Add(TEXT("hunter_greeting"), HunterGreeting);

    FNarr_DialogueLine CrafterAdvice;
    CrafterAdvice.SpeakerName = TEXT("Tool Maker");
    CrafterAdvice.DialogueText = TEXT("Sharp stone cuts deep, but fire hardens the edge. Remember this.");
    CrafterAdvice.Duration = 5.0f;
    DialogueDatabase.Add(TEXT("crafter_advice"), CrafterAdvice);

    FNarr_DialogueLine ScoutWarning;
    ScoutWarning.SpeakerName = TEXT("Valley Scout");
    ScoutWarning.DialogueText = TEXT("Massive tracks by the river. Something big passed through here.");
    ScoutWarning.Duration = 4.5f;
    DialogueDatabase.Add(TEXT("scout_warning"), ScoutWarning);
}

void UNarrativeManager::InitializeMainStoryBeats()
{
    // First Hunt - Tutorial beat
    FNarr_StoryBeat FirstHunt;
    FirstHunt.BeatID = TEXT("first_hunt");
    FirstHunt.BeatTitle = TEXT("The First Hunt");
    FirstHunt.BeatDescription = TEXT("Learn to track and hunt small prey to survive");
    FirstHunt.RequiredConditions.Add(TEXT("game_started"));
    FirstHunt.CompletionFlags.Add(TEXT("hunting_learned"));
    FirstHunt.bIsActive = false;
    FirstHunt.bIsCompleted = false;
    StoryBeats.Add(FirstHunt);

    // Valley Discovery
    FNarr_StoryBeat ValleyDiscovery;
    ValleyDiscovery.BeatID = TEXT("valley_discovery");
    ValleyDiscovery.BeatTitle = TEXT("The Hidden Valley");
    ValleyDiscovery.BeatDescription = TEXT("Discover the valley where great beasts roam");
    ValleyDiscovery.RequiredConditions.Add(TEXT("hunting_learned"));
    ValleyDiscovery.CompletionFlags.Add(TEXT("valley_found"));
    ValleyDiscovery.bIsActive = false;
    ValleyDiscovery.bIsCompleted = false;
    StoryBeats.Add(ValleyDiscovery);

    // Tribe Formation
    FNarr_StoryBeat TribeFormation;
    TribeFormation.BeatID = TEXT("tribe_formation");
    TribeFormation.BeatTitle = TEXT("Gathering the Tribe");
    TribeFormation.BeatDescription = TEXT("Unite scattered survivors into a hunting party");
    TribeFormation.RequiredConditions.Add(TEXT("valley_found"));
    TribeFormation.CompletionFlags.Add(TEXT("tribe_formed"));
    TribeFormation.bIsActive = false;
    TribeFormation.bIsCompleted = false;
    StoryBeats.Add(TribeFormation);

    // Initialize starting flags
    SetStoryFlag(TEXT("game_started"), true);
}