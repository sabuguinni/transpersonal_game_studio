#include "Narr_DialogueSystem.h"
#include "Engine/Engine.h"

UNarr_DialogueSystem::UNarr_DialogueSystem()
{
    PrimaryComponentTick.bCanEverTick = false;
    bDialogueActive = false;
    CurrentLineIndex = 0;
    CurrentSequenceID = TEXT("");
}

void UNarr_DialogueSystem::BeginPlay()
{
    Super::BeginPlay();
    InitializeDefaultDialogues();
}

void UNarr_DialogueSystem::InitializeDefaultDialogues()
{
    // Elder Kael - Tribal Wisdom
    FNarr_DialogueSequence ElderIntro;
    ElderIntro.SequenceID = TEXT("elder_intro");
    ElderIntro.bRepeatable = false;

    FNarr_DialogueLine ElderLine1;
    ElderLine1.SpeakerName = TEXT("Elder Kael");
    ElderLine1.DialogueText = FText::FromString(TEXT("Welcome, young hunter. I am Kael, keeper of the ancient ways."));
    ElderLine1.Duration = 4.0f;

    FNarr_DialogueLine ElderLine2;
    ElderLine2.SpeakerName = TEXT("Elder Kael");
    ElderLine2.DialogueText = FText::FromString(TEXT("The great beasts follow patterns older than memory. Learn them, or perish."));
    ElderLine2.Duration = 5.0f;

    ElderIntro.DialogueLines.Add(ElderLine1);
    ElderIntro.DialogueLines.Add(ElderLine2);
    DialogueDatabase.Add(ElderIntro.SequenceID, ElderIntro);

    // War Chief - Danger Warning
    FNarr_DialogueSequence WarningSequence;
    WarningSequence.SequenceID = TEXT("danger_warning");
    WarningSequence.bRepeatable = true;

    FNarr_DialogueLine WarningLine;
    WarningLine.SpeakerName = TEXT("War Chief");
    WarningLine.DialogueText = FText::FromString(TEXT("Blood on the wind... something massive approaches. All hunters to the perimeter!"));
    WarningLine.Duration = 4.5f;

    WarningSequence.DialogueLines.Add(WarningLine);
    DialogueDatabase.Add(WarningSequence.SequenceID, WarningSequence);

    // Resource Gatherer - Survival Tips
    FNarr_DialogueSequence ResourceTips;
    ResourceTips.SequenceID = TEXT("resource_tips");
    ResourceTips.bRepeatable = true;

    FNarr_DialogueLine ResourceLine1;
    ResourceLine1.SpeakerName = TEXT("Vera");
    ResourceLine1.DialogueText = FText::FromString(TEXT("The earth provides for those who know where to look."));
    ResourceLine1.Duration = 3.5f;

    FNarr_DialogueLine ResourceLine2;
    ResourceLine2.SpeakerName = TEXT("Vera");
    ResourceLine2.DialogueText = FText::FromString(TEXT("Sharp stones by the river, healing herbs in the shadow of great trees."));
    ResourceLine2.Duration = 4.0f;

    ResourceTips.DialogueLines.Add(ResourceLine1);
    ResourceTips.DialogueLines.Add(ResourceLine2);
    DialogueDatabase.Add(ResourceTips.SequenceID, ResourceTips);

    UE_LOG(LogTemp, Warning, TEXT("Dialogue System: Initialized %d default sequences"), DialogueDatabase.Num());
}

bool UNarr_DialogueSystem::StartDialogue(const FString& SequenceID)
{
    if (bDialogueActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("Dialogue System: Cannot start dialogue - another dialogue is active"));
        return false;
    }

    if (!DialogueDatabase.Contains(SequenceID))
    {
        UE_LOG(LogTemp, Error, TEXT("Dialogue System: Sequence ID '%s' not found"), *SequenceID);
        return false;
    }

    const FNarr_DialogueSequence& Sequence = DialogueDatabase[SequenceID];
    
    // Check required flags
    if (!CheckRequiredFlags(Sequence.RequiredFlags))
    {
        UE_LOG(LogTemp, Warning, TEXT("Dialogue System: Required flags not met for sequence '%s'"), *SequenceID);
        return false;
    }

    CurrentSequenceID = SequenceID;
    CurrentLineIndex = 0;
    bDialogueActive = true;

    UE_LOG(LogTemp, Log, TEXT("Dialogue System: Started sequence '%s'"), *SequenceID);
    return true;
}

void UNarr_DialogueSystem::AdvanceDialogue()
{
    if (!bDialogueActive || CurrentSequenceID.IsEmpty())
    {
        return;
    }

    if (!DialogueDatabase.Contains(CurrentSequenceID))
    {
        EndDialogue();
        return;
    }

    const FNarr_DialogueSequence& Sequence = DialogueDatabase[CurrentSequenceID];
    
    if (CurrentLineIndex >= Sequence.DialogueLines.Num() - 1)
    {
        EndDialogue();
        return;
    }

    CurrentLineIndex++;
    UE_LOG(LogTemp, Log, TEXT("Dialogue System: Advanced to line %d"), CurrentLineIndex);
}

void UNarr_DialogueSystem::EndDialogue()
{
    bDialogueActive = false;
    CurrentSequenceID = TEXT("");
    CurrentLineIndex = 0;
    UE_LOG(LogTemp, Log, TEXT("Dialogue System: Dialogue ended"));
}

FNarr_DialogueLine UNarr_DialogueSystem::GetCurrentLine() const
{
    if (!bDialogueActive || CurrentSequenceID.IsEmpty())
    {
        return FNarr_DialogueLine();
    }

    if (!DialogueDatabase.Contains(CurrentSequenceID))
    {
        return FNarr_DialogueLine();
    }

    const FNarr_DialogueSequence& Sequence = DialogueDatabase[CurrentSequenceID];
    
    if (CurrentLineIndex >= 0 && CurrentLineIndex < Sequence.DialogueLines.Num())
    {
        return Sequence.DialogueLines[CurrentLineIndex];
    }

    return FNarr_DialogueLine();
}

bool UNarr_DialogueSystem::HasFlag(const FString& FlagName) const
{
    return StoryFlags.Contains(FlagName);
}

void UNarr_DialogueSystem::SetFlag(const FString& FlagName)
{
    if (!StoryFlags.Contains(FlagName))
    {
        StoryFlags.Add(FlagName);
        UE_LOG(LogTemp, Log, TEXT("Dialogue System: Set story flag '%s'"), *FlagName);
    }
}

void UNarr_DialogueSystem::LoadDialogueFromDataTable(UDataTable* DialogueTable)
{
    if (!DialogueTable)
    {
        UE_LOG(LogTemp, Error, TEXT("Dialogue System: Invalid DataTable provided"));
        return;
    }

    // Implementation for loading from DataTable would go here
    UE_LOG(LogTemp, Log, TEXT("Dialogue System: DataTable loading not yet implemented"));
}

void UNarr_DialogueSystem::RegisterDialogueSequence(const FString& SequenceID, const FNarr_DialogueSequence& Sequence)
{
    DialogueDatabase.Add(SequenceID, Sequence);
    UE_LOG(LogTemp, Log, TEXT("Dialogue System: Registered sequence '%s' with %d lines"), *SequenceID, Sequence.DialogueLines.Num());
}

bool UNarr_DialogueSystem::CheckRequiredFlags(const TArray<FString>& RequiredFlags) const
{
    for (const FString& Flag : RequiredFlags)
    {
        if (!HasFlag(Flag))
        {
            return false;
        }
    }
    return true;
}