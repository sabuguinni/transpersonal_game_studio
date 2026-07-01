#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "CraftingSystem.generated.h"

// ============================================================
// ENUMS — global scope, Quest_ prefix to avoid collisions
// ============================================================

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
    Vine      UMETA(DisplayName = "Vine"),
    Water     UMETA(DisplayName = "Water"),
    Meat      UMETA(DisplayName = "Meat")
};

UENUM(BlueprintType)
enum class EQuest_CraftResult : uint8
{
    Success         UMETA(DisplayName = "Success"),
    InsufficientMaterials UMETA(DisplayName = "InsufficientMaterials"),
    InventoryFull   UMETA(DisplayName = "InventoryFull"),
    UnknownRecipe   UMETA(DisplayName = "UnknownRecipe")
};

UENUM(BlueprintType)
enum class EQuest_CraftedItem : uint8
{
    None            UMETA(DisplayName = "None"),
    StoneAxe        UMETA(DisplayName = "Stone Axe"),
    Campfire        UMETA(DisplayName = "Campfire"),
    WaterContainer  UMETA(DisplayName = "Water Container"),
    BoneScraper     UMETA(DisplayName = "Bone Scraper"),
    VineRope        UMETA(DisplayName = "Vine Rope"),
    LeatherPouch    UMETA(DisplayName = "Leather Pouch"),
    FlintKnife      UMETA(DisplayName = "Flint Knife"),
    MeatSkewer      UMETA(DisplayName = "Meat Skewer")
};

// ============================================================
// STRUCTS — global scope
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
    FString DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    TArray<FQuest_CraftingIngredient> Ingredients;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    float CraftTimeSeconds = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    FString Description;
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
    EQuest_CraftResult Result = EQuest_CraftResult::UnknownRecipe;

    UPROPERTY(BlueprintReadOnly, Category = "Crafting")
    EQuest_CraftedItem CraftedItem = EQuest_CraftedItem::None;

    UPROPERTY(BlueprintReadOnly, Category = "Crafting")
    FString Message;
};

// ============================================================
// RESOURCE PICKUP ACTOR
// ============================================================

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_ResourcePickup : public AActor
{
    GENERATED_BODY()

public:
    AQuest_ResourcePickup();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    UStaticMeshComponent* MeshComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    USphereComponent* PickupRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    EQuest_ResourceType ResourceType = EQuest_ResourceType::Stone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    int32 ResourceAmount = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    bool bAutoPickup = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    float PickupRadius_CM = 150.0f;

    UFUNCTION(BlueprintCallable, Category = "Resource")
    void OnPickedUp(AActor* Collector);

    UFUNCTION(BlueprintCallable, Category = "Resource")
    EQuest_ResourceType GetResourceType() const { return ResourceType; }

    UFUNCTION(BlueprintCallable, Category = "Resource")
    int32 GetResourceAmount() const { return ResourceAmount; }

protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

private:
    bool bHasBeenPickedUp = false;
};

// ============================================================
// CRAFTING MANAGER ACTOR
// ============================================================

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_CraftingManager : public AActor
{
    GENERATED_BODY()

public:
    AQuest_CraftingManager();

    // Inventory
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    TArray<FQuest_InventorySlot> Inventory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    int32 MaxInventorySlots = 20;

    // Recipes
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    TArray<FQuest_CraftingRecipe> AvailableRecipes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    bool bCraftingMenuOpen = false;

    // Core functions
    UFUNCTION(BlueprintCallable, Category = "Crafting")
    FQuest_CraftResultData TryCraft(EQuest_CraftedItem ItemToCraft);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool AddResource(EQuest_ResourceType Type, int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool RemoveResource(EQuest_ResourceType Type, int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    int32 GetResourceCount(EQuest_ResourceType Type) const;

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    TArray<FQuest_CraftingRecipe> GetCraftableRecipes() const;

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    void ToggleCraftingMenu();

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    void InitializeDefaultRecipes();

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    bool HasIngredientsForRecipe(const FQuest_CraftingRecipe& Recipe) const;

protected:
    virtual void BeginPlay() override;

private:
    void ConsumeIngredients(const FQuest_CraftingRecipe& Recipe);
    FQuest_CraftingRecipe* FindRecipeForItem(EQuest_CraftedItem Item);
};
