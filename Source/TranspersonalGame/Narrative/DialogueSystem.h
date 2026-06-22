// DialogueSystem.h
// Narrative & Dialogue Agent #15 — PROD_CYCLE_AUTO_20260622_009
// Dialogue system for NPC interactions in prehistoric survival game

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "DialogueSystem.generated.h"

UENUM(BlueprintType)
enum class ENarr_DialogueState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Greeting    UMETA(DisplayName = "Greeting"),
    InDialogue  UMETA(DisplayName = "InDialogue"),
    Farewell    UMETA(DisplayName = "Farewell"),
    Busy        UMETA(DisplayName = "Busy")
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
    float DisplayDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bRequiresPlayerResponse;

    FNarr_DialogueLine()
        : SpeakerName(TEXT("Unknown"))
        , LineText(TEXT(""))
        , DisplayDuration(3.0f)
        , bRequiresPlayerResponse(false)
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
    bool bCanRepeat;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FName NextSequenceID;

    FNarr_DialogueSequence()
        : SequenceID(NAME_None)
        , bCanRepeat(false)
        , NextSequenceID(NAME_None)
    {}
};

UCLASS(ClassGroup = (Narrative), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNarr_DialogueComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNarr_DialogueComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueSequence> DialogueSequences;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FName ActiveSequenceID;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative", meta = (AllowPrivateAccess = "true"))
    ENarr_DialogueState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString NPCName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float InteractionRadius;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void StartDialogue(FName SequenceID);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void AdvanceDialogue();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void EndDialogue();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FNarr_DialogueLine GetCurrentLine() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool IsInDialogue() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    ENarr_DialogueState GetDialogueState() const;

protected:
    virtual void BeginPlay() override;

private:
    int32 CurrentLineIndex;
    FNarr_DialogueSequence* ActiveSequence;
};

UCLASS()
class TRANSPERSONALGAME_API ANarr_DialogueTrigger : public AActor
{
    GENERATED_BODY()

public:
    ANarr_DialogueTrigger();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* TriggerSphere;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FName DialogueSequenceToTrigger;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    AActor* LinkedNPCActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bOneShot;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void OnPlayerEnterRange(AActor* PlayerActor);

protected:
    virtual void BeginPlay() override;

private:
    bool bHasTriggered;
};
