#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CraftingSystem.generated.h"

// ── Resource types available in the world ────────────────────────────────────
UENUM(BlueprintType)
enum class EQuest_ResourceType : uint8
{
    None        UMETA(DisplayName = "None"),
    Stone       UMETA(DisplayName = "Stone"),
    Stick       UMETA(DisplayName = "Stick"),
    Leaf        UMETA(DisplayName = "Leaf"),
    Bone        UMETA(DisplayName = "Bone"),
    Hide        UMETA(DisplayName = "Hide"),
};

// ── Single ingredient in a recipe ────────────────────────────────────────────
USTRUCT(BlueprintType)
struct FQuest_CraftingIngredient
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    EQuest_ResourceType ResourceType = EQuest_ResourceType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    int32 Quantity = 1;
};

// ── A complete crafting recipe ────────────────────────────────────────────────
USTRUCT(BlueprintType)
struct FQuest_CraftingRecipe
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    FName RecipeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    TArray<FQuest_CraftingIngredient> Ingredients;

    /** Crafting time in seconds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    float CraftDurationSeconds = 2.0f;

    /** Whether this recipe unlocks a quest objective */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    bool bUnlocksQuestObjective = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    FName LinkedQuestID;
};

// ── Player inventory slot ─────────────────────────────────────────────────────
USTRUCT(BlueprintType)
struct FQuest_InventorySlot
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    EQuest_ResourceType ResourceType = EQuest_ResourceType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    int32 Count = 0;
};

// ── Crafting result ───────────────────────────────────────────────────────────
USTRUCT(BlueprintType)
struct FQuest_CraftResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Crafting")
    bool bSuccess = false;

    UPROPERTY(BlueprintReadOnly, Category = "Crafting")
    FName RecipeID;

    UPROPERTY(BlueprintReadOnly, Category = "Crafting")
    FText ResultMessage;
};

// ─────────────────────────────────────────────────────────────────────────────
// UCraftingSystem — ActorComponent attached to the player character
// Manages inventory, recipes, and crafting logic.
// ─────────────────────────────────────────────────────────────────────────────
UCLASS(ClassGroup = (Quest), meta = (BlueprintSpawnableComponent), DisplayName = "Crafting System")
class TRANSPERSONALGAME_API UCraftingSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UCraftingSystem();

    virtual void BeginPlay() override;

    // ── Inventory ─────────────────────────────────────────────────────────────

    /** Add a resource to the player's inventory */
    UFUNCTION(BlueprintCallable, Category = "Crafting|Inventory")
    void AddResource(EQuest_ResourceType Type, int32 Amount = 1);

    /** Remove a resource from the player's inventory */
    UFUNCTION(BlueprintCallable, Category = "Crafting|Inventory")
    bool RemoveResource(EQuest_ResourceType Type, int32 Amount = 1);

    /** Get current count of a resource type */
    UFUNCTION(BlueprintPure, Category = "Crafting|Inventory")
    int32 GetResourceCount(EQuest_ResourceType Type) const;

    /** Get full inventory snapshot */
    UFUNCTION(BlueprintPure, Category = "Crafting|Inventory")
    TArray<FQuest_InventorySlot> GetInventory() const;

    // ── Recipes ───────────────────────────────────────────────────────────────

    /** Attempt to craft a recipe by ID. Returns result struct. */
    UFUNCTION(BlueprintCallable, Category = "Crafting")
    FQuest_CraftResult TryCraft(FName RecipeID);

    /** Check if player has ingredients for a recipe */
    UFUNCTION(BlueprintPure, Category = "Crafting")
    bool CanCraft(FName RecipeID) const;

    /** Get all available recipes */
    UFUNCTION(BlueprintPure, Category = "Crafting")
    TArray<FQuest_CraftingRecipe> GetAllRecipes() const;

    /** Get a single recipe by ID */
    UFUNCTION(BlueprintPure, Category = "Crafting")
    bool GetRecipe(FName RecipeID, FQuest_CraftingRecipe& OutRecipe) const;

    // ── UI trigger ────────────────────────────────────────────────────────────

    /** Called when player presses C — toggles crafting menu state */
    UFUNCTION(BlueprintCallable, Category = "Crafting|UI")
    void ToggleCraftingMenu();

    UFUNCTION(BlueprintPure, Category = "Crafting|UI")
    bool IsCraftingMenuOpen() const { return bCraftingMenuOpen; }

    // ── Delegates ─────────────────────────────────────────────────────────────

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCraftSuccess, FName, RecipeID);
    UPROPERTY(BlueprintAssignable, Category = "Crafting|Events")
    FOnCraftSuccess OnCraftSuccess;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInventoryChanged, EQuest_ResourceType, ChangedType);
    UPROPERTY(BlueprintAssignable, Category = "Crafting|Events")
    FOnInventoryChanged OnInventoryChanged;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCraftingMenuToggled, bool, bIsOpen);
    UPROPERTY(BlueprintAssignable, Category = "Crafting|Events")
    FOnCraftingMenuToggled OnCraftingMenuToggled;

private:
    /** Internal inventory map: resource type → count */
    TMap<EQuest_ResourceType, int32> InventoryMap;

    /** All registered recipes */
    TArray<FQuest_CraftingRecipe> Recipes;

    bool bCraftingMenuOpen = false;

    /** Populate the default recipes (Stone Axe, Campfire, Water Container) */
    void RegisterDefaultRecipes();

    /** Deduct ingredients from inventory for a recipe (assumes CanCraft is true) */
    void ConsumeIngredients(const FQuest_CraftingRecipe& Recipe);
};
