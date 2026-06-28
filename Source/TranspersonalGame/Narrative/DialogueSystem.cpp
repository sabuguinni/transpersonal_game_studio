#include "DialogueSystem.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "TimerManager.h"

// ============================================================
// ANarr_DialogueTrigger — Implementation
// ============================================================

ANarr_DialogueTrigger::ANarr_DialogueTrigger()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create trigger volume
    TriggerVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerVolume"));
    TriggerVolume->SetBoxExtent(FVector(200.0f, 200.0f, 100.0f));
    TriggerVolume->SetCollisionProfileName(TEXT("Trigger"));
    TriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &ANarr_DialogueTrigger::OnOverlapBegin);
    RootComponent = TriggerVolume;
}

void ANarr_DialogueTrigger::BeginPlay()
{
    Super::BeginPlay();
    DialogueState = ENarr_DialogueState::Idle;
    CurrentLineIndex = 0;
    TimeSinceLastDialogue = 0.0f;
}

void ANarr_DialogueTrigger::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Advance cooldown timer
    if (DialogueState == ENarr_DialogueState::Cooldown)
    {
        TimeSinceLastDialogue += DeltaTime;
        if (TimeSinceLastDialogue >= DialogueSequence.CooldownSeconds)
        {
            DialogueState = ENarr_DialogueState::Idle;
            TimeSinceLastDialogue = 0.0f;
        }
    }
}

void ANarr_DialogueTrigger::OnOverlapBegin(
    UPrimitiveComponent* OverlappedComp,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    // Only respond to player characters
    if (!OtherActor || !OtherActor->IsA(ACharacter::StaticClass()))
    {
        return;
    }

    if (DialogueState == ENarr_DialogueState::Idle)
    {
        OnPlayerEnterRange(OtherActor);
    }
}

void ANarr_DialogueTrigger::OnPlayerEnterRange_Implementation(AActor* PlayerActor)
{
    if (DialogueSequence.Lines.Num() == 0)
    {
        return;
    }

    DialogueState = ENarr_DialogueState::Greeting;
    bHasBeenActivated = true;
    CurrentLineIndex = 0;

    // Immediately transition to active
    DialogueState = ENarr_DialogueState::Active;
}

void ANarr_DialogueTrigger::OnDialogueLineComplete_Implementation(int32 LineIndex)
{
    AdvanceDialogue();
}

void ANarr_DialogueTrigger::AdvanceDialogue()
{
    if (DialogueSequence.Lines.Num() == 0)
    {
        return;
    }

    CurrentLineIndex++;

    if (CurrentLineIndex >= DialogueSequence.Lines.Num())
    {
        // Sequence complete
        DialogueState = ENarr_DialogueState::Completed;

        if (DialogueSequence.bRepeatable)
        {
            DialogueState = ENarr_DialogueState::Cooldown;
            TimeSinceLastDialogue = 0.0f;
        }
    }
}

void ANarr_DialogueTrigger::ResetDialogue()
{
    CurrentLineIndex = 0;
    TimeSinceLastDialogue = 0.0f;
    DialogueState = ENarr_DialogueState::Idle;
}

FText ANarr_DialogueTrigger::GetCurrentLineText() const
{
    if (DialogueSequence.Lines.IsValidIndex(CurrentLineIndex))
    {
        return DialogueSequence.Lines[CurrentLineIndex].LineText;
    }
    return FText::GetEmpty();
}

FString ANarr_DialogueTrigger::GetNPCDisplayName() const
{
    switch (NPCRole)
    {
        case ENarr_NPCRole::ElderTracker:   return TEXT("Elder Tracker");
        case ENarr_NPCRole::ChiefHunter:    return TEXT("Chief Hunter");
        case ENarr_NPCRole::Craftmaster:    return TEXT("Craftmaster");
        case ENarr_NPCRole::ScoutRanger:    return TEXT("Scout Ranger");
        default:                            return TEXT("Survivor");
    }
}
