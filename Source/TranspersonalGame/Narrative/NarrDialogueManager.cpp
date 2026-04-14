#include "NarrDialogueManager.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Engine/Engine.h"

UNarrDialogueManager::UNarrDialogueManager()
{
    CurrentState = ENarr_DialogueState::Inactive;
    CurrentDialogueID = TEXT("");
    CurrentLineIndex = 0;
    CurrentNPCActor = nullptr;
    CurrentPlayerActor = nullptr;
}

void UNarrDialogueManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("NarrDialogueManager initialized"));
    
    // Initialize dialogue trees with sample data
    FNarr_DialogueTree SampleTree;
    SampleTree.DialogueID = TEXT("elder_greeting");
    SampleTree.NPCName = TEXT("Elder Shaman");
    SampleTree.TriggerType = ENarr_DialogueTrigger::Proximity;
    SampleTree.bRepeatable = true;
    SampleTree.MaxRepetitions = 3;
    SampleTree.CooldownTime = 30.0f;
    
    FNarr_DialogueLine GreetingLine;
    GreetingLine.SpeakerName = TEXT("Elder Shaman");
    GreetingLine.DialogueText = FText::FromString(TEXT("Young one, the ancestors speak through the wind and the stones. I have seen many seasons pass, and in each one, the spirits reveal new truths to those who listen with their hearts."));
    GreetingLine.Duration = 8.0f;
    GreetingLine.PlayerResponseOptions.Add(TEXT("Tell me about the ancestors."));
    GreetingLine.PlayerResponseOptions.Add(TEXT("What truths do the spirits reveal?"));
    GreetingLine.PlayerResponseOptions.Add(TEXT("I must go now."));
    GreetingLine.NextDialogueIDs.Add(TEXT("elder_ancestors"));
    GreetingLine.NextDialogueIDs.Add(TEXT("elder_spirits"));
    GreetingLine.NextDialogueIDs.Add(TEXT("elder_farewell"));
    
    SampleTree.DialogueLines.Add(GreetingLine);
    RegisterDialogueTree(SampleTree);
    
    // Start cooldown timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(CooldownTimerHandle, 
            FTimerDelegate::CreateLambda([this]() { UpdateCooldowns(1.0f); }), 
            1.0f, true);
    }
}

void UNarrDialogueManager::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(CooldownTimerHandle);
    }
    
    EndDialogue();
    DialogueTrees.Empty();
    DialogueUsageCounts.Empty();
    DialogueCooldowns.Empty();
    
    Super::Deinitialize();
}

bool UNarrDialogueManager::StartDialogue(const FString& DialogueID, AActor* NPCActor, AActor* PlayerActor)
{
    if (!CanStartDialogue(DialogueID) || !NPCActor || !PlayerActor)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot start dialogue: %s"), *DialogueID);
        return false;
    }
    
    if (CurrentState != ENarr_DialogueState::Inactive)
    {
        EndDialogue();
    }
    
    const FNarr_DialogueTree* DialogueTree = DialogueTrees.Find(DialogueID);
    if (!DialogueTree)
    {
        UE_LOG(LogTemp, Error, TEXT("Dialogue tree not found: %s"), *DialogueID);
        return false;
    }
    
    CurrentDialogueID = DialogueID;
    CurrentNPCActor = NPCActor;
    CurrentPlayerActor = PlayerActor;
    CurrentLineIndex = 0;
    CurrentState = ENarr_DialogueState::NPCSpeaking;
    
    // Update usage count
    int32* UsageCount = DialogueUsageCounts.Find(DialogueID);
    if (UsageCount)
    {
        (*UsageCount)++;
    }
    else
    {
        DialogueUsageCounts.Add(DialogueID, 1);
    }
    
    // Set cooldown
    DialogueCooldowns.Add(DialogueID, DialogueTree->CooldownTime);
    
    UE_LOG(LogTemp, Log, TEXT("Started dialogue: %s with NPC: %s"), *DialogueID, *DialogueTree->NPCName);
    
    OnDialogueStarted.Broadcast(DialogueID, DialogueTree->NPCName);
    ProcessCurrentLine();
    
    return true;
}

void UNarrDialogueManager::EndDialogue()
{
    if (CurrentState == ENarr_DialogueState::Inactive)
    {
        return;
    }
    
    FString EndingDialogueID = CurrentDialogueID;
    
    CurrentState = ENarr_DialogueState::Inactive;
    CurrentDialogueID = TEXT("");
    CurrentLineIndex = 0;
    CurrentNPCActor = nullptr;
    CurrentPlayerActor = nullptr;
    
    UE_LOG(LogTemp, Log, TEXT("Ended dialogue: %s"), *EndingDialogueID);
    OnDialogueEnded.Broadcast(EndingDialogueID);
}

void UNarrDialogueManager::SelectPlayerResponse(int32 ResponseIndex)
{
    if (CurrentState != ENarr_DialogueState::PlayerChoosing)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot select response - not in player choosing state"));
        return;
    }
    
    const FNarr_DialogueTree* DialogueTree = DialogueTrees.Find(CurrentDialogueID);
    if (!DialogueTree || CurrentLineIndex >= DialogueTree->DialogueLines.Num())
    {
        EndDialogue();
        return;
    }
    
    const FNarr_DialogueLine& CurrentLine = DialogueTree->DialogueLines[CurrentLineIndex];
    
    if (ResponseIndex < 0 || ResponseIndex >= CurrentLine.NextDialogueIDs.Num())
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid response index: %d"), ResponseIndex);
        EndDialogue();
        return;
    }
    
    // Handle response selection
    FString NextDialogueID = CurrentLine.NextDialogueIDs[ResponseIndex];
    
    if (NextDialogueID == TEXT("end") || NextDialogueID.IsEmpty())
    {
        EndDialogue();
        return;
    }
    
    // Check if it's a new dialogue tree or continue current one
    if (DialogueTrees.Contains(NextDialogueID))
    {
        // Start new dialogue
        StartDialogue(NextDialogueID, CurrentNPCActor.Get(), CurrentPlayerActor.Get());
    }
    else
    {
        // Continue current dialogue
        CurrentLineIndex++;
        AdvanceDialogue();
    }
}

void UNarrDialogueManager::AdvanceDialogue()
{
    if (CurrentState == ENarr_DialogueState::Inactive)
    {
        return;
    }
    
    const FNarr_DialogueTree* DialogueTree = DialogueTrees.Find(CurrentDialogueID);
    if (!DialogueTree)
    {
        EndDialogue();
        return;
    }
    
    if (CurrentLineIndex >= DialogueTree->DialogueLines.Num())
    {
        EndDialogue();
        return;
    }
    
    ProcessCurrentLine();
}

void UNarrDialogueManager::RegisterDialogueTree(const FNarr_DialogueTree& DialogueTree)
{
    if (DialogueTree.DialogueID.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot register dialogue tree with empty ID"));
        return;
    }
    
    DialogueTrees.Add(DialogueTree.DialogueID, DialogueTree);
    UE_LOG(LogTemp, Log, TEXT("Registered dialogue tree: %s for NPC: %s"), 
           *DialogueTree.DialogueID, *DialogueTree.NPCName);
}

bool UNarrDialogueManager::HasDialogue(const FString& DialogueID) const
{
    return DialogueTrees.Contains(DialogueID);
}

FNarr_DialogueTree UNarrDialogueManager::GetDialogueTree(const FString& DialogueID) const
{
    const FNarr_DialogueTree* Tree = DialogueTrees.Find(DialogueID);
    return Tree ? *Tree : FNarr_DialogueTree();
}

void UNarrDialogueManager::ProcessCurrentLine()
{
    const FNarr_DialogueTree* DialogueTree = DialogueTrees.Find(CurrentDialogueID);
    if (!DialogueTree || CurrentLineIndex >= DialogueTree->DialogueLines.Num())
    {
        EndDialogue();
        return;
    }
    
    const FNarr_DialogueLine& CurrentLine = DialogueTree->DialogueLines[CurrentLineIndex];
    
    UE_LOG(LogTemp, Log, TEXT("Processing dialogue line: %s - %s"), 
           *CurrentLine.SpeakerName, *CurrentLine.DialogueText.ToString());
    
    // Broadcast the dialogue line event
    OnDialogueLineSpoken.Broadcast(CurrentLine.SpeakerName, CurrentLine.DialogueText, CurrentLine.Duration);
    
    // Determine next state
    if (CurrentLine.PlayerResponseOptions.Num() > 0)
    {
        CurrentState = ENarr_DialogueState::PlayerChoosing;
    }
    else
    {
        CurrentState = ENarr_DialogueState::NPCSpeaking;
        
        // Auto-advance after duration if no player choices
        if (UWorld* World = GetWorld())
        {
            FTimerHandle AutoAdvanceTimer;
            World->GetTimerManager().SetTimer(AutoAdvanceTimer, 
                FTimerDelegate::CreateUObject(this, &UNarrDialogueManager::AdvanceDialogue),
                CurrentLine.Duration, false);
        }
    }
}

bool UNarrDialogueManager::CanStartDialogue(const FString& DialogueID) const
{
    const FNarr_DialogueTree* DialogueTree = DialogueTrees.Find(DialogueID);
    if (!DialogueTree)
    {
        return false;
    }
    
    // Check cooldown
    const float* CooldownTime = DialogueCooldowns.Find(DialogueID);
    if (CooldownTime && *CooldownTime > 0.0f)
    {
        return false;
    }
    
    // Check max repetitions
    if (DialogueTree->MaxRepetitions >= 0)
    {
        const int32* UsageCount = DialogueUsageCounts.Find(DialogueID);
        if (UsageCount && *UsageCount >= DialogueTree->MaxRepetitions)
        {
            return false;
        }
    }
    
    return true;
}

void UNarrDialogueManager::UpdateCooldowns(float DeltaTime)
{
    TArray<FString> KeysToRemove;
    
    for (auto& CooldownPair : DialogueCooldowns)
    {
        CooldownPair.Value -= DeltaTime;
        if (CooldownPair.Value <= 0.0f)
        {
            KeysToRemove.Add(CooldownPair.Key);
        }
    }
    
    for (const FString& Key : KeysToRemove)
    {
        DialogueCooldowns.Remove(Key);
    }
}