#include "NarrativeManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"

UNarrativeManager::UNarrativeManager()
{
    CurrentDialogueID = TEXT("");
    CurrentDialogueLineIndex = 0;
    bDialogueActive = false;
    DefaultDialogueLineDelay = 3.0f;
    bAutoAdvanceDialogue = false;
}

void UNarrativeManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Initializing narrative system"));
    
    LoadDialogueData();
    LoadStoryData();
    InitializeCharacterRelationships();
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Loaded %d dialogue sequences, %d story beats"), 
           DialogueDatabase.Num(), StoryBeats.Num());
}

void UNarrativeManager::Deinitialize()
{
    if (bDialogueActive)
    {
        EndCurrentDialogue();
    }
    
    DialogueDatabase.Empty();
    StoryBeats.Empty();
    CharacterRelationships.Empty();
    
    Super::Deinitialize();
}

bool UNarrativeManager::StartDialogue(const FString& DialogueID, AActor* Speaker)
{
    if (bDialogueActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("NarrativeManager: Cannot start dialogue %s - dialogue already active"), *DialogueID);
        return false;
    }
    
    if (!DialogueDatabase.Contains(DialogueID))
    {
        UE_LOG(LogTemp, Error, TEXT("NarrativeManager: Dialogue %s not found in database"), *DialogueID);
        return false;
    }
    
    CurrentDialogueID = DialogueID;
    CurrentDialogueLineIndex = 0;
    bDialogueActive = true;
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Started dialogue %s"), *DialogueID);
    
    OnDialogueStarted.Broadcast(DialogueID);
    
    return true;
}

void UNarrativeManager::EndCurrentDialogue()
{
    if (!bDialogueActive)
    {
        return;
    }
    
    FString CompletedDialogueID = CurrentDialogueID;
    
    CurrentDialogueID = TEXT("");
    CurrentDialogueLineIndex = 0;
    bDialogueActive = false;
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Ended dialogue %s"), *CompletedDialogueID);
    
    OnDialogueCompleted.Broadcast(CompletedDialogueID);
}

bool UNarrativeManager::IsDialogueActive() const
{
    return bDialogueActive;
}

FNarr_DialogueSequence UNarrativeManager::GetDialogueSequence(const FString& DialogueID) const
{
    if (DialogueDatabase.Contains(DialogueID))
    {
        return DialogueDatabase[DialogueID];
    }
    
    UE_LOG(LogTemp, Warning, TEXT("NarrativeManager: Dialogue sequence %s not found"), *DialogueID);
    return FNarr_DialogueSequence();
}

void UNarrativeManager::CompleteStoryBeat(const FString& BeatID)
{
    if (!StoryBeats.Contains(BeatID))
    {
        UE_LOG(LogTemp, Error, TEXT("NarrativeManager: Story beat %s not found"), *BeatID);
        return;
    }
    
    FNarr_StoryBeat& Beat = StoryBeats[BeatID];
    if (!Beat.bIsCompleted)
    {
        Beat.bIsCompleted = true;
        UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Completed story beat %s"), *BeatID);
        OnStoryBeatCompleted.Broadcast(BeatID);
    }
}

bool UNarrativeManager::IsStoryBeatCompleted(const FString& BeatID) const
{
    if (StoryBeats.Contains(BeatID))
    {
        return StoryBeats[BeatID].bIsCompleted;
    }
    return false;
}

TArray<FNarr_StoryBeat> UNarrativeManager::GetAvailableStoryBeats() const
{
    TArray<FNarr_StoryBeat> AvailableBeats;
    
    for (const auto& BeatPair : StoryBeats)
    {
        const FNarr_StoryBeat& Beat = BeatPair.Value;
        
        if (Beat.bIsCompleted)
        {
            continue;
        }
        
        // Check if all required beats are completed
        bool bCanStart = true;
        for (const FString& RequiredBeatID : Beat.RequiredBeats)
        {
            if (!IsStoryBeatCompleted(RequiredBeatID))
            {
                bCanStart = false;
                break;
            }
        }
        
        if (bCanStart)
        {
            AvailableBeats.Add(Beat);
        }
    }
    
    return AvailableBeats;
}

void UNarrativeManager::ModifyCharacterRelationship(const FString& CharacterID, int32 RelationshipChange)
{
    if (!CharacterRelationships.Contains(CharacterID))
    {
        CharacterRelationships.Add(CharacterID, 0);
    }
    
    CharacterRelationships[CharacterID] += RelationshipChange;
    
    // Clamp relationship values between -100 and 100
    CharacterRelationships[CharacterID] = FMath::Clamp(CharacterRelationships[CharacterID], -100, 100);
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Character %s relationship changed by %d, now %d"), 
           *CharacterID, RelationshipChange, CharacterRelationships[CharacterID]);
}

int32 UNarrativeManager::GetCharacterRelationship(const FString& CharacterID) const
{
    if (CharacterRelationships.Contains(CharacterID))
    {
        return CharacterRelationships[CharacterID];
    }
    return 0;
}

void UNarrativeManager::LoadDialogueData()
{
    // Initialize with basic survival-focused dialogue sequences
    
    // Elder Hunter dialogue
    FNarr_DialogueSequence ElderHunterIntro;
    ElderHunterIntro.SequenceID = TEXT("elder_hunter_intro");
    ElderHunterIntro.bIsRepeatable = false;
    
    FNarr_DialogueLine Line1;
    Line1.SpeakerName = TEXT("Elder Hunter");
    Line1.DialogueText = FText::FromString(TEXT("You look lost, young one. This valley is dangerous for those who don't know its ways."));
    Line1.Duration = 4.0f;
    ElderHunterIntro.DialogueLines.Add(Line1);
    
    FNarr_DialogueLine Line2;
    Line2.SpeakerName = TEXT("Elder Hunter");
    Line2.DialogueText = FText::FromString(TEXT("The three-toed tracks by the river... Carnotaurus. It hunts at dawn and dusk."));
    Line2.Duration = 3.5f;
    ElderHunterIntro.DialogueLines.Add(Line2);
    
    DialogueDatabase.Add(TEXT("elder_hunter_intro"), ElderHunterIntro);
    
    // Scout Runner dialogue
    FNarr_DialogueSequence ScoutReport;
    ScoutReport.SequenceID = TEXT("scout_herd_report");
    ScoutReport.bIsRepeatable = true;
    
    FNarr_DialogueLine ScoutLine1;
    ScoutLine1.SpeakerName = TEXT("Scout Runner");
    ScoutLine1.DialogueText = FText::FromString(TEXT("The herds move toward the great lake! We must hurry if we want fresh meat."));
    ScoutLine1.Duration = 3.0f;
    ScoutReport.DialogueLines.Add(ScoutLine1);
    
    DialogueDatabase.Add(TEXT("scout_herd_report"), ScoutReport);
    
    // Tribe Leader dialogue
    FNarr_DialogueSequence LeaderHunt;
    LeaderHunt.SequenceID = TEXT("leader_hunt_preparation");
    LeaderHunt.bIsRepeatable = true;
    
    FNarr_DialogueLine LeaderLine1;
    LeaderLine1.SpeakerName = TEXT("Tribe Leader");
    LeaderLine1.DialogueText = FText::FromString(TEXT("Today we hunt the giant lizards. Stay together, watch the flanks."));
    LeaderLine1.Duration = 3.5f;
    LeaderHunt.DialogueLines.Add(LeaderLine1);
    
    FNarr_DialogueLine LeaderLine2;
    LeaderLine2.SpeakerName = TEXT("Tribe Leader");
    LeaderLine2.DialogueText = FText::FromString(TEXT("One mistake feeds us to the predators. Are you ready?"));
    LeaderLine2.Duration = 3.0f;
    LeaderHunt.DialogueLines.Add(LeaderLine2);
    
    DialogueDatabase.Add(TEXT("leader_hunt_preparation"), LeaderHunt);
}

void UNarrativeManager::LoadStoryData()
{
    // Initialize main story beats focused on survival progression
    
    FNarr_StoryBeat FirstSurvival;
    FirstSurvival.BeatID = TEXT("first_survival");
    FirstSurvival.BeatTitle = FText::FromString(TEXT("First Day"));
    FirstSurvival.BeatDescription = FText::FromString(TEXT("Survive your first day in the prehistoric valley"));
    FirstSurvival.bIsCompleted = false;
    StoryBeats.Add(TEXT("first_survival"), FirstSurvival);
    
    FNarr_StoryBeat MeetTribe;
    MeetTribe.BeatID = TEXT("meet_tribe");
    MeetTribe.BeatTitle = FText::FromString(TEXT("Contact"));
    MeetTribe.BeatDescription = FText::FromString(TEXT("Make contact with the local tribe"));
    MeetTribe.bIsCompleted = false;
    MeetTribe.RequiredBeats.Add(TEXT("first_survival"));
    StoryBeats.Add(TEXT("meet_tribe"), MeetTribe);
    
    FNarr_StoryBeat FirstHunt;
    FirstHunt.BeatID = TEXT("first_hunt");
    FirstHunt.BeatTitle = FText::FromString(TEXT("The Hunt"));
    FirstHunt.BeatDescription = FText::FromString(TEXT("Participate in your first dinosaur hunt"));
    FirstHunt.bIsCompleted = false;
    FirstHunt.RequiredBeats.Add(TEXT("meet_tribe"));
    StoryBeats.Add(TEXT("first_hunt"), FirstHunt);
    
    FNarr_StoryBeat ProveWorth;
    ProveWorth.BeatID = TEXT("prove_worth");
    ProveWorth.BeatTitle = FText::FromString(TEXT("Proving Ground"));
    ProveWorth.BeatDescription = FText::FromString(TEXT("Prove your worth to the tribe through skill and courage"));
    ProveWorth.bIsCompleted = false;
    ProveWorth.RequiredBeats.Add(TEXT("first_hunt"));
    StoryBeats.Add(TEXT("prove_worth"), ProveWorth);
}

void UNarrativeManager::InitializeCharacterRelationships()
{
    // Initialize neutral relationships with key NPCs
    CharacterRelationships.Add(TEXT("elder_hunter"), 0);
    CharacterRelationships.Add(TEXT("scout_runner"), 0);
    CharacterRelationships.Add(TEXT("tribe_leader"), 0);
    CharacterRelationships.Add(TEXT("wounded_warrior"), 0);
    CharacterRelationships.Add(TEXT("tribal_crafter"), 0);
    CharacterRelationships.Add(TEXT("cave_painter"), 0);
}