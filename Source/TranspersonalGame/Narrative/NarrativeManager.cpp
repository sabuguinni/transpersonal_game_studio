#include "NarrativeManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

UNarrativeManager::UNarrativeManager()
{
    CurrentDialogueIndex = 0;
    bIsInDialogue = false;
    CurrentCharacterID = TEXT("");
}

void UNarrativeManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Initializing narrative subsystem"));
    
    LoadStoryData();
    LoadCharacterData();
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Initialized with %d story beats and %d characters"), 
           StoryBeats.Num(), CharacterProfiles.Num());
}

void UNarrativeManager::Deinitialize()
{
    UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Shutting down narrative subsystem"));
    
    StoryBeats.Empty();
    CharacterProfiles.Empty();
    CurrentDialogue.Empty();
    
    Super::Deinitialize();
}

void UNarrativeManager::AdvanceStoryBeat(const FString& BeatID)
{
    for (FNarr_StoryBeat& Beat : StoryBeats)
    {
        if (Beat.BeatID == BeatID && !Beat.bIsCompleted)
        {
            Beat.bIsCompleted = true;
            UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Completed story beat '%s'"), *BeatID);
            
            // Unlock new quests
            for (const FString& QuestID : Beat.UnlockedQuests)
            {
                UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Unlocked quest '%s'"), *QuestID);
            }
            
            break;
        }
    }
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

TArray<FString> UNarrativeManager::GetAvailableQuests() const
{
    TArray<FString> AvailableQuests;
    
    for (const FNarr_StoryBeat& Beat : StoryBeats)
    {
        if (Beat.bIsCompleted)
        {
            for (const FString& QuestID : Beat.UnlockedQuests)
            {
                AvailableQuests.AddUnique(QuestID);
            }
        }
    }
    
    return AvailableQuests;
}

void UNarrativeManager::StartDialogue(const FString& CharacterID, const FString& DialogueID)
{
    if (bIsInDialogue)
    {
        UE_LOG(LogTemp, Warning, TEXT("NarrativeManager: Already in dialogue, ending current dialogue first"));
        EndDialogue();
    }
    
    CurrentCharacterID = CharacterID;
    CurrentDialogueIndex = 0;
    bIsInDialogue = true;
    
    // Load dialogue lines for this character/dialogue combination
    // For now, create a sample dialogue
    CurrentDialogue.Empty();
    
    FNarr_DialogueLine SampleLine;
    SampleLine.SpeakerName = CharacterID;
    SampleLine.DialogueText = FText::FromString(TEXT("Greetings, traveler. The spirits have guided you here."));
    SampleLine.Duration = 4.0f;
    SampleLine.ResponseOptions.Add(TEXT("Tell me about the spirits."));
    SampleLine.ResponseOptions.Add(TEXT("I seek wisdom."));
    SampleLine.ResponseOptions.Add(TEXT("Farewell."));
    
    CurrentDialogue.Add(SampleLine);
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Started dialogue with %s"), *CharacterID);
}

void UNarrativeManager::EndDialogue()
{
    if (bIsInDialogue)
    {
        UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Ended dialogue with %s"), *CurrentCharacterID);
        
        bIsInDialogue = false;
        CurrentCharacterID = TEXT("");
        CurrentDialogueIndex = 0;
        CurrentDialogue.Empty();
    }
}

FNarr_DialogueLine UNarrativeManager::GetCurrentDialogueLine() const
{
    if (bIsInDialogue && CurrentDialogue.IsValidIndex(CurrentDialogueIndex))
    {
        return CurrentDialogue[CurrentDialogueIndex];
    }
    
    return FNarr_DialogueLine();
}

void UNarrativeManager::SelectDialogueResponse(int32 ResponseIndex)
{
    if (!bIsInDialogue || !CurrentDialogue.IsValidIndex(CurrentDialogueIndex))
    {
        return;
    }
    
    const FNarr_DialogueLine& CurrentLine = CurrentDialogue[CurrentDialogueIndex];
    
    if (CurrentLine.ResponseOptions.IsValidIndex(ResponseIndex))
    {
        const FString& SelectedResponse = CurrentLine.ResponseOptions[ResponseIndex];
        UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Player selected response: %s"), *SelectedResponse);
        
        // Advance dialogue or end based on response
        if (SelectedResponse.Contains(TEXT("Farewell")))
        {
            EndDialogue();
        }
        else
        {
            // For now, just end dialogue after any response
            // In a full implementation, this would branch to different dialogue paths
            EndDialogue();
        }
    }
}

void UNarrativeManager::ModifyRelationship(const FString& CharacterID, float DeltaValue)
{
    for (FNarr_CharacterProfile& Profile : CharacterProfiles)
    {
        if (Profile.CharacterID == CharacterID)
        {
            Profile.RelationshipLevel = FMath::Clamp(Profile.RelationshipLevel + DeltaValue, -100.0f, 100.0f);
            UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Modified relationship with %s by %.1f (now %.1f)"), 
                   *CharacterID, DeltaValue, Profile.RelationshipLevel);
            return;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("NarrativeManager: Character %s not found for relationship modification"), *CharacterID);
}

float UNarrativeManager::GetRelationshipLevel(const FString& CharacterID) const
{
    for (const FNarr_CharacterProfile& Profile : CharacterProfiles)
    {
        if (Profile.CharacterID == CharacterID)
        {
            return Profile.RelationshipLevel;
        }
    }
    
    return 0.0f;
}

FNarr_CharacterProfile UNarrativeManager::GetCharacterProfile(const FString& CharacterID) const
{
    for (const FNarr_CharacterProfile& Profile : CharacterProfiles)
    {
        if (Profile.CharacterID == CharacterID)
        {
            return Profile;
        }
    }
    
    return FNarr_CharacterProfile();
}

void UNarrativeManager::TriggerConsciousnessEvent(const FString& EventID, float IntensityLevel)
{
    UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Consciousness event '%s' triggered with intensity %.2f"), 
           *EventID, IntensityLevel);
    
    // This would integrate with the consciousness system to trigger narrative events
    // based on the player's consciousness level and specific events
}

void UNarrativeManager::UpdateNarrativeBasedOnConsciousness(float ConsciousnessLevel)
{
    UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Updating narrative based on consciousness level %.2f"), 
           ConsciousnessLevel);
    
    // Adjust available story beats and dialogue options based on consciousness level
    // Higher consciousness might unlock deeper spiritual dialogues and story paths
}

void UNarrativeManager::LoadStoryData()
{
    // In a full implementation, this would load from data tables or JSON files
    InitializeDefaultStoryBeats();
}

void UNarrativeManager::LoadCharacterData()
{
    // In a full implementation, this would load from data tables or JSON files
    InitializeDefaultCharacters();
}

void UNarrativeManager::InitializeDefaultStoryBeats()
{
    StoryBeats.Empty();
    
    // Awakening Arc
    FNarr_StoryBeat AwakeningBeat;
    AwakeningBeat.BeatID = TEXT("awakening_intro");
    AwakeningBeat.Title = FText::FromString(TEXT("The Great Awakening"));
    AwakeningBeat.Description = FText::FromString(TEXT("The player awakens to their consciousness in the prehistoric world."));
    AwakeningBeat.UnlockedQuests.Add(TEXT("meet_elder"));
    AwakeningBeat.UnlockedQuests.Add(TEXT("explore_sacred_grove"));
    AwakeningBeat.bIsCompleted = false;
    StoryBeats.Add(AwakeningBeat);
    
    // Elder Meeting Arc
    FNarr_StoryBeat ElderBeat;
    ElderBeat.BeatID = TEXT("elder_meeting");
    ElderBeat.Title = FText::FromString(TEXT("Wisdom of the Ancients"));
    ElderBeat.Description = FText::FromString(TEXT("The tribal elder shares ancient wisdom about consciousness and the spirit world."));
    ElderBeat.RequiredQuests.Add(TEXT("meet_elder"));
    ElderBeat.UnlockedQuests.Add(TEXT("spirit_quest"));
    ElderBeat.UnlockedQuests.Add(TEXT("beast_communication"));
    ElderBeat.bIsCompleted = false;
    StoryBeats.Add(ElderBeat);
    
    // Beast Speaker Arc
    FNarr_StoryBeat BeastBeat;
    BeastBeat.BeatID = TEXT("beast_communion");
    BeastBeat.Title = FText::FromString(TEXT("Language of the Ancient Ones"));
    BeastBeat.Description = FText::FromString(TEXT("Learning to communicate with the great beasts of the prehistoric world."));
    BeastBeat.RequiredQuests.Add(TEXT("beast_communication"));
    BeastBeat.UnlockedQuests.Add(TEXT("dinosaur_alliance"));
    BeastBeat.bIsCompleted = false;
    StoryBeats.Add(BeastBeat);
}

void UNarrativeManager::InitializeDefaultCharacters()
{
    CharacterProfiles.Empty();
    
    // Elder Shaman
    FNarr_CharacterProfile ElderProfile;
    ElderProfile.CharacterID = TEXT("elder_shaman");
    ElderProfile.CharacterName = FText::FromString(TEXT("Keeper of Ancient Ways"));
    ElderProfile.BackgroundStory = FText::FromString(TEXT("A wise elder who has spent decades studying the connection between consciousness and the natural world."));
    ElderProfile.PersonalityTraits.Add(TEXT("Wise"));
    ElderProfile.PersonalityTraits.Add(TEXT("Patient"));
    ElderProfile.PersonalityTraits.Add(TEXT("Mystical"));
    ElderProfile.VoiceActorProfile = TEXT("Elder_Shaman");
    ElderProfile.RelationshipLevel = 10.0f;
    CharacterProfiles.Add(ElderProfile);
    
    // Beast Speaker
    FNarr_CharacterProfile BeastProfile;
    BeastProfile.CharacterID = TEXT("beast_speaker");
    BeastProfile.CharacterName = FText::FromString(TEXT("Voice of the Wild"));
    BeastProfile.BackgroundStory = FText::FromString(TEXT("A tribal member with the rare gift of communicating with the great beasts."));
    BeastProfile.PersonalityTraits.Add(TEXT("Intuitive"));
    BeastProfile.PersonalityTraits.Add(TEXT("Brave"));
    BeastProfile.PersonalityTraits.Add(TEXT("Empathetic"));
    BeastProfile.VoiceActorProfile = TEXT("Beast_Speaker");
    BeastProfile.RelationshipLevel = 0.0f;
    CharacterProfiles.Add(BeastProfile);
    
    // Tribal Leader
    FNarr_CharacterProfile LeaderProfile;
    LeaderProfile.CharacterID = TEXT("tribal_leader");
    LeaderProfile.CharacterName = FText::FromString(TEXT("Guardian of the People"));
    LeaderProfile.BackgroundStory = FText::FromString(TEXT("The strong leader who guides the tribe through the challenges of the prehistoric world."));
    LeaderProfile.PersonalityTraits.Add(TEXT("Charismatic"));
    LeaderProfile.PersonalityTraits.Add(TEXT("Protective"));
    LeaderProfile.PersonalityTraits.Add(TEXT("Inspiring"));
    LeaderProfile.VoiceActorProfile = TEXT("Tribal_Leader");
    LeaderProfile.RelationshipLevel = 5.0f;
    CharacterProfiles.Add(LeaderProfile);
}