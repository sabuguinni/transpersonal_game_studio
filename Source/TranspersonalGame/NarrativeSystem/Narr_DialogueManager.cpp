#include "Narr_DialogueManager.h"
#include "Engine/Engine.h"

UNarr_DialogueManager::UNarr_DialogueManager()
{
    CurrentDialogueCharacter = TEXT("");
    CurrentContext = ENarr_DialogueContext::Information;
}

void UNarr_DialogueManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Narr_DialogueManager: Initializing narrative dialogue system"));
    
    // Initialize default dialogues and story beats
    InitializeDefaultDialogues();
    CreateSurvivalStoryBeats();
    
    UE_LOG(LogTemp, Warning, TEXT("Narr_DialogueManager: Initialized with %d dialogue lines and %d story beats"), 
           DialogueDatabase.Num(), StoryBeats.Num());
}

void UNarr_DialogueManager::Deinitialize()
{
    DialogueDatabase.Empty();
    StoryBeats.Empty();
    NarrativeContext.Empty();
    
    Super::Deinitialize();
}

void UNarr_DialogueManager::StartDialogue(const FString& CharacterName, ENarr_CharacterType CharacterType, ENarr_DialogueContext Context)
{
    CurrentDialogueCharacter = CharacterName;
    CurrentContext = Context;
    
    FNarr_DialogueLine DialogueLine = GetDialogueLine(CharacterName, Context);
    
    if (!DialogueLine.DialogueText.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("Narr_DialogueManager: Starting dialogue with %s: %s"), 
               *CharacterName, *DialogueLine.DialogueText);
        
        // Set narrative context for this interaction
        SetNarrativeContext(TEXT("LastSpeaker"), CharacterName);
        SetNarrativeContext(TEXT("LastContext"), UEnum::GetValueAsString(Context));
    }
}

FNarr_DialogueLine UNarr_DialogueManager::GetDialogueLine(const FString& CharacterName, ENarr_DialogueContext Context)
{
    // Find dialogue line matching character and context
    for (const FNarr_DialogueLine& Dialogue : DialogueDatabase)
    {
        if (Dialogue.SpeakerName == CharacterName && Dialogue.Context == Context)
        {
            return Dialogue;
        }
    }
    
    // Find any dialogue line matching context for this character type
    for (const FNarr_DialogueLine& Dialogue : DialogueDatabase)
    {
        if (Dialogue.Context == Context)
        {
            return Dialogue;
        }
    }
    
    // Return empty dialogue if nothing found
    return FNarr_DialogueLine();
}

void UNarr_DialogueManager::AddDialogueLine(const FNarr_DialogueLine& NewDialogue)
{
    DialogueDatabase.Add(NewDialogue);
    UE_LOG(LogTemp, Warning, TEXT("Narr_DialogueManager: Added dialogue line for %s"), *NewDialogue.SpeakerName);
}

TArray<FNarr_DialogueLine> UNarr_DialogueManager::GetDialoguesByCharacterType(ENarr_CharacterType CharacterType)
{
    TArray<FNarr_DialogueLine> FilteredDialogues;
    
    for (const FNarr_DialogueLine& Dialogue : DialogueDatabase)
    {
        if (Dialogue.CharacterType == CharacterType)
        {
            FilteredDialogues.Add(Dialogue);
        }
    }
    
    return FilteredDialogues;
}

void UNarr_DialogueManager::TriggerStoryBeat(const FString& BeatID)
{
    for (FNarr_StoryBeat& Beat : StoryBeats)
    {
        if (Beat.BeatID == BeatID)
        {
            UE_LOG(LogTemp, Warning, TEXT("Narr_DialogueManager: Triggered story beat: %s - %s"), 
                   *Beat.BeatID, *Beat.Title);
            
            SetNarrativeContext(TEXT("CurrentStoryBeat"), BeatID);
            SetNarrativeContext(TEXT("StoryBeatTriggered"), TEXT("true"));
            break;
        }
    }
}

void UNarr_DialogueManager::CompleteStoryBeat(const FString& BeatID)
{
    for (FNarr_StoryBeat& Beat : StoryBeats)
    {
        if (Beat.BeatID == BeatID)
        {
            Beat.bIsCompleted = true;
            UE_LOG(LogTemp, Warning, TEXT("Narr_DialogueManager: Completed story beat: %s"), *Beat.Title);
            
            SetNarrativeContext(TEXT("LastCompletedBeat"), BeatID);
            break;
        }
    }
}

bool UNarr_DialogueManager::IsStoryBeatCompleted(const FString& BeatID)
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

TArray<FNarr_StoryBeat> UNarr_DialogueManager::GetAvailableStoryBeats()
{
    TArray<FNarr_StoryBeat> AvailableBeats;
    
    for (const FNarr_StoryBeat& Beat : StoryBeats)
    {
        if (!Beat.bIsCompleted)
        {
            // Check if all required quests are completed
            bool bCanTrigger = true;
            for (const FString& RequiredQuest : Beat.RequiredQuests)
            {
                // This would integrate with the quest system
                // For now, assume all requirements are met
            }
            
            if (bCanTrigger)
            {
                AvailableBeats.Add(Beat);
            }
        }
    }
    
    return AvailableBeats;
}

void UNarr_DialogueManager::SetNarrativeContext(const FString& ContextKey, const FString& ContextValue)
{
    NarrativeContext.Add(ContextKey, ContextValue);
}

FString UNarr_DialogueManager::GetNarrativeContext(const FString& ContextKey)
{
    if (NarrativeContext.Contains(ContextKey))
    {
        return NarrativeContext[ContextKey];
    }
    return TEXT("");
}

void UNarr_DialogueManager::InitializeDefaultDialogues()
{
    CreateTribalElderDialogues();
    CreateTribalScoutDialogues();
    CreateTribalShamanDialogues();
    CreateTribalGuideDialogues();
}

void UNarr_DialogueManager::CreateTribalElderDialogues()
{
    FNarr_DialogueLine ElderWelcome;
    ElderWelcome.SpeakerName = TEXT("Tribal Elder");
    ElderWelcome.CharacterType = ENarr_CharacterType::TribalElder;
    ElderWelcome.Context = ENarr_DialogueContext::FirstMeeting;
    ElderWelcome.DialogueText = TEXT("Welcome, young one. I have seen many seasons pass in these dangerous lands. Listen well to my words if you wish to survive the trials ahead.");
    ElderWelcome.Duration = 8.0f;
    ElderWelcome.PlayerResponses.Add(TEXT("I will listen, Elder."));
    ElderWelcome.PlayerResponses.Add(TEXT("What dangers should I know about?"));
    DialogueDatabase.Add(ElderWelcome);

    FNarr_DialogueLine ElderWisdom;
    ElderWisdom.SpeakerName = TEXT("Tribal Elder");
    ElderWisdom.CharacterType = ENarr_CharacterType::TribalElder;
    ElderWisdom.Context = ENarr_DialogueContext::Information;
    ElderWisdom.DialogueText = TEXT("The great herds move with the changing seasons. When the Thunder Lizards migrate north, it means the rains will come soon. Watch for their massive footprints in the mud.");
    ElderWisdom.Duration = 12.0f;
    DialogueDatabase.Add(ElderWisdom);

    FNarr_DialogueLine ElderQuest;
    ElderQuest.SpeakerName = TEXT("Tribal Elder");
    ElderQuest.CharacterType = ENarr_CharacterType::TribalElder;
    ElderQuest.Context = ENarr_DialogueContext::QuestGiving;
    ElderQuest.DialogueText = TEXT("Prove your worth to our tribe. Survive alone in the wilderness for ten sunrises. Return to us, and you will be welcomed as one of our own.");
    ElderQuest.Duration = 10.0f;
    DialogueDatabase.Add(ElderQuest);
}

void UNarr_DialogueManager::CreateTribalScoutDialogues()
{
    FNarr_DialogueLine ScoutWarning;
    ScoutWarning.SpeakerName = TEXT("Tribal Scout");
    ScoutWarning.CharacterType = ENarr_CharacterType::TribalScout;
    ScoutWarning.Context = ENarr_DialogueContext::Warning;
    ScoutWarning.DialogueText = TEXT("Pack hunters spotted near the eastern cliffs! Three, maybe four raptors moving in formation. They hunt at dusk when the shadows grow long. Stay close to the fire tonight.");
    ScoutWarning.Duration = 11.0f;
    DialogueDatabase.Add(ScoutWarning);

    FNarr_DialogueLine ScoutInfo;
    ScoutInfo.SpeakerName = TEXT("Tribal Scout");
    ScoutInfo.CharacterType = ENarr_CharacterType::TribalScout;
    ScoutInfo.Context = ENarr_DialogueContext::Information;
    ScoutInfo.DialogueText = TEXT("I know these lands like the back of my hand. The safe paths, the hidden water sources, the places where death lurks in the shadows. Knowledge is survival out here.");
    ScoutInfo.Duration = 9.0f;
    DialogueDatabase.Add(ScoutInfo);
}

void UNarr_DialogueManager::CreateTribalShamanDialogues()
{
    FNarr_DialogueLine ShamanLore;
    ShamanLore.SpeakerName = TEXT("Tribal Shaman");
    ShamanLore.CharacterType = ENarr_CharacterType::TribalShaman;
    ShamanLore.Context = ENarr_DialogueContext::Information;
    ShamanLore.DialogueText = TEXT("The bones tell a story of survival. This skeleton belonged to a young hunter who ventured too far from the tribe. Learn from their mistake - the wilderness shows no mercy to the unprepared.");
    ShamanLore.Duration = 13.0f;
    DialogueDatabase.Add(ShamanLore);

    FNarr_DialogueLine ShamanHealing;
    ShamanHealing.SpeakerName = TEXT("Tribal Shaman");
    ShamanHealing.CharacterType = ENarr_CharacterType::TribalShaman;
    ShamanLore.Context = ENarr_DialogueContext::Trading;
    ShamanHealing.DialogueText = TEXT("These healing herbs will mend your wounds and restore your strength. But use them wisely - they grow only in the most dangerous places, where the great beasts roam.");
    ShamanHealing.Duration = 10.0f;
    DialogueDatabase.Add(ShamanHealing);
}

void UNarr_DialogueManager::CreateTribalGuideDialogues()
{
    FNarr_DialogueLine GuideWater;
    GuideWater.SpeakerName = TEXT("Tribal Guide");
    GuideWater.CharacterType = ENarr_CharacterType::TribalGuide;
    GuideWater.Context = ENarr_DialogueContext::Information;
    GuideWater.DialogueText = TEXT("Fresh water flows from the northern springs, but beware the giant crocodiles that lurk beneath the surface. Fill your waterskins quickly and quietly.");
    GuideWater.Duration = 9.0f;
    DialogueDatabase.Add(GuideWater);

    FNarr_DialogueLine GuidePath;
    GuidePath.SpeakerName = TEXT("Tribal Guide");
    GuidePath.CharacterType = ENarr_CharacterType::TribalGuide;
    GuidePath.Context = ENarr_DialogueContext::QuestProgress;
    GuidePath.DialogueText = TEXT("Follow the river south until you reach the great fallen tree. From there, head east toward the smoking mountain. That path will lead you to the hunting grounds.");
    GuidePath.Duration = 11.0f;
    DialogueDatabase.Add(GuidePath);
}

void UNarr_DialogueManager::CreateSurvivalStoryBeats()
{
    FNarr_StoryBeat IntroductionBeat;
    IntroductionBeat.BeatID = TEXT("introduction");
    IntroductionBeat.Title = TEXT("Arrival in the Prehistoric World");
    IntroductionBeat.Description = TEXT("The player awakens in a dangerous prehistoric world and must learn to survive among the dinosaurs.");
    IntroductionBeat.bIsCompleted = false;
    
    FNarr_DialogueLine IntroDialogue;
    IntroDialogue.SpeakerName = TEXT("Narrator");
    IntroDialogue.CharacterType = ENarr_CharacterType::Survivor;
    IntroDialogue.Context = ENarr_DialogueContext::Information;
    IntroDialogue.DialogueText = TEXT("You awaken on unfamiliar ground. The air is thick and humid, filled with sounds you've never heard before. In the distance, massive shapes move through the mist. This is not your world.");
    IntroDialogue.Duration = 15.0f;
    IntroductionBeat.DialogueLines.Add(IntroDialogue);
    
    StoryBeats.Add(IntroductionBeat);

    FNarr_StoryBeat FirstEncounterBeat;
    FirstEncounterBeat.BeatID = TEXT("first_encounter");
    FirstEncounterBeat.Title = TEXT("First Dinosaur Encounter");
    FirstEncounterBeat.Description = TEXT("The player encounters their first dinosaur and learns about the dangers of this world.");
    FirstEncounterBeat.RequiredQuests.Add(TEXT("survival_10min"));
    FirstEncounterBeat.bIsCompleted = false;
    StoryBeats.Add(FirstEncounterBeat);

    FNarr_StoryBeat TribalContactBeat;
    TribalContactBeat.BeatID = TEXT("tribal_contact");
    TribalContactBeat.Title = TEXT("Meeting the Tribe");
    TribalContactBeat.Description = TEXT("The player makes contact with a primitive human tribe and begins to learn their ways.");
    TribalContactBeat.RequiredQuests.Add(TEXT("hunt_raptor"));
    TribalContactBeat.bIsCompleted = false;
    StoryBeats.Add(TribalContactBeat);
}