#include "DialogueComponent.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"

UDialogueComponent::UDialogueComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    bDialogueEnabled = true;
    DefaultDialogueID = TEXT("");
    DialogueRange = 500.0f;
    bAutoTriggerOnOverlap = false;
    bDialogueActive = false;
    CurrentDialogueID = TEXT("");
    NarrativeManager = nullptr;
}

void UDialogueComponent::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeNarrativeManager();
}

void UDialogueComponent::TriggerDialogue(const FString& DialogueID)
{
    if (!bDialogueEnabled || bDialogueActive)
    {
        return;
    }

    if (!IsPlayerInRange())
    {
        UE_LOG(LogTemp, Warning, TEXT("DialogueComponent: Player not in range for dialogue: %s"), *DialogueID);
        return;
    }

    FString DialogueToTrigger = DialogueID.IsEmpty() ? DefaultDialogueID : DialogueID;
    
    if (DialogueToTrigger.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("DialogueComponent: No dialogue ID specified"));
        return;
    }

    bDialogueActive = true;
    CurrentDialogueID = DialogueToTrigger;
    CurrentDialogueOptions.Empty();

    UE_LOG(LogTemp, Log, TEXT("DialogueComponent: Triggering dialogue: %s"), *DialogueToTrigger);

    // Trigger dialogue through narrative manager
    if (NarrativeManager)
    {
        NarrativeManager->TriggerDialogue(DialogueToTrigger, GetOwner());
    }

    // Broadcast dialogue triggered event
    OnDialogueTriggered.Broadcast(DialogueToTrigger, GetOwner());
}

void UDialogueComponent::SetDialogueEnabled(bool bEnabled)
{
    bDialogueEnabled = bEnabled;
    
    if (!bEnabled && bDialogueActive)
    {
        // End current dialogue if disabling
        bDialogueActive = false;
        CurrentDialogueID = TEXT("");
        CurrentDialogueOptions.Empty();
        OnDialogueCompleted.Broadcast(CurrentDialogueID);
    }
}

bool UDialogueComponent::IsDialogueActive() const
{
    return bDialogueActive;
}

void UDialogueComponent::AddDialogueOption(const FString& DialogueID, const FString& OptionText)
{
    if (!bDialogueActive)
    {
        return;
    }

    CurrentDialogueOptions.Add(OptionText);
    UE_LOG(LogTemp, Log, TEXT("DialogueComponent: Added dialogue option: %s"), *OptionText);
}

void UDialogueComponent::SelectDialogueOption(int32 OptionIndex)
{
    if (!bDialogueActive || !CurrentDialogueOptions.IsValidIndex(OptionIndex))
    {
        return;
    }

    FString SelectedOption = CurrentDialogueOptions[OptionIndex];
    UE_LOG(LogTemp, Log, TEXT("DialogueComponent: Selected dialogue option: %s"), *SelectedOption);

    // Process the selected option
    // This could trigger new dialogue, complete quests, etc.
    
    // For now, end the dialogue
    bDialogueActive = false;
    FString CompletedDialogueID = CurrentDialogueID;
    CurrentDialogueID = TEXT("");
    CurrentDialogueOptions.Empty();
    
    OnDialogueCompleted.Broadcast(CompletedDialogueID);
}

void UDialogueComponent::InitializeNarrativeManager()
{
    if (UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this))
    {
        NarrativeManager = GameInstance->GetSubsystem<UNarrativeManager>();
        if (NarrativeManager)
        {
            UE_LOG(LogTemp, Log, TEXT("DialogueComponent: Connected to NarrativeManager"));
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("DialogueComponent: Failed to get NarrativeManager"));
        }
    }
}

bool UDialogueComponent::IsPlayerInRange() const
{
    APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
    if (!PlayerController || !PlayerController->GetPawn())
    {
        return false;
    }

    FVector PlayerLocation = PlayerController->GetPawn()->GetActorLocation();
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    
    float Distance = FVector::Dist(PlayerLocation, OwnerLocation);
    return Distance <= DialogueRange;
}