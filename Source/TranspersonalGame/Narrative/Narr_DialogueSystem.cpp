#include "Narr_DialogueSystem.h"
#include "Engine/World.h"
#include "TimerManager.h"

UNarr_DialogueSystem::UNarr_DialogueSystem()
{
    PrimaryComponentTick.bCanEverTick = false;
    DefaultDialogueDuration = 4.0f;
    bAutoAdvanceDialogue = true;
    bDialogueActive = false;
    CurrentDialogueIndex = 0;
    CurrentSpeaker = TEXT("");
    CurrentDialogue = TEXT("");
    ActiveSequenceID = TEXT("");
}

void UNarr_DialogueSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Register default dialogue sequences
    RegisterTribalDialogues();
    RegisterSurvivalDialogues();
    RegisterHuntDialogues();
}

void UNarr_DialogueSystem::StartDialogueSequence(const FString& SequenceID)
{
    if (bDialogueActive)
    {
        EndDialogue();
    }

    // Find the requested sequence
    FNarr_DialogueSequence* FoundSequence = nullptr;
    for (FNarr_DialogueSequence& Sequence : DialogueSequences)
    {
        if (Sequence.SequenceID == SequenceID)
        {
            FoundSequence = &Sequence;
            break;
        }
    }

    if (!FoundSequence || FoundSequence->DialogueEntries.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Dialogue sequence not found or empty: %s"), *SequenceID);
        return;
    }

    ActiveSequenceID = SequenceID;
    CurrentDialogueIndex = 0;
    bDialogueActive = true;
    
    ProcessCurrentDialogue();
}

void UNarr_DialogueSystem::AdvanceDialogue()
{
    if (!bDialogueActive)
    {
        return;
    }

    CurrentDialogueIndex++;
    
    FNarr_DialogueSequence* ActiveSequence = nullptr;
    for (FNarr_DialogueSequence& Sequence : DialogueSequences)
    {
        if (Sequence.SequenceID == ActiveSequenceID)
        {
            ActiveSequence = &Sequence;
            break;
        }
    }

    if (!ActiveSequence || CurrentDialogueIndex >= ActiveSequence->DialogueEntries.Num())
    {
        EndDialogue();
        return;
    }

    ProcessCurrentDialogue();
}

void UNarr_DialogueSystem::EndDialogue()
{
    bDialogueActive = false;
    CurrentSpeaker = TEXT("");
    CurrentDialogue = TEXT("");
    ActiveSequenceID = TEXT("");
    CurrentDialogueIndex = 0;
    
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(DialogueTimerHandle);
    }
}

void UNarr_DialogueSystem::AddDialogueSequence(const FNarr_DialogueSequence& NewSequence)
{
    DialogueSequences.Add(NewSequence);
}

void UNarr_DialogueSystem::ProcessCurrentDialogue()
{
    FNarr_DialogueSequence* ActiveSequence = nullptr;
    for (FNarr_DialogueSequence& Sequence : DialogueSequences)
    {
        if (Sequence.SequenceID == ActiveSequenceID)
        {
            ActiveSequence = &Sequence;
            break;
        }
    }

    if (!ActiveSequence || CurrentDialogueIndex >= ActiveSequence->DialogueEntries.Num())
    {
        EndDialogue();
        return;
    }

    const FNarr_DialogueEntry& CurrentEntry = ActiveSequence->DialogueEntries[CurrentDialogueIndex];
    CurrentSpeaker = CurrentEntry.SpeakerName;
    CurrentDialogue = CurrentEntry.DialogueText;

    // Auto-advance if enabled and not a player choice
    if (bAutoAdvanceDialogue && !CurrentEntry.bIsPlayerChoice && GetWorld())
    {
        float Duration = CurrentEntry.Duration > 0 ? CurrentEntry.Duration : DefaultDialogueDuration;
        GetWorld()->GetTimerManager().SetTimer(DialogueTimerHandle, this, &UNarr_DialogueSystem::AutoAdvanceToNext, Duration, false);
    }
}

void UNarr_DialogueSystem::AutoAdvanceToNext()
{
    AdvanceDialogue();
}

void UNarr_DialogueSystem::RegisterTribalDialogues()
{
    // Elder Gatherer dialogue sequence
    FNarr_DialogueSequence ElderSequence;
    ElderSequence.SequenceID = TEXT("ElderGatherer_Introduction");
    ElderSequence.bRepeatable = true;
    ElderSequence.RequiredQuestStage = 0;

    FNarr_DialogueEntry Entry1;
    Entry1.SpeakerName = TEXT("Elder Gatherer");
    Entry1.DialogueText = TEXT("Greetings, traveler. I am Elder Gatherer of the valley tribe. The berries in our sacred grove have become scarce.");
    Entry1.Duration = 5.0f;
    ElderSequence.DialogueEntries.Add(Entry1);

    FNarr_DialogueEntry Entry2;
    Entry2.SpeakerName = TEXT("Elder Gatherer");
    Entry2.DialogueText = TEXT("The great beasts have been feeding there. We need a skilled hunter to drive them away safely.");
    Entry2.Duration = 4.0f;
    ElderSequence.DialogueEntries.Add(Entry2);

    DialogueSequences.Add(ElderSequence);

    // Lost Hunter dialogue sequence
    FNarr_DialogueSequence LostHunterSequence;
    LostHunterSequence.SequenceID = TEXT("LostHunter_Plea");
    LostHunterSequence.bRepeatable = false;
    LostHunterSequence.RequiredQuestStage = 0;

    FNarr_DialogueEntry Entry3;
    Entry3.SpeakerName = TEXT("Lost Hunter");
    Entry3.DialogueText = TEXT("Help me, please! I am Lost Hunter, separated from my tribe during the great migration.");
    Entry3.Duration = 4.0f;
    LostHunterSequence.DialogueEntries.Add(Entry3);

    FNarr_DialogueEntry Entry4;
    Entry4.SpeakerName = TEXT("Lost Hunter");
    Entry4.DialogueText = TEXT("The path through the hills is treacherous. I fear the pack-hunters have caught my scent.");
    Entry4.Duration = 4.0f;
    LostHunterSequence.DialogueEntries.Add(Entry4);

    DialogueSequences.Add(LostHunterSequence);
}

void UNarr_DialogueSystem::RegisterSurvivalDialogues()
{
    // Shelter Builder dialogue
    FNarr_DialogueSequence ShelterSequence;
    ShelterSequence.SequenceID = TEXT("ShelterBuilder_Advice");
    ShelterSequence.bRepeatable = true;
    ShelterSequence.RequiredQuestStage = 0;

    FNarr_DialogueEntry Entry1;
    Entry1.SpeakerName = TEXT("Shelter Builder");
    Entry1.DialogueText = TEXT("The storms come without warning in these lands. A wise hunter always prepares shelter before nightfall.");
    Entry1.Duration = 5.0f;
    ShelterSequence.DialogueEntries.Add(Entry1);

    FNarr_DialogueEntry Entry2;
    Entry2.SpeakerName = TEXT("Shelter Builder");
    Entry2.DialogueText = TEXT("Look for caves or overhanging rocks. Build walls with stones and branches. Fire keeps the night predators away.");
    Entry2.Duration = 6.0f;
    ShelterSequence.DialogueEntries.Add(Entry2);

    DialogueSequences.Add(ShelterSequence);
}

void UNarr_DialogueSystem::RegisterHuntDialogues()
{
    // Hunt Master dialogue
    FNarr_DialogueSequence HuntSequence;
    HuntSequence.SequenceID = TEXT("HuntMaster_Training");
    HuntSequence.bRepeatable = true;
    HuntSequence.RequiredQuestStage = 0;

    FNarr_DialogueEntry Entry1;
    Entry1.SpeakerName = TEXT("Hunt Master");
    Entry1.DialogueText = TEXT("The great hunt begins at dawn. Listen well, young hunter. The Velociraptors hunt in coordinated packs.");
    Entry1.Duration = 5.0f;
    HuntSequence.DialogueEntries.Add(Entry1);

    FNarr_DialogueEntry Entry2;
    Entry2.SpeakerName = TEXT("Hunt Master");
    Entry2.DialogueText = TEXT("Never face them alone. Use terrain to your advantage. High ground and narrow passages break their formation.");
    Entry2.Duration = 5.0f;
    HuntSequence.DialogueEntries.Add(Entry2);

    FNarr_DialogueEntry Entry3;
    Entry3.SpeakerName = TEXT("Hunt Master");
    Entry3.DialogueText = TEXT("The Tyrannosaurus Rex is a different challenge entirely. Its roar alone can paralyze prey. Stay mobile, strike from distance.");
    Entry3.Duration = 6.0f;
    HuntSequence.DialogueEntries.Add(Entry3);

    DialogueSequences.Add(HuntSequence);
}