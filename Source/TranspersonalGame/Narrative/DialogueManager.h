#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "SharedTypes.h"
#include "DialogueManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsNarration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ESurvivalThreat ThreatLevel;

    FNarr_DialogueEntry()
    {
        SpeakerName = TEXT("Unknown");
        DialogueText = TEXT("");
        Duration = 3.0f;
        bIsNarration = false;
        ThreatLevel = ESurvivalThreat::None;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueSequence
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SequenceID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FNarr_DialogueEntry> DialogueEntries;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsRepeatable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float CooldownTime;

    FNarr_DialogueSequence()
    {
        SequenceID = TEXT("");
        bIsRepeatable = false;
        CooldownTime = 60.0f;
    }
};

UCLASS()
class TRANSPERSONALGAME_API UDialogueManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UDialogueManager();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void PlayDialogueSequence(const FString& SequenceID);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void PlaySurvivalWarning(ESurvivalThreat ThreatType, const FString& CustomMessage = TEXT(""));

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void PlayNarration(const FString& NarrationText, float Duration = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool IsDialoguePlaying() const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void StopCurrentDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void RegisterDialogueSequence(const FNarr_DialogueSequence& NewSequence);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
    TArray<FNarr_DialogueSequence> DialogueSequences;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
    TMap<ESurvivalThreat, FString> ThreatWarnings;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    bool bIsCurrentlyPlaying;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    FString CurrentSequenceID;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    int32 CurrentEntryIndex;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    float CurrentEntryTimer;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    TMap<FString, float> SequenceCooldowns;

private:
    void InitializeThreatWarnings();
    void UpdateDialoguePlayback(float DeltaTime);
    void PlayNextDialogueEntry();
    void OnDialogueSequenceComplete();
    bool CanPlaySequence(const FString& SequenceID) const;

    FTimerHandle DialogueUpdateHandle;
};