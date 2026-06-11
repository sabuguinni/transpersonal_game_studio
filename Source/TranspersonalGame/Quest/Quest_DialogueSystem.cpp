#include "Quest_DialogueSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

UQuest_DialogueSystem::UQuest_DialogueSystem()
{
    bIsDialogueActive = false;
    CurrentNPCName = TEXT("");
    CurrentLineIndex = 0;
}

void UQuest_DialogueSystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    InitializeDefaultDialogues();
    
    UE_LOG(LogTemp, Warning, TEXT("Quest Dialogue System initialized"));
}

void UQuest_DialogueSystem::RegisterDialogueTree(const FQuest_DialogueTree& DialogueTree)
{
    if (!DialogueTree.NPCName.IsEmpty())
    {
        DialogueTrees.Add(DialogueTree.NPCName, DialogueTree);
        UE_LOG(LogTemp, Warning, TEXT("Registered dialogue tree for NPC: %s"), *DialogueTree.NPCName);
    }
}

FQuest_DialogueTree UQuest_DialogueSystem::GetDialogueTreeForNPC(const FString& NPCName)
{
    if (DialogueTrees.Contains(NPCName))
    {
        return DialogueTrees[NPCName];
    }
    
    return FQuest_DialogueTree();
}

bool UQuest_DialogueSystem::StartDialogue(const FString& NPCName, AActor* PlayerActor)
{
    if (bIsDialogueActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot start dialogue - dialogue already active"));
        return false;
    }
    
    if (!DialogueTrees.Contains(NPCName))
    {
        UE_LOG(LogTemp, Warning, TEXT("No dialogue tree found for NPC: %s"), *NPCName);
        return false;
    }
    
    FQuest_DialogueTree DialogueTree = DialogueTrees[NPCName];
    
    if (!CanAccessDialogue(DialogueTree))
    {
        UE_LOG(LogTemp, Warning, TEXT("Player cannot access dialogue for NPC: %s"), *NPCName);
        return false;
    }
    
    bIsDialogueActive = true;
    CurrentNPCName = NPCName;
    CurrentLineIndex = 0;
    
    // Play first dialogue line if available
    if (DialogueTree.DialogueLines.Num() > 0)
    {
        PlayDialogueLine(DialogueTree.DialogueLines[0]);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Started dialogue with NPC: %s"), *NPCName);
    return true;
}

void UQuest_DialogueSystem::PlayDialogueLine(const FQuest_DialogueLine& DialogueLine)
{
    // Log the dialogue line for now
    UE_LOG(LogTemp, Warning, TEXT("%s: %s"), *DialogueLine.SpeakerName, *DialogueLine.DialogueText);
    
    // In a full implementation, this would:
    // 1. Display UI with dialogue text
    // 2. Play audio if AudioURL is provided
    // 3. Handle timing based on Duration
    // 4. Trigger quest events if bIsQuestRelated is true
}

void UQuest_DialogueSystem::EndDialogue()
{
    bIsDialogueActive = false;
    CurrentNPCName = TEXT("");
    CurrentLineIndex = 0;
    
    UE_LOG(LogTemp, Warning, TEXT("Dialogue ended"));
}

bool UQuest_DialogueSystem::IsDialogueActive() const
{
    return bIsDialogueActive;
}

void UQuest_DialogueSystem::InitializeDefaultDialogues()
{
    // Hunt Master Dialogue
    FQuest_DialogueTree HuntMasterTree;
    HuntMasterTree.TreeID = TEXT("hunt_master_01");
    HuntMasterTree.NPCName = TEXT("HuntMaster_NPC");
    HuntMasterTree.bIsRepeatable = true;
    
    FQuest_DialogueLine HuntLine1;
    HuntLine1.SpeakerName = TEXT("Hunt Master");
    HuntLine1.DialogueText = TEXT("Greetings, survivor. I am the Hunt Master. The great beasts of this land hold the key to our tribe's survival.");
    HuntLine1.Duration = 5.0f;
    HuntLine1.bIsQuestRelated = true;
    
    FQuest_DialogueLine HuntLine2;
    HuntLine2.SpeakerName = TEXT("Hunt Master");
    HuntLine2.DialogueText = TEXT("Bring me proof of your hunting prowess - the hide of a mighty Triceratops, the claw of a swift Raptor, or if you dare... the tooth of the apex predator, the Tyrannosaurus Rex.");
    HuntLine2.Duration = 8.0f;
    HuntLine2.bIsQuestRelated = true;
    
    FQuest_DialogueLine HuntLine3;
    HuntLine3.SpeakerName = TEXT("Hunt Master");
    HuntLine3.DialogueText = TEXT("Each hunt will test your courage and skill. Are you ready to prove yourself as a true hunter of the ancient world?");
    HuntLine3.Duration = 6.0f;
    HuntLine3.bIsQuestRelated = true;
    
    HuntMasterTree.DialogueLines.Add(HuntLine1);
    HuntMasterTree.DialogueLines.Add(HuntLine2);
    HuntMasterTree.DialogueLines.Add(HuntLine3);
    
    RegisterDialogueTree(HuntMasterTree);
    
    // Gatherer Elder Dialogue
    FQuest_DialogueTree GathererTree;
    GathererTree.TreeID = TEXT("gatherer_elder_01");
    GathererTree.NPCName = TEXT("GathererElder_NPC");
    GathererTree.bIsRepeatable = true;
    
    FQuest_DialogueLine GatherLine1;
    GatherLine1.SpeakerName = TEXT("Gatherer Elder");
    GatherLine1.DialogueText = TEXT("Young one, the earth provides all we need, but only to those who know where to look and how to gather with respect.");
    GatherLine1.Duration = 6.0f;
    GatherLine1.bIsQuestRelated = true;
    
    FQuest_DialogueLine GatherLine2;
    GatherLine2.SpeakerName = TEXT("Gatherer Elder");
    GatherLine2.DialogueText = TEXT("The sacred stones near the canyon hold power for our tools. The ancient trees whisper secrets of strong wood for our shelters.");
    GatherLine2.Duration = 7.0f;
    GatherLine2.bIsQuestRelated = true;
    
    FQuest_DialogueLine GatherLine3;
    GatherLine3.SpeakerName = TEXT("Gatherer Elder");
    GatherLine3.DialogueText = TEXT("Learn the ways of gathering, and you will never go hungry. But remember - take only what you need, for the land must provide for generations to come.");
    GatherLine3.Duration = 8.0f;
    GatherLine3.bIsQuestRelated = true;
    
    GathererTree.DialogueLines.Add(GatherLine1);
    GathererTree.DialogueLines.Add(GatherLine2);
    GathererTree.DialogueLines.Add(GatherLine3);
    
    RegisterDialogueTree(GathererTree);
    
    // Scout Dialogue
    FQuest_DialogueTree ScoutTree;
    ScoutTree.TreeID = TEXT("scout_01");
    ScoutTree.NPCName = TEXT("Scout_NPC");
    ScoutTree.bIsRepeatable = true;
    
    FQuest_DialogueLine ScoutLine1;
    ScoutLine1.SpeakerName = TEXT("Scout");
    ScoutLine1.DialogueText = TEXT("I've seen movement beyond the ridge. Large shapes moving through the mist. We must know what dangers lurk in the unknown territories.");
    ScoutLine1.Duration = 7.0f;
    ScoutLine1.bIsQuestRelated = true;
    
    FQuest_DialogueLine ScoutLine2;
    ScoutLine2.SpeakerName = TEXT("Scout");
    ScoutLine2.DialogueText = TEXT("Will you venture into the unexplored lands? Map the territories, mark the dangers, and return with knowledge that could save our people?");
    ScoutLine2.Duration = 6.0f;
    ScoutLine2.bIsQuestRelated = true;
    
    ScoutTree.DialogueLines.Add(ScoutLine1);
    ScoutTree.DialogueLines.Add(ScoutLine2);
    
    RegisterDialogueTree(ScoutTree);
}

bool UQuest_DialogueSystem::CanAccessDialogue(const FQuest_DialogueTree& DialogueTree)
{
    // For now, all dialogues are accessible
    // In a full implementation, this would check:
    // 1. Required quest completion status
    // 2. Player level/stats
    // 3. Previous dialogue history
    // 4. Time-based restrictions
    
    return true;
}