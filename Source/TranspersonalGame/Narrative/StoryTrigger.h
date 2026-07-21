#pragma once

#include "CoreMinimal.h"
#include "Engine/TriggerBox.h"
#include "NarrativeManager.h"
#include "StoryTrigger.generated.h"

UCLASS()
class TRANSPERSONALGAME_API AStoryTrigger : public ATriggerBox
{
    GENERATED_BODY()

public:
    AStoryTrigger();

protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString StoryBeatToTrigger;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FString> RequiredFlags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    bool bTriggerOnce;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    bool bRequirePlayerCharacter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString TriggerDialogue;

protected:
    UPROPERTY()
    UNarrativeManager* NarrativeManager;

    UPROPERTY()
    bool bHasTriggered;

    bool CanTrigger(AActor* TriggeringActor) const;
};