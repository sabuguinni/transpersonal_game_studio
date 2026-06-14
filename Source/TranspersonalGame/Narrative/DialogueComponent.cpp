#include "DialogueComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

UDialogueComponent::UDialogueComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    CharacterName = TEXT("Unnamed NPC");
    InteractionRange = 300.0f;
    bCanRepeatDialogue = true;
    CurrentInteractor = nullptr;
}

void UDialogueComponent::BeginPlay()
{
    Super::BeginPlay();
    
    if (UGameInstance* GameInstance = GetWorld()->GetGameInstance())
    {
        NarrativeManager = GameInstance->GetSubsystem<UNarrativeManager>();
    }
}

void UDialogueComponent::TriggerDialogue(int32 DialogueIndex)
{
    if (!CanInteract())
    {
        return;
    }

    if (AvailableDialogues.IsValidIndex(DialogueIndex))
    {
        const FString& DialogueID = AvailableDialogues[DialogueIndex];
        
        if (NarrativeManager)
        {
            NarrativeManager->TriggerDialogue(DialogueID, GetOwner());
        }

        // Mark dialogue as used if it can't be repeated
        if (!bCanRepeatDialogue && !UsedDialogues.Contains(DialogueID))
        {
            UsedDialogues.Add(DialogueID);
        }

        // Broadcast the dialogue event
        OnDialogueTriggered.Broadcast(DialogueID, CharacterName);
        
        UE_LOG(LogTemp, Log, TEXT("Dialogue triggered: %s from %s"), *DialogueID, *CharacterName);
    }
}

bool UDialogueComponent::CanInteract() const
{
    if (!CheckStoryRequirements())
    {
        return false;
    }

    if (!bCanRepeatDialogue && UsedDialogues.Num() >= AvailableDialogues.Num())
    {
        return false;
    }

    return true;
}

TArray<FString> UDialogueComponent::GetAvailableDialogueOptions() const
{
    TArray<FString> Options;
    
    for (const FString& DialogueID : AvailableDialogues)
    {
        if (bCanRepeatDialogue || !UsedDialogues.Contains(DialogueID))
        {
            Options.Add(DialogueID);
        }
    }
    
    return Options;
}

void UDialogueComponent::StartConversation(AActor* Interactor)
{
    if (!CanInteract())
    {
        return;
    }

    CurrentInteractor = Interactor;
    
    // Trigger the first available dialogue automatically
    TArray<FString> Options = GetAvailableDialogueOptions();
    if (Options.Num() > 0)
    {
        int32 RandomIndex = FMath::RandRange(0, Options.Num() - 1);
        FString SelectedDialogue = Options[RandomIndex];
        
        int32 OriginalIndex = AvailableDialogues.Find(SelectedDialogue);
        if (OriginalIndex != INDEX_NONE)
        {
            TriggerDialogue(OriginalIndex);
        }
    }
}

void UDialogueComponent::EndConversation()
{
    CurrentInteractor = nullptr;
}

bool UDialogueComponent::CheckStoryRequirements() const
{
    if (!NarrativeManager)
    {
        return true; // If no narrative manager, allow all dialogues
    }

    for (const FString& RequiredFlag : RequiredStoryFlags)
    {
        if (!NarrativeManager->GetStoryFlag(RequiredFlag))
        {
            return false;
        }
    }

    return true;
}

FString UDialogueComponent::GetRandomAvailableDialogue() const
{
    TArray<FString> Options = GetAvailableDialogueOptions();
    if (Options.Num() > 0)
    {
        int32 RandomIndex = FMath::RandRange(0, Options.Num() - 1);
        return Options[RandomIndex];
    }
    return TEXT("");
}