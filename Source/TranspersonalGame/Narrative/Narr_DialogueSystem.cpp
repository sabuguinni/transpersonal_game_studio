#include "Narr_DialogueSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/TriggerVolume.h"
#include "Kismet/GameplayStatics.h"

// UNarr_DialogueComponent Implementation
UNarr_DialogueComponent::UNarr_DialogueComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    CurrentState = ENarr_DialogueState::Inactive;
    ActiveSequenceIndex = -1;
    CurrentLineIndex = 0;
}

void UNarr_DialogueComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Register with dialogue manager
    if (UNarr_DialogueManager* DialogueManager = GetWorld()->GetGameInstance()->GetSubsystem<UNarr_DialogueManager>())
    {
        DialogueManager->RegisterDialogueComponent(this);
    }
    
    // Initialize default dialogue sequences if empty
    if (DialogueSequences.Num() == 0)
    {
        // Create default tutorial dialogue
        FNarr_DialogueSequence TutorialSequence;
        TutorialSequence.SequenceID = TEXT("tutorial_basic");
        TutorialSequence.TriggerType = ENarr_DialogueTriggerType::Tutorial;
        TutorialSequence.bCanRepeat = true;
        
        FNarr_DialogueLine TutorialLine1;
        TutorialLine1.SpeakerName = TEXT("Narrator");
        TutorialLine1.DialogueText = FText::FromString(TEXT("Welcome to the ancient hunting grounds, survivor."));
        TutorialLine1.DisplayDuration = 4.0f;
        
        FNarr_DialogueLine TutorialLine2;
        TutorialLine2.SpeakerName = TEXT("Narrator");
        TutorialLine2.DialogueText = FText::FromString(TEXT("Use WASD to move and Space to jump. Stay alert for predators."));
        TutorialLine2.DisplayDuration = 5.0f;
        
        TutorialSequence.DialogueLines.Add(TutorialLine1);
        TutorialSequence.DialogueLines.Add(TutorialLine2);
        DialogueSequences.Add(TutorialSequence);
        
        // Create warning dialogue
        FNarr_DialogueSequence WarningSequence;
        WarningSequence.SequenceID = TEXT("warning_predator");
        WarningSequence.TriggerType = ENarr_DialogueTriggerType::Warning;
        WarningSequence.bCanRepeat = true;
        
        FNarr_DialogueLine WarningLine;
        WarningLine.SpeakerName = TEXT("Tribal Guide");
        WarningLine.DialogueText = FText::FromString(TEXT("Danger ahead! Massive predator tracks in the mud. Seek high ground or shelter."));
        WarningLine.DisplayDuration = 6.0f;
        
        WarningSequence.DialogueLines.Add(WarningLine);
        DialogueSequences.Add(WarningSequence);
    }
}

bool UNarr_DialogueComponent::StartDialogueSequence(const FString& SequenceID)
{
    if (CurrentState == ENarr_DialogueState::InProgress)
    {
        return false; // Already in dialogue
    }
    
    int32 SequenceIndex = FindSequenceIndex(SequenceID);
    if (SequenceIndex == -1)
    {
        UE_LOG(LogTemp, Warning, TEXT("Dialogue sequence not found: %s"), *SequenceID);
        return false;
    }
    
    const FNarr_DialogueSequence& Sequence = DialogueSequences[SequenceIndex];
    if (!ValidateSequenceRequirements(Sequence))
    {
        return false;
    }
    
    ActiveSequenceIndex = SequenceIndex;
    CurrentLineIndex = 0;
    CurrentState = ENarr_DialogueState::InProgress;
    
    UE_LOG(LogTemp, Log, TEXT("Started dialogue sequence: %s"), *SequenceID);
    return true;
}

bool UNarr_DialogueComponent::AdvanceDialogue()
{
    if (CurrentState != ENarr_DialogueState::InProgress || ActiveSequenceIndex == -1)
    {
        return false;
    }
    
    const FNarr_DialogueSequence& ActiveSequence = DialogueSequences[ActiveSequenceIndex];
    
    CurrentLineIndex++;
    if (CurrentLineIndex >= ActiveSequence.DialogueLines.Num())
    {
        // End of sequence
        EndDialogue();
        return false;
    }
    
    return true;
}

void UNarr_DialogueComponent::EndDialogue()
{
    CurrentState = ENarr_DialogueState::Completed;
    ActiveSequenceIndex = -1;
    CurrentLineIndex = 0;
    
    UE_LOG(LogTemp, Log, TEXT("Dialogue sequence ended"));
}

bool UNarr_DialogueComponent::IsDialogueAvailable(const FString& SequenceID) const
{
    int32 SequenceIndex = FindSequenceIndex(SequenceID);
    if (SequenceIndex == -1)
    {
        return false;
    }
    
    return ValidateSequenceRequirements(DialogueSequences[SequenceIndex]);
}

FNarr_DialogueLine UNarr_DialogueComponent::GetCurrentDialogueLine() const
{
    if (CurrentState != ENarr_DialogueState::InProgress || ActiveSequenceIndex == -1)
    {
        return FNarr_DialogueLine(); // Return default empty line
    }
    
    const FNarr_DialogueSequence& ActiveSequence = DialogueSequences[ActiveSequenceIndex];
    if (CurrentLineIndex < ActiveSequence.DialogueLines.Num())
    {
        return ActiveSequence.DialogueLines[CurrentLineIndex];
    }
    
    return FNarr_DialogueLine();
}

void UNarr_DialogueComponent::AddDialogueSequence(const FNarr_DialogueSequence& NewSequence)
{
    DialogueSequences.Add(NewSequence);
    UE_LOG(LogTemp, Log, TEXT("Added dialogue sequence: %s"), *NewSequence.SequenceID);
}

int32 UNarr_DialogueComponent::FindSequenceIndex(const FString& SequenceID) const
{
    for (int32 i = 0; i < DialogueSequences.Num(); i++)
    {
        if (DialogueSequences[i].SequenceID == SequenceID)
        {
            return i;
        }
    }
    return -1;
}

bool UNarr_DialogueComponent::ValidateSequenceRequirements(const FNarr_DialogueSequence& Sequence) const
{
    // Check if required quest is completed (if specified)
    if (!Sequence.RequiredQuestID.IsEmpty())
    {
        // TODO: Integrate with quest system to check quest status
        // For now, assume requirements are met
        UE_LOG(LogTemp, Log, TEXT("Quest requirement check for: %s"), *Sequence.RequiredQuestID);
    }
    
    return true;
}

// ANarr_DialogueTrigger Implementation
ANarr_DialogueTrigger::ANarr_DialogueTrigger()
{
    PrimaryActorTick.bCanEverTick = false;
    
    DialogueSequenceID = TEXT("tutorial_basic");
    TriggerType = ENarr_DialogueTriggerType::Tutorial;
    bCanRetrigger = false;
    bHasTriggered = false;
    
    // Set up collision
    if (GetCollisionComponent())
    {
        GetCollisionComponent()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        GetCollisionComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);
        GetCollisionComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    }
}

void ANarr_DialogueTrigger::BeginPlay()
{
    Super::BeginPlay();
    
    // Get reference to dialogue manager
    DialogueManager = GetWorld()->GetGameInstance()->GetSubsystem<UNarr_DialogueManager>();
    
    // Bind overlap events
    if (GetCollisionComponent())
    {
        GetCollisionComponent()->OnComponentBeginOverlap.AddDynamic(this, &ANarr_DialogueTrigger::OnOverlapBegin);
    }
}

void ANarr_DialogueTrigger::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, 
                                          UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
                                          bool bFromSweep, const FHitResult& SweepResult)
{
    // Check if it's the player character
    if (!OtherActor || !OtherActor->IsA<ACharacter>())
    {
        return;
    }
    
    // Check if already triggered and can't retrigger
    if (bHasTriggered && !bCanRetrigger)
    {
        return;
    }
    
    // Check if dialogue manager is available
    if (!DialogueManager)
    {
        UE_LOG(LogTemp, Warning, TEXT("Dialogue manager not available"));
        return;
    }
    
    // Find dialogue component on this actor or create one
    UNarr_DialogueComponent* DialogueComponent = GetComponentByClass<UNarr_DialogueComponent>();
    if (!DialogueComponent)
    {
        // Create dialogue component if it doesn't exist
        DialogueComponent = NewObject<UNarr_DialogueComponent>(this);
        if (DialogueComponent)
        {
            DialogueComponent->RegisterComponent();
        }
    }
    
    if (DialogueComponent)
    {
        bool bStarted = DialogueManager->StartDialogue(DialogueComponent, DialogueSequenceID);
        if (bStarted)
        {
            bHasTriggered = true;
            UE_LOG(LogTemp, Log, TEXT("Dialogue triggered: %s"), *DialogueSequenceID);
        }
    }
}

// UNarr_DialogueManager Implementation
void UNarr_DialogueManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    ActiveDialogueComponent = nullptr;
    
    UE_LOG(LogTemp, Log, TEXT("Dialogue Manager initialized"));
}

void UNarr_DialogueManager::RegisterDialogueComponent(UNarr_DialogueComponent* Component)
{
    if (Component && !RegisteredComponents.Contains(Component))
    {
        RegisteredComponents.Add(Component);
        UE_LOG(LogTemp, Log, TEXT("Registered dialogue component"));
    }
}

void UNarr_DialogueManager::UnregisterDialogueComponent(UNarr_DialogueComponent* Component)
{
    if (Component)
    {
        RegisteredComponents.Remove(Component);
        if (ActiveDialogueComponent == Component)
        {
            ActiveDialogueComponent = nullptr;
        }
        UE_LOG(LogTemp, Log, TEXT("Unregistered dialogue component"));
    }
}

bool UNarr_DialogueManager::StartDialogue(UNarr_DialogueComponent* Component, const FString& SequenceID)
{
    if (!Component)
    {
        return false;
    }
    
    // End current dialogue if active
    if (ActiveDialogueComponent && ActiveDialogueComponent != Component)
    {
        ActiveDialogueComponent->EndDialogue();
    }
    
    // Start new dialogue
    bool bStarted = Component->StartDialogueSequence(SequenceID);
    if (bStarted)
    {
        ActiveDialogueComponent = Component;
    }
    
    return bStarted;
}

UNarr_DialogueComponent* UNarr_DialogueManager::GetActiveDialogueComponent() const
{
    return ActiveDialogueComponent;
}

bool UNarr_DialogueManager::IsDialogueActive() const
{
    return ActiveDialogueComponent != nullptr && 
           ActiveDialogueComponent->CurrentState == ENarr_DialogueState::InProgress;
}

void UNarr_DialogueManager::LoadDialogueSequences(UDataTable* DialogueTable)
{
    if (!DialogueTable)
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid dialogue data table"));
        return;
    }
    
    // TODO: Implement data table loading for dialogue sequences
    UE_LOG(LogTemp, Log, TEXT("Loading dialogue sequences from data table"));
}