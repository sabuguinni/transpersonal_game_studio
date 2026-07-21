// NPCDialogueComponent.cpp
// Agent #15 — Narrative & Dialogue Agent
// Implements contextual NPC dialogue system for prehistoric survival game

#include "NPCDialogueComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

UNPCDialogueComponent::UNPCDialogueComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.5f; // Check every 0.5s for performance

    NPCName = FText::FromString(TEXT("Unknown"));
    NPCRole = ENarr_NPCRole::Hunter;
    CurrentRelationship = ENarr_NPCRelationship::Neutral;
    DialogueTriggerRadius = 400.0f;
    bIsInDialogue = false;
    bHasGreeted = false;
    LastTrigger = ENarr_DialogueTrigger::PlayerApproach;
    DialogueCooldownSeconds = 30.0f;
    bCooldownActive = false;
    TrustPoints = 0;
    InteractionCount = 0;
}

void UNPCDialogueComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Populate dialogue lines based on role
    InitialiseDialogueLines();
}

void UNPCDialogueComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bCooldownActive || bIsInDialogue)
    {
        return;
    }

    // Check for player proximity
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        return;
    }

    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }

    float DistanceToPlayer = FVector::Dist(Owner->GetActorLocation(), PlayerPawn->GetActorLocation());

    if (DistanceToPlayer <= DialogueTriggerRadius)
    {
        if (!bHasGreeted)
        {
            TriggerDialogue(ENarr_DialogueTrigger::PlayerApproach);
            bHasGreeted = true;
        }
    }
    else
    {
        // Reset greeting when player leaves
        if (bHasGreeted && DistanceToPlayer > DialogueTriggerRadius * 1.5f)
        {
            bHasGreeted = false;
        }
    }
}

void UNPCDialogueComponent::TriggerDialogue(ENarr_DialogueTrigger Trigger)
{
    if (bCooldownActive || bIsInDialogue)
    {
        return;
    }

    LastTrigger = Trigger;
    bIsInDialogue = true;

    // Get appropriate line for trigger
    FNarr_DialogueLine Line = GetDialogueLineForTrigger(Trigger);

    if (!Line.DialogueText.IsEmpty())
    {
        // Broadcast to Blueprint/UI
        OnDialogueTriggered.Broadcast(Line, CurrentRelationship);

        // Log for debugging
        UE_LOG(LogTemp, Log, TEXT("[NPC Dialogue] %s (%s): %s"),
            *NPCName.ToString(),
            *UEnum::GetValueAsString(NPCRole),
            *Line.DialogueText.ToString());
    }

    // Start cooldown
    StartDialogueCooldown();
}

FNarr_DialogueLine UNPCDialogueComponent::GetDialogueLineForTrigger(ENarr_DialogueTrigger Trigger)
{
    // Filter lines by trigger type
    TArray<FNarr_DialogueLine> MatchingLines;
    for (const FNarr_DialogueLine& Line : DialogueLines)
    {
        if (Line.Trigger == Trigger)
        {
            // Check relationship requirement
            if (static_cast<int32>(CurrentRelationship) >= static_cast<int32>(Line.MinRelationshipRequired))
            {
                MatchingLines.Add(Line);
            }
        }
    }

    if (MatchingLines.Num() == 0)
    {
        // Return default fallback line
        FNarr_DialogueLine Fallback;
        Fallback.DialogueText = FText::FromString(TEXT("..."));
        Fallback.Trigger = Trigger;
        Fallback.MinRelationshipRequired = ENarr_NPCRelationship::Neutral;
        Fallback.bIsContextual = false;
        return Fallback;
    }

    // Return random matching line
    int32 RandomIndex = FMath::RandRange(0, MatchingLines.Num() - 1);
    return MatchingLines[RandomIndex];
}

void UNPCDialogueComponent::StartDialogueCooldown()
{
    bIsInDialogue = false;
    bCooldownActive = true;

    GetWorld()->GetTimerManager().SetTimer(
        CooldownTimerHandle,
        this,
        &UNPCDialogueComponent::OnCooldownExpired,
        DialogueCooldownSeconds,
        false
    );
}

void UNPCDialogueComponent::OnCooldownExpired()
{
    bCooldownActive = false;
}

void UNPCDialogueComponent::ModifyRelationship(int32 TrustDelta)
{
    TrustPoints = FMath::Clamp(TrustPoints + TrustDelta, -100, 100);

    // Update relationship tier based on trust points
    ENarr_NPCRelationship NewRelationship;

    if (TrustPoints <= -60)
    {
        NewRelationship = ENarr_NPCRelationship::Hostile;
    }
    else if (TrustPoints <= -20)
    {
        NewRelationship = ENarr_NPCRelationship::Wary;
    }
    else if (TrustPoints <= 20)
    {
        NewRelationship = ENarr_NPCRelationship::Neutral;
    }
    else if (TrustPoints <= 50)
    {
        NewRelationship = ENarr_NPCRelationship::Friendly;
    }
    else if (TrustPoints <= 80)
    {
        NewRelationship = ENarr_NPCRelationship::Trusted;
    }
    else
    {
        NewRelationship = ENarr_NPCRelationship::Allied;
    }

    if (NewRelationship != CurrentRelationship)
    {
        ENarr_NPCRelationship OldRelationship = CurrentRelationship;
        CurrentRelationship = NewRelationship;
        OnRelationshipChanged.Broadcast(OldRelationship, NewRelationship);

        UE_LOG(LogTemp, Log, TEXT("[NPC Dialogue] %s relationship changed: %s -> %s"),
            *NPCName.ToString(),
            *UEnum::GetValueAsString(OldRelationship),
            *UEnum::GetValueAsString(NewRelationship));
    }
}

void UNPCDialogueComponent::NotifyDinosaurNearby(FName DinosaurSpecies, float Distance)
{
    if (bCooldownActive || bIsInDialogue)
    {
        return;
    }

    // Only trigger warning if dinosaur is close enough to be threatening
    if (Distance < 1500.0f)
    {
        TriggerDialogue(ENarr_DialogueTrigger::DinosaurNearby);
    }
}

void UNPCDialogueComponent::NotifyPlayerCombatVictory()
{
    ModifyRelationship(10);
    TriggerDialogue(ENarr_DialogueTrigger::PlayerCombatVictory);
    InteractionCount++;
}

void UNPCDialogueComponent::NotifyPlayerNearDeath()
{
    TriggerDialogue(ENarr_DialogueTrigger::PlayerNearDeath);
}

void UNPCDialogueComponent::NotifyMigrationEvent()
{
    TriggerDialogue(ENarr_DialogueTrigger::MigrationSeen);
}

void UNPCDialogueComponent::NotifyNightfall()
{
    TriggerDialogue(ENarr_DialogueTrigger::Nightfall);
}

void UNPCDialogueComponent::InitialiseDialogueLines()
{
    DialogueLines.Empty();

    // Lines are populated based on NPC role
    // In production, these would be loaded from a DataTable
    // Here we seed with role-appropriate defaults

    switch (NPCRole)
    {
        case ENarr_NPCRole::Hunter:
            AddDialogueLine(ENarr_DialogueTrigger::PlayerApproach, ENarr_NPCRelationship::Neutral,
                TEXT("You look like you can handle yourself. Good. This valley needs people who can fight."), false);
            AddDialogueLine(ENarr_DialogueTrigger::DinosaurNearby, ENarr_NPCRelationship::Neutral,
                TEXT("Quiet. Do not move. Wait for it to pass."), true);
            AddDialogueLine(ENarr_DialogueTrigger::PlayerCombatVictory, ENarr_NPCRelationship::Neutral,
                TEXT("Not bad. Most people freeze their first time. You did not."), false);
            AddDialogueLine(ENarr_DialogueTrigger::PlayerNearDeath, ENarr_NPCRelationship::Friendly,
                TEXT("Stay with me! You are not dying today — I will not allow it."), true);
            break;

        case ENarr_NPCRole::Scout:
            AddDialogueLine(ENarr_DialogueTrigger::PlayerApproach, ENarr_NPCRelationship::Neutral,
                TEXT("I have mapped three valleys to the east. None of them are safe. But the northern ridge — that has possibilities."), false);
            AddDialogueLine(ENarr_DialogueTrigger::MigrationSeen, ENarr_NPCRelationship::Neutral,
                TEXT("The herd is moving again. When they move, the predators follow. We need to move first."), true);
            AddDialogueLine(ENarr_DialogueTrigger::Nightfall, ENarr_NPCRelationship::Neutral,
                TEXT("Find shelter before full dark. The raptors hunt by sound at night — and they are very good at it."), true);
            break;

        case ENarr_NPCRole::Elder:
            AddDialogueLine(ENarr_DialogueTrigger::PlayerApproach, ENarr_NPCRelationship::Neutral,
                TEXT("Sit. You have the look of someone who has survived things they should not have. That is useful."), false);
            AddDialogueLine(ENarr_DialogueTrigger::PlayerApproach, ENarr_NPCRelationship::Friendly,
                TEXT("I have been watching you. You learn fast. That is the only thing that matters out here."), false);
            AddDialogueLine(ENarr_DialogueTrigger::DinosaurNearby, ENarr_NPCRelationship::Neutral,
                TEXT("I have seen that beast before. It has a territory. Stay out of it and it will not bother you."), true);
            AddDialogueLine(ENarr_DialogueTrigger::Nightfall, ENarr_NPCRelationship::Trusted,
                TEXT("Tonight we tell the old stories. About the time before the great beasts ruled. About what we lost — and what we must rebuild."), false);
            break;

        case ENarr_NPCRole::Crafter:
            AddDialogueLine(ENarr_DialogueTrigger::PlayerApproach, ENarr_NPCRelationship::Neutral,
                TEXT("You need tools? I can make them. But I need materials. Flint from the river, sinew from a kill — bring me those and we talk."), false);
            AddDialogueLine(ENarr_DialogueTrigger::PlayerCombatVictory, ENarr_NPCRelationship::Neutral,
                TEXT("Good kill. Bring me the bones — I can make something useful from them."), true);
            break;

        case ENarr_NPCRole::Tracker:
            AddDialogueLine(ENarr_DialogueTrigger::PlayerApproach, ENarr_NPCRelationship::Neutral,
                TEXT("I can read the ground like you read the sky. These tracks — three raptors, moving fast, heading west. They found something."), false);
            AddDialogueLine(ENarr_DialogueTrigger::DinosaurNearby, ENarr_NPCRelationship::Neutral,
                TEXT("TRex. Half a kilometre. Downwind of us for now — but the wind is shifting."), true);
            AddDialogueLine(ENarr_DialogueTrigger::MigrationSeen, ENarr_NPCRelationship::Neutral,
                TEXT("The migration is three days early. Something spooked them. Something big."), true);
            break;

        default:
            AddDialogueLine(ENarr_DialogueTrigger::PlayerApproach, ENarr_NPCRelationship::Neutral,
                TEXT("..."), false);
            break;
    }
}

void UNPCDialogueComponent::AddDialogueLine(ENarr_DialogueTrigger Trigger, ENarr_NPCRelationship MinRelationship, const FString& Text, bool bContextual)
{
    FNarr_DialogueLine NewLine;
    NewLine.Trigger = Trigger;
    NewLine.MinRelationshipRequired = MinRelationship;
    NewLine.DialogueText = FText::FromString(Text);
    NewLine.bIsContextual = bContextual;
    DialogueLines.Add(NewLine);
}
