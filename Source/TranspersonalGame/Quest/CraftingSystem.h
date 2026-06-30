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
    Flint       UMETA(DisplayName = "Flint"),
    Vine        UMETA(DisplayName = "Vine")
};

UENUM(BlueprintType)
enum class EQuest_CraftedItem : uint8
{
    None            UMETA(DisplayName = "None"),
    StoneAxe        UMETA(DisplayName = "Stone Axe"),
    Campfire        UMETA(DisplayName = "Campfire"),
    WaterContainer  UMETA(DisplayName = "Water Container"),
    WoodenSpear     UMETA(DisplayName = "Wooden Spear"),
    LeafShelter     UMETA(DisplayName = "Leaf Shelter")
};

// ─── Structs ─────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FQuest_ResourceRequirement
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
    FString RecipeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    TArray<FQuest_ResourceRequirement> Requirements;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    float CraftTimeSeconds = 2.0f;
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
    int32 Flint = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    int32 Vines = 0;
};

// ─── Resource Pickup Actor ────────────────────────────────────────────────────

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_ResourcePickup : public AActor
{
    GENERATED_BODY()

public:
    AQuest_ResourcePickup();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UStaticMeshComponent> MeshComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USphereComponent> PickupCollision;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    EQuest_ResourceType ResourceType = EQuest_ResourceType::Rock;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    int32 Quantity = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    bool bAutoRespawn = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    float RespawnDelay = 60.0f;

    UFUNCTION(BlueprintCallable, Category = "Resource")
    void OnPlayerOverlap(AActor* OverlappingActor);

    UFUNCTION(BlueprintCallable, Category = "Resource")
    void RespawnResource();

    UFUNCTION(BlueprintPure, Category = "Resource")
    FString GetResourceDisplayName() const;

protected:
    virtual void BeginPlay() override;

private:
    FTimerHandle RespawnTimerHandle;
    bool bIsPickedUp = false;
};

// ─── Crafting System Manager ──────────────────────────────────────────────────

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_CraftingSystem : public AActor
{
    GENERATED_BODY()

public:
    AQuest_CraftingSystem();

    // All available recipes
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    TArray<FQuest_CraftingRecipe> Recipes;

    // Player's current inventory
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    FQuest_PlayerInventory PlayerInventory;

    // Is crafting menu open
    UPROPERTY(BlueprintReadOnly, Category = "Crafting")
    bool bCraftingMenuOpen = false;

    // Crafted items this session
    UPROPERTY(BlueprintReadOnly, Category = "Crafting")
    TArray<EQuest_CraftedItem> CraftedItems;

    // ─── Core Methods ─────────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    void ToggleCraftingMenu();

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    bool CanCraft(EQuest_CraftedItem Item) const;

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    bool CraftItem(EQuest_CraftedItem Item);

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    void AddResource(EQuest_ResourceType Resource, int32 Amount = 1);

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    int32 GetResourceCount(EQuest_ResourceType Resource) const;

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    TArray<FQuest_CraftingRecipe> GetAvailableRecipes() const;

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    FQuest_CraftingRecipe GetRecipeForItem(EQuest_CraftedItem Item) const;

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    bool HasCraftedItem(EQuest_CraftedItem Item) const;

    // ─── Input Handler ────────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    void HandleCraftingInput();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    void InitializeRecipes();
    bool ConsumeResources(const FQuest_CraftingRecipe& Recipe);
    void OnCraftingComplete(EQuest_CraftedItem Item);

    bool bIsCrafting = false;
    float CraftingProgress = 0.0f;
    EQuest_CraftedItem CurrentlyCrafting = EQuest_CraftedItem::None;
    float CurrentCraftTime = 0.0f;
};
