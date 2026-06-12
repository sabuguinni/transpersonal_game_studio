#include "Narr_DialogueSystem.h"
#include "Engine/Engine.h"

UNarr_DialogueSystem::UNarr_DialogueSystem()
{
    CurrentConversationID = TEXT("");
    CurrentDialogueIndex = 0;
    bConversationActive = false;
}

void UNarr_DialogueSystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    LoadDefaultConversations();
    UE_LOG(LogTemp, Warning, TEXT("Narr_DialogueSystem initialized"));
}

void UNarr_DialogueSystem::StartConversation(const FString& ConversationID)
{
    if (RegisteredConversations.Contains(ConversationID))
    {
        CurrentConversationID = ConversationID;
        CurrentDialogueIndex = 0;
        bConversationActive = true;
        UE_LOG(LogTemp, Warning, TEXT("Started conversation: %s"), *ConversationID);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Conversation not found: %s"), *ConversationID);
    }
}

void UNarr_DialogueSystem::EndConversation()
{
    CurrentConversationID = TEXT("");
    CurrentDialogueIndex = 0;
    bConversationActive = false;
    UE_LOG(LogTemp, Warning, TEXT("Conversation ended"));
}

void UNarr_DialogueSystem::AdvanceDialogue()
{
    if (!bConversationActive || !RegisteredConversations.Contains(CurrentConversationID))
    {
        return;
    }

    const FNarr_ConversationData& CurrentConversation = RegisteredConversations[CurrentConversationID];
    CurrentDialogueIndex++;

    if (CurrentDialogueIndex >= CurrentConversation.DialogueEntries.Num())
    {
        EndConversation();
    }
}

bool UNarr_DialogueSystem::IsConversationActive() const
{
    return bConversationActive;
}

FNarr_DialogueEntry UNarr_DialogueSystem::GetCurrentDialogue() const
{
    if (!bConversationActive || !RegisteredConversations.Contains(CurrentConversationID))
    {
        return FNarr_DialogueEntry();
    }

    const FNarr_ConversationData& CurrentConversation = RegisteredConversations[CurrentConversationID];
    if (CurrentDialogueIndex < CurrentConversation.DialogueEntries.Num())
    {
        return CurrentConversation.DialogueEntries[CurrentDialogueIndex];
    }

    return FNarr_DialogueEntry();
}

void UNarr_DialogueSystem::RegisterConversation(const FNarr_ConversationData& ConversationData)
{
    RegisteredConversations.Add(ConversationData.ConversationID, ConversationData);
    UE_LOG(LogTemp, Warning, TEXT("Registered conversation: %s"), *ConversationData.ConversationID);
}

void UNarr_DialogueSystem::LoadDefaultConversations()
{
    // Tracker conversation
    FNarr_ConversationData TrackerConversation;
    TrackerConversation.ConversationID = TEXT("OldTracker_Hunt");
    TrackerConversation.bIsRepeatable = false;

    FNarr_DialogueEntry Entry1;
    Entry1.SpeakerName = TEXT("Old Tracker");
    Entry1.DialogueText = TEXT("The old tracker's eyes narrow as he studies the ground. Three-toed prints, deep in the mud. Fresh blood on the leaves. The great hunter has passed this way.");
    Entry1.DisplayDuration = 5.0f;
    TrackerConversation.DialogueEntries.Add(Entry1);

    FNarr_DialogueEntry Entry2;
    Entry2.SpeakerName = TEXT("Old Tracker");
    Entry2.DialogueText = TEXT("You must be careful, young one. This beast is not like the others. It hunts with purpose, with intelligence.");
    Entry2.DisplayDuration = 4.0f;
    TrackerConversation.DialogueEntries.Add(Entry2);

    RegisterConversation(TrackerConversation);

    // Warning conversation
    FNarr_ConversationData WarningConversation;
    WarningConversation.ConversationID = TEXT("TribalWarner_Alert");
    WarningConversation.bIsRepeatable = true;

    FNarr_DialogueEntry Warning1;
    Warning1.SpeakerName = TEXT("Tribal Warner");
    Warning1.DialogueText = TEXT("Warning echoes through the valley - the great predator stirs. All who hear this call must seek shelter immediately. The hunt begins at dawn.");
    Warning1.DisplayDuration = 6.0f;
    WarningConversation.DialogueEntries.Add(Warning1);

    RegisterConversation(WarningConversation);

    // Survival tips conversation
    FNarr_ConversationData SurvivalConversation;
    SurvivalConversation.ConversationID = TEXT("Survivor_Tips");
    SurvivalConversation.bIsRepeatable = true;

    FNarr_DialogueEntry Tip1;
    Tip1.SpeakerName = TEXT("Experienced Survivor");
    Tip1.DialogueText = TEXT("Stay downwind when the great beasts hunt. Their nose is keener than their eyes.");
    Tip1.DisplayDuration = 3.5f;
    SurvivalConversation.DialogueEntries.Add(Tip1);

    FNarr_DialogueEntry Tip2;
    Tip2.SpeakerName = TEXT("Experienced Survivor");
    Tip2.DialogueText = TEXT("Water sources draw both prey and predator. Approach with caution, leave quickly.");
    Tip2.DisplayDuration = 4.0f;
    SurvivalConversation.DialogueEntries.Add(Tip2);

    RegisterConversation(SurvivalConversation);
}