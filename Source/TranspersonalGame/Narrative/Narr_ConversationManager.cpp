#include "Narr_ConversationManager.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"

UNarr_ConversationManager::UNarr_ConversationManager()
{
    PrimaryComponentTick.bCanEverTick = false;
    bIsInConversation = false;
    CurrentNPC = nullptr;
    CurrentDialogueNode = nullptr;
}

void UNarr_ConversationManager::BeginPlay()
{
    Super::BeginPlay();
    CreateSampleDialogue();
}

bool UNarr_ConversationManager::StartConversation(AActor* NPC, const FString& ConversationID)
{
    if (bIsInConversation || !NPC)
    {
        return false;
    }

    LoadConversationData(ConversationID);
    
    if (DialogueNodes.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("No dialogue nodes found for conversation: %s"), *ConversationID);
        return false;
    }

    bIsInConversation = true;
    CurrentNPC = NPC;
    CurrentConversationID = ConversationID;
    
    // Start with the first node (assuming "start" is the entry point)
    NavigateToNode(TEXT("start"));
    
    OnConversationStarted.Broadcast(NPC, ConversationID);
    
    UE_LOG(LogTemp, Warning, TEXT("Started conversation %s with %s"), *ConversationID, *NPC->GetName());
    
    return true;
}

void UNarr_ConversationManager::EndConversation()
{
    if (!bIsInConversation)
    {
        return;
    }

    bIsInConversation = false;
    CurrentNPC = nullptr;
    CurrentConversationID = TEXT("");
    CurrentDialogueNode = nullptr;
    
    OnConversationEnded.Broadcast();
    
    UE_LOG(LogTemp, Warning, TEXT("Conversation ended"));
}

bool UNarr_ConversationManager::SelectDialogueChoice(int32 ChoiceIndex)
{
    if (!bIsInConversation || !CurrentDialogueNode)
    {
        return false;
    }

    TArray<FString> PlayerInventory = GetPlayerInventory();
    
    if (!CurrentDialogueNode->CanSelectChoice(ChoiceIndex, PlayerInventory))
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot select choice %d - requirements not met"), ChoiceIndex);
        return false;
    }

    FString NextNodeID = CurrentDialogueNode->GetNextNodeID(ChoiceIndex);
    
    if (NextNodeID.IsEmpty() || NextNodeID == TEXT("end"))
    {
        EndConversation();
        return true;
    }

    NavigateToNode(NextNodeID);
    return true;
}

void UNarr_ConversationManager::LoadConversationData(const FString& ConversationID)
{
    DialogueNodes.Empty();
    
    // For now, use the sample dialogue created in BeginPlay
    // In a full implementation, this would load from data assets or JSON files
}

void UNarr_ConversationManager::NavigateToNode(const FString& NodeID)
{
    UNarr_DialogueNode** FoundNode = DialogueNodes.Find(NodeID);
    
    if (FoundNode && *FoundNode)
    {
        CurrentDialogueNode = *FoundNode;
        OnDialogueNodeChanged.Broadcast(CurrentDialogueNode);
        
        // Trigger any events associated with this node
        if (!CurrentDialogueNode->TriggerEventID.IsEmpty())
        {
            UE_LOG(LogTemp, Warning, TEXT("Triggering event: %s"), *CurrentDialogueNode->TriggerEventID);
        }
        
        // Auto-end conversation if this node has no choices and ends conversation
        if (CurrentDialogueNode->PlayerChoices.Num() == 0 && CurrentDialogueNode->bEndsConversation)
        {
            EndConversation();
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Dialogue node not found: %s"), *NodeID);
        EndConversation();
    }
}

TArray<FString> UNarr_ConversationManager::GetPlayerInventory() const
{
    TArray<FString> Inventory;
    
    // Placeholder - in full implementation, this would query the player's actual inventory
    Inventory.Add(TEXT("stone_spear"));
    Inventory.Add(TEXT("hide_pouch"));
    
    return Inventory;
}

void UNarr_ConversationManager::CreateSampleDialogue()
{
    // Create sample dialogue for Elder Thok
    UNarr_DialogueNode* StartNode = NewObject<UNarr_DialogueNode>(this);
    StartNode->NodeID = TEXT("start");
    StartNode->NodeType = ENarr_DialogueNodeType::NPCResponse;
    StartNode->SpeakerName = FText::FromString(TEXT("Elder Thok"));
    StartNode->DialogueText = FText::FromString(TEXT("Greetings, young hunter. The ancient grounds hold many secrets. What brings you to my fire?"));
    
    FNarr_DialogueChoice Choice1;
    Choice1.ChoiceText = FText::FromString(TEXT("I seek knowledge of the great beasts."));
    Choice1.NextNodeID = TEXT("beast_knowledge");
    StartNode->PlayerChoices.Add(Choice1);
    
    FNarr_DialogueChoice Choice2;
    Choice2.ChoiceText = FText::FromString(TEXT("I need supplies for hunting."));
    Choice2.NextNodeID = TEXT("supplies");
    StartNode->PlayerChoices.Add(Choice2);
    
    DialogueNodes.Add(TEXT("start"), StartNode);
    
    // Beast knowledge node
    UNarr_DialogueNode* BeastNode = NewObject<UNarr_DialogueNode>(this);
    BeastNode->NodeID = TEXT("beast_knowledge");
    BeastNode->NodeType = ENarr_DialogueNodeType::NPCResponse;
    BeastNode->SpeakerName = FText::FromString(TEXT("Elder Thok"));
    BeastNode->DialogueText = FText::FromString(TEXT("The Tyrannosaurus rules through fear, but even it fears the pack hunters. Watch for the Velociraptors - they strike from shadows."));
    BeastNode->bEndsConversation = true;
    
    DialogueNodes.Add(TEXT("beast_knowledge"), BeastNode);
    
    // Supplies node
    UNarr_DialogueNode* SuppliesNode = NewObject<UNarr_DialogueNode>(this);
    SuppliesNode->NodeID = TEXT("supplies");
    SuppliesNode->NodeType = ENarr_DialogueNodeType::Trader;
    SuppliesNode->SpeakerName = FText::FromString(TEXT("Elder Thok"));
    SuppliesNode->DialogueText = FText::FromString(TEXT("I have sharpened stones and dried meat. What can you offer in trade?"));
    SuppliesNode->bEndsConversation = true;
    
    DialogueNodes.Add(TEXT("supplies"), SuppliesNode);
}