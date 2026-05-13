#include "Narr_DialogueManager.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"

UNarr_DialogueManager::UNarr_DialogueManager()
{
    bIsDialoguePlaying = false;
    CurrentSequenceID = TEXT("");
    CurrentLineIndex = 0;
    GlobalDialogueVolume = 1.0f;
    DefaultCooldownTime = 5.0f;
}

void UNarr_DialogueManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("Narrative Dialogue Manager initialized"));
    
    // Initialize default dialogue sequences for prehistoric survival
    InitializeDefaultDialogues();
}

void UNarr_DialogueManager::InitializeDefaultDialogues()
{
    // Tribal Elder Wisdom Sequence
    FNarr_DialogueSequence ElderWisdom;
    ElderWisdom.SequenceID = TEXT("TribalElder_Wisdom");
    ElderWisdom.bLooping = false;
    ElderWisdom.SequenceCooldown = 60.0f;
    
    FNarr_DialogueLine ElderLine1;
    ElderLine1.SpeakerName = TEXT("Tribal Elder");
    ElderLine1.DialogueText = FText::FromString(TEXT("Listen carefully, young survivor. The elders speak of ancient wisdom passed down through countless seasons."));
    ElderLine1.DialogueType = ENarr_DialogueType::Tribal_Elder;
    ElderLine1.TriggerType = ENarr_DialogueTrigger::Player_Approach;
    ElderLine1.Priority = 8.0f;
    ElderLine1.CooldownTime = 30.0f;
    ElderLine1.bCanInterrupt = false;
    
    ElderWisdom.DialogueLines.Add(ElderLine1);
    DialogueSequences.Add(ElderWisdom.SequenceID, ElderWisdom);
    
    // Hunt Leader Warning Sequence
    FNarr_DialogueSequence HuntWarning;
    HuntWarning.SequenceID = TEXT("HuntLeader_RaptorWarning");
    HuntWarning.bLooping = false;
    HuntWarning.SequenceCooldown = 45.0f;
    
    FNarr_DialogueLine HuntLine1;
    HuntLine1.SpeakerName = TEXT("Hunt Leader");
    HuntLine1.DialogueText = FText::FromString(TEXT("The pack moves as one through the dense undergrowth. Three raptors, coordinated and deadly."));
    HuntLine1.DialogueType = ENarr_DialogueType::Hunt_Leader;
    HuntLine1.TriggerType = ENarr_DialogueTrigger::Dinosaur_Spotted;
    HuntLine1.Priority = 9.0f;
    HuntLine1.CooldownTime = 20.0f;
    HuntLine1.bCanInterrupt = true;
    
    HuntWarning.DialogueLines.Add(HuntLine1);
    DialogueSequences.Add(HuntWarning.SequenceID, HuntWarning);
    
    // War Chief Battle Cry Sequence
    FNarr_DialogueSequence WarCry;
    WarCry.SequenceID = TEXT("WarChief_BattleCry");
    WarCry.bLooping = false;
    WarCry.SequenceCooldown = 120.0f;
    
    FNarr_DialogueLine WarLine1;
    WarLine1.SpeakerName = TEXT("War Chief");
    WarLine1.DialogueText = FText::FromString(TEXT("The great thunder-foot approaches! All hunters to defensive positions!"));
    WarLine1.DialogueType = ENarr_DialogueType::War_Chief;
    WarLine1.TriggerType = ENarr_DialogueTrigger::Danger_Detected;
    WarLine1.Priority = 10.0f;
    WarLine1.CooldownTime = 60.0f;
    WarLine1.bCanInterrupt = false;
    
    WarCry.DialogueLines.Add(WarLine1);
    DialogueSequences.Add(WarCry.SequenceID, WarCry);
    
    // Water Seeker Resource Guide Sequence
    FNarr_DialogueSequence WaterGuide;
    WaterGuide.SequenceID = TEXT("WaterSeeker_ResourceGuide");
    WaterGuide.bLooping = false;
    WaterGuide.SequenceCooldown = 90.0f;
    
    FNarr_DialogueLine WaterLine1;
    WaterLine1.SpeakerName = TEXT("Water Seeker");
    WaterLine1.DialogueText = FText::FromString(TEXT("Water flows from the sacred spring beyond the bone valley. But beware - the territory belongs to the spiked-backs."));
    WaterLine1.DialogueType = ENarr_DialogueType::Water_Seeker;
    WaterLine1.TriggerType = ENarr_DialogueTrigger::Resource_Found;
    WaterLine1.Priority = 7.0f;
    WaterLine1.CooldownTime = 40.0f;
    WaterLine1.bCanInterrupt = true;
    
    WaterGuide.DialogueLines.Add(WaterLine1);
    DialogueSequences.Add(WaterGuide.SequenceID, WaterGuide);
}

void UNarr_DialogueManager::TriggerDialogue(ENarr_DialogueTrigger TriggerType, AActor* TriggeringActor, const FString& ContextData)
{
    if (!CanTriggerDialogue(TriggerType))
    {
        return;
    }
    
    // Find highest priority dialogue for this trigger type
    FNarr_DialogueLine* BestDialogue = nullptr;
    float HighestPriority = 0.0f;
    
    for (auto& SequencePair : DialogueSequences)
    {
        FNarr_DialogueSequence& Sequence = SequencePair.Value;
        for (FNarr_DialogueLine& Line : Sequence.DialogueLines)
        {
            if (Line.TriggerType == TriggerType && Line.Priority > HighestPriority)
            {
                BestDialogue = &Line;
                HighestPriority = Line.Priority;
            }
        }
    }
    
    if (BestDialogue)
    {
        // Add to queue or play immediately
        if (!bIsDialoguePlaying || BestDialogue->bCanInterrupt)
        {
            if (BestDialogue->bCanInterrupt && bIsDialoguePlaying)
            {
                StopCurrentDialogue();
            }
            
            DialogueQueue.Insert(*BestDialogue, 0);
            ProcessDialogueQueue();
        }
        else
        {
            DialogueQueue.Add(*BestDialogue);
        }
        
        // Set cooldown for this trigger type
        TriggerCooldowns.Add(TriggerType, BestDialogue->CooldownTime);
        
        // Broadcast event
        OnDialogueTriggered.Broadcast(*BestDialogue, TriggeringActor);
    }
}

void UNarr_DialogueManager::PlayDialogueSequence(const FString& SequenceID, AActor* TargetActor)
{
    if (DialogueSequences.Contains(SequenceID))
    {
        const FNarr_DialogueSequence& Sequence = DialogueSequences[SequenceID];
        
        if (!bIsDialoguePlaying)
        {
            CurrentSequenceID = SequenceID;
            CurrentLineIndex = 0;
            
            // Add all lines to queue
            for (const FNarr_DialogueLine& Line : Sequence.DialogueLines)
            {
                DialogueQueue.Add(Line);
            }
            
            ProcessDialogueQueue();
        }
    }
}

void UNarr_DialogueManager::RegisterDialogueSequence(const FNarr_DialogueSequence& NewSequence)
{
    DialogueSequences.Add(NewSequence.SequenceID, NewSequence);
    UE_LOG(LogTemp, Log, TEXT("Registered dialogue sequence: %s"), *NewSequence.SequenceID);
}

void UNarr_DialogueManager::StopCurrentDialogue()
{
    bIsDialoguePlaying = false;
    CurrentSequenceID = TEXT("");
    CurrentLineIndex = 0;
    DialogueQueue.Empty();
    
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(DialogueTimerHandle);
    }
}

bool UNarr_DialogueManager::IsDialoguePlaying() const
{
    return bIsDialoguePlaying;
}

void UNarr_DialogueManager::SetDialogueVolume(float Volume)
{
    GlobalDialogueVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
}

void UNarr_DialogueManager::ProcessDialogueQueue()
{
    if (DialogueQueue.Num() > 0 && !bIsDialoguePlaying)
    {
        bIsDialoguePlaying = true;
        PlayNextDialogueLine();
    }
}

void UNarr_DialogueManager::PlayNextDialogueLine()
{
    if (DialogueQueue.Num() == 0)
    {
        bIsDialoguePlaying = false;
        OnDialogueCompleted.Broadcast(CurrentSequenceID);
        return;
    }
    
    FNarr_DialogueLine CurrentLine = DialogueQueue[0];
    DialogueQueue.RemoveAt(0);
    
    UE_LOG(LogTemp, Log, TEXT("Playing dialogue: %s - %s"), 
           *CurrentLine.SpeakerName, 
           *CurrentLine.DialogueText.ToString());
    
    // Play audio if available
    if (CurrentLine.VoiceAudio.IsValid())
    {
        if (UWorld* World = GetWorld())
        {
            UGameplayStatics::PlaySound2D(World, CurrentLine.VoiceAudio.Get(), GlobalDialogueVolume);
        }
    }
    
    // Set timer for next line (estimate 3 seconds per line + audio duration)
    float LineDisplayTime = 3.0f + (CurrentLine.DialogueText.ToString().Len() * 0.05f);
    
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(DialogueTimerHandle, 
                                        this, 
                                        &UNarr_DialogueManager::PlayNextDialogueLine, 
                                        LineDisplayTime, 
                                        false);
    }
}

bool UNarr_DialogueManager::CanTriggerDialogue(ENarr_DialogueTrigger TriggerType) const
{
    if (TriggerCooldowns.Contains(TriggerType))
    {
        // Check if cooldown has expired
        float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
        float LastTriggerTime = TriggerCooldowns[TriggerType];
        return (CurrentTime - LastTriggerTime) > DefaultCooldownTime;
    }
    
    return true;
}

void UNarr_DialogueManager::UpdateCooldowns(float DeltaTime)
{
    // This would be called from a tick function if needed
    // For now, cooldowns are managed through timer-based checks
}