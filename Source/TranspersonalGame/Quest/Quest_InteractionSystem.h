#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/TriggerVolume.h"
#include "Quest_InteractionSystem.generated.h"

UENUM(BlueprintType)
enum class EQuest_InteractionType : uint8
{
    None = 0,
    Gather = 1,
    Hunt = 2,
    Craft = 3,
    Explore = 4,
    Defend = 5,
    Escort = 6,
    Dialogue = 7
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_InteractionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Interaction")
    FString InteractionName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Interaction")
    EQuest_InteractionType InteractionType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Interaction")
    FText InteractionPrompt;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Interaction")
    float InteractionRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Interaction")
    bool bRequiresHold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Interaction")
    float HoldDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Interaction")
    bool bIsRepeatable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Interaction")
    int32 MaxUses;

    FQuest_InteractionData()
    {
        InteractionName = TEXT("Default Interaction");
        InteractionType = EQuest_InteractionType::None;
        InteractionPrompt = FText::FromString(TEXT("Press E to interact"));
        InteractionRange = 200.0f;
        bRequiresHold = false;
        HoldDuration = 1.0f;
        bIsRepeatable = true;
        MaxUses = -1;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_InteractionActor : public AActor
{
    GENERATED_BODY()

public:
    AQuest_InteractionActor();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* InteractionMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBoxComponent* InteractionTrigger;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Interaction")
    FQuest_InteractionData InteractionData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Interaction")
    bool bIsActive;

    UPROPERTY(BlueprintReadOnly, Category = "Quest Interaction")
    int32 CurrentUses;

    UPROPERTY(BlueprintReadOnly, Category = "Quest Interaction")
    bool bPlayerInRange;

    UPROPERTY(BlueprintReadOnly, Category = "Quest Interaction")
    float CurrentHoldTime;

    UFUNCTION(BlueprintCallable, Category = "Quest Interaction")
    bool CanInteract() const;

    UFUNCTION(BlueprintCallable, Category = "Quest Interaction")
    void StartInteraction();

    UFUNCTION(BlueprintCallable, Category = "Quest Interaction")
    void CompleteInteraction();

    UFUNCTION(BlueprintCallable, Category = "Quest Interaction")
    void CancelInteraction();

    UFUNCTION(BlueprintCallable, Category = "Quest Interaction")
    void SetInteractionActive(bool bActive);

    UFUNCTION(BlueprintImplementableEvent, Category = "Quest Interaction")
    void OnInteractionStarted();

    UFUNCTION(BlueprintImplementableEvent, Category = "Quest Interaction")
    void OnInteractionCompleted();

    UFUNCTION(BlueprintImplementableEvent, Category = "Quest Interaction")
    void OnInteractionCancelled();

    UFUNCTION(BlueprintImplementableEvent, Category = "Quest Interaction")
    void OnPlayerEnterRange();

    UFUNCTION(BlueprintImplementableEvent, Category = "Quest Interaction")
    void OnPlayerExitRange();

protected:
    UFUNCTION()
    void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex);

private:
    bool bIsInteracting;
    AActor* InteractingPlayer;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_GatherableResource : public AQuest_InteractionActor
{
    GENERATED_BODY()

public:
    AQuest_GatherableResource();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    FString ResourceType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    int32 ResourceAmount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    bool bDestroyOnGather;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    float RespawnTime;

    UFUNCTION(BlueprintCallable, Category = "Resource")
    void GatherResource(int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "Resource")
    void RespawnResource();

protected:
    virtual void BeginPlay() override;

private:
    FTimerHandle RespawnTimerHandle;
    int32 OriginalResourceAmount;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_CraftingStation : public AQuest_InteractionActor
{
    GENERATED_BODY()

public:
    AQuest_CraftingStation();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    TArray<FString> AvailableRecipes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    bool bRequiresTools;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    TArray<FString> RequiredTools;

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    bool CanCraftRecipe(const FString& RecipeName) const;

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    void StartCrafting(const FString& RecipeName);

    UFUNCTION(BlueprintImplementableEvent, Category = "Crafting")
    void OnCraftingStarted(const FString& RecipeName);

    UFUNCTION(BlueprintImplementableEvent, Category = "Crafting")
    void OnCraftingCompleted(const FString& RecipeName);
};

#include "Quest_InteractionSystem.generated.h"