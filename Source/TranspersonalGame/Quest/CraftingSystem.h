#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "CraftingSystem.generated.h"

// ============================================================
// ENUMS — all at global scope, Quest_ prefix (unique project-wide)
// ============================================================

UENUM(BlueprintType)
enum class EQuest_ResourceType : uint8
{
    None        UMETA(DisplayName = "None"),
    Stone       UMETA(DisplayName = "Stone"),
    Stick       UMETA(DisplayName = "Stick"),
    Leaf        UMETA(DisplayName = "Leaf"),
    Bone        UMETA(DisplayName = "Bone"),
    Hide        UMETA(DisplayName = "Hide"),
    Flint       UMETA(DisplayName = "Flint"),
    Vine        UMETA(DisplayName = "Vine"),
    Water       UMETA(DisplayName = "Water"),
    Meat        UMETA(DisplayName = "Meat")
};

UENUM(BlueprintType)
enum class EQuest_CraftedItem : uint8
{
    None            UMETA(DisplayName = "None"),
    StoneAxe        UMETA(DisplayName = "Stone Axe"),
    Campfire        UMETA(DisplayName = "Campfire"),
    WaterContainer  UMETA(DisplayName = "Water Container"),
    Spear           UMETA(DisplayName = "Spear"),
    LeatherWrap     UMETA(DisplayName = "Leather Wrap"),
    BoneDagger      UMETA(DisplayName = "Bone Dagger"),
    Torch           UMETA(DisplayName = "Torch"),
    Trap            UMETA(DisplayName = "Trap")
};

UENUM(BlueprintType)
enum class EQuest_CraftResult : uint8
{
    Success         UMETA(DisplayName = "Success"),
    MissingIngredient UMETA(DisplayName = "Missing Ingredient"),
    InventoryFull   UMETA(DisplayName = "Inventory Full"),
    InvalidRecipe   UMETA(DisplayName = "Invalid Recipe")
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
    FString DisplayName = TEXT("Unknown Item");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    TArray<FQuest_CraftingIngredient> Ingredients;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    float CraftingTimeSeconds = 2.0f;
};

USTRUCT(BlueprintType)
struct FQuest_InventorySlot
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    EQuest_ResourceType ResourceType = EQuest_ResourceType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    int32 Count = 0;
};

// ============================================================
// CRAFTING MANAGER COMPONENT
// ============================================================

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UQuest_CraftingManagerComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuest_CraftingManagerComponent();

    // Inventory
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting|Inventory")
    TArray<FQuest_InventorySlot> Inventory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting|Inventory")
    int32 MaxInventorySlots = 20;

    // Recipes
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting|Recipes")
    TArray<FQuest_CraftingRecipe> KnownRecipes;

    // Crafting state
    UPROPERTY(BlueprintReadOnly, Category = "Crafting")
    bool bCraftingMenuOpen = false;

    UPROPERTY(BlueprintReadOnly, Category = "Crafting")
    EQuest_CraftedItem LastCraftedItem = EQuest_CraftedItem::None;

    // Core functions
    UFUNCTION(BlueprintCallable, Category = "Crafting")
    bool AddResource(EQuest_ResourceType ResourceType, int32 Amount = 1);

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    bool RemoveResource(EQuest_ResourceType ResourceType, int32 Amount = 1);

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    int32 GetResourceCount(EQuest_ResourceType ResourceType) const;

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    EQuest_CraftResult TryCraftItem(EQuest_CraftedItem ItemToCraft);

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    bool HasIngredientsForRecipe(const FQuest_CraftingRecipe& Recipe) const;

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    void ToggleCraftingMenu();

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    TArray<FQuest_CraftingRecipe> GetAvailableRecipes() const;

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Crafting")
    void InitializeDefaultRecipes();

protected:
    virtual void BeginPlay() override;

private:
    void RegisterDefaultRecipes();
};

// ============================================================
// RESOURCE PICKUP ACTOR
// ============================================================

UCLASS()
class TRANSPERSONALGAME_API AQuest_ResourcePickup : public AActor
{
    GENERATED_BODY()

public:
    AQuest_ResourcePickup();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    EQuest_ResourceType ResourceType = EQuest_ResourceType::Stone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    int32 ResourceAmount = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    bool bAutoRespawn = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    float RespawnTimeSeconds = 120.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components",
        meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UStaticMeshComponent> MeshComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components",
        meta = (AllowPrivateAccess = "true"))
    TObjectPtr<USphereComponent> PickupRadius;

    UFUNCTION(BlueprintCallable, Category = "Resource")
    void OnPickedUp(AActor* PickingActor);

    UFUNCTION(BlueprintCallable, Category = "Resource")
    void Respawn();

protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

private:
    FTimerHandle RespawnTimer;
    bool bIsAvailable = true;
};
