#include "DialogueComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DataTable.h"

UNarr_DialogueComponent::UNarr_DialogueComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = false;
    
    MaxInteractionDistance = 300.0f;
    bIsDialogueActive = false;
    CurrentDialogueIndex = 0;
    CurrentPlayerActor = nullptr;
    DialogueTimer = 0.0f;
    CurrentSpeakerName = TEXT("");
    
    // Initialize character data with default values
    CharacterData.CharacterName = TEXT("Unknown NPC");
    CharacterData.Archetype = ENarr_CharacterArchetype::Elder;
    CharacterData.BackgroundStory = FText::FromString(TEXT("A mysterious character with untold stories."));
    CharacterData.PrimaryTheme = ENarr_NarrativeTheme::SelfDiscovery;
    CharacterData.bIsEssentialCharacter = false;
    CharacterData.VoiceActorName = TEXT("DefaultVoice");
}

void UNarr_DialogueComponent::BeginPlay()
{
    Super::BeginPlay();
    
    LoadDialogueFromDataTable();
    
    // Set up default dialogue if none exists
    if (CharacterData.DialogueLines.Num() == 0)
    {
        FNarr_DialogueLine DefaultLine;
        DefaultLine.SpeakerName = CharacterData.CharacterName;
        DefaultLine.DialogueText = FText::FromString(TEXT("Greetings, traveler. I sense you are on a journey of discovery."));
        DefaultLine.Emotion = ENarr_DialogueEmotion::Wise;
        DefaultLine.RequiredConsciousnessLevel = ENarr_ConsciousnessLevel::Unaware;
        DefaultLine.Duration = 4.0f;
        DefaultLine.bIsPlayerChoice = false;
        
        CharacterData.DialogueLines.Add(DefaultLine);
    }
}

void UNarr_DialogueComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bIsDialogueActive && CurrentPlayerActor)
    {
        // Check distance to player
        float Distance = FVector::Dist(GetOwner()->GetActorLocation(), CurrentPlayerActor->GetActorLocation());
        if (Distance > MaxInteractionDistance)
        {
            EndDialogue();
            return;
        }
        
        // Update dialogue timer
        DialogueTimer += DeltaTime;
        
        // Auto-advance dialogue if no player choices are available
        if (CurrentPlayerChoices.Num() == 0 && DialogueTimer >= CurrentDialogueLine.Duration)
        {
            ProcessNextDialogueLine();
        }
    }
}

bool UNarr_DialogueComponent::StartDialogue(AActor* PlayerActor)
{
    if (!PlayerActor || bIsDialogueActive)
    {
        return false;
    }
    
    // Check distance
    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), PlayerActor->GetActorLocation());
    if (Distance > MaxInteractionDistance)
    {
        return false;
    }
    
    CurrentPlayerActor = PlayerActor;
    bIsDialogueActive = true;
    CurrentDialogueIndex = 0;
    DialogueTimer = 0.0f;
    CurrentSpeakerName = CharacterData.CharacterName;
    
    SetComponentTickEnabled(true);
    
    // Start with the first dialogue line
    if (CharacterData.DialogueLines.Num() > 0)
    {
        CurrentDialogueLine = CharacterData.DialogueLines[0];
        PlayDialogueAudio(CurrentDialogueLine);
        OnDialogueStarted.Broadcast(CurrentSpeakerName, CurrentDialogueLine.DialogueText);
        
        // Check for player choices
        FindPlayerChoices();
    }
    
    return true;
}

void UNarr_DialogueComponent::EndDialogue()
{
    if (!bIsDialogueActive)
    {
        return;
    }
    
    bIsDialogueActive = false;
    SetComponentTickEnabled(false);
    
    OnDialogueEnded.Broadcast(CurrentSpeakerName);
    
    CurrentPlayerActor = nullptr;
    CurrentDialogueIndex = 0;
    DialogueTimer = 0.0f;
    CurrentPlayerChoices.Empty();
}

void UNarr_DialogueComponent::SelectPlayerChoice(int32 ChoiceIndex)
{
    if (!bIsDialogueActive || ChoiceIndex < 0 || ChoiceIndex >= CurrentPlayerChoices.Num())
    {
        return;
    }
    
    // Process the selected choice
    FNarr_DialogueLine SelectedChoice = CurrentPlayerChoices[ChoiceIndex];
    CurrentDialogueLine = SelectedChoice;
    CurrentSpeakerName = TEXT("Player");
    DialogueTimer = 0.0f;
    
    // Broadcast the player's choice
    OnDialogueStarted.Broadcast(CurrentSpeakerName, SelectedChoice.DialogueText);
    
    // Clear current choices and move to next NPC line
    CurrentPlayerChoices.Empty();
    ProcessNextDialogueLine();
}

TArray<FNarr_DialogueLine> UNarr_DialogueComponent::GetAvailableDialogue(ENarr_ConsciousnessLevel PlayerConsciousnessLevel) const
{
    TArray<FNarr_DialogueLine> AvailableLines;
    
    for (const FNarr_DialogueLine& Line : CharacterData.DialogueLines)
    {
        if (MeetsDialogueRequirements(Line, PlayerConsciousnessLevel))
        {
            AvailableLines.Add(Line);
        }
    }
    
    return AvailableLines;
}

void UNarr_DialogueComponent::AddDialogueLine(const FNarr_DialogueLine& NewLine)
{
    CharacterData.DialogueLines.Add(NewLine);
}

void UNarr_DialogueComponent::RemoveDialogueLine(int32 LineIndex)
{
    if (LineIndex >= 0 && LineIndex < CharacterData.DialogueLines.Num())
    {
        CharacterData.DialogueLines.RemoveAt(LineIndex);
    }
}

void UNarr_DialogueComponent::SetCharacterData(const FNarr_CharacterData& InCharacterData)
{
    CharacterData = InCharacterData;
}

void UNarr_DialogueComponent::ProcessNextDialogueLine()
{
    CurrentDialogueIndex++;
    DialogueTimer = 0.0f;
    
    // Find next appropriate dialogue line
    ENarr_ConsciousnessLevel PlayerLevel = GetPlayerConsciousnessLevel(CurrentPlayerActor);
    TArray<FNarr_DialogueLine> AvailableLines = GetAvailableDialogue(PlayerLevel);
    
    // Filter out player choice lines for NPC responses
    TArray<FNarr_DialogueLine> NPCLines;
    for (const FNarr_DialogueLine& Line : AvailableLines)
    {
        if (!Line.bIsPlayerChoice)
        {
            NPCLines.Add(Line);
        }
    }
    
    if (CurrentDialogueIndex < NPCLines.Num())
    {
        CurrentDialogueLine = NPCLines[CurrentDialogueIndex];
        CurrentSpeakerName = CharacterData.CharacterName;
        PlayDialogueAudio(CurrentDialogueLine);
        OnDialogueStarted.Broadcast(CurrentSpeakerName, CurrentDialogueLine.DialogueText);
        
        // Check for new player choices
        FindPlayerChoices();
    }
    else
    {
        // End dialogue if no more lines
        EndDialogue();
    }
}

void UNarr_DialogueComponent::FindPlayerChoices()
{
    CurrentPlayerChoices.Empty();
    
    ENarr_ConsciousnessLevel PlayerLevel = GetPlayerConsciousnessLevel(CurrentPlayerActor);
    
    // Find all available player choice lines
    for (const FNarr_DialogueLine& Line : CharacterData.DialogueLines)
    {
        if (Line.bIsPlayerChoice && MeetsDialogueRequirements(Line, PlayerLevel))
        {
            CurrentPlayerChoices.Add(Line);
        }
    }
    
    // If player choices are available, broadcast them
    if (CurrentPlayerChoices.Num() > 0)
    {
        OnPlayerChoiceAvailable.Broadcast(CurrentPlayerChoices, 0, 30.0f); // 30 second time limit
    }
}

bool UNarr_DialogueComponent::MeetsDialogueRequirements(const FNarr_DialogueLine& DialogueLine, ENarr_ConsciousnessLevel PlayerLevel) const
{
    // Check consciousness level requirement
    if (static_cast<int32>(PlayerLevel) < static_cast<int32>(DialogueLine.RequiredConsciousnessLevel))
    {
        return false;
    }
    
    // TODO: Add more complex conditional logic based on tags
    // For now, all lines that meet consciousness requirements are available
    
    return true;
}

ENarr_ConsciousnessLevel UNarr_DialogueComponent::GetPlayerConsciousnessLevel(AActor* PlayerActor) const
{
    if (!PlayerActor)
    {
        return ENarr_ConsciousnessLevel::Unaware;
    }
    
    // Try to get consciousness component from player
    UConsciousnessComponent* ConsciousnessComp = PlayerActor->FindComponentByClass<UConsciousnessComponent>();
    if (ConsciousnessComp)
    {
        // TODO: Map consciousness component state to narrative consciousness level
        // For now, return a default level
        return ENarr_ConsciousnessLevel::Aware;
    }
    
    return ENarr_ConsciousnessLevel::Unaware;
}

void UNarr_DialogueComponent::PlayDialogueAudio(const FNarr_DialogueLine& DialogueLine)
{
    if (DialogueLine.AudioAssetPath.IsEmpty())
    {
        return;
    }
    
    // Try to load and play audio asset
    USoundBase* DialogueSound = LoadObject<USoundBase>(nullptr, *DialogueLine.AudioAssetPath);
    if (DialogueSound)
    {
        UGameplayStatics::PlaySoundAtLocation(
            GetWorld(),
            DialogueSound,
            GetOwner()->GetActorLocation(),
            1.0f, // Volume
            1.0f, // Pitch
            0.0f  // Start time
        );
    }
}

void UNarr_DialogueComponent::LoadDialogueFromDataTable()
{
    if (!DialogueDataTable)
    {
        return;
    }
    
    // Get all rows from the data table
    TArray<FNarr_DialogueLine*> AllRows;
    DialogueDataTable->GetAllRows<FNarr_DialogueLine>(TEXT("LoadDialogueFromDataTable"), AllRows);
    
    // Add rows to character dialogue lines
    for (FNarr_DialogueLine* Row : AllRows)
    {
        if (Row)
        {
            CharacterData.DialogueLines.Add(*Row);
        }
    }
}