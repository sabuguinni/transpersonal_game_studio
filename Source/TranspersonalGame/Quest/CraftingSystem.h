// CraftingSystem.h
// Agent #14 — Quest & Mission Designer
// Cycle: PROD_CYCLE_AUTO_20260627_007
// Crafting system: recipes, resource pickups, crafting station interaction

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
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
};

UENUM(BlueprintType)
enum class EQuest_CraftedItem : uint8
{
    None            UMETA(DisplayName = "None"),
    StoneAxe        UMETA(DisplayName = "Stone Axe"),
    Campfire        UMETA(DisplayName = "Campfire"),
    WaterContainer  UMETA(DisplayName = "Water Container"),
    Spear           UMETA(DisplayName = "Spear"),
    LeafBandage     UMETA(DisplayName = "Leaf Bandage"),
};

// ─── Structs ─────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FQuest_ResourceIngredient
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
    EQuest_CraftedItem Result = EQuest_CraftedItem::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    FString RecipeName = TEXT("Unknown Recipe");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    TArray<FQuest_ResourceIngredient> Ingredients;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    float CraftTimeSeconds = 3.0f;
};

// ─── Resource Pickup Actor ────────────────────────────────────────────────────

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_ResourcePickup : public AActor
{
    GENERATED_BODY()

public:
    AQuest_ResourcePickup();

    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* MeshComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* PickupRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    EQuest_ResourceType ResourceType = EQuest_ResourceType::Rock;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    int32 Quantity = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    bool bIsCollected = false;

    UFUNCTION(BlueprintCallable, Category = "Resource")
    bool TryCollect(AActor* Collector);

    UFUNCTION(BlueprintCallable, Category = "Resource")
    FString GetResourceLabel() const;
};

// ─── Crafting Manager Component ───────────────────────────────────────────────

UCLASS(BlueprintType, Blueprintable, ClassGroup = "Quest", meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UQuest_CraftingManagerComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuest_CraftingManagerComponent();

    virtual void BeginPlay() override;

    // Inventory: resource type → count
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
    TMap<EQuest_ResourceType, int32> ResourceInventory;

    // All available recipes
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    TArray<FQuest_CraftingRecipe> Recipes;

    // Crafted items log
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crafting")
    TArray<EQuest_CraftedItem> CraftedItems;

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    void AddResource(EQuest_ResourceType Type, int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    bool CanCraft(EQuest_CraftedItem Item) const;

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    bool TryCraft(EQuest_CraftedItem Item);

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    int32 GetResourceCount(EQuest_ResourceType Type) const;

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    TArray<FQuest_CraftingRecipe> GetAvailableRecipes() const;

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    void InitialiseDefaultRecipes();

private:
    void ConsumeIngredients(const FQuest_CraftingRecipe& Recipe);
};
