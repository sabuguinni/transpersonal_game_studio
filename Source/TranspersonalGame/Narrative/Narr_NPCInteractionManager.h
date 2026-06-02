#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "Narr_NPCInteractionManager.generated.h"

class ANarr_TribalNPC;

USTRUCT(BlueprintType)
struct FNarr_NPCInteraction
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    FString InteractionText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    float InteractionRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    bool bRequiresItem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    FString RequiredItemName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    FString AudioPath;

    FNarr_NPCInteraction()
    {
        InteractionText = TEXT("Talk to tribal member");
        InteractionRange = 200.0f;
        bRequiresItem = false;
        RequiredItemName = TEXT("");
        AudioPath = TEXT("");
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNarr_NPCInteractionManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UNarr_NPCInteractionManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactions")
    TArray<FNarr_NPCInteraction> AvailableInteractions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactions")
    int32 CurrentInteractionIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactions")
    bool bCanInteract;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactions")
    float LastInteractionTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactions")
    float InteractionCooldown;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Interactions")
    bool CanPlayerInteract(AActor* PlayerActor);

    UFUNCTION(BlueprintCallable, Category = "Interactions")
    FNarr_NPCInteraction GetCurrentInteraction();

    UFUNCTION(BlueprintCallable, Category = "Interactions")
    void TriggerInteraction(AActor* PlayerActor);

    UFUNCTION(BlueprintCallable, Category = "Interactions")
    void AddInteraction(const FNarr_NPCInteraction& NewInteraction);

    UFUNCTION(BlueprintCallable, Category = "Interactions")
    void SetInteractionEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Interactions")
    void PlayInteractionAudio(const FString& AudioPath);
};