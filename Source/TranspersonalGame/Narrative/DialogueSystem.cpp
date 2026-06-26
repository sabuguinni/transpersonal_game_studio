// DialogueSystem.cpp
// Agent #15 — Narrative & Dialogue Agent
// Prehistoric survival game — NPC dialogue component implementation

#include "DialogueSystem.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"

UNarr_DialogueSystem::UNarr_DialogueSystem()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.1f;

	ProximityTriggerRadius = 400.0f;
	SpeakerDisplayName = TEXT("NPC");
	bIsSpeaking = false;
	ActiveSequenceIndex = -1;
	ActiveLineIndex = -1;
	TimeSinceLastProximityCheck = 0.0f;
}

void UNarr_DialogueSystem::BeginPlay()
{
	Super::BeginPlay();
}

void UNarr_DialogueSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Proximity check at reduced frequency for performance
	TimeSinceLastProximityCheck += DeltaTime;
	if (TimeSinceLastProximityCheck >= ProximityCheckInterval)
	{
		TimeSinceLastProximityCheck = 0.0f;

		if (!bIsSpeaking && IsPlayerInRange())
		{
			TriggerDialogueByType(ENarr_DialogueTriggerType::Proximity);
		}
	}
}

bool UNarr_DialogueSystem::TriggerDialogueSequence(const FString& SequenceID)
{
	if (bIsSpeaking)
	{
		return false;
	}

	int32 FoundIndex = FindSequenceIndexByID(SequenceID);
	if (FoundIndex == -1)
	{
		return false;
	}

	FNarr_DialogueSequence& Seq = DialogueSequences[FoundIndex];

	// Respect play-once flag
	if (Seq.bPlayOnce && Seq.bHasPlayed)
	{
		return false;
	}

	if (Seq.Lines.Num() == 0)
	{
		return false;
	}

	ActiveSequenceIndex = FoundIndex;
	ActiveLineIndex = 0;
	bIsSpeaking = true;

	PlayCurrentLine();
	return true;
}

void UNarr_DialogueSystem::TriggerDialogueByType(ENarr_DialogueTriggerType TriggerType)
{
	if (bIsSpeaking)
	{
		return;
	}

	// Find first unplayed sequence matching this trigger type
	for (int32 i = 0; i < DialogueSequences.Num(); ++i)
	{
		FNarr_DialogueSequence& Seq = DialogueSequences[i];

		if (Seq.bPlayOnce && Seq.bHasPlayed)
		{
			continue;
		}

		if (Seq.Lines.Num() == 0)
		{
			continue;
		}

		// Check if any line in sequence matches trigger type
		bool bMatchFound = false;
		for (const FNarr_DialogueLine& Line : Seq.Lines)
		{
			if (Line.TriggerType == TriggerType)
			{
				bMatchFound = true;
				break;
			}
		}

		if (bMatchFound)
		{
			ActiveSequenceIndex = i;
			ActiveLineIndex = 0;
			bIsSpeaking = true;
			PlayCurrentLine();
			return;
		}
	}
}

void UNarr_DialogueSystem::AdvanceDialogue()
{
	if (!bIsSpeaking || ActiveSequenceIndex == -1)
	{
		return;
	}

	ActiveLineIndex++;

	const FNarr_DialogueSequence& Seq = DialogueSequences[ActiveSequenceIndex];

	if (ActiveLineIndex >= Seq.Lines.Num())
	{
		// Sequence complete
		MarkSequencePlayed(ActiveSequenceIndex);
		StopDialogue();
		return;
	}

	PlayCurrentLine();
}

void UNarr_DialogueSystem::StopDialogue()
{
	bIsSpeaking = false;
	ActiveSequenceIndex = -1;
	ActiveLineIndex = -1;

	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().ClearTimer(DialogueAdvanceTimer);
	}
}

FString UNarr_DialogueSystem::GetCurrentLineText() const
{
	if (!bIsSpeaking || ActiveSequenceIndex == -1 || ActiveLineIndex == -1)
	{
		return FString();
	}

	if (ActiveSequenceIndex >= DialogueSequences.Num())
	{
		return FString();
	}

	const FNarr_DialogueSequence& Seq = DialogueSequences[ActiveSequenceIndex];

	if (ActiveLineIndex >= Seq.Lines.Num())
	{
		return FString();
	}

	return Seq.Lines[ActiveLineIndex].LineText;
}

FString UNarr_DialogueSystem::GetCurrentSpeakerName() const
{
	if (!bIsSpeaking || ActiveSequenceIndex == -1 || ActiveLineIndex == -1)
	{
		return SpeakerDisplayName;
	}

	if (ActiveSequenceIndex >= DialogueSequences.Num())
	{
		return SpeakerDisplayName;
	}

	const FNarr_DialogueSequence& Seq = DialogueSequences[ActiveSequenceIndex];

	if (ActiveLineIndex >= Seq.Lines.Num())
	{
		return SpeakerDisplayName;
	}

	const FString& LineSpeaker = Seq.Lines[ActiveLineIndex].SpeakerID;
	return LineSpeaker.IsEmpty() ? SpeakerDisplayName : LineSpeaker;
}

bool UNarr_DialogueSystem::IsPlayerInRange() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	APlayerController* PC = World->GetFirstPlayerController();
	if (!PC)
	{
		return false;
	}

	APawn* PlayerPawn = PC->GetPawn();
	if (!PlayerPawn)
	{
		return false;
	}

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return false;
	}

	float DistSq = FVector::DistSquared(Owner->GetActorLocation(), PlayerPawn->GetActorLocation());
	return DistSq <= (ProximityTriggerRadius * ProximityTriggerRadius);
}

void UNarr_DialogueSystem::RegisterDialogueSequence(const FNarr_DialogueSequence& NewSequence)
{
	// Check for duplicate ID
	for (const FNarr_DialogueSequence& Existing : DialogueSequences)
	{
		if (Existing.SequenceID == NewSequence.SequenceID)
		{
			return;
		}
	}

	DialogueSequences.Add(NewSequence);
}

int32 UNarr_DialogueSystem::FindSequenceIndexByID(const FString& SequenceID) const
{
	for (int32 i = 0; i < DialogueSequences.Num(); ++i)
	{
		if (DialogueSequences[i].SequenceID == SequenceID)
		{
			return i;
		}
	}
	return -1;
}

void UNarr_DialogueSystem::PlayCurrentLine()
{
	if (ActiveSequenceIndex == -1 || ActiveLineIndex == -1)
	{
		return;
	}

	if (ActiveSequenceIndex >= DialogueSequences.Num())
	{
		StopDialogue();
		return;
	}

	const FNarr_DialogueSequence& Seq = DialogueSequences[ActiveSequenceIndex];

	if (ActiveLineIndex >= Seq.Lines.Num())
	{
		StopDialogue();
		return;
	}

	const FNarr_DialogueLine& Line = Seq.Lines[ActiveLineIndex];

	// Schedule auto-advance after line duration
	float AdvanceDelay = FMath::Max(Line.DisplayDuration, 1.0f);

	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().SetTimer(
			DialogueAdvanceTimer,
			this,
			&UNarr_DialogueSystem::AdvanceDialogue,
			AdvanceDelay,
			false
		);
	}
}

void UNarr_DialogueSystem::MarkSequencePlayed(int32 SequenceIndex)
{
	if (SequenceIndex >= 0 && SequenceIndex < DialogueSequences.Num())
	{
		DialogueSequences[SequenceIndex].bHasPlayed = true;
	}
}
