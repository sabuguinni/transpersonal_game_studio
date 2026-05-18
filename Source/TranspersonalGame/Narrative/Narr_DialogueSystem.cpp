#include "Narr_DialogueSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SphereComponent.h"

UNarr_DialogueComponent::UNarr_DialogueComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    bDialogueActive = false;
    ProximityTriggerDistance = 500.0f;
    CurrentTreeName = TEXT("");
    PlayerReference = nullptr;
}

void UNarr_DialogueComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Find player reference
    if (UWorld* World = GetWorld())
    {
        if (APlayerController* PC = World->GetFirstPlayerController())
        {
            PlayerReference = PC->GetPawn();
        }
    }
}

void UNarr_DialogueComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bDialogueActive)
    {
        UpdateProximityTriggers();
    }
}

bool UNarr_DialogueComponent::StartDialogue(const FString& TreeName)
{
    FNarr_DialogueTree* Tree = GetDialogueTree(TreeName);
    if (!Tree || Tree->DialogueLines.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Dialogue tree not found or empty: %s"), *TreeName);
        return false;
    }

    CurrentTreeName = TreeName;
    Tree->CurrentLineIndex = 0;
    Tree->bHasBeenTriggered = true;
    bDialogueActive = true;

    UE_LOG(LogTemp, Log, TEXT("Started dialogue tree: %s"), *TreeName);
    return true;
}

void UNarr_DialogueComponent::EndDialogue()
{
    bDialogueActive = false;
    CurrentTreeName = TEXT("");
    UE_LOG(LogTemp, Log, TEXT("Dialogue ended"));
}

bool UNarr_DialogueComponent::AdvanceDialogue()
{
    FNarr_DialogueTree* Tree = GetDialogueTree(CurrentTreeName);
    if (!Tree)
    {
        EndDialogue();
        return false;
    }

    Tree->CurrentLineIndex++;
    if (Tree->CurrentLineIndex >= Tree->DialogueLines.Num())
    {
        EndDialogue();
        return false;
    }

    return true;
}

void UNarr_DialogueComponent::SelectPlayerChoice(int32 ChoiceIndex)
{
    FNarr_DialogueTree* Tree = GetDialogueTree(CurrentTreeName);
    if (!Tree || Tree->CurrentLineIndex >= Tree->DialogueLines.Num())
    {
        return;
    }

    FNarr_DialogueLine& CurrentLine = Tree->DialogueLines[Tree->CurrentLineIndex];
    if (ChoiceIndex >= 0 && ChoiceIndex < CurrentLine.NextDialogueIDs.Num())
    {
        int32 NextLineID = CurrentLine.NextDialogueIDs[ChoiceIndex];
        if (NextLineID >= 0 && NextLineID < Tree->DialogueLines.Num())
        {
            Tree->CurrentLineIndex = NextLineID;
        }
        else
        {
            EndDialogue();
        }
    }
}

FNarr_DialogueLine UNarr_DialogueComponent::GetCurrentDialogueLine() const
{
    const FNarr_DialogueTree* Tree = const_cast<UNarr_DialogueComponent*>(this)->GetDialogueTree(CurrentTreeName);
    if (Tree && Tree->CurrentLineIndex >= 0 && Tree->CurrentLineIndex < Tree->DialogueLines.Num())
    {
        return Tree->DialogueLines[Tree->CurrentLineIndex];
    }

    return FNarr_DialogueLine();
}

void UNarr_DialogueComponent::RegisterDialogueTree(const FNarr_DialogueTree& NewTree)
{
    // Remove existing tree with same name
    DialogueTrees.RemoveAll([&NewTree](const FNarr_DialogueTree& Tree) {
        return Tree.TreeName == NewTree.TreeName;
    });

    DialogueTrees.Add(NewTree);
    UE_LOG(LogTemp, Log, TEXT("Registered dialogue tree: %s"), *NewTree.TreeName);
}

bool UNarr_DialogueComponent::CheckTriggerCondition(ENarr_DialogueTrigger TriggerType, const FString& ConditionData)
{
    switch (TriggerType)
    {
        case ENarr_DialogueTrigger::Proximity:
            if (PlayerReference && GetOwner())
            {
                float Distance = FVector::Dist(PlayerReference->GetActorLocation(), GetOwner()->GetActorLocation());
                return Distance <= ProximityTriggerDistance;
            }
            break;

        case ENarr_DialogueTrigger::FirstVisit:
            // Check if this is the first time triggering this dialogue
            return true; // Simplified for now

        case ENarr_DialogueTrigger::TimeOfDay:
            // Check game time - simplified implementation
            return true;

        default:
            return false;
    }

    return false;
}

FNarr_DialogueTree* UNarr_DialogueComponent::GetDialogueTree(const FString& TreeName)
{
    for (FNarr_DialogueTree& Tree : DialogueTrees)
    {
        if (Tree.TreeName == TreeName)
        {
            return &Tree;
        }
    }
    return nullptr;
}

void UNarr_DialogueComponent::UpdateProximityTriggers()
{
    if (!PlayerReference || !GetOwner())
    {
        return;
    }

    float Distance = FVector::Dist(PlayerReference->GetActorLocation(), GetOwner()->GetActorLocation());
    if (Distance <= ProximityTriggerDistance)
    {
        // Check for proximity-triggered dialogues
        for (FNarr_DialogueTree& Tree : DialogueTrees)
        {
            if (Tree.TriggerCondition == ENarr_DialogueTrigger::Proximity && 
                (!Tree.bHasBeenTriggered || Tree.bIsRepeatable))
            {
                StartDialogue(Tree.TreeName);
                break;
            }
        }
    }
}

// Dialogue Manager Implementation
ANarr_DialogueManager::ANarr_DialogueManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    DialogueComponent = CreateDefaultSubobject<UNarr_DialogueComponent>(TEXT("DialogueComponent"));
}

void ANarr_DialogueManager::BeginPlay()
{
    Super::BeginPlay();
    InitializeDialogueSystem();
}

void ANarr_DialogueManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ANarr_DialogueManager::InitializeDialogueSystem()
{
    LoadPresetDialogueTrees();
    UE_LOG(LogTemp, Log, TEXT("Dialogue system initialized"));
}

void ANarr_DialogueManager::RegisterDialogueActor(AActor* DialogueActor)
{
    if (DialogueActor && !RegisteredDialogueActors.Contains(DialogueActor))
    {
        RegisteredDialogueActors.Add(DialogueActor);
        UE_LOG(LogTemp, Log, TEXT("Registered dialogue actor: %s"), *DialogueActor->GetName());
    }
}

void ANarr_DialogueManager::TriggerLocationBasedDialogue(const FString& LocationName)
{
    SetGlobalDialogueFlag(FString::Printf(TEXT("Visited_%s"), *LocationName), true);
    UE_LOG(LogTemp, Log, TEXT("Triggered location dialogue for: %s"), *LocationName);
}

void ANarr_DialogueManager::SetGlobalDialogueFlag(const FString& FlagName, bool bValue)
{
    GlobalDialogueFlags.Add(FlagName, bValue);
}

bool ANarr_DialogueManager::GetGlobalDialogueFlag(const FString& FlagName) const
{
    const bool* FoundFlag = GlobalDialogueFlags.Find(FlagName);
    return FoundFlag ? *FoundFlag : false;
}

void ANarr_DialogueManager::LoadPresetDialogueTrees()
{
    CreateTribalHunterDialogue();
    CreateTribalGathererDialogue();
    CreateTribalElderDialogue();
    CreateNarratorDialogue();
}

void ANarr_DialogueManager::CreateTribalHunterDialogue()
{
    FNarr_DialogueTree HunterTree;
    HunterTree.TreeName = TEXT("TribalHunter_Introduction");
    HunterTree.TreeType = ENarr_DialogueNodeType::Standard;
    HunterTree.TriggerCondition = ENarr_DialogueTrigger::Proximity;

    FNarr_DialogueLine Line1;
    Line1.SpeakerName = TEXT("Tribal Hunter");
    Line1.DialogueText = FText::FromString(TEXT("Greetings, survivor. I am the Hunter of this tribe. The wilderness is dangerous, but it provides for those who know its ways."));
    Line1.SpeakerArchetype = ENarr_CharacterArchetype::TribalHunter;
    Line1.DisplayDuration = 4.0f;

    FNarr_DialogueLine Line2;
    Line2.SpeakerName = TEXT("Tribal Hunter");
    Line2.DialogueText = FText::FromString(TEXT("The great beasts roam these lands. Learn their patterns, respect their territory, and you might survive another day."));
    Line2.SpeakerArchetype = ENarr_CharacterArchetype::TribalHunter;
    Line2.DisplayDuration = 4.0f;

    HunterTree.DialogueLines.Add(Line1);
    HunterTree.DialogueLines.Add(Line2);

    if (DialogueComponent)
    {
        DialogueComponent->RegisterDialogueTree(HunterTree);
    }
}

void ANarr_DialogueManager::CreateTribalGathererDialogue()
{
    FNarr_DialogueTree GathererTree;
    GathererTree.TreeName = TEXT("TribalGatherer_Introduction");
    GathererTree.TreeType = ENarr_DialogueNodeType::Standard;
    GathererTree.TriggerCondition = ENarr_DialogueTrigger::Proximity;

    FNarr_DialogueLine Line1;
    Line1.SpeakerName = TEXT("Tribal Gatherer");
    Line1.DialogueText = FText::FromString(TEXT("Welcome, fellow survivor. I am the Gatherer, keeper of knowledge about the resources of this ancient world."));
    Line1.SpeakerArchetype = ENarr_CharacterArchetype::TribalGatherer;
    Line1.DisplayDuration = 4.0f;

    FNarr_DialogueLine Line2;
    Line2.SpeakerName = TEXT("Tribal Gatherer");
    Line2.DialogueText = FText::FromString(TEXT("The plants and stones hold secrets. Learn which berries nourish and which bring death. Knowledge is survival."));
    Line2.SpeakerArchetype = ENarr_CharacterArchetype::TribalGatherer;
    Line2.DisplayDuration = 4.0f;

    GathererTree.DialogueLines.Add(Line1);
    GathererTree.DialogueLines.Add(Line2);

    if (DialogueComponent)
    {
        DialogueComponent->RegisterDialogueTree(GathererTree);
    }
}

void ANarr_DialogueManager::CreateTribalElderDialogue()
{
    FNarr_DialogueTree ElderTree;
    ElderTree.TreeName = TEXT("TribalElder_Wisdom");
    ElderTree.TreeType = ENarr_DialogueNodeType::Lore;
    ElderTree.TriggerCondition = ENarr_DialogueTrigger::Proximity;

    FNarr_DialogueLine Line1;
    Line1.SpeakerName = TEXT("Tribal Elder");
    Line1.DialogueText = FText::FromString(TEXT("In the time before memory, our ancestors learned the sacred laws of survival. Listen well, young one."));
    Line1.SpeakerArchetype = ENarr_CharacterArchetype::TribalElder;
    Line1.DisplayDuration = 4.0f;

    FNarr_DialogueLine Line2;
    Line2.SpeakerName = TEXT("Tribal Elder");
    Line2.DialogueText = FText::FromString(TEXT("Hunt only what you need. Respect the territory of the great beasts. And never venture into the deep valley alone after sunset."));
    Line2.SpeakerArchetype = ENarr_CharacterArchetype::TribalElder;
    Line2.DisplayDuration = 5.0f;

    ElderTree.DialogueLines.Add(Line1);
    ElderTree.DialogueLines.Add(Line2);

    if (DialogueComponent)
    {
        DialogueComponent->RegisterDialogueTree(ElderTree);
    }
}

void ANarr_DialogueManager::CreateNarratorDialogue()
{
    FNarr_DialogueTree NarratorTree;
    NarratorTree.TreeName = TEXT("Narrator_ValleyIntroduction");
    NarratorTree.TreeType = ENarr_DialogueNodeType::Standard;
    NarratorTree.TriggerCondition = ENarr_DialogueTrigger::LocationBased;

    FNarr_DialogueLine Line1;
    Line1.SpeakerName = TEXT("Narrator");
    Line1.DialogueText = FText::FromString(TEXT("The ancient valley holds many secrets, survivor. Listen carefully to the wind - it carries the scent of predators and the whispers of danger."));
    Line1.SpeakerArchetype = ENarr_CharacterArchetype::Narrator;
    Line1.DisplayDuration = 5.0f;

    FNarr_DialogueLine Line2;
    Line2.SpeakerName = TEXT("Narrator");
    Line2.DialogueText = FText::FromString(TEXT("Pack hunters move in coordinated patterns. Their intelligence should not be underestimated. Stay alert, stay alive."));
    Line2.SpeakerArchetype = ENarr_CharacterArchetype::Narrator;
    Line2.DisplayDuration = 4.0f;

    NarratorTree.DialogueLines.Add(Line1);
    NarratorTree.DialogueLines.Add(Line2);

    if (DialogueComponent)
    {
        DialogueComponent->RegisterDialogueTree(NarratorTree);
    }
}