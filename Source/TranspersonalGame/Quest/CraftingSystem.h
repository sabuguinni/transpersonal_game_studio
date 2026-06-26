#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CraftingSystem.generated.h"

// --- Enums ---

UENUM(BlueprintType)
enum class EQuest_ResourceType : uint8
{
    None        UMETA(DisplayName = "None"),
    Rock        UMETA(DisplayName = "Rock"),
    Stick       UMETA(DisplayName = "Stick"),
    Leaf        UMETA(DisplayName = "Leaf"),
    Bone        UMETA(DisplayName = "Bone"),
    Hide        UMETA(DisplayName = "Hide"),
};

UENUM(BlueprintType)
enum class EQuest_CraftedItemType : uint8
{
    None            UMETA(DisplayName = "None"),
    StoneAxe        UMETA(DisplayName = "Stone Axe"),
    Campfire        UMETA(DisplayName = "Campfire"),
    WaterContainer  UMETA(DisplayName = "Water Container"),
    Spear           UMETA(DisplayName = "Spear"),
};

// --- Structs ---

USTRUCT(BlueprintType)
struct FQuest_ResourceIngredient
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    EQuest_ResourceType ResourceType = EQuest_ResourceType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    int32 Quantity = 1;
};

USTRUCT(BlueprintType)
struct FQuest_CraftingRecipe
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    EQuest_CraftedItemType OutputItem = EQuest_CraftedItemType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    FString RecipeName = TEXT("Unknown Recipe");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    TArray<FQuest_ResourceIngredient> Ingredients;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    float CraftingTimeSec = 2.0f;
};

USTRUCT(BlueprintType)
struct FQuest_PlayerInventory
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    int32 Rocks = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    int32 Sticks = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    int32 Leaves = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    int32 Bones = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    int32 Hides = 0;
};

// --- CraftingSystem Actor ---

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACraftingSystem : public AActor
{
    GENERATED_BODY()

public:
    ACraftingSystem();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // All available recipes
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    TArray<FQuest_CraftingRecipe> Recipes;

    // Player's current inventory
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    FQuest_PlayerInventory PlayerInventory;

    // Is crafting menu open
    UPROPERTY(BlueprintReadOnly, Category = "Crafting")
    bool bCraftingMenuOpen = false;

    // Currently selected recipe index
    UPROPERTY(BlueprintReadWrite, Category = "Crafting")
    int32 SelectedRecipeIndex = 0;

    // Open/close crafting menu (bound to C key)
    UFUNCTION(BlueprintCallable, Category = "Crafting")
    void ToggleCraftingMenu();

    // Attempt to craft the selected recipe
    UFUNCTION(BlueprintCallable, Category = "Crafting")
    bool TryCraftItem(int32 RecipeIndex);

    // Check if player has enough resources for a recipe
    UFUNCTION(BlueprintCallable, Category = "Crafting")
    bool CanCraftRecipe(int32 RecipeIndex) const;

    // Add resource to inventory (called when player picks up a resource)
    UFUNCTION(BlueprintCallable, Category = "Crafting")
    void AddResource(EQuest_ResourceType ResourceType, int32 Amount = 1);

    // Get resource count from inventory
    UFUNCTION(BlueprintCallable, Category = "Crafting")
    int32 GetResourceCount(EQuest_ResourceType ResourceType) const;

    // Consume resources for crafting
    UFUNCTION(BlueprintCallable, Category = "Crafting")
    bool ConsumeResources(const FQuest_CraftingRecipe& Recipe);

    // Event called when item is crafted successfully
    UFUNCTION(BlueprintImplementableEvent, Category = "Crafting")
    void OnItemCrafted(EQuest_CraftedItemType CraftedItem);

    // Event called when crafting fails (not enough resources)
    UFUNCTION(BlueprintImplementableEvent, Category = "Crafting")
    void OnCraftingFailed(int32 RecipeIndex);

private:
    void InitializeDefaultRecipes();
};
