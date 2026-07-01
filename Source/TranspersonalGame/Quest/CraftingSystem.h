#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CraftingSystem.generated.h"

// ============================================================
// ENUMS — global scope, Quest_ prefix (RULE 2)
// ============================================================

UENUM(BlueprintType)
enum class EQuest_ResourceType : uint8
{
    None    UMETA(DisplayName = "None"),
    Stone   UMETA(DisplayName = "Stone"),
    Stick   UMETA(DisplayName = "Stick"),
    Leaf    UMETA(DisplayName = "Leaf"),
    Bone    UMETA(DisplayName = "Bone"),
    Hide    UMETA(DisplayName = "Hide"),
    Flint   UMETA(DisplayName = "Flint"),
    Vine    UMETA(DisplayName = "Vine"),
    Water   UMETA(DisplayName = "Water"),
    Meat    UMETA(DisplayName = "Meat")
};

UENUM(BlueprintType)
enum class EQuest_CraftResult : uint8
{
    Success         UMETA(DisplayName = "Success"),
    MissingIngredients UMETA(DisplayName = "Missing Ingredients"),
    InventoryFull   UMETA(DisplayName = "Inventory Full"),
    InvalidRecipe   UMETA(DisplayName = "Invalid Recipe")
};

UENUM(BlueprintType)
enum class EQuest_CraftedItem : uint8
{
    None            UMETA(DisplayName = "None"),
    StoneAxe        UMETA(DisplayName = "Stone Axe"),
    Campfire        UMETA(DisplayName = "Campfire"),
    WaterContainer  UMETA(DisplayName = "Water Container"),
    Spear           UMETA(DisplayName = "Spear"),
    LeatherWrap     UMETA(DisplayName = "Leather Wrap"),
    BoneDagger      UMETA(DisplayName = "Bone Dagger"),
    FlintKnife      UMETA(DisplayName = "Flint Knife"),
    Torch           UMETA(DisplayName = "Torch")
};

// ============================================================
// STRUCTS — global scope (RULE 1)
// ============================================================

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_CraftingIngredient
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    EQuest_ResourceType ResourceType = EQuest_ResourceType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    int32 Quantity = 1;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_CraftingRecipe
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    EQuest_CraftedItem OutputItem = EQuest_CraftedItem::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    FString RecipeName = TEXT("Unknown Recipe");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    TArray<FQuest_CraftingIngredient> Ingredients;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    float CraftingTimeSeconds = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    bool bRequiresCampfire = false;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_InventorySlot
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
struct TRANSPERSONALGAME_API FQuest_CraftResultData
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
// COMPONENT CLASS
// ============================================================

UCLASS(ClassGroup = (Quest), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCraftingSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UCraftingSystem();

    // --- Inventory ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    TArray<FQuest_InventorySlot> Inventory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    int32 MaxInventorySlots = 20;

    // --- Recipes ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    TArray<FQuest_CraftingRecipe> AvailableRecipes;

    // --- State ---
    UPROPERTY(BlueprintReadOnly, Category = "Crafting")
    bool bCraftingMenuOpen = false;

    UPROPERTY(BlueprintReadOnly, Category = "Crafting")
    bool bIsCrafting = false;

    UPROPERTY(BlueprintReadOnly, Category = "Crafting")
    float CraftingProgress = 0.0f;

    // --- Public API ---
    UFUNCTION(BlueprintCallable, Category = "Crafting")
    FQuest_CraftResultData TryCraft(EQuest_CraftedItem ItemToCraft);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool AddResource(EQuest_ResourceType ResourceType, int32 Amount = 1);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool RemoveResource(EQuest_ResourceType ResourceType, int32 Amount = 1);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    int32 GetResourceCount(EQuest_ResourceType ResourceType) const;

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    bool HasIngredientsForRecipe(const FQuest_CraftingRecipe& Recipe) const;

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    TArray<FQuest_CraftingRecipe> GetAvailableRecipes() const;

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    void OpenCraftingMenu();

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    void CloseCraftingMenu();

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    void ToggleCraftingMenu();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    void InitializeDefaultRecipes();
    const FQuest_CraftingRecipe* FindRecipe(EQuest_CraftedItem Item) const;
    bool ConsumeIngredients(const FQuest_CraftingRecipe& Recipe);

    float CurrentCraftTimer = 0.0f;
    EQuest_CraftedItem PendingCraftItem = EQuest_CraftedItem::None;
};
