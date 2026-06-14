#include "Narr_DialogueSystem.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"

UNarr_DialogueComponent::UNarr_DialogueComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    bIsDialogueActive = false;
    InteractionRange = 300.0f;
    CurrentTreeID = TEXT("");
}

void UNarr_DialogueComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize default dialogue trees
    FNarr_DialogueTree DefaultTree;
    DefaultTree.TreeID = TEXT("DefaultGreeting");
    
    FNarr_DialogueEntry GreetingEntry;
    GreetingEntry.SpeakerName = TEXT("Tribal Elder");
    GreetingEntry.DialogueText = FText::FromString(TEXT("Greetings, traveler. These lands are dangerous."));
    GreetingEntry.Duration = 4.0f;
    GreetingEntry.ResponseOptions.Add(TEXT("Tell me about the dangers"));
    GreetingEntry.ResponseOptions.Add(TEXT("I can handle myself"));
    
    DefaultTree.Entries.Add(GreetingEntry);
    DialogueTrees.Add(DefaultTree);
}

void UNarr_DialogueComponent::StartDialogue(const FString& TreeID)
{
    if (TreeID.IsEmpty())
    {
        return;
    }

    for (const FNarr_DialogueTree& Tree : DialogueTrees)
    {
        if (Tree.TreeID == TreeID)
        {
            CurrentTreeID = TreeID;
            bIsDialogueActive = true;
            break;
        }
    }
}

void UNarr_DialogueComponent::EndDialogue()
{
    bIsDialogueActive = false;
    CurrentTreeID = TEXT("");
}

FNarr_DialogueEntry UNarr_DialogueComponent::GetCurrentEntry()
{
    if (!bIsDialogueActive || CurrentTreeID.IsEmpty())
    {
        return FNarr_DialogueEntry();
    }

    for (FNarr_DialogueTree& Tree : DialogueTrees)
    {
        if (Tree.TreeID == CurrentTreeID)
        {
            if (Tree.Entries.IsValidIndex(Tree.CurrentEntryIndex))
            {
                return Tree.Entries[Tree.CurrentEntryIndex];
            }
            break;
        }
    }

    return FNarr_DialogueEntry();
}

void UNarr_DialogueComponent::AdvanceDialogue()
{
    if (!bIsDialogueActive || CurrentTreeID.IsEmpty())
    {
        return;
    }

    for (FNarr_DialogueTree& Tree : DialogueTrees)
    {
        if (Tree.TreeID == CurrentTreeID)
        {
            Tree.CurrentEntryIndex++;
            if (Tree.CurrentEntryIndex >= Tree.Entries.Num())
            {
                EndDialogue();
            }
            break;
        }
    }
}

bool UNarr_DialogueComponent::CanInteract(AActor* InteractingActor)
{
    if (!InteractingActor || !GetOwner())
    {
        return false;
    }

    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), InteractingActor->GetActorLocation());
    return Distance <= InteractionRange;
}

void UNarr_DialogueComponent::AddDialogueTree(const FNarr_DialogueTree& NewTree)
{
    DialogueTrees.Add(NewTree);
}

void UNarr_DialogueManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    // Register default dialogue trees
    FNarr_DialogueTree HunterTree;
    HunterTree.TreeID = TEXT("HunterIntro");
    
    FNarr_DialogueEntry HunterEntry1;
    HunterEntry1.SpeakerName = TEXT("Tribal Hunter");
    HunterEntry1.DialogueText = FText::FromString(TEXT("The great beasts stalk these lands. You need weapons."));
    HunterEntry1.Duration = 5.0f;
    HunterEntry1.ResponseOptions.Add(TEXT("What weapons do you recommend?"));
    HunterEntry1.ResponseOptions.Add(TEXT("I have my own tools"));
    
    FNarr_DialogueEntry HunterEntry2;
    HunterEntry2.SpeakerName = TEXT("Tribal Hunter");
    HunterEntry2.DialogueText = FText::FromString(TEXT("Stone spears and fire. Nothing else will pierce their hide."));
    HunterEntry2.Duration = 4.0f;
    
    HunterTree.Entries.Add(HunterEntry1);
    HunterTree.Entries.Add(HunterEntry2);
    
    RegisterDialogueTree(HunterTree.TreeID, HunterTree);
}

void UNarr_DialogueManager::RegisterDialogueTree(const FString& TreeID, const FNarr_DialogueTree& Tree)
{
    if (!TreeID.IsEmpty())
    {
        GlobalDialogueTrees.Add(TreeID, Tree);
    }
}

FNarr_DialogueTree UNarr_DialogueManager::GetDialogueTree(const FString& TreeID)
{
    if (GlobalDialogueTrees.Contains(TreeID))
    {
        return GlobalDialogueTrees[TreeID];
    }
    
    return FNarr_DialogueTree();
}

void UNarr_DialogueManager::MarkDialogueCompleted(const FString& TreeID)
{
    if (!TreeID.IsEmpty() && !CompletedDialogues.Contains(TreeID))
    {
        CompletedDialogues.Add(TreeID);
    }
}

bool UNarr_DialogueManager::IsDialogueCompleted(const FString& TreeID)
{
    return CompletedDialogues.Contains(TreeID);
}

TArray<FString> UNarr_DialogueManager::GetAvailableDialogues()
{
    TArray<FString> AvailableDialogues;
    
    for (const auto& DialoguePair : GlobalDialogueTrees)
    {
        if (!IsDialogueCompleted(DialoguePair.Key))
        {
            AvailableDialogues.Add(DialoguePair.Key);
        }
    }
    
    return AvailableDialogues;
}