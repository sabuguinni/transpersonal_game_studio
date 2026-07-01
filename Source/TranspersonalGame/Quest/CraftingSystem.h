#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CraftingSystem.generated.h"

// ============================================================
// ENUMs — global scope (RULE 1)
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
    StoneAxe        UMETA(DisplayName = "Stone Axe"),
    Campfire        UMETA(DisplayName = "Campfire"),
    WaterContainer  UMETA(DisplayName = "Water Container"),
    BoneSword       UMETA(DisplayName = "Bone Sword"),
    HideArmor       UMETA(DisplayName = "Hide Armor"),
    VineRope        UMETA(DisplayName = "Vine Rope"),
    FlintKnife      UMETA(DisplayName = "Flint Knife")
};

// ============================================================
// STRUCTs — global scope (RULE 1)
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
    EQuest_CraftedItem OutputItem = EQuest_CraftedItem::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    FString RecipeName = TEXT("Unknown Recipe");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    FString Description = TEXT("");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    TArray<FQuest_CraftingIngredient> Ingredients;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    float CraftTimeSeconds = 2.0f;

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
    int32 Quantity = 0;

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
    FString Message = TEXT("");
};

// ============================================================
// UCraftingSystem — ActorComponent
// ============================================================

UCLASS(ClassGroup = (Quest), meta = (BlueprintSpawnableComponent), DisplayName = "Crafting System")
class TRANSPERSONALGAME_API UCraftingSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UCraftingSystem();

    virtual void BeginPlay() override;

    // --- Inventory ---

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    TArray<FQuest_InventorySlot> Inventory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    int32 MaxInventorySlots = 20;

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool AddResource(EQuest_ResourceType ResourceType, int32 Amount = 1);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool RemoveResource(EQuest_ResourceType ResourceType, int32 Amount = 1);

    UFUNCTION(BlueprintPure, Category = "Inventory")
    int32 GetResourceCount(EQuest_ResourceType ResourceType) const;

    UFUNCTION(BlueprintPure, Category = "Inventory")
    bool HasResource(EQuest_ResourceType ResourceType, int32 Amount = 1) const;

    // --- Recipes ---

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    TArray<FQuest_CraftingRecipe> Recipes;

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    void InitializeDefaultRecipes();

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    FQuest_CraftResultData TryCraft(EQuest_CraftedItem ItemToCraft);

    UFUNCTION(BlueprintPure, Category = "Crafting")
    bool CanCraft(EQuest_CraftedItem ItemToCraft) const;

    UFUNCTION(BlueprintPure, Category = "Crafting")
    TArray<FQuest_CraftingRecipe> GetAvailableRecipes() const;

    // --- Crafting Menu State ---

    UPROPERTY(BlueprintReadOnly, Category = "Crafting", meta = (AllowPrivateAccess = "true"))
    bool bCraftingMenuOpen = false;

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    void ToggleCraftingMenu();

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    void OpenCraftingMenu();

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    void CloseCraftingMenu();

    // --- Delegates ---

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemCrafted, EQuest_CraftedItem, CraftedItem);
    UPROPERTY(BlueprintAssignable, Category = "Crafting")
    FOnItemCrafted OnItemCrafted;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnResourceChanged, EQuest_ResourceType, ResourceType);
    UPROPERTY(BlueprintAssignable, Category = "Crafting")
    FOnResourceChanged OnResourceChanged;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCraftingMenuToggled, bool, bIsOpen);
    UPROPERTY(BlueprintAssignable, Category = "Crafting")
    FOnCraftingMenuToggled OnCraftingMenuToggled;

private:
    FQuest_CraftingRecipe* FindRecipe(EQuest_CraftedItem ItemToCraft);
    FQuest_InventorySlot* FindSlot(EQuest_ResourceType ResourceType);
    FQuest_InventorySlot* FindOrCreateSlot(EQuest_ResourceType ResourceType);
};
