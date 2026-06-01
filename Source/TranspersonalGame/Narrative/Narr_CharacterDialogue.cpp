#include "Narr_CharacterDialogue.h"

UNarr_CharacterDialogue::UNarr_CharacterDialogue()
{
    CharacterName = TEXT("Unnamed NPC");
    NPCType = ENarr_NPCType::Villager;
    
    // Setup default greeting
    DefaultGreeting.SequenceID = TEXT("greeting");
    DefaultGreeting.bIsRepeatable = true;
    
    FNarr_DialogueLine greetingLine;
    greetingLine.SpeakerName = CharacterName;
    greetingLine.DialogueText = FText::FromString(TEXT("Greetings, hunter. The wilds are dangerous today."));
    greetingLine.Duration = 3.5f;
    
    DefaultGreeting.DialogueLines.Add(greetingLine);
    
    // Setup emergency warning
    EmergencyWarning.SequenceID = TEXT("emergency");
    EmergencyWarning.bIsRepeatable = true;
    
    FNarr_DialogueLine warningLine;
    warningLine.SpeakerName = CharacterName;
    warningLine.DialogueText = FText::FromString(TEXT("Danger approaches! Seek shelter immediately!"));
    warningLine.Duration = 2.5f;
    
    EmergencyWarning.DialogueLines.Add(warningLine);
}

FNarr_DialogueSequence UNarr_CharacterDialogue::GetDialogueByID(const FString& SequenceID)
{
    if (SequenceID == TEXT("greeting"))
    {
        return DefaultGreeting;
    }
    
    if (SequenceID == TEXT("emergency"))
    {
        return EmergencyWarning;
    }
    
    for (const FNarr_DialogueSequence& Sequence : DialogueSequences)
    {
        if (Sequence.SequenceID == SequenceID)
        {
            return Sequence;
        }
    }
    
    // Return default if not found
    return DefaultGreeting;
}

TArray<FString> UNarr_CharacterDialogue::GetAvailableSequences()
{
    TArray<FString> AvailableIDs;
    
    AvailableIDs.Add(DefaultGreeting.SequenceID);
    AvailableIDs.Add(EmergencyWarning.SequenceID);
    
    for (const FNarr_DialogueSequence& Sequence : DialogueSequences)
    {
        AvailableIDs.Add(Sequence.SequenceID);
    }
    
    return AvailableIDs;
}

bool UNarr_CharacterDialogue::HasPrerequisites(const FString& SequenceID, const TArray<FString>& PlayerProgress)
{
    FNarr_DialogueSequence FoundSequence = GetDialogueByID(SequenceID);
    
    if (FoundSequence.Prerequisites.Num() == 0)
    {
        return true; // No prerequisites needed
    }
    
    for (const FString& Prerequisite : FoundSequence.Prerequisites)
    {
        if (!PlayerProgress.Contains(Prerequisite))
        {
            return false; // Missing prerequisite
        }
    }
    
    return true; // All prerequisites met
}