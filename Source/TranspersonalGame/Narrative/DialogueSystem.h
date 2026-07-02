
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Sound/SoundBase.h"
#include "DialogueSystem.generated.h"

UENUM(BlueprintType)
enum class ENarr_DialogueTrigger : uint8
{
    OnProximity     UMETA(DisplayName = "On Proximity"),
    OnQuestStart    UMETA(DisplayName = "On Quest Start"),
    OnQuestComplete UMETA(DisplayName = "On Quest Complete"),
    OnCombatStart   UMETA(DisplayName = "On Combat Start"),
    OnDanger        UMETA(DisplayName = "On Danger"),
    OnIdle          UMETA(DisplayName = "On Idle"),
    OnDeath         UMETA(DisplayName = "On Death")
};

USTRUCT(BlueprintType)
struct FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString SpeakerID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FText LineText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TSoftObjectPtr<USoundBase> VoiceAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float DisplayDuration = 4.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_DialogueTrigger Trigger = ENarr_DialogueTrigger::OnProximity;
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
    bool bPlayOnce = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bHasPlayed = false;
};

UCLASS(ClassGroup = (Narrative), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNarr_DialogueSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UNarr_DialogueSystem();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    TArray<FNarr_DialogueSequence> DialogueSequences;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    float ProximityTriggerRadius = 400.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative|Dialogue", meta = (AllowPrivateAccess = "true"))
    bool bIsPlayingDialogue = false;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative|Dialogue", meta = (AllowPrivateAccess = "true"))
    int32 CurrentLineIndex = 0;

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void TriggerDialogueSequence(FName SequenceID);

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void AdvanceLine();

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void StopDialogue();

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    bool IsDialoguePlaying() const { return bIsPlayingDialogue; }

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    FNarr_DialogueLine GetCurrentLine() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void CheckProximityTrigger(FVector PlayerLocation);

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    FNarr_DialogueSequence* ActiveSequence = nullptr;
    float LineTimer = 0.0f;
};
