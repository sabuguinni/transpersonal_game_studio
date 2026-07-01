#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CraftingSystem.generated.h"

// ============================================================
// Enums — must be at global scope (UE5 UHT rule)
// ============================================================

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
    Water       UMETA(DisplayName = "Water")
};

UENUM(BlueprintType)
enum class EQuest_CraftResult : uint8
{
    Success         UMETA(DisplayName = "Success"),
    MissingIngredients UMETA(DisplayName = "MissingIngredients"),
    InvalidRecipe   UMETA(DisplayName = "InvalidRecipe"),
    InventoryFull   UMETA(DisplayName = "InventoryFull")
};

// ============================================================
// Structs — must be at global scope (UE5 UHT rule)
// ============================================================

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
    FName RecipeID = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    TArray<FQuest_CraftingIngredient> Ingredients;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    FName OutputItemID = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    float CraftTimeSeconds = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    bool bRequiresCampfire = false;
};

USTRUCT(BlueprintType)
struct FQuest_InventorySlot
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    EQuest_ResourceType ResourceType = EQuest_ResourceType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    int32 Count = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    int32 MaxStack = 10;
};

USTRUCT(BlueprintType)
struct FQuest_CraftResultData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Crafting")
    EQuest_CraftResult Result = EQuest_CraftResult::InvalidRecipe;

    UPROPERTY(BlueprintReadOnly, Category = "Crafting")
    FName OutputItemID = NAME_None;

    UPROPERTY(BlueprintReadOnly, Category = "Crafting")
    FText ResultMessage;
};

// ============================================================
// UCraftingSystem — ActorComponent
// ============================================================

UCLASS(ClassGroup = (Quest), meta = (BlueprintSpawnableComponent), BlueprintType)
class TRANSPERSONALGAME_API UCraftingSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UCraftingSystem();

    // Lifecycle
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ---- Inventory ----

    UFUNCTION(BlueprintCallable, Category = "Crafting|Inventory")
    bool AddResource(EQuest_ResourceType ResourceType, int32 Amount = 1);

    UFUNCTION(BlueprintCallable, Category = "Crafting|Inventory")
    bool RemoveResource(EQuest_ResourceType ResourceType, int32 Amount = 1);

    UFUNCTION(BlueprintCallable, Category = "Crafting|Inventory")
    int32 GetResourceCount(EQuest_ResourceType ResourceType) const;

    UFUNCTION(BlueprintCallable, Category = "Crafting|Inventory")
    bool HasResources(const TArray<FQuest_CraftingIngredient>& Ingredients) const;

    // ---- Recipes ----

    UFUNCTION(BlueprintCallable, Category = "Crafting|Recipes")
    void InitializeDefaultRecipes();

    UFUNCTION(BlueprintCallable, Category = "Crafting|Recipes")
    TArray<FQuest_CraftingRecipe> GetAvailableRecipes() const;

    UFUNCTION(BlueprintCallable, Category = "Crafting|Recipes")
    bool GetRecipeByID(FName RecipeID, FQuest_CraftingRecipe& OutRecipe) const;

    // ---- Crafting ----

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    FQuest_CraftResultData TryCraft(FName RecipeID);

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    void OpenCraftingMenu();

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    void CloseCraftingMenu();

    UFUNCTION(BlueprintPure, Category = "Crafting")
    bool IsCraftingMenuOpen() const { return bCraftingMenuOpen; }

    // ---- Delegates ----

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCraftComplete, FQuest_CraftResultData, Result);
    UPROPERTY(BlueprintAssignable, Category = "Crafting|Events")
    FOnCraftComplete OnCraftComplete;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInventoryChanged, EQuest_ResourceType, ResourceType, int32, NewCount);
    UPROPERTY(BlueprintAssignable, Category = "Crafting|Events")
    FOnInventoryChanged OnInventoryChanged;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneSig(FOnCraftingMenuToggled, bool, bIsOpen);
    UPROPERTY(BlueprintAssignable, Category = "Crafting|Events")
    FOnCraftingMenuToggled OnCraftingMenuToggled;

    // ---- Properties ----

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    int32 MaxInventorySlots = 20;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    bool bNearCampfire = false;

private:
    UPROPERTY()
    TArray<FQuest_CraftingRecipe> Recipes;

    UPROPERTY()
    TArray<FQuest_InventorySlot> Inventory;

    bool bCraftingMenuOpen = false;

    int32 FindInventorySlot(EQuest_ResourceType ResourceType) const;
};
