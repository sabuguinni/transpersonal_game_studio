#include "DialogueManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

UDialogueManager::UDialogueManager()
{
    bIsDialogueActive = false;
    CurrentNPCName = TEXT("");
    CurrentDialogueTreeID = TEXT("");
    CurrentDialogueIndex = 0;
    CurrentStoryPhase = ENarr_StoryPhase::Introduction;
}

void UDialogueManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("DialogueManager: Initializing narrative system"));
    
    // Initialize default dialogues and story progression
    InitializeDefaultDialogues();
    
    // Set initial story phase
    SetStoryPhase(ENarr_StoryPhase::Introduction);
    
    UE_LOG(LogTemp, Log, TEXT("DialogueManager: Initialization complete"));
}

void UDialogueManager::Deinitialize()
{
    // Clean up dialogue state
    EndDialogue();
    DialogueTrees.Empty();
    StoryProgressValues.Empty();
    PlayerActionCounts.Empty();
    
    Super::Deinitialize();
}

void UDialogueManager::StartDialogue(const FString& NPCName, const FString& DialogueTreeID)
{
    if (bIsDialogueActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("DialogueManager: Cannot start dialogue - another dialogue is active"));
        return;
    }

    if (!DialogueTrees.Contains(DialogueTreeID))
    {
        UE_LOG(LogTemp, Error, TEXT("DialogueManager: Dialogue tree not found: %s"), *DialogueTreeID);
        return;
    }

    bIsDialogueActive = true;
    CurrentNPCName = NPCName;
    CurrentDialogueTreeID = DialogueTreeID;
    CurrentDialogueIndex = 0;

    UE_LOG(LogTemp, Log, TEXT("DialogueManager: Started dialogue with %s using tree %s"), *NPCName, *DialogueTreeID);
    
    // Broadcast dialogue started event
    OnDialogueStarted.Broadcast(NPCName, DialogueTreeID);
}

void UDialogueManager::EndDialogue()
{
    if (!bIsDialogueActive)
    {
        return;
    }

    FString EndedDialogueID = CurrentDialogueTreeID;
    
    bIsDialogueActive = false;
    CurrentNPCName = TEXT("");
    CurrentDialogueTreeID = TEXT("");
    CurrentDialogueIndex = 0;

    UE_LOG(LogTemp, Log, TEXT("DialogueManager: Ended dialogue"));
    
    // Broadcast dialogue ended event
    OnDialogueEnded.Broadcast(EndedDialogueID);
}

bool UDialogueManager::IsDialogueActive() const
{
    return bIsDialogueActive;
}

FNarr_DialogueLine UDialogueManager::GetCurrentDialogueLine() const
{
    if (!bIsDialogueActive || !DialogueTrees.Contains(CurrentDialogueTreeID))
    {
        return FNarr_DialogueLine();
    }

    const FNarr_DialogueTree& CurrentTree = DialogueTrees[CurrentDialogueTreeID];
    
    if (CurrentDialogueIndex >= 0 && CurrentDialogueIndex < CurrentTree.DialogueLines.Num())
    {
        return CurrentTree.DialogueLines[CurrentDialogueIndex];
    }

    return FNarr_DialogueLine();
}

void UDialogueManager::AdvanceDialogue(int32 ResponseIndex)
{
    if (!bIsDialogueActive)
    {
        return;
    }

    CurrentDialogueIndex++;
    
    const FNarr_DialogueTree& CurrentTree = DialogueTrees[CurrentDialogueTreeID];
    
    // Check if we've reached the end of the dialogue
    if (CurrentDialogueIndex >= CurrentTree.DialogueLines.Num())
    {
        EndDialogue();
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("DialogueManager: Advanced to dialogue line %d"), CurrentDialogueIndex);
}

void UDialogueManager::RegisterDialogueTree(const FNarr_DialogueTree& DialogueTree)
{
    if (!ValidateDialogueTree(DialogueTree))
    {
        UE_LOG(LogTemp, Error, TEXT("DialogueManager: Invalid dialogue tree: %s"), *DialogueTree.TreeID);
        return;
    }

    DialogueTrees.Add(DialogueTree.TreeID, DialogueTree);
    UE_LOG(LogTemp, Log, TEXT("DialogueManager: Registered dialogue tree: %s"), *DialogueTree.TreeID);
}

FNarr_DialogueTree UDialogueManager::GetDialogueTree(const FString& TreeID) const
{
    if (DialogueTrees.Contains(TreeID))
    {
        return DialogueTrees[TreeID];
    }
    
    return FNarr_DialogueTree();
}

TArray<FString> UDialogueManager::GetAvailableDialogues(const FString& NPCName) const
{
    TArray<FString> AvailableDialogues;
    
    for (const auto& TreePair : DialogueTrees)
    {
        if (TreePair.Value.NPCName == NPCName)
        {
            AvailableDialogues.Add(TreePair.Key);
        }
    }
    
    return AvailableDialogues;
}

void UDialogueManager::SetStoryPhase(ENarr_StoryPhase NewPhase)
{
    ENarr_StoryPhase OldPhase = CurrentStoryPhase;
    CurrentStoryPhase = NewPhase;
    
    UE_LOG(LogTemp, Log, TEXT("DialogueManager: Story phase changed from %d to %d"), 
           (int32)OldPhase, (int32)NewPhase);
    
    // Broadcast story phase change event
    OnStoryPhaseChanged.Broadcast(OldPhase, NewPhase);
}

ENarr_StoryPhase UDialogueManager::GetCurrentStoryPhase() const
{
    return CurrentStoryPhase;
}

void UDialogueManager::UpdateStoryProgress(const FString& EventName, float ProgressValue)
{
    StoryProgressValues.Add(EventName, ProgressValue);
    
    UE_LOG(LogTemp, Log, TEXT("DialogueManager: Updated story progress - %s: %f"), 
           *EventName, ProgressValue);
    
    // Check for story phase transitions based on progress
    if (EventName == TEXT("FirstHunt") && ProgressValue >= 1.0f && CurrentStoryPhase == ENarr_StoryPhase::EarlyExploration)
    {
        SetStoryPhase(ENarr_StoryPhase::FirstHunt);
    }
    else if (EventName == TEXT("TribeMembers") && ProgressValue >= 3.0f && CurrentStoryPhase == ENarr_StoryPhase::FirstHunt)
    {
        SetStoryPhase(ENarr_StoryPhase::TribeBuilding);
    }
}

void UDialogueManager::OnQuestCompleted(const FString& QuestID)
{
    UE_LOG(LogTemp, Log, TEXT("DialogueManager: Quest completed: %s"), *QuestID);
    
    // Update story progress based on quest completion
    if (QuestID == TEXT("RaptorThreat"))
    {
        UpdateStoryProgress(TEXT("FirstHunt"), 1.0f);
    }
    else if (QuestID == TEXT("CrystalHarvest"))
    {
        UpdateStoryProgress(TEXT("ResourceGathering"), 1.0f);
    }
}

void UDialogueManager::OnQuestStarted(const FString& QuestID)
{
    UE_LOG(LogTemp, Log, TEXT("DialogueManager: Quest started: %s"), *QuestID);
    
    // Track quest initiation for story progression
    TrackPlayerAction(TEXT("QuestStarted"), QuestID);
}

void UDialogueManager::TrackPlayerAction(const FString& ActionType, const FString& ActionData)
{
    if (PlayerActionCounts.Contains(ActionType))
    {
        PlayerActionCounts[ActionType]++;
    }
    else
    {
        PlayerActionCounts.Add(ActionType, 1);
    }
    
    UE_LOG(LogTemp, Log, TEXT("DialogueManager: Tracked action %s (count: %d)"), 
           *ActionType, PlayerActionCounts[ActionType]);
}

int32 UDialogueManager::GetPlayerActionCount(const FString& ActionType) const
{
    if (PlayerActionCounts.Contains(ActionType))
    {
        return PlayerActionCounts[ActionType];
    }
    
    return 0;
}

void UDialogueManager::InitializeDefaultDialogues()
{
    CreateSurvivalDialogues();
    CreateQuestDialogues();
}

void UDialogueManager::CreateSurvivalDialogues()
{
    // Tribal Elder introduction dialogue
    FNarr_DialogueTree ElderIntro;
    ElderIntro.TreeID = TEXT("ElderIntroduction");
    ElderIntro.NPCName = TEXT("TribalElder");
    ElderIntro.bIsQuestRelated = false;
    
    FNarr_DialogueLine ElderLine1;
    ElderLine1.SpeakerName = TEXT("Tribal Elder");
    ElderLine1.DialogueText = FText::FromString(TEXT("Welcome, young hunter. You have survived your first night in these dangerous lands. That alone marks you as stronger than most."));
    ElderLine1.DisplayDuration = 4.0f;
    ElderLine1.PlayerResponseOptions.Add(TEXT("Thank you for the guidance."));
    ElderLine1.PlayerResponseOptions.Add(TEXT("What dangers should I know about?"));
    
    FNarr_DialogueLine ElderLine2;
    ElderLine2.SpeakerName = TEXT("Tribal Elder");
    ElderLine2.DialogueText = FText::FromString(TEXT("The great beasts follow ancient patterns. Learn their rhythms - when they hunt, when they rest, when they migrate. This knowledge will keep you alive."));
    ElderLine2.DisplayDuration = 5.0f;
    
    ElderIntro.DialogueLines.Add(ElderLine1);
    ElderIntro.DialogueLines.Add(ElderLine2);
    
    RegisterDialogueTree(ElderIntro);
    
    // Scout warning dialogue
    FNarr_DialogueTree ScoutWarning;
    ScoutWarning.TreeID = TEXT("ScoutWarning");
    ScoutWarning.NPCName = TEXT("Scout");
    ScoutWarning.bIsQuestRelated = true;
    ScoutWarning.AssociatedQuestID = TEXT("RaptorThreat");
    
    FNarr_DialogueLine ScoutLine1;
    ScoutLine1.SpeakerName = TEXT("Scout");
    ScoutLine1.DialogueText = FText::FromString(TEXT("The pack hunters have been circling our territory. Three of them, moving in coordinated patterns. They're learning our routines."));
    ScoutLine1.DisplayDuration = 4.0f;
    ScoutLine1.PlayerResponseOptions.Add(TEXT("I'll help deal with them."));
    ScoutLine1.PlayerResponseOptions.Add(TEXT("How do we avoid them?"));
    
    ScoutWarning.DialogueLines.Add(ScoutLine1);
    RegisterDialogueTree(ScoutWarning);
}

void UDialogueManager::CreateQuestDialogues()
{
    // Hunter quest giver dialogue
    FNarr_DialogueTree HunterQuest;
    HunterQuest.TreeID = TEXT("HunterQuestGiver");
    HunterQuest.NPCName = TEXT("Hunter");
    HunterQuest.bIsQuestRelated = true;
    HunterQuest.AssociatedQuestID = TEXT("RaptorThreat");
    
    FNarr_DialogueLine HunterLine1;
    HunterLine1.SpeakerName = TEXT("Hunter");
    HunterLine1.DialogueText = FText::FromString(TEXT("The raptors have been hunting too close to our camp. We need someone brave enough to thin their numbers before they become bolder."));
    HunterLine1.DisplayDuration = 4.0f;
    HunterLine1.PlayerResponseOptions.Add(TEXT("I accept this hunt."));
    HunterLine1.PlayerResponseOptions.Add(TEXT("Tell me more about these raptors."));
    
    HunterQuest.DialogueLines.Add(HunterLine1);
    RegisterDialogueTree(HunterQuest);
    
    // Gatherer quest giver dialogue
    FNarr_DialogueTree GathererQuest;
    GathererQuest.TreeID = TEXT("GathererQuestGiver");
    GathererQuest.NPCName = TEXT("Gatherer");
    GathererQuest.bIsQuestRelated = true;
    GathererQuest.AssociatedQuestID = TEXT("CrystalHarvest");
    
    FNarr_DialogueLine GathererLine1;
    GathererLine1.SpeakerName = TEXT("Gatherer");
    GathererLine1.DialogueText = FText::FromString(TEXT("The crystal formations near the old riverbed hold powerful energy. We need those shards to craft better tools and weapons."));
    GathererLine1.DisplayDuration = 4.0f;
    GathererLine1.PlayerResponseOptions.Add(TEXT("I'll gather the crystals."));
    GathererLine1.PlayerResponseOptions.Add(TEXT("Where exactly is this riverbed?"));
    
    GathererQuest.DialogueLines.Add(GathererLine1);
    RegisterDialogueTree(GathererQuest);
}

bool UDialogueManager::ValidateDialogueTree(const FNarr_DialogueTree& DialogueTree) const
{
    if (DialogueTree.TreeID.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("DialogueManager: Dialogue tree missing TreeID"));
        return false;
    }
    
    if (DialogueTree.NPCName.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("DialogueManager: Dialogue tree missing NPCName"));
        return false;
    }
    
    if (DialogueTree.DialogueLines.Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("DialogueManager: Dialogue tree has no dialogue lines"));
        return false;
    }
    
    return true;
}