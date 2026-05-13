#include "Narr_DialogueSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"

// UNarr_DialogueNode Implementation
UNarr_DialogueNode::UNarr_DialogueNode()
{
    NodeID = 0;
    bIsQuestNode = false;
    QuestID = TEXT("");
}

bool UNarr_DialogueNode::HasValidChoices() const
{
    return PlayerChoices.Num() > 0;
}

FNarr_DialogueChoice UNarr_DialogueNode::GetChoiceByIndex(int32 Index) const
{
    if (PlayerChoices.IsValidIndex(Index))
    {
        return PlayerChoices[Index];
    }
    return FNarr_DialogueChoice();
}

// UNarr_CharacterProfile Implementation
UNarr_CharacterProfile::UNarr_CharacterProfile()
{
    CharacterName = TEXT("Unknown Survivor");
    Role = ENarr_CharacterRole::Survivor;
    CharacterDescription = FText::FromString(TEXT("A survivor in the prehistoric world"));
    DefaultGreeting = TEXT("Greetings, fellow survivor.");
}

// UNarr_DialogueComponent Implementation
UNarr_DialogueComponent::UNarr_DialogueComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = false;
    
    CharacterProfile = nullptr;
    CurrentState = ENarr_DialogueState::Inactive;
    CurrentNodeID = -1;
    InteractionRange = 300.0f;
    bCanRepeatDialogues = true;
}

void UNarr_DialogueComponent::BeginPlay()
{
    Super::BeginPlay();
    
    if (CharacterProfile)
    {
        UE_LOG(LogTemp, Log, TEXT("Dialogue Component initialized for character: %s"), *CharacterProfile->CharacterName);
    }
}

void UNarr_DialogueComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (CurrentState == ENarr_DialogueState::Playing)
    {
        ProcessCurrentNode();
    }
}

bool UNarr_DialogueComponent::StartDialogue(int32 NodeID)
{
    if (CurrentState != ENarr_DialogueState::Inactive && !bCanRepeatDialogues)
    {
        return false;
    }

    UNarr_DialogueNode* StartNode = FindNodeByID(NodeID);
    if (!StartNode)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot start dialogue: Node %d not found"), NodeID);
        return false;
    }

    CurrentNodeID = NodeID;
    CurrentState = ENarr_DialogueState::Playing;
    
    SetComponentTickEnabled(true);
    
    OnDialogueStarted(StartNode->DialogueLine);
    
    UE_LOG(LogTemp, Log, TEXT("Started dialogue with %s at node %d"), 
           CharacterProfile ? *CharacterProfile->CharacterName : TEXT("Unknown"), NodeID);
    
    return true;
}

void UNarr_DialogueComponent::EndDialogue()
{
    CurrentState = ENarr_DialogueState::Completed;
    CurrentNodeID = -1;
    
    SetComponentTickEnabled(false);
    
    OnDialogueEnded();
    
    UE_LOG(LogTemp, Log, TEXT("Dialogue ended"));
}

bool UNarr_DialogueComponent::SelectChoice(int32 ChoiceIndex)
{
    UNarr_DialogueNode* CurrentNode = GetCurrentNode();
    if (!CurrentNode || !CurrentNode->HasValidChoices())
    {
        return false;
    }

    if (!CurrentNode->PlayerChoices.IsValidIndex(ChoiceIndex))
    {
        return false;
    }

    FNarr_DialogueChoice SelectedChoice = CurrentNode->PlayerChoices[ChoiceIndex];
    
    // Check if choice requires an item
    if (SelectedChoice.bRequiresItem)
    {
        // TODO: Implement item checking when inventory system is available
        UE_LOG(LogTemp, Log, TEXT("Choice requires item: %s"), *SelectedChoice.RequiredItemName);
    }
    
    // Move to next node
    if (SelectedChoice.NextNodeID >= 0)
    {
        CurrentNodeID = SelectedChoice.NextNodeID;
        UNarr_DialogueNode* NextNode = FindNodeByID(CurrentNodeID);
        if (NextNode)
        {
            OnDialogueStarted(NextNode->DialogueLine);
        }
    }
    else
    {
        EndDialogue();
    }
    
    return true;
}

UNarr_DialogueNode* UNarr_DialogueComponent::GetCurrentNode() const
{
    return FindNodeByID(CurrentNodeID);
}

bool UNarr_DialogueComponent::IsInRange(AActor* Player) const
{
    if (!Player || !GetOwner())
    {
        return false;
    }
    
    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Player->GetActorLocation());
    return Distance <= InteractionRange;
}

void UNarr_DialogueComponent::ProcessCurrentNode()
{
    UNarr_DialogueNode* CurrentNode = GetCurrentNode();
    if (!CurrentNode)
    {
        EndDialogue();
        return;
    }
    
    if (CurrentNode->HasValidChoices())
    {
        CurrentState = ENarr_DialogueState::WaitingForInput;
        OnChoicesPresented(CurrentNode->PlayerChoices);
    }
    else
    {
        // Auto-advance after duration if no choices
        static float Timer = 0.0f;
        Timer += GetWorld()->GetDeltaSeconds();
        
        if (Timer >= CurrentNode->DialogueLine.Duration)
        {
            Timer = 0.0f;
            EndDialogue();
        }
    }
}

UNarr_DialogueNode* UNarr_DialogueComponent::FindNodeByID(int32 NodeID) const
{
    for (UNarr_DialogueNode* Node : DialogueNodes)
    {
        if (Node && Node->NodeID == NodeID)
        {
            return Node;
        }
    }
    return nullptr;
}

// UNarr_DialogueSubsystem Implementation
void UNarr_DialogueSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    InitializeDefaultCharacters();
    
    UE_LOG(LogTemp, Log, TEXT("Dialogue Subsystem initialized with %d characters"), RegisteredCharacters.Num());
}

void UNarr_DialogueSubsystem::Deinitialize()
{
    RegisteredCharacters.Empty();
    Super::Deinitialize();
}

void UNarr_DialogueSubsystem::RegisterCharacter(UNarr_CharacterProfile* Character)
{
    if (Character && !RegisteredCharacters.Contains(Character))
    {
        RegisteredCharacters.Add(Character);
        UE_LOG(LogTemp, Log, TEXT("Registered character: %s"), *Character->CharacterName);
    }
}

UNarr_CharacterProfile* UNarr_DialogueSubsystem::GetCharacterByName(const FString& Name) const
{
    for (UNarr_CharacterProfile* Character : RegisteredCharacters)
    {
        if (Character && Character->CharacterName == Name)
        {
            return Character;
        }
    }
    return nullptr;
}

TArray<UNarr_CharacterProfile*> UNarr_DialogueSubsystem::GetCharactersByRole(ENarr_CharacterRole Role) const
{
    TArray<UNarr_CharacterProfile*> Result;
    
    for (UNarr_CharacterProfile* Character : RegisteredCharacters)
    {
        if (Character && Character->Role == Role)
        {
            Result.Add(Character);
        }
    }
    
    return Result;
}

void UNarr_DialogueSubsystem::InitializeDefaultCharacters()
{
    CreateElderKarak();
    CreateScoutZara();
    CreateTrackerJoren();
    CreateFireKeeperMira();
}

void UNarr_DialogueSubsystem::CreateElderKarak()
{
    UNarr_CharacterProfile* Elder = NewObject<UNarr_CharacterProfile>();
    Elder->CharacterName = TEXT("Elder Karak");
    Elder->Role = ENarr_CharacterRole::Elder;
    Elder->CharacterDescription = FText::FromString(TEXT("Wise leader who guides the tribe through ancient knowledge and survival wisdom"));
    Elder->DefaultGreeting = TEXT("Listen carefully, young hunter. The elders speak of ancient wisdom passed down through countless seasons.");
    Elder->VoiceLinePaths.Add(TEXT("/Game/Audio/Characters/Elder_Karak_Wisdom.wav"));
    Elder->AvailableDialogueNodes.Add(0);
    Elder->AvailableDialogueNodes.Add(1);
    Elder->AvailableDialogueNodes.Add(2);
    
    RegisterCharacter(Elder);
}

void UNarr_DialogueSubsystem::CreateScoutZara()
{
    UNarr_CharacterProfile* Scout = NewObject<UNarr_CharacterProfile>();
    Scout->CharacterName = TEXT("Scout Zara");
    Scout->Role = ENarr_CharacterRole::Scout;
    Scout->CharacterDescription = FText::FromString(TEXT("Alert reconnaissance specialist who monitors predator movements and safe passages"));
    Scout->DefaultGreeting = TEXT("Scout report from the northern ridge. Stay alert, stay alive.");
    Scout->VoiceLinePaths.Add(TEXT("/Game/Audio/Characters/Scout_Zara_Report.wav"));
    Scout->AvailableDialogueNodes.Add(10);
    Scout->AvailableDialogueNodes.Add(11);
    
    RegisterCharacter(Scout);
}

void UNarr_DialogueSubsystem::CreateTrackerJoren()
{
    UNarr_CharacterProfile* Tracker = NewObject<UNarr_CharacterProfile>();
    Tracker->CharacterName = TEXT("Tracker Joren");
    Tracker->Role = ENarr_CharacterRole::Tracker;
    Tracker->CharacterDescription = FText::FromString(TEXT("Expert hunter who understands migration patterns and animal behavior"));
    Tracker->DefaultGreeting = TEXT("The great migration has begun. This is our chance, but we must be careful.");
    Tracker->VoiceLinePaths.Add(TEXT("/Game/Audio/Characters/Tracker_Joren_Migration.wav"));
    Tracker->AvailableDialogueNodes.Add(20);
    Tracker->AvailableDialogueNodes.Add(21);
    
    RegisterCharacter(Tracker);
}

void UNarr_DialogueSubsystem::CreateFireKeeperMira()
{
    UNarr_CharacterProfile* FireKeeper = NewObject<UNarr_CharacterProfile>();
    FireKeeper->CharacterName = TEXT("Fire Keeper Mira");
    FireKeeper->Role = ENarr_CharacterRole::FireKeeper;
    FireKeeper->CharacterDescription = FText::FromString(TEXT("Guardian of the sacred flames who protects the tribe from night terrors"));
    FireKeeper->DefaultGreeting = TEXT("Fire keeper's warning - the sacred flames grow weak. We need your help.");
    FireKeeper->VoiceLinePaths.Add(TEXT("/Game/Audio/Characters/Fire_Keeper_Mira_Warning.wav"));
    FireKeeper->AvailableDialogueNodes.Add(30);
    FireKeeper->AvailableDialogueNodes.Add(31);
    
    RegisterCharacter(FireKeeper);
}