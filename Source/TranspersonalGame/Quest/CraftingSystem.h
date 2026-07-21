#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "CraftingSystem.generated.h"

// ─────────────────────────────────────────────────────────────────────────────
// Enums — global scope (UHT requirement)
// ─────────────────────────────────────────────────────────────────────────────

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
    Vine        UMETA(DisplayName = "Vine"),
    Meat        UMETA(DisplayName = "Meat"),
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
};

// ─────────────────────────────────────────────────────────────────────────────
// Structs — global scope
// ─────────────────────────────────────────────────────────────────────────────

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
    EQuest_CraftedItem OutputItem = EQuest_CraftedItem::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    FString DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    TArray<FQuest_ResourceStack> Ingredients;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    float CraftingTimeSec = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    bool bRequiresCampfire = false;
};

// ─────────────────────────────────────────────────────────────────────────────
// AQuest_ResourcePickup — world pickup actor (rock, stick, leaf, etc.)
// ─────────────────────────────────────────────────────────────────────────────

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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    bool bAutoRespawn = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    float RespawnDelaySec = 120.0f;

    UFUNCTION(BlueprintCallable, Category = "Resource")
    bool TryPickup(AActor* Collector);

    UFUNCTION(BlueprintCallable, Category = "Resource")
    FString GetResourceLabel() const;

protected:
    virtual void BeginPlay() override;

private:
    bool bPickedUp = false;
    FTimerHandle RespawnTimer;

    UFUNCTION()
    void HandleRespawn();
};

// ─────────────────────────────────────────────────────────────────────────────
// UCraftingSystemComponent — attached to player character
// ─────────────────────────────────────────────────────────────────────────────

UCLASS(ClassGroup = (Quest), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCraftingSystemComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCraftingSystemComponent();

    // Inventory
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crafting")
    TArray<FQuest_ResourceStack> Inventory;

    // All available recipes (populated in BeginPlay)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crafting")
    TArray<FQuest_CraftingRecipe> AllRecipes;

    // Currently crafting
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crafting")
    bool bIsCrafting = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crafting")
    EQuest_CraftedItem CurrentlyCraftingItem = EQuest_CraftedItem::None;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crafting")
    float CraftingProgress = 0.0f;

    // ── Inventory API ──
    UFUNCTION(BlueprintCallable, Category = "Crafting")
    void AddResource(EQuest_ResourceType Type, int32 Amount = 1);

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    int32 GetResourceCount(EQuest_ResourceType Type) const;

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    bool RemoveResource(EQuest_ResourceType Type, int32 Amount);

    // ── Recipe API ──
    UFUNCTION(BlueprintCallable, Category = "Crafting")
    bool CanCraft(EQuest_CraftedItem Item) const;

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    bool StartCrafting(EQuest_CraftedItem Item);

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    void CancelCrafting();

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    TArray<FQuest_CraftingRecipe> GetAvailableRecipes() const;

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    FQuest_CraftingRecipe GetRecipeForItem(EQuest_CraftedItem Item) const;

    // ── Input trigger ──
    UFUNCTION(BlueprintCallable, Category = "Crafting")
    void OpenCraftingMenu();

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    void CloseCraftingMenu();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crafting")
    bool bCraftingMenuOpen = false;

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                               FActorComponentTickFunction* ThisTickFunction) override;

private:
    void RegisterDefaultRecipes();
    void CompleteCrafting();

    float CraftingTimeRemaining = 0.0f;
    EQuest_CraftedItem PendingItem = EQuest_CraftedItem::None;
};
