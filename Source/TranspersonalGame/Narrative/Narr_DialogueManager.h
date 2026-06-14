#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "SharedTypes.h"
#include "Narr_DialogueManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueEntry
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
    TArray<FString> ResponseOptions;

    FNarr_DialogueEntry()
    {
        SpeakerName = TEXT("Unknown");
        DialogueText = FText::FromString(TEXT("..."));
        AudioPath = TEXT("");
        Duration = 3.0f;
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
    TArray<FString> Prerequisites;

    FNarr_DialogueSequence()
    {
        SequenceID = TEXT("default");
        bIsRepeatable = true;
    }
};

UCLASS()
class TRANSPERSONALGAME_API UNarr_DialogueManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void StartDialogue(const FString& SequenceID, AActor* Speaker, AActor* Listener);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void EndDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void AdvanceDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool IsDialogueActive() const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    FNarr_DialogueEntry GetCurrentDialogue() const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void RegisterDialogueSequence(const FNarr_DialogueSequence& Sequence);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    TArray<FString> GetAvailableDialogues(AActor* Speaker) const;

protected:
    UPROPERTY()
    TMap<FString, FNarr_DialogueSequence> DialogueDatabase;

    UPROPERTY()
    FNarr_DialogueSequence CurrentSequence;

    UPROPERTY()
    int32 CurrentDialogueIndex;

    UPROPERTY()
    bool bIsDialogueActive;

    UPROPERTY()
    AActor* CurrentSpeaker;

    UPROPERTY()
    AActor* CurrentListener;

    void LoadDefaultDialogues();
    bool CheckPrerequisites(const TArray<FString>& Prerequisites) const;
};