#include "NarrativeManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "Kismet/GameplayStatics.h"
#include "../Character/TranspersonalCharacter.h"
#include "../TranspersonalGameState.h"

UNarrativeManager::UNarrativeManager()
{
    bDialogueActive = false;
    CurrentDialogueID = TEXT("");
    CurrentLineIndex = 0;
    DialogueTimer = 0.0f;
    NarrationAudioComponent = nullptr;
}

void UNarrativeManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("NarrativeManager: Initializing narrative system"));
    
    InitializeStoryCheckpoints();
    LoadQuestDialogues();
    
    // Create audio component for narration
    if (UWorld* World = GetWorld())
    {
        if (AActor* WorldActor = World->GetFirstPlayerController())
        {
            NarrationAudioComponent = WorldActor->CreateDefaultSubobject<UAudioComponent>(TEXT("NarrationAudio"));
            if (NarrationAudioComponent)
            {
                NarrationAudioComponent->bAutoActivate = false;
                UE_LOG(LogTemp, Warning, TEXT("NarrativeManager: Audio component created"));
            }
        }
    }
}

void UNarrativeManager::Deinitialize()
{
    EndCurrentDialogue();
    
    if (NarrationAudioComponent)
    {
        NarrationAudioComponent->Stop();
        NarrationAudioComponent = nullptr;
    }
    
    Super::Deinitialize();
}

void UNarrativeManager::StartDialogue(const FString& DialogueID, ATranspersonalCharacter* Player)
{
    if (bDialogueActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("NarrativeManager: Cannot start dialogue - another dialogue is active"));
        return;
    }

    if (!QuestDialogues.Contains(DialogueID))
    {
        UE_LOG(LogTemp, Error, TEXT("NarrativeManager: Dialogue ID not found: %s"), *DialogueID);
        return;
    }

    const FNarr_QuestDialogue& QuestDialogue = QuestDialogues[DialogueID];
    
    bDialogueActive = true;
    CurrentDialogueID = DialogueID;
    CurrentLineIndex = 0;
    DialogueTimer = 0.0f;
    
    // Start with intro lines
    CurrentDialogueLines = QuestDialogue.IntroLines;
    
    if (CurrentDialogueLines.Num() > 0)
    {
        PlayDialogueLine(CurrentDialogueLines[0]);
        UE_LOG(LogTemp, Warning, TEXT("NarrativeManager: Started dialogue: %s"), *DialogueID);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("NarrativeManager: No dialogue lines found for: %s"), *DialogueID);
        EndCurrentDialogue();
    }
}

void UNarrativeManager::TriggerNarration(const FString& CheckpointID)
{
    for (const FNarr_StoryCheckpoint& Checkpoint : StoryCheckpoints)
    {
        if (Checkpoint.CheckpointID == CheckpointID)
        {
            TriggerCheckpointNarration(Checkpoint);
            MarkCheckpointReached(CheckpointID);
            break;
        }
    }
}

void UNarrativeManager::RegisterQuestDialogue(const FString& QuestID, const FNarr_QuestDialogue& DialogueData)
{
    QuestDialogues.Add(QuestID, DialogueData);
    UE_LOG(LogTemp, Warning, TEXT("NarrativeManager: Registered dialogue for quest: %s"), *QuestID);
}

bool UNarrativeManager::IsStoryCheckpointReached(const FString& CheckpointID) const
{
    return ReachedCheckpoints.Contains(CheckpointID);
}

void UNarrativeManager::MarkCheckpointReached(const FString& CheckpointID)
{
    if (!ReachedCheckpoints.Contains(CheckpointID))
    {
        ReachedCheckpoints.Add(CheckpointID);
        UE_LOG(LogTemp, Warning, TEXT("NarrativeManager: Checkpoint reached: %s"), *CheckpointID);
    }
}

void UNarrativeManager::PlayDialogueLine(const FNarr_DialogueLine& DialogueLine)
{
    // Display text (in a real implementation, this would update UI)
    UE_LOG(LogTemp, Warning, TEXT("NarrativeManager: %s: %s"), *DialogueLine.SpeakerName, *DialogueLine.DialogueText.ToString());
    
    // Play voice clip if available
    if (DialogueLine.VoiceClip.IsValid() && NarrationAudioComponent)
    {
        if (USoundBase* Sound = DialogueLine.VoiceClip.LoadSynchronous())
        {
            NarrationAudioComponent->SetSound(Sound);
            NarrationAudioComponent->Play();
        }
    }
    
    // Set timer for next line
    DialogueTimer = DialogueLine.Duration;
}

void UNarrativeManager::EndCurrentDialogue()
{
    if (!bDialogueActive)
        return;
        
    bDialogueActive = false;
    CurrentDialogueID = TEXT("");
    CurrentLineIndex = 0;
    DialogueTimer = 0.0f;
    CurrentDialogueLines.Empty();
    
    if (NarrationAudioComponent)
    {
        NarrationAudioComponent->Stop();
    }
    
    UE_LOG(LogTemp, Warning, TEXT("NarrativeManager: Dialogue ended"));
}

void UNarrativeManager::CheckLocationTriggers(const FVector& PlayerLocation)
{
    for (const FNarr_StoryCheckpoint& Checkpoint : StoryCheckpoints)
    {
        if (Checkpoint.bTriggerAutomatically && 
            !IsStoryCheckpointReached(Checkpoint.CheckpointID) &&
            IsPlayerNearCheckpoint(Checkpoint, PlayerLocation))
        {
            TriggerNarration(Checkpoint.CheckpointID);
        }
    }
}

float UNarrativeManager::GetStoryProgress() const
{
    if (StoryCheckpoints.Num() == 0)
        return 0.0f;
        
    return static_cast<float>(ReachedCheckpoints.Num()) / static_cast<float>(StoryCheckpoints.Num());
}

void UNarrativeManager::InitializeStoryCheckpoints()
{
    // Initialize basic story checkpoints
    FNarr_StoryCheckpoint IntroCheckpoint;
    IntroCheckpoint.CheckpointID = TEXT("Intro_001");
    IntroCheckpoint.NarrativeText = FText::FromString(TEXT("Day 233 in the Cretaceous wilderness. The morning reveals disturbing signs..."));
    IntroCheckpoint.TriggerLocation = FVector(0, 0, 100);
    IntroCheckpoint.TriggerRadius = 1000.0f;
    IntroCheckpoint.bTriggerAutomatically = true;
    StoryCheckpoints.Add(IntroCheckpoint);
    
    FNarr_StoryCheckpoint ThreatCheckpoint;
    ThreatCheckpoint.CheckpointID = TEXT("Threat_001");
    ThreatCheckpoint.NarrativeText = FText::FromString(TEXT("Critical threat assessment! Massive Carnotaurus pack detected..."));
    ThreatCheckpoint.TriggerLocation = FVector(2000, 0, 100);
    ThreatCheckpoint.TriggerRadius = 800.0f;
    ThreatCheckpoint.bTriggerAutomatically = true;
    StoryCheckpoints.Add(ThreatCheckpoint);
    
    FNarr_StoryCheckpoint GuideCheckpoint;
    GuideCheckpoint.CheckpointID = TEXT("Guide_001");
    GuideCheckpoint.NarrativeText = FText::FromString(TEXT("The ancient river crossing holds secrets of survival..."));
    GuideCheckpoint.TriggerLocation = FVector(1000, 1500, 50);
    GuideCheckpoint.TriggerRadius = 600.0f;
    GuideCheckpoint.bTriggerAutomatically = true;
    StoryCheckpoints.Add(GuideCheckpoint);
    
    UE_LOG(LogTemp, Warning, TEXT("NarrativeManager: Initialized %d story checkpoints"), StoryCheckpoints.Num());
}

void UNarrativeManager::LoadQuestDialogues()
{
    // Create sample quest dialogue
    FNarr_QuestDialogue SurvivalQuest;
    SurvivalQuest.QuestID = TEXT("Survival_001");
    
    // Intro lines
    FNarr_DialogueLine IntroLine1;
    IntroLine1.SpeakerName = TEXT("Survival Guide");
    IntroLine1.DialogueText = FText::FromString(TEXT("Welcome to the Cretaceous period. Your survival depends on understanding this dangerous world."));
    IntroLine1.Duration = 4.0f;
    SurvivalQuest.IntroLines.Add(IntroLine1);
    
    FNarr_DialogueLine IntroLine2;
    IntroLine2.SpeakerName = TEXT("Survival Guide");
    IntroLine2.DialogueText = FText::FromString(TEXT("Find shelter, gather resources, and avoid the apex predators. Your journey begins now."));
    IntroLine2.Duration = 4.0f;
    SurvivalQuest.IntroLines.Add(IntroLine2);
    
    // Progress lines
    FNarr_DialogueLine ProgressLine;
    ProgressLine.SpeakerName = TEXT("Survival Guide");
    ProgressLine.DialogueText = FText::FromString(TEXT("Good progress. You're learning to read the signs of this ancient world."));
    ProgressLine.Duration = 3.0f;
    SurvivalQuest.ProgressLines.Add(ProgressLine);
    
    // Completion lines
    FNarr_DialogueLine CompletionLine;
    CompletionLine.SpeakerName = TEXT("Survival Guide");
    CompletionLine.DialogueText = FText::FromString(TEXT("Excellent work, survivor. You've taken your first steps toward mastering this prehistoric realm."));
    CompletionLine.Duration = 4.0f;
    SurvivalQuest.CompletionLines.Add(CompletionLine);
    
    RegisterQuestDialogue(TEXT("Survival_001"), SurvivalQuest);
}

void UNarrativeManager::UpdateDialogueTimer(float DeltaTime)
{
    if (!bDialogueActive)
        return;
        
    DialogueTimer -= DeltaTime;
    
    if (DialogueTimer <= 0.0f)
    {
        ProcessNextDialogueLine();
    }
}

void UNarrativeManager::ProcessNextDialogueLine()
{
    CurrentLineIndex++;
    
    if (CurrentLineIndex >= CurrentDialogueLines.Num())
    {
        EndCurrentDialogue();
        return;
    }
    
    PlayDialogueLine(CurrentDialogueLines[CurrentLineIndex]);
}

bool UNarrativeManager::IsPlayerNearCheckpoint(const FNarr_StoryCheckpoint& Checkpoint, const FVector& PlayerLocation) const
{
    float Distance = FVector::Dist(PlayerLocation, Checkpoint.TriggerLocation);
    return Distance <= Checkpoint.TriggerRadius;
}

void UNarrativeManager::TriggerCheckpointNarration(const FNarr_StoryCheckpoint& Checkpoint)
{
    UE_LOG(LogTemp, Warning, TEXT("NarrativeManager: Triggered checkpoint: %s"), *Checkpoint.CheckpointID);
    UE_LOG(LogTemp, Warning, TEXT("Narration: %s"), *Checkpoint.NarrativeText.ToString());
    
    // Play narration audio if available
    if (Checkpoint.NarrationClip.IsValid() && NarrationAudioComponent)
    {
        if (USoundBase* Sound = Checkpoint.NarrationClip.LoadSynchronous())
        {
            NarrationAudioComponent->SetSound(Sound);
            NarrationAudioComponent->Play();
        }
    }
}