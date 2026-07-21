#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/TriggerBox.h"
#include "../SharedTypes.h"
#include "Narr_DialogueTrigger.generated.h"

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANarr_DialogueTrigger : public ATriggerBox
{
    GENERATED_BODY()

public:
    ANarr_DialogueTrigger();

protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    void OnTriggerEnter(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnTriggerExit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    // Dialogue properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString DialogueID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FString> DialogueLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_DialogueType DialogueType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bIsRepeatable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bRequiresPlayerInput;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float TriggerRadius;

    // State tracking
    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    bool bHasBeenTriggered;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    bool bPlayerInRange;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    int32 TimesTriggered;

public:
    // Blueprint callable functions
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerDialogue();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void ResetTrigger();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool CanTrigger() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void SetDialogueLines(const TArray<FString>& NewLines);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void AddDialogueLine(const FString& NewLine);

    // Editor functions
    UFUNCTION(CallInEditor, Category = "Narrative")
    void TestDialogue();

private:
    void InitializeDialogue();
    void PlayDialogueAudio();
    void ShowDialogueUI();
    void HideDialogueUI();
};