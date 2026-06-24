#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CraftingSystem.generated.h"

// -------------------------------------------------------
// CraftingSystem — Agent #14 Quest & Mission Designer
// Implements 3 core recipes: Stone Axe, Campfire, Water Container
// Press C to open crafting menu (Blueprint-bindable)
// -------------------------------------------------------

UENUM(BlueprintType)
enum class EQuest_ResourceType : uint8
{
    Rock        UMETA(DisplayName = "Rock"),
    Stick       UMETA(DisplayName = "Stick"),
    Leaf        UMETA(DisplayName = "Leaf"),
    Bone        UMETA(DisplayName = "Bone"),
    Hide        UMETA(DisplayName = "Hide"),
    None        UMETA(DisplayName = "None")
};

UENUM(BlueprintType)
enum class EQuest_CraftedItem : uint8
{
    StoneAxe        UMETA(DisplayName = "Stone Axe"),
    Campfire        UMETA(DisplayName = "Campfire"),
    WaterContainer  UMETA(DisplayName = "Water Container"),
    SpearHead       UMETA(DisplayName = "Spear Head"),
    None            UMETA(DisplayName = "None")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_ResourceStack
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    EQuest_ResourceType ResourceType = EQuest_ResourceType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    int32 Quantity = 0;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_CraftingRecipe
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    EQuest_CraftedItem OutputItem = EQuest_CraftedItem::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    FString DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    TArray<FQuest_ResourceStack> RequiredIngredients;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    float CraftingTimeSeconds = 2.0f;
};

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCraftingSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UCraftingSystem();

    // Player inventory — current resource counts
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    TMap<EQuest_ResourceType, int32> PlayerInventory;

    // All available recipes
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    TArray<FQuest_CraftingRecipe> Recipes;

    // Whether crafting menu is open
    UPROPERTY(BlueprintReadOnly, Category = "Crafting", meta = (AllowPrivateAccess = "true"))
    bool bCraftingMenuOpen = false;

    // Add a resource to inventory (called when player picks up a resource actor)
    UFUNCTION(BlueprintCallable, Category = "Crafting")
    void PickupResource(EQuest_ResourceType ResourceType, int32 Amount = 1);

    // Attempt to craft an item — returns true if ingredients met and crafting started
    UFUNCTION(BlueprintCallable, Category = "Crafting")
    bool TryCraft(EQuest_CraftedItem Item);

    // Toggle crafting menu (bound to C key in Blueprint)
    UFUNCTION(BlueprintCallable, Category = "Crafting")
    void ToggleCraftingMenu();

    // Check if player has enough ingredients for a recipe
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Crafting")
    bool CanCraft(EQuest_CraftedItem Item) const;

    // Get current count of a resource in inventory
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Crafting")
    int32 GetResourceCount(EQuest_ResourceType ResourceType) const;

    // Debug: log all inventory and recipes
    UFUNCTION(CallInEditor, Category = "Crafting")
    void LogInventoryState() const;

    // Delegate for Blueprint UI updates
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCraftingMenuToggled, bool, bIsOpen);
    UPROPERTY(BlueprintAssignable, Category = "Crafting")
    FOnCraftingMenuToggled OnCraftingMenuToggled;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemCrafted, EQuest_CraftedItem, CraftedItem);
    UPROPERTY(BlueprintAssignable, Category = "Crafting")
    FOnItemCrafted OnItemCrafted;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnResourcePickedUp, EQuest_ResourceType, ResourceType, int32, NewTotal);
    UPROPERTY(BlueprintAssignable, Category = "Crafting")
    FOnResourcePickedUp OnResourcePickedUp;

protected:
    virtual void BeginPlay() override;

private:
    void InitializeRecipes();
    const FQuest_CraftingRecipe* FindRecipe(EQuest_CraftedItem Item) const;
    void ConsumeIngredients(const FQuest_CraftingRecipe& Recipe);
};
