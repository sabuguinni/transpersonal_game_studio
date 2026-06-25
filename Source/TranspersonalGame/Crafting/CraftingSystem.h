// CraftingSystem.h
// Agent #14 — Quest & Mission Designer
// Crafting system: recipes, resource pickup, crafting UI trigger

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "CraftingSystem.generated.h"

// ─────────────────────────────────────────────────────────────────────────────
// Enums & Structs (global scope — RULE 1)
// ─────────────────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class EQuest_ResourceType : uint8
{
    None        UMETA(DisplayName = "None"),
    Rock        UMETA(DisplayName = "Rock"),
    Stick       UMETA(DisplayName = "Stick"),
    Leaf        UMETA(DisplayName = "Leaf"),
    Flint       UMETA(DisplayName = "Flint"),
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
    LeatherPouch    UMETA(DisplayName = "Leather Pouch"),
};

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
    EQuest_CraftedItem OutputItem = EQuest_CraftedItem::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    FString ItemName = TEXT("Unknown Item");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    TArray<FQuest_ResourceIngredient> Ingredients;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    FString Description = TEXT("");
};

// ─────────────────────────────────────────────────────────────────────────────
// Resource Pickup Actor
// ─────────────────────────────────────────────────────────────────────────────

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_ResourcePickup : public AActor
{
    GENERATED_BODY()

public:
    AQuest_ResourcePickup();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components",
        meta = (AllowPrivateAccess = "true"))
    USphereComponent* CollisionSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components",
        meta = (AllowPrivateAccess = "true"))
    UStaticMeshComponent* MeshComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    EQuest_ResourceType ResourceType = EQuest_ResourceType::Rock;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    int32 Quantity = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    bool bAutoRespawn = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    float RespawnDelay = 120.0f;

    UFUNCTION(BlueprintCallable, Category = "Resource")
    void OnPickedUp(AActor* Collector);

    UFUNCTION(BlueprintCallable, Category = "Resource")
    void Respawn();

protected:
    virtual void BeginPlay() override;

private:
    FTimerHandle RespawnTimer;
};

// ─────────────────────────────────────────────────────────────────────────────
// Crafting System Manager (Actor Component)
// ─────────────────────────────────────────────────────────────────────────────

UCLASS(BlueprintType, Blueprintable, ClassGroup = (Custom),
    meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UQuest_CraftingSystemComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuest_CraftingSystemComponent();

    // Player inventory (resource counts)
    UPROPERTY(BlueprintReadWrite, Category = "Inventory")
    TMap<EQuest_ResourceType, int32> ResourceInventory;

    // All available recipes
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    TArray<FQuest_CraftingRecipe> Recipes;

    // Crafted items held by player
    UPROPERTY(BlueprintReadWrite, Category = "Inventory")
    TArray<EQuest_CraftedItem> CraftedItems;

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    void InitialiseDefaultRecipes();

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    bool CanCraft(EQuest_CraftedItem Item) const;

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    bool TryCraft(EQuest_CraftedItem Item);

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    void AddResource(EQuest_ResourceType Type, int32 Amount = 1);

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    int32 GetResourceCount(EQuest_ResourceType Type) const;

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    bool HasCraftedItem(EQuest_CraftedItem Item) const;

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    TArray<FQuest_CraftingRecipe> GetAvailableRecipes() const;

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    void ToggleCraftingMenu();

    UPROPERTY(BlueprintReadOnly, Category = "UI")
    bool bCraftingMenuOpen = false;

protected:
    virtual void BeginPlay() override;

private:
    const FQuest_CraftingRecipe* FindRecipe(EQuest_CraftedItem Item) const;
};
