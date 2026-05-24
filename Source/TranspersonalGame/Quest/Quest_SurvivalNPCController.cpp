#include "Quest_SurvivalNPCController.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Components/CapsuleComponent.h"
#include "NavigationSystem.h"
#include "AIModule/Classes/BehaviorTree/BlackboardComponent.h"

AQuest_SurvivalNPCController::AQuest_SurvivalNPCController()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Default NPC setup
    InteractionRange = 500.0f;
    bCanMoveAround = true;
    MovementRadius = 1000.0f;
    
    // State initialization
    PlayerInRange = nullptr;
    bIsInConversation = false;
    LastPlayerCheckTime = 0.0f;
    PatrolTimer = 0.0f;
    bMovingToPatrolTarget = false;
    
    // Default personality
    NPCPersonality.Role = EQuest_NPCRole::TutorialGuide;
    NPCPersonality.CurrentMood = EQuest_NPCMood::Friendly;
    NPCPersonality.NPCName = TEXT("Survival Guide");
    NPCPersonality.BackgroundStory = TEXT("An experienced survivor who helps newcomers adapt to this dangerous prehistoric world.");
    
    // Setup default dialogue
    FQuest_DialogueOption greeting;
    greeting.OptionText = TEXT("Hello");
    greeting.ResponseText = TEXT("Welcome, fellow survivor. This world is dangerous, but I can help you learn to survive.");
    greeting.bStartsQuest = false;
    NPCPersonality.AvailableDialogue.Add(greeting);
    
    FQuest_DialogueOption questOption;
    questOption.OptionText = TEXT("Can you teach me?");
    questOption.ResponseText = TEXT("Of course! Let me show you the basics of survival. First, you'll need to gather some basic resources.");
    questOption.bStartsQuest = true;
    questOption.QuestID = TEXT("BasicSurvivalTutorial");
    NPCPersonality.AvailableDialogue.Add(questOption);
}

void AQuest_SurvivalNPCController::BeginPlay()
{
    Super::BeginPlay();
    
    // Store home location
    if (GetPawn())
    {
        HomeLocation = GetPawn()->GetActorLocation();
        UE_LOG(LogTemp, Log, TEXT("NPC %s initialized at home location: %s"), 
               *NPCPersonality.NPCName, *HomeLocation.ToString());
    }
    
    // Initialize patrol target
    CurrentPatrolTarget = HomeLocation;
}

void AQuest_SurvivalNPCController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Check for nearby players periodically
    LastPlayerCheckTime += DeltaTime;
    if (LastPlayerCheckTime >= 1.0f) // Check every second
    {
        CheckForNearbyPlayer();
        LastPlayerCheckTime = 0.0f;
    }
    
    // Handle movement if not in conversation
    if (!bIsInConversation && bCanMoveAround)
    {
        HandleMovement(DeltaTime);
    }
}

void AQuest_SurvivalNPCController::CheckForNearbyPlayer()
{
    if (!GetPawn())
        return;
        
    UWorld* World = GetWorld();
    if (!World)
        return;
    
    // Find the player character
    ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(World, 0);
    if (!PlayerCharacter)
        return;
    
    float Distance = FVector::Dist(GetPawn()->GetActorLocation(), PlayerCharacter->GetActorLocation());
    
    if (Distance <= InteractionRange)
    {
        if (PlayerInRange != PlayerCharacter)
        {
            PlayerInRange = PlayerCharacter;
            UE_LOG(LogTemp, Log, TEXT("Player entered interaction range of NPC %s"), *NPCPersonality.NPCName);
            
            // Show interaction prompt or start automatic greeting
            if (GEngine)
            {
                FString Message = FString::Printf(TEXT("Press E to talk to %s"), *NPCPersonality.NPCName);
                GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, Message);
            }
        }
    }
    else
    {
        if (PlayerInRange == PlayerCharacter)
        {
            PlayerInRange = nullptr;
            UE_LOG(LogTemp, Log, TEXT("Player left interaction range of NPC %s"), *NPCPersonality.NPCName);
        }
    }
}

void AQuest_SurvivalNPCController::HandleMovement(float DeltaTime)
{
    if (!GetPawn())
        return;
    
    PatrolTimer += DeltaTime;
    
    // Change patrol target every 10 seconds
    if (PatrolTimer >= 10.0f || !bMovingToPatrolTarget)
    {
        PatrolAroundHome();
        PatrolTimer = 0.0f;
    }
    
    // Move towards patrol target
    if (bMovingToPatrolTarget)
    {
        FVector CurrentLocation = GetPawn()->GetActorLocation();
        float DistanceToTarget = FVector::Dist(CurrentLocation, CurrentPatrolTarget);
        
        if (DistanceToTarget > 100.0f)
        {
            MoveToLocation(CurrentPatrolTarget);
        }
        else
        {
            bMovingToPatrolTarget = false;
        }
    }
}

void AQuest_SurvivalNPCController::StartConversation(APawn* Player)
{
    if (!Player || bIsInConversation)
        return;
    
    bIsInConversation = true;
    PlayerInRange = Player;
    
    // Stop movement
    StopMovement();
    
    // Face the player
    if (GetPawn())
    {
        FVector Direction = Player->GetActorLocation() - GetPawn()->GetActorLocation();
        Direction.Z = 0.0f;
        Direction.Normalize();
        
        FRotator NewRotation = Direction.Rotation();
        GetPawn()->SetActorRotation(NewRotation);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Started conversation with %s"), *NPCPersonality.NPCName);
    
    // Show greeting
    if (GEngine)
    {
        FString Greeting = GetGreeting();
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, Greeting);
    }
}

void AQuest_SurvivalNPCController::EndConversation()
{
    bIsInConversation = false;
    UE_LOG(LogTemp, Log, TEXT("Ended conversation with %s"), *NPCPersonality.NPCName);
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Gray, TEXT("Conversation ended."));
    }
}

bool AQuest_SurvivalNPCController::CanInteract(APawn* Player) const
{
    if (!Player || !GetPawn())
        return false;
    
    float Distance = FVector::Dist(GetPawn()->GetActorLocation(), Player->GetActorLocation());
    return Distance <= InteractionRange && !bIsInConversation;
}

FString AQuest_SurvivalNPCController::GetGreeting() const
{
    FString Greeting = FString::Printf(TEXT("%s: "), *NPCPersonality.NPCName);
    
    switch (NPCPersonality.CurrentMood)
    {
        case EQuest_NPCMood::Friendly:
            Greeting += TEXT("Hello there, fellow survivor! Good to see another friendly face.");
            break;
        case EQuest_NPCMood::Cautious:
            Greeting += TEXT("Who goes there? Oh, another survivor. Stay alert in these parts.");
            break;
        case EQuest_NPCMood::Urgent:
            Greeting += TEXT("Thank goodness you're here! I need help urgently!");
            break;
        case EQuest_NPCMood::Desperate:
            Greeting += TEXT("Please, you have to help me! I don't know what else to do!");
            break;
        case EQuest_NPCMood::Grateful:
            Greeting += TEXT("I can't thank you enough for your help. How can I repay you?");
            break;
        default:
            Greeting += TEXT("Greetings, survivor.");
            break;
    }
    
    return Greeting;
}

TArray<FQuest_DialogueOption> AQuest_SurvivalNPCController::GetAvailableDialogue() const
{
    return NPCPersonality.AvailableDialogue;
}

void AQuest_SurvivalNPCController::GiveQuestToPlayer(const FString& QuestID, APawn* Player)
{
    if (!Player || QuestID.IsEmpty())
        return;
    
    UE_LOG(LogTemp, Log, TEXT("NPC %s giving quest %s to player"), *NPCPersonality.NPCName, *QuestID);
    
    // TODO: Integrate with quest system to actually give the quest
    // For now, just log and show message
    if (GEngine)
    {
        FString Message = FString::Printf(TEXT("Quest Started: %s"), *QuestID);
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, Message);
    }
    
    // Update mood to grateful after giving quest
    SetNPCMood(EQuest_NPCMood::Grateful);
}

bool AQuest_SurvivalNPCController::HasQuestForPlayer(APawn* Player) const
{
    // Check if any dialogue options start quests
    for (const FQuest_DialogueOption& Option : NPCPersonality.AvailableDialogue)
    {
        if (Option.bStartsQuest && !Option.QuestID.IsEmpty())
        {
            return true;
        }
    }
    return false;
}

void AQuest_SurvivalNPCController::UpdateQuestDialogue()
{
    // Update available dialogue based on current context
    // This could check quest completion status, player progress, etc.
    UE_LOG(LogTemp, Log, TEXT("Updating dialogue for NPC %s"), *NPCPersonality.NPCName);
}

void AQuest_SurvivalNPCController::SetNPCMood(EQuest_NPCMood NewMood)
{
    NPCPersonality.CurrentMood = NewMood;
    UE_LOG(LogTemp, Log, TEXT("NPC %s mood changed to %s"), 
           *NPCPersonality.NPCName, *GetNPCMoodString());
}

void AQuest_SurvivalNPCController::PatrolAroundHome()
{
    if (!GetPawn())
        return;
    
    // Generate random point within movement radius
    FVector RandomDirection = FVector(
        FMath::RandRange(-1.0f, 1.0f),
        FMath::RandRange(-1.0f, 1.0f),
        0.0f
    );
    RandomDirection.Normalize();
    
    float RandomDistance = FMath::RandRange(200.0f, MovementRadius);
    CurrentPatrolTarget = HomeLocation + (RandomDirection * RandomDistance);
    CurrentPatrolTarget.Z = HomeLocation.Z; // Keep same height
    
    bMovingToPatrolTarget = true;
    
    UE_LOG(LogTemp, Log, TEXT("NPC %s patrolling to %s"), 
           *NPCPersonality.NPCName, *CurrentPatrolTarget.ToString());
}

void AQuest_SurvivalNPCController::ReturnHome()
{
    CurrentPatrolTarget = HomeLocation;
    bMovingToPatrolTarget = true;
    
    UE_LOG(LogTemp, Log, TEXT("NPC %s returning home"), *NPCPersonality.NPCName);
}

FString AQuest_SurvivalNPCController::GetNPCRoleString() const
{
    switch (NPCPersonality.Role)
    {
        case EQuest_NPCRole::TutorialGuide:
            return TEXT("Tutorial Guide");
        case EQuest_NPCRole::HuntingExpert:
            return TEXT("Hunting Expert");
        case EQuest_NPCRole::SurvivalMentor:
            return TEXT("Survival Mentor");
        case EQuest_NPCRole::CraftingMaster:
            return TEXT("Crafting Master");
        case EQuest_NPCRole::ScoutLeader:
            return TEXT("Scout Leader");
        default:
            return TEXT("Unknown");
    }
}

FString AQuest_SurvivalNPCController::GetNPCMoodString() const
{
    switch (NPCPersonality.CurrentMood)
    {
        case EQuest_NPCMood::Friendly:
            return TEXT("Friendly");
        case EQuest_NPCMood::Cautious:
            return TEXT("Cautious");
        case EQuest_NPCMood::Urgent:
            return TEXT("Urgent");
        case EQuest_NPCMood::Desperate:
            return TEXT("Desperate");
        case EQuest_NPCMood::Grateful:
            return TEXT("Grateful");
        default:
            return TEXT("Unknown");
    }
}