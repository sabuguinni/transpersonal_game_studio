#include "NarrativeDialogueSystem.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Components/BoxComponent.h"
#include "Components/AudioComponent.h"

// ── ANarr_DialogueTriggerActor ────────────────────────────────────────────────

ANarr_DialogueTriggerActor::ANarr_DialogueTriggerActor()
{
    PrimaryActorTick.bCanEverTick = true;

    TriggerVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerVolume"));
    TriggerVolume->SetBoxExtent(FVector(500.0f, 500.0f, 200.0f));
    TriggerVolume->SetCollisionProfileName(TEXT("Trigger"));
    RootComponent = TriggerVolume;

    AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
    AudioComponent->SetupAttachment(RootComponent);
    AudioComponent->bAutoActivate = false;
}

void ANarr_DialogueTriggerActor::BeginPlay()
{
    Super::BeginPlay();

    TriggerVolume->OnComponentBeginOverlap.AddDynamic(
        this, &ANarr_DialogueTriggerActor::OnTriggerOverlapBegin);

    // Update box extent to match configured radius
    TriggerVolume->SetBoxExtent(FVector(TriggerRadius, TriggerRadius, 200.0f));

    // Register with dialogue manager
    if (UGameInstance* GI = GetGameInstance())
    {
        if (UNarr_DialogueManager* Manager = GI->GetSubsystem<UNarr_DialogueManager>())
        {
            Manager->RegisterDialogueTrigger(this);
        }
    }
}

void ANarr_DialogueTriggerActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Handle cooldown
    if (bOnCooldown)
    {
        CooldownTimer -= DeltaTime;
        if (CooldownTimer <= 0.0f)
        {
            bOnCooldown = false;
            CooldownTimer = 0.0f;
        }
    }

    // Advance line timer during active dialogue
    if (bIsPlaying && DialogueSequence.Lines.IsValidIndex(CurrentLineIndex))
    {
        const FNarr_DialogueLine& CurrentLine = DialogueSequence.Lines[CurrentLineIndex];

        // If not waiting for audio or audio has finished, use display duration timer
        if (!CurrentLine.bWaitForAudioEnd || !AudioComponent->IsPlaying())
        {
            LineTimer += DeltaTime;
            if (LineTimer >= CurrentLine.DisplayDuration)
            {
                AdvanceLine();
            }
        }
    }
}

void ANarr_DialogueTriggerActor::TriggerDialogue()
{
    if (bIsPlaying) return;
    if (bOnCooldown) return;
    if (DialogueSequence.bPlayOnce && DialogueSequence.bHasPlayed) return;
    if (DialogueSequence.Lines.Num() == 0) return;

    bIsPlaying = true;
    CurrentLineIndex = 0;
    LineTimer = 0.0f;

    const FNarr_DialogueLine& FirstLine = DialogueSequence.Lines[0];

    // Play audio if available
    if (FirstLine.VoiceAudio && AudioComponent)
    {
        AudioComponent->SetSound(FirstLine.VoiceAudio);
        AudioComponent->Play();
    }

    OnDialogueLineStarted(FirstLine, CurrentLineIndex);

    UE_LOG(LogTemp, Log, TEXT("[Narrative] Dialogue started: %s — '%s'"),
        *FirstLine.SpeakerName, *FirstLine.LineText);
}

void ANarr_DialogueTriggerActor::StopDialogue()
{
    bIsPlaying = false;
    CurrentLineIndex = 0;
    LineTimer = 0.0f;

    if (AudioComponent && AudioComponent->IsPlaying())
    {
        AudioComponent->Stop();
    }
}

void ANarr_DialogueTriggerActor::AdvanceLine()
{
    LineTimer = 0.0f;
    CurrentLineIndex++;

    if (!DialogueSequence.Lines.IsValidIndex(CurrentLineIndex))
    {
        // Sequence complete
        bIsPlaying = false;
        DialogueSequence.bHasPlayed = true;

        // Start cooldown
        bOnCooldown = true;
        CooldownTimer = DialogueSequence.CooldownSeconds;

        // Mark in manager
        if (UGameInstance* GI = GetGameInstance())
        {
            if (UNarr_DialogueManager* Manager = GI->GetSubsystem<UNarr_DialogueManager>())
            {
                Manager->MarkSequencePlayed(DialogueSequence.SequenceID);
            }
        }

        OnDialogueSequenceComplete();
        UE_LOG(LogTemp, Log, TEXT("[Narrative] Dialogue sequence complete: %s"),
            *DialogueSequence.SequenceID.ToString());
        return;
    }

    // Play next line
    const FNarr_DialogueLine& NextLine = DialogueSequence.Lines[CurrentLineIndex];

    if (NextLine.VoiceAudio && AudioComponent)
    {
        AudioComponent->SetSound(NextLine.VoiceAudio);
        AudioComponent->Play();
    }

    OnDialogueLineStarted(NextLine, CurrentLineIndex);

    UE_LOG(LogTemp, Log, TEXT("[Narrative] Line %d: %s — '%s'"),
        CurrentLineIndex, *NextLine.SpeakerName, *NextLine.LineText);
}

bool ANarr_DialogueTriggerActor::IsDialogueComplete() const
{
    return !bIsPlaying && DialogueSequence.bHasPlayed;
}

FNarr_DialogueLine ANarr_DialogueTriggerActor::GetCurrentLine() const
{
    if (DialogueSequence.Lines.IsValidIndex(CurrentLineIndex))
    {
        return DialogueSequence.Lines[CurrentLineIndex];
    }
    return FNarr_DialogueLine();
}

void ANarr_DialogueTriggerActor::ResetDialogue()
{
    bIsPlaying = false;
    bOnCooldown = false;
    CooldownTimer = 0.0f;
    CurrentLineIndex = 0;
    LineTimer = 0.0f;
    DialogueSequence.bHasPlayed = false;
}

void ANarr_DialogueTriggerActor::OnTriggerOverlapBegin(
    UPrimitiveComponent* OverlappedComp,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    // Only trigger for player character
    if (!OtherActor) return;

    ACharacter* PlayerChar = Cast<ACharacter>(OtherActor);
    if (!PlayerChar) return;

    // Only trigger for locally controlled player
    if (!PlayerChar->IsPlayerControlled()) return;

    if (TriggerType == ENarr_DialogueTriggerType::Proximity)
    {
        OnPlayerEnterTrigger();
        TriggerDialogue();
    }
}

// ── UNarr_DialogueManager ─────────────────────────────────────────────────────

void UNarr_DialogueManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    UE_LOG(LogTemp, Log, TEXT("[Narrative] DialogueManager initialized"));
}

void UNarr_DialogueManager::Deinitialize()
{
    RegisteredTriggers.Empty();
    PlayedSequenceIDs.Empty();
    Super::Deinitialize();
}

void UNarr_DialogueManager::RegisterDialogueTrigger(ANarr_DialogueTriggerActor* Trigger)
{
    if (Trigger && !RegisteredTriggers.Contains(Trigger))
    {
        RegisteredTriggers.Add(Trigger);
        UE_LOG(LogTemp, Log, TEXT("[Narrative] Registered trigger: %s"), *Trigger->GetName());
    }
}

void UNarr_DialogueManager::UnregisterDialogueTrigger(ANarr_DialogueTriggerActor* Trigger)
{
    RegisteredTriggers.Remove(Trigger);
}

void UNarr_DialogueManager::BroadcastGameEvent(ENarr_DialogueTriggerType EventType)
{
    for (ANarr_DialogueTriggerActor* Trigger : RegisteredTriggers)
    {
        if (!Trigger) continue;
        if (Trigger->TriggerType == EventType)
        {
            Trigger->TriggerDialogue();
        }
    }
}

int32 UNarr_DialogueManager::GetActiveDialogueCount() const
{
    int32 Count = 0;
    for (const ANarr_DialogueTriggerActor* Trigger : RegisteredTriggers)
    {
        if (Trigger && Trigger->bIsPlaying)
        {
            Count++;
        }
    }
    return Count;
}

TArray<FName> UNarr_DialogueManager::GetPlayedSequenceIDs() const
{
    return PlayedSequenceIDs.Array();
}

void UNarr_DialogueManager::MarkSequencePlayed(FName SequenceID)
{
    PlayedSequenceIDs.Add(SequenceID);
}

bool UNarr_DialogueManager::HasSequencePlayed(FName SequenceID) const
{
    return PlayedSequenceIDs.Contains(SequenceID);
}
