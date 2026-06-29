#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "QuestCraftingSystem.generated.h"

// ============================================================
// ENUMS — must be at global scope (UE5 compilation rule)
// ============================================================

UENUM(BlueprintType)
enum class EQuest_ResourceType : uint8
{
    None        UMETA(DisplayName = "None"),
    Flint       UMETA(DisplayName = "Flint Rock"),
    Stick       UMETA(DisplayName = "Wooden Stick"),
    Leaf        UMETA(DisplayName = "Large Leaf"),
    Bone        UMETA(DisplayName = "Animal Bone"),
    Vine        UMETA(DisplayName = "Vine"),
    Clay        UMETA(DisplayName = "Clay"),
    Tinder      UMETA(DisplayName = "Dry Tinder")
};

UENUM(BlueprintType)
enum class EQuest_CraftedItem : uint8
{
    None            UMETA(DisplayName = "None"),
    StoneAxe        UMETA(DisplayName = "Stone Axe"),
    Campfire        UMETA(DisplayName = "Campfire"),
    WaterContainer  UMETA(DisplayName = "Water Container"),
    BoneTip         UMETA(DisplayName = "Bone-Tipped Spear"),
    Torch           UMETA(DisplayName = "Torch")
};

UENUM(BlueprintType)
enum class EQuest_CraftingState : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    MenuOpen        UMETA(DisplayName = "Menu Open"),
    Crafting        UMETA(DisplayName = "Crafting"),
    Success         UMETA(DisplayName = "Success"),
    Failed          UMETA(DisplayName = "Failed — Missing Materials")
};

// ============================================================
// STRUCTS
// ============================================================

USTRUCT(BlueprintType)
struct FQuest_CraftingIngredient
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    EQuest_ResourceType ResourceType = EQuest_ResourceType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    int32 RequiredCount = 1;
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
    float CraftingTimeSec = 2.0f;
};

USTRUCT(BlueprintType)
struct FQuest_PlayerInventory
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    int32 FlintCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    int32 StickCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    int32 LeafCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    int32 BoneCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    int32 VineCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    int32 ClayCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    int32 TinderCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    TArray<EQuest_CraftedItem> CraftedItems;
};

// ============================================================
// RESOURCE PICKUP ACTOR — spawned in world for player to collect
// ============================================================

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_ResourcePickup : public AActor
{
    GENERATED_BODY()

public:
    AQuest_ResourcePickup();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    EQuest_ResourceType ResourceType = EQuest_ResourceType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    int32 PickupCount = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    bool bHasBeenPickedUp = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Resource")
    class UStaticMeshComponent* MeshComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Resource")
    class USphereComponent* PickupRadius;

    UFUNCTION(BlueprintCallable, Category = "Resource")
    bool TryPickup(FQuest_PlayerInventory& OutInventory);

    UFUNCTION(BlueprintCallable, Category = "Resource")
    FString GetResourceDisplayName() const;

protected:
    virtual void BeginPlay() override;
};

// ============================================================
// CRAFTING SYSTEM MANAGER
// ============================================================

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_CraftingSystemManager : public AActor
{
    GENERATED_BODY()

public:
    AQuest_CraftingSystemManager();

    // Current crafting state
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crafting")
    EQuest_CraftingState CraftingState = EQuest_CraftingState::Idle;

    // Player's current inventory
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    FQuest_PlayerInventory PlayerInventory;

    // All available recipes
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    TArray<FQuest_CraftingRecipe> Recipes;

    // Currently selected recipe index
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crafting")
    int32 SelectedRecipeIndex = -1;

    // Crafting progress timer
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crafting")
    float CraftingProgress = 0.0f;

    // ---- Core Functions ----

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    void InitializeRecipes();

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    bool CanCraft(int32 RecipeIndex) const;

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    bool TryCraft(int32 RecipeIndex);

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    void OpenCraftingMenu();

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    void CloseCraftingMenu();

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    TArray<FQuest_CraftingRecipe> GetAvailableRecipes() const;

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    int32 GetResourceCount(EQuest_ResourceType ResourceType) const;

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    void AddResource(EQuest_ResourceType ResourceType, int32 Count);

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    bool HasCraftedItem(EQuest_CraftedItem Item) const;

    // Quest integration — called by quest system to check progress
    UFUNCTION(BlueprintCallable, Category = "Quest Integration")
    bool HasCraftedStoneAxe() const;

    UFUNCTION(BlueprintCallable, Category = "Quest Integration")
    bool HasCraftedCampfire() const;

    UFUNCTION(BlueprintCallable, Category = "Quest Integration")
    bool HasCraftedTorch() const;

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    void ConsumeMaterials(int32 RecipeIndex);
    FTimerHandle CraftingTimerHandle;
    void OnCraftingComplete();
};
