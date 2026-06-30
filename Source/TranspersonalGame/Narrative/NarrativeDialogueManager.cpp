#include "NarrativeDialogueManager.h"
#include "Engine/World.h"
#include "TimerManager.h"

// ─── Constructor ──────────────────────────────────────────────────────────────

ANarrativeDialogueManager::ANarrativeDialogueManager()
{
    PrimaryActorTick.bCanEverTick = true;

    DialogueCooldownSeconds = 3.0f;
    MaxQueueSize = 8;
    bEnableSubtitles = true;
    ProximityTriggerRadius = 800.0f;

    TimeSinceLastDialogue = 0.0f;
    CurrentLineTimer = 0.0f;
    bDialoguePlaying = false;
}

// ─── BeginPlay ────────────────────────────────────────────────────────────────

void ANarrativeDialogueManager::BeginPlay()
{
    Super::BeginPlay();
    UE_LOG(LogTemp, Log, TEXT("[NarrativeDialogueManager] Initialized — subtitles=%s, cooldown=%.1fs"),
        bEnableSubtitles ? TEXT("ON") : TEXT("OFF"), DialogueCooldownSeconds);
}

// ─── Tick ─────────────────────────────────────────────────────────────────────

void ANarrativeDialogueManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    TimeSinceLastDialogue += DeltaTime;

    if (bDialoguePlaying)
    {
        CurrentLineTimer -= DeltaTime;
        CurrentDialogue.RemainingTime = FMath::Max(0.0f, CurrentLineTimer);

        if (CurrentLineTimer <= 0.0f)
        {
            bDialoguePlaying = false;
            CurrentDialogue = FNarr_ActiveDialogue();
            AdvanceQueue();
        }
    }
}

// ─── TriggerDialogue ──────────────────────────────────────────────────────────

void ANarrativeDialogueManager::TriggerDialogue(ENarr_DialogueTrigger TriggerType,
                                                  AActor* InstigatorActor)
{
    if (TimeSinceLastDialogue < DialogueCooldownSeconds && !bDialoguePlaying)
    {
        return; // Still in cooldown
    }

    // Build a contextual line based on trigger type
    FString ContextText;
    ENarr_SpeakerRole Speaker = ENarr_SpeakerRole::Narrator;
    bool bUrgent = false;

    switch (TriggerType)
    {
        case ENarr_DialogueTrigger::DinosaurNearby:
            ContextText = TEXT("Something large is moving through the trees. Do not make a sound.");
            Speaker = ENarr_SpeakerRole::Scout;
            bUrgent = true;
            break;

        case ENarr_DialogueTrigger::PlayerInjured:
            ContextText = TEXT("You are bleeding. Find shelter and treat the wound before infection sets in.");
            Speaker = ENarr_SpeakerRole::Narrator;
            bUrgent = true;
            break;

        case ENarr_DialogueTrigger::NightFall:
            ContextText = TEXT("Night is coming. The predators grow bolder in the dark. Find high ground or a cave.");
            Speaker = ENarr_SpeakerRole::TribalElder;
            bUrgent = false;
            break;

        case ENarr_DialogueTrigger::ResourceFound:
            ContextText = TEXT("Good find. These materials will serve us well.");
            Speaker = ENarr_SpeakerRole::HunterLeader;
            bUrgent = false;
            break;

        case ENarr_DialogueTrigger::QuestStart:
            ContextText = TEXT("The task is clear. Move carefully and do not underestimate the terrain.");
            Speaker = ENarr_SpeakerRole::TribalElder;
            bUrgent = false;
            break;

        case ENarr_DialogueTrigger::QuestComplete:
            ContextText = TEXT("Well done. The tribe will remember this.");
            Speaker = ENarr_SpeakerRole::HunterLeader;
            bUrgent = false;
            break;

        case ENarr_DialogueTrigger::TribeEvent:
            ContextText = TEXT("The tribe has spoken. We move together.");
            Speaker = ENarr_SpeakerRole::TribalElder;
            bUrgent = false;
            break;

        default:
            ContextText = TEXT("Stay alert.");
            Speaker = ENarr_SpeakerRole::Narrator;
            bUrgent = false;
            break;
    }

    FNarr_DialogueLine Line = BuildSurvivalLine(TriggerType, ContextText, Speaker, bUrgent);
    QueueDialogueLine(Line);
}

// ─── PlayDialogueLine ─────────────────────────────────────────────────────────

void ANarrativeDialogueManager::PlayDialogueLine(const FNarr_DialogueLine& Line)
{
    bDialoguePlaying = true;
    CurrentLineTimer = Line.DisplayDuration;
    TimeSinceLastDialogue = 0.0f;

    CurrentDialogue.LineID = Line.LineID;
    CurrentDialogue.Text = Line.DialogueText;
    CurrentDialogue.Speaker = Line.Speaker;
    CurrentDialogue.RemainingTime = Line.DisplayDuration;
    CurrentDialogue.bIsUrgent = Line.bIsUrgent;

    UE_LOG(LogTemp, Log, TEXT("[NarrativeDialogueManager] Playing: [%s] \"%s\" (%.1fs)"),
        *Line.LineID, *Line.DialogueText, Line.DisplayDuration);
}

// ─── QueueDialogueLine ────────────────────────────────────────────────────────

void ANarrativeDialogueManager::QueueDialogueLine(const FNarr_DialogueLine& Line)
{
    if (DialogueQueue.Num() >= MaxQueueSize)
    {
        UE_LOG(LogTemp, Warning, TEXT("[NarrativeDialogueManager] Queue full — dropping line: %s"), *Line.LineID);
        return;
    }

    // Urgent lines jump to front of queue
    if (Line.bIsUrgent)
    {
        DialogueQueue.Insert(Line, 0);
    }
    else
    {
        DialogueQueue.Add(Line);
    }

    if (!bDialoguePlaying)
    {
        AdvanceQueue();
    }
}

// ─── ClearDialogueQueue ───────────────────────────────────────────────────────

void ANarrativeDialogueManager::ClearDialogueQueue()
{
    DialogueQueue.Empty();
    bDialoguePlaying = false;
    CurrentDialogue = FNarr_ActiveDialogue();
    CurrentLineTimer = 0.0f;
    UE_LOG(LogTemp, Log, TEXT("[NarrativeDialogueManager] Queue cleared."));
}

// ─── IsDialoguePlaying ────────────────────────────────────────────────────────

bool ANarrativeDialogueManager::IsDialoguePlaying() const
{
    return bDialoguePlaying;
}

// ─── GetCurrentDialogue ───────────────────────────────────────────────────────

FNarr_ActiveDialogue ANarrativeDialogueManager::GetCurrentDialogue() const
{
    return CurrentDialogue;
}

// ─── Survival Context Triggers ────────────────────────────────────────────────

void ANarrativeDialogueManager::OnDinosaurDetected(FName DinosaurSpecies, float DistanceMeters)
{
    bool bUrgent = (DistanceMeters < 300.0f);
    FString Text;

    if (DinosaurSpecies == FName("TRex"))
    {
        Text = bUrgent
            ? TEXT("Tyrannosaurus — close range! Do NOT run. Back away slowly and find cover NOW.")
            : TEXT("Tyrannosaurus tracks. It passed through here recently. Stay downwind.");
    }
    else if (DinosaurSpecies == FName("Raptor"))
    {
        Text = bUrgent
            ? TEXT("Raptors nearby — they hunt in packs. Climb or find a narrow space they cannot follow.")
            : TEXT("Raptor signs. They are scouting. Keep moving and do not linger.");
    }
    else if (DinosaurSpecies == FName("Brachiosaurus"))
    {
        Text = TEXT("A Brachiosaurus herd. They are peaceful unless startled. Move slowly around them.");
        bUrgent = false;
    }
    else if (DinosaurSpecies == FName("Triceratops"))
    {
        Text = bUrgent
            ? TEXT("Triceratops charging! Get behind a tree — it cannot turn fast.")
            : TEXT("Triceratops grazing ahead. Give them space and they will ignore you.");
    }
    else
    {
        Text = FString::Printf(TEXT("Unknown creature detected at %.0f meters. Approach with caution."), DistanceMeters);
    }

    FNarr_DialogueLine Line = BuildSurvivalLine(
        ENarr_DialogueTrigger::DinosaurNearby, Text,
        ENarr_SpeakerRole::Scout, bUrgent);
    QueueDialogueLine(Line);
}

void ANarrativeDialogueManager::OnPlayerHealthCritical(float HealthPercent)
{
    FString Text;
    if (HealthPercent < 0.15f)
    {
        Text = TEXT("Critical injury. You will not survive another hit. Find shelter immediately.");
    }
    else if (HealthPercent < 0.30f)
    {
        Text = TEXT("You are badly wounded. Use medicinal leaves or rest before continuing.");
    }
    else
    {
        Text = TEXT("You are hurt. Treat the wound when you have a safe moment.");
    }

    FNarr_DialogueLine Line = BuildSurvivalLine(
        ENarr_DialogueTrigger::PlayerInjured, Text,
        ENarr_SpeakerRole::Narrator, HealthPercent < 0.20f);
    QueueDialogueLine(Line);
}

void ANarrativeDialogueManager::OnNightfallApproaching(float HoursUntilDark)
{
    FString Text;
    if (HoursUntilDark < 1.0f)
    {
        Text = TEXT("Darkness is minutes away. You need shelter now — predators own the night.");
    }
    else if (HoursUntilDark < 2.0f)
    {
        Text = TEXT("The sun is low. Start looking for a cave or high ground to camp.");
    }
    else
    {
        Text = TEXT("Afternoon light. You have time, but do not waste it — nightfall comes fast.");
    }

    FNarr_DialogueLine Line = BuildSurvivalLine(
        ENarr_DialogueTrigger::NightFall, Text,
        ENarr_SpeakerRole::TribalElder, HoursUntilDark < 1.0f);
    QueueDialogueLine(Line);
}

void ANarrativeDialogueManager::OnResourceDiscovered(FName ResourceType, int32 Quantity)
{
    FString Text = FString::Printf(
        TEXT("Found %d units of %s. Gather what you can carry."),
        Quantity, *ResourceType.ToString());

    FNarr_DialogueLine Line = BuildSurvivalLine(
        ENarr_DialogueTrigger::ResourceFound, Text,
        ENarr_SpeakerRole::HunterLeader, false);
    QueueDialogueLine(Line);
}

void ANarrativeDialogueManager::OnTribeEventOccurred(FName EventID)
{
    FString Text = FString::Printf(
        TEXT("Tribe event: %s. The council has decided — we act together."),
        *EventID.ToString());

    FNarr_DialogueLine Line = BuildSurvivalLine(
        ENarr_DialogueTrigger::TribeEvent, Text,
        ENarr_SpeakerRole::TribalElder, false);
    QueueDialogueLine(Line);
}

// ─── Private Helpers ──────────────────────────────────────────────────────────

void ANarrativeDialogueManager::AdvanceQueue()
{
    if (DialogueQueue.Num() == 0 || bDialoguePlaying)
    {
        return;
    }

    FNarr_DialogueLine Next = DialogueQueue[0];
    DialogueQueue.RemoveAt(0);
    PlayDialogueLine(Next);
}

FNarr_DialogueLine ANarrativeDialogueManager::BuildSurvivalLine(
    ENarr_DialogueTrigger Trigger,
    const FString& ContextText,
    ENarr_SpeakerRole Speaker,
    bool bUrgent)
{
    FNarr_DialogueLine Line;
    Line.LineID = FString::Printf(TEXT("LINE_%d"), FMath::Rand());
    Line.DialogueText = ContextText;
    Line.Speaker = Speaker;
    Line.TriggerType = Trigger;
    Line.DisplayDuration = bUrgent ? 3.5f : 5.0f;
    Line.bIsUrgent = bUrgent;
    Line.AudioAssetPath = TEXT("");
    return Line;
}
