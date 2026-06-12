#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SharedTypes.h"
#include "Narr_NPCInteractionComponent.generated.h"

UENUM(BlueprintType)
enum class ENarr_NPCMood : uint8
{
    Friendly    UMETA(DisplayName = "Friendly"),
    Neutral     UMETA(DisplayName = "Neutral"),
    Cautious    UMETA(DisplayName = "Cautious"),
    Hostile     UMETA(DisplayName = "Hostile"),
    Fearful     UMETA(DisplayName = "Fearful")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_NPCPersonality
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    FString NPCName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    ENarr_NPCMood CurrentMood;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float TrustLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float FearLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    TArray<FString> KnownConversations;

    FNarr_NPCPersonality()
    {
        NPCName = TEXT("Unknown");
        CurrentMood = ENarr_NPCMood::Neutral;
        TrustLevel = 0.5f;
        FearLevel = 0.3f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNarr_NPCInteractionComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNarr_NPCInteractionComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "NPC Interaction")
    void InitiateInteraction(AActor* InteractingActor);

    UFUNCTION(BlueprintCallable, Category = "NPC Interaction")
    void EndInteraction();

    UFUNCTION(BlueprintCallable, Category = "NPC Interaction")
    bool CanInteract(AActor* InteractingActor) const;

    UFUNCTION(BlueprintCallable, Category = "NPC Interaction")
    FString GetGreetingDialogue() const;

    UFUNCTION(BlueprintCallable, Category = "NPC Interaction")
    void ModifyTrust(float TrustChange);

    UFUNCTION(BlueprintCallable, Category = "NPC Interaction")
    void ModifyFear(float FearChange);

    UFUNCTION(BlueprintCallable, Category = "NPC Interaction")
    ENarr_NPCMood GetCurrentMood() const;

    UFUNCTION(BlueprintCallable, Category = "NPC Interaction")
    void SetPersonality(const FNarr_NPCPersonality& NewPersonality);

    UFUNCTION(BlueprintCallable, Category = "NPC Interaction")
    FNarr_NPCPersonality GetPersonality() const;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Settings")
    FNarr_NPCPersonality Personality;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Settings")
    float InteractionRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Settings")
    bool bCanInteract;

    UPROPERTY()
    AActor* CurrentInteractingActor;

    UPROPERTY()
    bool bIsInteracting;

private:
    void UpdateMoodBasedOnStats();
    FString GenerateContextualDialogue() const;
};