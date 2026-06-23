#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
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
    FString SpeakerID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString LineText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_DialogueTone Tone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float DisplayDuration;

    FNarr_DialogueLine()
        : SpeakerID(TEXT("Unknown"))
        , LineText(TEXT(""))
        , Tone(ENarr_DialogueTone::Informative)
        , DisplayDuration(4.0f)
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
    bool bIsTriggered;

    FNarr_DialogueSequence()
        : SequenceID(NAME_None)
        , bIsTriggered(false)
    {}
};

UCLASS(ClassGroup=(Narrative), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UDialogueSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UDialogueSystem();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    TArray<FNarr_DialogueSequence> DialogueLibrary;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative|Dialogue")
    bool bIsDialogueActive;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative|Dialogue")
    int32 CurrentLineIndex;

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void StartDialogueSequence(FName SequenceID);

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void AdvanceLine();

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void EndDialogue();

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    FNarr_DialogueLine GetCurrentLine() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    bool HasMoreLines() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void RegisterSequence(FNarr_DialogueSequence NewSequence);

protected:
    virtual void BeginPlay() override;

private:
    FNarr_DialogueSequence* ActiveSequence;
    void InitializeDefaultDialogues();
};
