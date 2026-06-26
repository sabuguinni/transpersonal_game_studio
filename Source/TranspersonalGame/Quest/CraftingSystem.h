#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "CraftingSystem.generated.h"

// ============================================================
// CRAFTING SYSTEM — Agent #14 Quest & Mission Designer
// Recipes: Stone Axe, Campfire, Water Container
// Resource pickups: Rock, Stick, Leaf
// ============================================================

UENUM(BlueprintType)
enum class EQuest_ResourceType : uint8
{
    None        UMETA(DisplayName = "None"),
    Rock        UMETA(DisplayName = "Rock"),
    Stick       UMETA(DisplayName = "Stick"),
    Leaf        UMETA(DisplayName = "Leaf"),
    Bone        UMETA(DisplayName = "Bone"),
    Flint       UMETA(DisplayName = "Flint"),
    Vine        UMETA(DisplayName = "Vine"),
};

UENUM(BlueprintType)
enum class EQuest_CraftedItem : uint8
{
    None            UMETA(DisplayName = "None"),
    StoneAxe        UMETA(DisplayName = "Stone Axe"),
    Campfire        UMETA(DisplayName = "Campfire"),
    WaterContainer  UMETA(DisplayName = "Water Container"),
    WoodenSpear     UMETA(DisplayName = "Wooden Spear"),
    LeafShelter     UMETA(DisplayName = "Leaf Shelter"),
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
    EQuest_CraftedItem OutputItem = EQuest_CraftedItem::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    FString RecipeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    TArray<FQuest_CraftingIngredient> Ingredients;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    float CraftingTimeSeconds = 2.0f;
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
    int32 Flints = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    int32 Vines = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    TArray<EQuest_CraftedItem> CraftedItems;
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

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    void OnPlayerOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    EQuest_ResourceType GetResourceType() const { return ResourceType; }

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    int32 GetQuantity() const { return Quantity; }

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    bool IsCollected() const { return bCollected; }

protected:
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
    bool bRespawns = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    float RespawnTimeSeconds = 60.0f;

private:
    bool bCollected = false;
    float RespawnTimer = 0.0f;

    void ResetPickup();
};

// ============================================================
// Crafting Manager Component — attached to PlayerCharacter
// ============================================================
UCLASS(BlueprintType, Blueprintable, ClassGroup = (Custom),
    meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UQuest_CraftingManagerComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuest_CraftingManagerComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
        FActorComponentTickFunction* ThisTickFunction) override;

    // ---- Inventory ----
    UFUNCTION(BlueprintCallable, Category = "Crafting")
    void AddResource(EQuest_ResourceType Type, int32 Amount = 1);

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    int32 GetResourceCount(EQuest_ResourceType Type) const;

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    bool HasIngredients(const FQuest_CraftingRecipe& Recipe) const;

    // ---- Crafting ----
    UFUNCTION(BlueprintCallable, Category = "Crafting")
    bool StartCrafting(EQuest_CraftedItem Item);

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    bool IsCrafting() const { return bIsCrafting; }

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    float GetCraftingProgress() const;

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    TArray<FQuest_CraftingRecipe> GetAvailableRecipes() const;

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    FQuest_PlayerInventory GetInventory() const { return Inventory; }

    // ---- UI Toggle ----
    UFUNCTION(BlueprintCallable, Category = "Crafting")
    void ToggleCraftingMenu();

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    bool IsCraftingMenuOpen() const { return bCraftingMenuOpen; }

    UPROPERTY(BlueprintAssignable, Category = "Crafting")
    FSimpleMulticastDelegate OnCraftingMenuToggled;

    UPROPERTY(BlueprintAssignable, Category = "Crafting")
    FSimpleMulticastDelegate OnItemCrafted;

    UPROPERTY(BlueprintAssignable, Category = "Crafting")
    FSimpleMulticastDelegate OnResourceCollected;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    FQuest_PlayerInventory Inventory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    TArray<FQuest_CraftingRecipe> Recipes;

private:
    bool bIsCrafting = false;
    bool bCraftingMenuOpen = false;
    float CraftingElapsed = 0.0f;
    float CraftingDuration = 0.0f;
    EQuest_CraftedItem CurrentlyCrafting = EQuest_CraftedItem::None;

    void InitializeRecipes();
    void CompleteCrafting();
    void ConsumeIngredients(const FQuest_CraftingRecipe& Recipe);
};
