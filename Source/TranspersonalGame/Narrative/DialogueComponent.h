#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NarrativeManager.h"
#include "DialogueComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDialogueTriggered, const FString&, DialogueText, const FString&, SpeakerName);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UDialogueComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UDialogueComponent();

protected:
    virtual void BeginPlay() override;

public:
    UPROPERTY(BlueprintAssignable, Category = "Dialogue")
    FOnDialogueTriggered OnDialogueTriggered;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FString> AvailableDialogues;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float InteractionRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bCanRepeatDialogue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FString> RequiredStoryFlags;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void TriggerDialogue(int32 DialogueIndex = 0);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool CanInteract() const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    TArray<FString> GetAvailableDialogueOptions() const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void StartConversation(AActor* Interactor);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void EndConversation();

protected:
    UPROPERTY()
    UNarrativeManager* NarrativeManager;

    UPROPERTY()
    AActor* CurrentInteractor;

    UPROPERTY()
    TArray<FString> UsedDialogues;

    bool CheckStoryRequirements() const;
    FString GetRandomAvailableDialogue() const;
};