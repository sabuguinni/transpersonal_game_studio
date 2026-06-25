// DialogueSystem.cpp
// Agent #15 — Narrative & Dialogue Agent
// Cycle: PROD_CYCLE_AUTO_20260625_011

#include "DialogueSystem.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"

// ─────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────

ANarr_DialogueActor::ANarr_DialogueActor()
{
    PrimaryActorTick.bCanEverTick = true;

    // Interaction sphere — player proximity detection
    InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
    InteractionSphere->SetSphereRadius(300.0f);
    InteractionSphere->SetCollisionProfileName(TEXT("Trigger"));
    RootComponent = InteractionSphere;

    // Defaults
    InteractionRadius = 300.0f;
    bIsPlayerInRange = false;
    CurrentDialogueIndex = 0;
}

// ─────────────────────────────────────────────────────────────
// BeginPlay
// ─────────────────────────────────────────────────────────────

void ANarr_DialogueActor::BeginPlay()
{
    Super::BeginPlay();

    // Bind overlap events
    InteractionSphere->OnComponentBeginOverlap.AddDynamic(
        this, &ANarr_DialogueActor::OnPlayerEnterRange);
    InteractionSphere->OnComponentEndOverlap.AddDynamic(
        this, &ANarr_DialogueActor::OnPlayerExitRange);

    // Populate default dialogue if none set in editor
    if (NPCData.DialogueLines.Num() == 0)
    {
        InitialiseDefaultDialogue();
    }
}

// ─────────────────────────────────────────────────────────────
// Tick
// ─────────────────────────────────────────────────────────────

void ANarr_DialogueActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    // Tick reserved for future dialogue timing logic
}

// ─────────────────────────────────────────────────────────────
// Dialogue Logic
// ─────────────────────────────────────────────────────────────

void ANarr_DialogueActor::TriggerDialogue(ENarr_DialogueTrigger Trigger)
{
    if (NPCData.DialogueLines.Num() == 0)
    {
        return;
    }

    // Find first line matching this trigger
    for (int32 i = 0; i < NPCData.DialogueLines.Num(); ++i)
    {
        if (NPCData.DialogueLines[i].TriggerCondition == Trigger)
        {
            CurrentDialogueIndex = i;
            NPCData.bHasBeenSpokenTo = true;
            return;
        }
    }

    // Fallback: start from beginning
    CurrentDialogueIndex = 0;
    NPCData.bHasBeenSpokenTo = true;
}

FNarr_DialogueLine ANarr_DialogueActor::GetCurrentDialogueLine() const
{
    if (NPCData.DialogueLines.IsValidIndex(CurrentDialogueIndex))
    {
        return NPCData.DialogueLines[CurrentDialogueIndex];
    }
    return FNarr_DialogueLine();
}

void ANarr_DialogueActor::AdvanceDialogue()
{
    if (CurrentDialogueIndex < NPCData.DialogueLines.Num() - 1)
    {
        ++CurrentDialogueIndex;
    }
}

bool ANarr_DialogueActor::HasMoreDialogue() const
{
    return CurrentDialogueIndex < NPCData.DialogueLines.Num() - 1;
}

void ANarr_DialogueActor::ResetDialogue()
{
    CurrentDialogueIndex = 0;
}

FString ANarr_DialogueActor::GetNPCName() const
{
    return NPCData.NPCName;
}

ENarr_NPCRole ANarr_DialogueActor::GetNPCRole() const
{
    return NPCData.Role;
}

// ─────────────────────────────────────────────────────────────
// Overlap Callbacks
// ─────────────────────────────────────────────────────────────

void ANarr_DialogueActor::OnPlayerEnterRange(
    UPrimitiveComponent* OverlappedComp,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    if (OtherActor && OtherActor->IsA<ACharacter>())
    {
        bIsPlayerInRange = true;
        TriggerDialogue(ENarr_DialogueTrigger::Proximity);
    }
}

void ANarr_DialogueActor::OnPlayerExitRange(
    UPrimitiveComponent* OverlappedComp,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex)
{
    if (OtherActor && OtherActor->IsA<ACharacter>())
    {
        bIsPlayerInRange = false;
    }
}

// ─────────────────────────────────────────────────────────────
// Default Dialogue Initialisation
// Hardcoded survival-focused lines for the three core NPCs
// ─────────────────────────────────────────────────────────────

void ANarr_DialogueActor::InitialiseDefaultDialogue()
{
    // Determine which NPC this is by role and populate accordingly
    switch (NPCData.Role)
    {
        case ENarr_NPCRole::Elder:
        {
            NPCData.NPCName = TEXT("Elder Kael");

            FNarr_DialogueLine Line1;
            Line1.SpeakerName = TEXT("Elder Kael");
            Line1.LineText = TEXT("The river crossing is safe at dawn. The great lizards sleep until the sun warms them. Cross fast, stay silent.");
            Line1.TriggerCondition = ENarr_DialogueTrigger::Proximity;
            Line1.DisplayDuration = 5.0f;
            NPCData.DialogueLines.Add(Line1);

            FNarr_DialogueLine Line2;
            Line2.SpeakerName = TEXT("Elder Kael");
            Line2.LineText = TEXT("Build your shelter before you build your weapons. Survival is not about strength. It is about preparation.");
            Line2.TriggerCondition = ENarr_DialogueTrigger::NightFall;
            Line2.DisplayDuration = 5.0f;
            NPCData.DialogueLines.Add(Line2);

            FNarr_DialogueLine Line3;
            Line3.SpeakerName = TEXT("Elder Kael");
            Line3.LineText = TEXT("We found the bones of twelve hunters near the northern ridge. Not killed by beasts — killed by cold.");
            Line3.TriggerCondition = ENarr_DialogueTrigger::PlayerLowHealth;
            Line3.DisplayDuration = 5.0f;
            NPCData.DialogueLines.Add(Line3);
            break;
        }

        case ENarr_NPCRole::Scout:
        {
            NPCData.NPCName = TEXT("Scout Mira");

            FNarr_DialogueLine Line1;
            Line1.SpeakerName = TEXT("Scout Mira");
            Line1.LineText = TEXT("Stay low. The pack hunts together — three raptors, maybe four. They will circle from the east while the big one drives you toward them.");
            Line1.TriggerCondition = ENarr_DialogueTrigger::DinosaurNearby;
            Line1.DisplayDuration = 6.0f;
            NPCData.DialogueLines.Add(Line1);

            FNarr_DialogueLine Line2;
            Line2.SpeakerName = TEXT("Scout Mira");
            Line2.LineText = TEXT("Do not run. Find high ground, use the rocks. A spear through the throat will stop even the largest one, but only if your aim is true.");
            Line2.TriggerCondition = ENarr_DialogueTrigger::Proximity;
            Line2.DisplayDuration = 5.0f;
            NPCData.DialogueLines.Add(Line2);
            break;
        }

        case ENarr_NPCRole::Hunter:
        {
            NPCData.NPCName = TEXT("Hunter Brak");

            FNarr_DialogueLine Line1;
            Line1.SpeakerName = TEXT("Hunter Brak");
            Line1.LineText = TEXT("I tracked the Triceratops herd for three days. The young ones stay in the center, the big bulls on the outside. You cannot take one from the herd.");
            Line1.TriggerCondition = ENarr_DialogueTrigger::Proximity;
            Line1.DisplayDuration = 6.0f;
            NPCData.DialogueLines.Add(Line1);

            FNarr_DialogueLine Line2;
            Line2.SpeakerName = TEXT("Hunter Brak");
            Line2.LineText = TEXT("Wait for a straggler, an injured one, or drive one away from the group using fire and noise. Patience is the hunter's greatest weapon.");
            Line2.TriggerCondition = ENarr_DialogueTrigger::QuestStart;
            Line2.DisplayDuration = 5.0f;
            NPCData.DialogueLines.Add(Line2);
            break;
        }

        default:
        {
            FNarr_DialogueLine DefaultLine;
            DefaultLine.SpeakerName = TEXT("Tribesperson");
            DefaultLine.LineText = TEXT("Stay alert. The dinosaurs are active today.");
            DefaultLine.TriggerCondition = ENarr_DialogueTrigger::Proximity;
            DefaultLine.DisplayDuration = 3.0f;
            NPCData.DialogueLines.Add(DefaultLine);
            break;
        }
    }
}
