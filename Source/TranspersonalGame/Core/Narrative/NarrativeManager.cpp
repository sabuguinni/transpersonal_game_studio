#include "NarrativeManager.h"
#include "Engine/Engine.h"
#include "Engine/DataTable.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"

UNarrativeManager::UNarrativeManager()
{
    CurrentDialogueState = ENarr_DialogueState::Idle;
    DefaultDialogueSpeed = 1.0f;
    bAutoAdvanceDialogue = false;
}

void UNarrativeManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    LoadDialogueData();
    InitializeStoryBeats();
    InitializeCharacters();
    
    UE_LOG(LogTemp, Warning, TEXT("NarrativeManager initialized successfully"));
}

void UNarrativeManager::Deinitialize()
{
    EndDialogue();
    Super::Deinitialize();
}

bool UNarrativeManager::StartDialogue(const FString& DialogueID, AActor* Speaker)
{
    if (CurrentDialogueState == ENarr_DialogueState::Playing)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot start dialogue %s - another dialogue is already playing"), *DialogueID);
        return false;
    }

    // Find dialogue entry (in real implementation, this would query the data table)
    FNarr_DialogueEntry DialogueEntry;
    DialogueEntry.CharacterName = TEXT("Unknown Speaker");
    DialogueEntry.DialogueText = FString::Printf(TEXT("Dialogue: %s"), *DialogueID);
    DialogueEntry.EmotionalIntensity = 0.5f;

    // Check conditions
    if (!CheckPrerequisites(DialogueEntry.RequiredConditions))
    {
        UE_LOG(LogTemp, Warning, TEXT("Dialogue %s prerequisites not met"), *DialogueID);
        return false;
    }

    CurrentDialogue = DialogueEntry;
    CurrentDialogueState = ENarr_DialogueState::Playing;

    // Play audio if available
    if (!DialogueEntry.AudioAssetPath.IsEmpty())
    {
        PlayDialogueAudio(DialogueEntry.AudioAssetPath);
    }

    // Execute triggered events
    ExecuteEvents(DialogueEntry.TriggeredEvents);

    UE_LOG(LogTemp, Log, TEXT("Started dialogue: %s by %s"), *DialogueEntry.DialogueText, *DialogueEntry.CharacterName);
    return true;
}

void UNarrativeManager::EndDialogue()
{
    if (CurrentDialogueState != ENarr_DialogueState::Idle)
    {
        StopDialogueAudio();
        CurrentDialogueState = ENarr_DialogueState::Completed;
        
        // Reset after a frame
        GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
        {
            CurrentDialogueState = ENarr_DialogueState::Idle;
            CurrentDialogue = FNarr_DialogueEntry();
        });
    }
}

bool UNarrativeManager::IsDialogueActive() const
{
    return CurrentDialogueState == ENarr_DialogueState::Playing || 
           CurrentDialogueState == ENarr_DialogueState::WaitingForInput;
}

FNarr_DialogueEntry UNarrativeManager::GetCurrentDialogue() const
{
    return CurrentDialogue;
}

void UNarrativeManager::TriggerStoryBeat(const FString& BeatID)
{
    if (FNarr_StoryBeat* Beat = StoryBeats.Find(BeatID))
    {
        if (Beat->bIsCompleted)
        {
            UE_LOG(LogTemp, Warning, TEXT("Story beat %s already completed"), *BeatID);
            return;
        }

        if (!CheckPrerequisites(Beat->Prerequisites))
        {
            UE_LOG(LogTemp, Warning, TEXT("Story beat %s prerequisites not met"), *BeatID);
            return;
        }

        Beat->bIsCompleted = true;
        
        // Trigger associated dialogues
        for (const FString& DialogueID : Beat->DialogueIDs)
        {
            StartDialogue(DialogueID);
        }

        UE_LOG(LogTemp, Log, TEXT("Triggered story beat: %s - %s"), *Beat->BeatTitle, *Beat->BeatDescription);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Story beat %s not found"), *BeatID);
    }
}

bool UNarrativeManager::IsStoryBeatCompleted(const FString& BeatID) const
{
    if (const FNarr_StoryBeat* Beat = StoryBeats.Find(BeatID))
    {
        return Beat->bIsCompleted;
    }
    return false;
}

TArray<FNarr_StoryBeat> UNarrativeManager::GetAvailableStoryBeats() const
{
    TArray<FNarr_StoryBeat> AvailableBeats;
    
    for (const auto& BeatPair : StoryBeats)
    {
        const FNarr_StoryBeat& Beat = BeatPair.Value;
        if (!Beat.bIsCompleted && CheckPrerequisites(Beat.Prerequisites))
        {
            AvailableBeats.Add(Beat);
        }
    }
    
    // Sort by priority
    AvailableBeats.Sort([](const FNarr_StoryBeat& A, const FNarr_StoryBeat& B)
    {
        return A.Priority > B.Priority;
    });
    
    return AvailableBeats;
}

void UNarrativeManager::RegisterCharacter(const FNarr_CharacterProfile& Character)
{
    Characters.Add(Character.CharacterID, Character);
    UE_LOG(LogTemp, Log, TEXT("Registered character: %s (%s)"), *Character.DisplayName, *Character.CharacterID);
}

FNarr_CharacterProfile UNarrativeManager::GetCharacterProfile(const FString& CharacterID) const
{
    if (const FNarr_CharacterProfile* Profile = Characters.Find(CharacterID))
    {
        return *Profile;
    }
    return FNarr_CharacterProfile();
}

void UNarrativeManager::UpdateCharacterTrust(const FString& CharacterID, float TrustDelta)
{
    if (FNarr_CharacterProfile* Profile = Characters.Find(CharacterID))
    {
        Profile->TrustLevel = FMath::Clamp(Profile->TrustLevel + TrustDelta, 0.0f, 1.0f);
        UE_LOG(LogTemp, Log, TEXT("Updated trust for %s: %f"), *CharacterID, Profile->TrustLevel);
    }
}

bool UNarrativeManager::EvaluateCondition(const FString& Condition) const
{
    // Simple condition evaluation - in real implementation this would be more complex
    if (Condition.StartsWith(TEXT("flag:")))
    {
        FString FlagName = Condition.Mid(5);
        return GetGameFlag(FlagName);
    }
    else if (Condition.StartsWith(TEXT("trust:")))
    {
        TArray<FString> Parts;
        Condition.ParseIntoArray(Parts, TEXT(":"));
        if (Parts.Num() >= 3)
        {
            FString CharacterID = Parts[1];
            float RequiredTrust = FCString::Atof(*Parts[2]);
            const FNarr_CharacterProfile* Profile = Characters.Find(CharacterID);
            return Profile && Profile->TrustLevel >= RequiredTrust;
        }
    }
    else if (Condition.StartsWith(TEXT("beat:")))
    {
        FString BeatID = Condition.Mid(5);
        return IsStoryBeatCompleted(BeatID);
    }
    
    return true; // Default to true for unknown conditions
}

void UNarrativeManager::SetGameFlag(const FString& FlagName, bool bValue)
{
    GameFlags.Add(FlagName, bValue);
    UE_LOG(LogTemp, Log, TEXT("Set game flag %s to %s"), *FlagName, bValue ? TEXT("true") : TEXT("false"));
}

bool UNarrativeManager::GetGameFlag(const FString& FlagName) const
{
    if (const bool* Flag = GameFlags.Find(FlagName))
    {
        return *Flag;
    }
    return false;
}

void UNarrativeManager::PlayDialogueAudio(const FString& AudioPath)
{
    if (AudioPath.IsEmpty())
    {
        return;
    }

    // In a real implementation, this would load and play the audio asset
    UE_LOG(LogTemp, Log, TEXT("Playing dialogue audio: %s"), *AudioPath);
    
    // For now, just log that we would play audio
    // UGameplayStatics::PlaySound2D(GetWorld(), AudioAsset);
}

void UNarrativeManager::StopDialogueAudio()
{
    // In a real implementation, this would stop the currently playing audio
    UE_LOG(LogTemp, Log, TEXT("Stopping dialogue audio"));
}

void UNarrativeManager::LoadDialogueData()
{
    // In a real implementation, this would load from a data table
    UE_LOG(LogTemp, Log, TEXT("Loading dialogue data..."));
}

void UNarrativeManager::InitializeStoryBeats()
{
    // Initialize core story beats for prehistoric survival game
    FNarr_StoryBeat IntroductionBeat;
    IntroductionBeat.BeatID = TEXT("intro_awakening");
    IntroductionBeat.BeatTitle = TEXT("The Awakening");
    IntroductionBeat.BeatDescription = TEXT("Player awakens alone in the prehistoric wilderness");
    IntroductionBeat.Priority = 100;
    IntroductionBeat.DialogueIDs.Add(TEXT("intro_narration"));
    StoryBeats.Add(IntroductionBeat.BeatID, IntroductionBeat);

    FNarr_StoryBeat FirstHuntBeat;
    FirstHuntBeat.BeatID = TEXT("first_hunt");
    FirstHuntBeat.BeatTitle = TEXT("First Hunt");
    FirstHuntBeat.BeatDescription = TEXT("Player encounters their first predator");
    FirstHuntBeat.Priority = 90;
    FirstHuntBeat.Prerequisites.Add(TEXT("flag:has_weapon"));
    FirstHuntBeat.DialogueIDs.Add(TEXT("hunter_warning"));
    StoryBeats.Add(FirstHuntBeat.BeatID, FirstHuntBeat);

    FNarr_StoryBeat TribeEncounterBeat;
    TribeEncounterBeat.BeatID = TEXT("tribe_encounter");
    TribeEncounterBeat.BeatTitle = TEXT("Tribe Encounter");
    TribeEncounterBeat.BeatDescription = TEXT("Player meets other survivors");
    TribeEncounterBeat.Priority = 80;
    TribeEncounterBeat.Prerequisites.Add(TEXT("beat:first_hunt"));
    TribeEncounterBeat.DialogueIDs.Add(TEXT("elder_greeting"));
    StoryBeats.Add(TribeEncounterBeat.BeatID, TribeEncounterBeat);

    UE_LOG(LogTemp, Log, TEXT("Initialized %d story beats"), StoryBeats.Num());
}

void UNarrativeManager::InitializeCharacters()
{
    // Initialize key characters for the prehistoric survival narrative
    FNarr_CharacterProfile ElderHunter;
    ElderHunter.CharacterID = TEXT("elder_hunter");
    ElderHunter.DisplayName = TEXT("Kael the Tracker");
    ElderHunter.BackgroundStory = TEXT("A weathered hunter who has survived countless encounters with apex predators. His knowledge of animal behavior is unmatched.");
    ElderHunter.PersonalityTraits.Add(TEXT("Cautious"));
    ElderHunter.PersonalityTraits.Add(TEXT("Wise"));
    ElderHunter.PersonalityTraits.Add(TEXT("Protective"));
    ElderHunter.VoiceProfile = TEXT("Injured_Hunter");
    ElderHunter.TrustLevel = 0.3f;
    ElderHunter.KnownInformation.Add(TEXT("Predator migration patterns"));
    ElderHunter.KnownInformation.Add(TEXT("Safe hunting grounds"));
    RegisterCharacter(ElderHunter);

    FNarr_CharacterProfile TribalElder;
    TribalElder.CharacterID = TEXT("tribal_elder");
    TribalElder.DisplayName = TEXT("Nara the Wise");
    TribalElder.BackgroundStory = TEXT("The eldest member of a small surviving tribe. She remembers the old ways and guides the group's decisions.");
    TribalElder.PersonalityTraits.Add(TEXT("Wise"));
    TribalElder.PersonalityTraits.Add(TEXT("Authoritative"));
    TribalElder.PersonalityTraits.Add(TEXT("Traditional"));
    TribalElder.VoiceProfile = TEXT("Tribal_Elder");
    TribalElder.TrustLevel = 0.5f;
    TribalElder.KnownInformation.Add(TEXT("Tribal history"));
    TribalElder.KnownInformation.Add(TEXT("Seasonal patterns"));
    RegisterCharacter(TribalElder);

    FNarr_CharacterProfile YoungScout;
    YoungScout.CharacterID = TEXT("young_scout");
    YoungScout.DisplayName = TEXT("Zara the Swift");
    YoungScout.BackgroundStory = TEXT("A young and agile scout who serves as the tribe's eyes and ears. Quick to spot danger but sometimes reckless.");
    YoungScout.PersonalityTraits.Add(TEXT("Energetic"));
    YoungScout.PersonalityTraits.Add(TEXT("Brave"));
    YoungScout.PersonalityTraits.Add(TEXT("Impulsive"));
    YoungScout.VoiceProfile = TEXT("Valley_Lookout");
    YoungScout.TrustLevel = 0.7f;
    YoungScout.KnownInformation.Add(TEXT("Current threats"));
    YoungScout.KnownInformation.Add(TEXT("Escape routes"));
    RegisterCharacter(YoungScout);

    FNarr_CharacterProfile Apprentice;
    Apprentice.CharacterID = TEXT("apprentice");
    Apprentice.DisplayName = TEXT("Ren the Learner");
    Apprentice.BackgroundStory = TEXT("A young apprentice learning the ways of survival. Eager to prove themselves but still inexperienced.");
    Apprentice.PersonalityTraits.Add(TEXT("Curious"));
    Apprentice.PersonalityTraits.Add(TEXT("Determined"));
    Apprentice.PersonalityTraits.Add(TEXT("Inexperienced"));
    Apprentice.VoiceProfile = TEXT("Apprentice_Tracker");
    Apprentice.TrustLevel = 0.8f;
    Apprentice.KnownInformation.Add(TEXT("Basic tracking"));
    RegisterCharacter(Apprentice);

    UE_LOG(LogTemp, Log, TEXT("Initialized %d character profiles"), Characters.Num());
}

bool UNarrativeManager::CheckPrerequisites(const TArray<FString>& Prerequisites) const
{
    for (const FString& Prerequisite : Prerequisites)
    {
        if (!EvaluateCondition(Prerequisite))
        {
            return false;
        }
    }
    return true;
}

void UNarrativeManager::ExecuteEvents(const TArray<FString>& Events)
{
    for (const FString& Event : Events)
    {
        if (Event.StartsWith(TEXT("set_flag:")))
        {
            FString FlagName = Event.Mid(9);
            SetGameFlag(FlagName, true);
        }
        else if (Event.StartsWith(TEXT("trust_change:")))
        {
            TArray<FString> Parts;
            Event.ParseIntoArray(Parts, TEXT(":"));
            if (Parts.Num() >= 3)
            {
                FString CharacterID = Parts[1];
                float TrustDelta = FCString::Atof(*Parts[2]);
                UpdateCharacterTrust(CharacterID, TrustDelta);
            }
        }
        else if (Event.StartsWith(TEXT("trigger_beat:")))
        {
            FString BeatID = Event.Mid(13);
            TriggerStoryBeat(BeatID);
        }
        
        UE_LOG(LogTemp, Log, TEXT("Executed narrative event: %s"), *Event);
    }
}