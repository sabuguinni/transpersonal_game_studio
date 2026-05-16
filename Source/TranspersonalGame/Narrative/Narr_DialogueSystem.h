#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Components/StaticMeshComponent.h"
#include "../SharedTypes.h"
#include "Narr_DialogueSystem.generated.h"

UENUM(BlueprintType)
enum class ENarr_DialogueType : uint8
{
    SurvivalTip     UMETA(DisplayName = "Survival Tip"),
    DangerWarning   UMETA(DisplayName = "Danger Warning"),
    Discovery       UMETA(DisplayName = "Discovery"),
    Achievement     UMETA(DisplayName = "Achievement"),
    Environmental   UMETA(DisplayName = "Environmental")
};

UENUM(BlueprintType)
enum class ENarr_TriggerCondition : uint8
{
    ProximityEnter  UMETA(DisplayName = "Proximity Enter"),
    ResourceFound   UMETA(DisplayName = "Resource Found"),
    DinosaurSighted UMETA(DisplayName = "Dinosaur Sighted"),
    QuestComplete   UMETA(DisplayName = "Quest Complete"),
    TimeOfDay       UMETA(DisplayName = "Time of Day")
};

USTRUCT(BlueprintType)
struct FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_DialogueType DialogueType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsNarration;

    FNarr_DialogueLine()
    {
        DialogueText = TEXT("");
        CharacterName = TEXT("Narrator");
        Duration = 3.0f;
        DialogueType = ENarr_DialogueType::SurvivalTip;
        bIsNarration = true;
    }
};

USTRUCT(BlueprintType)
struct FNarr_DialogueTrigger
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger")
    ENarr_TriggerCondition TriggerType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger")
    float TriggerRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger")
    FString RequiredTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger")
    bool bOnlyTriggerOnce;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger")
    float CooldownTime;

    FNarr_DialogueTrigger()
    {
        TriggerType = ENarr_TriggerCondition::ProximityEnter;
        TriggerRadius = 500.0f;
        RequiredTag = TEXT("");
        bOnlyTriggerOnce = false;
        CooldownTime = 10.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANarr_DialogueSystem : public AActor
{
    GENERATED_BODY()

public:
    ANarr_DialogueSystem();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* TriggerMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAudioComponent* AudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FNarr_DialogueLine> DialogueLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FNarr_DialogueTrigger TriggerSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    EBiomeType AssociatedBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bAutoPlayOnTrigger;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float DialogueVolume;

private:
    UPROPERTY()
    bool bHasTriggered;

    UPROPERTY()
    float LastTriggerTime;

    UPROPERTY()
    int32 CurrentDialogueIndex;

    UPROPERTY()
    bool bIsPlayingDialogue;

    UPROPERTY()
    float CurrentDialogueTimer;

public:
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void TriggerDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void PlayNextDialogueLine();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void StopDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void AddDialogueLine(const FNarr_DialogueLine& NewLine);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool CheckTriggerConditions(AActor* TriggeringActor);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void SetBiomeContext(EBiomeType NewBiome);

    UFUNCTION(BlueprintPure, Category = "Dialogue")
    bool IsDialogueActive() const { return bIsPlayingDialogue; }

    UFUNCTION(BlueprintPure, Category = "Dialogue")
    int32 GetCurrentDialogueIndex() const { return CurrentDialogueIndex; }

    UFUNCTION(BlueprintPure, Category = "Dialogue")
    int32 GetTotalDialogueLines() const { return DialogueLines.Num(); }

protected:
    UFUNCTION()
    void OnDialogueComplete();

    UFUNCTION()
    void CheckProximityTrigger();

    void InitializeDialogueSystem();
    void LoadBiomeSpecificDialogue();
};