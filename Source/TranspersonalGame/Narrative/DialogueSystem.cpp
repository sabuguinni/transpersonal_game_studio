// DialogueSystem.cpp
// Narrative & Dialogue Agent #15 — Cycle PROD_CYCLE_AUTO_20260630_005
// Elder Kael NPC — "First Craft" quest arc dialogue tree
// Full implementation: greeting → resource gathering → crafting → completion

#include "DialogueSystem.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

// ============================================================
// ANarr_ElderKaelNPC — Constructor
// ============================================================
ANarr_ElderKaelNPC::ANarr_ElderKaelNPC()
{
    PrimaryActorTick.bCanEverTick = true;

    // Root: Interaction sphere
    InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
    RootComponent = InteractionSphere;
    InteractionSphere->SetSphereRadius(300.0f);
    InteractionSphere->SetCollisionProfileName(TEXT("Trigger"));

    // NPC mesh (placeholder cylinder — replaced by Character Artist)
    NPCMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("NPCMesh"));
    NPCMesh->SetupAttachment(RootComponent);
    NPCMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));

    // Defaults
    CurrentDialogueState = ENarr_DialogueState::Greeting;
    CurrentQuestStage = ENarr_QuestStage::NotStarted;
    InteractionRadius = 300.0f;
    bPlayerInRange = false;
    DialogueTimer = 0.0f;
    CurrentLineIndex = 0;
}

void ANarr_ElderKaelNPC::BeginPlay()
{
    Super::BeginPlay();

    // Bind overlap events
    InteractionSphere->OnComponentBeginOverlap.AddDynamic(this, &ANarr_ElderKaelNPC::OnPlayerEnterRange);
    InteractionSphere->OnComponentEndOverlap.AddDynamic(this, &ANarr_ElderKaelNPC::OnPlayerExitRange);

    // Build dialogue tree
    InitializeDialogueTree();

    // Build quest objectives
    InitializeObjectives();

    UE_LOG(LogTemp, Log, TEXT("[Narrative] Elder Kael NPC initialized — First Craft quest ready"));
}

void ANarr_ElderKaelNPC::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Auto-advance dialogue timer
    if (bPlayerInRange && DialogueLines.IsValidIndex(CurrentLineIndex))
    {
        DialogueTimer -= DeltaTime;
        if (DialogueTimer <= 0.0f)
        {
            AdvanceDialogue();
        }
    }
}

// ============================================================
// InitializeDialogueTree — Full Elder Kael dialogue lines
// ============================================================
void ANarr_ElderKaelNPC::InitializeDialogueTree()
{
    DialogueLines.Empty();

    // --- GREETING ---
    {
        FNarr_DialogueLine Line;
        Line.SpeakerName = TEXT("Elder Kael");
        Line.LineText = TEXT("You are new. I can tell by the way you look at the trees — like they are just trees. They are not just trees. Sit.");
        Line.TriggerState = ENarr_DialogueState::Greeting;
        Line.DisplayDuration = 6.0f;
        DialogueLines.Add(Line);
    }
    {
        FNarr_DialogueLine Line;
        Line.SpeakerName = TEXT("Elder Kael");
        Line.LineText = TEXT("My name is Kael. I have been here longer than anyone. That is not something to be proud of — it means I have watched everyone else die.");
        Line.TriggerState = ENarr_DialogueState::Greeting;
        Line.DisplayDuration = 7.0f;
        DialogueLines.Add(Line);
    }

    // --- QUEST INTRO ---
    {
        FNarr_DialogueLine Line;
        Line.SpeakerName = TEXT("Elder Kael");
        Line.LineText = TEXT("You need three things to survive the night. Shelter, fire, and water. The river is east, but raptors hunt that bank after dusk.");
        Line.TriggerState = ENarr_DialogueState::QuestIntro;
        Line.DisplayDuration = 7.0f;
        DialogueLines.Add(Line);
    }
    {
        FNarr_DialogueLine Line;
        Line.SpeakerName = TEXT("Elder Kael");
        Line.LineText = TEXT("Start with the axe. Two rocks, one stick. Knock them together at the right angle — you will know when you hear the crack. Bring me the axe when it is done.");
        Line.TriggerState = ENarr_DialogueState::QuestIntro;
        Line.DisplayDuration = 8.0f;
        DialogueLines.Add(Line);
    }

    // --- CRAFTING GUIDE ---
    {
        FNarr_DialogueLine Line;
        Line.SpeakerName = TEXT("Elder Kael");
        Line.LineText = TEXT("Craft the axe first. Then the spear. Then the shelter. In that order. Every time. Anyone who changes that order does not survive to argue about it.");
        Line.TriggerState = ENarr_DialogueState::CraftingGuide;
        Line.DisplayDuration = 8.0f;
        DialogueLines.Add(Line);
    }
    {
        FNarr_DialogueLine Line;
        Line.SpeakerName = TEXT("Elder Kael");
        Line.LineText = TEXT("The grey rocks near the stream — those are the ones you want. Flint. Harder than the brown ones. Takes longer to find but splits clean.");
        Line.TriggerState = ENarr_DialogueState::CraftingGuide;
        Line.DisplayDuration = 7.0f;
        DialogueLines.Add(Line);
    }
    {
        FNarr_DialogueLine Line;
        Line.SpeakerName = TEXT("Elder Kael");
        Line.LineText = TEXT("For fire: three dry sticks, stacked. Not wet ones — wet ones just smoke and draw attention. Smoke draws the big ones.");
        Line.TriggerState = ENarr_DialogueState::CraftingGuide;
        Line.DisplayDuration = 7.0f;
        DialogueLines.Add(Line);
    }

    // --- DANGER WARNING ---
    {
        FNarr_DialogueLine Line;
        Line.SpeakerName = TEXT("Elder Kael");
        Line.LineText = TEXT("The river bends east, past the grey stones. That is where they found the last scout — nothing left but his spear. Do not go there alone.");
        Line.TriggerState = ENarr_DialogueState::DangerWarning;
        Line.DisplayDuration = 8.0f;
        DialogueLines.Add(Line);
    }
    {
        FNarr_DialogueLine Line;
        Line.SpeakerName = TEXT("Elder Kael");
        Line.LineText = TEXT("You hear that? Low rumble, from the north ridge. That is the big one. The one that took Mara's brother. We do not hunt it. We avoid it. We survive it.");
        Line.TriggerState = ENarr_DialogueState::DangerWarning;
        Line.DisplayDuration = 8.0f;
        DialogueLines.Add(Line);
    }
    {
        FNarr_DialogueLine Line;
        Line.SpeakerName = TEXT("Elder Kael");
        Line.LineText = TEXT("The small ones — raptors — they are worse than the big one. The big one you can hear coming. Raptors you cannot. Stay in groups of three or more.");
        Line.TriggerState = ENarr_DialogueState::DangerWarning;
        Line.DisplayDuration = 8.0f;
        DialogueLines.Add(Line);
    }

    // --- QUEST COMPLETE ---
    {
        FNarr_DialogueLine Line;
        Line.SpeakerName = TEXT("Elder Kael");
        Line.LineText = TEXT("You made the axe. Good. Now you are not completely useless. The spear is next — two sticks, one flint head. Bind them with the sinew from the deer carcass south of camp.");
        Line.TriggerState = ENarr_DialogueState::QuestComplete;
        Line.DisplayDuration = 9.0f;
        DialogueLines.Add(Line);
    }
    {
        FNarr_DialogueLine Line;
        Line.SpeakerName = TEXT("Elder Kael");
        Line.LineText = TEXT("I have seen three winters here. The first winter, half our people died. The second, we learned. The third — we were ready. You will learn too. Start with fire. Always start with fire.");
        Line.TriggerState = ENarr_DialogueState::QuestComplete;
        Line.DisplayDuration = 10.0f;
        DialogueLines.Add(Line);
    }

    // --- IDLE ---
    {
        FNarr_DialogueLine Line;
        Line.SpeakerName = TEXT("Elder Kael");
        Line.LineText = TEXT("...");
        Line.TriggerState = ENarr_DialogueState::Idle;
        Line.DisplayDuration = 3.0f;
        DialogueLines.Add(Line);
    }

    UE_LOG(LogTemp, Log, TEXT("[Narrative] Dialogue tree initialized: %d lines for Elder Kael"), DialogueLines.Num());
}

// ============================================================
// InitializeObjectives — First Craft quest objectives
// ============================================================
void ANarr_ElderKaelNPC::InitializeObjectives()
{
    ActiveObjectives.Empty();

    // Objective 1: Collect rocks
    {
        FNarr_QuestObjective Obj;
        Obj.ObjectiveText = TEXT("Collect rocks (0/2)");
        Obj.bCompleted = false;
        Obj.RequiredCount = 2;
        Obj.CurrentCount = 0;
        ActiveObjectives.Add(Obj);
    }

    // Objective 2: Collect sticks
    {
        FNarr_QuestObjective Obj;
        Obj.ObjectiveText = TEXT("Collect sticks (0/1)");
        Obj.bCompleted = false;
        Obj.RequiredCount = 1;
        Obj.CurrentCount = 0;
        ActiveObjectives.Add(Obj);
    }

    // Objective 3: Craft stone axe
    {
        FNarr_QuestObjective Obj;
        Obj.ObjectiveText = TEXT("Craft a Stone Axe (press C near crafting station)");
        Obj.bCompleted = false;
        Obj.RequiredCount = 1;
        Obj.CurrentCount = 0;
        ActiveObjectives.Add(Obj);
    }

    // Objective 4: Return to Elder Kael
    {
        FNarr_QuestObjective Obj;
        Obj.ObjectiveText = TEXT("Return to Elder Kael");
        Obj.bCompleted = false;
        Obj.RequiredCount = 1;
        Obj.CurrentCount = 0;
        ActiveObjectives.Add(Obj);
    }

    UE_LOG(LogTemp, Log, TEXT("[Narrative] Quest objectives initialized: %d objectives"), ActiveObjectives.Num());
}

// ============================================================
// TriggerDialogue — Set state and reset to first matching line
// ============================================================
void ANarr_ElderKaelNPC::TriggerDialogue(ENarr_DialogueState State)
{
    CurrentDialogueState = State;
    CurrentLineIndex = 0;

    // Find first line matching this state
    for (int32 i = 0; i < DialogueLines.Num(); ++i)
    {
        if (DialogueLines[i].TriggerState == State)
        {
            CurrentLineIndex = i;
            break;
        }
    }

    if (DialogueLines.IsValidIndex(CurrentLineIndex))
    {
        const FNarr_DialogueLine& Line = DialogueLines[CurrentLineIndex];
        DialogueTimer = Line.DisplayDuration;

        // Display on screen
        if (GEngine)
        {
            FString Display = FString::Printf(TEXT("[%s]: %s"), *Line.SpeakerName, *Line.LineText);
            GEngine->AddOnScreenDebugMessage(-1, Line.DisplayDuration, FColor::Yellow, Display);
        }

        UE_LOG(LogTemp, Log, TEXT("[Narrative] Dialogue triggered — State: %d, Line: %s"),
            (int32)State, *Line.LineText);
    }
}

// ============================================================
// GetCurrentLine
// ============================================================
FNarr_DialogueLine ANarr_ElderKaelNPC::GetCurrentLine() const
{
    if (DialogueLines.IsValidIndex(CurrentLineIndex))
    {
        return DialogueLines[CurrentLineIndex];
    }
    return FNarr_DialogueLine();
}

// ============================================================
// AdvanceDialogue — Move to next line in current state
// ============================================================
void ANarr_ElderKaelNPC::AdvanceDialogue()
{
    int32 NextIndex = CurrentLineIndex + 1;

    // Check if next line is still in the same state
    if (DialogueLines.IsValidIndex(NextIndex) &&
        DialogueLines[NextIndex].TriggerState == CurrentDialogueState)
    {
        CurrentLineIndex = NextIndex;
        const FNarr_DialogueLine& Line = DialogueLines[CurrentLineIndex];
        DialogueTimer = Line.DisplayDuration;

        if (GEngine)
        {
            FString Display = FString::Printf(TEXT("[%s]: %s"), *Line.SpeakerName, *Line.LineText);
            GEngine->AddOnScreenDebugMessage(-1, Line.DisplayDuration, FColor::Yellow, Display);
        }
    }
    else
    {
        // End of this state's lines — transition to idle
        CurrentDialogueState = ENarr_DialogueState::Idle;
        DialogueTimer = 0.0f;
        UE_LOG(LogTemp, Log, TEXT("[Narrative] Dialogue state complete — returning to Idle"));
    }
}

// ============================================================
// OnQuestStageAdvanced — Respond to quest progression
// ============================================================
void ANarr_ElderKaelNPC::OnQuestStageAdvanced(ENarr_QuestStage NewStage)
{
    CurrentQuestStage = NewStage;

    switch (NewStage)
    {
    case ENarr_QuestStage::GatherResources:
        TriggerDialogue(ENarr_DialogueState::CraftingGuide);
        break;

    case ENarr_QuestStage::CraftAxe:
        TriggerDialogue(ENarr_DialogueState::CraftingGuide);
        break;

    case ENarr_QuestStage::ReturnToElder:
        TriggerDialogue(ENarr_DialogueState::QuestComplete);
        break;

    case ENarr_QuestStage::Completed:
        TriggerDialogue(ENarr_DialogueState::QuestComplete);
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Green,
                TEXT("QUEST COMPLETE: First Craft — Stone Axe crafted. New quests unlocked: Hunt, Build Shelter"));
        }
        break;

    default:
        break;
    }

    UE_LOG(LogTemp, Log, TEXT("[Narrative] Quest stage advanced to: %d"), (int32)NewStage);
}

// ============================================================
// UpdateObjective
// ============================================================
void ANarr_ElderKaelNPC::UpdateObjective(int32 ObjectiveIndex, int32 NewCount)
{
    if (!ActiveObjectives.IsValidIndex(ObjectiveIndex)) return;

    FNarr_QuestObjective& Obj = ActiveObjectives[ObjectiveIndex];
    Obj.CurrentCount = NewCount;

    if (Obj.CurrentCount >= Obj.RequiredCount)
    {
        Obj.bCompleted = true;
        UE_LOG(LogTemp, Log, TEXT("[Narrative] Objective %d complete: %s"), ObjectiveIndex, *Obj.ObjectiveText);
    }

    // Check if all objectives done
    if (AreAllObjectivesComplete())
    {
        OnQuestStageAdvanced(ENarr_QuestStage::Completed);
    }
}

// ============================================================
// AreAllObjectivesComplete
// ============================================================
bool ANarr_ElderKaelNPC::AreAllObjectivesComplete() const
{
    for (const FNarr_QuestObjective& Obj : ActiveObjectives)
    {
        if (!Obj.bCompleted) return false;
    }
    return ActiveObjectives.Num() > 0;
}

// ============================================================
// GetObjectiveSummary — Returns formatted string of objectives
// ============================================================
FString ANarr_ElderKaelNPC::GetObjectiveSummary() const
{
    FString Summary = TEXT("=== FIRST CRAFT QUEST ===\n");
    for (int32 i = 0; i < ActiveObjectives.Num(); ++i)
    {
        const FNarr_QuestObjective& Obj = ActiveObjectives[i];
        FString Status = Obj.bCompleted ? TEXT("[X]") : TEXT("[ ]");
        Summary += FString::Printf(TEXT("%s %s (%d/%d)\n"),
            *Status, *Obj.ObjectiveText, Obj.CurrentCount, Obj.RequiredCount);
    }
    return Summary;
}

// ============================================================
// Overlap callbacks
// ============================================================
void ANarr_ElderKaelNPC::OnPlayerEnterRange(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor || OtherActor == this) return;

    ACharacter* PlayerChar = Cast<ACharacter>(OtherActor);
    if (PlayerChar)
    {
        bPlayerInRange = true;

        // First interaction — trigger greeting then quest intro
        if (CurrentQuestStage == ENarr_QuestStage::NotStarted)
        {
            TriggerDialogue(ENarr_DialogueState::Greeting);
            CurrentQuestStage = ENarr_QuestStage::GatherResources;

            // Show quest objectives on screen
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Cyan, GetObjectiveSummary());
            }
        }
        else if (CurrentQuestStage == ENarr_QuestStage::ReturnToElder)
        {
            TriggerDialogue(ENarr_DialogueState::QuestComplete);
            OnQuestStageAdvanced(ENarr_QuestStage::Completed);
        }
        else
        {
            TriggerDialogue(ENarr_DialogueState::DangerWarning);
        }

        UE_LOG(LogTemp, Log, TEXT("[Narrative] Player entered Elder Kael range — dialogue triggered"));
    }
}

void ANarr_ElderKaelNPC::OnPlayerExitRange(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (!OtherActor || OtherActor == this) return;

    ACharacter* PlayerChar = Cast<ACharacter>(OtherActor);
    if (PlayerChar)
    {
        bPlayerInRange = false;
        DialogueTimer = 0.0f;
        UE_LOG(LogTemp, Log, TEXT("[Narrative] Player left Elder Kael range"));
    }
}
