#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CraftingSystem.generated.h"

// ─── Enums (global scope — RULE 1) ───────────────────────────────────────────

UENUM(BlueprintType)
enum class EQuest_ResourceType : uint8
{
    None        UMETA(DisplayName = "None"),
    Stone       UMETA(DisplayName = "Stone"),
    Stick       UMETA(DisplayName = "Stick"),
    Leaf        UMETA(DisplayName = "Leaf"),
    Bone        UMETA(DisplayName = "Bone"),
    Hide        UMETA(DisplayName = "Hide"),
    Flint       UMETA(DisplayName = "Flint"),
    Vine        UMETA(DisplayName = "Vine"),
    Feather     UMETA(DisplayName = "Feather")
};

UENUM(BlueprintType)
enum class EQuest_CraftResult : uint8
{
    Success             UMETA(DisplayName = "Success"),
    InsufficientItems   UMETA(DisplayName = "InsufficientItems"),
    RecipeNotFound      UMETA(DisplayName = "RecipeNotFound"),
    InventoryFull       UMETA(DisplayName = "InventoryFull")
};

// ─── Structs (global scope — RULE 1) ─────────────────────────────────────────

USTRUCT(BlueprintType)
struct FQuest_CraftingIngredient
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
    FName RecipeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    TArray<FQuest_CraftingIngredient> Ingredients;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    FName OutputItemID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    int32 OutputQuantity = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    float CraftTimeSeconds = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    FText Description;
};

USTRUCT(BlueprintType)
struct FQuest_InventorySlot
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    EQuest_ResourceType ResourceType = EQuest_ResourceType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    int32 Quantity = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    FName ItemID;
};

USTRUCT(BlueprintType)
struct FQuest_CraftResultData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Crafting")
    EQuest_CraftResult Result = EQuest_CraftResult::RecipeNotFound;

    UPROPERTY(BlueprintReadOnly, Category = "Crafting")
    FName OutputItemID;

    UPROPERTY(BlueprintReadOnly, Category = "Crafting")
    int32 OutputQuantity = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Crafting")
    FText ResultMessage;
};

// ─── Main Component ───────────────────────────────────────────────────────────

UCLASS(ClassGroup = (Quest), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCraftingSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UCraftingSystem();

    // Lifecycle
    virtual void BeginPlay() override;

    // ── Recipe Management ──────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    void InitializeDefaultRecipes();

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    bool AddRecipe(const FQuest_CraftingRecipe& Recipe);

    UFUNCTION(BlueprintPure, Category = "Crafting")
    bool GetRecipe(FName RecipeID, FQuest_CraftingRecipe& OutRecipe) const;

    UFUNCTION(BlueprintPure, Category = "Crafting")
    TArray<FQuest_CraftingRecipe> GetAllRecipes() const;

    UFUNCTION(BlueprintPure, Category = "Crafting")
    TArray<FQuest_CraftingRecipe> GetAvailableRecipes() const;

    // ── Inventory ──────────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool AddResource(EQuest_ResourceType Type, int32 Amount = 1);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool RemoveResource(EQuest_ResourceType Type, int32 Amount = 1);

    UFUNCTION(BlueprintPure, Category = "Inventory")
    int32 GetResourceCount(EQuest_ResourceType Type) const;

    UFUNCTION(BlueprintPure, Category = "Inventory")
    TArray<FQuest_InventorySlot> GetInventory() const;

    UFUNCTION(BlueprintPure, Category = "Inventory")
    bool HasIngredients(FName RecipeID) const;

    // ── Crafting ───────────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    FQuest_CraftResultData TryCraft(FName RecipeID);

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    void OpenCraftingMenu();

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    void CloseCraftingMenu();

    UFUNCTION(BlueprintPure, Category = "Crafting")
    bool IsCraftingMenuOpen() const { return bCraftingMenuOpen; }

    // ── Delegates ─────────────────────────────────────────────────────────

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCraftSuccess, FQuest_CraftResultData, Result);
    UPROPERTY(BlueprintAssignable, Category = "Crafting|Events")
    FOnCraftSuccess OnCraftSuccess;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCraftFailed, EQuest_CraftResult, Reason);
    UPROPERTY(BlueprintAssignable, Category = "Crafting|Events")
    FOnCraftFailed OnCraftFailed;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnResourceChanged, EQuest_ResourceType, Type, int32, NewCount);
    UPROPERTY(BlueprintAssignable, Category = "Inventory|Events")
    FOnResourceChanged OnResourceChanged;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneBoolParam(FOnCraftingMenuToggled, bool, bIsOpen);
    UPROPERTY(BlueprintAssignable, Category = "Crafting|Events")
    FOnCraftingMenuToggled OnCraftingMenuToggled;

    // ── Config ────────────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting|Config")
    int32 MaxInventorySlots = 20;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting|Config")
    int32 MaxStackSize = 99;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting|Config")
    bool bRequireProximityToCampfire = false;

private:
    UPROPERTY()
    TMap<FName, FQuest_CraftingRecipe> RecipeMap;

    UPROPERTY()
    TArray<FQuest_InventorySlot> Inventory;

    UPROPERTY()
    bool bCraftingMenuOpen = false;

    // Internal helpers
    int32 FindInventorySlot(EQuest_ResourceType Type) const;
    bool ConsumeIngredients(const FQuest_CraftingRecipe& Recipe);
};
