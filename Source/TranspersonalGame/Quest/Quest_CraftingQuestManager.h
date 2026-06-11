#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "../SharedTypes.h"
#include "Quest_CraftingQuestManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_CraftingRecipe
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe")
    FString RecipeName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe")
    TArray<FString> RequiredMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe")
    TArray<int32> RequiredQuantities;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe")
    FString CraftedItem;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe")
    float CraftingTime;

    FQuest_CraftingRecipe()
    {
        RecipeName = TEXT("Unknown Recipe");
        CraftedItem = TEXT("Unknown Item");
        CraftingTime = 5.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_CraftingObjective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objective")
    FString ObjectiveID;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objective")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objective")
    FString RequiredItem;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objective")
    int32 RequiredQuantity;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objective")
    bool bIsCompleted;

    FQuest_CraftingObjective()
    {
        ObjectiveID = TEXT("craft_001");
        Description = TEXT("Craft basic tool");
        RequiredItem = TEXT("Stone Axe");
        RequiredQuantity = 1;
        bIsCompleted = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_CraftingQuestManager : public AActor
{
    GENERATED_BODY()

public:
    AQuest_CraftingQuestManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting System")
    TArray<FQuest_CraftingRecipe> AvailableRecipes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    TArray<FQuest_CraftingObjective> ActiveObjectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Inventory")
    TMap<FString, int32> PlayerInventory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting System")
    bool bCraftingMenuOpen;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting System")
    float CraftingProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting System")
    FString CurrentlyCrafting;

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    void InitializeBasicRecipes();

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    bool CanCraftItem(const FString& ItemName);

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    bool StartCrafting(const FString& ItemName);

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    void UpdateCraftingProgress(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    void CompleteCrafting();

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void AddItemToInventory(const FString& ItemName, int32 Quantity);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    int32 GetItemQuantity(const FString& ItemName);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool RemoveItemFromInventory(const FString& ItemName, int32 Quantity);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void CheckCraftingObjectives();

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void CompleteObjective(const FString& ObjectiveID);

    UFUNCTION(BlueprintCallable, Category = "UI")
    void ToggleCraftingMenu();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DebugPrintInventory();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DebugPrintRecipes();
};