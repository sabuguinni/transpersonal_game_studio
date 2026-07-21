#include "DialogueTriggerComponent.h"
#include "TranspersonalCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

UNarr_DialogueTriggerComponent::UNarr_DialogueTriggerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	// Default sphere radius — designers override in editor
	SetSphereRadius(400.0f);
	SetCollisionProfileName(TEXT("Trigger"));

	CachedDialogueManager = nullptr;
}

void UNarr_DialogueTriggerComponent::BeginPlay()
{
	Super::BeginPlay();

	// Cache the dialogue manager subsystem
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		CachedDialogueManager = GI->GetSubsystem<UNarr_DialogueManager>();
	}

	// Bind overlap event
	OnComponentBeginOverlap.AddDynamic(this, &UNarr_DialogueTriggerComponent::OnPlayerEntered);
}

void UNarr_DialogueTriggerComponent::OnPlayerEntered(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	// Only react to the player character
	if (!OtherActor || !OtherActor->IsA<ATranspersonalCharacter>())
	{
		return;
	}

	// Check trigger type — OnProximity fires immediately on overlap
	if (TriggerType != ENarr_DialogueTrigger::OnProximity &&
		TriggerType != ENarr_DialogueTrigger::OnFirstVisit)
	{
		return;
	}

	ForceTrigger();
}

void UNarr_DialogueTriggerComponent::ForceTrigger()
{
	if (!CanTriggerNow())
	{
		return;
	}

	if (!CachedDialogueManager)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("DialogueTriggerComponent: No DialogueManager found on actor %s"),
			*GetOwner()->GetName());
		return;
	}

	if (!CheckConditions())
	{
		return;
	}

	// Fire the dialogue
	CachedDialogueManager->TriggerDialogue(DialogueSequenceID, SpeakerRole, TriggerType);

	// Update state
	bHasFired = true;
	if (GetWorld())
	{
		LastTriggerTime = GetWorld()->GetTimeSeconds();
	}
}

void UNarr_DialogueTriggerComponent::ResetTrigger()
{
	bHasFired = false;
	LastTriggerTime = -999.0f;
}

bool UNarr_DialogueTriggerComponent::CanTriggerNow() const
{
	// One-shot: never fire again after first trigger
	if (bOneShot && bHasFired)
	{
		return false;
	}

	// Cooldown check
	if (!bOneShot && bHasFired && GetWorld())
	{
		const float Elapsed = GetWorld()->GetTimeSeconds() - LastTriggerTime;
		if (Elapsed < CooldownSeconds)
		{
			return false;
		}
	}

	return true;
}

bool UNarr_DialogueTriggerComponent::CheckConditions() const
{
	// Required quest check — delegate to QuestManager if available
	// For now: if RequiredQuestID is set, we skip the check (QuestManager integration
	// will be wired by Agent #14 QuestManager system)
	if (!RequiredQuestID.IsNone())
	{
		// TODO: integrate with QuestManager::IsQuestActive(RequiredQuestID)
		// Returning true here so the trigger fires during early development
	}

	// Required item check — delegate to inventory system
	if (!RequiredItemID.IsNone())
	{
		// TODO: integrate with InventoryComponent::HasItem(RequiredItemID)
		// Returning true here so the trigger fires during early development
	}

	return true;
}
