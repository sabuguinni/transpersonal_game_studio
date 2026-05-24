#include "Quest_NPCDialogueSystem.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/PrimitiveComponent.h"

UQuest_NPCDialogueSystem::UQuest_NPCDialogueSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    CurrentNodeIndex = 0;
    bIsInConversation = false;
    InteractionRange = 500.0f;
}

void UQuest_NPCDialogueSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Setup default dialogue based on owner's location
    AActor* Owner = GetOwner();
    if (Owner)
    {
        FVector Location = Owner->GetActorLocation();
        
        // Determine biome based on location (using brain memory coordinates)
        if (Location.X < -25000 && Location.Y < -15000)
        {
            SetupBiomeSpecificDialogue(TEXT("Pantano"));
        }
        else if (Location.X < -15000 && Location.Y > 15000)
        {
            SetupBiomeSpecificDialogue(TEXT("Floresta"));
        }
        else if (FMath::Abs(Location.X) < 20000 && FMath::Abs(Location.Y) < 20000)
        {
            SetupBiomeSpecificDialogue(TEXT("Savana"));
        }
        else if (Location.X > 25000 && FMath::Abs(Location.Y) < 30000)
        {
            SetupBiomeSpecificDialogue(TEXT("Deserto"));
        }
        else if (Location.X > 15000 && Location.Y > 20000)
        {
            SetupBiomeSpecificDialogue(TEXT("Montanha"));
        }
    }
}

void UQuest_NPCDialogueSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Check for nearby players if not in conversation
    if (!bIsInConversation)
    {
        UWorld* World = GetWorld();
        if (World)
        {
            APawn* PlayerPawn = World->GetFirstPlayerController()->GetPawn();
            if (PlayerPawn && IsPlayerInRange(PlayerPawn))
            {
                // Player is in range - could show interaction prompt here
            }
        }
    }
}

bool UQuest_NPCDialogueSystem::StartConversation(AActor* PlayerActor)
{
    if (!PlayerActor || bIsInConversation || DialogueNodes.Num() == 0)
    {
        return false;
    }
    
    if (!IsPlayerInRange(PlayerActor))
    {
        return false;
    }
    
    bIsInConversation = true;
    CurrentNodeIndex = 0;
    
    UE_LOG(LogTemp, Warning, TEXT("Conversation started with %s"), *GetOwner()->GetName());
    return true;
}

void UQuest_NPCDialogueSystem::EndConversation()
{
    bIsInConversation = false;
    CurrentNodeIndex = 0;
    UE_LOG(LogTemp, Warning, TEXT("Conversation ended"));
}

FQuest_DialogueNode UQuest_NPCDialogueSystem::GetCurrentDialogueNode() const
{
    if (DialogueNodes.IsValidIndex(CurrentNodeIndex))
    {
        return DialogueNodes[CurrentNodeIndex];
    }
    
    return FQuest_DialogueNode();
}

void UQuest_NPCDialogueSystem::SelectDialogueOption(int32 OptionIndex)
{
    if (!bIsInConversation || !DialogueNodes.IsValidIndex(CurrentNodeIndex))
    {
        return;
    }
    
    const FQuest_DialogueNode& CurrentNode = DialogueNodes[CurrentNodeIndex];
    if (!CurrentNode.Options.IsValidIndex(OptionIndex))
    {
        return;
    }
    
    const FQuest_DialogueOption& SelectedOption = CurrentNode.Options[OptionIndex];
    
    // Handle quest starting
    if (SelectedOption.bStartsQuest && !SelectedOption.QuestID.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("Starting quest: %s"), *SelectedOption.QuestID);
        // Quest system integration would go here
    }
    
    // Move to next node or end conversation
    if (CurrentNode.bEndsConversation)
    {
        EndConversation();
    }
    else
    {
        CurrentNodeIndex++;
        if (CurrentNodeIndex >= DialogueNodes.Num())
        {
            EndConversation();
        }
    }
}

void UQuest_NPCDialogueSystem::SetupBiomeSpecificDialogue(const FString& BiomeName)
{
    ClearDialogue();
    
    if (BiomeName == TEXT("Pantano"))
    {
        SetupMarshDialogue();
    }
    else if (BiomeName == TEXT("Floresta"))
    {
        SetupForestDialogue();
    }
    else if (BiomeName == TEXT("Savana"))
    {
        SetupSavannaDialogue();
    }
    else if (BiomeName == TEXT("Deserto"))
    {
        SetupDesertDialogue();
    }
    else if (BiomeName == TEXT("Montanha"))
    {
        SetupMountainDialogue();
    }
}

bool UQuest_NPCDialogueSystem::IsPlayerInRange(AActor* PlayerActor) const
{
    if (!PlayerActor || !GetOwner())
    {
        return false;
    }
    
    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), PlayerActor->GetActorLocation());
    return Distance <= InteractionRange;
}

void UQuest_NPCDialogueSystem::AddDialogueNode(const FQuest_DialogueNode& NewNode)
{
    DialogueNodes.Add(NewNode);
}

void UQuest_NPCDialogueSystem::ClearDialogue()
{
    DialogueNodes.Empty();
    CurrentNodeIndex = 0;
    bIsInConversation = false;
}

void UQuest_NPCDialogueSystem::SetupMarshDialogue()
{
    FQuest_DialogueNode GreetingNode;
    GreetingNode.SpeakerName = TEXT("Elder Marsh Survivor");
    GreetingNode.DialogueText = TEXT("Greetings, survivor. These swamplands hold ancient secrets and deadly predators.");
    GreetingNode.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1777859211742_Elder_Marsh_Survivor.mp3");
    
    FQuest_DialogueOption QuestOption;
    QuestOption.OptionText = TEXT("Teach me to survive in the marsh");
    QuestOption.ResponseText = TEXT("Collect ten marsh reeds without falling prey to the lurking dangers.");
    QuestOption.bStartsQuest = true;
    QuestOption.QuestID = TEXT("marsh_survival_basics");
    
    FQuest_DialogueOption LeaveOption;
    LeaveOption.OptionText = TEXT("I'll return later");
    LeaveOption.ResponseText = TEXT("Be careful out there, young one.");
    
    GreetingNode.Options.Add(QuestOption);
    GreetingNode.Options.Add(LeaveOption);
    GreetingNode.bEndsConversation = true;
    
    AddDialogueNode(GreetingNode);
}

void UQuest_NPCDialogueSystem::SetupForestDialogue()
{
    FQuest_DialogueNode GreetingNode;
    GreetingNode.SpeakerName = TEXT("Forest Guide");
    GreetingNode.DialogueText = TEXT("The forest whispers secrets to those who know how to listen. I can teach you to read its signs.");
    
    FQuest_DialogueOption QuestOption;
    QuestOption.OptionText = TEXT("Show me the ways of the forest");
    QuestOption.ResponseText = TEXT("Gather twenty forest branches and learn to craft a shelter that will protect you from the night predators.");
    QuestOption.bStartsQuest = true;
    QuestOption.QuestID = TEXT("forest_shelter_craft");
    
    FQuest_DialogueOption LeaveOption;
    LeaveOption.OptionText = TEXT("Not now");
    LeaveOption.ResponseText = TEXT("The forest will be here when you're ready.");
    
    GreetingNode.Options.Add(QuestOption);
    GreetingNode.Options.Add(LeaveOption);
    GreetingNode.bEndsConversation = true;
    
    AddDialogueNode(GreetingNode);
}

void UQuest_NPCDialogueSystem::SetupSavannaDialogue()
{
    FQuest_DialogueNode GreetingNode;
    GreetingNode.SpeakerName = TEXT("Plains Hunter");
    GreetingNode.DialogueText = TEXT("Welcome to the heart of the plains, young hunter. The grasslands hide fierce predators and valuable resources.");
    GreetingNode.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1777859219629_Plains_Hunter.mp3");
    
    FQuest_DialogueOption QuestOption;
    QuestOption.OptionText = TEXT("Teach me to hunt");
    QuestOption.ResponseText = TEXT("Learn to craft a stone axe, and I will share the secrets of tracking the great beasts.");
    QuestOption.bStartsQuest = true;
    QuestOption.QuestID = TEXT("plains_hunting_basics");
    
    FQuest_DialogueOption LeaveOption;
    LeaveOption.OptionText = TEXT("I need to prepare first");
    LeaveOption.ResponseText = TEXT("Wise. Preparation saves lives on the plains.");
    
    GreetingNode.Options.Add(QuestOption);
    GreetingNode.Options.Add(LeaveOption);
    GreetingNode.bEndsConversation = true;
    
    AddDialogueNode(GreetingNode);
}

void UQuest_NPCDialogueSystem::SetupDesertDialogue()
{
    FQuest_DialogueNode GreetingNode;
    GreetingNode.SpeakerName = TEXT("Desert Wanderer");
    GreetingNode.DialogueText = TEXT("The desert tests all who enter. Water is life, shade is salvation, and the crystal formations hold power.");
    
    FQuest_DialogueOption QuestOption;
    QuestOption.OptionText = TEXT("Help me survive the desert");
    QuestOption.ResponseText = TEXT("Find six desert crystals and craft a water container. Without water, you are already dead.");
    QuestOption.bStartsQuest = true;
    QuestOption.QuestID = TEXT("desert_water_survival");
    
    FQuest_DialogueOption LeaveOption;
    LeaveOption.OptionText = TEXT("I'm not ready for the desert");
    LeaveOption.ResponseText = TEXT("Smart. The desert claims the unprepared.");
    
    GreetingNode.Options.Add(QuestOption);
    GreetingNode.Options.Add(LeaveOption);
    GreetingNode.bEndsConversation = true;
    
    AddDialogueNode(GreetingNode);
}

void UQuest_NPCDialogueSystem::SetupMountainDialogue()
{
    FQuest_DialogueNode GreetingNode;
    GreetingNode.SpeakerName = TEXT("Mountain Tracker");
    GreetingNode.DialogueText = TEXT("The peaks are unforgiving. Cold kills faster than any predator, but the mountain provides tools for those who know where to look.");
    
    FQuest_DialogueOption QuestOption;
    QuestOption.OptionText = TEXT("Teach me mountain survival");
    QuestOption.ResponseText = TEXT("Collect eight mountain flint stones and craft a fire starter. Fire means life in these heights.");
    QuestOption.bStartsQuest = true;
    QuestOption.QuestID = TEXT("mountain_fire_craft");
    
    FQuest_DialogueOption LeaveOption;
    LeaveOption.OptionText = TEXT("The mountains intimidate me");
    LeaveOption.ResponseText = TEXT("Fear keeps you alive. Return when you're stronger.");
    
    GreetingNode.Options.Add(QuestOption);
    GreetingNode.Options.Add(LeaveOption);
    GreetingNode.bEndsConversation = true;
    
    AddDialogueNode(GreetingNode);
}