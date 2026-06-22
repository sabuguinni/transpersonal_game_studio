#include "DialogueSystem.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"

// ============================================================
// UNarr_DialogueComponent
// ============================================================

UNarr_DialogueComponent::UNarr_DialogueComponent()
{
    PrimaryComponentTick.bCanEverTick = false;

    NPCRole = ENarr_NPCRole::Survivor;
    NPCName = TEXT("Unknown");
    InteractionRadius = 300.0f;
    CurrentState = ENarr_DialogueState::Idle;
    CurrentLineIndex = 0;
    ActiveTree = nullptr;
}

void UNarr_DialogueComponent::BeginPlay()
{
    Super::BeginPlay();
}

bool UNarr_DialogueComponent::StartDialogue(const FString& DialogueID)
{
    if (CurrentState == ENarr_DialogueState::Active)
    {
        return false;
    }

    for (FNarr_DialogueTree& Tree : DialogueTrees)
    {
        if (Tree.DialogueID == DialogueID)
        {
            ActiveTree = &Tree;
            CurrentLineIndex = 0;
            CurrentState = ENarr_DialogueState::Active;
            return true;
        }
    }

    return false;
}

bool UNarr_DialogueComponent::AdvanceDialogue()
{
    if (CurrentState != ENarr_DialogueState::Active || ActiveTree == nullptr)
    {
        return false;
    }

    CurrentLineIndex++;

    if (CurrentLineIndex >= ActiveTree->Lines.Num())
    {
        EndDialogue();
        return false;
    }

    return true;
}

void UNarr_DialogueComponent::EndDialogue()
{
    CurrentState = ENarr_DialogueState::Completed;
    CurrentLineIndex = 0;
    ActiveTree = nullptr;
}

FNarr_DialogueLine UNarr_DialogueComponent::GetCurrentLine() const
{
    if (ActiveTree == nullptr || CurrentLineIndex >= ActiveTree->Lines.Num())
    {
        return FNarr_DialogueLine();
    }

    return ActiveTree->Lines[CurrentLineIndex];
}

bool UNarr_DialogueComponent::IsPlayerInRange(AActor* Player) const
{
    if (!Player || !GetOwner())
    {
        return false;
    }

    const float Distance = FVector::Dist(Player->GetActorLocation(), GetOwner()->GetActorLocation());
    return Distance <= InteractionRadius;
}

void UNarr_DialogueComponent::AddDialogueTree(const FNarr_DialogueTree& NewTree)
{
    DialogueTrees.Add(NewTree);
}

// ============================================================
// ANarr_DialogueTrigger
// ============================================================

ANarr_DialogueTrigger::ANarr_DialogueTrigger()
{
    PrimaryActorTick.bCanEverTick = true;

    TriggerDialogueID = TEXT("");
    TriggerRadius = 400.0f;
    bOneShot = true;
    bHasTriggered = false;
}

void ANarr_DialogueTrigger::BeginPlay()
{
    Super::BeginPlay();
}

void ANarr_DialogueTrigger::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bOneShot && bHasTriggered)
    {
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Check for player proximity
    for (TActorIterator<AActor> It(World); It; ++It)
    {
        AActor* Actor = *It;
        if (Actor && Actor->ActorHasTag(TEXT("Player")))
        {
            const float Distance = FVector::Dist(Actor->GetActorLocation(), GetActorLocation());
            if (Distance <= TriggerRadius)
            {
                OnPlayerEnterRange(Actor);
                break;
            }
        }
    }
}

void ANarr_DialogueTrigger::OnPlayerEnterRange(AActor* Player)
{
    if (!Player)
    {
        return;
    }

    if (bOneShot)
    {
        bHasTriggered = true;
    }

    UE_LOG(LogTemp, Log, TEXT("DialogueTrigger: Player entered range — triggering dialogue '%s'"), *TriggerDialogueID);
}
