#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "SharedTypes.h"
#include "DialogueSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FText DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString AudioPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ESurvivalState RequiredState;

    FNarr_DialogueLine()
    {
        SpeakerName = TEXT("Unknown");
        DialogueText = FText::FromString(TEXT("..."));
        AudioPath = TEXT("");
        Duration = 3.0f;
        RequiredState = ESurvivalState::Normal;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueSequence
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SequenceID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FNarr_DialogueLine> DialogueLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsRepeatable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float CooldownTime;

    FNarr_DialogueSequence()
    {
        SequenceID = TEXT("DefaultSequence");
        bIsRepeatable = false;
        CooldownTime = 30.0f;
    }
};

UENUM(BlueprintType)
enum class ENarr_DialogueTrigger : uint8
{
    None            UMETA(DisplayName = "None"),
    PlayerEnter     UMETA(DisplayName = "Player Enter"),
    LowHealth       UMETA(DisplayName = "Low Health"),
    LowWater        UMETA(DisplayName = "Low Water"),
    DinosaurNear    UMETA(DisplayName = "Dinosaur Near"),
    SafeZone        UMETA(DisplayName = "Safe Zone"),
    DangerZone      UMETA(DisplayName = "Danger Zone")
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDialogueSystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UDialogueSystem();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void PlayDialogueSequence(const FString& SequenceID);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void TriggerDialogue(ENarr_DialogueTrigger TriggerType, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void RegisterDialogueSequence(const FNarr_DialogueSequence& Sequence);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool IsDialoguePlaying() const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void StopCurrentDialogue();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TMap<FString, FNarr_DialogueSequence> DialogueSequences;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TMap<ENarr_DialogueTrigger, FString> TriggerToSequenceMap;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString CurrentSequenceID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    int32 CurrentLineIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsPlaying;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float LastTriggerTime;

private:
    void InitializeDefaultDialogues();
    void PlayNextLine();
    void OnLineFinished();

    FTimerHandle DialogueTimerHandle;
};

#include "DialogueSystem.generated.h"