#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DialogueSystem.generated.h"

// Narr_ prefix per UE5 compilation rules — unique type names across project

UENUM(BlueprintType)
enum class ENarr_DialogueTriggerType : uint8
{
	Proximity       UMETA(DisplayName = "Proximity"),
	QuestStart      UMETA(DisplayName = "QuestStart"),
	QuestComplete   UMETA(DisplayName = "QuestComplete"),
	CombatAlert     UMETA(DisplayName = "CombatAlert"),
	Discovery       UMETA(DisplayName = "Discovery"),
};

USTRUCT(BlueprintType)
struct FNarr_DialogueLine
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
	FString SpeakerID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
	FString LineText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
	FString AudioAssetPath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
	float DisplayDuration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
	ENarr_DialogueTriggerType TriggerType;

	FNarr_DialogueLine()
		: SpeakerID(TEXT("Unknown"))
		, LineText(TEXT(""))
		, AudioAssetPath(TEXT(""))
		, DisplayDuration(4.0f)
		, TriggerType(ENarr_DialogueTriggerType::Proximity)
	{}
};

USTRUCT(BlueprintType)
struct FNarr_DialogueSequence
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
	FString SequenceID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
	TArray<FNarr_DialogueLine> Lines;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
	bool bPlayOnce;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
	bool bHasPlayed;

	FNarr_DialogueSequence()
		: SequenceID(TEXT(""))
		, bPlayOnce(true)
		, bHasPlayed(false)
	{}
};

/**
 * UNarr_DialogueSystem
 * ActorComponent that manages NPC dialogue sequences for the prehistoric survival game.
 * Attach to NPC actors. Triggers voice lines based on player proximity and game events.
 */
UCLASS(ClassGroup = (Narrative), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNarr_DialogueSystem : public UActorComponent
{
	GENERATED_BODY()

public:
	UNarr_DialogueSystem();

	// All registered dialogue sequences for this NPC
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
	TArray<FNarr_DialogueSequence> DialogueSequences;

	// Radius within which player triggers proximity dialogue
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
	float ProximityTriggerRadius;

	// Speaker name shown in HUD subtitle bar
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
	FString SpeakerDisplayName;

	// Whether this NPC is currently speaking
	UPROPERTY(BlueprintReadOnly, Category = "Narrative|Dialogue")
	bool bIsSpeaking;

	// Index of currently active sequence
	UPROPERTY(BlueprintReadOnly, Category = "Narrative|Dialogue")
	int32 ActiveSequenceIndex;

	// Index of current line within active sequence
	UPROPERTY(BlueprintReadOnly, Category = "Narrative|Dialogue")
	int32 ActiveLineIndex;

	// Trigger a dialogue sequence by ID
	UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
	bool TriggerDialogueSequence(const FString& SequenceID);

	// Trigger dialogue by event type
	UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
	void TriggerDialogueByType(ENarr_DialogueTriggerType TriggerType);

	// Advance to next line in active sequence
	UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
	void AdvanceDialogue();

	// Stop all active dialogue
	UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
	void StopDialogue();

	// Get current line text for HUD display
	UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
	FString GetCurrentLineText() const;

	// Get current speaker name
	UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
	FString GetCurrentSpeakerName() const;

	// Check if player is within proximity radius
	UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
	bool IsPlayerInRange() const;

	// Register a new dialogue sequence at runtime
	UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
	void RegisterDialogueSequence(const FNarr_DialogueSequence& NewSequence);

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	// Timer handle for auto-advancing dialogue lines
	FTimerHandle DialogueAdvanceTimer;

	// Time since last proximity check
	float TimeSinceLastProximityCheck;

	// Proximity check interval (seconds)
	static constexpr float ProximityCheckInterval = 0.5f;

	// Internal: find sequence by ID
	int32 FindSequenceIndexByID(const FString& SequenceID) const;

	// Internal: play current line
	void PlayCurrentLine();

	// Internal: mark sequence as played
	void MarkSequencePlayed(int32 SequenceIndex);
};
