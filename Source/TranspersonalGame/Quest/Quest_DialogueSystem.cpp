#include "Quest_DialogueSystem.h"
#include "Engine/Engine.h"

UQuest_DialogueSystem::UQuest_DialogueSystem()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    CurrentSequenceID = TEXT("");
    CurrentLineIndex = 0;
    bIsDialogueActive = false;
    InteractionRange = 300.0f;
}

void UQuest_DialogueSystem::BeginPlay()
{
    Super::BeginPlay();
    InitializeDefaultDialogues();
}

bool UQuest_DialogueSystem::StartDialogue(const FString& SequenceID)
{
    if (DialogueSequences.Contains(SequenceID))
    {
        CurrentSequenceID = SequenceID;
        CurrentLineIndex = 0;
        bIsDialogueActive = true;
        
        UE_LOG(LogTemp, Log, TEXT("Started dialogue sequence: %s"), *SequenceID);
        return true;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Dialogue sequence not found: %s"), *SequenceID);
    return false;
}

void UQuest_DialogueSystem::NextDialogueLine()
{
    if (!bIsDialogueActive || !DialogueSequences.Contains(CurrentSequenceID))
    {
        return;
    }
    
    FQuest_DialogueSequence& CurrentSequence = DialogueSequences[CurrentSequenceID];
    
    if (CurrentLineIndex < CurrentSequence.DialogueLines.Num() - 1)
    {
        CurrentLineIndex++;
    }
    else
    {
        // End of sequence
        if (!CurrentSequence.NextSequenceID.IsEmpty())
        {
            StartDialogue(CurrentSequence.NextSequenceID);
        }
        else
        {
            EndDialogue();
        }
    }
}

void UQuest_DialogueSystem::EndDialogue()
{
    bIsDialogueActive = false;
    CurrentSequenceID = TEXT("");
    CurrentLineIndex = 0;
    
    UE_LOG(LogTemp, Log, TEXT("Dialogue ended"));
}

FQuest_DialogueLine UQuest_DialogueSystem::GetCurrentDialogueLine() const
{
    if (bIsDialogueActive && DialogueSequences.Contains(CurrentSequenceID))
    {
        const FQuest_DialogueSequence& CurrentSequence = DialogueSequences[CurrentSequenceID];
        if (CurrentLineIndex < CurrentSequence.DialogueLines.Num())
        {
            return CurrentSequence.DialogueLines[CurrentLineIndex];
        }
    }
    
    return FQuest_DialogueLine();
}

void UQuest_DialogueSystem::AddDialogueSequence(const FString& SequenceID, const FQuest_DialogueSequence& Sequence)
{
    DialogueSequences.Add(SequenceID, Sequence);
    UE_LOG(LogTemp, Log, TEXT("Added dialogue sequence: %s"), *SequenceID);
}

void UQuest_DialogueSystem::SetupHuntQuestDialogue()
{
    FQuest_DialogueSequence HuntQuestGive;
    HuntQuestGive.SequenceID = TEXT("hunt_quest_give");
    HuntQuestGive.bIsRepeatable = false;
    
    FQuest_DialogueLine Line1;
    Line1.SpeakerName = TEXT("Elder Korth");
    Line1.DialogueText = FText::FromString(TEXT("Attention survivor! A massive Tyrannosaurus Rex has been spotted near the eastern cliffs."));
    Line1.DialogueType = EQuest_DialogueType::QuestGive;
    Line1.DisplayDuration = 4.0f;
    
    FQuest_DialogueLine Line2;
    Line2.SpeakerName = TEXT("Elder Korth");
    Line2.DialogueText = FText::FromString(TEXT("This apex predator poses a significant threat to our settlement. Your mission: track and eliminate this beast."));
    Line2.DialogueType = EQuest_DialogueType::QuestGive;
    Line2.DisplayDuration = 4.5f;
    
    FQuest_DialogueLine Line3;
    Line3.SpeakerName = TEXT("Elder Korth");
    Line3.DialogueText = FText::FromString(TEXT("Gather your weapons, study its movement patterns, and strike when the moment is right. The safety of our people depends on your success."));
    Line3.DialogueType = EQuest_DialogueType::QuestGive;
    Line3.DisplayDuration = 5.0f;
    Line3.PlayerResponses.Add(TEXT("I'll hunt the beast down."));
    Line3.PlayerResponses.Add(TEXT("This sounds too dangerous."));
    
    HuntQuestGive.DialogueLines.Add(Line1);
    HuntQuestGive.DialogueLines.Add(Line2);
    HuntQuestGive.DialogueLines.Add(Line3);
    
    AddDialogueSequence(TEXT("hunt_quest_give"), HuntQuestGive);
    
    // Hunt quest completion dialogue
    FQuest_DialogueSequence HuntQuestComplete;
    HuntQuestComplete.SequenceID = TEXT("hunt_quest_complete");
    HuntQuestComplete.bIsRepeatable = false;
    
    FQuest_DialogueLine CompleteLine1;
    CompleteLine1.SpeakerName = TEXT("Elder Korth");
    CompleteLine1.DialogueText = FText::FromString(TEXT("Well done, hunter! The great beast has fallen and our camp is safe once more."));
    CompleteLine1.DialogueType = EQuest_DialogueType::QuestComplete;
    CompleteLine1.DisplayDuration = 3.5f;
    
    FQuest_DialogueLine CompleteLine2;
    CompleteLine2.SpeakerName = TEXT("Elder Korth");
    CompleteLine2.DialogueText = FText::FromString(TEXT("Your courage and skill have earned you the respect of all survivors. Take these crafted tools as your reward."));
    CompleteLine2.DialogueType = EQuest_DialogueType::QuestComplete;
    CompleteLine2.DisplayDuration = 4.0f;
    
    HuntQuestComplete.DialogueLines.Add(CompleteLine1);
    HuntQuestComplete.DialogueLines.Add(CompleteLine2);
    
    AddDialogueSequence(TEXT("hunt_quest_complete"), HuntQuestComplete);
}

void UQuest_DialogueSystem::SetupCollectQuestDialogue()
{
    FQuest_DialogueSequence CollectQuestGive;
    CollectQuestGive.SequenceID = TEXT("collect_quest_give");
    CollectQuestGive.bIsRepeatable = true;
    
    FQuest_DialogueLine Line1;
    Line1.SpeakerName = TEXT("Gatherer Naia");
    Line1.DialogueText = FText::FromString(TEXT("Our food stores are running dangerously low. We need fresh berries and roots to survive the coming days."));
    Line1.DialogueType = EQuest_DialogueType::QuestGive;
    Line1.DisplayDuration = 4.0f;
    
    FQuest_DialogueLine Line2;
    Line2.SpeakerName = TEXT("Gatherer Naia");
    Line2.DialogueText = FText::FromString(TEXT("Search the forest groves for purple berries and dig up the thick roots near the river. Bring back 10 of each."));
    Line2.DialogueType = EQuest_DialogueType::QuestGive;
    Line2.DisplayDuration = 4.5f;
    Line2.PlayerResponses.Add(TEXT("I'll gather what you need."));
    Line2.PlayerResponses.Add(TEXT("Can't someone else do this?"));
    
    CollectQuestGive.DialogueLines.Add(Line1);
    CollectQuestGive.DialogueLines.Add(Line2);
    
    AddDialogueSequence(TEXT("collect_quest_give"), CollectQuestGive);
}

void UQuest_DialogueSystem::SetupExploreQuestDialogue()
{
    FQuest_DialogueSequence ExploreQuestGive;
    ExploreQuestGive.SequenceID = TEXT("explore_quest_give");
    ExploreQuestGive.bIsRepeatable = false;
    
    FQuest_DialogueLine Line1;
    Line1.SpeakerName = TEXT("Scout Theron");
    Line1.DialogueText = FText::FromString(TEXT("Strange smoke has been seen rising from the northern valleys. We need to know what lies beyond those hills."));
    Line1.DialogueType = EQuest_DialogueType::QuestGive;
    Line1.DisplayDuration = 4.0f;
    
    FQuest_DialogueLine Line2;
    Line2.SpeakerName = TEXT("Scout Theron");
    Line2.DialogueText = FText::FromString(TEXT("Venture north, map the territory, and report back what you discover. Be careful - unknown lands hold unknown dangers."));
    Line2.DialogueType = EQuest_DialogueType::QuestGive;
    Line2.DisplayDuration = 4.5f;
    Line2.PlayerResponses.Add(TEXT("I'll scout the northern lands."));
    Line2.PlayerResponses.Add(TEXT("Send someone more experienced."));
    
    ExploreQuestGive.DialogueLines.Add(Line1);
    ExploreQuestGive.DialogueLines.Add(Line2);
    
    AddDialogueSequence(TEXT("explore_quest_give"), ExploreQuestGive);
}

void UQuest_DialogueSystem::InitializeDefaultDialogues()
{
    SetupHuntQuestDialogue();
    SetupCollectQuestDialogue();
    SetupExploreQuestDialogue();
    
    // Generic greeting dialogue
    FQuest_DialogueSequence Greeting;
    Greeting.SequenceID = TEXT("generic_greeting");
    Greeting.bIsRepeatable = true;
    
    FQuest_DialogueLine GreetLine;
    GreetLine.SpeakerName = TEXT("Survivor");
    GreetLine.DialogueText = FText::FromString(TEXT("Stay alert out there. This world doesn't forgive the careless."));
    GreetLine.DialogueType = EQuest_DialogueType::Greeting;
    GreetLine.DisplayDuration = 3.0f;
    
    Greeting.DialogueLines.Add(GreetLine);
    AddDialogueSequence(TEXT("generic_greeting"), Greeting);
    
    UE_LOG(LogTemp, Log, TEXT("Quest dialogue system initialized with %d sequences"), DialogueSequences.Num());
}