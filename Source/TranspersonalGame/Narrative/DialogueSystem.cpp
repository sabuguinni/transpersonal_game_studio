// DialogueSystem.cpp
// Narrative & Dialogue Agent #15 — Transpersonal Game Studio
// PROD_CYCLE_AUTO_20260623_005

#include "DialogueSystem.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

// ─── ANarr_DialogueTrigger ─────────────────────────────────────────────────────

ANarr_DialogueTrigger::ANarr_DialogueTrigger()
{
    PrimaryActorTick.bCanEverTick = false;

    TriggerVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerVolume"));
    RootComponent = TriggerVolume;
    TriggerVolume->SetBoxExtent(FVector(200.0f, 200.0f, 100.0f));
    TriggerVolume->SetCollisionProfileName(TEXT("Trigger"));

    TriggerType = ENarr_DialogueType::Ambient;
    TriggerRadius = 200.0f;
    bTriggerOnce = true;
    bTriggered = false;
}

void ANarr_DialogueTrigger::BeginPlay()
{
    Super::BeginPlay();
    TriggerVolume->OnActorBeginOverlap.AddDynamic(this, &ANarr_DialogueTrigger::OnPlayerEnterTrigger);
}

void ANarr_DialogueTrigger::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ANarr_DialogueTrigger::OnPlayerEnterTrigger(AActor* OverlappedActor, AActor* OtherActor)
{
    if (!OtherActor)
    {
        return;
    }

    // Only trigger for player-controlled pawns
    if (!OtherActor->IsA(APawn::StaticClass()))
    {
        return;
    }

    if (bTriggerOnce && bTriggered)
    {
        return;
    }

    PlayDialogueSequence();
}

void ANarr_DialogueTrigger::PlayDialogueSequence()
{
    if (DialogueSequence.Lines.Num() == 0)
    {
        return;
    }

    if (bTriggerOnce && bTriggered)
    {
        return;
    }

    bTriggered = true;
    DialogueSequence.bHasBeenPlayed = true;

    // Get the dialogue manager and trigger the sequence
    if (UWorld* World = GetWorld())
    {
        if (UNarr_DialogueManager* Manager = World->GetSubsystem<UNarr_DialogueManager>())
        {
            Manager->TriggerDialogue(DialogueSequence.SequenceID);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("DialogueTrigger: Playing sequence '%s' with %d lines"),
        *DialogueSequence.SequenceID.ToString(),
        DialogueSequence.Lines.Num());
}

bool ANarr_DialogueTrigger::HasBeenTriggered() const
{
    return bTriggered;
}

void ANarr_DialogueTrigger::ResetTrigger()
{
    bTriggered = false;
    DialogueSequence.bHasBeenPlayed = false;
}

// ─── ANarr_NPCQuestGiver ──────────────────────────────────────────────────────

ANarr_NPCQuestGiver::ANarr_NPCQuestGiver()
{
    PrimaryActorTick.bCanEverTick = false;

    InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
    RootComponent = InteractionSphere;
    InteractionSphere->SetSphereRadius(300.0f);
    InteractionSphere->SetCollisionProfileName(TEXT("Trigger"));

    NPCName = TEXT("Elder Hunter");
    NPCRole = ENarr_NPCRole::TribalElder;
    InteractionRange = 300.0f;
    CurrentDialogueIndex = 0;
    CurrentLineIndex = 0;
    bInDialogue = false;
}

void ANarr_NPCQuestGiver::BeginPlay()
{
    Super::BeginPlay();

    // Populate default dialogue for the tribal elder
    if (AvailableDialogues.Num() == 0 && NPCRole == ENarr_NPCRole::TribalElder)
    {
        FNarr_DialogueSequence IntroSeq;
        IntroSeq.SequenceID = FName("Elder_Intro");
        IntroSeq.bCanRepeat = false;

        FNarr_DialogueLine Line1;
        Line1.SpeakerName = NPCName;
        Line1.LineText = TEXT("You are new here. The jungle does not forgive the careless.");
        Line1.DialogueType = ENarr_DialogueType::Ambient;
        Line1.DisplayDuration = 4.0f;

        FNarr_DialogueLine Line2;
        Line2.SpeakerName = NPCName;
        Line2.LineText = TEXT("The TRex hunts the eastern valley at dusk. Stay away from there until you have better weapons.");
        Line2.DialogueType = ENarr_DialogueType::Warning;
        Line2.DisplayDuration = 5.0f;

        FNarr_DialogueLine Line3;
        Line3.SpeakerName = NPCName;
        Line3.LineText = TEXT("Find flint near the river. Make a blade. Then we talk about the raptors.");
        Line3.DialogueType = ENarr_DialogueType::QuestGive;
        Line3.DisplayDuration = 5.0f;

        IntroSeq.Lines.Add(Line1);
        IntroSeq.Lines.Add(Line2);
        IntroSeq.Lines.Add(Line3);
        AvailableDialogues.Add(IntroSeq);

        // Danger warning sequence
        FNarr_DialogueSequence DangerSeq;
        DangerSeq.SequenceID = FName("Elder_DangerWarning");
        DangerSeq.bCanRepeat = true;

        FNarr_DialogueLine DLine1;
        DLine1.SpeakerName = NPCName;
        DLine1.LineText = TEXT("I hear the ground shake. Something big is moving. Get to cover — now.");
        DLine1.DialogueType = ENarr_DialogueType::Danger;
        DLine1.DisplayDuration = 4.0f;

        DangerSeq.Lines.Add(DLine1);
        AvailableDialogues.Add(DangerSeq);
    }
}

void ANarr_NPCQuestGiver::Interact(AActor* Interactor)
{
    if (!Interactor)
    {
        return;
    }

    if (AvailableDialogues.Num() == 0)
    {
        return;
    }

    bInDialogue = true;
    CurrentLineIndex = 0;

    // Find first unplayed dialogue
    for (int32 i = 0; i < AvailableDialogues.Num(); ++i)
    {
        if (!AvailableDialogues[i].bHasBeenPlayed || AvailableDialogues[i].bCanRepeat)
        {
            CurrentDialogueIndex = i;
            break;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("NPC '%s' interaction started. Dialogue: '%s'"),
        *NPCName,
        *AvailableDialogues[CurrentDialogueIndex].SequenceID.ToString());
}

FNarr_DialogueLine ANarr_NPCQuestGiver::GetCurrentLine() const
{
    if (AvailableDialogues.Num() == 0)
    {
        return FNarr_DialogueLine();
    }

    const FNarr_DialogueSequence& Seq = AvailableDialogues[CurrentDialogueIndex];
    if (CurrentLineIndex < Seq.Lines.Num())
    {
        return Seq.Lines[CurrentLineIndex];
    }

    return FNarr_DialogueLine();
}

void ANarr_NPCQuestGiver::AdvanceDialogue()
{
    if (!bInDialogue || AvailableDialogues.Num() == 0)
    {
        return;
    }

    FNarr_DialogueSequence& Seq = AvailableDialogues[CurrentDialogueIndex];
    CurrentLineIndex++;

    if (CurrentLineIndex >= Seq.Lines.Num())
    {
        // End of dialogue
        Seq.bHasBeenPlayed = true;
        bInDialogue = false;
        CurrentLineIndex = 0;
        UE_LOG(LogTemp, Log, TEXT("NPC '%s' dialogue sequence complete."), *NPCName);
    }
}

bool ANarr_NPCQuestGiver::IsPlayerInRange(AActor* Player) const
{
    if (!Player)
    {
        return false;
    }

    float Distance = FVector::Dist(GetActorLocation(), Player->GetActorLocation());
    return Distance <= InteractionRange;
}

// ─── UNarr_DialogueManager ────────────────────────────────────────────────────

void UNarr_DialogueManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    ActiveLineIndex = 0;
    bDialogueActive = false;
    UE_LOG(LogTemp, Log, TEXT("NarrDialogueManager: Initialized."));
}

void UNarr_DialogueManager::Deinitialize()
{
    RegisteredTriggers.Empty();
    bDialogueActive = false;
    Super::Deinitialize();
}

void UNarr_DialogueManager::RegisterDialogueTrigger(ANarr_DialogueTrigger* Trigger)
{
    if (Trigger && !RegisteredTriggers.Contains(Trigger))
    {
        RegisteredTriggers.Add(Trigger);
        UE_LOG(LogTemp, Log, TEXT("NarrDialogueManager: Registered trigger '%s'"),
            *Trigger->GetActorLabel());
    }
}

void UNarr_DialogueManager::TriggerDialogue(FName SequenceID)
{
    if (bDialogueActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("NarrDialogueManager: Dialogue already active, ignoring '%s'"),
            *SequenceID.ToString());
        return;
    }

    // Find the sequence in registered triggers
    for (ANarr_DialogueTrigger* Trigger : RegisteredTriggers)
    {
        if (!Trigger)
        {
            continue;
        }

        if (Trigger->DialogueSequence.SequenceID == SequenceID)
        {
            ActiveSequence = Trigger->DialogueSequence;
            ActiveLineIndex = 0;
            bDialogueActive = true;
            UE_LOG(LogTemp, Log, TEXT("NarrDialogueManager: Started dialogue '%s' (%d lines)"),
                *SequenceID.ToString(),
                ActiveSequence.Lines.Num());
            return;
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("NarrDialogueManager: Sequence '%s' not found in registered triggers"),
        *SequenceID.ToString());
}

bool UNarr_DialogueManager::IsDialogueActive() const
{
    return bDialogueActive;
}

FNarr_DialogueLine UNarr_DialogueManager::GetActiveDialogueLine() const
{
    if (!bDialogueActive || ActiveLineIndex >= ActiveSequence.Lines.Num())
    {
        return FNarr_DialogueLine();
    }

    return ActiveSequence.Lines[ActiveLineIndex];
}

void UNarr_DialogueManager::AdvanceActiveDialogue()
{
    if (!bDialogueActive)
    {
        return;
    }

    ActiveLineIndex++;

    if (ActiveLineIndex >= ActiveSequence.Lines.Num())
    {
        EndActiveDialogue();
    }
}

void UNarr_DialogueManager::EndActiveDialogue()
{
    bDialogueActive = false;
    ActiveLineIndex = 0;
    UE_LOG(LogTemp, Log, TEXT("NarrDialogueManager: Dialogue ended."));
}
