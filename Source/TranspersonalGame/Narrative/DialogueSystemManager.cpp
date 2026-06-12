#include "DialogueSystemManager.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/Engine.h"
#include "TimerManager.h"

ADialogueSystemManager::ADialogueSystemManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Create audio component
    AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
    RootComponent = AudioComponent;
    
    // Initialize default values
    DialogueVolume = 0.8f;
    DialogueFadeTime = 0.5f;
    bIsDialogueActive = false;
    CurrentTreeID = TEXT("");
    CurrentLineIndex = 0;
    DialogueTimer = 0.0f;
}

void ADialogueSystemManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize default dialogue conditions
    DialogueConditions.Add(TEXT("FirstMeeting"), true);
    DialogueConditions.Add(TEXT("HasHuntedDinosaur"), false);
    DialogueConditions.Add(TEXT("KnowsAboutCave"), false);
    DialogueConditions.Add(TEXT("HasCraftedWeapon"), false);
    DialogueConditions.Add(TEXT("SurvivedNight"), false);
    DialogueConditions.Add(TEXT("MetAllNPCs"), false);
    
    // Initialize default dialogue trees
    InitializeDefaultDialogues();
    
    UE_LOG(LogTemp, Warning, TEXT("DialogueSystemManager initialized with %d dialogue trees"), DialogueTrees.Num());
}

void ADialogueSystemManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bIsDialogueActive)
    {
        DialogueTimer += DeltaTime;
        ProcessDialogueLine();
    }
}

void ADialogueSystemManager::StartDialogue(const FString& TreeID, ENarr_NPCType NPCType)
{
    // Find the dialogue tree
    FNarr_DialogueTree* FoundTree = nullptr;
    for (FNarr_DialogueTree& Tree : DialogueTrees)
    {
        if (Tree.TreeID == TreeID)
        {
            FoundTree = &Tree;
            break;
        }
    }
    
    if (!FoundTree)
    {
        UE_LOG(LogTemp, Warning, TEXT("Dialogue tree not found: %s"), *TreeID);
        return;
    }
    
    // Check if dialogue is available
    if (FoundTree->DialogueLines.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Dialogue tree has no lines: %s"), *TreeID);
        return;
    }
    
    // Start the dialogue
    bIsDialogueActive = true;
    CurrentTreeID = TreeID;
    CurrentLineIndex = 0;
    DialogueTimer = 0.0f;
    
    OnDialogueStarted(TreeID);
    UE_LOG(LogTemp, Log, TEXT("Started dialogue: %s"), *TreeID);
}

void ADialogueSystemManager::StopDialogue()
{
    if (bIsDialogueActive)
    {
        bIsDialogueActive = false;
        
        // Stop audio
        if (AudioComponent && AudioComponent->IsPlaying())
        {
            AudioComponent->FadeOut(DialogueFadeTime, 0.0f);
        }
        
        OnDialogueEnded(CurrentTreeID);
        UE_LOG(LogTemp, Log, TEXT("Stopped dialogue: %s"), *CurrentTreeID);
        
        CurrentTreeID = TEXT("");
        CurrentLineIndex = 0;
        DialogueTimer = 0.0f;
    }
}

bool ADialogueSystemManager::IsDialogueActive() const
{
    return bIsDialogueActive;
}

void ADialogueSystemManager::SetDialogueCondition(const FString& ConditionName, bool bValue)
{
    DialogueConditions.Add(ConditionName, bValue);
    UE_LOG(LogTemp, Log, TEXT("Set dialogue condition: %s = %s"), *ConditionName, bValue ? TEXT("true") : TEXT("false"));
}

bool ADialogueSystemManager::CheckDialogueCondition(const FString& ConditionName) const
{
    const bool* FoundCondition = DialogueConditions.Find(ConditionName);
    return FoundCondition ? *FoundCondition : false;
}

TArray<FNarr_DialogueTree> ADialogueSystemManager::GetAvailableDialogues(ENarr_NPCType NPCType) const
{
    TArray<FNarr_DialogueTree> AvailableDialogues;
    
    for (const FNarr_DialogueTree& Tree : DialogueTrees)
    {
        bool bIsAvailable = true;
        
        // Check if any dialogue line matches the NPC type
        bool bHasMatchingNPC = false;
        for (const FNarr_DialogueLine& Line : Tree.DialogueLines)
        {
            if (Line.SpeakerType == NPCType)
            {
                bHasMatchingNPC = true;
                
                // Check line conditions
                if (!CheckLineConditions(Line))
                {
                    bIsAvailable = false;
                    break;
                }
            }
        }
        
        if (bHasMatchingNPC && bIsAvailable)
        {
            AvailableDialogues.Add(Tree);
        }
    }
    
    // Sort by priority
    AvailableDialogues.Sort([](const FNarr_DialogueTree& A, const FNarr_DialogueTree& B) {
        return A.Priority > B.Priority;
    });
    
    return AvailableDialogues;
}

void ADialogueSystemManager::RegisterDialogueTree(const FNarr_DialogueTree& NewTree)
{
    // Check if tree already exists
    for (int32 i = 0; i < DialogueTrees.Num(); i++)
    {
        if (DialogueTrees[i].TreeID == NewTree.TreeID)
        {
            DialogueTrees[i] = NewTree;
            UE_LOG(LogTemp, Log, TEXT("Updated dialogue tree: %s"), *NewTree.TreeID);
            return;
        }
    }
    
    // Add new tree
    DialogueTrees.Add(NewTree);
    UE_LOG(LogTemp, Log, TEXT("Registered new dialogue tree: %s"), *NewTree.TreeID);
}

void ADialogueSystemManager::PlayDialogueAudio(const FNarr_DialogueLine& DialogueLine)
{
    if (AudioComponent && DialogueLine.VoiceAudio.IsValid())
    {
        USoundCue* SoundCue = DialogueLine.VoiceAudio.LoadSynchronous();
        if (SoundCue)
        {
            AudioComponent->SetSound(SoundCue);
            AudioComponent->SetVolumeMultiplier(DialogueVolume);
            AudioComponent->Play();
        }
    }
}

void ADialogueSystemManager::ProcessDialogueLine()
{
    // Find current dialogue tree
    FNarr_DialogueTree* CurrentTree = nullptr;
    for (FNarr_DialogueTree& Tree : DialogueTrees)
    {
        if (Tree.TreeID == CurrentTreeID)
        {
            CurrentTree = &Tree;
            break;
        }
    }
    
    if (!CurrentTree || CurrentLineIndex >= CurrentTree->DialogueLines.Num())
    {
        StopDialogue();
        return;
    }
    
    const FNarr_DialogueLine& CurrentLine = CurrentTree->DialogueLines[CurrentLineIndex];
    
    // Check if enough time has passed for this line
    if (DialogueTimer >= CurrentLine.Duration)
    {
        CurrentLineIndex++;
        DialogueTimer = 0.0f;
        
        if (CurrentLineIndex < CurrentTree->DialogueLines.Num())
        {
            const FNarr_DialogueLine& NextLine = CurrentTree->DialogueLines[CurrentLineIndex];
            OnDialogueLineChanged(NextLine);
            PlayDialogueAudio(NextLine);
        }
        else
        {
            StopDialogue();
        }
    }
}

bool ADialogueSystemManager::CheckLineConditions(const FNarr_DialogueLine& Line) const
{
    for (const FString& Condition : Line.RequiredConditions)
    {
        if (!CheckDialogueCondition(Condition))
        {
            return false;
        }
    }
    return true;
}

void ADialogueSystemManager::InitializeDefaultDialogues()
{
    // Elder NPC Introduction Dialogue
    FNarr_DialogueTree ElderIntro;
    ElderIntro.TreeID = TEXT("Elder_Introduction");
    ElderIntro.bIsRepeatable = false;
    ElderIntro.Priority = 10;
    
    FNarr_DialogueLine ElderLine1;
    ElderLine1.DialogueText = TEXT("Welcome, young hunter. I am the keeper of our tribe's memory. In this harsh world, knowledge is survival.");
    ElderLine1.DialogueType = ENarr_DialogueType::Story;
    ElderLine1.SpeakerType = ENarr_NPCType::Elder;
    ElderLine1.Duration = 6.0f;
    ElderLine1.RequiredConditions.Add(TEXT("FirstMeeting"));
    
    FNarr_DialogueLine ElderLine2;
    ElderLine2.DialogueText = TEXT("The great beasts rule this land, but they follow patterns. Learn these patterns, and you might live to see another dawn.");
    ElderLine2.DialogueType = ENarr_DialogueType::Tutorial;
    ElderLine2.SpeakerType = ENarr_NPCType::Elder;
    ElderLine2.Duration = 7.0f;
    
    ElderIntro.DialogueLines.Add(ElderLine1);
    ElderIntro.DialogueLines.Add(ElderLine2);
    
    // Hunter NPC Warning Dialogue
    FNarr_DialogueTree HunterWarning;
    HunterWarning.TreeID = TEXT("Hunter_PackWarning");
    HunterWarning.bIsRepeatable = true;
    HunterWarning.Priority = 8;
    
    FNarr_DialogueLine HunterLine1;
    HunterLine1.DialogueText = TEXT("I've tracked a pack of raptors moving through the eastern canyon. Eight of them, maybe more.");
    HunterLine1.DialogueType = ENarr_DialogueType::Warning;
    HunterLine1.SpeakerType = ENarr_NPCType::Hunter;
    HunterLine1.Duration = 5.0f;
    
    FNarr_DialogueLine HunterLine2;
    HunterLine2.DialogueText = TEXT("Stay away from that area until they move on. No prey is worth becoming prey yourself.");
    HunterLine2.DialogueType = ENarr_DialogueType::Warning;
    HunterLine2.SpeakerType = ENarr_NPCType::Hunter;
    HunterLine2.Duration = 4.0f;
    
    HunterWarning.DialogueLines.Add(HunterLine1);
    HunterWarning.DialogueLines.Add(HunterLine2);
    
    // Shaman NPC Lore Dialogue
    FNarr_DialogueTree ShamanLore;
    ShamanLore.TreeID = TEXT("Shaman_AncientWisdom");
    ShamanLore.bIsRepeatable = false;
    ShamanLore.Priority = 6;
    
    FNarr_DialogueLine ShamanLine1;
    ShamanLine1.DialogueText = TEXT("The old caves hold secrets from the time before. Paintings on the walls tell of the first hunters.");
    ShamanLine1.DialogueType = ENarr_DialogueType::Story;
    ShamanLine1.SpeakerType = ENarr_NPCType::Shaman;
    ShamanLine1.Duration = 6.0f;
    ShamanLine1.RequiredConditions.Add(TEXT("KnowsAboutCave"));
    
    FNarr_DialogueLine ShamanLine2;
    ShamanLine2.DialogueText = TEXT("They learned to read the signs - the way the earth trembles before the thunder lizard walks, the silence that comes before the pack hunts.");
    ShamanLine2.DialogueType = ENarr_DialogueType::Tutorial;
    ShamanLine2.SpeakerType = ENarr_NPCType::Shaman;
    ShamanLine2.Duration = 8.0f;
    
    ShamanLore.DialogueLines.Add(ShamanLine1);
    ShamanLore.DialogueLines.Add(ShamanLine2);
    
    // Register all dialogue trees
    RegisterDialogueTree(ElderIntro);
    RegisterDialogueTree(HunterWarning);
    RegisterDialogueTree(ShamanLore);
}