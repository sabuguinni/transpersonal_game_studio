#include "DialogueComponent.h"
#include "NarrativeManager.h"
#include "Components/AudioComponent.h"
#include "Engine/DataTable.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

UDialogueComponent::UDialogueComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    bIsDialogueActive = false;
    CharacterArchetype = ENarr_CharacterArchetype::Elder;
    DefaultSpeakerName = TEXT("Unknown Speaker");
    
    // Create audio component
    AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
    if (AudioComponent)
    {
        AudioComponent->bAutoActivate = false;
        AudioComponent->SetVolumeMultiplier(1.0f);
    }
}

void UDialogueComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Get reference to narrative manager
    if (UWorld* World = GetWorld())
    {
        if (UGameInstance* GameInstance = World->GetGameInstance())
        {
            NarrativeManager = GameInstance->GetSubsystem<UNarrativeManager>();
        }
    }
}

bool UDialogueComponent::StartDialogue(const FString& DialogueID, AActor* InteractingActor)
{
    if (bIsDialogueActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("Dialogue already active, ending current dialogue first"));
        EndDialogue();
    }
    
    if (DialogueID.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot start dialogue with empty DialogueID"));
        return false;
    }
    
    // Load the dialogue line
    if (!LoadDialogueLine(DialogueID))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load dialogue line: %s"), *DialogueID);
        return false;
    }
    
    // Check consciousness level requirements
    if (!CanAccessDialogueLine(CurrentDialogueLine))
    {
        UE_LOG(LogTemp, Warning, TEXT("Player consciousness level too low for dialogue: %s"), *DialogueID);
        return false;
    }
    
    bIsDialogueActive = true;
    CurrentDialogueID = DialogueID;
    
    // Update dialogue choices
    UpdateDialogueChoices();
    
    // Broadcast events
    OnDialogueStarted.Broadcast(DialogueID, CurrentDialogueLine.SpeakerName);
    OnDialogueLineChanged.Broadcast(CurrentDialogueLine.DialogueText, CurrentDialogueLine.SpeakerName, CurrentDialogueLine.EmotionalState);
    
    // Play audio if available
    PlayDialogueAudio();
    
    // Register with narrative manager
    if (NarrativeManager)
    {
        NarrativeManager->RegisterActiveDialogue(this);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Started dialogue: %s"), *DialogueID);
    return true;
}

void UDialogueComponent::EndDialogue()
{
    if (!bIsDialogueActive)
    {
        return;
    }
    
    StopDialogueAudio();
    
    FString EndedDialogueID = CurrentDialogueID;
    
    bIsDialogueActive = false;
    CurrentDialogueID.Empty();
    CurrentChoices.Empty();
    CurrentDialogueLine = FNarr_DialogueLine();
    
    // Unregister from narrative manager
    if (NarrativeManager)
    {
        NarrativeManager->UnregisterActiveDialogue(this);
    }
    
    OnDialogueEnded.Broadcast(EndedDialogueID);
    
    UE_LOG(LogTemp, Log, TEXT("Ended dialogue: %s"), *EndedDialogueID);
}

bool UDialogueComponent::AdvanceDialogue()
{
    if (!bIsDialogueActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot advance dialogue - no active dialogue"));
        return false;
    }
    
    // Check if there's a next dialogue line
    if (CurrentDialogueLine.NextDialogueID.IsEmpty())
    {
        UE_LOG(LogTemp, Log, TEXT("No next dialogue line, ending dialogue"));
        EndDialogue();
        return false;
    }
    
    // Load next dialogue line
    if (!LoadDialogueLine(CurrentDialogueLine.NextDialogueID))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load next dialogue line: %s"), *CurrentDialogueLine.NextDialogueID);
        EndDialogue();
        return false;
    }
    
    CurrentDialogueID = CurrentDialogueLine.NextDialogueID;
    
    // Update dialogue choices
    UpdateDialogueChoices();
    
    // Broadcast line change
    OnDialogueLineChanged.Broadcast(CurrentDialogueLine.DialogueText, CurrentDialogueLine.SpeakerName, CurrentDialogueLine.EmotionalState);
    
    // Play new audio
    PlayDialogueAudio();
    
    return true;
}

bool UDialogueComponent::SelectDialogueChoice(int32 ChoiceIndex)
{
    if (!bIsDialogueActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot select choice - no active dialogue"));
        return false;
    }
    
    if (!CurrentChoices.IsValidIndex(ChoiceIndex))
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid choice index: %d"), ChoiceIndex);
        return false;
    }
    
    FString ChoiceDialogueID = CurrentChoices[ChoiceIndex];
    
    // Load the chosen dialogue line
    if (!LoadDialogueLine(ChoiceDialogueID))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load choice dialogue: %s"), *ChoiceDialogueID);
        return false;
    }
    
    CurrentDialogueID = ChoiceDialogueID;
    
    // Update dialogue choices for the new line
    UpdateDialogueChoices();
    
    // Broadcast line change
    OnDialogueLineChanged.Broadcast(CurrentDialogueLine.DialogueText, CurrentDialogueLine.SpeakerName, CurrentDialogueLine.EmotionalState);
    
    // Play audio
    PlayDialogueAudio();
    
    return true;
}

TArray<FString> UDialogueComponent::GetDialogueChoices() const
{
    return CurrentChoices;
}

void UDialogueComponent::SetDialogueDataTable(UDataTable* NewDialogueTable)
{
    DialogueDataTable = NewDialogueTable;
    UE_LOG(LogTemp, Log, TEXT("Set dialogue data table: %s"), NewDialogueTable ? *NewDialogueTable->GetName() : TEXT("None"));
}

void UDialogueComponent::PlayDialogueAudio()
{
    if (!AudioComponent || CurrentDialogueLine.AudioAssetPath.IsEmpty())
    {
        return;
    }
    
    // Stop current audio
    StopDialogueAudio();
    
    // Load and play audio asset
    if (USoundBase* DialogueSound = LoadObject<USoundBase>(nullptr, *CurrentDialogueLine.AudioAssetPath))
    {
        AudioComponent->SetSound(DialogueSound);
        AudioComponent->Play();
        UE_LOG(LogTemp, Log, TEXT("Playing dialogue audio: %s"), *CurrentDialogueLine.AudioAssetPath);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to load dialogue audio: %s"), *CurrentDialogueLine.AudioAssetPath);
    }
}

void UDialogueComponent::StopDialogueAudio()
{
    if (AudioComponent && AudioComponent->IsPlaying())
    {
        AudioComponent->Stop();
    }
}

bool UDialogueComponent::LoadDialogueLine(const FString& DialogueID)
{
    if (!DialogueDataTable)
    {
        UE_LOG(LogTemp, Error, TEXT("No dialogue data table set"));
        return false;
    }
    
    // Find the dialogue line in the data table
    FNarr_DialogueLine* DialogueLine = DialogueDataTable->FindRow<FNarr_DialogueLine>(FName(*DialogueID), TEXT("LoadDialogueLine"));
    
    if (!DialogueLine)
    {
        UE_LOG(LogTemp, Error, TEXT("Dialogue line not found in data table: %s"), *DialogueID);
        return false;
    }
    
    CurrentDialogueLine = *DialogueLine;
    
    // Use default speaker name if not specified
    if (CurrentDialogueLine.SpeakerName.IsEmpty())
    {
        CurrentDialogueLine.SpeakerName = DefaultSpeakerName;
    }
    
    return true;
}

bool UDialogueComponent::CanAccessDialogueLine(const FNarr_DialogueLine& DialogueLine) const
{
    // Check consciousness level requirement
    int32 PlayerConsciousnessLevel = GetPlayerConsciousnessLevel();
    if (PlayerConsciousnessLevel < DialogueLine.ConsciousnessLevel)
    {
        return false;
    }
    
    // Check quest stage requirement
    if (DialogueLine.RequiredQuestStage >= 0)
    {
        // TODO: Implement quest stage checking when quest system is available
        // For now, assume all quest requirements are met
    }
    
    return true;
}

int32 UDialogueComponent::GetPlayerConsciousnessLevel() const
{
    // TODO: Get actual player consciousness level from player character or game state
    // For now, return a default level
    return 1;
}

void UDialogueComponent::UpdateDialogueChoices()
{
    CurrentChoices.Empty();
    
    if (!DialogueDataTable || !bIsDialogueActive)
    {
        return;
    }
    
    // Find all dialogue lines that are player choices and reference the current line
    TArray<FNarr_DialogueLine*> AllDialogueLines;
    DialogueDataTable->GetAllRows<FNarr_DialogueLine>(TEXT("UpdateDialogueChoices"), AllDialogueLines);
    
    for (const FNarr_DialogueLine* DialogueLine : AllDialogueLines)
    {
        if (DialogueLine && DialogueLine->IsPlayerChoice)
        {
            // Check if this choice is accessible
            if (CanAccessDialogueLine(*DialogueLine))
            {
                // For now, add all player choices - in a full implementation,
                // you'd want to check if this choice is valid for the current dialogue context
                CurrentChoices.Add(DialogueLine->DialogueText);
            }
        }
    }
}