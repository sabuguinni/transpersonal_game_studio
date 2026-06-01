#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "Quest_CraftingSystem.generated.h"

UENUM(BlueprintType)
enum class EQuest_ResourceType : uint8
{
    None        UMETA(DisplayName = "None"),
    Rock        UMETA(DisplayName = "Rock"),
    Stick       UMETA(DisplayName = "Stick"),
    Leaf        UMETA(DisplayName = "Leaf"),
    Fiber       UMETA(DisplayName = "Fiber"),
    Bone        UMETA(DisplayName = "Bone"),
    Hide        UMETA(DisplayName = "Hide"),
    Stone       UMETA(DisplayName = "Stone"),
    Wood        UMETA(DisplayName = "Wood")
};

USTRUCT(BlueprintType)
struct FQuest_CraftingRecipe
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
    FString RecipeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
    TMap<EQuest_ResourceType, int32> RequiredResources;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
    FString ResultItemName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
    int32 CraftingTime;

    FQuest_CraftingRecipe()
    {
        RecipeName = TEXT("");
        ResultItemName = TEXT("");
        CraftingTime = 5;
    }
};

USTRUCT(BlueprintType)
struct FQuest_PlayerInventory
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    TMap<EQuest_ResourceType, int32> Resources;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    TArray<FString> CraftedItems;

    FQuest_PlayerInventory()
    {
        Resources.Add(EQuest_ResourceType::Rock, 0);
        Resources.Add(EQuest_ResourceType::Stick, 0);
        Resources.Add(EQuest_ResourceType::Leaf, 0);
        Resources.Add(EQuest_ResourceType::Fiber, 0);
        Resources.Add(EQuest_ResourceType::Bone, 0);
        Resources.Add(EQuest_ResourceType::Hide, 0);
        Resources.Add(EQuest_ResourceType::Stone, 0);
        Resources.Add(EQuest_ResourceType::Wood, 0);
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UQuest_CraftingSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuest_CraftingSystem();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    TArray<FQuest_CraftingRecipe> AvailableRecipes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    FQuest_PlayerInventory PlayerInventory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    bool bCraftingUIOpen;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    float CraftingRange;

public:
    UFUNCTION(BlueprintCallable, Category = "Crafting")
    void InitializeDefaultRecipes();

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    bool CanCraftItem(const FString& RecipeName);

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    bool CraftItem(const FString& RecipeName);

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    void AddResource(EQuest_ResourceType ResourceType, int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    int32 GetResourceCount(EQuest_ResourceType ResourceType);

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    void ToggleCraftingUI();

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    TArray<FQuest_CraftingRecipe> GetAvailableRecipes();

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    FQuest_PlayerInventory GetPlayerInventory();

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    void SpawnResourcePickups();

private:
    void CreateStoneAxeRecipe();
    void CreateCampfireRecipe();
    void CreateWaterContainerRecipe();
    void CreateSpearRecipe();
    void CreateShelterRecipe();
};