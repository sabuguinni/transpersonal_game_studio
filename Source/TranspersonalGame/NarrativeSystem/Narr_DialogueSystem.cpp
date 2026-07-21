#include "Narr_DialogueSystem.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"

void UNarr_DialogueSystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    bDialogueActive = false;
    CurrentSequenceID = TEXT("");
    CurrentLineIndex = 0;
    
    CreateDefaultDialogues();
    
    UE_LOG(LogTemp, Log, TEXT("Narrative Dialogue System initialized"));
}

void UNarr_DialogueSystem::Deinitialize()
{
    if (bDialogueActive)
    {
        StopCurrentDialogue();
    }
    
    DialogueDatabase.Empty();
    
    Super::Deinitialize();
}

void UNarr_DialogueSystem::PlayDialogue(const FString& SequenceID, AActor* Speaker)
{
    if (bDialogueActive)
    {
        StopCurrentDialogue();
    }
    
    if (!DialogueDatabase.Contains(SequenceID))
    {
        UE_LOG(LogTemp, Warning, TEXT("Dialogue sequence not found: %s"), *SequenceID);
        return;
    }
    
    CurrentSequenceID = SequenceID;
    CurrentLineIndex = 0;
    bDialogueActive = true;
    
    PlayNextLine();
}

void UNarr_DialogueSystem::RegisterDialogueSequence(const FNarr_DialogueSequence& Sequence)
{
    DialogueDatabase.Add(Sequence.SequenceID, Sequence);
    UE_LOG(LogTemp, Log, TEXT("Registered dialogue sequence: %s"), *Sequence.SequenceID);
}

FNarr_DialogueSequence UNarr_DialogueSystem::GetDialogueSequence(const FString& SequenceID)
{
    if (DialogueDatabase.Contains(SequenceID))
    {
        return DialogueDatabase[SequenceID];
    }
    
    return FNarr_DialogueSequence();
}

TArray<FString> UNarr_DialogueSystem::GetAvailableDialogues(ENarr_CharacterType CharacterType)
{
    TArray<FString> AvailableDialogues;
    
    for (const auto& DialoguePair : DialogueDatabase)
    {
        const FNarr_DialogueSequence& Sequence = DialoguePair.Value;
        if (Sequence.DialogueLines.Num() > 0 && 
            Sequence.DialogueLines[0].CharacterType == CharacterType)
        {
            AvailableDialogues.Add(DialoguePair.Key);
        }
    }
    
    return AvailableDialogues;
}

void UNarr_DialogueSystem::InitializePrehistoricDialogues()
{
    CreateDefaultDialogues();
}

void UNarr_DialogueSystem::StopCurrentDialogue()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(DialogueTimerHandle);
    }
    
    bDialogueActive = false;
    CurrentSequenceID = TEXT("");
    CurrentLineIndex = 0;
}

void UNarr_DialogueSystem::PlayNextLine()
{
    if (!bDialogueActive || !DialogueDatabase.Contains(CurrentSequenceID))
    {
        return;
    }
    
    const FNarr_DialogueSequence& Sequence = DialogueDatabase[CurrentSequenceID];
    
    if (CurrentLineIndex >= Sequence.DialogueLines.Num())
    {
        // Dialogue sequence complete
        bDialogueActive = false;
        CurrentSequenceID = TEXT("");
        CurrentLineIndex = 0;
        return;
    }
    
    const FNarr_DialogueLine& CurrentLine = Sequence.DialogueLines[CurrentLineIndex];
    
    // Display dialogue text (in a real implementation, this would update UI)
    UE_LOG(LogTemp, Log, TEXT("[%s]: %s"), *CurrentLine.SpeakerName, *CurrentLine.DialogueText.ToString());
    
    // Set timer for next line
    float LineDisplayTime = FMath::Max(CurrentLine.Duration, 3.0f);
    
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            DialogueTimerHandle,
            this,
            &UNarr_DialogueSystem::OnDialogueLineComplete,
            LineDisplayTime,
            false
        );
    }
}

void UNarr_DialogueSystem::OnDialogueLineComplete()
{
    CurrentLineIndex++;
    PlayNextLine();
}

void UNarr_DialogueSystem::CreateDefaultDialogues()
{
    // Ancient Storyteller sequence
    FNarr_DialogueSequence StorytellerSequence;
    StorytellerSequence.SequenceID = TEXT("AncientStoryteller_Intro");
    StorytellerSequence.bIsRepeatable = true;
    StorytellerSequence.Priority = 1;
    
    FNarr_DialogueLine StoryLine;
    StoryLine.SpeakerName = TEXT("Ancient Storyteller");
    StoryLine.DialogueText = FText::FromString(TEXT("The ancient storytellers speak of the time before memory, when the great beasts ruled the earth. Listen well, for these tales carry the wisdom of survival in the prehistoric world."));
    StoryLine.DialogueType = ENarr_DialogueType::Information;
    StoryLine.CharacterType = ENarr_CharacterType::TribalElder;
    StoryLine.Duration = 24.0f;
    
    StorytellerSequence.DialogueLines.Add(StoryLine);
    RegisterDialogueSequence(StorytellerSequence);
    
    // Tribal Scout warning sequence
    FNarr_DialogueSequence ScoutSequence;
    ScoutSequence.SequenceID = TEXT("TribalScout_DangerWarning");
    ScoutSequence.bIsRepeatable = true;
    ScoutSequence.Priority = 3;
    
    FNarr_DialogueLine WarningLine;
    WarningLine.SpeakerName = TEXT("Tribal Scout");
    WarningLine.DialogueText = FText::FromString(TEXT("Danger approaches from the eastern ridge! The ground trembles beneath the massive footsteps of the thunder-lizard. All tribe members must seek shelter in the caves immediately!"));
    WarningLine.DialogueType = ENarr_DialogueType::Warning;
    WarningLine.CharacterType = ENarr_CharacterType::Scout;
    WarningLine.Duration = 21.0f;
    
    ScoutSequence.DialogueLines.Add(WarningLine);
    RegisterDialogueSequence(ScoutSequence);
    
    // Hunt Chief sequence
    FNarr_DialogueSequence HuntSequence;
    HuntSequence.SequenceID = TEXT("HuntChief_PostBattle");
    HuntSequence.bIsRepeatable = false;
    HuntSequence.Priority = 2;
    
    FNarr_DialogueLine HuntLine;
    HuntLine.SpeakerName = TEXT("Hunt Chief");
    HuntLine.DialogueText = FText::FromString(TEXT("The hunt was successful, but at great cost. Three of our finest warriors fell to the razor-claws. We must honor their sacrifice and learn from their courage."));
    HuntLine.DialogueType = ENarr_DialogueType::Information;
    HuntLine.CharacterType = ENarr_CharacterType::HuntLeader;
    HuntLine.Duration = 20.0f;
    
    HuntSequence.DialogueLines.Add(HuntLine);
    RegisterDialogueSequence(HuntSequence);
    
    // Water Guide sequence
    FNarr_DialogueSequence WaterSequence;
    WaterSequence.SequenceID = TEXT("WaterGuide_ResourceInfo");
    WaterSequence.bIsRepeatable = true;
    WaterSequence.Priority = 2;
    
    FNarr_DialogueLine WaterLine;
    WaterLine.SpeakerName = TEXT("Water Guide");
    WaterLine.DialogueText = FText::FromString(TEXT("Fresh water flows beyond the thorn valley, but beware - the territory is claimed by the long-necks. Move swiftly and silently if you wish to return alive."));
    WaterLine.DialogueType = ENarr_DialogueType::Information;
    WaterLine.CharacterType = ENarr_CharacterType::Scout;
    WaterLine.Duration = 20.0f;
    
    WaterSequence.DialogueLines.Add(WaterLine);
    RegisterDialogueSequence(WaterSequence);
    
    UE_LOG(LogTemp, Log, TEXT("Created %d default dialogue sequences"), DialogueDatabase.Num());
}