#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/TriggerBox.h"
#include "SharedTypes.h"
#include "Char_InteractionSystem.generated.h"

UENUM(BlueprintType)
enum class EChar_InteractionType : uint8
{
    None = 0,
    Dialogue,
    Trade,
    Crafting,
    Gathering,
    Social,
    Combat,
    Ritual
};

USTRUCT(BlueprintType)
struct FChar_InteractionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    EChar_InteractionType InteractionType = EChar_InteractionType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    FString InteractionName = TEXT("");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    FString InteractionDescription = TEXT("");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    float InteractionRange = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    float InteractionDuration = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    bool bRequiresLineOfSight = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    bool bIsRepeatable = true;

    FChar_InteractionData()
    {
        InteractionType = EChar_InteractionType::None;
        InteractionName = TEXT("");
        InteractionDescription = TEXT("");
        InteractionRange = 200.0f;
        InteractionDuration = 2.0f;
        bRequiresLineOfSight = true;
        bIsRepeatable = true;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UChar_InteractionSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UChar_InteractionSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Interaction Management
    UFUNCTION(BlueprintCallable, Category = "Character Interaction")
    void RegisterInteraction(const FChar_InteractionData& InteractionData);

    UFUNCTION(BlueprintCallable, Category = "Character Interaction")
    void UnregisterInteraction(const FString& InteractionName);

    UFUNCTION(BlueprintCallable, Category = "Character Interaction")
    bool StartInteraction(AActor* InteractingActor, const FString& InteractionName);

    UFUNCTION(BlueprintCallable, Category = "Character Interaction")
    void EndInteraction(const FString& InteractionName);

    UFUNCTION(BlueprintCallable, Category = "Character Interaction")
    TArray<FChar_InteractionData> GetAvailableInteractions(AActor* InteractingActor) const;

    // Proximity Detection
    UFUNCTION(BlueprintCallable, Category = "Character Interaction")
    void CheckProximityInteractions();

    UFUNCTION(BlueprintCallable, Category = "Character Interaction")
    bool IsActorInInteractionRange(AActor* OtherActor, float Range) const;

    // Social Interactions
    UFUNCTION(BlueprintCallable, Category = "Character Interaction")
    void InitiateSocialInteraction(AActor* TargetActor, EChar_InteractionType SocialType);

    UFUNCTION(BlueprintCallable, Category = "Character Interaction")
    void ProcessTribalGathering(const TArray<AActor*>& ParticipatingActors);

    UFUNCTION(BlueprintCallable, Category = "Character Interaction")
    void HandleCraftingInteraction(AActor* CraftingStation);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction Settings")
    TArray<FChar_InteractionData> RegisteredInteractions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction Settings")
    float ProximityCheckInterval = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction Settings")
    float MaxInteractionRange = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction Settings")
    bool bEnableAutoProximityCheck = true;

    UPROPERTY(BlueprintReadOnly, Category = "Interaction State")
    TMap<FString, bool> ActiveInteractions;

    UPROPERTY(BlueprintReadOnly, Category = "Interaction State")
    TArray<AActor*> NearbyInteractableActors;

private:
    float ProximityCheckTimer = 0.0f;
    
    void UpdateNearbyActors();
    bool ValidateInteractionConditions(const FChar_InteractionData& InteractionData, AActor* InteractingActor) const;
    void ProcessInteractionEffects(const FChar_InteractionData& InteractionData, AActor* InteractingActor);
};