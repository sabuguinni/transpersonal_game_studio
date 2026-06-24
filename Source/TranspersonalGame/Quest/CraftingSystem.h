#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CraftingSystem.generated.h"

// Resource type enum — prefix Quest_ to avoid collisions
UENUM(BlueprintType)
enum class EQuest_ResourceType : uint8
{
    None        UMETA(DisplayName = "None"),
    Rock        UMETA(DisplayName = "Rock"),
    Stick       UMETA(DisplayName = "Stick"),
    Leaf        UMETA(DisplayName = "Leaf"),
    Bone        UMETA(DisplayName = "Bone"),
    Hide        UMETA(DisplayName = "Hide"),
    Flint       UMETA(DisplayName = "Flint"),
};

// Crafted item enum
UENUM(BlueprintType)
enum class EQuest_CraftedItem : uint8
{
    None            UMETA(DisplayName = "None"),
    StoneAxe        UMETA(DisplayName = "Stone Axe"),
    Campfire        UMETA(DisplayName = "Campfire"),
    WaterContainer  UMETA(DisplayName = "Water Container"),
    Spear           UMETA(DisplayName = "Spear"),
    Shelter         UMETA(DisplayName = "Shelter"),
};

// Recipe ingredient
USTRUCT(BlueprintType)
struct FQuest_RecipeIngredient
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    EQuest_ResourceType ResourceType = EQuest_ResourceType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    int32 Quantity = 1;
};

// Full recipe definition
USTRUCT(BlueprintType)
struct FQuest_CraftingRecipe
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    EQuest_CraftedItem ResultItem = EQuest_CraftedItem::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    FString RecipeName = TEXT("Unknown Recipe");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    FString Description = TEXT("");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    TArray<FQuest_RecipeIngredient> Ingredients;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    float CraftingTime = 2.0f;
};

// Player inventory slot
USTRUCT(BlueprintType)
struct FQuest_InventorySlot
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    EQuest_ResourceType ResourceType = EQuest_ResourceType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    int32 Count = 0;
};

/**
 * UCraftingSystem — manages crafting recipes and player inventory
 * Agent #14 Quest & Mission Designer
 */
UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCraftingSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UCraftingSystem();

    // All available recipes
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    TArray<FQuest_CraftingRecipe> Recipes;

    // Player inventory
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    TArray<FQuest_InventorySlot> Inventory;

    // Whether crafting menu is open
    UPROPERTY(BlueprintReadOnly, Category = "Crafting")
    bool bCraftingMenuOpen = false;

    // Add resource to inventory
    UFUNCTION(BlueprintCallable, Category = "Crafting")
    void AddResource(EQuest_ResourceType ResourceType, int32 Amount = 1);

    // Remove resource from inventory (returns false if not enough)
    UFUNCTION(BlueprintCallable, Category = "Crafting")
    bool RemoveResource(EQuest_ResourceType ResourceType, int32 Amount = 1);

    // Get count of a resource in inventory
    UFUNCTION(BlueprintCallable, Category = "Crafting")
    int32 GetResourceCount(EQuest_ResourceType ResourceType) const;

    // Check if player can craft a given item
    UFUNCTION(BlueprintCallable, Category = "Crafting")
    bool CanCraft(EQuest_CraftedItem Item) const;

    // Attempt to craft an item — consumes ingredients on success
    UFUNCTION(BlueprintCallable, Category = "Crafting")
    bool TryCraft(EQuest_CraftedItem Item);

    // Toggle crafting menu (bound to C key)
    UFUNCTION(BlueprintCallable, Category = "Crafting")
    void ToggleCraftingMenu();

    // Get recipe for item
    UFUNCTION(BlueprintCallable, Category = "Crafting")
    bool GetRecipe(EQuest_CraftedItem Item, FQuest_CraftingRecipe& OutRecipe) const;

    // Initialize default recipes
    UFUNCTION(BlueprintCallable, Category = "Crafting")
    void InitializeDefaultRecipes();

protected:
    virtual void BeginPlay() override;
};

/**
 * AResourcePickup — a pickup actor that adds resources to player inventory
 */
UCLASS()
class TRANSPERSONALGAME_API AResourcePickup : public AActor
{
    GENERATED_BODY()

public:
    AResourcePickup();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    EQuest_ResourceType ResourceType = EQuest_ResourceType::Rock;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    int32 ResourceAmount = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    bool bHasBeenPickedUp = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Resource")
    TObjectPtr<UStaticMeshComponent> MeshComponent;

    // Called when player overlaps this actor
    UFUNCTION(BlueprintCallable, Category = "Resource")
    void OnPickup(AActor* PickingActor);

protected:
    virtual void BeginPlay() override;
};
