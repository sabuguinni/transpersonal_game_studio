#include "Narr_DialogueManager.h"
#include "TranspersonalCharacter.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/DataTable.h"
#include "Engine/Engine.h"

UNarr_DialogueManager::UNarr_DialogueManager()
{
    bInConversation = false;
    CurrentNPC = TEXT("");
    DialogueDataTable = nullptr;
    DialogueAudioComponent = nullptr;
}

void UNarr_DialogueManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    LoadDialogueData();
    InitializeNPCStates();
    
    UE_LOG(LogTemp, Log, TEXT("Narrative Dialogue Manager initialized"));
}

void UNarr_DialogueManager::Deinitialize()
{
    if (DialogueAudioComponent && DialogueAudioComponent->IsValidLowLevel())
    {
        DialogueAudioComponent->Stop();
        DialogueAudioComponent = nullptr;
    }
    
    ConversationStates.Empty();
    
    Super::Deinitialize();
}

bool UNarr_DialogueManager::StartConversation(const FString& NPCName, ATranspersonalCharacter* Player)
{
    if (bInConversation)
    {
        UE_LOG(LogTemp, Warning, TEXT("Already in conversation with %s"), *CurrentNPC);
        return false;
    }
    
    if (!Player)
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid player reference for dialogue"));
        return false;
    }
    
    // Get player's survival condition
    ESurvivalCondition PlayerCondition = ESurvivalCondition::Healthy;
    if (Player->GetHealthPercentage() < 0.3f)
    {
        PlayerCondition = ESurvivalCondition::Critical;
    }
    else if (Player->GetHealthPercentage() < 0.6f || Player->GetHungerLevel() > 0.7f)
    {
        PlayerCondition = ESurvivalCondition::Struggling;
    }
    
    // Select appropriate dialogue
    CurrentDialogue = SelectAppropriateDialogue(NPCName, PlayerCondition);
    
    if (CurrentDialogue.SpeakerName.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("No dialogue found for NPC: %s"), *NPCName);
        return false;
    }
    
    bInConversation = true;
    CurrentNPC = NPCName;
    
    // Update conversation state
    FNarr_ConversationState& State = ConversationStates.FindOrAdd(NPCName);
    State.bHasMetBefore = true;
    State.LastInteractionTime = GetWorld()->GetTimeSeconds();
    
    // Play dialogue audio if available
    if (!CurrentDialogue.AudioFilePath.IsEmpty())
    {
        PlayDialogueAudio(CurrentDialogue.AudioFilePath);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Started conversation with %s: %s"), *NPCName, *CurrentDialogue.DialogueText.ToString());
    return true;
}

void UNarr_DialogueManager::EndConversation()
{
    if (!bInConversation)
    {
        return;
    }
    
    StopDialogueAudio();
    
    bInConversation = false;
    CurrentNPC = TEXT("");
    CurrentDialogue = FNarr_DialogueEntry();
    
    UE_LOG(LogTemp, Log, TEXT("Conversation ended"));
}

FNarr_DialogueEntry UNarr_DialogueManager::GetCurrentDialogue() const
{
    return CurrentDialogue;
}

TArray<FText> UNarr_DialogueManager::GetAvailableResponses() const
{
    if (!bInConversation)
    {
        return TArray<FText>();
    }
    
    return CurrentDialogue.PlayerResponses;
}

void UNarr_DialogueManager::SelectResponse(int32 ResponseIndex)
{
    if (!bInConversation || ResponseIndex < 0 || ResponseIndex >= CurrentDialogue.PlayerResponses.Num())
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid response selection: %d"), ResponseIndex);
        return;
    }
    
    FText SelectedResponse = CurrentDialogue.PlayerResponses[ResponseIndex];
    UE_LOG(LogTemp, Log, TEXT("Player selected response: %s"), *SelectedResponse.ToString());
    
    // Update conversation stage based on response
    FNarr_ConversationState& State = ConversationStates.FindOrAdd(CurrentNPC);
    State.ConversationStage++;
    
    // End conversation for now (can be extended for multi-stage dialogues)
    EndConversation();
}

void UNarr_DialogueManager::UpdateConversationState(const FString& NPCName, int32 NewStage)
{
    FNarr_ConversationState& State = ConversationStates.FindOrAdd(NPCName);
    State.ConversationStage = NewStage;
    State.NPCName = NPCName;
}

FNarr_ConversationState UNarr_DialogueManager::GetConversationState(const FString& NPCName) const
{
    const FNarr_ConversationState* State = ConversationStates.Find(NPCName);
    if (State)
    {
        return *State;
    }
    
    return FNarr_ConversationState();
}

void UNarr_DialogueManager::MarkTopicCompleted(const FString& NPCName, const FString& TopicName)
{
    FNarr_ConversationState& State = ConversationStates.FindOrAdd(NPCName);
    State.CompletedTopics.AddUnique(TopicName);
    
    UE_LOG(LogTemp, Log, TEXT("Marked topic '%s' as completed for %s"), *TopicName, *NPCName);
}

TArray<FNarr_DialogueEntry> UNarr_DialogueManager::GetSurvivalDialogue(ESurvivalCondition PlayerCondition) const
{
    TArray<FNarr_DialogueEntry> SurvivalDialogues;
    
    // Create contextual survival dialogues based on player condition
    if (PlayerCondition == ESurvivalCondition::Critical)
    {
        FNarr_DialogueEntry CriticalDialogue;
        CriticalDialogue.SpeakerName = TEXT("Elder Kava");
        CriticalDialogue.DialogueText = FText::FromString(TEXT("You look gravely wounded, young one. Rest by our fire and tend to your injuries before the predators catch your scent."));
        CriticalDialogue.Duration = 6.0f;
        SurvivalDialogues.Add(CriticalDialogue);
    }
    else if (PlayerCondition == ESurvivalCondition::Struggling)
    {
        FNarr_DialogueEntry StrugglingDialogue;
        StrugglingDialogue.SpeakerName = TEXT("Scout Thane");
        StrugglingDialogue.DialogueText = FText::FromString(TEXT("I can see the hunger in your eyes. The berry bushes near the eastern cliffs bear fruit this season, but beware the raptors that hunt there."));
        StrugglingDialogue.Duration = 7.0f;
        SurvivalDialogues.Add(StrugglingDialogue);
    }
    
    return SurvivalDialogues;
}

bool UNarr_DialogueManager::CanAccessDialogue(const FNarr_DialogueEntry& Dialogue, ESurvivalCondition PlayerCondition) const
{
    return PlayerCondition == Dialogue.RequiredCondition || Dialogue.RequiredCondition == ESurvivalCondition::Healthy;
}

void UNarr_DialogueManager::PlayDialogueAudio(const FString& AudioPath)
{
    if (AudioPath.IsEmpty())
    {
        return;
    }
    
    // Stop current audio if playing
    StopDialogueAudio();
    
    // Load and play audio (simplified implementation)
    UE_LOG(LogTemp, Log, TEXT("Playing dialogue audio: %s"), *AudioPath);
    
    // In a full implementation, this would load the actual audio file
    // and play it through the audio component
}

void UNarr_DialogueManager::StopDialogueAudio()
{
    if (DialogueAudioComponent && DialogueAudioComponent->IsValidLowLevel())
    {
        DialogueAudioComponent->Stop();
    }
}

void UNarr_DialogueManager::LoadDialogueData()
{
    // Initialize default dialogue entries for key NPCs
    UE_LOG(LogTemp, Log, TEXT("Loading dialogue data for NPCs"));
    
    // This would typically load from a data table in a full implementation
    // For now, we initialize with hardcoded survival-focused dialogues
}

void UNarr_DialogueManager::InitializeNPCStates()
{
    // Initialize conversation states for key NPCs
    TArray<FString> NPCNames = {
        TEXT("Elder_Kava_Stormwatcher"),
        TEXT("Scout_Thane_Pathfinder"),
        TEXT("Craft_Master_Nira_Stonehand")
    };
    
    for (const FString& NPCName : NPCNames)
    {
        FNarr_ConversationState NewState;
        NewState.NPCName = NPCName;
        NewState.ConversationStage = 0;
        NewState.bHasMetBefore = false;
        NewState.LastInteractionTime = 0.0f;
        
        ConversationStates.Add(NPCName, NewState);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Initialized conversation states for %d NPCs"), NPCNames.Num());
}

FNarr_DialogueEntry UNarr_DialogueManager::SelectAppropriateDialogue(const FString& NPCName, ESurvivalCondition PlayerCondition) const
{
    FNarr_DialogueEntry SelectedDialogue;
    
    // Select dialogue based on NPC and player condition
    if (NPCName == TEXT("Elder_Kava_Stormwatcher"))
    {
        SelectedDialogue.SpeakerName = TEXT("Elder Kava");
        if (PlayerCondition == ESurvivalCondition::Critical)
        {
            SelectedDialogue.DialogueText = FText::FromString(TEXT("The spirits whisper of your pain, young hunter. Rest here and let our healer tend to your wounds."));
        }
        else
        {
            SelectedDialogue.DialogueText = FText::FromString(TEXT("Welcome, traveler. Our tribe has survived these lands for many seasons. What wisdom do you seek?"));
        }
        SelectedDialogue.PlayerResponses = {
            FText::FromString(TEXT("Tell me about the dangers here")),
            FText::FromString(TEXT("Where can I find food and water?")),
            FText::FromString(TEXT("I must continue my journey"))
        };
    }
    else if (NPCName == TEXT("Scout_Thane_Pathfinder"))
    {
        SelectedDialogue.SpeakerName = TEXT("Scout Thane");
        SelectedDialogue.DialogueText = FText::FromString(TEXT("I know every path through these hunting grounds. The great beasts follow ancient routes - learn them, and you might survive."));
        SelectedDialogue.PlayerResponses = {
            FText::FromString(TEXT("Show me the safe paths")),
            FText::FromString(TEXT("What creatures roam here?")),
            FText::FromString(TEXT("I can handle myself"))
        };
    }
    else if (NPCName == TEXT("Craft_Master_Nira_Stonehand"))
    {
        SelectedDialogue.SpeakerName = TEXT("Craft Master Nira");
        SelectedDialogue.DialogueText = FText::FromString(TEXT("Your tools are crude, but they show promise. Let me teach you the old ways of shaping stone and bone."));
        SelectedDialogue.PlayerResponses = {
            FText::FromString(TEXT("Teach me to craft better weapons")),
            FText::FromString(TEXT("How do I build a proper shelter?")),
            FText::FromString(TEXT("I prefer to learn on my own"))
        };
    }
    
    SelectedDialogue.Duration = 5.0f;
    SelectedDialogue.RequiredCondition = ESurvivalCondition::Healthy;
    
    return SelectedDialogue;
}