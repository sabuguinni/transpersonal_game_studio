// DialogueSystem.cpp
// Agent #15 — Narrative & Dialogue Agent
// Full implementation of dialogue trigger and manager systems

#include "DialogueSystem.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"

// ============================================================
// ANarr_DialogueTrigger — Implementation
// ============================================================

ANarr_DialogueTrigger::ANarr_DialogueTrigger()
{
    PrimaryActorTick.bCanEverTick = false;

    TriggerVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerVolume"));
    TriggerVolume->SetBoxExtent(FVector(150.0f, 150.0f, 100.0f));
    TriggerVolume->SetCollisionProfileName(TEXT("Trigger"));
    RootComponent = TriggerVolume;

    // Default dialogue tree with 3 lines for TribalElder
    DialogueTree.DialogueID = TEXT("DIALOGUE_ELDER_DEFAULT");
    DialogueTree.NPCRole = ENarr_NPCRole::TribalElder;
    DialogueTree.State = ENarr_DialogueState::Idle;

    FNarr_DialogueLine Line1;
    Line1.SpeakerName = TEXT("Tribal Elder");
    Line1.LineText = TEXT("Stranger. You come to our fire. The great lizards grow bold — three nights ago they took two of ours.");
    Line1.FearDelta = 5.0f;
    DialogueTree.Lines.Add(Line1);

    FNarr_DialogueLine Line2;
    Line2.SpeakerName = TEXT("Tribal Elder");
    Line2.LineText = TEXT("If you want to stay, you must prove you can survive. Go east. Find what the hunters left behind.");
    Line2.FearDelta = 0.0f;
    DialogueTree.Lines.Add(Line2);

    FNarr_DialogueLine Line3;
    Line3.SpeakerName = TEXT("Tribal Elder");
    Line3.LineText = TEXT("Do not go after dark. The big one — the one with the small arms — it hunts by sound.");
    Line3.FearDelta = 10.0f;
    DialogueTree.Lines.Add(Line3);

    DialogueTree.UnlocksQuestID = TEXT("QUEST_FIND_HUNTERS_CACHE");
}

void ANarr_DialogueTrigger::BeginPlay()
{
    Super::BeginPlay();

    if (TriggerVolume)
    {
        TriggerVolume->OnComponentBeginOverlap.AddDynamic(
            this, &ANarr_DialogueTrigger::OnTriggerBeginOverlap);
    }
}

void ANarr_DialogueTrigger::OnTriggerBeginOverlap(
    UPrimitiveComponent* OverlappedComp,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    // Only trigger for player characters
    if (!OtherActor || !OtherActor->IsA(ACharacter::StaticClass()))
    {
        return;
    }

    if (bFireOnce && bHasFired)
    {
        return;
    }

    if (CurrentState == ENarr_DialogueState::Locked)
    {
        return;
    }

    StartDialogue();
}

void ANarr_DialogueTrigger::StartDialogue()
{
    if (DialogueTree.Lines.Num() == 0)
    {
        return;
    }

    CurrentLineIndex = 0;
    CurrentState = ENarr_DialogueState::Active;
    DialogueTree.State = ENarr_DialogueState::Active;

    UE_LOG(LogTemp, Log, TEXT("[Narrative] Dialogue started: %s | Speaker: %s | Line: %s"),
        *DialogueTree.DialogueID,
        *DialogueTree.Lines[0].SpeakerName,
        *DialogueTree.Lines[0].LineText);
}

void ANarr_DialogueTrigger::AdvanceLine()
{
    if (CurrentState != ENarr_DialogueState::Active)
    {
        return;
    }

    CurrentLineIndex++;

    if (CurrentLineIndex >= DialogueTree.Lines.Num())
    {
        EndDialogue();
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("[Narrative] Line %d: %s — %s"),
        CurrentLineIndex,
        *DialogueTree.Lines[CurrentLineIndex].SpeakerName,
        *DialogueTree.Lines[CurrentLineIndex].LineText);
}

void ANarr_DialogueTrigger::EndDialogue()
{
    CurrentState = ENarr_DialogueState::Completed;
    DialogueTree.State = ENarr_DialogueState::Completed;
    bHasFired = true;

    UE_LOG(LogTemp, Log, TEXT("[Narrative] Dialogue completed: %s | Unlocks quest: %s"),
        *DialogueTree.DialogueID,
        *DialogueTree.UnlocksQuestID);

    // If not fire-once, reset after completion
    if (!bFireOnce)
    {
        ResetDialogue();
    }
}

void ANarr_DialogueTrigger::ResetDialogue()
{
    CurrentLineIndex = 0;
    CurrentState = ENarr_DialogueState::Idle;
    DialogueTree.State = ENarr_DialogueState::Idle;
    bHasFired = false;
}

FString ANarr_DialogueTrigger::GetCurrentLineText() const
{
    if (CurrentState != ENarr_DialogueState::Active)
    {
        return FString();
    }

    if (!DialogueTree.Lines.IsValidIndex(CurrentLineIndex))
    {
        return FString();
    }

    return DialogueTree.Lines[CurrentLineIndex].LineText;
}

FString ANarr_DialogueTrigger::GetCurrentSpeakerName() const
{
    if (CurrentState != ENarr_DialogueState::Active)
    {
        return FString();
    }

    if (!DialogueTree.Lines.IsValidIndex(CurrentLineIndex))
    {
        return FString();
    }

    return DialogueTree.Lines[CurrentLineIndex].SpeakerName;
}

// ============================================================
// UNarr_DialogueManager — Implementation
// ============================================================

UNarr_DialogueManager::UNarr_DialogueManager()
{
    PrimaryComponentTick.bCanEverTick = false;
    bAutoActivate = true;
}

void UNarr_DialogueManager::InitializeComponent()
{
    Super::InitializeComponent();

    // Pre-populate with default dialogue trees for the main story beats
    FNarr_DialogueTree ElderTree;
    ElderTree.DialogueID = TEXT("DIALOGUE_ELDER_DEFAULT");
    ElderTree.NPCRole = ENarr_NPCRole::TribalElder;
    ElderTree.State = ENarr_DialogueState::Idle;
    ElderTree.UnlocksQuestID = TEXT("QUEST_FIND_HUNTERS_CACHE");

    FNarr_DialogueLine E1;
    E1.SpeakerName = TEXT("Tribal Elder");
    E1.LineText = TEXT("Three suns have passed since our hunters returned empty-handed. The great beasts move south now.");
    E1.FearDelta = 5.0f;
    ElderTree.Lines.Add(E1);

    FNarr_DialogueLine E2;
    E2.SpeakerName = TEXT("Tribal Elder");
    E2.LineText = TEXT("We cannot stay here much longer. The valley is no longer safe.");
    E2.FearDelta = 8.0f;
    ElderTree.Lines.Add(E2);

    AllDialogueTrees.Add(ElderTree);

    FNarr_DialogueTree ScoutTree;
    ScoutTree.DialogueID = TEXT("DIALOGUE_SCOUT_PATROL");
    ScoutTree.NPCRole = ENarr_NPCRole::Scout;
    ScoutTree.State = ENarr_DialogueState::Idle;
    ScoutTree.UnlocksQuestID = TEXT("QUEST_SCOUT_VALLEY");

    FNarr_DialogueLine S1;
    S1.SpeakerName = TEXT("Scout");
    S1.LineText = TEXT("I tracked the pack to the river bend. Four of them — maybe five. They move fast.");
    S1.FearDelta = 12.0f;
    ScoutTree.Lines.Add(S1);

    FNarr_DialogueLine S2;
    S2.SpeakerName = TEXT("Scout");
    S2.LineText = TEXT("Do not go near the water at dusk. That is when they drink — and when they hunt.");
    S2.FearDelta = 15.0f;
    ScoutTree.Lines.Add(S2);

    AllDialogueTrees.Add(ScoutTree);

    FNarr_DialogueTree HunterTree;
    HunterTree.DialogueID = TEXT("DIALOGUE_HUNTER_RETURN");
    HunterTree.NPCRole = ENarr_NPCRole::Hunter;
    HunterTree.State = ENarr_DialogueState::Idle;
    HunterTree.UnlocksQuestID = TEXT("QUEST_HUNT_SMALL_PREY");

    FNarr_DialogueLine H1;
    H1.SpeakerName = TEXT("Hunter");
    H1.LineText = TEXT("The spear is not enough against the big ones. You need fire — or distance.");
    H1.FearDelta = 3.0f;
    HunterTree.Lines.Add(H1);

    FNarr_DialogueLine H2;
    H2.SpeakerName = TEXT("Hunter");
    H2.LineText = TEXT("I brought back what I could. The rest... the rest I had to leave behind.");
    H2.FearDelta = 5.0f;
    HunterTree.Lines.Add(H2);

    AllDialogueTrees.Add(HunterTree);
}

void UNarr_DialogueManager::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogTemp, Log, TEXT("[Narrative] DialogueManager initialized with %d dialogue trees"),
        AllDialogueTrees.Num());
}

void UNarr_DialogueManager::RegisterDialogue(const FNarr_DialogueTree& Tree)
{
    // Check for duplicate ID
    for (const FNarr_DialogueTree& Existing : AllDialogueTrees)
    {
        if (Existing.DialogueID == Tree.DialogueID)
        {
            UE_LOG(LogTemp, Warning, TEXT("[Narrative] Dialogue ID already registered: %s"), *Tree.DialogueID);
            return;
        }
    }

    AllDialogueTrees.Add(Tree);
    UE_LOG(LogTemp, Log, TEXT("[Narrative] Registered dialogue: %s"), *Tree.DialogueID);
}

void UNarr_DialogueManager::CompleteDialogue(const FString& DialogueID)
{
    for (FNarr_DialogueTree& Tree : AllDialogueTrees)
    {
        if (Tree.DialogueID == DialogueID)
        {
            Tree.State = ENarr_DialogueState::Completed;
            UE_LOG(LogTemp, Log, TEXT("[Narrative] Dialogue completed: %s | Unlocks: %s"),
                *DialogueID, *Tree.UnlocksQuestID);
            return;
        }
    }
}

bool UNarr_DialogueManager::IsDialogueAvailable(const FString& DialogueID) const
{
    for (const FNarr_DialogueTree& Tree : AllDialogueTrees)
    {
        if (Tree.DialogueID == DialogueID)
        {
            return Tree.State == ENarr_DialogueState::Idle;
        }
    }
    return false;
}

bool UNarr_DialogueManager::GetDialogueByID(const FString& DialogueID, FNarr_DialogueTree& OutTree) const
{
    for (const FNarr_DialogueTree& Tree : AllDialogueTrees)
    {
        if (Tree.DialogueID == DialogueID)
        {
            OutTree = Tree;
            return true;
        }
    }
    return false;
}
