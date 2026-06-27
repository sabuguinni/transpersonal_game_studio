#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CraftingSystem.generated.h"

UENUM(BlueprintType)
enum class EQuest_ResourceType : uint8
{
    Rock    UMETA(DisplayName = "Rock"),
    Stick   UMETA(DisplayName = "Stick"),
    Leaf    UMETA(DisplayName = "Leaf"),
    Hide    UMETA(DisplayName = "Hide"),
    Bone    UMETA(DisplayName = "Bone"),
    None    UMETA(DisplayName = "None")
};

UENUM(BlueprintType)
enum class EQuest_CraftedItemType : uint8
{
    StoneAxe        UMETA(DisplayName = "Stone Axe"),
    Campfire        UMETA(DisplayName = "Campfire"),
    WaterContainer  UMETA(DisplayName = "Water Container"),
    Spear           UMETA(DisplayName = "Spear"),
    None            UMETA(DisplayName = "None")
};

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
    EQuest_CraftedItemType OutputItem = EQuest_CraftedItemType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    FString RecipeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    TArray<FQuest_CraftingIngredient> Ingredients;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    float CraftingTimeSec = 2.0f;
};

/**
 * ACraftingSystem — manages crafting recipes and player inventory for the survival crafting loop.
 * Recipes: Stone Axe (2 Rocks + 1 Stick), Campfire (3 Sticks), Water Container (1 Rock + 1 Leaf).
 */
UCLASS(ClassGroup = (Quest), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ACraftingSystem : public AActor
{
    GENERATED_BODY()

public:
    ACraftingSystem();

    /** All available crafting recipes */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    TArray<FQuest_CraftingRecipe> Recipes;

    /** Player resource inventory: resource type → count */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crafting")
    TMap<EQuest_ResourceType, int32> PlayerInventory;

    /** Add a resource to the player inventory */
    UFUNCTION(BlueprintCallable, Category = "Crafting")
    void AddResource(EQuest_ResourceType ResourceType, int32 Amount = 1);

    /** Attempt to craft an item. Returns true if successful. */
    UFUNCTION(BlueprintCallable, Category = "Crafting")
    bool TryCraft(EQuest_CraftedItemType ItemType);

    /** Check if player has enough resources for a recipe */
    UFUNCTION(BlueprintCallable, Category = "Crafting")
    bool CanCraft(EQuest_CraftedItemType ItemType) const;

    /** Get the recipe for a given item type */
    UFUNCTION(BlueprintCallable, Category = "Crafting")
    bool GetRecipe(EQuest_CraftedItemType ItemType, FQuest_CraftingRecipe& OutRecipe) const;

    /** Open/close crafting UI (press C) */
    UFUNCTION(BlueprintCallable, Category = "Crafting")
    void ToggleCraftingUI();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crafting")
    bool bCraftingUIOpen = false;

protected:
    virtual void BeginPlay() override;

private:
    void InitializeRecipes();
};

/**
 * AResourcePickupActor — a pickup actor the player can collect (rock, stick, leaf).
 */
UCLASS()
class TRANSPERSONALGAME_API AResourcePickupActor : public AActor
{
    GENERATED_BODY()

public:
    AResourcePickupActor();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    EQuest_ResourceType ResourceType = EQuest_ResourceType::Rock;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    int32 ResourceAmount = 1;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Resource")
    bool bCollected = false;

    UFUNCTION(BlueprintCallable, Category = "Resource")
    void Collect(ACraftingSystem* CraftingSystem);

protected:
    UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* MeshComponent;

    virtual void BeginPlay() override;
};
