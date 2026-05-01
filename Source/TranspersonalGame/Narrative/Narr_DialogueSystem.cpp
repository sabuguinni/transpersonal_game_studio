#include "Narr_DialogueSystem.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/Engine.h"

UNarr_DialogueSystem::UNarr_DialogueSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    CurrentTree = nullptr;
    bIsDialogueActive = false;
    DialogueTimer = 0.0f;
    NPCType = ENarr_NPCType::TribalElder;
    DialogueAudioComponent = nullptr;
}

void UNarr_DialogueSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Create audio component for dialogue playback
    DialogueAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("DialogueAudioComponent"));
    if (DialogueAudioComponent)
    {
        DialogueAudioComponent->bAutoActivate = false;
        DialogueAudioComponent->SetVolumeMultiplier(1.0f);
    }
    
    InitializeDefaultDialogueTrees();
    
    UE_LOG(LogTemp, Log, TEXT("Narr_DialogueSystem initialized with %d dialogue trees"), DialogueTrees.Num());
}

void UNarr_DialogueSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bIsDialogueActive && CurrentTree)
    {
        DialogueTimer += DeltaTime;
        
        // Auto-advance non-choice dialogue after duration
        if (CurrentTree->CurrentNodeIndex < CurrentTree->DialogueNodes.Num())
        {
            const FNarr_DialogueNode& CurrentNode = CurrentTree->DialogueNodes[CurrentTree->CurrentNodeIndex];
            if (!CurrentNode.bIsPlayerChoice && DialogueTimer >= CurrentNode.DisplayDuration)
            {
                AdvanceDialogue();
            }
        }
    }
}

void UNarr_DialogueSystem::StartDialogue(const FString& TreeName)
{
    FNarr_DialogueTree* Tree = GetDialogueTree(TreeName);
    if (Tree && Tree->DialogueNodes.Num() > 0)
    {
        CurrentTree = Tree;
        CurrentTree->CurrentNodeIndex = 0;
        CurrentTree->bIsActive = true;
        bIsDialogueActive = true;
        DialogueTimer = 0.0f;
        
        // Play first node audio if available
        const FNarr_DialogueNode& FirstNode = CurrentTree->DialogueNodes[0];
        if (!FirstNode.AudioPath.IsEmpty())
        {
            PlayDialogueAudio(FirstNode.AudioPath);
        }
        
        UE_LOG(LogTemp, Log, TEXT("Started dialogue tree: %s"), *TreeName);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to start dialogue tree: %s"), *TreeName);
    }
}

void UNarr_DialogueSystem::EndDialogue()
{
    if (CurrentTree)
    {
        CurrentTree->bIsActive = false;
        CurrentTree = nullptr;
    }
    
    bIsDialogueActive = false;
    DialogueTimer = 0.0f;
    
    StopDialogueAudio();
    
    UE_LOG(LogTemp, Log, TEXT("Dialogue ended"));
}

void UNarr_DialogueSystem::AdvanceDialogue(int32 ChoiceIndex)
{
    if (!CurrentTree || !bIsDialogueActive)
    {
        return;
    }
    
    if (CurrentTree->CurrentNodeIndex >= CurrentTree->DialogueNodes.Num())
    {
        EndDialogue();
        return;
    }
    
    const FNarr_DialogueNode& CurrentNode = CurrentTree->DialogueNodes[CurrentTree->CurrentNodeIndex];
    
    // Determine next node
    int32 NextNodeIndex = -1;
    if (CurrentNode.NextNodeIndices.Num() > 0)
    {
        if (CurrentNode.bIsPlayerChoice && ChoiceIndex < CurrentNode.NextNodeIndices.Num())
        {
            NextNodeIndex = CurrentNode.NextNodeIndices[ChoiceIndex];
        }
        else
        {
            NextNodeIndex = CurrentNode.NextNodeIndices[0];
        }
    }
    
    if (NextNodeIndex >= 0 && NextNodeIndex < CurrentTree->DialogueNodes.Num())
    {
        CurrentTree->CurrentNodeIndex = NextNodeIndex;
        DialogueTimer = 0.0f;
        
        // Play next node audio
        const FNarr_DialogueNode& NextNode = CurrentTree->DialogueNodes[NextNodeIndex];
        if (!NextNode.AudioPath.IsEmpty())
        {
            PlayDialogueAudio(NextNode.AudioPath);
        }
        
        UE_LOG(LogTemp, Log, TEXT("Advanced to dialogue node %d"), NextNodeIndex);
    }
    else
    {
        EndDialogue();
    }
}

bool UNarr_DialogueSystem::IsDialogueActive() const
{
    return bIsDialogueActive;
}

FNarr_DialogueNode UNarr_DialogueSystem::GetCurrentDialogueNode() const
{
    if (CurrentTree && CurrentTree->CurrentNodeIndex < CurrentTree->DialogueNodes.Num())
    {
        return CurrentTree->DialogueNodes[CurrentTree->CurrentNodeIndex];
    }
    
    return FNarr_DialogueNode();
}

void UNarr_DialogueSystem::AddDialogueTree(const FNarr_DialogueTree& NewTree)
{
    DialogueTrees.Add(NewTree);
    UE_LOG(LogTemp, Log, TEXT("Added dialogue tree: %s"), *NewTree.TreeName);
}

void UNarr_DialogueSystem::RemoveDialogueTree(const FString& TreeName)
{
    DialogueTrees.RemoveAll([&TreeName](const FNarr_DialogueTree& Tree)
    {
        return Tree.TreeName == TreeName;
    });
    
    UE_LOG(LogTemp, Log, TEXT("Removed dialogue tree: %s"), *TreeName);
}

FNarr_DialogueTree* UNarr_DialogueSystem::GetDialogueTree(const FString& TreeName)
{
    for (FNarr_DialogueTree& Tree : DialogueTrees)
    {
        if (Tree.TreeName == TreeName)
        {
            return &Tree;
        }
    }
    
    return nullptr;
}

void UNarr_DialogueSystem::PlayDialogueAudio(const FString& AudioPath)
{
    if (DialogueAudioComponent && !AudioPath.IsEmpty())
    {
        // In a real implementation, you would load the audio asset from the path
        // For now, we just log the audio path
        UE_LOG(LogTemp, Log, TEXT("Playing dialogue audio: %s"), *AudioPath);
        
        // Stop current audio
        StopDialogueAudio();
        
        // Play new audio (placeholder implementation)
        // DialogueAudioComponent->SetSound(LoadedSoundCue);
        // DialogueAudioComponent->Play();
    }
}

void UNarr_DialogueSystem::StopDialogueAudio()
{
    if (DialogueAudioComponent && DialogueAudioComponent->IsPlaying())
    {
        DialogueAudioComponent->Stop();
    }
}

void UNarr_DialogueSystem::SetNPCType(ENarr_NPCType NewNPCType)
{
    NPCType = NewNPCType;
    UE_LOG(LogTemp, Log, TEXT("NPC type set to: %d"), (int32)NPCType);
}

ENarr_NPCType UNarr_DialogueSystem::GetNPCType() const
{
    return NPCType;
}

void UNarr_DialogueSystem::TriggerQuestDialogue(EQuest_MissionType MissionType)
{
    FString TreeName;
    
    switch (MissionType)
    {
        case EQuest_MissionType::WaterGathering:
            TreeName = TEXT("ElderWaterQuest");
            break;
        case EQuest_MissionType::FoodForaging:
            TreeName = TEXT("GathererFoodQuest");
            break;
        case EQuest_MissionType::ShelterBuilding:
            TreeName = TEXT("ElderShelterQuest");
            break;
        case EQuest_MissionType::ToolCrafting:
            TreeName = TEXT("HunterToolQuest");
            break;
        case EQuest_MissionType::TerritoryMapping:
            TreeName = TEXT("HunterTerritoryQuest");
            break;
        case EQuest_MissionType::PredatorAvoidance:
            TreeName = TEXT("ShamanDangerQuest");
            break;
        default:
            TreeName = TEXT("ElderDefault");
            break;
    }
    
    StartDialogue(TreeName);
    UE_LOG(LogTemp, Log, TEXT("Triggered quest dialogue for mission type: %d"), (int32)MissionType);
}

void UNarr_DialogueSystem::CompleteQuestDialogue(EQuest_MissionType MissionType)
{
    FString CompletionTreeName = FString::Printf(TEXT("Complete_%d"), (int32)MissionType);
    StartDialogue(CompletionTreeName);
    UE_LOG(LogTemp, Log, TEXT("Triggered quest completion dialogue for mission type: %d"), (int32)MissionType);
}

void UNarr_DialogueSystem::InitializeDefaultDialogueTrees()
{
    CreateElderDialogueTree();
    CreateHunterDialogueTree();
    CreateGathererDialogueTree();
    CreateShamanDialogueTree();
}

void UNarr_DialogueSystem::CreateElderDialogueTree()
{
    FNarr_DialogueTree ElderTree;
    ElderTree.TreeName = TEXT("ElderDefault");
    ElderTree.NPCType = ENarr_NPCType::TribalElder;
    
    // Node 0: Greeting
    FNarr_DialogueNode GreetingNode = CreateDialogueNode(
        TEXT("Welcome, young survivor. The wilderness tests us all, but those who learn its ways shall endure."),
        TEXT("Tribal Elder"),
        TEXT("TribalElder_Greeting.mp3"),
        4.0f
    );
    GreetingNode.NextNodeIndices.Add(1);
    
    // Node 1: Wisdom
    FNarr_DialogueNode WisdomNode = CreateDialogueNode(
        TEXT("I have knowledge of water sources hidden in the eastern ravines. Bring me proof of your courage, and I will share this wisdom."),
        TEXT("Tribal Elder"),
        TEXT("TribalElder_Wisdom.mp3"),
        5.0f
    );
    
    ElderTree.DialogueNodes.Add(GreetingNode);
    ElderTree.DialogueNodes.Add(WisdomNode);
    
    AddDialogueTree(ElderTree);
    
    // Water Quest Tree
    FNarr_DialogueTree WaterQuestTree;
    WaterQuestTree.TreeName = TEXT("ElderWaterQuest");
    WaterQuestTree.NPCType = ENarr_NPCType::TribalElder;
    
    FNarr_DialogueNode WaterQuestNode = CreateDialogueNode(
        TEXT("The streams run dry in the heat, but ancient springs flow beneath the stone. Find the hidden waters and return with proof."),
        TEXT("Tribal Elder"),
        TEXT("TribalElder_WaterQuest.mp3"),
        4.5f
    );
    
    WaterQuestTree.DialogueNodes.Add(WaterQuestNode);
    AddDialogueTree(WaterQuestTree);
}

void UNarr_DialogueSystem::CreateHunterDialogueTree()
{
    FNarr_DialogueTree HunterTree;
    HunterTree.TreeName = TEXT("HunterDefault");
    HunterTree.NPCType = ENarr_NPCType::Hunter;
    
    FNarr_DialogueNode HunterGreeting = CreateDialogueNode(
        TEXT("The hunt calls to those with steady hands and patient hearts. I've tracked the great beasts to their watering grounds."),
        TEXT("Hunter"),
        TEXT("Hunter_Greeting.mp3"),
        4.0f
    );
    HunterGreeting.NextNodeIndices.Add(1);
    
    FNarr_DialogueNode HunterWisdom = CreateDialogueNode(
        TEXT("Complete my trial of stealth and cunning, and I'll teach you the ancient ways of the silent hunter."),
        TEXT("Hunter"),
        TEXT("Hunter_Wisdom.mp3"),
        4.0f
    );
    
    HunterTree.DialogueNodes.Add(HunterGreeting);
    HunterTree.DialogueNodes.Add(HunterWisdom);
    
    AddDialogueTree(HunterTree);
}

void UNarr_DialogueSystem::CreateGathererDialogueTree()
{
    FNarr_DialogueTree GathererTree;
    GathererTree.TreeName = TEXT("GathererDefault");
    GathererTree.NPCType = ENarr_NPCType::Gatherer;
    
    FNarr_DialogueNode GathererGreeting = CreateDialogueNode(
        TEXT("The earth provides for those who know where to look. Berries, roots, healing herbs - all have their seasons and places."),
        TEXT("Gatherer"),
        TEXT("Gatherer_Greeting.mp3"),
        4.5f
    );
    
    GathererTree.DialogueNodes.Add(GathererGreeting);
    AddDialogueTree(GathererTree);
}

void UNarr_DialogueSystem::CreateShamanDialogueTree()
{
    FNarr_DialogueTree ShamanTree;
    ShamanTree.TreeName = TEXT("ShamanDefault");
    ShamanTree.NPCType = ENarr_NPCType::WiseShaman;
    
    FNarr_DialogueNode ShamanGreeting = CreateDialogueNode(
        TEXT("Listen well, stranger. The great lizards follow ancient patterns. When the earth trembles and birds fall silent, the tyrant king approaches."),
        TEXT("Wise Shaman"),
        TEXT("WiseShaman_Warning.mp3"),
        5.0f
    );
    ShamanGreeting.NextNodeIndices.Add(1);
    
    FNarr_DialogueNode ShamanAdvice = CreateDialogueNode(
        TEXT("Run to high ground and pray he has already fed. These lands have claimed many before you."),
        TEXT("Wise Shaman"),
        TEXT("WiseShaman_Advice.mp3"),
        4.0f
    );
    
    ShamanTree.DialogueNodes.Add(ShamanGreeting);
    ShamanTree.DialogueNodes.Add(ShamanAdvice);
    
    AddDialogueTree(ShamanTree);
}

FNarr_DialogueNode UNarr_DialogueSystem::CreateDialogueNode(const FString& Text, const FString& Speaker, 
                                                           const FString& AudioPath, float Duration, bool bIsChoice)
{
    FNarr_DialogueNode Node;
    Node.DialogueText = Text;
    Node.SpeakerName = Speaker;
    Node.AudioPath = AudioPath;
    Node.DisplayDuration = Duration;
    Node.bIsPlayerChoice = bIsChoice;
    Node.TriggerCondition = ENarr_DialogueTrigger::Proximity;
    
    return Node;
}