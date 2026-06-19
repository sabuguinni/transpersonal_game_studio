// CraftingSystem.h
// Agent #14 — Quest & Mission Designer
// Crafting system: recipes, resource pickup, crafting UI trigger
// PROD_CYCLE_AUTO_20260619_007

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "SharedTypes.h"
#include "CraftingSystem.generated.h"

// ─── Enums ───────────────────────────────────────────────────────────────────

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

UENUM(BlueprintType)
enum class EQuest_CraftedItem : uint8
{
    None            UMETA(DisplayName = "None"),
    StoneAxe        UMETA(DisplayName = "Stone Axe"),
    Campfire        UMETA(DisplayName = "Campfire"),
    WaterContainer  UMETA(DisplayName = "Water Container"),
    SpearTip        UMETA(DisplayName = "Spear Tip"),
    BoneKnife       UMETA(DisplayName = "Bone Knife"),
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
    FString RecipeName = TEXT("Unknown Recipe");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    FString Description = TEXT("");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    TArray<FQuest_CraftingIngredient> Ingredients;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    float CraftingTimeSeconds = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    bool bRequiresCampfire = false;
};

USTRUCT(BlueprintType)
struct FQuest_PlayerInventory
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    int32 Rocks = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    int32 Sticks = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    int32 Leaves = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    int32 Bones = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    int32 Hides = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    int32 Flints = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    TArray<EQuest_CraftedItem> CraftedItems;
};

// ─── CraftingSystemManager ───────────────────────────────────────────────────

UCLASS(ClassGroup = (Quest), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCraftingSystemManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UCraftingSystemManager();

    // Recipes
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting|Recipes")
    TArray<FQuest_CraftingRecipe> AllRecipes;

    // Player inventory
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting|Inventory")
    FQuest_PlayerInventory PlayerInventory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting|UI")
    bool bCraftingMenuOpen = false;

    // Initialize default recipes
    UFUNCTION(BlueprintCallable, Category = "Crafting")
    void InitializeRecipes();

    // Toggle crafting menu (bound to C key)
    UFUNCTION(BlueprintCallable, Category = "Crafting|UI")
    void ToggleCraftingMenu();

    // Add resource to inventory
    UFUNCTION(BlueprintCallable, Category = "Crafting|Inventory")
    void AddResource(EQuest_ResourceType ResourceType, int32 Amount = 1);

    // Check if player can craft an item
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Crafting")
    bool CanCraft(EQuest_CraftedItem Item) const;

    // Attempt to craft an item
    UFUNCTION(BlueprintCallable, Category = "Crafting")
    bool TryCraft(EQuest_CraftedItem Item);

    // Get recipe for item
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Crafting")
    bool GetRecipe(EQuest_CraftedItem Item, FQuest_CraftingRecipe& OutRecipe) const;

    // Get resource count
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Crafting|Inventory")
    int32 GetResourceCount(EQuest_ResourceType ResourceType) const;

protected:
    virtual void BeginPlay() override;

private:
    void ConsumeIngredients(const FQuest_CraftingRecipe& Recipe);
};

// ─── ResourcePickupActor ─────────────────────────────────────────────────────

UCLASS(ClassGroup = (Quest), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API AQuest_ResourcePickup : public AActor
{
    GENERATED_BODY()

public:
    AQuest_ResourcePickup();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    EQuest_ResourceType ResourceType = EQuest_ResourceType::Rock;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    int32 ResourceAmount = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    bool bHasBeenPickedUp = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components",
        meta = (AllowPrivateAccess = "true"))
    UStaticMeshComponent* MeshComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components",
        meta = (AllowPrivateAccess = "true"))
    USphereComponent* PickupRadius;

    UFUNCTION(BlueprintCallable, Category = "Resource")
    void OnPlayerPickup(UCraftingSystemManager* CraftingManager);

protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);
};
