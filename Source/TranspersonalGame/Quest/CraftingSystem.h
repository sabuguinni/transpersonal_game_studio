#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CraftingSystem.generated.h"

// ─── Enums ───────────────────────────────────────────────────────────────────

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
    Water       UMETA(DisplayName = "Water"),
    Meat        UMETA(DisplayName = "Meat")
};

UENUM(BlueprintType)
enum class EQuest_CraftResult : uint8
{
    Success         UMETA(DisplayName = "Success"),
    MissingIngredients UMETA(DisplayName = "MissingIngredients"),
    InventoryFull   UMETA(DisplayName = "InventoryFull"),
    InvalidRecipe   UMETA(DisplayName = "InvalidRecipe")
};

UENUM(BlueprintType)
enum class EQuest_CraftedItem : uint8
{
    None            UMETA(DisplayName = "None"),
    StoneAxe        UMETA(DisplayName = "StoneAxe"),
    Campfire        UMETA(DisplayName = "Campfire"),
    WaterContainer  UMETA(DisplayName = "WaterContainer"),
    BoneSword       UMETA(DisplayName = "BoneSword"),
    HideShield      UMETA(DisplayName = "HideShield"),
    FlintKnife      UMETA(DisplayName = "FlintKnife"),
    VineRope        UMETA(DisplayName = "VineRope")
};

// ─── Structs ─────────────────────────────────────────────────────────────────

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
    EQuest_CraftedItem ResultItem = EQuest_CraftedItem::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    TArray<FQuest_CraftingIngredient> Ingredients;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    float CraftTimeSeconds = 2.0f;
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
    EQuest_CraftedItem CraftedItem = EQuest_CraftedItem::None;

    UPROPERTY(BlueprintReadOnly, Category = "Crafting")
    FString Message;
};

// ─── Component ───────────────────────────────────────────────────────────────

UCLASS(ClassGroup = (Quest), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCraftingSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UCraftingSystem();

    virtual void BeginPlay() override;

    // ── Inventory ──────────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Crafting|Inventory")
    bool AddResource(EQuest_ResourceType Type, int32 Amount = 1);

    UFUNCTION(BlueprintCallable, Category = "Crafting|Inventory")
    bool RemoveResource(EQuest_ResourceType Type, int32 Amount = 1);

    UFUNCTION(BlueprintPure, Category = "Crafting|Inventory")
    int32 GetResourceCount(EQuest_ResourceType Type) const;

    UFUNCTION(BlueprintPure, Category = "Crafting|Inventory")
    TArray<FQuest_InventorySlot> GetInventory() const { return Inventory; }

    // ── Crafting ───────────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    FQuest_CraftResultData TryCraft(EQuest_CraftedItem Item);

    UFUNCTION(BlueprintPure, Category = "Crafting")
    bool CanCraft(EQuest_CraftedItem Item) const;

    UFUNCTION(BlueprintPure, Category = "Crafting")
    TArray<FQuest_CraftingRecipe> GetAvailableRecipes() const;

    UFUNCTION(BlueprintPure, Category = "Crafting")
    FQuest_CraftingRecipe GetRecipeForItem(EQuest_CraftedItem Item) const;

    // ── UI Toggle ──────────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Crafting|UI")
    void ToggleCraftingMenu();

    UFUNCTION(BlueprintPure, Category = "Crafting|UI")
    bool IsCraftingMenuOpen() const { return bCraftingMenuOpen; }

    // ── Events ─────────────────────────────────────────────────────────────

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCraftingComplete, FQuest_CraftResultData, Result);
    UPROPERTY(BlueprintAssignable, Category = "Crafting|Events")
    FOnCraftingComplete OnCraftingComplete;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnResourceChanged, EQuest_ResourceType, Type, int32, NewCount);
    UPROPERTY(BlueprintAssignable, Category = "Crafting|Events")
    FOnResourceChanged OnResourceChanged;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneBoolParam(FOnCraftingMenuToggled, bool, bIsOpen);
    UPROPERTY(BlueprintAssignable, Category = "Crafting|Events")
    FOnCraftingMenuToggled OnCraftingMenuToggled;

private:
    void InitialiseRecipes();

    UPROPERTY()
    TArray<FQuest_InventorySlot> Inventory;

    UPROPERTY()
    TArray<FQuest_CraftingRecipe> Recipes;

    UPROPERTY()
    bool bCraftingMenuOpen = false;

    static const int32 MaxInventorySlots = 20;
};
