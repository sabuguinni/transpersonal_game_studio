#include "NarrativeDialogueManager.h"
#include "NarrativeCharacterSystem.h"
#include "Engine/Engine.h"

UNarrativeDialogueManager::UNarrativeDialogueManager()
{
    PrimaryComponentTick.bCanEverTick = false;
    CurrentNodeID = 0;
    ActiveTreeID = TEXT("");
    CharacterSystemRef = nullptr;
}

void UNarrativeDialogueManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeDefaultDialogueTrees();
    
    // Find character system reference
    if (AActor* Owner = GetOwner())
    {
        CharacterSystemRef = Owner->FindComponentByClass<UNarrativeCharacterSystem>();
    }
}

bool UNarrativeDialogueManager::StartDialogue(const FString& TreeID, const FNarr_DialogueContext& Context)
{
    FNarr_DialogueTree* Tree = FindDialogueTree(TreeID);
    if (!Tree)
    {
        UE_LOG(LogTemp, Warning, TEXT("Dialogue tree not found: %s"), *TreeID);
        return false;
    }
    
    ActiveTreeID = TreeID;
    CurrentNodeID = Tree->RootNodeID;
    CurrentContext = Context;
    
    UE_LOG(LogTemp, Log, TEXT("Started dialogue tree: %s"), *TreeID);
    return true;
}

FNarr_DialogueNode UNarrativeDialogueManager::GetCurrentDialogueNode()
{
    FNarr_DialogueNode* Node = FindDialogueNode(ActiveTreeID, CurrentNodeID);
    if (Node)
    {
        return *Node;
    }
    
    return FNarr_DialogueNode();
}

bool UNarrativeDialogueManager::SelectPlayerResponse(int32 ResponseIndex)
{
    FNarr_DialogueNode* CurrentNode = FindDialogueNode(ActiveTreeID, CurrentNodeID);
    if (!CurrentNode || ResponseIndex >= CurrentNode->NextNodeIDs.Num())
    {
        return false;
    }
    
    CurrentNodeID = CurrentNode->NextNodeIDs[ResponseIndex];
    
    // Check if this is an end node
    FNarr_DialogueNode* NextNode = FindDialogueNode(ActiveTreeID, CurrentNodeID);
    if (!NextNode || NextNode->NextNodeIDs.Num() == 0)
    {
        EndDialogue();
    }
    
    return true;
}

void UNarrativeDialogueManager::EndDialogue()
{
    ActiveTreeID = TEXT("");
    CurrentNodeID = 0;
    UE_LOG(LogTemp, Log, TEXT("Dialogue ended"));
}

void UNarrativeDialogueManager::UpdateDialogueContext(const FNarr_DialogueContext& NewContext)
{
    CurrentContext = NewContext;
}

TArray<FString> UNarrativeDialogueManager::GetContextualDialogue(ENarr_CharacterType CharacterType, const FNarr_DialogueContext& Context)
{
    TArray<FString> ContextualLines;
    
    // Generate contextual dialogue based on character type and current situation
    switch (CharacterType)
    {
        case ENarr_CharacterType::TribalElder:
            if (Context.PlayerFearLevel > 75.0f)
            {
                ContextualLines.Add(TEXT("I see fear in your eyes, young one. The great beasts command respect, not terror."));
            }
            if (Context.NearbyDinosaurs.Contains(ENarr_DinosaurSpecies::TRex))
            {
                ContextualLines.Add(TEXT("The thunder-foot walks near. Seek shelter until it passes."));
            }
            break;
            
        case ENarr_CharacterType::HerdTracker:
            if (Context.CurrentBiome == ENarr_BiomeType::Floresta)
            {
                ContextualLines.Add(TEXT("The Brachiosaurus favor these ancient trees. Watch for their feeding patterns."));
            }
            if (Context.NearbyDinosaurs.Contains(ENarr_DinosaurSpecies::Triceratops))
            {
                ContextualLines.Add(TEXT("Three-horns are protective of their young. Approach with caution."));
            }
            break;
            
        case ENarr_CharacterType::SurvivalGuide:
            if (Context.PlayerHealthLevel < 50.0f)
            {
                ContextualLines.Add(TEXT("Your body weakens. Find shelter and tend to your wounds."));
            }
            if (Context.CurrentBiome == ENarr_BiomeType::Deserto)
            {
                ContextualLines.Add(TEXT("The desert tests all who enter. Water is life here."));
            }
            break;
    }
    
    return ContextualLines;
}

void UNarrativeDialogueManager::RegisterDialogueTree(const FNarr_DialogueTree& NewTree)
{
    // Remove existing tree with same ID
    DialogueTrees.RemoveAll([&](const FNarr_DialogueTree& Tree) 
    {
        return Tree.TreeID == NewTree.TreeID;
    });
    
    DialogueTrees.Add(NewTree);
    UE_LOG(LogTemp, Log, TEXT("Registered dialogue tree: %s"), *NewTree.TreeID);
}

bool UNarrativeDialogueManager::IsDialogueActive() const
{
    return !ActiveTreeID.IsEmpty();
}

FString UNarrativeDialogueManager::GenerateContextualGreeting(ENarr_CharacterType CharacterType, const FNarr_DialogueContext& Context)
{
    FString Greeting;
    
    switch (CharacterType)
    {
        case ENarr_CharacterType::TribalElder:
            if (Context.DaysInWorld < 3)
            {
                Greeting = TEXT("Welcome, new blood. The ancient world tests all who walk its paths.");
            }
            else
            {
                Greeting = TEXT("You have survived another day. The ancestors smile upon you.");
            }
            break;
            
        case ENarr_CharacterType::HerdTracker:
            if (Context.CurrentBiome == ENarr_BiomeType::Savana)
            {
                Greeting = TEXT("The grasslands hold many secrets. What brings you to the Savana?");
            }
            else
            {
                Greeting = TEXT("A fellow tracker! Share what you have seen in your travels.");
            }
            break;
            
        case ENarr_CharacterType::SurvivalGuide:
            if (Context.PlayerHealthLevel < 75.0f)
            {
                Greeting = TEXT("I see you carry wounds. Let me share what I know of healing.");
            }
            else
            {
                Greeting = TEXT("Strong and healthy - good. The wilderness respects the prepared.");
            }
            break;
            
        default:
            Greeting = TEXT("Greetings, traveler.");
            break;
    }
    
    return Greeting;
}

void UNarrativeDialogueManager::InitializeDefaultDialogueTrees()
{
    // Elder Introduction Tree
    FNarr_DialogueTree ElderIntroTree;
    ElderIntroTree.TreeID = TEXT("elder_introduction");
    ElderIntroTree.TreeName = TEXT("Elder Introduction");
    ElderIntroTree.AssociatedCharacterType = ENarr_CharacterType::TribalElder;
    ElderIntroTree.RootNodeID = 0;
    
    // Root node
    FNarr_DialogueNode RootNode;
    RootNode.DialogueText = TEXT("You have awakened in the time of giants. I am Keth, keeper of the old ways. Will you listen to the wisdom of ages?");
    RootNode.SpeakerName = TEXT("Elder Keth");
    RootNode.PlayerResponses.Add(TEXT("Yes, I need guidance."));
    RootNode.PlayerResponses.Add(TEXT("I can survive on my own."));
    RootNode.NextNodeIDs.Add(1);
    RootNode.NextNodeIDs.Add(2);
    RootNode.TriggerCondition = ENarr_DialogueTrigger::Always;
    ElderIntroTree.DialogueNodes.Add(RootNode);
    
    // Guidance path
    FNarr_DialogueNode GuidanceNode;
    GuidanceNode.DialogueText = TEXT("Wise choice. The great beasts are not our enemies - they are teachers. Learn their ways, and you may yet thrive.");
    GuidanceNode.SpeakerName = TEXT("Elder Keth");
    GuidanceNode.PlayerResponses.Add(TEXT("What should I know first?"));
    GuidanceNode.NextNodeIDs.Add(3);
    ElderIntroTree.DialogueNodes.Add(GuidanceNode);
    
    // Rejection path
    FNarr_DialogueNode RejectionNode;
    RejectionNode.DialogueText = TEXT("Pride comes before the fall, young one. When you are ready to learn, return to me."));
    RejectionNode.SpeakerName = TEXT("Elder Keth");
    RejectionNode.PlayerResponses.Add(TEXT("I understand."));
    RejectionNode.NextNodeIDs.Add(-1); // End dialogue
    ElderIntroTree.DialogueNodes.Add(RejectionNode);
    
    // Teaching node
    FNarr_DialogueNode TeachingNode;
    TeachingNode.DialogueText = TEXT("First - respect the territory of others. The thunder-foot rules the mountains, the long-necks the forests. Learn their patterns, and you will know where safety lies."));
    TeachingNode.SpeakerName = TEXT("Elder Keth");
    TeachingNode.PlayerResponses.Add(TEXT("Thank you for the wisdom."));
    TeachingNode.NextNodeIDs.Add(-1); // End dialogue
    ElderIntroTree.DialogueNodes.Add(TeachingNode);
    
    RegisterDialogueTree(ElderIntroTree);
    
    // Tracker Herd Info Tree
    FNarr_DialogueTree TrackerTree;
    TrackerTree.TreeID = TEXT("tracker_herd_info");
    TrackerTree.TreeName = TEXT("Herd Tracker Information");
    TrackerTree.AssociatedCharacterType = ENarr_CharacterType::HerdTracker;
    TrackerTree.RootNodeID = 0;
    
    FNarr_DialogueNode TrackerRoot;
    TrackerRoot.DialogueText = TEXT("I have tracked the great herds across all five biomes. Each holds different dangers and opportunities.");
    TrackerRoot.SpeakerName = TEXT("Tracker Naia");
    TrackerRoot.PlayerResponses.Add(TEXT("Tell me about the herds."));
    TrackerRoot.PlayerResponses.Add(TEXT("Which biome is safest?"));
    TrackerRoot.NextNodeIDs.Add(1);
    TrackerRoot.NextNodeIDs.Add(2);
    TrackerTree.DialogueNodes.Add(TrackerRoot);
    
    FNarr_DialogueNode HerdInfo;
    HerdInfo.DialogueText = TEXT("The three-horns gather in the Savana and Pantano. Long-necks prefer the Floresta. But beware - where prey gathers, predators follow."));
    HerdInfo.SpeakerName = TEXT("Tracker Naia");
    HerdInfo.PlayerResponses.Add(TEXT("What about predators?"));
    HerdInfo.NextNodeIDs.Add(3);
    TrackerTree.DialogueNodes.Add(HerdInfo);
    
    FNarr_DialogueNode SafetyInfo;
    SafetyInfo.DialogueText = TEXT("No biome is truly safe, but the Floresta offers the most shelter. The great trees hide you from the thunder-foot's gaze."));
    SafetyInfo.SpeakerName = TEXT("Tracker Naia");
    SafetyInfo.PlayerResponses.Add(TEXT("Good to know."));
    SafetyInfo.NextNodeIDs.Add(-1);
    TrackerTree.DialogueNodes.Add(SafetyInfo);
    
    FNarr_DialogueNode PredatorInfo;
    PredatorInfo.DialogueText = TEXT("The thunder-foot hunts alone but claims vast territory. The pack-hunters are smaller but cunning. Never let them surround you."));
    PredatorInfo.SpeakerName = TEXT("Tracker Naia");
    PredatorInfo.PlayerResponses.Add(TEXT("I'll remember that."));
    PredatorInfo.NextNodeIDs.Add(-1);
    TrackerTree.DialogueNodes.Add(PredatorInfo);
    
    RegisterDialogueTree(TrackerTree);
    
    UE_LOG(LogTemp, Log, TEXT("Initialized %d default dialogue trees"), DialogueTrees.Num());
}

bool UNarrativeDialogueManager::EvaluateDialogueTrigger(ENarr_DialogueTrigger Trigger, const FNarr_DialogueContext& Context)
{
    switch (Trigger)
    {
        case ENarr_DialogueTrigger::Always:
            return true;
            
        case ENarr_DialogueTrigger::FirstMeeting:
            return Context.DaysInWorld <= 1;
            
        case ENarr_DialogueTrigger::HighFear:
            return Context.PlayerFearLevel > 75.0f;
            
        case ENarr_DialogueTrigger::LowHealth:
            return Context.PlayerHealthLevel < 50.0f;
            
        case ENarr_DialogueTrigger::PredatorNearby:
            return Context.NearbyDinosaurs.Contains(ENarr_DinosaurSpecies::TRex) || 
                   Context.NearbyDinosaurs.Contains(ENarr_DinosaurSpecies::Velociraptor);
            
        default:
            return false;
    }
}

FNarr_DialogueTree* UNarrativeDialogueManager::FindDialogueTree(const FString& TreeID)
{
    for (FNarr_DialogueTree& Tree : DialogueTrees)
    {
        if (Tree.TreeID == TreeID)
        {
            return &Tree;
        }
    }
    return nullptr;
}

FNarr_DialogueNode* UNarrativeDialogueManager::FindDialogueNode(const FString& TreeID, int32 NodeID)
{
    FNarr_DialogueTree* Tree = FindDialogueTree(TreeID);
    if (!Tree || NodeID < 0 || NodeID >= Tree->DialogueNodes.Num())
    {
        return nullptr;
    }
    
    return &Tree->DialogueNodes[NodeID];
}