#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "DialogueManager.h"
#include "DialogueTriggerComponent.generated.h"

/**
 * UNarr_DialogueTriggerComponent
 * Attach to any actor to auto-trigger dialogue when player enters proximity.
 * Supports one-shot, repeatable (with cooldown), and condition-gated triggers.
 */
UCLASS(ClassGroup = "Narrative", meta = (BlueprintSpawnableComponent), DisplayName = "Dialogue Trigger")
class TRANSPERSONALGAME_API UNarr_DialogueTriggerComponent : public USphereComponent
{
	GENERATED_BODY()

public:
	UNarr_DialogueTriggerComponent();

	virtual void BeginPlay() override;

	// ─── Configuration ──────────────────────────────────────────────────────

	/** DataTable row name of the dialogue sequence to play */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
	FName DialogueSequenceID;

	/** Speaker role for this trigger's NPC */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
	ENarr_SpeakerRole SpeakerRole = ENarr_SpeakerRole::Scout;

	/** Trigger type — controls when dialogue fires */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
	ENarr_DialogueTrigger TriggerType = ENarr_DialogueTrigger::OnProximity;

	/** If true, dialogue fires only once per game session */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
	bool bOneShot = true;

	/** Seconds before this trigger can fire again (0 = one-shot only) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue",
		meta = (ClampMin = "0.0", EditCondition = "!bOneShot"))
	float CooldownSeconds = 120.0f;

	/** Optional: only trigger if player has this item in inventory */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Conditions")
	FName RequiredItemID;

	/** Optional: only trigger if this quest is active */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Conditions")
	FName RequiredQuestID;

	/** Optional: minimum danger level to trigger (0 = always) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Conditions",
		meta = (ClampMin = "0", ClampMax = "10"))
	int32 MinDangerLevel = 0;

	// ─── State ──────────────────────────────────────────────────────────────

	/** Has this trigger already fired? */
	UPROPERTY(BlueprintReadOnly, Category = "Narrative|State",
		meta = (AllowPrivateAccess = "true"))
	bool bHasFired = false;

	/** World time when last triggered */
	UPROPERTY(BlueprintReadOnly, Category = "Narrative|State",
		meta = (AllowPrivateAccess = "true"))
	float LastTriggerTime = -999.0f;

	// ─── Interface ──────────────────────────────────────────────────────────

	/** Force-trigger the dialogue (callable from Blueprint/code) */
	UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
	void ForceTrigger();

	/** Reset the one-shot flag so this trigger can fire again */
	UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
	void ResetTrigger();

	/** Returns true if this trigger is ready to fire */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Narrative|Dialogue")
	bool CanTriggerNow() const;

protected:
	UFUNCTION()
	void OnPlayerEntered(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

private:
	/** Cached reference to the DialogueManager subsystem */
	UPROPERTY()
	UNarr_DialogueManager* CachedDialogueManager;

	bool CheckConditions() const;
};
