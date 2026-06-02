#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/TriggerBox.h"
#include "Narr_InteractionManager.generated.h"

UENUM(BlueprintType)
enum class ENarr_InteractionType : uint8
{
    Dialogue        UMETA(DisplayName = "Dialogue"),
    QuestGiver      UMETA(DisplayName = "Quest Giver"),
    Storyteller     UMETA(DisplayName = "Storyteller"),
    Mentor          UMETA(DisplayName = "Mentor"),
    Scout           UMETA(DisplayName = "Scout")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_InteractionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    FString InteractionText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    FString AudioPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    ENarr_InteractionType InteractionType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    bool bRequiresQuestCompletion;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    FString RequiredQuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    int32 MinPlayerLevel;

    FNarr_InteractionData()
    {
        InteractionText = TEXT("Default interaction");
        AudioPath = TEXT("");
        InteractionType = ENarr_InteractionType::Dialogue;
        bRequiresQuestCompletion = false;
        RequiredQuestID = TEXT("");
        MinPlayerLevel = 1;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANarr_InteractionManager : public ATriggerBox
{
    GENERATED_BODY()

public:
    ANarr_InteractionManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FNarr_InteractionData InteractionData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    UStaticMeshComponent* VisualMarker;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float InteractionRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bIsActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bShowMarker;

public:
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerInteraction(AActor* InteractingActor);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool CanInteract(AActor* InteractingActor) const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void SetInteractionData(const FNarr_InteractionData& NewData);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FNarr_InteractionData GetInteractionData() const { return InteractionData; }

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void SetActive(bool bActive);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void PlayInteractionAudio();

protected:
    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, 
                       UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
                       bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, 
                     UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
    bool bPlayerInRange;
    AActor* CurrentPlayer;
};