#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "../SharedTypes.h"
#include "Quest_CraftingQuestManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_CraftingRecipe
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    FString RecipeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    TArray<FString> RequiredMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    TArray<int32> MaterialQuantities;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    FString ResultItem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    float CraftingTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    int32 ExperienceReward;

    FQuest_CraftingRecipe()
    {
        RecipeName = TEXT("");
        CraftingTime = 5.0f;
        ExperienceReward = 10;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_CraftingObjective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString ObjectiveID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString ItemToCraft;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 QuantityRequired;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 CurrentProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsCompleted;

    FQuest_CraftingObjective()
    {
        ObjectiveID = TEXT("");
        ItemToCraft = TEXT("");
        QuantityRequired = 1;
        CurrentProgress = 0;
        bIsCompleted = false;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UQuest_CraftingQuestManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuest_CraftingQuestManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting System")
    TArray<FQuest_CraftingRecipe> AvailableRecipes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    TArray<FQuest_CraftingObjective> ActiveCraftingObjectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    TMap<FString, int32> PlayerInventory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    bool bCraftingMenuOpen;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Experience")
    int32 CraftingExperience;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Experience")
    int32 CraftingLevel;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    void InitializeCraftingSystem();

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    void AddBasicRecipes();

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    bool CanCraftItem(const FString& ItemName);

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    bool CraftItem(const FString& ItemName);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void AddItemToInventory(const FString& ItemName, int32 Quantity);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    int32 GetItemQuantity(const FString& ItemName);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool RemoveItemFromInventory(const FString& ItemName, int32 Quantity);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void AddCraftingObjective(const FString& ObjectiveID, const FString& ItemName, int32 Quantity);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void UpdateCraftingProgress(const FString& ItemName, int32 Quantity);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool IsObjectiveCompleted(const FString& ObjectiveID);

    UFUNCTION(BlueprintCallable, Category = "UI")
    void ToggleCraftingMenu();

    UFUNCTION(BlueprintCallable, Category = "Experience")
    void AddCraftingExperience(int32 Experience);

    UFUNCTION(BlueprintCallable, Category = "Experience")
    void CheckLevelUp();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DebugPrintInventory();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DebugPrintRecipes();
};