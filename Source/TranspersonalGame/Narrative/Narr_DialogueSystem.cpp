#include "Narr_DialogueSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"

UNarr_DialogueSystem::UNarr_DialogueSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    CurrentSequenceID = TEXT("");
    CurrentLineIndex = 0;
    bIsDialogueActive = false;
    DialogueRange = 1500.0f;
}

void UNarr_DialogueSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeDefaultDialogues();
}

void UNarr_DialogueSystem::InitializeDefaultDialogues()
{
    // Danger Warning Sequences
    FNarr_DialogueSequence DangerSequence;
    DangerSequence.SequenceID = TEXT("DangerWarning");
    DangerSequence.bIsRepeatable = true;
    DangerSequence.CooldownTime = 30.0f;
    DangerSequence.TriggerType = EQuestType::Combat;

    FNarr_DialogueLine DangerLine1;
    DangerLine1.SpeakerName = TEXT("Survival Guide");
    DangerLine1.DialogueText = TEXT("Danger ahead, survivor. The massive T-Rex has caught your scent.");
    DangerLine1.Duration = 4.0f;
    DangerLine1.ThreatLevel = ESurvivalThreat::Extreme;

    FNarr_DialogueLine DangerLine2;
    DangerLine2.SpeakerName = TEXT("Survival Guide");
    DangerLine2.DialogueText = TEXT("Move slowly and avoid direct eye contact. Your survival depends on understanding predator behavior.");
    DangerLine2.Duration = 5.0f;
    DangerLine2.ThreatLevel = ESurvivalThreat::Extreme;

    DangerSequence.DialogueLines.Add(DangerLine1);
    DangerSequence.DialogueLines.Add(DangerLine2);
    DialogueSequences.Add(DangerSequence);

    // Exploration Discovery Sequence
    FNarr_DialogueSequence ExplorationSequence;
    ExplorationSequence.SequenceID = TEXT("BiomeDiscovery");
    ExplorationSequence.bIsRepeatable = false;
    ExplorationSequence.CooldownTime = 120.0f;
    ExplorationSequence.TriggerType = EQuestType::Exploration;

    FNarr_DialogueLine ExplorationLine1;
    ExplorationLine1.SpeakerName = TEXT("Nature Narrator");
    ExplorationLine1.DialogueText = TEXT("The ancient Brachiosaurus herds are migrating through the forest biome.");
    ExplorationLine1.Duration = 4.5f;
    ExplorationLine1.ThreatLevel = ESurvivalThreat::None;
    ExplorationLine1.RequiredBiome = EBiomeType::Forest;

    FNarr_DialogueLine ExplorationLine2;
    ExplorationLine2.SpeakerName = TEXT("Nature Narrator");
    ExplorationLine2.DialogueText = TEXT("Follow their trails to discover fresh water sources and safe shelter locations.");
    ExplorationLine2.Duration = 4.0f;
    ExplorationLine2.ThreatLevel = ESurvivalThreat::None;
    ExplorationLine2.RequiredBiome = EBiomeType::Forest;

    ExplorationSequence.DialogueLines.Add(ExplorationLine1);
    ExplorationSequence.DialogueLines.Add(ExplorationLine2);
    DialogueSequences.Add(ExplorationSequence);

    // Survival Tips Sequence
    FNarr_DialogueSequence SurvivalSequence;
    SurvivalSequence.SequenceID = TEXT("SurvivalTips");
    SurvivalSequence.bIsRepeatable = true;
    SurvivalSequence.CooldownTime = 90.0f;
    SurvivalSequence.TriggerType = EQuestType::Survival;

    FNarr_DialogueLine SurvivalLine1;
    SurvivalLine1.SpeakerName = TEXT("Elder Tracker");
    SurvivalLine1.DialogueText = TEXT("Watch the ground for dinosaur tracks. Fresh prints mean danger is near.");
    SurvivalLine1.Duration = 3.5f;
    SurvivalLine1.ThreatLevel = ESurvivalThreat::Moderate;

    SurvivalSequence.DialogueLines.Add(SurvivalLine1);
    DialogueSequences.Add(SurvivalSequence);
}

bool UNarr_DialogueSystem::StartDialogueSequence(const FString& SequenceID)
{
    if (!CanTriggerSequence(SequenceID) || bIsDialogueActive)
    {
        return false;
    }

    for (const FNarr_DialogueSequence& Sequence : DialogueSequences)
    {
        if (Sequence.SequenceID == SequenceID)
        {
            CurrentSequenceID = SequenceID;
            CurrentLineIndex = 0;
            bIsDialogueActive = true;
            
            // Set cooldown
            SequenceCooldowns.Add(SequenceID, Sequence.CooldownTime);
            
            UE_LOG(LogTemp, Log, TEXT("Started dialogue sequence: %s"), *SequenceID);
            return true;
        }
    }
    
    return false;
}

void UNarr_DialogueSystem::NextDialogueLine()
{
    if (!bIsDialogueActive || CurrentSequenceID.IsEmpty())
    {
        return;
    }

    for (const FNarr_DialogueSequence& Sequence : DialogueSequences)
    {
        if (Sequence.SequenceID == CurrentSequenceID)
        {
            CurrentLineIndex++;
            
            if (CurrentLineIndex >= Sequence.DialogueLines.Num())
            {
                EndDialogue();
            }
            else
            {
                PlayAudioForCurrentLine();
            }
            break;
        }
    }
}

void UNarr_DialogueSystem::EndDialogue()
{
    bIsDialogueActive = false;
    CurrentSequenceID = TEXT("");
    CurrentLineIndex = 0;
    
    UE_LOG(LogTemp, Log, TEXT("Dialogue sequence ended"));
}

FNarr_DialogueLine UNarr_DialogueSystem::GetCurrentDialogueLine() const
{
    if (!bIsDialogueActive || CurrentSequenceID.IsEmpty())
    {
        return FNarr_DialogueLine();
    }

    for (const FNarr_DialogueSequence& Sequence : DialogueSequences)
    {
        if (Sequence.SequenceID == CurrentSequenceID)
        {
            if (CurrentLineIndex < Sequence.DialogueLines.Num())
            {
                return Sequence.DialogueLines[CurrentLineIndex];
            }
            break;
        }
    }
    
    return FNarr_DialogueLine();
}

bool UNarr_DialogueSystem::CanTriggerSequence(const FString& SequenceID) const
{
    if (!IsPlayerInRange())
    {
        return false;
    }

    const float* CooldownTime = SequenceCooldowns.Find(SequenceID);
    if (CooldownTime && *CooldownTime > 0.0f)
    {
        return false;
    }

    return true;
}

void UNarr_DialogueSystem::TriggerContextualDialogue(ESurvivalThreat ThreatLevel, EBiomeType CurrentBiome)
{
    FString BestSequenceID = TEXT("");
    
    // Priority: Threat level first, then biome-specific
    if (ThreatLevel == ESurvivalThreat::Extreme)
    {
        BestSequenceID = TEXT("DangerWarning");
    }
    else if (CurrentBiome == EBiomeType::Forest)
    {
        BestSequenceID = TEXT("BiomeDiscovery");
    }
    else
    {
        BestSequenceID = TEXT("SurvivalTips");
    }
    
    if (CanTriggerSequence(BestSequenceID))
    {
        StartDialogueSequence(BestSequenceID);
        PlayAudioForCurrentLine();
    }
}

void UNarr_DialogueSystem::RegisterDialogueSequence(const FNarr_DialogueSequence& NewSequence)
{
    // Remove existing sequence with same ID
    DialogueSequences.RemoveAll([&](const FNarr_DialogueSequence& Sequence)
    {
        return Sequence.SequenceID == NewSequence.SequenceID;
    });
    
    DialogueSequences.Add(NewSequence);
    UE_LOG(LogTemp, Log, TEXT("Registered dialogue sequence: %s"), *NewSequence.SequenceID);
}

void UNarr_DialogueSystem::PlayAudioForCurrentLine()
{
    FNarr_DialogueLine CurrentLine = GetCurrentDialogueLine();
    
    if (!CurrentLine.AudioFilePath.IsEmpty())
    {
        // Audio playback would be implemented here
        UE_LOG(LogTemp, Log, TEXT("Playing audio: %s"), *CurrentLine.AudioFilePath);
    }
    
    // Display text in UI
    UE_LOG(LogTemp, Warning, TEXT("[%s]: %s"), *CurrentLine.SpeakerName, *CurrentLine.DialogueText);
}

TArray<FString> UNarr_DialogueSystem::GetAvailableSequences() const
{
    TArray<FString> AvailableSequences;
    
    for (const FNarr_DialogueSequence& Sequence : DialogueSequences)
    {
        if (CanTriggerSequence(Sequence.SequenceID))
        {
            AvailableSequences.Add(Sequence.SequenceID);
        }
    }
    
    return AvailableSequences;
}

void UNarr_DialogueSystem::SetDialogueRange(float NewRange)
{
    DialogueRange = FMath::Max(0.0f, NewRange);
}

bool UNarr_DialogueSystem::IsPlayerInRange() const
{
    if (!GetWorld())
    {
        return false;
    }

    APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
    if (!PlayerController || !PlayerController->GetPawn())
    {
        return false;
    }

    FVector PlayerLocation = PlayerController->GetPawn()->GetActorLocation();
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    
    float Distance = FVector::Dist(PlayerLocation, OwnerLocation);
    return Distance <= DialogueRange;
}

void UNarr_DialogueSystem::UpdateCooldowns(float DeltaTime)
{
    for (auto& CooldownPair : SequenceCooldowns)
    {
        if (CooldownPair.Value > 0.0f)
        {
            CooldownPair.Value -= DeltaTime;
            if (CooldownPair.Value <= 0.0f)
            {
                CooldownPair.Value = 0.0f;
            }
        }
    }
}

void UNarr_DialogueSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdateCooldowns(DeltaTime);
    
    // Auto-advance dialogue lines based on duration
    if (bIsDialogueActive)
    {
        FNarr_DialogueLine CurrentLine = GetCurrentDialogueLine();
        static float LineTimer = 0.0f;
        
        LineTimer += DeltaTime;
        if (LineTimer >= CurrentLine.Duration)
        {
            LineTimer = 0.0f;
            NextDialogueLine();
        }
    }
}