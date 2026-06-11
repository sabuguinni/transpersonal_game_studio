#include "Narr_CharacterDialogue.h"
#include "Engine/Engine.h"

UNarr_CharacterDialogue::UNarr_CharacterDialogue()
{
    CharacterName = TEXT("Unknown Character");
    CharacterType = ECharacterArchetype::Survivor;
    CurrentDialogueIndex = 0;
    bDialogueCompleted = false;
}

FNarr_DialogueLine UNarr_CharacterDialogue::GetCurrentDialogue() const
{
    if (DialogueLines.IsValidIndex(CurrentDialogueIndex))
    {
        return DialogueLines[CurrentDialogueIndex];
    }
    
    // Return default dialogue if index is invalid
    FNarr_DialogueLine DefaultLine;
    DefaultLine.SpeakerName = CharacterName;
    DefaultLine.DialogueText = FText::FromString(TEXT("I have nothing more to say."));
    return DefaultLine;
}

TArray<FNarr_DialogueChoice> UNarr_CharacterDialogue::GetAvailableChoices() const
{
    TArray<FNarr_DialogueChoice> AvailableChoices;
    
    // Filter choices based on current dialogue state
    for (const FNarr_DialogueChoice& Choice : DialogueChoices)
    {
        // For now, include all choices - can add item/condition checks later
        if (!Choice.bRequiresItem)
        {
            AvailableChoices.Add(Choice);
        }
    }
    
    // If no specific choices, add default continue option
    if (AvailableChoices.Num() == 0 && HasMoreDialogue())
    {
        FNarr_DialogueChoice ContinueChoice;
        ContinueChoice.ChoiceText = FText::FromString(TEXT("Continue..."));
        ContinueChoice.NextDialogueID = CurrentDialogueIndex + 1;
        AvailableChoices.Add(ContinueChoice);
    }
    
    return AvailableChoices;
}

bool UNarr_CharacterDialogue::AdvanceDialogue(int32 ChoiceIndex)
{
    TArray<FNarr_DialogueChoice> AvailableChoices = GetAvailableChoices();
    
    if (!AvailableChoices.IsValidIndex(ChoiceIndex))
    {
        return false;
    }
    
    const FNarr_DialogueChoice& SelectedChoice = AvailableChoices[ChoiceIndex];
    
    if (SelectedChoice.NextDialogueID >= 0)
    {
        CurrentDialogueIndex = SelectedChoice.NextDialogueID;
    }
    else
    {
        // Auto-advance to next dialogue
        CurrentDialogueIndex++;
    }
    
    // Check if dialogue is completed
    if (CurrentDialogueIndex >= DialogueLines.Num())
    {
        bDialogueCompleted = true;
        return false;
    }
    
    return true;
}

void UNarr_CharacterDialogue::ResetDialogue()
{
    CurrentDialogueIndex = 0;
    bDialogueCompleted = false;
}

bool UNarr_CharacterDialogue::HasMoreDialogue() const
{
    return !bDialogueCompleted && CurrentDialogueIndex < DialogueLines.Num();
}