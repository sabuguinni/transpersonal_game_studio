// CraftingSystem.h
// Agent #14 — Quest & Mission Designer
// Crafting system: recipes, resource pickups, crafting UI trigger
// Cycle: PROD_CYCLE_AUTO_20260626_008

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
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
    Flint       UMETA(DisplayName = "Flint"),
    Bone        UMETA(DisplayName = "Bone"),
    Hide        UMETA(DisplayName = "Hide"),
    Fiber       UMETA(DisplayName = "Fiber"),
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
    FName RecipeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    EQuest_CraftedItemType OutputItem = EQuest_CraftedItemType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    TArray<FQuest_CraftingIngredient> Ingredients;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    float CraftingTimeSec = 2.0f;

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
    int32 Flint = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    int32 Bones = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    int32 Hides = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    int32 Fiber = 0;

    // Crafted items
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    TArray<EQuest_CraftedItemType> CraftedItems;
};

// ─── Resource Pickup Actor ────────────────────────────────────────────────────

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_ResourcePickup : public AActor
{
    GENERATED_BODY()

public:
    AQuest_ResourcePickup();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components",
        meta = (AllowPrivateAccess = "true"))
    UStaticMeshComponent* MeshComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components",
        meta = (AllowPrivateAccess = "true"))
    UBoxComponent* PickupCollision;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    EQuest_ResourceType ResourceType = EQuest_ResourceType::Rock;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    int32 Quantity = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    bool bAutoPickup = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    float RespawnTimeSec = 120.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Resource")
    bool bIsPickedUp = false;

    UFUNCTION(BlueprintCallable, Category = "Resource")
    bool TryPickup(AActor* Collector);

    UFUNCTION(BlueprintCallable, Category = "Resource")
    void Respawn();

    UFUNCTION(BlueprintCallable, Category = "Resource")
    FText GetPickupPrompt() const;

protected:
    FTimerHandle RespawnTimer;

    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);
};

// ─── Crafting Station Actor ───────────────────────────────────────────────────

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_CraftingStation : public AActor
{
    GENERATED_BODY()

public:
    AQuest_CraftingStation();

    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components",
        meta = (AllowPrivateAccess = "true"))
    UStaticMeshComponent* StationMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components",
        meta = (AllowPrivateAccess = "true"))
    UBoxComponent* InteractionZone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    TArray<FQuest_CraftingRecipe> AvailableRecipes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    bool bIsCampfire = false;

    UPROPERTY(BlueprintReadOnly, Category = "Crafting")
    bool bPlayerInRange = false;

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    bool CanCraft(const FQuest_CraftingRecipe& Recipe,
                  const FQuest_PlayerInventory& Inventory) const;

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    bool TryCraft(const FName& RecipeID, FQuest_PlayerInventory& Inventory);

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    TArray<FQuest_CraftingRecipe> GetAvailableRecipes() const;

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    void OpenCraftingMenu();

protected:
    UFUNCTION()
    void OnInteractionOverlapBegin(UPrimitiveComponent* OverlappedComp,
        AActor* OtherActor, UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnInteractionOverlapEnd(UPrimitiveComponent* OverlappedComp,
        AActor* OtherActor, UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex);

    void InitDefaultRecipes();
};

// ─── Crafting Manager (GameInstance Subsystem style) ─────────────────────────

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_CraftingManager : public AActor
{
    GENERATED_BODY()

public:
    AQuest_CraftingManager();

    virtual void BeginPlay() override;

    // Global recipe registry
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    TArray<FQuest_CraftingRecipe> AllRecipes;

    // Player inventory (single player)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    FQuest_PlayerInventory PlayerInventory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    bool bCraftingMenuOpen = false;

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    void AddResource(EQuest_ResourceType Type, int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    int32 GetResourceCount(EQuest_ResourceType Type) const;

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    bool HasCraftedItem(EQuest_CraftedItemType Item) const;

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    bool TryCraftItem(EQuest_CraftedItemType Item);

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    void ToggleCraftingMenu();

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    FQuest_CraftingRecipe GetRecipeForItem(EQuest_CraftedItemType Item) const;

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    TArray<EQuest_CraftedItemType> GetCraftableItems() const;

protected:
    void RegisterDefaultRecipes();
};
