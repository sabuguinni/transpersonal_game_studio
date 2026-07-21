#include "NarrativeDialogueManager.h"
#include "Components/SphereComponent.h"
#include "Components/AudioComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "TimerManager.h"

// ============================================================
// Constructor
// ============================================================

ANarrativeDialogueManager::ANarrativeDialogueManager()
{
	PrimaryActorTick.bCanEverTick = true;

	TriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphere"));
	TriggerSphere->SetSphereRadius(500.0f);
	TriggerSphere->SetCollisionProfileName(TEXT("Trigger"));
	RootComponent = TriggerSphere;

	DialogueAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("DialogueAudio"));
	DialogueAudio->SetupAttachment(RootComponent);
	DialogueAudio->bAutoActivate = false;

	ProximityRadius = 500.0f;
	AssignedSpeaker = ENarr_SpeakerRole::Narrator;
	CurrentState = ENarr_DialogueState::Idle;
	ActiveLineID = TEXT("");
	ActiveTreeID = TEXT("");
	LineTimer = 0.0f;
	CooldownTimer = 0.0f;
	bOnCooldown = false;
}

// ============================================================
// BeginPlay
// ============================================================

void ANarrativeDialogueManager::BeginPlay()
{
	Super::BeginPlay();

	TriggerSphere->SetSphereRadius(ProximityRadius);

	TriggerSphere->OnComponentBeginOverlap.AddDynamic(
		this, &ANarrativeDialogueManager::OnProximityOverlap);

	BuildDefaultDialogueTrees();
}

// ============================================================
// Tick
// ============================================================

void ANarrativeDialogueManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Cooldown countdown
	if (bOnCooldown)
	{
		CooldownTimer -= DeltaTime;
		if (CooldownTimer <= 0.0f)
		{
			bOnCooldown = false;
			CooldownTimer = 0.0f;
			CurrentState = ENarr_DialogueState::Idle;
		}
		return;
	}

	// Auto-advance playing line
	if (CurrentState == ENarr_DialogueState::Playing)
	{
		LineTimer -= DeltaTime;
		if (LineTimer <= 0.0f)
		{
			AdvanceDialogue();
		}
	}
}

// ============================================================
// Proximity Overlap
// ============================================================

void ANarrativeDialogueManager::OnProximityOverlap(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (!OtherActor) return;
	if (CurrentState != ENarr_DialogueState::Idle) return;
	if (bOnCooldown) return;

	// Find a tree with ProximityEnter trigger
	for (const FNarr_DialogueTree& Tree : DialogueTrees)
	{
		if (Tree.TriggerType == ENarr_DialogueTriggerType::ProximityEnter)
		{
			TriggerDialogueTree(Tree.TreeID);
			return;
		}
	}
}

// ============================================================
// TriggerDialogueTree
// ============================================================

void ANarrativeDialogueManager::TriggerDialogueTree(const FString& TreeID)
{
	const FNarr_DialogueTree* Tree = FindTree(TreeID);
	if (!Tree) return;

	ActiveTreeID = TreeID;
	ActiveLineID = Tree->StartLineID;
	CurrentState = ENarr_DialogueState::Playing;

	FNarr_DialogueLine* Line = FindLine(ActiveLineID);
	if (Line)
	{
		PlayLine(*Line);
	}
}

// ============================================================
// AdvanceDialogue
// ============================================================

void ANarrativeDialogueManager::AdvanceDialogue()
{
	FNarr_DialogueLine* CurrentLine = FindLine(ActiveLineID);
	if (!CurrentLine)
	{
		EndDialogue();
		return;
	}

	// Check for choices on current line
	const FNarr_DialogueTree* Tree = FindTree(ActiveTreeID);
	if (Tree && Tree->Choices.Num() > 0 && CurrentLine->NextLineID.IsEmpty())
	{
		CurrentState = ENarr_DialogueState::WaitingForChoice;
		return;
	}

	// Auto-advance to next line
	if (!CurrentLine->NextLineID.IsEmpty())
	{
		ActiveLineID = CurrentLine->NextLineID;
		FNarr_DialogueLine* NextLine = FindLine(ActiveLineID);
		if (NextLine)
		{
			PlayLine(*NextLine);
		}
		else
		{
			EndDialogue();
		}
	}
	else
	{
		EndDialogue();
	}
}

// ============================================================
// EndDialogue
// ============================================================

void ANarrativeDialogueManager::EndDialogue()
{
	const FNarr_DialogueTree* Tree = FindTree(ActiveTreeID);
	float Cooldown = Tree ? Tree->CooldownSeconds : 60.0f;

	CurrentState = ENarr_DialogueState::Cooldown;
	bOnCooldown = true;
	CooldownTimer = Cooldown;
	ActiveLineID = TEXT("");
}

// ============================================================
// IsDialoguePlaying
// ============================================================

bool ANarrativeDialogueManager::IsDialoguePlaying() const
{
	return CurrentState == ENarr_DialogueState::Playing
		|| CurrentState == ENarr_DialogueState::WaitingForChoice;
}

// ============================================================
// GetCurrentLine
// ============================================================

FNarr_DialogueLine ANarrativeDialogueManager::GetCurrentLine() const
{
	for (const FNarr_DialogueTree& Tree : DialogueTrees)
	{
		for (const FNarr_DialogueLine& Line : Tree.Lines)
		{
			if (Line.LineID == ActiveLineID)
			{
				return Line;
			}
		}
	}
	return FNarr_DialogueLine();
}

// ============================================================
// GetCurrentChoices
// ============================================================

TArray<FNarr_DialogueChoice> ANarrativeDialogueManager::GetCurrentChoices() const
{
	const FNarr_DialogueTree* Tree = FindTree(ActiveTreeID);
	if (Tree)
	{
		return Tree->Choices;
	}
	return TArray<FNarr_DialogueChoice>();
}

// ============================================================
// SelectChoice
// ============================================================

void ANarrativeDialogueManager::SelectChoice(int32 ChoiceIndex)
{
	if (CurrentState != ENarr_DialogueState::WaitingForChoice) return;

	const FNarr_DialogueTree* Tree = FindTree(ActiveTreeID);
	if (!Tree) return;
	if (!Tree->Choices.IsValidIndex(ChoiceIndex)) return;

	const FNarr_DialogueChoice& Choice = Tree->Choices[ChoiceIndex];

	if (Choice.bEndsDialogue || Choice.NextLineID.IsEmpty())
	{
		EndDialogue();
		return;
	}

	ActiveLineID = Choice.NextLineID;
	FNarr_DialogueLine* NextLine = FindLine(ActiveLineID);
	if (NextLine)
	{
		CurrentState = ENarr_DialogueState::Playing;
		PlayLine(*NextLine);
	}
	else
	{
		EndDialogue();
	}
}

// ============================================================
// RegisterDialogueTree
// ============================================================

void ANarrativeDialogueManager::RegisterDialogueTree(const FNarr_DialogueTree& NewTree)
{
	// Replace existing tree with same ID or add new
	for (int32 i = 0; i < DialogueTrees.Num(); ++i)
	{
		if (DialogueTrees[i].TreeID == NewTree.TreeID)
		{
			DialogueTrees[i] = NewTree;
			return;
		}
	}
	DialogueTrees.Add(NewTree);
}

// ============================================================
// BuildDefaultDialogueTrees — Tribal Elder + Herd Tracker trees
// ============================================================

void ANarrativeDialogueManager::BuildDefaultDialogueTrees()
{
	// ---- Tribal Elder: Raptor Warning ----
	{
		FNarr_DialogueTree ElderTree;
		ElderTree.TreeID = TEXT("ELDER_RAPTOR_WARNING");
		ElderTree.TriggerType = ENarr_DialogueTriggerType::ProximityEnter;
		ElderTree.CooldownSeconds = 120.0f;
		ElderTree.StartLineID = TEXT("ELDER_01");

		FNarr_DialogueLine Line1;
		Line1.LineID = TEXT("ELDER_01");
		Line1.Speaker = ENarr_SpeakerRole::TribalElder;
		Line1.LineText = TEXT("Listen carefully. The raptors hunt in threes. One drives the prey, two flank from the sides.");
		Line1.Duration = 5.0f;
		Line1.NextLineID = TEXT("ELDER_02");

		FNarr_DialogueLine Line2;
		Line2.LineID = TEXT("ELDER_02");
		Line2.Speaker = ENarr_SpeakerRole::TribalElder;
		Line2.LineText = TEXT("If you hear clicking from the left, run right. Never in a straight line. The tribe that forgets this does not survive the season.");
		Line2.Duration = 6.0f;
		Line2.NextLineID = TEXT("");

		ElderTree.Lines.Add(Line1);
		ElderTree.Lines.Add(Line2);

		FNarr_DialogueChoice Choice1;
		Choice1.ChoiceText = TEXT("I understand. I will be careful.");
		Choice1.bEndsDialogue = true;

		FNarr_DialogueChoice Choice2;
		Choice2.ChoiceText = TEXT("What if there are more than three?");
		Choice2.NextLineID = TEXT("ELDER_03");
		Choice2.bEndsDialogue = false;

		FNarr_DialogueLine Line3;
		Line3.LineID = TEXT("ELDER_03");
		Line3.Speaker = ENarr_SpeakerRole::TribalElder;
		Line3.LineText = TEXT("Then you climb. They cannot follow you into the trees. Remember that.");
		Line3.Duration = 4.0f;
		Line3.NextLineID = TEXT("");

		ElderTree.Lines.Add(Line3);
		ElderTree.Choices.Add(Choice1);
		ElderTree.Choices.Add(Choice2);

		RegisterDialogueTree(ElderTree);
	}

	// ---- Herd Tracker: Brachiosaurus Observation ----
	{
		FNarr_DialogueTree TrackerTree;
		TrackerTree.TreeID = TEXT("TRACKER_HERD_OBSERVATION");
		TrackerTree.TriggerType = ENarr_DialogueTriggerType::DinosaurSighted;
		TrackerTree.CooldownSeconds = 90.0f;
		TrackerTree.StartLineID = TEXT("TRACKER_01");

		FNarr_DialogueLine T1;
		T1.LineID = TEXT("TRACKER_01");
		T1.Speaker = ENarr_SpeakerRole::HerdTracker;
		T1.LineText = TEXT("We have tracked the herd for three days. The big ones — the long-necks — they move slow but they remember.");
		T1.Duration = 5.5f;
		T1.NextLineID = TEXT("TRACKER_02");

		FNarr_DialogueLine T2;
		T2.LineID = TEXT("TRACKER_02");
		T2.Speaker = ENarr_SpeakerRole::HerdTracker;
		T2.LineText = TEXT("Scare them once and they will not return to this valley for a full moon cycle. Do not hunt them. Watch them.");
		T2.Duration = 5.0f;
		T2.NextLineID = TEXT("");

		TrackerTree.Lines.Add(T1);
		TrackerTree.Lines.Add(T2);

		RegisterDialogueTree(TrackerTree);
	}

	// ---- Scout: Flood Warning ----
	{
		FNarr_DialogueTree ScoutTree;
		ScoutTree.TreeID = TEXT("SCOUT_FLOOD_WARNING");
		ScoutTree.TriggerType = ENarr_DialogueTriggerType::TimeOfDay;
		ScoutTree.CooldownSeconds = 300.0f;
		ScoutTree.StartLineID = TEXT("SCOUT_01");

		FNarr_DialogueLine S1;
		S1.LineID = TEXT("SCOUT_01");
		S1.Speaker = ENarr_SpeakerRole::Scout;
		S1.LineText = TEXT("The cave paintings do not lie. This valley has flooded seven times. The signs are here again.");
		S1.Duration = 5.0f;
		S1.NextLineID = TEXT("SCOUT_02");

		FNarr_DialogueLine S2;
		S2.LineID = TEXT("SCOUT_02");
		S2.Speaker = ENarr_SpeakerRole::Scout;
		S2.LineText = TEXT("The river bends, the birds fly south early, the ground lizards burrow deep. We have maybe four days. Move the camp now.");
		S2.Duration = 5.5f;
		S2.NextLineID = TEXT("");

		ScoutTree.Lines.Add(S1);
		ScoutTree.Lines.Add(S2);

		RegisterDialogueTree(ScoutTree);
	}
}

// ============================================================
// Private helpers
// ============================================================

FNarr_DialogueLine* ANarrativeDialogueManager::FindLine(const FString& LineID)
{
	for (FNarr_DialogueTree& Tree : DialogueTrees)
	{
		for (FNarr_DialogueLine& Line : Tree.Lines)
		{
			if (Line.LineID == LineID)
			{
				return &Line;
			}
		}
	}
	return nullptr;
}

const FNarr_DialogueTree* ANarrativeDialogueManager::FindTree(const FString& TreeID) const
{
	for (const FNarr_DialogueTree& Tree : DialogueTrees)
	{
		if (Tree.TreeID == TreeID)
		{
			return &Tree;
		}
	}
	return nullptr;
}

void ANarrativeDialogueManager::PlayLine(const FNarr_DialogueLine& Line)
{
	LineTimer = Line.Duration;
	UE_LOG(LogTemp, Log, TEXT("[Narrative] Playing line [%s] — %s: %s"),
		*Line.LineID,
		*UEnum::GetValueAsString(Line.Speaker),
		*Line.LineText);
}
