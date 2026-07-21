#include "Quest_NPCDialogueManager.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"

UQuest_NPCDialogueManager::UQuest_NPCDialogueManager()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    NPCName = TEXT("Unnamed NPC");
    NPCRole = ENPCRole::Villager;
    RelationshipLevel = 0.5f;
    bCanGiveQuests = true;
    
    // Initialize default personality
    NPCPersonality = FQuest_NPCPersonality();
}

void UQuest_NPCDialogueManager::BeginPlay()
{
    Super::BeginPlay();
    
    SetupDialogueForRole();
}

void UQuest_NPCDialogueManager::InitializeNPCRole(ENPCRole Role)
{
    NPCRole = Role;
    
    // Set personality based on role
    switch (Role)
    {
        case ENPCRole::Hunter:
            NPCPersonality.Aggressiveness = 0.8f;
            NPCPersonality.Helpfulness = 0.6f;
            NPCPersonality.Wisdom = 0.7f;
            NPCPersonality.Fearfulness = 0.2f;
            NPCPersonality.Trustworthiness = 0.8f;
            NPCName = TEXT("Experienced Hunter");
            break;
            
        case ENPCRole::Elder:
            NPCPersonality.Aggressiveness = 0.2f;
            NPCPersonality.Helpfulness = 0.9f;
            NPCPersonality.Wisdom = 0.95f;
            NPCPersonality.Fearfulness = 0.3f;
            NPCPersonality.Trustworthiness = 0.9f;
            NPCName = TEXT("Village Elder");
            break;
            
        case ENPCRole::Scout:
            NPCPersonality.Aggressiveness = 0.5f;
            NPCPersonality.Helpfulness = 0.7f;
            NPCPersonality.Wisdom = 0.6f;
            NPCPersonality.Fearfulness = 0.4f;
            NPCPersonality.Trustworthiness = 0.7f;
            NPCName = TEXT("Swift Scout");
            break;
            
        case ENPCRole::Gatherer:
            NPCPersonality.Aggressiveness = 0.3f;
            NPCPersonality.Helpfulness = 0.8f;
            NPCPersonality.Wisdom = 0.6f;
            NPCPersonality.Fearfulness = 0.5f;
            NPCPersonality.Trustworthiness = 0.8f;
            NPCName = TEXT("Skilled Gatherer");
            break;
            
        case ENPCRole::Warrior:
            NPCPersonality.Aggressiveness = 0.9f;
            NPCPersonality.Helpfulness = 0.5f;
            NPCPersonality.Wisdom = 0.4f;
            NPCPersonality.Fearfulness = 0.1f;
            NPCPersonality.Trustworthiness = 0.7f;
            NPCName = TEXT("Fierce Warrior");
            break;
            
        default:
            NPCPersonality = FQuest_NPCPersonality();
            NPCName = TEXT("Tribal Member");
            break;
    }
    
    SetupDialogueForRole();
}

void UQuest_NPCDialogueManager::SetupDialogueForRole()
{
    DialogueNodes.Empty();
    AvailableQuestIDs.Empty();
    
    switch (NPCRole)
    {
        case ENPCRole::Hunter:
        {
            FQuest_DialogueNode huntNode;
            huntNode.DialogueText = TEXT("The great beasts roam these lands. I can teach you to track them, but first you must prove your courage.");
            huntNode.PlayerResponses.Add(TEXT("I'm ready to learn hunting."));
            huntNode.PlayerResponses.Add(TEXT("What dangers should I know about?"));
            huntNode.PlayerResponses.Add(TEXT("I'll come back when I'm stronger."));
            huntNode.bIsQuestGiver = true;
            huntNode.QuestID = TEXT("HUNT_BASIC_TRAINING");
            huntNode.EmotionalWeight = 0.7f;
            DialogueNodes.Add(huntNode);
            
            AvailableQuestIDs.Add(TEXT("HUNT_BASIC_TRAINING"));
            AvailableQuestIDs.Add(TEXT("HUNT_DANGEROUS_PREDATOR"));
            break;
        }
        
        case ENPCRole::Elder:
        {
            FQuest_DialogueNode wisdomNode;
            wisdomNode.DialogueText = TEXT("Young one, our ancestors survived by understanding this land. Let me share their wisdom with you.");
            wisdomNode.PlayerResponses.Add(TEXT("Please teach me about survival."));
            wisdomNode.PlayerResponses.Add(TEXT("Tell me about the ancient ways."));
            wisdomNode.PlayerResponses.Add(TEXT("I seek knowledge of this place."));
            wisdomNode.bIsQuestGiver = true;
            wisdomNode.QuestID = TEXT("WISDOM_ANCIENT_KNOWLEDGE");
            wisdomNode.EmotionalWeight = 0.9f;
            DialogueNodes.Add(wisdomNode);
            
            AvailableQuestIDs.Add(TEXT("WISDOM_ANCIENT_KNOWLEDGE"));
            AvailableQuestIDs.Add(TEXT("EXPLORATION_SACRED_SITES"));
            break;
        }
        
        case ENPCRole::Scout:
        {
            FQuest_DialogueNode scoutNode;
            scoutNode.DialogueText = TEXT("I've seen paths through dangerous territory. If you're brave enough, I can guide you to places others fear to tread.");
            scoutNode.PlayerResponses.Add(TEXT("Show me these hidden paths."));
            scoutNode.PlayerResponses.Add(TEXT("What have you discovered?"));
            scoutNode.PlayerResponses.Add(TEXT("I need safe passage somewhere."));
            scoutNode.bIsQuestGiver = true;
            scoutNode.QuestID = TEXT("EXPLORATION_HIDDEN_PATHS");
            scoutNode.EmotionalWeight = 0.6f;
            DialogueNodes.Add(scoutNode);
            
            AvailableQuestIDs.Add(TEXT("EXPLORATION_HIDDEN_PATHS"));
            AvailableQuestIDs.Add(TEXT("EXPLORATION_MAP_TERRITORY"));
            break;
        }
        
        case ENPCRole::Gatherer:
        {
            FQuest_DialogueNode gatherNode;
            gatherNode.DialogueText = TEXT("The land provides for those who know where to look. I can teach you to find what you need to survive.");
            gatherNode.PlayerResponses.Add(TEXT("Teach me about gathering resources."));
            gatherNode.PlayerResponses.Add(TEXT("Where can I find rare materials?"));
            gatherNode.PlayerResponses.Add(TEXT("I need help finding food."));
            gatherNode.bIsQuestGiver = true;
            gatherNode.QuestID = TEXT("GATHER_SURVIVAL_RESOURCES");
            gatherNode.EmotionalWeight = 0.8f;
            DialogueNodes.Add(gatherNode);
            
            AvailableQuestIDs.Add(TEXT("GATHER_SURVIVAL_RESOURCES"));
            AvailableQuestIDs.Add(TEXT("GATHER_RARE_MATERIALS"));
            break;
        }
        
        case ENPCRole::Warrior:
        {
            FQuest_DialogueNode warNode;
            warNode.DialogueText = TEXT("Combat is survival. The weak become prey. Show me you have the strength to face what lurks in the shadows."));
            warNode.PlayerResponses.Add(TEXT("Train me in combat."));
            warNode.PlayerResponses.Add(TEXT("I'm ready to fight."));
            warNode.PlayerResponses.Add(TEXT("What enemies threaten us?"));
            warNode.bIsQuestGiver = true;
            warNode.QuestID = TEXT("COMBAT_WARRIOR_TRAINING");
            warNode.EmotionalWeight = 0.5f;
            DialogueNodes.Add(warNode);
            
            AvailableQuestIDs.Add(TEXT("COMBAT_WARRIOR_TRAINING"));
            AvailableQuestIDs.Add(TEXT("COMBAT_DEFEND_SETTLEMENT"));
            break;
        }
        
        default:
        {
            FQuest_DialogueNode defaultNode;
            defaultNode.DialogueText = TEXT("Greetings, traveler. How can I help you?");
            defaultNode.PlayerResponses.Add(TEXT("Just passing through."));
            defaultNode.PlayerResponses.Add(TEXT("Tell me about this place."));
            defaultNode.bIsQuestGiver = false;
            defaultNode.EmotionalWeight = 0.5f;
            DialogueNodes.Add(defaultNode);
            break;
        }
    }
}

FQuest_DialogueNode UQuest_NPCDialogueManager::GetDialogueForPlayer(APawn* Player)
{
    if (DialogueNodes.Num() == 0)
    {
        SetupDialogueForRole();
    }
    
    if (DialogueNodes.Num() > 0)
    {
        FQuest_DialogueNode selectedNode = DialogueNodes[0];
        
        // Modify dialogue based on relationship level
        if (RelationshipLevel > 0.7f)
        {
            selectedNode.DialogueText = GetPersonalityModifiedDialogue(selectedNode.DialogueText);
        }
        
        return selectedNode;
    }
    
    // Fallback dialogue
    FQuest_DialogueNode fallback;
    fallback.DialogueText = GetRoleBasedGreeting();
    fallback.PlayerResponses.Add(TEXT("Farewell."));
    return fallback;
}

void UQuest_NPCDialogueManager::ProcessPlayerResponse(const FString& Response, APawn* Player)
{
    // Update relationship based on response
    if (Response.Contains(TEXT("teach")) || Response.Contains(TEXT("learn")))
    {
        UpdateRelationship(0.1f);
    }
    else if (Response.Contains(TEXT("ready")) || Response.Contains(TEXT("brave")))
    {
        UpdateRelationship(0.15f);
    }
    else if (Response.Contains(TEXT("back")) || Response.Contains(TEXT("later")))
    {
        UpdateRelationship(-0.05f);
    }
    
    UE_LOG(LogTemp, Log, TEXT("NPC %s processed player response: %s"), *NPCName, *Response);
}

bool UQuest_NPCDialogueManager::CanOfferQuest(const FString& QuestID) const
{
    return bCanGiveQuests && AvailableQuestIDs.Contains(QuestID) && RelationshipLevel >= 0.3f;
}

void UQuest_NPCDialogueManager::UpdateRelationship(float DeltaRelationship)
{
    RelationshipLevel = FMath::Clamp(RelationshipLevel + DeltaRelationship, 0.0f, 1.0f);
    
    UE_LOG(LogTemp, Log, TEXT("NPC %s relationship updated to: %f"), *NPCName, RelationshipLevel);
}

FString UQuest_NPCDialogueManager::GenerateContextualDialogue(APawn* Player)
{
    FString baseDialogue = GetRoleBasedGreeting();
    return GetPersonalityModifiedDialogue(baseDialogue);
}

FString UQuest_NPCDialogueManager::GetRoleBasedGreeting() const
{
    switch (NPCRole)
    {
        case ENPCRole::Hunter:
            return TEXT("The hunt calls to those with courage.");
        case ENPCRole::Elder:
            return TEXT("Wisdom comes to those who seek it, young one.");
        case ENPCRole::Scout:
            return TEXT("The paths ahead are treacherous but rewarding.");
        case ENPCRole::Gatherer:
            return TEXT("The earth provides for those who respect it.");
        case ENPCRole::Warrior:
            return TEXT("Strength is earned through trial and combat.");
        default:
            return TEXT("Welcome, traveler.");
    }
}

FString UQuest_NPCDialogueManager::GetPersonalityModifiedDialogue(const FString& BaseDialogue) const
{
    FString modifiedDialogue = BaseDialogue;
    
    if (NPCPersonality.Aggressiveness > 0.7f)
    {
        modifiedDialogue += TEXT(" Prove yourself worthy!");
    }
    else if (NPCPersonality.Helpfulness > 0.8f)
    {
        modifiedDialogue += TEXT(" I'm here to help you succeed.");
    }
    
    if (NPCPersonality.Wisdom > 0.8f)
    {
        modifiedDialogue += TEXT(" Remember, survival requires both strength and knowledge.");
    }
    
    if (RelationshipLevel > 0.8f)
    {
        modifiedDialogue = TEXT("My trusted friend, ") + modifiedDialogue.ToLower();
    }
    else if (RelationshipLevel < 0.3f)
    {
        modifiedDialogue = TEXT("Stranger, ") + modifiedDialogue.ToLower();
    }
    
    return modifiedDialogue;
}