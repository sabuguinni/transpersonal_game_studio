#include "Narr_DialogueManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "Kismet/GameplayStatics.h"

UNarr_DialogueManager::UNarr_DialogueManager()
{
    bIsDialogueActive = false;
    bIsConversationActive = false;
    CurrentDialogueID = "";
    CurrentConversationID = "";
    CurrentNodeID = "";
    DialogueAudioComponent = nullptr;
    CurrentSpeaker = nullptr;
    CurrentListener = nullptr;
}

void UNarr_DialogueManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("Narrative Dialogue Manager initialized"));
    
    // Initialize default dialogues and conversations
    InitializeDefaultDialogues();
    InitializeDefaultConversations();
}

void UNarr_DialogueManager::Deinitialize()
{
    EndDialogue();
    
    if (DialogueAudioComponent && IsValid(DialogueAudioComponent))
    {
        DialogueAudioComponent->Stop();
        DialogueAudioComponent = nullptr;
    }
    
    Super::Deinitialize();
}

bool UNarr_DialogueManager::StartDialogue(const FString& DialogueID, AActor* Speaker, AActor* Listener)
{
    if (DialogueID.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot start dialogue with empty ID"));
        return false;
    }

    FNarr_DialogueEntry* DialogueEntry = DialogueDatabase.Find(DialogueID);
    if (!DialogueEntry)
    {
        UE_LOG(LogTemp, Warning, TEXT("Dialogue not found: %s"), *DialogueID);
        return false;
    }

    // End current dialogue if active
    if (bIsDialogueActive)
    {
        EndDialogue();
    }

    CurrentDialogueID = DialogueID;
    CurrentSpeaker = Speaker;
    CurrentListener = Listener;
    bIsDialogueActive = true;

    // Process dialogue conditions
    ProcessDialogueConditions(*DialogueEntry);

    // Broadcast dialogue started event
    OnDialogueStarted.Broadcast(DialogueID, DialogueEntry->Speaker);
    OnDialogueLineSpoken.Broadcast(DialogueID, DialogueEntry->DialogueText, DialogueEntry->Duration);

    // Play audio if available
    PlayDialogueAudio(DialogueID);

    UE_LOG(LogTemp, Log, TEXT("Started dialogue: %s"), *DialogueID);
    return true;
}

void UNarr_DialogueManager::EndDialogue()
{
    if (!bIsDialogueActive)
    {
        return;
    }

    FString EndedDialogueID = CurrentDialogueID;
    
    StopDialogueAudio();
    
    CurrentDialogueID = "";
    CurrentSpeaker = nullptr;
    CurrentListener = nullptr;
    bIsDialogueActive = false;

    OnDialogueEnded.Broadcast(EndedDialogueID);
    
    UE_LOG(LogTemp, Log, TEXT("Ended dialogue: %s"), *EndedDialogueID);
}

bool UNarr_DialogueManager::IsDialogueActive() const
{
    return bIsDialogueActive;
}

FString UNarr_DialogueManager::GetCurrentDialogueID() const
{
    return CurrentDialogueID;
}

bool UNarr_DialogueManager::StartConversation(const FString& ConversationID, AActor* Speaker, AActor* Listener)
{
    if (ConversationID.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot start conversation with empty ID"));
        return false;
    }

    TArray<FNarr_ConversationNode>* ConversationNodes = ConversationDatabase.Find(ConversationID);
    if (!ConversationNodes || ConversationNodes->Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Conversation not found: %s"), *ConversationID);
        return false;
    }

    // End current dialogue/conversation if active
    if (bIsDialogueActive || bIsConversationActive)
    {
        EndDialogue();
    }

    CurrentConversationID = ConversationID;
    CurrentNodeID = (*ConversationNodes)[0].NodeID; // Start with first node
    CurrentSpeaker = Speaker;
    CurrentListener = Listener;
    bIsConversationActive = true;

    // Start the first dialogue node
    FNarr_ConversationNode* FirstNode = GetConversationNode(ConversationID, CurrentNodeID);
    if (FirstNode)
    {
        CurrentDialogueID = FirstNode->DialogueEntry.DialogueID;
        bIsDialogueActive = true;
        
        OnDialogueStarted.Broadcast(CurrentDialogueID, FirstNode->DialogueEntry.Speaker);
        OnDialogueLineSpoken.Broadcast(CurrentDialogueID, FirstNode->DialogueEntry.DialogueText, FirstNode->DialogueEntry.Duration);
        
        PlayDialogueAudio(CurrentDialogueID);
    }

    UE_LOG(LogTemp, Log, TEXT("Started conversation: %s"), *ConversationID);
    return true;
}

bool UNarr_DialogueManager::AdvanceConversation(const FString& ChoiceID)
{
    if (!bIsConversationActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("No active conversation to advance"));
        return false;
    }

    FNarr_ConversationNode* CurrentNode = GetConversationNode(CurrentConversationID, CurrentNodeID);
    if (!CurrentNode)
    {
        UE_LOG(LogTemp, Warning, TEXT("Current conversation node not found"));
        return false;
    }

    // Check if this is an end node
    if (CurrentNode->bIsEndNode)
    {
        EndDialogue();
        bIsConversationActive = false;
        return true;
    }

    // Find next node
    FString NextNodeID = "";
    if (CurrentNode->NextNodeIDs.Num() > 0)
    {
        if (CurrentNode->bRequiresPlayerChoice && !ChoiceID.IsEmpty())
        {
            // Find the node corresponding to the choice
            for (const FString& NodeID : CurrentNode->NextNodeIDs)
            {
                if (NodeID.Contains(ChoiceID))
                {
                    NextNodeID = NodeID;
                    break;
                }
            }
        }
        else
        {
            // Take the first available next node
            NextNodeID = CurrentNode->NextNodeIDs[0];
        }
    }

    if (NextNodeID.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("No valid next node found"));
        EndDialogue();
        bIsConversationActive = false;
        return false;
    }

    // Advance to next node
    CurrentNodeID = NextNodeID;
    FNarr_ConversationNode* NextNode = GetConversationNode(CurrentConversationID, CurrentNodeID);
    if (NextNode)
    {
        CurrentDialogueID = NextNode->DialogueEntry.DialogueID;
        
        OnDialogueLineSpoken.Broadcast(CurrentDialogueID, NextNode->DialogueEntry.DialogueText, NextNode->DialogueEntry.Duration);
        PlayDialogueAudio(CurrentDialogueID);
    }

    return true;
}

TArray<FString> UNarr_DialogueManager::GetCurrentChoices() const
{
    TArray<FString> Choices;
    
    if (!bIsConversationActive)
    {
        return Choices;
    }

    FNarr_ConversationNode* CurrentNode = const_cast<UNarr_DialogueManager*>(this)->GetConversationNode(CurrentConversationID, CurrentNodeID);
    if (CurrentNode && CurrentNode->bRequiresPlayerChoice)
    {
        for (const FString& ResponseOption : CurrentNode->DialogueEntry.ResponseOptions)
        {
            Choices.Add(ResponseOption);
        }
    }

    return Choices;
}

void UNarr_DialogueManager::RegisterDialogue(const FNarr_DialogueEntry& DialogueEntry)
{
    if (DialogueEntry.DialogueID.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot register dialogue with empty ID"));
        return;
    }

    DialogueDatabase.Add(DialogueEntry.DialogueID, DialogueEntry);
    UE_LOG(LogTemp, Log, TEXT("Registered dialogue: %s"), *DialogueEntry.DialogueID);
}

void UNarr_DialogueManager::RegisterConversation(const FString& ConversationID, const TArray<FNarr_ConversationNode>& Nodes)
{
    if (ConversationID.IsEmpty() || Nodes.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot register conversation with empty ID or no nodes"));
        return;
    }

    ConversationDatabase.Add(ConversationID, Nodes);
    
    // Also register individual dialogue entries
    for (const FNarr_ConversationNode& Node : Nodes)
    {
        RegisterDialogue(Node.DialogueEntry);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Registered conversation: %s with %d nodes"), *ConversationID, Nodes.Num());
}

bool UNarr_DialogueManager::GetDialogueEntry(const FString& DialogueID, FNarr_DialogueEntry& OutEntry) const
{
    const FNarr_DialogueEntry* Entry = DialogueDatabase.Find(DialogueID);
    if (Entry)
    {
        OutEntry = *Entry;
        return true;
    }
    return false;
}

TArray<FString> UNarr_DialogueManager::GetDialoguesByType(ENarr_DialogueType Type) const
{
    TArray<FString> Results;
    
    for (const auto& Pair : DialogueDatabase)
    {
        if (Pair.Value.Type == Type)
        {
            Results.Add(Pair.Key);
        }
    }
    
    return Results;
}

TArray<FString> UNarr_DialogueManager::GetDialoguesByCharacter(ENarr_CharacterRole Character) const
{
    TArray<FString> Results;
    
    for (const auto& Pair : DialogueDatabase)
    {
        if (Pair.Value.Speaker == Character)
        {
            Results.Add(Pair.Key);
        }
    }
    
    return Results;
}

void UNarr_DialogueManager::PlayDialogueAudio(const FString& DialogueID)
{
    FNarr_DialogueEntry* Entry = DialogueDatabase.Find(DialogueID);
    if (!Entry || Entry->VoiceClip.IsNull())
    {
        return;
    }

    USoundBase* SoundToPlay = Entry->VoiceClip.LoadSynchronous();
    if (!SoundToPlay)
    {
        return;
    }

    // Stop current audio if playing
    StopDialogueAudio();

    // Create audio component if needed
    if (!DialogueAudioComponent)
    {
        UWorld* World = GetWorld();
        if (World)
        {
            DialogueAudioComponent = UGameplayStatics::SpawnSound2D(World, SoundToPlay, 1.0f, 1.0f, 0.0f);
        }
    }
    else
    {
        DialogueAudioComponent->SetSound(SoundToPlay);
        DialogueAudioComponent->Play();
    }
}

void UNarr_DialogueManager::StopDialogueAudio()
{
    if (DialogueAudioComponent && IsValid(DialogueAudioComponent))
    {
        DialogueAudioComponent->Stop();
    }
}

bool UNarr_DialogueManager::IsDialogueAudioPlaying() const
{
    return DialogueAudioComponent && IsValid(DialogueAudioComponent) && DialogueAudioComponent->IsPlaying();
}

void UNarr_DialogueManager::InitializeDefaultDialogues()
{
    // Hunt Master dialogues
    FNarr_DialogueEntry HuntMasterGreeting;
    HuntMasterGreeting.DialogueID = "hunt_master_greeting";
    HuntMasterGreeting.Speaker = ENarr_CharacterRole::HuntMaster;
    HuntMasterGreeting.Type = ENarr_DialogueType::Greeting;
    HuntMasterGreeting.DialogueText = FText::FromString("The great hunt begins at dawn. Our scouts have spotted a massive herd of long-necks moving through the valley below.");
    HuntMasterGreeting.Duration = 21.0f;
    RegisterDialogue(HuntMasterGreeting);

    // Tribal Scout warning
    FNarr_DialogueEntry ScoutWarning;
    ScoutWarning.DialogueID = "scout_predator_warning";
    ScoutWarning.Speaker = ENarr_CharacterRole::Scout;
    ScoutWarning.Type = ENarr_DialogueType::Warning;
    ScoutWarning.DialogueText = FText::FromString("Beware the shadow of the thunder-foot! The ground shakes with each step as the great predator stalks through our territory.");
    ScoutWarning.Duration = 22.0f;
    RegisterDialogue(ScoutWarning);

    // Water Seeker survival advice
    FNarr_DialogueEntry WaterSeekerAdvice;
    WaterSeekerAdvice.DialogueID = "water_seeker_survival";
    WaterSeekerAdvice.Speaker = ENarr_CharacterRole::WaterSeeker;
    WaterSeekerAdvice.Type = ENarr_DialogueType::Survival;
    WaterSeekerAdvice.DialogueText = FText::FromString("The water runs low and the dry season approaches. Our scouts report fresh springs beyond the bone valley, but the path is treacherous.");
    WaterSeekerAdvice.Duration = 24.0f;
    RegisterDialogue(WaterSeekerAdvice);

    // Tribal Elder lore
    FNarr_DialogueEntry ElderLore;
    ElderLore.DialogueID = "elder_ancient_wisdom";
    ElderLore.Speaker = ENarr_CharacterRole::TribalElder;
    ElderLore.Type = ENarr_DialogueType::Lore;
    ElderLore.DialogueText = FText::FromString("The elders speak of the old ways, when our ancestors first learned to read the signs in stone and sky. Watch the flight of the leather-wings at sunset.");
    ElderLore.Duration = 25.0f;
    RegisterDialogue(ElderLore);

    UE_LOG(LogTemp, Log, TEXT("Initialized %d default dialogues"), DialogueDatabase.Num());
}

void UNarr_DialogueManager::InitializeDefaultConversations()
{
    // Create a sample hunt preparation conversation
    TArray<FNarr_ConversationNode> HuntConversation;
    
    // Node 1: Hunt Master introduction
    FNarr_ConversationNode Node1;
    Node1.NodeID = "hunt_intro";
    Node1.DialogueEntry.DialogueID = "hunt_master_greeting";
    Node1.DialogueEntry.Speaker = ENarr_CharacterRole::HuntMaster;
    Node1.DialogueEntry.Type = ENarr_DialogueType::Quest;
    Node1.DialogueEntry.DialogueText = FText::FromString("The great hunt begins at dawn. Are you ready to join us?");
    Node1.DialogueEntry.Duration = 8.0f;
    Node1.DialogueEntry.ResponseOptions.Add("Yes, I'm ready");
    Node1.DialogueEntry.ResponseOptions.Add("I need more preparation");
    Node1.NextNodeIDs.Add("hunt_ready");
    Node1.NextNodeIDs.Add("hunt_prepare");
    Node1.bRequiresPlayerChoice = true;
    HuntConversation.Add(Node1);
    
    // Node 2: Ready response
    FNarr_ConversationNode Node2;
    Node2.NodeID = "hunt_ready";
    Node2.DialogueEntry.DialogueID = "hunt_ready_response";
    Node2.DialogueEntry.Speaker = ENarr_CharacterRole::HuntMaster;
    Node2.DialogueEntry.Type = ENarr_DialogueType::Instruction;
    Node2.DialogueEntry.DialogueText = FText::FromString("Excellent! Take position on the eastern ridge. Strike when the sun touches the peaks.");
    Node2.DialogueEntry.Duration = 10.0f;
    Node2.bIsEndNode = true;
    HuntConversation.Add(Node2);
    
    // Node 3: Preparation response
    FNarr_ConversationNode Node3;
    Node3.NodeID = "hunt_prepare";
    Node3.DialogueEntry.DialogueID = "hunt_prepare_response";
    Node3.DialogueEntry.Speaker = ENarr_CharacterRole::HuntMaster;
    Node3.DialogueEntry.Type = ENarr_DialogueType::Instruction;
    Node3.DialogueEntry.DialogueText = FText::FromString("Wise choice. Gather your spears and check your gear. The hunt waits for no one.");
    Node3.DialogueEntry.Duration = 12.0f;
    Node3.bIsEndNode = true;
    HuntConversation.Add(Node3);
    
    RegisterConversation("hunt_preparation", HuntConversation);
    
    UE_LOG(LogTemp, Log, TEXT("Initialized %d default conversations"), ConversationDatabase.Num());
}

FNarr_ConversationNode* UNarr_DialogueManager::GetConversationNode(const FString& ConversationID, const FString& NodeID)
{
    TArray<FNarr_ConversationNode>* Nodes = ConversationDatabase.Find(ConversationID);
    if (!Nodes)
    {
        return nullptr;
    }
    
    for (FNarr_ConversationNode& Node : *Nodes)
    {
        if (Node.NodeID == NodeID)
        {
            return &Node;
        }
    }
    
    return nullptr;
}

void UNarr_DialogueManager::ProcessDialogueConditions(const FNarr_DialogueEntry& Entry)
{
    // Process any required conditions for the dialogue
    // This could check player stats, quest progress, time of day, etc.
    for (const FString& Condition : Entry.RequiredConditions)
    {
        UE_LOG(LogTemp, Log, TEXT("Processing dialogue condition: %s"), *Condition);
        // Add condition logic here as needed
    }
}