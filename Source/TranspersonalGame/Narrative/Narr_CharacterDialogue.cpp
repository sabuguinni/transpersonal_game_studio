#include "Narr_CharacterDialogue.h"
#include "Engine/Engine.h"

UNarr_CharacterDialogue::UNarr_CharacterDialogue()
{
    // Initialize default character profile
    CharacterProfile.CharacterName = FText::FromString("Unknown Survivor");
    CharacterProfile.Role = ENarr_CharacterRole::CampKeeper;
    CharacterProfile.TrustLevel = 0.5f;
}

FNarr_DialogueLine UNarr_CharacterDialogue::GetRandomDialogue(ENarr_DialogueType Type)
{
    if (DialoguesByType.Contains(Type) && DialoguesByType[Type].Num() > 0)
    {
        const TArray<FNarr_DialogueLine>& Lines = DialoguesByType[Type];
        int32 RandomIndex = FMath::RandRange(0, Lines.Num() - 1);
        return Lines[RandomIndex];
    }

    // Return default dialogue if type not found
    FNarr_DialogueLine DefaultLine;
    DefaultLine.DialogueText = FText::FromString("The survivor remains silent, watching the horizon.");
    DefaultLine.DialogueType = Type;
    DefaultLine.Duration = 2.0f;
    return DefaultLine;
}

TArray<FNarr_DialogueLine> UNarr_CharacterDialogue::GetDialogueByType(ENarr_DialogueType Type)
{
    if (DialoguesByType.Contains(Type))
    {
        return DialoguesByType[Type];
    }
    return TArray<FNarr_DialogueLine>();
}

bool UNarr_CharacterDialogue::HasDialogueForType(ENarr_DialogueType Type)
{
    return DialoguesByType.Contains(Type) && DialoguesByType[Type].Num() > 0;
}

void UNarr_CharacterDialogue::ModifyTrustLevel(float Delta)
{
    CharacterProfile.TrustLevel = FMath::Clamp(CharacterProfile.TrustLevel + Delta, 0.0f, 1.0f);
    
    // Log trust change for debugging
    UE_LOG(LogTemp, Log, TEXT("Character %s trust level changed by %f, now %f"), 
           *CharacterProfile.CharacterName.ToString(), 
           Delta, 
           CharacterProfile.TrustLevel);
}

bool UNarr_CharacterDialogue::CanDiscussTopics(const TArray<FString>& Topics)
{
    for (const FString& Topic : Topics)
    {
        if (CharacterProfile.KnownTopics.Contains(Topic))
        {
            return true;
        }
    }
    return false;
}