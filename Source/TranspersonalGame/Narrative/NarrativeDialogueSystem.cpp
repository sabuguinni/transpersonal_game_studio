#include "NarrativeDialogueSystem.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

// ============================================================
// ANarr_DialogueTriggerActor
// ============================================================

ANarr_DialogueTriggerActor::ANarr_DialogueTriggerActor()
{
    PrimaryActorTick.bCanEverTick = true;

    TriggerType = ENarr_DialogueTriggerType::QuestStart;
    ActivationRadius = 300.0f;
    bOneShot = true;
    QuestTimerThreshold = 1.0f;
    CurrentState = ENarr_DialogueState::Inactive;
    bHasTriggered = false;
    CurrentLineIndex = 0;
}

void ANarr_DialogueTriggerActor::BeginPlay()
{
    Super::BeginPlay();
    PopulateDefaultLines();
}

void ANarr_DialogueTriggerActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Auto-trigger on proximity for QuestStart type
    if (TriggerType == ENarr_DialogueTriggerType::QuestStart
        && !bHasTriggered
        && CurrentState == ENarr_DialogueState::Inactive)
    {
        if (IsPlayerInRange())
        {
            TriggerDialogue();
        }
    }
}

void ANarr_DialogueTriggerActor::TriggerDialogue()
{
    if (bOneShot && bHasTriggered)
    {
        return;
    }

    if (DialogueLines.Num() == 0)
    {
        return;
    }

    CurrentState = ENarr_DialogueState::Playing;
    bHasTriggered = true;
    CurrentLineIndex = 0;

    OnDialogueStarted(DialogueLines[0]);

    // Schedule auto-advance after display duration
    if (DialogueLines.Num() > 1)
    {
        float Duration = DialogueLines[0].DisplayDuration;
        GetWorldTimerManager().SetTimer(
            LineAdvanceTimer,
            this,
            &ANarr_DialogueTriggerActor::AdvanceLineInternal,
            Duration,
            false
        );
    }
    else
    {
        // Single line — complete after duration
        float Duration = DialogueLines[0].DisplayDuration;
        FTimerHandle CompleteTimer;
        GetWorldTimerManager().SetTimer(
            CompleteTimer,
            [this]()
            {
                CurrentState = ENarr_DialogueState::Completed;
                OnDialogueCompleted();
            },
            Duration,
            false
        );
    }
}

void ANarr_DialogueTriggerActor::AdvanceLine()
{
    AdvanceLineInternal();
}

void ANarr_DialogueTriggerActor::AdvanceLineInternal()
{
    GetWorldTimerManager().ClearTimer(LineAdvanceTimer);

    CurrentLineIndex++;

    if (CurrentLineIndex >= DialogueLines.Num())
    {
        CurrentState = ENarr_DialogueState::Completed;
        OnDialogueCompleted();
        return;
    }

    OnLineAdvanced(DialogueLines[CurrentLineIndex]);

    float Duration = DialogueLines[CurrentLineIndex].DisplayDuration;
    GetWorldTimerManager().SetTimer(
        LineAdvanceTimer,
        this,
        &ANarr_DialogueTriggerActor::AdvanceLineInternal,
        Duration,
        false
    );
}

void ANarr_DialogueTriggerActor::InterruptDialogue()
{
    GetWorldTimerManager().ClearTimer(LineAdvanceTimer);
    CurrentState = ENarr_DialogueState::Interrupted;
}

void ANarr_DialogueTriggerActor::ResetTrigger()
{
    GetWorldTimerManager().ClearTimer(LineAdvanceTimer);
    CurrentState = ENarr_DialogueState::Inactive;
    bHasTriggered = false;
    CurrentLineIndex = 0;
}

bool ANarr_DialogueTriggerActor::IsPlayerInRange() const
{
    ACharacter* Player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (!Player)
    {
        return false;
    }

    float Distance = FVector::Dist(GetActorLocation(), Player->GetActorLocation());
    return Distance <= ActivationRadius;
}

FNarr_DialogueLine ANarr_DialogueTriggerActor::GetCurrentLine() const
{
    if (CurrentLineIndex >= 0 && CurrentLineIndex < DialogueLines.Num())
    {
        return DialogueLines[CurrentLineIndex];
    }
    return FNarr_DialogueLine();
}

bool ANarr_DialogueTriggerActor::IsDialogueActive() const
{
    return CurrentState == ENarr_DialogueState::Playing;
}

void ANarr_DialogueTriggerActor::PopulateDefaultLines()
{
    if (DialogueLines.Num() > 0)
    {
        return; // Already configured in editor
    }

    // Populate default lines based on trigger type
    switch (TriggerType)
    {
        case ENarr_DialogueTriggerType::QuestStart:
        {
            FNarr_DialogueLine Line1;
            Line1.SpeakerName = TEXT("Elder");
            Line1.LineText = TEXT("The herd is moving. If they reach the camp, we lose everything.");
            Line1.DisplayDuration = 4.5f;
            Line1.TriggerType = ENarr_DialogueTriggerType::QuestStart;
            DialogueLines.Add(Line1);

            FNarr_DialogueLine Line2;
            Line2.SpeakerName = TEXT("Elder");
            Line2.LineText = TEXT("Get to high ground. Now. Do not stop running.");
            Line2.DisplayDuration = 3.5f;
            Line2.TriggerType = ENarr_DialogueTriggerType::QuestStart;
            DialogueLines.Add(Line2);
            break;
        }

        case ENarr_DialogueTriggerType::UrgencyMid:
        {
            FNarr_DialogueLine Line1;
            Line1.SpeakerName = TEXT("Hunter");
            Line1.LineText = TEXT("Hurry! The ground is shaking — they are close!");
            Line1.DisplayDuration = 3.0f;
            Line1.TriggerType = ENarr_DialogueTriggerType::UrgencyMid;
            DialogueLines.Add(Line1);
            break;
        }

        case ENarr_DialogueTriggerType::UrgencyFinal:
        {
            FNarr_DialogueLine Line1;
            Line1.SpeakerName = TEXT("Hunter");
            Line1.LineText = TEXT("Almost there! Keep moving!");
            Line1.DisplayDuration = 2.5f;
            Line1.TriggerType = ENarr_DialogueTriggerType::UrgencyFinal;
            DialogueLines.Add(Line1);
            break;
        }

        case ENarr_DialogueTriggerType::Victory:
        {
            FNarr_DialogueLine Line1;
            Line1.SpeakerName = TEXT("Elder");
            Line1.LineText = TEXT("You made it. The herd passed below. The camp still stands.");
            Line1.DisplayDuration = 5.0f;
            Line1.TriggerType = ENarr_DialogueTriggerType::Victory;
            DialogueLines.Add(Line1);

            FNarr_DialogueLine Line2;
            Line2.SpeakerName = TEXT("Elder");
            Line2.LineText = TEXT("You are faster than I thought. Good.");
            Line2.DisplayDuration = 3.5f;
            Line2.TriggerType = ENarr_DialogueTriggerType::Victory;
            DialogueLines.Add(Line2);
            break;
        }

        case ENarr_DialogueTriggerType::Failure:
        {
            FNarr_DialogueLine Line1;
            Line1.SpeakerName = TEXT("Elder");
            Line1.LineText = TEXT("The herd took everything. We rebuild. We survive. That is what we do.");
            Line1.DisplayDuration = 5.0f;
            Line1.TriggerType = ENarr_DialogueTriggerType::Failure;
            DialogueLines.Add(Line1);
            break;
        }

        default:
            break;
    }
}

// ============================================================
// ANarr_StampedeNarrativeManager
// ============================================================

ANarr_StampedeNarrativeManager::ANarr_StampedeNarrativeManager()
{
    PrimaryActorTick.bCanEverTick = false;

    UrgencyMidThreshold = 0.5f;
    UrgencyFinalThreshold = 0.2f;
    bMidUrgencyFired = false;
    bFinalUrgencyFired = false;
}

void ANarr_StampedeNarrativeManager::BeginPlay()
{
    Super::BeginPlay();

    // Auto-discover trigger actors in the level
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ANarr_DialogueTriggerActor::StaticClass(), FoundActors);
    for (AActor* Actor : FoundActors)
    {
        ANarr_DialogueTriggerActor* Trigger = Cast<ANarr_DialogueTriggerActor>(Actor);
        if (Trigger)
        {
            RegisteredTriggers.AddUnique(Trigger);
        }
    }
}

void ANarr_StampedeNarrativeManager::OnQuestStarted()
{
    bMidUrgencyFired = false;
    bFinalUrgencyFired = false;
    FireTriggerByType(ENarr_DialogueTriggerType::QuestStart);
}

void ANarr_StampedeNarrativeManager::OnQuestTimerUpdated(float NormalizedTimeRemaining)
{
    if (!bMidUrgencyFired && NormalizedTimeRemaining <= UrgencyMidThreshold)
    {
        bMidUrgencyFired = true;
        FireTriggerByType(ENarr_DialogueTriggerType::UrgencyMid);
    }

    if (!bFinalUrgencyFired && NormalizedTimeRemaining <= UrgencyFinalThreshold)
    {
        bFinalUrgencyFired = true;
        FireTriggerByType(ENarr_DialogueTriggerType::UrgencyFinal);
    }
}

void ANarr_StampedeNarrativeManager::OnQuestSucceeded()
{
    FireTriggerByType(ENarr_DialogueTriggerType::Victory);
}

void ANarr_StampedeNarrativeManager::OnQuestFailed()
{
    FireTriggerByType(ENarr_DialogueTriggerType::Failure);
}

void ANarr_StampedeNarrativeManager::RegisterTrigger(ANarr_DialogueTriggerActor* Trigger)
{
    if (Trigger)
    {
        RegisteredTriggers.AddUnique(Trigger);
    }
}

void ANarr_StampedeNarrativeManager::FireTriggerByType(ENarr_DialogueTriggerType Type)
{
    for (ANarr_DialogueTriggerActor* Trigger : RegisteredTriggers)
    {
        if (Trigger && Trigger->TriggerType == Type)
        {
            Trigger->TriggerDialogue();
        }
    }
}
