#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DialogueSystem.generated.h"

UENUM(BlueprintType)
enum class ENarr_DialogueTone : uint8
{
    Urgent      UMETA(DisplayName = "Urgent"),
    Cautious    UMETA(DisplayName = "Cautious"),
    Informative UMETA(DisplayName = "Informative"),
    Warning     UMETA(DisplayName = "Warning")
};

USTRUCT(BlueprintType)
struct FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString LineText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_DialogueTone Tone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float DisplayDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString AudioAssetPath;

    FNarr_DialogueLine()
        : SpeakerName(TEXT("Unknown"))
        , LineText(TEXT(""))
        , Tone(ENarr_DialogueTone::Informative)
        , DisplayDuration(4.0f)
        , AudioAssetPath(TEXT(""))
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
    bool bTriggeredOnce;

    FNarr_DialogueSequence()
        : SequenceID(TEXT(""))
        , bTriggeredOnce(false)
    {}
};

UCLASS(ClassGroup = (Narrative), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNarr_DialogueComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNarr_DialogueComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString NPCName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueSequence> DialogueSequences;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float InteractionRadius;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    bool bPlayerInRange;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerDialogue(const FString& SequenceID);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void SetPlayerInRange(bool bInRange);

    UFUNCTION(BlueprintPure, Category = "Narrative")
    FNarr_DialogueLine GetCurrentLine() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void AdvanceLine();

    UFUNCTION(BlueprintPure, Category = "Narrative")
    bool HasActiveDialogue() const;

protected:
    virtual void BeginPlay() override;

private:
    int32 CurrentSequenceIndex;
    int32 CurrentLineIndex;
    bool bDialogueActive;
};

UCLASS()
class TRANSPERSONALGAME_API ANarr_DialogueTriggerActor : public AActor
{
    GENERATED_BODY()

public:
    ANarr_DialogueTriggerActor();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Narrative",
        meta = (AllowPrivateAccess = "true"))
    UNarr_DialogueComponent* DialogueComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString TriggerDialogueID;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void OnPlayerEnterRange();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void OnPlayerExitRange();

protected:
    virtual void BeginPlay() override;
};
