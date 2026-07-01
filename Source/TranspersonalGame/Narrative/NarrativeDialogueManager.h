#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Components/AudioComponent.h"
#include "NarrativeDialogueManager.generated.h"

// ============================================================
// Enums — global scope (UHT requirement)
// ============================================================

UENUM(BlueprintType)
enum class ENarr_DialogueTriggerType : uint8
{
	ProximityEnter      UMETA(DisplayName = "Proximity Enter"),
	ProximityExit       UMETA(DisplayName = "Proximity Exit"),
	QuestComplete       UMETA(DisplayName = "Quest Complete"),
	DinosaurSighted     UMETA(DisplayName = "Dinosaur Sighted"),
	ResourceGathered    UMETA(DisplayName = "Resource Gathered"),
	PlayerDamaged       UMETA(DisplayName = "Player Damaged"),
	TimeOfDay           UMETA(DisplayName = "Time Of Day"),
	Manual              UMETA(DisplayName = "Manual Trigger")
};

UENUM(BlueprintType)
enum class ENarr_SpeakerRole : uint8
{
	TribalElder         UMETA(DisplayName = "Tribal Elder"),
	HerdTracker         UMETA(DisplayName = "Herd Tracker"),
	Scout               UMETA(DisplayName = "Scout"),
	Craftsman           UMETA(DisplayName = "Craftsman"),
	HunterLeader        UMETA(DisplayName = "Hunter Leader"),
	Narrator            UMETA(DisplayName = "Narrator"),
	PlayerCharacter     UMETA(DisplayName = "Player Character")
};

UENUM(BlueprintType)
enum class ENarr_DialogueState : uint8
{
	Idle                UMETA(DisplayName = "Idle"),
	Playing             UMETA(DisplayName = "Playing"),
	WaitingForChoice    UMETA(DisplayName = "Waiting For Choice"),
	Cooldown            UMETA(DisplayName = "Cooldown"),
	Completed           UMETA(DisplayName = "Completed")
};

// ============================================================
// Structs — global scope
// ============================================================

USTRUCT(BlueprintType)
struct FNarr_DialogueLine
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
	FString LineID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
	FString LineText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
	ENarr_SpeakerRole Speaker;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
	float Duration;

	/** Optional audio asset path — e.g. /Game/Audio/Dialogue/TribalElder_01 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
	FString AudioAssetPath;

	/** Next line ID to play automatically (empty = end or choice) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
	FString NextLineID;

	FNarr_DialogueLine()
		: LineID(TEXT(""))
		, LineText(TEXT(""))
		, Speaker(ENarr_SpeakerRole::Narrator)
		, Duration(3.0f)
		, AudioAssetPath(TEXT(""))
		, NextLineID(TEXT(""))
	{}
};

USTRUCT(BlueprintType)
struct FNarr_DialogueChoice
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
	FString ChoiceText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
	FString NextLineID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
	bool bEndsDialogue;

	FNarr_DialogueChoice()
		: ChoiceText(TEXT(""))
		, NextLineID(TEXT(""))
		, bEndsDialogue(false)
	{}
};

USTRUCT(BlueprintType)
struct FNarr_DialogueTree
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
	FString TreeID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
	FString StartLineID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
	TArray<FNarr_DialogueLine> Lines;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
	TArray<FNarr_DialogueChoice> Choices;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
	ENarr_DialogueTriggerType TriggerType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
	float CooldownSeconds;

	FNarr_DialogueTree()
		: TreeID(TEXT(""))
		, StartLineID(TEXT(""))
		, TriggerType(ENarr_DialogueTriggerType::ProximityEnter)
		, CooldownSeconds(60.0f)
	{}
};

// ============================================================
// ANarrativeDialogueManager — Actor placed in level
// ============================================================

UCLASS(ClassGroup = (Narrative), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ANarrativeDialogueManager : public AActor
{
	GENERATED_BODY()

public:
	ANarrativeDialogueManager();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	// ---- Components ----

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Narrative",
		meta = (AllowPrivateAccess = "true"))
	USphereComponent* TriggerSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Narrative",
		meta = (AllowPrivateAccess = "true"))
	UAudioComponent* DialogueAudio;

	// ---- Dialogue Data ----

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
	TArray<FNarr_DialogueTree> DialogueTrees;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
	float ProximityRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
	ENarr_SpeakerRole AssignedSpeaker;

	// ---- Runtime State ----

	UPROPERTY(BlueprintReadOnly, Category = "Narrative")
	ENarr_DialogueState CurrentState;

	UPROPERTY(BlueprintReadOnly, Category = "Narrative")
	FString ActiveLineID;

	UPROPERTY(BlueprintReadOnly, Category = "Narrative")
	FString ActiveTreeID;

	// ---- Public API ----

	UFUNCTION(BlueprintCallable, Category = "Narrative")
	void TriggerDialogueTree(const FString& TreeID);

	UFUNCTION(BlueprintCallable, Category = "Narrative")
	void AdvanceDialogue();

	UFUNCTION(BlueprintCallable, Category = "Narrative")
	void EndDialogue();

	UFUNCTION(BlueprintCallable, Category = "Narrative")
	bool IsDialoguePlaying() const;

	UFUNCTION(BlueprintCallable, Category = "Narrative")
	FNarr_DialogueLine GetCurrentLine() const;

	UFUNCTION(BlueprintCallable, Category = "Narrative")
	TArray<FNarr_DialogueChoice> GetCurrentChoices() const;

	UFUNCTION(BlueprintCallable, Category = "Narrative")
	void SelectChoice(int32 ChoiceIndex);

	UFUNCTION(BlueprintCallable, Category = "Narrative")
	void RegisterDialogueTree(const FNarr_DialogueTree& NewTree);

	/** Build default dialogue trees for all speaker roles */
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Narrative")
	void BuildDefaultDialogueTrees();

private:
	float LineTimer;
	float CooldownTimer;
	bool bOnCooldown;

	FNarr_DialogueLine* FindLine(const FString& LineID);
	const FNarr_DialogueTree* FindTree(const FString& TreeID) const;
	void PlayLine(const FNarr_DialogueLine& Line);
	void OnProximityOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);
};
