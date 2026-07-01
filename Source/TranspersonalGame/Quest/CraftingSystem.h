#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CraftingSystem.generated.h"

// ─── Resource Types ───────────────────────────────────────────────────────────
UENUM(BlueprintType)
enum class EQuest_ResourceType : uint8
{
    None      UMETA(DisplayName = "None"),
    Stone     UMETA(DisplayName = "Stone"),
    Stick     UMETA(DisplayName = "Stick"),
    Leaf      UMETA(DisplayName = "Leaf"),
    Bone      UMETA(DisplayName = "Bone"),
    Hide      UMETA(DisplayName = "Hide"),
    Flint     UMETA(DisplayName = "Flint"),
    Vine      UMETA(DisplayName = "Vine")
};

// ─── Crafting Ingredient ──────────────────────────────────────────────────────
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_CraftingIngredient
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    EQuest_ResourceType ResourceType = EQuest_ResourceType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    int32 Quantity = 1;
};

// ─── Crafting Recipe ──────────────────────────────────────────────────────────
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_CraftingRecipe
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    FName RecipeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    TArray<FQuest_CraftingIngredient> Ingredients;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    FName OutputItemID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    float CraftTimeSeconds = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    bool bUnlockedByDefault = true;
};

// ─── Inventory Slot ───────────────────────────────────────────────────────────
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_InventorySlot
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    EQuest_ResourceType ResourceType = EQuest_ResourceType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    int32 Count = 0;
};

// ─── Craft Result ─────────────────────────────────────────────────────────────
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_CraftResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Crafting")
    bool bSuccess = false;

    UPROPERTY(BlueprintReadOnly, Category = "Crafting")
    FName OutputItemID;

    UPROPERTY(BlueprintReadOnly, Category = "Crafting")
    FText FailReason;
};

// ─── Crafting System Component ────────────────────────────────────────────────
UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCraftingSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UCraftingSystem();

    virtual void BeginPlay() override;

    // ── Inventory ──────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Crafting|Inventory")
    void AddResource(EQuest_ResourceType ResourceType, int32 Amount = 1);

    UFUNCTION(BlueprintCallable, Category = "Crafting|Inventory")
    bool RemoveResource(EQuest_ResourceType ResourceType, int32 Amount = 1);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Crafting|Inventory")
    int32 GetResourceCount(EQuest_ResourceType ResourceType) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Crafting|Inventory")
    TArray<FQuest_InventorySlot> GetInventory() const;

    // ── Recipes ────────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Crafting|Recipes")
    void InitializeDefaultRecipes();

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Crafting|Recipes")
    TArray<FQuest_CraftingRecipe> GetAvailableRecipes() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Crafting|Recipes")
    bool CanCraft(FName RecipeID) const;

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    FQuest_CraftResult TryCraft(FName RecipeID);

    // ── UI Trigger ─────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Crafting|UI")
    void OpenCraftingMenu();

    UFUNCTION(BlueprintCallable, Category = "Crafting|UI")
    void CloseCraftingMenu();

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Crafting|UI")
    bool IsCraftingMenuOpen() const { return bCraftingMenuOpen; }

    // ── Delegates ──────────────────────────────────────────────────────────
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemCrafted, FName, ItemID);
    UPROPERTY(BlueprintAssignable, Category = "Crafting|Events")
    FOnItemCrafted OnItemCrafted;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInventoryChanged, EQuest_ResourceType, ChangedResource);
    UPROPERTY(BlueprintAssignable, Category = "Crafting|Events")
    FOnInventoryChanged OnInventoryChanged;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCraftingMenuToggled, bool, bIsOpen);
    UPROPERTY(BlueprintAssignable, Category = "Crafting|Events")
    FOnCraftingMenuToggled OnCraftingMenuToggled;

private:
    UPROPERTY(VisibleAnywhere, Category = "Crafting|State")
    TArray<FQuest_InventorySlot> Inventory;

    UPROPERTY(VisibleAnywhere, Category = "Crafting|State")
    TArray<FQuest_CraftingRecipe> Recipes;

    UPROPERTY(VisibleAnywhere, Category = "Crafting|State")
    bool bCraftingMenuOpen = false;

    // Internal helpers
    FQuest_InventorySlot* FindSlot(EQuest_ResourceType ResourceType);
    const FQuest_InventorySlot* FindSlotConst(EQuest_ResourceType ResourceType) const;
    const FQuest_CraftingRecipe* FindRecipe(FName RecipeID) const;
};
