#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "../SharedTypes.h"
#include "Narr_DialogueSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString AudioFilePath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ESurvivalThreat ThreatLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    EBiomeType RequiredBiome;

    FNarr_DialogueLine()
    {
        SpeakerName = TEXT("Unknown");
        DialogueText = TEXT("");
        AudioFilePath = TEXT("");
        Duration = 3.0f;
        ThreatLevel = ESurvivalThreat::None;
        RequiredBiome = EBiomeType::Savanna;
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    EQuestType TriggerType;

    FNarr_DialogueSequence()
    {
        SequenceID = TEXT("Default");
        bIsRepeatable = false;
        CooldownTime = 60.0f;
        TriggerType = EQuestType::Exploration;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNarr_DialogueSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UNarr_DialogueSystem();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    TArray<FNarr_DialogueSequence> DialogueSequences;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    FString CurrentSequenceID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    int32 CurrentLineIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    bool bIsDialogueActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    float DialogueRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    TMap<FString, float> SequenceCooldowns;

public:
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool StartDialogueSequence(const FString& SequenceID);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void NextDialogueLine();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void EndDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    FNarr_DialogueLine GetCurrentDialogueLine() const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool CanTriggerSequence(const FString& SequenceID) const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void TriggerContextualDialogue(ESurvivalThreat ThreatLevel, EBiomeType CurrentBiome);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void RegisterDialogueSequence(const FNarr_DialogueSequence& NewSequence);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void PlayAudioForCurrentLine();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    TArray<FString> GetAvailableSequences() const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void SetDialogueRange(float NewRange);

private:
    void InitializeDefaultDialogues();
    bool IsPlayerInRange() const;
    void UpdateCooldowns(float DeltaTime);

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
};