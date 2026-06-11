#include "Narr_DialogueComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"

UNarr_DialogueComponent::UNarr_DialogueComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    CurrentSequenceID = TEXT("");
    CurrentLineIndex = 0;
    bDialogueActive = false;
    InteractionRange = 300.0f;
}

void UNarr_DialogueComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize default dialogue sequences for testing
    if (DialogueSequences.Num() == 0)
    {
        // Create default tribal elder dialogue
        FNarr_DialogueSequence ElderSequence;
        ElderSequence.SequenceID = TEXT("TribalElder_Greeting");
        ElderSequence.bIsRepeatable = true;
        ElderSequence.Priority = 1;
        
        FNarr_DialogueLine Line1;
        Line1.SpeakerName = TEXT("Tribal Elder");
        Line1.DialogueText = FText::FromString(TEXT("The ancient hunting grounds stretch before you, survivor. The river runs red with danger."));
        Line1.DisplayDuration = 4.0f;
        Line1.bRequiresPlayerResponse = false;
        
        FNarr_DialogueLine Line2;
        Line2.SpeakerName = TEXT("Tribal Elder");
        Line2.DialogueText = FText::FromString(TEXT("Massive predator tracks mark the muddy banks. Will you heed the warning?"));
        Line2.DisplayDuration = 3.5f;
        Line2.bRequiresPlayerResponse = true;
        Line2.PlayerResponses.Add(FText::FromString(TEXT("I understand the danger.")));
        Line2.PlayerResponses.Add(FText::FromString(TEXT("I fear no beast.")));
        
        ElderSequence.DialogueLines.Add(Line1);
        ElderSequence.DialogueLines.Add(Line2);
        DialogueSequences.Add(ElderSequence);
        
        // Create scout warning dialogue
        FNarr_DialogueSequence ScoutSequence;
        ScoutSequence.SequenceID = TEXT("Scout_Warning");
        ScoutSequence.bIsRepeatable = false;
        ScoutSequence.Priority = 2;
        
        FNarr_DialogueLine ScoutLine1;
        ScoutLine1.SpeakerName = TEXT("Scout");
        ScoutLine1.DialogueText = FText::FromString(TEXT("Warning! Thunderfoot approaches from the eastern ridge!"));
        ScoutLine1.DisplayDuration = 3.0f;
        ScoutLine1.bRequiresPlayerResponse = false;
        
        FNarr_DialogueLine ScoutLine2;
        ScoutLine2.SpeakerName = TEXT("Scout");
        ScoutLine2.DialogueText = FText::FromString(TEXT("Seek shelter in the stone caves immediately. The earth trembles beneath its stride."));
        ScoutLine2.DisplayDuration = 4.0f;
        ScoutLine2.bRequiresPlayerResponse = true;
        ScoutLine2.PlayerResponses.Add(FText::FromString(TEXT("Lead me to safety!")));
        ScoutLine2.PlayerResponses.Add(FText::FromString(TEXT("I will face this beast.")));
        
        ScoutSequence.DialogueLines.Add(ScoutLine1);
        ScoutSequence.DialogueLines.Add(ScoutLine2);
        DialogueSequences.Add(ScoutSequence);
    }
}

bool UNarr_DialogueComponent::StartDialogueSequence(const FString& SequenceID)
{
    if (bDialogueActive)
    {
        return false;
    }
    
    FNarr_DialogueSequence* Sequence = FindSequence(SequenceID);
    if (!Sequence || Sequence->DialogueLines.Num() == 0)
    {
        return false;
    }
    
    // Check if sequence can be started
    if (!Sequence->bIsRepeatable && HasCompletedSequence(SequenceID))
    {
        return false;
    }
    
    CurrentSequenceID = SequenceID;
    CurrentLineIndex = 0;
    bDialogueActive = true;
    
    UE_LOG(LogTemp, Log, TEXT("Started dialogue sequence: %s"), *SequenceID);
    return true;
}

bool UNarr_DialogueComponent::AdvanceDialogue()
{
    if (!bDialogueActive)
    {
        return false;
    }
    
    FNarr_DialogueSequence* CurrentSequence = FindSequence(CurrentSequenceID);
    if (!CurrentSequence)
    {
        EndDialogue();
        return false;
    }
    
    CurrentLineIndex++;
    
    if (CurrentLineIndex >= CurrentSequence->DialogueLines.Num())
    {
        // Sequence completed
        MarkSequenceCompleted(CurrentSequenceID);
        EndDialogue();
        return false;
    }
    
    return true;
}

void UNarr_DialogueComponent::EndDialogue()
{
    bDialogueActive = false;
    CurrentSequenceID = TEXT("");
    CurrentLineIndex = 0;
    
    UE_LOG(LogTemp, Log, TEXT("Dialogue ended"));
}

FNarr_DialogueLine UNarr_DialogueComponent::GetCurrentDialogueLine() const
{
    if (!bDialogueActive)
    {
        return FNarr_DialogueLine();
    }
    
    FNarr_DialogueSequence* CurrentSequence = const_cast<UNarr_DialogueComponent*>(this)->FindSequence(CurrentSequenceID);
    if (!CurrentSequence || CurrentLineIndex >= CurrentSequence->DialogueLines.Num())
    {
        return FNarr_DialogueLine();
    }
    
    return CurrentSequence->DialogueLines[CurrentLineIndex];
}

bool UNarr_DialogueComponent::CanInteract(AActor* PlayerActor) const
{
    if (!PlayerActor || !GetOwner())
    {
        return false;
    }
    
    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), PlayerActor->GetActorLocation());
    return Distance <= InteractionRange;
}

void UNarr_DialogueComponent::AddDialogueSequence(const FNarr_DialogueSequence& NewSequence)
{
    // Check if sequence already exists
    for (int32 i = 0; i < DialogueSequences.Num(); i++)
    {
        if (DialogueSequences[i].SequenceID == NewSequence.SequenceID)
        {
            DialogueSequences[i] = NewSequence;
            return;
        }
    }
    
    DialogueSequences.Add(NewSequence);
}

bool UNarr_DialogueComponent::HasCompletedSequence(const FString& SequenceID) const
{
    return CompletedSequences.Contains(SequenceID);
}

TArray<FString> UNarr_DialogueComponent::GetAvailableSequences() const
{
    TArray<FString> Available;
    
    for (const FNarr_DialogueSequence& Sequence : DialogueSequences)
    {
        if (Sequence.bIsRepeatable || !HasCompletedSequence(Sequence.SequenceID))
        {
            Available.Add(Sequence.SequenceID);
        }
    }
    
    return Available;
}

void UNarr_DialogueComponent::SetPlayerResponse(int32 ResponseIndex)
{
    FNarr_DialogueLine CurrentLine = GetCurrentDialogueLine();
    if (CurrentLine.bRequiresPlayerResponse && ResponseIndex >= 0 && ResponseIndex < CurrentLine.PlayerResponses.Num())
    {
        UE_LOG(LogTemp, Log, TEXT("Player selected response %d: %s"), ResponseIndex, *CurrentLine.PlayerResponses[ResponseIndex].ToString());
        AdvanceDialogue();
    }
}

FNarr_DialogueSequence* UNarr_DialogueComponent::FindSequence(const FString& SequenceID)
{
    for (FNarr_DialogueSequence& Sequence : DialogueSequences)
    {
        if (Sequence.SequenceID == SequenceID)
        {
            return &Sequence;
        }
    }
    return nullptr;
}

void UNarr_DialogueComponent::MarkSequenceCompleted(const FString& SequenceID)
{
    if (!CompletedSequences.Contains(SequenceID))
    {
        CompletedSequences.Add(SequenceID);
        UE_LOG(LogTemp, Log, TEXT("Marked dialogue sequence as completed: %s"), *SequenceID);
    }
}