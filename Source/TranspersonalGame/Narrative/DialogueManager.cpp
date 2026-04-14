#include "DialogueManager.h"
#include "Engine/DataTable.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

ADialogueManager::ADialogueManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Initialize dialogue state
    bIsDialogueActive = false;
    CurrentDialogueID = TEXT("");
    CurrentLineIndex = 0;
    bAutoAdvanceEnabled = false;
    AutoAdvanceDelay = 3.0f;
    AutoAdvanceTimer = 0.0f;
    bWaitingForChoice = false;
    
    DialogueDataTable = nullptr;
    CharacterDataTable = nullptr;
}

void ADialogueManager::BeginPlay()
{
    Super::BeginPlay();
    
    ResetDialogueState();
}

void ADialogueManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Handle auto-advance
    if (bIsDialogueActive && bAutoAdvanceEnabled && !bWaitingForChoice)
    {
        AutoAdvanceTimer += DeltaTime;
        if (AutoAdvanceTimer >= AutoAdvanceDelay)
        {
            AdvanceDialogue();
            AutoAdvanceTimer = 0.0f;
        }
    }
}

bool ADialogueManager::StartDialogue(const FString& DialogueID)
{
    if (bIsDialogueActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("Dialogue already active. Cannot start new dialogue."));
        return false;
    }
    
    FNarr_DialogueNode* DialogueNode = GetDialogueNode(DialogueID);
    if (!DialogueNode)
    {
        UE_LOG(LogTemp, Warning, TEXT("Dialogue ID not found: %s"), *DialogueID);
        return false;
    }
    
    // Start the dialogue
    CurrentDialogueID = DialogueID;
    CurrentDialogueNode = *DialogueNode;
    CurrentLineIndex = 0;
    bIsDialogueActive = true;
    bWaitingForChoice = false;
    AutoAdvanceTimer = 0.0f;
    
    // Broadcast dialogue started
    FString SpeakerName = TEXT("Unknown");
    if (CurrentDialogueNode.DialogueLines.Num() > 0)
    {
        SpeakerName = CurrentDialogueNode.DialogueLines[0].SpeakerName;
    }
    OnDialogueStarted.Broadcast(DialogueID, SpeakerName);
    
    // Process first line
    ProcessCurrentLine();
    
    UE_LOG(LogTemp, Log, TEXT("Started dialogue: %s"), *DialogueID);
    return true;
}

void ADialogueManager::EndDialogue()
{
    if (!bIsDialogueActive)
    {
        return;
    }
    
    FString EndedDialogueID = CurrentDialogueID;
    ResetDialogueState();
    
    OnDialogueEnded.Broadcast(EndedDialogueID);
    UE_LOG(LogTemp, Log, TEXT("Ended dialogue: %s"), *EndedDialogueID);
}

bool ADialogueManager::AdvanceDialogue()
{
    if (!bIsDialogueActive || bWaitingForChoice)
    {
        return false;
    }
    
    CurrentLineIndex++;
    
    // Check if we've reached the end of dialogue lines
    if (CurrentLineIndex >= CurrentDialogueNode.DialogueLines.Num())
    {
        // Check if there are player choices
        if (CurrentDialogueNode.PlayerChoices.Num() > 0)
        {
            bWaitingForChoice = true;
            OnChoicesPresented.Broadcast(CurrentDialogueNode.PlayerChoices);
            return true;
        }
        else
        {
            // No more lines and no choices - end dialogue
            EndDialogue();
            return false;
        }
    }
    
    // Process next line
    ProcessCurrentLine();
    return true;
}

bool ADialogueManager::SelectChoice(int32 ChoiceIndex)
{
    if (!bIsDialogueActive || !bWaitingForChoice)
    {
        return false;
    }
    
    if (ChoiceIndex < 0 || ChoiceIndex >= CurrentDialogueNode.PlayerChoices.Num())
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid choice index: %d"), ChoiceIndex);
        return false;
    }
    
    const FNarr_DialogueChoice& SelectedChoice = CurrentDialogueNode.PlayerChoices[ChoiceIndex];
    
    // Check if choice requirements are met
    if (!AreRequirementsMet(SelectedChoice.RequiredFlags))
    {
        UE_LOG(LogTemp, Warning, TEXT("Choice requirements not met"));
        return false;
    }
    
    // Apply choice effects
    ApplyChoiceEffects(SelectedChoice);
    
    // Continue to next dialogue or end
    if (SelectedChoice.NextDialogueID.IsEmpty())
    {
        EndDialogue();
    }
    else
    {
        EndDialogue();
        StartDialogue(SelectedChoice.NextDialogueID);
    }
    
    return true;
}

FNarr_DialogueLine ADialogueManager::GetCurrentLine() const
{
    if (!bIsDialogueActive || CurrentLineIndex >= CurrentDialogueNode.DialogueLines.Num())
    {
        return FNarr_DialogueLine();
    }
    
    return CurrentDialogueNode.DialogueLines[CurrentLineIndex];
}

TArray<FNarr_DialogueChoice> ADialogueManager::GetCurrentChoices() const
{
    if (!bIsDialogueActive || !bWaitingForChoice)
    {
        return TArray<FNarr_DialogueChoice>();
    }
    
    // Filter choices based on requirements
    TArray<FNarr_DialogueChoice> AvailableChoices;
    for (const FNarr_DialogueChoice& Choice : CurrentDialogueNode.PlayerChoices)
    {
        if (AreRequirementsMet(Choice.RequiredFlags))
        {
            AvailableChoices.Add(Choice);
        }
    }
    
    return AvailableChoices;
}

void ADialogueManager::SetPlayerFlag(const FString& FlagName)
{
    if (!FlagName.IsEmpty() && !PlayerFlags.Contains(FlagName))
    {
        PlayerFlags.Add(FlagName);
        UE_LOG(LogTemp, Log, TEXT("Set player flag: %s"), *FlagName);
    }
}

bool ADialogueManager::HasPlayerFlag(const FString& FlagName) const
{
    return PlayerFlags.Contains(FlagName);
}

void ADialogueManager::RemovePlayerFlag(const FString& FlagName)
{
    if (PlayerFlags.Contains(FlagName))
    {
        PlayerFlags.Remove(FlagName);
        UE_LOG(LogTemp, Log, TEXT("Removed player flag: %s"), *FlagName);
    }
}

void ADialogueManager::ClearAllFlags()
{
    PlayerFlags.Empty();
    UE_LOG(LogTemp, Log, TEXT("Cleared all player flags"));
}

bool ADialogueManager::IsDialogueAvailable(const FString& DialogueID) const
{
    return GetDialogueNode(DialogueID) != nullptr;
}

bool ADialogueManager::AreRequirementsMet(const TArray<FString>& RequiredFlags) const
{
    for (const FString& Flag : RequiredFlags)
    {
        if (!HasPlayerFlag(Flag))
        {
            return false;
        }
    }
    return true;
}

FNarr_CharacterLore ADialogueManager::GetCharacterLore(const FString& CharacterID) const
{
    if (!CharacterDataTable)
    {
        UE_LOG(LogTemp, Warning, TEXT("Character data table not set"));
        return FNarr_CharacterLore();
    }
    
    FNarr_CharacterTableRow* CharacterRow = CharacterDataTable->FindRow<FNarr_CharacterTableRow>(FName(*CharacterID), TEXT(""));
    if (CharacterRow)
    {
        return CharacterRow->CharacterData;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Character not found: %s"), *CharacterID);
    return FNarr_CharacterLore();
}

void ADialogueManager::ProcessCurrentLine()
{
    if (!bIsDialogueActive || CurrentLineIndex >= CurrentDialogueNode.DialogueLines.Num())
    {
        return;
    }
    
    const FNarr_DialogueLine& CurrentLine = CurrentDialogueNode.DialogueLines[CurrentLineIndex];
    
    // Broadcast line changed event
    OnDialogueLineChanged.Broadcast(CurrentLine, CurrentLineIndex, CurrentDialogueNode.DialogueLines.Num());
    
    // Reset auto-advance timer
    AutoAdvanceTimer = 0.0f;
    
    UE_LOG(LogTemp, Log, TEXT("Processing dialogue line %d: %s"), CurrentLineIndex, *CurrentLine.DialogueText.ToString());
}

void ADialogueManager::ApplyChoiceEffects(const FNarr_DialogueChoice& Choice)
{
    // Set flags from choice
    for (const FString& Flag : Choice.SetFlags)
    {
        SetPlayerFlag(Flag);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Applied choice effects. Set %d flags."), Choice.SetFlags.Num());
}

FNarr_DialogueNode* ADialogueManager::GetDialogueNode(const FString& DialogueID) const
{
    if (!DialogueDataTable)
    {
        UE_LOG(LogTemp, Warning, TEXT("Dialogue data table not set"));
        return nullptr;
    }
    
    FNarr_DialogueTableRow* DialogueRow = DialogueDataTable->FindRow<FNarr_DialogueTableRow>(FName(*DialogueID), TEXT(""));
    if (DialogueRow)
    {
        return &DialogueRow->DialogueData;
    }
    
    return nullptr;
}

void ADialogueManager::ResetDialogueState()
{
    bIsDialogueActive = false;
    CurrentDialogueID = TEXT("");
    CurrentLineIndex = 0;
    bWaitingForChoice = false;
    AutoAdvanceTimer = 0.0f;
    CurrentDialogueNode = FNarr_DialogueNode();
}

// UDialogueParticipantComponent Implementation

UDialogueParticipantComponent::UDialogueParticipantComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    CharacterID = TEXT("");
    DefaultDialogueID = TEXT("");
    bCanInitiateDialogue = true;
    InteractionDistance = 300.0f;
    DialogueManager = nullptr;
}

void UDialogueParticipantComponent::BeginPlay()
{
    Super::BeginPlay();
    
    FindDialogueManager();
}

bool UDialogueParticipantComponent::StartDialogueWithPlayer()
{
    if (!CanStartDialogue())
    {
        return false;
    }
    
    FString DialogueToStart = GetBestAvailableDialogue();
    if (DialogueToStart.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("No available dialogue for character: %s"), *CharacterID);
        return false;
    }
    
    return DialogueManager->StartDialogue(DialogueToStart);
}

bool UDialogueParticipantComponent::CanStartDialogue() const
{
    return bCanInitiateDialogue && DialogueManager && !DialogueManager->bIsDialogueActive;
}

FString UDialogueParticipantComponent::GetBestAvailableDialogue() const
{
    if (!DialogueManager)
    {
        return TEXT("");
    }
    
    // Check available dialogues in order
    for (const FString& DialogueID : AvailableDialogues)
    {
        if (DialogueManager->IsDialogueAvailable(DialogueID))
        {
            return DialogueID;
        }
    }
    
    // Fall back to default dialogue
    if (DialogueManager->IsDialogueAvailable(DefaultDialogueID))
    {
        return DefaultDialogueID;
    }
    
    return TEXT("");
}

void UDialogueParticipantComponent::AddAvailableDialogue(const FString& DialogueID)
{
    if (!DialogueID.IsEmpty() && !AvailableDialogues.Contains(DialogueID))
    {
        AvailableDialogues.Add(DialogueID);
    }
}

void UDialogueParticipantComponent::RemoveAvailableDialogue(const FString& DialogueID)
{
    AvailableDialogues.Remove(DialogueID);
}

void UDialogueParticipantComponent::FindDialogueManager()
{
    if (UWorld* World = GetWorld())
    {
        DialogueManager = Cast<ADialogueManager>(UGameplayStatics::GetActorOfClass(World, ADialogueManager::StaticClass()));
        
        if (!DialogueManager)
        {
            UE_LOG(LogTemp, Warning, TEXT("No DialogueManager found in the world"));
        }
    }
}