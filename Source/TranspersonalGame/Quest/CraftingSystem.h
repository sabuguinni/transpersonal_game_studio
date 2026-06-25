#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CraftingSystem.generated.h"

// ============================================================
// Enums & Structs — must be at global scope (UHT rule)
// ============================================================

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
enum class EQuest_CraftedItemType : uint8
{
    None            UMETA(DisplayName = "None"),
    StoneAxe        UMETA(DisplayName = "Stone Axe"),
    Campfire        UMETA(DisplayName = "Campfire"),
    WaterContainer  UMETA(DisplayName = "Water Container"),
    Spear           UMETA(DisplayName = "Spear"),
    Shelter         UMETA(DisplayName = "Shelter"),
};

USTRUCT(BlueprintType)
struct FQuest_ResourceStack
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    EQuest_ResourceType ResourceType = EQuest_ResourceType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    int32 Quantity = 0;
};

USTRUCT(BlueprintType)
struct FQuest_CraftingRecipe
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    EQuest_CraftedItemType ResultItem = EQuest_CraftedItemType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    FString DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    TArray<FQuest_ResourceStack> Ingredients;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    float CraftTimeSeconds = 2.0f;
};

// ============================================================
// Resource Pickup Actor — spawned in world, player walks over
// ============================================================

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_ResourcePickup : public AActor
{
    GENERATED_BODY()

public:
    AQuest_ResourcePickup();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    EQuest_ResourceType ResourceType = EQuest_ResourceType::Rock;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    int32 QuantityOnPickup = 1;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Resource",
              meta = (AllowPrivateAccess = "true"))
    UStaticMeshComponent* MeshComponent;

    UFUNCTION(BlueprintCallable, Category = "Resource")
    void OnPlayerPickup(AActor* PlayerActor);

    UFUNCTION(BlueprintPure, Category = "Resource")
    bool IsAvailable() const { return bIsAvailable; }

protected:
    virtual void BeginPlay() override;

private:
    bool bIsAvailable = true;
};

// ============================================================
// Crafting System Component — attach to PlayerCharacter
// ============================================================

UCLASS(BlueprintType, Blueprintable, ClassGroup = (Custom),
       meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UQuest_CraftingComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuest_CraftingComponent();

    // Inventory: map of resource type → quantity
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    TArray<FQuest_ResourceStack> Inventory;

    // All available recipes
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    TArray<FQuest_CraftingRecipe> Recipes;

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    void AddResource(EQuest_ResourceType Type, int32 Quantity);

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    bool HasIngredients(const FQuest_CraftingRecipe& Recipe) const;

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    bool CraftItem(EQuest_CraftedItemType ItemType);

    UFUNCTION(BlueprintPure, Category = "Crafting")
    int32 GetResourceCount(EQuest_ResourceType Type) const;

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    TArray<FQuest_CraftingRecipe> GetAvailableRecipes() const;

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    void InitialiseDefaultRecipes();

protected:
    virtual void BeginPlay() override;

private:
    void ConsumeIngredients(const FQuest_CraftingRecipe& Recipe);
};
