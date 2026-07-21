#include "Quest_NPCInteractionSystem.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/Engine.h"

UQuest_NPCInteractionSystem::UQuest_NPCInteractionSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    // Initialize default values
    NPCRole = EQuest_NPCRole::VillageElder;
    CurrentMood = EQuest_NPCMood::Neutral;
    NPCName = TEXT("Unknown NPC");
    InteractionRange = 300.0f;
    bCanInteract = true;
    bInConversation = false;
    bHasActiveQuest = false;
    InteractingPlayer = nullptr;
    LastInteractionTime = 0.0f;
}

void UQuest_NPCInteractionSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeDialogues();
    InitializeQuestData();
    
    UE_LOG(LogTemp, Log, TEXT("NPC Interaction System initialized for: %s"), *NPCName);
}

void UQuest_NPCInteractionSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update interaction availability
    if (InteractingPlayer)
    {
        float Distance = FVector::Dist(GetOwner()->GetActorLocation(), InteractingPlayer->GetActorLocation());
        if (Distance > InteractionRange)
        {
            EndInteraction();
        }
    }
    
    // Update dialogue based on mood changes
    UpdateDialogueBasedOnMood();
}

bool UQuest_NPCInteractionSystem::CanPlayerInteract(AActor* Player)
{
    if (!Player || !bCanInteract)
    {
        return false;
    }
    
    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Player->GetActorLocation());
    return Distance <= InteractionRange && !bInConversation;
}

void UQuest_NPCInteractionSystem::StartInteraction(AActor* Player)
{
    if (!CanPlayerInteract(Player))
    {
        return;
    }
    
    InteractingPlayer = Player;
    bInConversation = true;
    LastInteractionTime = GetWorld()->GetTimeSeconds();
    
    // Set initial dialogue based on current mood and available quests
    if (AvailableDialogues.Num() > 0)
    {
        for (const FQuest_NPCDialogue& Dialogue : AvailableDialogues)
        {
            if (Dialogue.RequiredMood == CurrentMood)
            {
                CurrentDialogue = Dialogue;
                break;
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Started interaction with %s"), *NPCName);
}

void UQuest_NPCInteractionSystem::EndInteraction()
{
    InteractingPlayer = nullptr;
    bInConversation = false;
    
    UE_LOG(LogTemp, Log, TEXT("Ended interaction with %s"), *NPCName);
}

FQuest_NPCDialogue UQuest_NPCInteractionSystem::GetCurrentDialogue()
{
    return CurrentDialogue;
}

void UQuest_NPCInteractionSystem::SetDialogue(const FString& DialogueText, EQuest_NPCMood Mood)
{
    CurrentDialogue.DialogueText = DialogueText;
    CurrentDialogue.RequiredMood = Mood;
    CurrentDialogue.SpeakerName = NPCName;
    CurrentMood = Mood;
}

TArray<FQuest_NPCQuestData> UQuest_NPCInteractionSystem::GetAvailableQuests()
{
    TArray<FQuest_NPCQuestData> Available;
    
    for (const FQuest_NPCQuestData& Quest : AvailableQuests)
    {
        if (Quest.bIsAvailable && !Quest.bIsCompleted)
        {
            Available.Add(Quest);
        }
    }
    
    return Available;
}

bool UQuest_NPCInteractionSystem::GiveQuestToPlayer(const FString& QuestID, AActor* Player)
{
    for (FQuest_NPCQuestData& Quest : AvailableQuests)
    {
        if (Quest.QuestID == QuestID && Quest.bIsAvailable && !Quest.bIsCompleted)
        {
            bHasActiveQuest = true;
            UE_LOG(LogTemp, Log, TEXT("Quest '%s' given to player by %s"), *Quest.QuestTitle, *NPCName);
            return true;
        }
    }
    
    return false;
}

bool UQuest_NPCInteractionSystem::CompleteQuest(const FString& QuestID)
{
    for (FQuest_NPCQuestData& Quest : AvailableQuests)
    {
        if (Quest.QuestID == QuestID)
        {
            Quest.bIsCompleted = true;
            CompletedQuests.Add(QuestID);
            bHasActiveQuest = false;
            
            // Change mood to excited after quest completion
            SetNPCMood(EQuest_NPCMood::Excited);
            
            UE_LOG(LogTemp, Log, TEXT("Quest '%s' completed for %s"), *Quest.QuestTitle, *NPCName);
            return true;
        }
    }
    
    return false;
}

void UQuest_NPCInteractionSystem::SetNPCMood(EQuest_NPCMood NewMood)
{
    CurrentMood = NewMood;
    UpdateDialogueBasedOnMood();
    
    UE_LOG(LogTemp, Log, TEXT("%s mood changed to: %d"), *NPCName, (int32)NewMood);
}

FString UQuest_NPCInteractionSystem::GetNPCRoleString()
{
    switch (NPCRole)
    {
        case EQuest_NPCRole::VillageElder: return TEXT("Village Elder");
        case EQuest_NPCRole::HunterGuide: return TEXT("Hunter Guide");
        case EQuest_NPCRole::Tracker: return TEXT("Tracker");
        case EQuest_NPCRole::Gatherer: return TEXT("Gatherer");
        case EQuest_NPCRole::Warrior: return TEXT("Warrior");
        case EQuest_NPCRole::Shaman: return TEXT("Shaman");
        case EQuest_NPCRole::Crafter: return TEXT("Crafter");
        case EQuest_NPCRole::Scout: return TEXT("Scout");
        default: return TEXT("Unknown");
    }
}

void UQuest_NPCInteractionSystem::UpdateDialogueBasedOnMood()
{
    // Update current dialogue based on mood
    for (const FQuest_NPCDialogue& Dialogue : AvailableDialogues)
    {
        if (Dialogue.RequiredMood == CurrentMood)
        {
            CurrentDialogue = Dialogue;
            break;
        }
    }
}

void UQuest_NPCInteractionSystem::InitializeQuestData()
{
    // Initialize quests based on NPC role
    AvailableQuests.Empty();
    
    switch (NPCRole)
    {
        case EQuest_NPCRole::VillageElder:
        {
            FQuest_NPCQuestData ElderQuest;
            ElderQuest.QuestID = TEXT("ELDER_WISDOM");
            ElderQuest.QuestTitle = TEXT("Wisdom of the Ancients");
            ElderQuest.QuestDescription = TEXT("Learn the ancient ways of survival from the village elder.");
            ElderQuest.bIsAvailable = true;
            ElderQuest.RewardExperience = 200;
            AvailableQuests.Add(ElderQuest);
            break;
        }
        case EQuest_NPCRole::HunterGuide:
        {
            FQuest_NPCQuestData HuntQuest;
            HuntQuest.QuestID = TEXT("FIRST_HUNT");
            HuntQuest.QuestTitle = TEXT("The First Hunt");
            HuntQuest.QuestDescription = TEXT("Learn to hunt dinosaurs safely with an experienced guide.");
            HuntQuest.bIsAvailable = true;
            HuntQuest.RewardExperience = 300;
            AvailableQuests.Add(HuntQuest);
            break;
        }
        case EQuest_NPCRole::Tracker:
        {
            FQuest_NPCQuestData TrackQuest;
            TrackQuest.QuestID = TEXT("TRACK_PREDATOR");
            TrackQuest.QuestTitle = TEXT("Track the Predator");
            TrackQuest.QuestDescription = TEXT("Follow dinosaur tracks to locate a dangerous predator.");
            TrackQuest.bIsAvailable = true;
            TrackQuest.RewardExperience = 250;
            AvailableQuests.Add(TrackQuest);
            break;
        }
        case EQuest_NPCRole::Gatherer:
        {
            FQuest_NPCQuestData GatherQuest;
            GatherQuest.QuestID = TEXT("GATHER_RESOURCES");
            GatherQuest.QuestTitle = TEXT("Essential Resources");
            GatherQuest.QuestDescription = TEXT("Gather vital resources for the tribe's survival.");
            GatherQuest.bIsAvailable = true;
            GatherQuest.RewardExperience = 150;
            AvailableQuests.Add(GatherQuest);
            break;
        }
        default:
            break;
    }
}

void UQuest_NPCInteractionSystem::InitializeDialogues()
{
    AvailableDialogues.Empty();
    
    // Create mood-based dialogues for each NPC role
    switch (NPCRole)
    {
        case EQuest_NPCRole::VillageElder:
        {
            FQuest_NPCDialogue FriendlyDialogue;
            FriendlyDialogue.DialogueText = TEXT("Welcome, young one. I have much wisdom to share with those who seek to survive in this dangerous world.");
            FriendlyDialogue.RequiredMood = EQuest_NPCMood::Friendly;
            FriendlyDialogue.SpeakerName = NPCName;
            AvailableDialogues.Add(FriendlyDialogue);
            
            FQuest_NPCDialogue WorriedDialogue;
            WorriedDialogue.DialogueText = TEXT("These are troubled times. The dinosaurs grow more aggressive, and our people struggle to survive.");
            WorriedDialogue.RequiredMood = EQuest_NPCMood::Worried;
            WorriedDialogue.SpeakerName = NPCName;
            AvailableDialogues.Add(WorriedDialogue);
            break;
        }
        case EQuest_NPCRole::HunterGuide:
        {
            FQuest_NPCDialogue ExcitedDialogue;
            ExcitedDialogue.DialogueText = TEXT("Ready for the hunt? The dinosaurs won't know what hit them!");
            ExcitedDialogue.RequiredMood = EQuest_NPCMood::Excited;
            ExcitedDialogue.SpeakerName = NPCName;
            AvailableDialogues.Add(ExcitedDialogue);
            
            FQuest_NPCDialogue NeutralDialogue;
            NeutralDialogue.DialogueText = TEXT("Hunting is dangerous work, but necessary for our survival. Are you prepared?");
            NeutralDialogue.RequiredMood = EQuest_NPCMood::Neutral;
            NeutralDialogue.SpeakerName = NPCName;
            AvailableDialogues.Add(NeutralDialogue);
            break;
        }
        default:
        {
            FQuest_NPCDialogue DefaultDialogue;
            DefaultDialogue.DialogueText = TEXT("Greetings, traveler. How may I assist you?");
            DefaultDialogue.RequiredMood = EQuest_NPCMood::Neutral;
            DefaultDialogue.SpeakerName = NPCName;
            AvailableDialogues.Add(DefaultDialogue);
            break;
        }
    }
    
    // Set initial dialogue
    if (AvailableDialogues.Num() > 0)
    {
        CurrentDialogue = AvailableDialogues[0];
    }
}