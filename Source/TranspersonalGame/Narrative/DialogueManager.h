#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/TriggerBox.h"
#include "DialogueManager.generated.h"

UENUM(BlueprintType)
enum class ENarr_DialogueType : uint8
{
    Warning         UMETA(DisplayName = "Warning"),
    Discovery       UMETA(DisplayName = "Discovery"),
    QuestGiver      UMETA(DisplayName = "QuestGiver"),
    Environmental   UMETA(DisplayName = "Environmental"),
    CombatAlert     UMETA(DisplayName = "CombatAlert")
};

UENUM(BlueprintType)
enum class ENarr_SpeakerRole : uint8
{
    TribeElder      UMETA(DisplayName = "TribeElder"),
    Scout           UMETA(DisplayName = "Scout"),
    Hunter          UMETA(DisplayName = "Hunter"),
    Narrator        UMETA(DisplayName = "Narrator"),
    PlayerInternal  UMETA(DisplayName = "PlayerInternal")
};

USTRUCT(BlueprintType)
struct FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString LineText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_SpeakerRole Speaker;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_DialogueType DialogueType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float DisplayDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bBlocksMovement;

    FNarr_DialogueLine()
        : LineText(TEXT(""))
        , Speaker(ENarr_SpeakerRole::Narrator)
        , DialogueType(ENarr_DialogueType::Environmental)
        , DisplayDuration(4.0f)
        , bBlocksMovement(false)
    {}
};

USTRUCT(BlueprintType)
struct FNarr_DialogueSequence
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FName SequenceID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueLine> Lines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bHasBeenPlayed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bCanReplay;

    FNarr_DialogueSequence()
        : SequenceID(NAME_None)
        , bHasBeenPlayed(false)
        , bCanReplay(false)
    {}
};

UCLASS(ClassGroup = (Narrative), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNarr_DialogueManagerComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNarr_DialogueManagerComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    TArray<FNarr_DialogueSequence> DialogueLibrary;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative|Dialogue")
    bool bIsDialogueActive;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative|Dialogue")
    int32 CurrentLineIndex;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative|Dialogue")
    FNarr_DialogueLine ActiveLine;

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    bool PlaySequence(FName SequenceID);

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void AdvanceLine();

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void EndDialogue();

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    bool HasSequenceBeenPlayed(FName SequenceID) const;

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void RegisterSequence(const FNarr_DialogueSequence& Sequence);

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    FNarr_DialogueLine GetCurrentLine() const;

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    FNarr_DialogueSequence* ActiveSequence;
    float LineTimer;

    void InitializeDefaultDialogues();
    void AdvanceLineInternal();
};

UCLASS(ClassGroup = (Narrative), BlueprintType)
class TRANSPERSONALGAME_API ANarr_DialogueTriggerActor : public AActor
{
    GENERATED_BODY()

public:
    ANarr_DialogueTriggerActor();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Trigger")
    FName TriggerSequenceID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Trigger")
    float TriggerRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Trigger")
    bool bOneShot;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative|Trigger")
    bool bHasTriggered;

    UFUNCTION(BlueprintCallable, Category = "Narrative|Trigger")
    void OnPlayerEnterZone();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    bool bPlayerInRange;
};
