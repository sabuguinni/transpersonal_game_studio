#include "Narr_DialogueSystem.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

UNarr_DialogueSystem::UNarr_DialogueSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;

    bIsInConversation = false;
    CurrentConversationID = TEXT("");
    CurrentLineIndex = 0;

    // Create audio component for dialogue playback
    DialogueAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("DialogueAudioComponent"));
    if (DialogueAudioComponent)
    {
        DialogueAudioComponent->bAutoActivate = false;
        DialogueAudioComponent->SetVolumeMultiplier(1.0f);
    }
}

void UNarr_DialogueSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeDefaultConversations();
    
    // Initialize survival context with default values
    CurrentSurvivalContext.CurrentPriority = ENarr_SurvivalPriority::Water;
    CurrentSurvivalContext.TimeOfDay = 12.0f;
    CurrentSurvivalContext.CurrentBiome = ENarr_BiomeType::Forest;
    CurrentSurvivalContext.ThreatLevel = 0.3f;
    
    UE_LOG(LogTemp, Log, TEXT("Narrative Dialogue System initialized"));
}

void UNarr_DialogueSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update dialogue audio state
    if (bIsInConversation && DialogueAudioComponent)
    {
        if (!DialogueAudioComponent->IsPlaying())
        {
            // Audio finished, could auto-advance dialogue here if desired
        }
    }
}

bool UNarr_DialogueSystem::StartConversation(const FString& ConversationID)
{
    if (bIsInConversation)
    {
        UE_LOG(LogTemp, Warning, TEXT("Already in conversation. End current conversation first."));
        return false;
    }

    if (!RegisteredConversations.Contains(ConversationID))
    {
        UE_LOG(LogTemp, Error, TEXT("Conversation not found: %s"), *ConversationID);
        return false;
    }

    const FNarr_DialogueConversation& Conversation = RegisteredConversations[ConversationID];
    if (Conversation.DialogueLines.Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("Conversation has no dialogue lines: %s"), *ConversationID);
        return false;
    }

    bIsInConversation = true;
    CurrentConversationID = ConversationID;
    CurrentLineIndex = Conversation.StartingLineIndex;

    const FNarr_DialogueLine& FirstLine = Conversation.DialogueLines[CurrentLineIndex];
    BroadcastDialogueEvents(FirstLine);
    PlayDialogueAudio(FirstLine);

    OnDialogueStarted.Broadcast(ConversationID, FirstLine);
    
    UE_LOG(LogTemp, Log, TEXT("Started conversation: %s"), *ConversationID);
    return true;
}

void UNarr_DialogueSystem::EndConversation()
{
    if (!bIsInConversation)
    {
        return;
    }

    StopDialogueAudio();
    
    FString EndedConversationID = CurrentConversationID;
    
    bIsInConversation = false;
    CurrentConversationID = TEXT("");
    CurrentLineIndex = 0;

    OnDialogueEnded.Broadcast(EndedConversationID);
    
    UE_LOG(LogTemp, Log, TEXT("Ended conversation: %s"), *EndedConversationID);
}

bool UNarr_DialogueSystem::AdvanceDialogue(int32 ChoiceIndex)
{
    if (!bIsInConversation || !RegisteredConversations.Contains(CurrentConversationID))
    {
        return false;
    }

    const FNarr_DialogueConversation& Conversation = RegisteredConversations[CurrentConversationID];
    if (!IsValidLineIndex(CurrentLineIndex))
    {
        EndConversation();
        return false;
    }

    const FNarr_DialogueLine& CurrentLine = Conversation.DialogueLines[CurrentLineIndex];
    
    // Check if we have valid next lines
    if (CurrentLine.NextLineIndices.Num() == 0)
    {
        // End of conversation
        EndConversation();
        return true;
    }

    // Validate choice index
    if (ChoiceIndex < 0 || ChoiceIndex >= CurrentLine.NextLineIndices.Num())
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid choice index: %d"), ChoiceIndex);
        return false;
    }

    // Advance to next line
    int32 NextLineIndex = CurrentLine.NextLineIndices[ChoiceIndex];
    if (!IsValidLineIndex(NextLineIndex))
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid next line index: %d"), NextLineIndex);
        EndConversation();
        return false;
    }

    CurrentLineIndex = NextLineIndex;
    const FNarr_DialogueLine& NextLine = Conversation.DialogueLines[CurrentLineIndex];
    
    BroadcastDialogueEvents(NextLine);
    PlayDialogueAudio(NextLine);

    return true;
}

void UNarr_DialogueSystem::RegisterConversation(const FNarr_DialogueConversation& Conversation)
{
    if (Conversation.ConversationID.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot register conversation with empty ID"));
        return;
    }

    RegisteredConversations.Add(Conversation.ConversationID, Conversation);
    UE_LOG(LogTemp, Log, TEXT("Registered conversation: %s"), *Conversation.ConversationID);
}

FString UNarr_DialogueSystem::GetContextualDialogue(const FNarr_SurvivalContext& Context)
{
    return GenerateContextualResponse(Context);
}

void UNarr_DialogueSystem::UpdateSurvivalContext(const FNarr_SurvivalContext& NewContext)
{
    CurrentSurvivalContext = NewContext;
    UE_LOG(LogTemp, Log, TEXT("Updated survival context - Priority: %d, Threat: %f"), 
           (int32)NewContext.CurrentPriority, NewContext.ThreatLevel);
}

void UNarr_DialogueSystem::PlayDialogueAudio(const FNarr_DialogueLine& DialogueLine)
{
    if (!DialogueAudioComponent)
    {
        return;
    }

    StopDialogueAudio();

    if (DialogueLine.VoiceAudio.IsValid())
    {
        USoundCue* AudioCue = DialogueLine.VoiceAudio.LoadSynchronous();
        if (AudioCue)
        {
            DialogueAudioComponent->SetSound(AudioCue);
            DialogueAudioComponent->Play();
            UE_LOG(LogTemp, Log, TEXT("Playing dialogue audio for: %s"), *DialogueLine.SpeakerName);
        }
    }
}

void UNarr_DialogueSystem::StopDialogueAudio()
{
    if (DialogueAudioComponent && DialogueAudioComponent->IsPlaying())
    {
        DialogueAudioComponent->Stop();
    }
}

FNarr_DialogueLine UNarr_DialogueSystem::GetCurrentDialogueLine() const
{
    if (!bIsInConversation || !RegisteredConversations.Contains(CurrentConversationID))
    {
        return FNarr_DialogueLine();
    }

    const FNarr_DialogueConversation& Conversation = RegisteredConversations[CurrentConversationID];
    if (IsValidLineIndex(CurrentLineIndex))
    {
        return Conversation.DialogueLines[CurrentLineIndex];
    }

    return FNarr_DialogueLine();
}

TArray<FNarr_DialogueLine> UNarr_DialogueSystem::GetCurrentChoices() const
{
    TArray<FNarr_DialogueLine> Choices;
    
    if (!bIsInConversation || !RegisteredConversations.Contains(CurrentConversationID))
    {
        return Choices;
    }

    const FNarr_DialogueConversation& Conversation = RegisteredConversations[CurrentConversationID];
    if (!IsValidLineIndex(CurrentLineIndex))
    {
        return Choices;
    }

    const FNarr_DialogueLine& CurrentLine = Conversation.DialogueLines[CurrentLineIndex];
    
    // Get all next dialogue lines as choices
    for (int32 NextIndex : CurrentLine.NextLineIndices)
    {
        if (IsValidLineIndex(NextIndex))
        {
            Choices.Add(Conversation.DialogueLines[NextIndex]);
        }
    }

    return Choices;
}

bool UNarr_DialogueSystem::IsValidLineIndex(int32 LineIndex) const
{
    if (!RegisteredConversations.Contains(CurrentConversationID))
    {
        return false;
    }

    const FNarr_DialogueConversation& Conversation = RegisteredConversations[CurrentConversationID];
    return LineIndex >= 0 && LineIndex < Conversation.DialogueLines.Num();
}

void UNarr_DialogueSystem::InitializeDefaultConversations()
{
    // Create default survival conversations
    
    // Water urgency conversation
    FNarr_DialogueConversation WaterConversation;
    WaterConversation.ConversationID = TEXT("WaterUrgency");
    WaterConversation.StartingLineIndex = 0;
    WaterConversation.bIsRepeatable = true;

    FNarr_DialogueLine WaterLine1;
    WaterLine1.SpeakerName = TEXT("Player");
    WaterLine1.DialogueText = FText::FromString(TEXT("Water... I need water. My throat burns like fire."));
    WaterLine1.DisplayDuration = 3.0f;
    WaterLine1.NextLineIndices.Add(1);

    FNarr_DialogueLine WaterLine2;
    WaterLine2.SpeakerName = TEXT("Narrator");
    WaterLine2.DialogueText = FText::FromString(TEXT("The river calls, but predators lurk near water sources."));
    WaterLine2.DisplayDuration = 4.0f;

    WaterConversation.DialogueLines.Add(WaterLine1);
    WaterConversation.DialogueLines.Add(WaterLine2);
    RegisterConversation(WaterConversation);

    // Predator warning conversation
    FNarr_DialogueConversation PredatorConversation;
    PredatorConversation.ConversationID = TEXT("PredatorWarning");
    PredatorConversation.StartingLineIndex = 0;

    FNarr_DialogueLine PredatorLine1;
    PredatorLine1.SpeakerName = TEXT("Tracker");
    PredatorLine1.DialogueText = FText::FromString(TEXT("Three-toed prints in the mud. Raptors hunt here."));
    PredatorLine1.DisplayDuration = 3.5f;
    PredatorLine1.NextLineIndices.Add(1);

    FNarr_DialogueLine PredatorLine2;
    PredatorLine2.SpeakerName = TEXT("Tracker");
    PredatorLine2.DialogueText = FText::FromString(TEXT("Stay downwind. Move slowly. Never turn your back."));
    PredatorLine2.DisplayDuration = 4.0f;

    PredatorConversation.DialogueLines.Add(PredatorLine1);
    PredatorConversation.DialogueLines.Add(PredatorLine2);
    RegisterConversation(PredatorConversation);

    UE_LOG(LogTemp, Log, TEXT("Initialized default survival conversations"));
}

FString UNarr_DialogueSystem::GenerateContextualResponse(const FNarr_SurvivalContext& Context)
{
    FString Response = TEXT("The ancient world watches...");

    // Generate response based on survival priority
    switch (Context.CurrentPriority)
    {
        case ENarr_SurvivalPriority::Water:
            Response = TEXT("Water is life. Without it, you have hours, not days.");
            break;
        case ENarr_SurvivalPriority::Food:
            Response = TEXT("Hunger weakens the body and clouds the mind. Find sustenance.");
            break;
        case ENarr_SurvivalPriority::Shelter:
            Response = TEXT("The elements show no mercy. Seek protection from wind and rain.");
            break;
        case ENarr_SurvivalPriority::Safety:
            Response = TEXT("Danger lurks in every shadow. Trust your instincts.");
            break;
    }

    // Modify based on threat level
    if (Context.ThreatLevel > 0.7f)
    {
        Response += TEXT(" Immediate danger detected!");
    }
    else if (Context.ThreatLevel > 0.4f)
    {
        Response += TEXT(" Stay alert.");
    }

    // Add time-based context
    if (Context.TimeOfDay < 6.0f || Context.TimeOfDay > 18.0f)
    {
        Response += TEXT(" Night brings greater perils.");
    }

    return Response;
}

void UNarr_DialogueSystem::BroadcastDialogueEvents(const FNarr_DialogueLine& Line)
{
    OnDialogueLineChanged.Broadcast(CurrentLineIndex, Line);
    
    UE_LOG(LogTemp, Log, TEXT("Dialogue: [%s] %s"), 
           *Line.SpeakerName, *Line.DialogueText.ToString());
}