#include "Narr_DialogueManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"

void UNarr_DialogueManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    bIsDialogueActive = false;
    CurrentNPCName = TEXT("");
    CurrentDialogueIndex = 0;
    CurrentPlayerPawn = nullptr;
    
    InitializeDefaultNPCs();
    InitializeDefaultDialogues();
    
    UE_LOG(LogTemp, Warning, TEXT("Narrative Dialogue Manager initialized"));
}

void UNarr_DialogueManager::Deinitialize()
{
    EndDialogue();
    RegisteredNPCs.Empty();
    DialogueTrees.Empty();
    
    Super::Deinitialize();
}

bool UNarr_DialogueManager::StartDialogue(const FString& NPCName, APawn* PlayerPawn)
{
    if (bIsDialogueActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot start dialogue - already in dialogue"));
        return false;
    }
    
    if (!RegisteredNPCs.Contains(NPCName))
    {
        UE_LOG(LogTemp, Warning, TEXT("NPC not found: %s"), *NPCName);
        return false;
    }
    
    if (!DialogueTrees.Contains(NPCName))
    {
        UE_LOG(LogTemp, Warning, TEXT("No dialogue tree for NPC: %s"), *NPCName);
        return false;
    }
    
    bIsDialogueActive = true;
    CurrentNPCName = NPCName;
    CurrentDialogueIndex = 0;
    CurrentPlayerPawn = PlayerPawn;
    
    UE_LOG(LogTemp, Warning, TEXT("Started dialogue with: %s"), *NPCName);
    return true;
}

void UNarr_DialogueManager::EndDialogue()
{
    if (!bIsDialogueActive)
    {
        return;
    }
    
    bIsDialogueActive = false;
    CurrentNPCName = TEXT("");
    CurrentDialogueIndex = 0;
    CurrentPlayerPawn = nullptr;
    
    UE_LOG(LogTemp, Warning, TEXT("Dialogue ended"));
}

FNarr_DialogueLine UNarr_DialogueManager::GetCurrentDialogueLine() const
{
    if (!bIsDialogueActive || !DialogueTrees.Contains(CurrentNPCName))
    {
        return FNarr_DialogueLine();
    }
    
    const FNarr_DialogueTree& Tree = DialogueTrees[CurrentNPCName];
    if (Tree.DialogueLines.IsValidIndex(CurrentDialogueIndex))
    {
        return Tree.DialogueLines[CurrentDialogueIndex];
    }
    
    return FNarr_DialogueLine();
}

void UNarr_DialogueManager::AdvanceDialogue(int32 ResponseIndex)
{
    if (!bIsDialogueActive || !DialogueTrees.Contains(CurrentNPCName))
    {
        return;
    }
    
    const FNarr_DialogueTree& Tree = DialogueTrees[CurrentNPCName];
    CurrentDialogueIndex++;
    
    if (CurrentDialogueIndex >= Tree.DialogueLines.Num())
    {
        EndDialogue();
    }
}

void UNarr_DialogueManager::RegisterNPC(const FNarr_NPCProfile& NPCProfile)
{
    if (NPCProfile.NPCName.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot register NPC with empty name"));
        return;
    }
    
    RegisteredNPCs.Add(NPCProfile.NPCName, NPCProfile);
    UE_LOG(LogTemp, Warning, TEXT("Registered NPC: %s"), *NPCProfile.NPCName);
}

FNarr_NPCProfile UNarr_DialogueManager::GetNPCProfile(const FString& NPCName) const
{
    if (RegisteredNPCs.Contains(NPCName))
    {
        return RegisteredNPCs[NPCName];
    }
    
    return FNarr_NPCProfile();
}

TArray<FString> UNarr_DialogueManager::GetNearbyNPCs(const FVector& PlayerLocation, float SearchRadius) const
{
    TArray<FString> NearbyNPCs;
    
    for (const auto& NPCPair : RegisteredNPCs)
    {
        const FNarr_NPCProfile& Profile = NPCPair.Value;
        float Distance = FVector::Dist(PlayerLocation, Profile.HomeLocation);
        
        if (Distance <= SearchRadius)
        {
            NearbyNPCs.Add(Profile.NPCName);
        }
    }
    
    return NearbyNPCs;
}

void UNarr_DialogueManager::LoadDialogueTree(const FNarr_DialogueTree& DialogueTree)
{
    if (!ValidateDialogueTree(DialogueTree))
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid dialogue tree for: %s"), *DialogueTree.NPCName);
        return;
    }
    
    DialogueTrees.Add(DialogueTree.NPCName, DialogueTree);
    UE_LOG(LogTemp, Warning, TEXT("Loaded dialogue tree for: %s"), *DialogueTree.NPCName);
}

FNarr_DialogueTree UNarr_DialogueManager::GetDialogueTree(const FString& NPCName) const
{
    if (DialogueTrees.Contains(NPCName))
    {
        return DialogueTrees[NPCName];
    }
    
    return FNarr_DialogueTree();
}

void UNarr_DialogueManager::TriggerQuestDialogue(const FString& QuestID, const FString& NPCName)
{
    if (!DialogueTrees.Contains(NPCName))
    {
        UE_LOG(LogTemp, Warning, TEXT("No dialogue tree for quest NPC: %s"), *NPCName);
        return;
    }
    
    const FNarr_DialogueTree& Tree = DialogueTrees[NPCName];
    if (Tree.bIsQuestRelated && Tree.QuestID == QuestID)
    {
        UE_LOG(LogTemp, Warning, TEXT("Triggered quest dialogue: %s for NPC: %s"), *QuestID, *NPCName);
    }
}

bool UNarr_DialogueManager::HasQuestDialogue(const FString& QuestID) const
{
    for (const auto& TreePair : DialogueTrees)
    {
        const FNarr_DialogueTree& Tree = TreePair.Value;
        if (Tree.bIsQuestRelated && Tree.QuestID == QuestID)
        {
            return true;
        }
    }
    
    return false;
}

void UNarr_DialogueManager::InitializeDefaultNPCs()
{
    // Tribal Elder
    FNarr_NPCProfile TribalElder;
    TribalElder.NPCName = TEXT("TribalElder");
    TribalElder.Personality = ENarr_NPCPersonality::Wise_Elder;
    TribalElder.BackgroundStory = TEXT("Ancient keeper of tribal wisdom, survivor of countless hunts and dangers. Speaks in riddles and warnings about the great predators.");
    TribalElder.SpecialKnowledge.Add(TEXT("Dinosaur behavior patterns"));
    TribalElder.SpecialKnowledge.Add(TEXT("Ancient hunting techniques"));
    TribalElder.SpecialKnowledge.Add(TEXT("Survival wisdom"));
    TribalElder.HomeLocation = FVector(2000, 0, 100);
    RegisterNPC(TribalElder);
    
    // Quest Giver
    FNarr_NPCProfile QuestGiver;
    QuestGiver.NPCName = TEXT("QuestGiver");
    QuestGiver.Personality = ENarr_NPCPersonality::Practical_Guide;
    QuestGiver.BackgroundStory = TEXT("Experienced hunter who coordinates resource gathering for the tribe. Practical and direct in speech, focused on survival needs.");
    QuestGiver.SpecialKnowledge.Add(TEXT("Resource locations"));
    QuestGiver.SpecialKnowledge.Add(TEXT("Crafting techniques"));
    QuestGiver.SpecialKnowledge.Add(TEXT("Territory mapping"));
    QuestGiver.HomeLocation = FVector(-2000, 1000, 100);
    RegisterNPC(QuestGiver);
    
    // Bone Reader
    FNarr_NPCProfile BoneReader;
    BoneReader.NPCName = TEXT("BoneReader");
    BoneReader.Personality = ENarr_NPCPersonality::Mystic_Reader;
    BoneReader.BackgroundStory = TEXT("Studies the remains of great beasts to understand their behavior and weaknesses. Speaks of bones as teachers and guides.");
    BoneReader.SpecialKnowledge.Add(TEXT("Dinosaur anatomy"));
    BoneReader.SpecialKnowledge.Add(TEXT("Combat strategies"));
    BoneReader.SpecialKnowledge.Add(TEXT("Bone crafting"));
    BoneReader.HomeLocation = FVector(0, 2000, 100);
    RegisterNPC(BoneReader);
    
    // Tracking Mentor
    FNarr_NPCProfile TrackingMentor;
    TrackingMentor.NPCName = TEXT("TrackingMentor");
    TrackingMentor.Personality = ENarr_NPCPersonality::Cautious_Scout;
    TrackingMentor.BackgroundStory = TEXT("Master tracker who teaches the art of reading signs and avoiding predators. Cautious but knowledgeable about safe paths.");
    TrackingMentor.SpecialKnowledge.Add(TEXT("Tracking techniques"));
    TrackingMentor.SpecialKnowledge.Add(TEXT("Predator avoidance"));
    TrackingMentor.SpecialKnowledge.Add(TEXT("Safe routes"));
    TrackingMentor.HomeLocation = FVector(1500, -1500, 100);
    RegisterNPC(TrackingMentor);
}

void UNarr_DialogueManager::InitializeDefaultDialogues()
{
    // Tribal Elder dialogue tree
    FNarr_DialogueTree ElderTree;
    ElderTree.TreeID = TEXT("TribalElder_Main");
    ElderTree.NPCName = TEXT("TribalElder");
    ElderTree.bIsQuestRelated = false;
    
    FNarr_DialogueLine ElderLine1;
    ElderLine1.SpeakerName = TEXT("Tribal Elder");
    ElderLine1.DialogueText = TEXT("The ancient hunters speak of the Great Hunt - when the earth trembles beneath the Thunder Lizard's feet and the sky darkens with the wings of death.");
    ElderLine1.AudioPath = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1778900005029_TribalElder.mp3");
    ElderLine1.Duration = 22.0f;
    ElderLine1.PlayerResponses.Add(TEXT("Tell me about the Thunder Lizard"));
    ElderLine1.PlayerResponses.Add(TEXT("How do I survive the pack hunters?"));
    
    FNarr_DialogueLine ElderLine2;
    ElderLine2.SpeakerName = TEXT("Tribal Elder");
    ElderLine2.DialogueText = TEXT("Listen well, young one, for survival depends on reading the signs. The pack hunters move in shadows, testing our defenses.");
    ElderLine2.Duration = 15.0f;
    ElderLine2.PlayerResponses.Add(TEXT("What signs should I watch for?"));
    ElderLine2.PlayerResponses.Add(TEXT("Thank you for the wisdom"));
    
    ElderTree.DialogueLines.Add(ElderLine1);
    ElderTree.DialogueLines.Add(ElderLine2);
    LoadDialogueTree(ElderTree);
    
    // Quest Giver dialogue tree
    FNarr_DialogueTree QuestTree;
    QuestTree.TreeID = TEXT("QuestGiver_ResourceGathering");
    QuestTree.NPCName = TEXT("QuestGiver");
    QuestTree.bIsQuestRelated = true;
    QuestTree.QuestID = TEXT("ResourceGathering_Stone");
    
    FNarr_DialogueLine QuestLine1;
    QuestLine1.SpeakerName = TEXT("Quest Giver");
    QuestLine1.DialogueText = TEXT("Hunter, you have proven yourself worthy. The resources you gather feed our tribe and strengthen our defenses.");
    QuestLine1.AudioPath = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1778900011245_QuestGiver.mp3");
    QuestLine1.Duration = 20.0f;
    QuestLine1.PlayerResponses.Add(TEXT("What resources do you need?"));
    QuestLine1.PlayerResponses.Add(TEXT("I'm ready for a mission"));
    
    QuestTree.DialogueLines.Add(QuestLine1);
    LoadDialogueTree(QuestTree);
}

bool UNarr_DialogueManager::ValidateDialogueTree(const FNarr_DialogueTree& Tree) const
{
    if (Tree.NPCName.IsEmpty() || Tree.TreeID.IsEmpty())
    {
        return false;
    }
    
    if (Tree.DialogueLines.Num() == 0)
    {
        return false;
    }
    
    for (const FNarr_DialogueLine& Line : Tree.DialogueLines)
    {
        if (Line.DialogueText.IsEmpty())
        {
            return false;
        }
    }
    
    return true;
}