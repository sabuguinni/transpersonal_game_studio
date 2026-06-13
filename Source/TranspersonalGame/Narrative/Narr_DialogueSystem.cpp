#include "Narr_DialogueSystem.h"
#include "TranspersonalGameState.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

UNarr_DialogueSystem::UNarr_DialogueSystem()
{
    CurrentConversation = FNarr_ConversationState();
}

void UNarr_DialogueSystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    LoadDefaultDialogues();
    UE_LOG(LogTemp, Warning, TEXT("Dialogue System initialized with %d dialogues"), DialogueDatabase.Num());
}

void UNarr_DialogueSystem::Deinitialize()
{
    EndConversation();
    DialogueDatabase.Empty();
    GlobalCompletedDialogues.Empty();
    Super::Deinitialize();
}

bool UNarr_DialogueSystem::StartConversation(const FString& NPCName, const FString& StartingDialogueID)
{
    if (CurrentConversation.bInConversation)
    {
        UE_LOG(LogTemp, Warning, TEXT("Already in conversation with %s"), *CurrentConversation.ConversationPartner);
        return false;
    }

    if (!DialogueDatabase.Contains(StartingDialogueID))
    {
        UE_LOG(LogTemp, Error, TEXT("Dialogue ID %s not found in database"), *StartingDialogueID);
        return false;
    }

    const FNarr_DialogueEntry& StartDialogue = DialogueDatabase[StartingDialogueID];
    if (!CheckDialogueRequirements(StartDialogue))
    {
        UE_LOG(LogTemp, Warning, TEXT("Player does not meet requirements for dialogue %s"), *StartingDialogueID);
        return false;
    }

    CurrentConversation.bInConversation = true;
    CurrentConversation.ConversationPartner = NPCName;
    CurrentConversation.CurrentDialogueID = StartingDialogueID;

    UE_LOG(LogTemp, Log, TEXT("Started conversation with %s, dialogue: %s"), *NPCName, *StartingDialogueID);
    return true;
}

void UNarr_DialogueSystem::EndConversation()
{
    if (CurrentConversation.bInConversation)
    {
        UE_LOG(LogTemp, Log, TEXT("Ended conversation with %s"), *CurrentConversation.ConversationPartner);
        
        // Mark current dialogue as completed
        if (!CurrentConversation.CurrentDialogueID.IsEmpty())
        {
            MarkDialogueCompleted(CurrentConversation.CurrentDialogueID);
        }
    }

    CurrentConversation = FNarr_ConversationState();
}

FNarr_DialogueEntry UNarr_DialogueSystem::GetCurrentDialogue()
{
    if (!CurrentConversation.bInConversation || CurrentConversation.CurrentDialogueID.IsEmpty())
    {
        return FNarr_DialogueEntry();
    }

    if (DialogueDatabase.Contains(CurrentConversation.CurrentDialogueID))
    {
        return DialogueDatabase[CurrentConversation.CurrentDialogueID];
    }

    return FNarr_DialogueEntry();
}

bool UNarr_DialogueSystem::SelectResponse(int32 ResponseIndex)
{
    if (!CurrentConversation.bInConversation)
    {
        return false;
    }

    FNarr_DialogueEntry CurrentDialogue = GetCurrentDialogue();
    if (ResponseIndex < 0 || ResponseIndex >= CurrentDialogue.NextDialogueIDs.Num())
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid response index %d for dialogue %s"), ResponseIndex, *CurrentConversation.CurrentDialogueID);
        return false;
    }

    // Mark current dialogue as completed
    MarkDialogueCompleted(CurrentConversation.CurrentDialogueID);

    // Move to next dialogue
    FString NextDialogueID = CurrentDialogue.NextDialogueIDs[ResponseIndex];
    if (NextDialogueID.IsEmpty() || NextDialogueID == "END")
    {
        EndConversation();
        return true;
    }

    if (!DialogueDatabase.Contains(NextDialogueID))
    {
        UE_LOG(LogTemp, Error, TEXT("Next dialogue ID %s not found"), *NextDialogueID);
        EndConversation();
        return false;
    }

    CurrentConversation.CurrentDialogueID = NextDialogueID;
    UE_LOG(LogTemp, Log, TEXT("Advanced to dialogue: %s"), *NextDialogueID);
    return true;
}

TArray<FString> UNarr_DialogueSystem::GetAvailableResponses()
{
    FNarr_DialogueEntry CurrentDialogue = GetCurrentDialogue();
    return CurrentDialogue.ResponseOptions;
}

bool UNarr_DialogueSystem::IsInConversation() const
{
    return CurrentConversation.bInConversation;
}

void UNarr_DialogueSystem::RegisterDialogue(const FNarr_DialogueEntry& DialogueEntry)
{
    if (DialogueEntry.DialogueID.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot register dialogue with empty ID"));
        return;
    }

    DialogueDatabase.Add(DialogueEntry.DialogueID, DialogueEntry);
    UE_LOG(LogTemp, Log, TEXT("Registered dialogue: %s"), *DialogueEntry.DialogueID);
}

bool UNarr_DialogueSystem::HasCompletedDialogue(const FString& DialogueID) const
{
    return GlobalCompletedDialogues.Contains(DialogueID);
}

void UNarr_DialogueSystem::LoadDefaultDialogues()
{
    // Elder Karn Introduction
    FNarr_DialogueEntry KarnIntro;
    KarnIntro.DialogueID = "karn_intro_01";
    KarnIntro.SpeakerName = "Elder Karn";
    KarnIntro.DialogueText = "Welcome, young survivor. I am Elder Karn, keeper of ancient knowledge. The prehistoric world is dangerous, but those who learn its ways can thrive.";
    KarnIntro.ResponseOptions = {"Tell me about survival", "What dangers should I know?", "I must go"};
    KarnIntro.NextDialogueIDs = {"karn_survival_01", "karn_dangers_01", "END"};
    RegisterDialogue(KarnIntro);

    // Survival advice branch
    FNarr_DialogueEntry KarnSurvival;
    KarnSurvival.DialogueID = "karn_survival_01";
    KarnSurvival.SpeakerName = "Elder Karn";
    KarnSurvival.DialogueText = "Survival requires four pillars: food, water, shelter, and knowledge. The river provides fresh water, but beware - predators also drink there. Hunt small game first, learn their patterns.";
    KarnSurvival.ResponseOptions = {"What about shelter?", "How do I hunt safely?", "Thank you for the advice"};
    KarnSurvival.NextDialogueIDs = {"karn_shelter_01", "karn_hunting_01", "END"};
    RegisterDialogue(KarnSurvival);

    // Dangers branch
    FNarr_DialogueEntry KarnDangers;
    KarnDangers.DialogueID = "karn_dangers_01";
    KarnDangers.SpeakerName = "Elder Karn";
    KarnDangers.DialogueText = "The great lizards rule this land. The pack hunters are cunning - they work together. The giant long-necks are gentle unless threatened. But the apex predator... it hunts alone and fears nothing.";
    KarnDangers.ResponseOptions = {"How do I avoid pack hunters?", "What is the apex predator?", "I understand"};
    KarnDangers.NextDialogueIDs = {"karn_packs_01", "karn_apex_01", "END"};
    RegisterDialogue(KarnDangers);

    // Shelter advice
    FNarr_DialogueEntry KarnShelter;
    KarnShelter.DialogueID = "karn_shelter_01";
    KarnShelter.SpeakerName = "Elder Karn";
    KarnShelter.DialogueText = "High ground is safe ground. Build near rocks for protection, but not too close to caves - you never know what sleeps within. Fire keeps the night terrors away.";
    KarnShelter.ResponseOptions = {"How do I make fire?", "What are night terrors?", "I'll remember this"};
    KarnShelter.NextDialogueIDs = {"karn_fire_01", "karn_night_01", "END"};
    RegisterDialogue(KarnShelter);

    // Hunting advice
    FNarr_DialogueEntry KarnHunting;
    KarnHunting.DialogueID = "karn_hunting_01";
    KarnHunting.SpeakerName = "Elder Karn";
    KarnHunting.DialogueText = "Patience is the hunter's greatest weapon. Study your prey's habits. Strike when they drink or feed. A wounded animal is more dangerous than a healthy one - finish what you start.";
    KarnHunting.ResponseOptions = {"What weapons work best?", "How do I track animals?", "Wise words"};
    KarnHunting.NextDialogueIDs = {"karn_weapons_01", "karn_tracking_01", "END"};
    RegisterDialogue(KarnHunting);

    // Pack hunter tactics
    FNarr_DialogueEntry KarnPacks;
    KarnPacks.DialogueID = "karn_packs_01";
    KarnPacks.SpeakerName = "Elder Karn";
    KarnPacks.DialogueText = "The swift hunters move in coordinated groups. They will try to surround you. Keep your back to rocks or trees. If you must run, run uphill - their claws struggle on steep slopes.";
    KarnPacks.ResponseOptions = {"What if I'm surrounded?", "Any other tactics?", "I'll be careful"};
    KarnPacks.NextDialogueIDs = {"karn_surrounded_01", "karn_tactics_01", "END"};
    RegisterDialogue(KarnPacks);

    // Apex predator warning
    FNarr_DialogueEntry KarnApex;
    KarnApex.DialogueID = "karn_apex_01";
    KarnApex.SpeakerName = "Elder Karn";
    KarnApex.DialogueText = "The Thunder Walker. Massive jaws that can crush boulders. Its roar shakes the earth. If you hear heavy footsteps and feel the ground tremble... hide. Do not run. Do not fight. Only hide and pray it passes.";
    KarnApex.RequiredNeed = ESurvivalNeed::Fear;
    KarnApex.RequiredNeedValue = 30.0f;
    KarnApex.ResponseOptions = {"How big is it?", "Has anyone survived an encounter?", "I... I understand"};
    KarnApex.NextDialogueIDs = {"karn_size_01", "karn_survivors_01", "END"};
    RegisterDialogue(KarnApex);

    UE_LOG(LogTemp, Warning, TEXT("Loaded %d default dialogues for Elder Karn"), DialogueDatabase.Num());
}

bool UNarr_DialogueSystem::CheckDialogueRequirements(const FNarr_DialogueEntry& Dialogue)
{
    if (Dialogue.RequiredNeed == ESurvivalNeed::None)
    {
        return true;
    }

    // Get player's current survival stats
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }

    ATranspersonalGameState* GameState = Cast<ATranspersonalGameState>(UGameplayStatics::GetGameState(World));
    if (!GameState)
    {
        return true; // Allow dialogue if we can't check requirements
    }

    float CurrentValue = 0.0f;
    switch (Dialogue.RequiredNeed)
    {
        case ESurvivalNeed::Health:
            CurrentValue = GameState->GetHealth();
            break;
        case ESurvivalNeed::Hunger:
            CurrentValue = GameState->GetHunger();
            break;
        case ESurvivalNeed::Thirst:
            CurrentValue = GameState->GetThirst();
            break;
        case ESurvivalNeed::Stamina:
            CurrentValue = GameState->GetStamina();
            break;
        case ESurvivalNeed::Fear:
            CurrentValue = GameState->GetFear();
            break;
        default:
            return true;
    }

    return CurrentValue >= Dialogue.RequiredNeedValue;
}

void UNarr_DialogueSystem::MarkDialogueCompleted(const FString& DialogueID)
{
    if (!DialogueID.IsEmpty() && !GlobalCompletedDialogues.Contains(DialogueID))
    {
        GlobalCompletedDialogues.Add(DialogueID);
        CurrentConversation.CompletedDialogues.Add(DialogueID);
        UE_LOG(LogTemp, Log, TEXT("Marked dialogue as completed: %s"), *DialogueID);
    }
}