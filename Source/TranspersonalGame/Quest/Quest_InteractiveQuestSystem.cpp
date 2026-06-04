#include "Quest_InteractiveQuestSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Components/SphereComponent.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

AQuest_InteractiveNPC::AQuest_InteractiveNPC()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Initialize default values
    NPCName = TEXT("Tribal Member");
    NPCRole = TEXT("Villager");
    InteractionType = EQuest_InteractionType::Dialogue;
    CurrentMood = EQuest_NPCMood::Neutral;
    PlayerRelationship = 0.0f;
    InteractionCount = 0;
    
    // Initialize personality with random values
    Personality.Friendliness = FMath::RandRange(0.3f, 0.8f);
    Personality.Helpfulness = FMath::RandRange(0.2f, 0.7f);
    Personality.Suspicion = FMath::RandRange(0.1f, 0.5f);
    Personality.Courage = FMath::RandRange(0.3f, 0.9f);
    Personality.Knowledge = FMath::RandRange(0.2f, 0.8f);
    
    // Setup default dialogue options
    FQuest_DialogueOption GreetingOption;
    GreetingOption.OptionText = TEXT("Hello, how are you?");
    GreetingOption.ResponseText = TEXT("Greetings, traveler. The hunt has been difficult lately.");
    GreetingOption.bLeadsToQuest = false;
    GreetingOption.RelationshipChange = 1;
    DialogueOptions.Add(GreetingOption);
    
    FQuest_DialogueOption QuestOption;
    QuestOption.OptionText = TEXT("Do you need any help?");
    QuestOption.ResponseText = TEXT("Actually, yes. We need someone brave to help with the dinosaur problem.");
    QuestOption.bLeadsToQuest = true;
    QuestOption.QuestID = TEXT("hunt_raptor_pack");
    QuestOption.RelationshipChange = 2;
    DialogueOptions.Add(QuestOption);
}

void AQuest_InteractiveNPC::BeginPlay()
{
    Super::BeginPlay();
    
    // Generate initial contextual dialogue based on role and personality
    GenerateContextualDialogue();
    UpdateMoodBasedOnRelationship();
}

void AQuest_InteractiveNPC::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update NPC behavior based on time and interactions
    if (InteractionCount > 0 && FMath::RandRange(0.0f, 1.0f) < 0.001f)
    {
        UpdatePersonalityBasedResponses();
    }
}

void AQuest_InteractiveNPC::StartInteraction(AActor* PlayerActor)
{
    if (!PlayerActor)
    {
        return;
    }
    
    InteractionCount++;
    
    // Log interaction start
    UE_LOG(LogTemp, Warning, TEXT("NPC %s started interaction with player"), *NPCName);
    
    // Update mood based on relationship
    UpdateMoodBasedOnRelationship();
    
    // Generate fresh dialogue options based on current state
    GenerateContextualDialogue();
}

void AQuest_InteractiveNPC::ProcessDialogueChoice(int32 ChoiceIndex)
{
    if (ChoiceIndex < 0 || ChoiceIndex >= DialogueOptions.Num())
    {
        return;
    }
    
    FQuest_DialogueOption& SelectedOption = DialogueOptions[ChoiceIndex];
    
    // Apply relationship change
    ModifyRelationship(SelectedOption.RelationshipChange);
    
    // Handle quest giving
    if (SelectedOption.bLeadsToQuest && !SelectedOption.QuestID.IsEmpty())
    {
        GiveQuestToPlayer(SelectedOption.QuestID);
    }
    
    // Log dialogue choice
    UE_LOG(LogTemp, Warning, TEXT("NPC %s processed dialogue choice: %s"), *NPCName, *SelectedOption.OptionText);
}

TArray<FString> AQuest_InteractiveNPC::GetAvailableDialogueOptions()
{
    TArray<FString> Options;
    
    for (const FQuest_DialogueOption& Option : DialogueOptions)
    {
        // Filter options based on relationship and mood
        bool bShouldShow = true;
        
        if (Option.bLeadsToQuest && PlayerRelationship < 10.0f)
        {
            bShouldShow = false; // Need better relationship for quests
        }
        
        if (CurrentMood == EQuest_NPCMood::Hostile && Option.RelationshipChange > 0)
        {
            bShouldShow = false; // Hostile NPCs don't offer friendly options
        }
        
        if (bShouldShow)
        {
            Options.Add(Option.OptionText);
        }
    }
    
    return Options;
}

bool AQuest_InteractiveNPC::CanGiveQuest(const FString& QuestID)
{
    // Check if NPC can give this quest based on relationship and role
    if (PlayerRelationship < 5.0f)
    {
        return false; // Need minimum relationship
    }
    
    if (CurrentMood == EQuest_NPCMood::Hostile || CurrentMood == EQuest_NPCMood::Fearful)
    {
        return false; // Wrong mood for quest giving
    }
    
    return AvailableQuests.Contains(QuestID);
}

void AQuest_InteractiveNPC::GiveQuestToPlayer(const FString& QuestID)
{
    if (!CanGiveQuest(QuestID))
    {
        return;
    }
    
    // Remove quest from available list (one-time quests)
    AvailableQuests.Remove(QuestID);
    
    // Log quest giving
    UE_LOG(LogTemp, Warning, TEXT("NPC %s gave quest %s to player"), *NPCName, *QuestID);
    
    // Increase relationship for quest giving
    ModifyRelationship(5.0f);
}

void AQuest_InteractiveNPC::ModifyRelationship(float Amount)
{
    PlayerRelationship = FMath::Clamp(PlayerRelationship + Amount, -100.0f, 100.0f);
    
    // Update mood based on new relationship
    UpdateMoodBasedOnRelationship();
}

void AQuest_InteractiveNPC::UpdateMoodBasedOnRelationship()
{
    if (PlayerRelationship >= 50.0f)
    {
        CurrentMood = EQuest_NPCMood::Friendly;
    }
    else if (PlayerRelationship >= 20.0f)
    {
        CurrentMood = EQuest_NPCMood::Neutral;
    }
    else if (PlayerRelationship >= -20.0f)
    {
        CurrentMood = EQuest_NPCMood::Suspicious;
    }
    else if (PlayerRelationship >= -50.0f)
    {
        CurrentMood = EQuest_NPCMood::Fearful;
    }
    else
    {
        CurrentMood = EQuest_NPCMood::Hostile;
    }
}

void AQuest_InteractiveNPC::GenerateContextualDialogue()
{
    // Clear existing dialogue and generate new based on current state
    DialogueOptions.Empty();
    
    // Always have a greeting option
    FQuest_DialogueOption GreetingOption;
    GreetingOption.OptionText = TEXT("Greetings");
    
    switch (CurrentMood)
    {
        case EQuest_NPCMood::Friendly:
            GreetingOption.ResponseText = TEXT("Welcome, friend! It's good to see you again.");
            break;
        case EQuest_NPCMood::Neutral:
            GreetingOption.ResponseText = TEXT("Hello there. What brings you to our settlement?");
            break;
        case EQuest_NPCMood::Suspicious:
            GreetingOption.ResponseText = TEXT("I see you... What do you want here?");
            break;
        case EQuest_NPCMood::Hostile:
            GreetingOption.ResponseText = TEXT("You're not welcome here. Leave now.");
            break;
        case EQuest_NPCMood::Fearful:
            GreetingOption.ResponseText = TEXT("Please... don't hurt me. I have nothing of value.");
            break;
        case EQuest_NPCMood::Excited:
            GreetingOption.ResponseText = TEXT("Oh! Perfect timing! I was hoping someone would come by!");
            break;
    }
    
    GreetingOption.RelationshipChange = (CurrentMood == EQuest_NPCMood::Friendly) ? 2 : 1;
    DialogueOptions.Add(GreetingOption);
    
    // Add role-specific dialogue options
    if (NPCRole == TEXT("Hunter") && CurrentMood != EQuest_NPCMood::Hostile)
    {
        FQuest_DialogueOption HuntOption;
        HuntOption.OptionText = TEXT("Tell me about the hunt");
        HuntOption.ResponseText = TEXT("The great beasts grow bolder each day. We need skilled hunters.");
        HuntOption.bLeadsToQuest = true;
        HuntOption.QuestID = TEXT("dinosaur_hunt_mission");
        HuntOption.RelationshipChange = 3;
        DialogueOptions.Add(HuntOption);
    }
    
    if (NPCRole == TEXT("Elder") && PlayerRelationship > 10.0f)
    {
        FQuest_DialogueOption WisdomOption;
        WisdomOption.OptionText = TEXT("Share your wisdom");
        WisdomOption.ResponseText = TEXT("Listen well, young one. The ancient paths hold many secrets.");
        WisdomOption.bLeadsToQuest = true;
        WisdomOption.QuestID = TEXT("ancient_knowledge_quest");
        HuntOption.RelationshipChange = 5;
        DialogueOptions.Add(WisdomOption);
    }
}

void AQuest_InteractiveNPC::UpdatePersonalityBasedResponses()
{
    // Gradually evolve personality based on interactions
    if (InteractionCount > 10)
    {
        if (PlayerRelationship > 20.0f)
        {
            Personality.Friendliness = FMath::Min(1.0f, Personality.Friendliness + 0.01f);
            Personality.Suspicion = FMath::Max(0.0f, Personality.Suspicion - 0.01f);
        }
        else if (PlayerRelationship < -10.0f)
        {
            Personality.Suspicion = FMath::Min(1.0f, Personality.Suspicion + 0.02f);
            Personality.Friendliness = FMath::Max(0.0f, Personality.Friendliness - 0.01f);
        }
    }
}

// UQuest_InteractionComponent Implementation

UQuest_InteractionComponent::UQuest_InteractionComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    InteractionRange = 200.0f;
    bCanInteract = true;
    InteractionPrompt = TEXT("Press E to interact");
    CachedPlayerActor = nullptr;
}

void UQuest_InteractionComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Cache player actor reference
    if (UWorld* World = GetWorld())
    {
        if (APlayerController* PC = World->GetFirstPlayerController())
        {
            CachedPlayerActor = PC->GetPawn();
        }
    }
}

void UQuest_InteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bCanInteract)
    {
        CheckPlayerProximity();
    }
}

bool UQuest_InteractionComponent::IsPlayerInRange()
{
    if (!CachedPlayerActor || !GetOwner())
    {
        return false;
    }
    
    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), CachedPlayerActor->GetActorLocation());
    return Distance <= InteractionRange;
}

void UQuest_InteractionComponent::EnableInteraction()
{
    bCanInteract = true;
}

void UQuest_InteractionComponent::DisableInteraction()
{
    bCanInteract = false;
}

void UQuest_InteractionComponent::CheckPlayerProximity()
{
    static bool bWasInRange = false;
    bool bIsInRange = IsPlayerInRange();
    
    if (bIsInRange && !bWasInRange)
    {
        OnInteractionAvailable();
    }
    else if (!bIsInRange && bWasInRange)
    {
        OnInteractionUnavailable();
    }
    
    bWasInRange = bIsInRange;
}

// AQuest_InteractiveQuestBoard Implementation

AQuest_InteractiveQuestBoard::AQuest_InteractiveQuestBoard()
{
    PrimaryActorTick.bCanEverTick = false;
    
    MaxQuestCount = 5;
    QuestRefreshTime = 300.0f; // 5 minutes
}

void AQuest_InteractiveQuestBoard::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize with some default quests
    PostedQuests.Add(TEXT("gather_medicinal_herbs"));
    PostedQuests.Add(TEXT("scout_raptor_territory"));
    PostedQuests.Add(TEXT("collect_rare_stones"));
    
    // Start auto-refresh timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(QuestRefreshTimer, this, &AQuest_InteractiveQuestBoard::AutoRefreshQuests, QuestRefreshTime, true);
    }
}

void AQuest_InteractiveQuestBoard::RefreshQuestBoard()
{
    // Remove old quests and add new ones
    PostedQuests.Empty();
    
    TArray<FString> PossibleQuests = {
        TEXT("hunt_triceratops"),
        TEXT("explore_deep_caves"),
        TEXT("gather_rare_materials"),
        TEXT("defend_settlement"),
        TEXT("track_migration_route"),
        TEXT("craft_advanced_weapons"),
        TEXT("rescue_missing_hunter"),
        TEXT("investigate_strange_sounds")
    };
    
    // Randomly select quests up to max count
    for (int32 i = 0; i < MaxQuestCount && PossibleQuests.Num() > 0; i++)
    {
        int32 RandomIndex = FMath::RandRange(0, PossibleQuests.Num() - 1);
        PostedQuests.Add(PossibleQuests[RandomIndex]);
        PossibleQuests.RemoveAt(RandomIndex);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Quest board refreshed with %d quests"), PostedQuests.Num());
}

TArray<FString> AQuest_InteractiveQuestBoard::GetAvailableQuests()
{
    return PostedQuests;
}

void AQuest_InteractiveQuestBoard::AddQuest(const FString& QuestID)
{
    if (!PostedQuests.Contains(QuestID) && PostedQuests.Num() < MaxQuestCount)
    {
        PostedQuests.Add(QuestID);
    }
}

void AQuest_InteractiveQuestBoard::RemoveQuest(const FString& QuestID)
{
    PostedQuests.Remove(QuestID);
}

void AQuest_InteractiveQuestBoard::PostCommunityQuest(const FString& QuestID, int32 Priority)
{
    // Remove lowest priority quest if board is full
    if (PostedQuests.Num() >= MaxQuestCount)
    {
        PostedQuests.RemoveAt(PostedQuests.Num() - 1);
    }
    
    // Insert based on priority (higher priority = earlier in array)
    int32 InsertIndex = FMath::Max(0, PostedQuests.Num() - Priority);
    PostedQuests.Insert(QuestID, InsertIndex);
}

void AQuest_InteractiveQuestBoard::AutoRefreshQuests()
{
    RefreshQuestBoard();
}