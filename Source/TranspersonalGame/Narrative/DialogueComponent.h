#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "NarrativeManager.h"
#include "DialogueComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDialogueTriggered, const FString&, DialogueID, AActor*, Speaker);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDialogueCompleted, const FString&, DialogueID);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UDialogueComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UDialogueComponent();

protected:
    virtual void BeginPlay() override;

public:
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void TriggerDialogue(const FString& DialogueID);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void SetDialogueEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool IsDialogueActive() const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void AddDialogueOption(const FString& DialogueID, const FString& OptionText);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void SelectDialogueOption(int32 OptionIndex);

    UPROPERTY(BlueprintAssignable, Category = "Dialogue")
    FOnDialogueTriggered OnDialogueTriggered;

    UPROPERTY(BlueprintAssignable, Category = "Dialogue")
    FOnDialogueCompleted OnDialogueCompleted;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bDialogueEnabled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DefaultDialogueID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float DialogueRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bAutoTriggerOnOverlap;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    bool bDialogueActive;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    FString CurrentDialogueID;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    TArray<FString> CurrentDialogueOptions;

private:
    UPROPERTY()
    UNarrativeManager* NarrativeManager;

    void InitializeNarrativeManager();
    bool IsPlayerInRange() const;
};