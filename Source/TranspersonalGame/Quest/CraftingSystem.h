#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "CraftingSystem.generated.h"

// ── Enums ────────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class EQuest_ResourceType : uint8
{
    None        UMETA(DisplayName = "None"),
    Flint       UMETA(DisplayName = "Flint Stone"),
    Stick       UMETA(DisplayName = "Dry Stick"),
    Leaf        UMETA(DisplayName = "Large Leaf"),
    Bone        UMETA(DisplayName = "Animal Bone"),
    Hide        UMETA(DisplayName = "Animal Hide"),
    Vine        UMETA(DisplayName = "Vine"),
};

UENUM(BlueprintType)
enum class EQuest_CraftedItemType : uint8
{
    None            UMETA(DisplayName = "None"),
    StoneAxe        UMETA(DisplayName = "Stone Axe"),
    Campfire        UMETA(DisplayName = "Campfire"),
    WaterContainer  UMETA(DisplayName = "Water Container"),
    Spear           UMETA(DisplayName = "Spear"),
    SkinPouch       UMETA(DisplayName = "Skin Pouch"),
};

// ── Structs ───────────────────────────────────────────────────────────────────

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
    FString DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    TArray<FQuest_CraftingIngredient> Ingredients;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    float CraftTimeSeconds = 3.0f;
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
    int32 HideCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    int32 VineCount = 0;
};

// ── Resource Pickup Actor ─────────────────────────────────────────────────────

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_ResourcePickup : public AActor
{
    GENERATED_BODY()

public:
    AQuest_ResourcePickup();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    EQuest_ResourceType ResourceType = EQuest_ResourceType::Flint;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    int32 QuantityOnPickup = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    bool bHasBeenPickedUp = false;

    UFUNCTION(BlueprintCallable, Category = "Resource")
    void PickUp(AActor* PickingActor);

    UFUNCTION(BlueprintCallable, Category = "Resource")
    FString GetResourceDisplayName() const;

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    UStaticMeshComponent* MeshComponent;
};

// ── Crafting System Component ─────────────────────────────────────────────────

UCLASS(BlueprintType, Blueprintable, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UQuest_CraftingComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuest_CraftingComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    FQuest_PlayerInventory Inventory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    TArray<FQuest_CraftingRecipe> AvailableRecipes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    bool bCraftingMenuOpen = false;

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    void InitialiseRecipes();

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    bool CanCraft(EQuest_CraftedItemType ItemType) const;

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    bool CraftItem(EQuest_CraftedItemType ItemType);

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    void AddResource(EQuest_ResourceType ResourceType, int32 Quantity = 1);

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    int32 GetResourceCount(EQuest_ResourceType ResourceType) const;

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    void ToggleCraftingMenu();

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    TArray<FQuest_CraftingRecipe> GetCraftableRecipes() const;

protected:
    virtual void BeginPlay() override;

private:
    void ConsumeIngredients(const FQuest_CraftingRecipe& Recipe);
};
