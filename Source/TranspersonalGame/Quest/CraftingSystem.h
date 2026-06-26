#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "CraftingSystem.generated.h"

// ============================================================
// Enums — must be at global scope (UHT rule)
// ============================================================

UENUM(BlueprintType)
enum class EQuest_ResourceType : uint8
{
    None        UMETA(DisplayName = "None"),
    Rock        UMETA(DisplayName = "Rock"),
    Stick       UMETA(DisplayName = "Stick"),
    Leaf        UMETA(DisplayName = "Leaf"),
    Bone        UMETA(DisplayName = "Bone"),
    Hide        UMETA(DisplayName = "Hide"),
    Flint       UMETA(DisplayName = "Flint"),
};

UENUM(BlueprintType)
enum class EQuest_CraftedItem : uint8
{
    None            UMETA(DisplayName = "None"),
    StoneAxe        UMETA(DisplayName = "Stone Axe"),
    Campfire        UMETA(DisplayName = "Campfire"),
    WaterContainer  UMETA(DisplayName = "Water Container"),
    SpearTip        UMETA(DisplayName = "Spear Tip"),
    BoneDagger      UMETA(DisplayName = "Bone Dagger"),
};

// ============================================================
// Structs — must be at global scope (UHT rule)
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
    EQuest_CraftedItem ResultItem = EQuest_CraftedItem::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    FString RecipeName;

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
    int32 Rocks = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    int32 Sticks = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    int32 Leaves = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    int32 Bones = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    int32 Hides = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    int32 Flints = 0;
};

// ============================================================
// Resource Pickup Actor
// ============================================================

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_ResourcePickup : public AActor
{
    GENERATED_BODY()

public:
    AQuest_ResourcePickup();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    UStaticMeshComponent* MeshComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    UBoxComponent* PickupCollision;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    EQuest_ResourceType ResourceType = EQuest_ResourceType::Rock;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    int32 ResourceQuantity = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    bool bHasBeenPickedUp = false;

    UFUNCTION(BlueprintCallable, Category = "Resource")
    void PickupResource(AActor* Collector);

    UFUNCTION(BlueprintCallable, Category = "Resource")
    EQuest_ResourceType GetResourceType() const { return ResourceType; }

protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    void OnPickupOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);
};

// ============================================================
// Crafting Station Actor
// ============================================================

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_CraftingStation : public AActor
{
    GENERATED_BODY()

public:
    AQuest_CraftingStation();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    UStaticMeshComponent* StationMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    UBoxComponent* InteractionZone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    TArray<FQuest_CraftingRecipe> AvailableRecipes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    bool bPlayerInRange = false;

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    bool CanCraftItem(EQuest_CraftedItem Item, const FQuest_PlayerInventory& Inventory) const;

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    bool CraftItem(EQuest_CraftedItem Item, FQuest_PlayerInventory& Inventory);

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    TArray<FQuest_CraftingRecipe> GetAvailableRecipes() const { return AvailableRecipes; }

protected:
    virtual void BeginPlay() override;
    void InitializeDefaultRecipes();

    UFUNCTION()
    void OnInteractionZoneBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnInteractionZoneEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};
