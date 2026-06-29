// NPCDialogueComponent.cpp
// Agent #15 — Narrative & Dialogue Agent
// Full implementation of NPC social dynamics and dialogue system

#include "NPCDialogueComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"

// ─── Constructor ──────────────────────────────────────────────────────────────

UNPCDialogueComponent::UNPCDialogueComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Check every second, not every frame

    NPCName = TEXT("Unknown");
    NPCRole = ENarr_NPCRole::Hunter;
    TribeName = TEXT("Valley Tribe");

    CurrentRelationship = ENarr_NPCRelationship::Neutral;
    RelationshipScore = 50.0f;
    RelationshipScoreMax = 100.0f;

    DialogueTriggerRadius = 400.0f;
    bCanSpeak = true;
    bHasMetPlayer = false;
    LastDialogueTime = -999.0f;
    LastSpokenLine = TEXT("");
}

// ─── BeginPlay ────────────────────────────────────────────────────────────────

void UNPCDialogueComponent::BeginPlay()
{
    Super::BeginPlay();

    // Seed default knowledge based on role
    switch (NPCRole)
    {
        case ENarr_NPCRole::Scout:
            AddKnowledge(TEXT("territory_safe"), TEXT("Eastern ridge — safe for 2 days"));
            AddKnowledge(TEXT("water_source"), TEXT("River bend 300 paces north"));
            break;
        case ENarr_NPCRole::Tracker:
            AddKnowledge(TEXT("predator_tracks"), TEXT("TRex tracks near salt flats — 1 day old"));
            AddKnowledge(TEXT("herd_direction"), TEXT("Brachiosaurus herd moving north-east"));
            break;
        case ENarr_NPCRole::Elder:
            AddKnowledge(TEXT("migration_pattern"), TEXT("Great herds come every dry season"));
            AddKnowledge(TEXT("danger_season"), TEXT("Raptors nest in valley during wet season"));
            break;
        case ENarr_NPCRole::Hunter:
            AddKnowledge(TEXT("hunting_ground"), TEXT("Salt flats — good ambush terrain"));
            AddKnowledge(TEXT("prey_weakness"), TEXT("Herbivores slow at river crossings"));
            break;
        default:
            break;
    }

    UpdateRelationshipTier();
}

// ─── TickComponent ────────────────────────────────────────────────────────────

void UNPCDialogueComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    // Tick is intentionally lightweight — dialogue triggers are event-driven
    // Future: proximity check to player for auto-trigger
}

// ─── TryTriggerDialogue ───────────────────────────────────────────────────────

bool UNPCDialogueComponent::TryTriggerDialogue(ENarr_DialogueTrigger Trigger)
{
    if (!bCanSpeak)
    {
        return false;
    }

    TArray<FNarr_DialogueLine> Available = GetAvailableLines(Trigger);
    if (Available.Num() == 0)
    {
        return false;
    }

    // Pick a random available line
    int32 Index = FMath::RandRange(0, Available.Num() - 1);
    const FNarr_DialogueLine& ChosenLine = Available[Index];

    LastSpokenLine = ChosenLine.LineText.ToString();

    UWorld* World = GetWorld();
    if (World)
    {
        LastDialogueTime = World->GetTimeSeconds();
        LastTriggerTimes.Add(Trigger, LastDialogueTime);
    }

    // Mark one-time lines as spoken
    for (int32 i = 0; i < DialogueLines.Num(); i++)
    {
        if (DialogueLines[i].LineText.EqualTo(ChosenLine.LineText) && DialogueLines[i].bOneTimeOnly)
        {
            SpokenOneTimeLines.Add(i);
            break;
        }
    }

    // First meeting bonus
    if (Trigger == ENarr_DialogueTrigger::PlayerFirstMeet && !bHasMetPlayer)
    {
        MarkPlayerMet();
    }

    return true;
}

// ─── ModifyRelationship ───────────────────────────────────────────────────────

void UNPCDialogueComponent::ModifyRelationship(float Delta)
{
    RelationshipScore = FMath::Clamp(RelationshipScore + Delta, 0.0f, RelationshipScoreMax);
    UpdateRelationshipTier();
}

// ─── GetRelationshipTier ──────────────────────────────────────────────────────

ENarr_NPCRelationship UNPCDialogueComponent::GetRelationshipTier() const
{
    return CurrentRelationship;
}

// ─── AddKnowledge ─────────────────────────────────────────────────────────────

void UNPCDialogueComponent::AddKnowledge(const FString& Key, const FString& Value)
{
    // Update if key exists
    for (FNarr_NPCKnowledge& K : KnowledgeBase)
    {
        if (K.KnowledgeKey == Key)
        {
            K.KnowledgeValue = Value;
            UWorld* World = GetWorld();
            K.AcquiredAtTime = World ? World->GetTimeSeconds() : 0.0f;
            return;
        }
    }

    // Add new entry
    FNarr_NPCKnowledge NewKnowledge;
    NewKnowledge.KnowledgeKey = Key;
    NewKnowledge.KnowledgeValue = Value;
    UWorld* World = GetWorld();
    NewKnowledge.AcquiredAtTime = World ? World->GetTimeSeconds() : 0.0f;
    NewKnowledge.bSharedWithPlayer = false;
    KnowledgeBase.Add(NewKnowledge);
}

// ─── GetKnowledge ─────────────────────────────────────────────────────────────

FString UNPCDialogueComponent::GetKnowledge(const FString& Key) const
{
    for (const FNarr_NPCKnowledge& K : KnowledgeBase)
    {
        if (K.KnowledgeKey == Key)
        {
            return K.KnowledgeValue;
        }
    }
    return TEXT("");
}

// ─── GetAvailableLines ────────────────────────────────────────────────────────

TArray<FNarr_DialogueLine> UNPCDialogueComponent::GetAvailableLines(ENarr_DialogueTrigger Trigger) const
{
    TArray<FNarr_DialogueLine> Result;

    for (int32 i = 0; i < DialogueLines.Num(); i++)
    {
        const FNarr_DialogueLine& Line = DialogueLines[i];

        // Wrong trigger
        if (Line.Trigger != Trigger) continue;

        // Relationship too low
        if (static_cast<uint8>(CurrentRelationship) < static_cast<uint8>(Line.MinRelationship)) continue;

        // Already spoken (one-time)
        if (Line.bOneTimeOnly && SpokenOneTimeLines.Contains(i)) continue;

        // On cooldown
        if (!IsLineCooledDown(Line, Trigger)) continue;

        Result.Add(Line);
    }

    return Result;
}

// ─── MarkPlayerMet ────────────────────────────────────────────────────────────

void UNPCDialogueComponent::MarkPlayerMet()
{
    if (!bHasMetPlayer)
    {
        bHasMetPlayer = true;
        // Small relationship boost on first meeting
        ModifyRelationship(5.0f);
    }
}

// ─── CanTriggerDialogue ───────────────────────────────────────────────────────

bool UNPCDialogueComponent::CanTriggerDialogue(ENarr_DialogueTrigger Trigger) const
{
    if (!bCanSpeak) return false;
    return GetAvailableLines(Trigger).Num() > 0;
}

// ─── UpdateRelationshipTier ───────────────────────────────────────────────────

void UNPCDialogueComponent::UpdateRelationshipTier()
{
    const float Score = RelationshipScore;

    if (Score < 10.0f)
        CurrentRelationship = ENarr_NPCRelationship::Hostile;
    else if (Score < 25.0f)
        CurrentRelationship = ENarr_NPCRelationship::Wary;
    else if (Score < 50.0f)
        CurrentRelationship = ENarr_NPCRelationship::Neutral;
    else if (Score < 70.0f)
        CurrentRelationship = ENarr_NPCRelationship::Friendly;
    else if (Score < 90.0f)
        CurrentRelationship = ENarr_NPCRelationship::Trusted;
    else
        CurrentRelationship = ENarr_NPCRelationship::Allied;
}

// ─── IsLineCooledDown ─────────────────────────────────────────────────────────

bool UNPCDialogueComponent::IsLineCooledDown(const FNarr_DialogueLine& Line, ENarr_DialogueTrigger Trigger) const
{
    const float* LastTime = LastTriggerTimes.Find(Trigger);
    if (!LastTime) return true;

    UWorld* World = GetWorld();
    if (!World) return true;

    const float Elapsed = World->GetTimeSeconds() - *LastTime;
    return Elapsed >= Line.CooldownSeconds;
}
