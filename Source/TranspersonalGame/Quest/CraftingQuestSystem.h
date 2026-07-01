#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "CraftingQuestSystem.generated.h"

// ── Enums ────────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class EQuest_ResourceType : uint8
{
    None        UMETA(DisplayName = "None"),
    Stone       UMETA(DisplayName = "Stone"),
    Stick       UMETA(DisplayName = "Stick"),
    Leaf        UMETA(DisplayName = "Leaf"),
    Bone        UMETA(DisplayName = "Bone"),
    Flint       UMETA(DisplayName = "Flint")
};

UENUM(BlueprintType)
enum class EQuest_CraftedItem : uint8
{
    None            UMETA(DisplayName = "None"),
    StoneAxe        UMETA(DisplayName = "Stone Axe"),
    Campfire        UMETA(DisplayName = "Campfire"),
    WaterContainer  UMETA(DisplayName = "Water Container"),
    SpearTip        UMETA(DisplayName = "Spear Tip")
};

UENUM(BlueprintType)
enum class EQuest_CraftingPhase : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    GatherMaterials UMETA(DisplayName = "Gather Materials"),
    CraftItem       UMETA(DisplayName = "Craft Item"),
    Completed       UMETA(DisplayName = "Completed"),
    Failed          UMETA(DisplayName = "Failed")
};

// ── Structs ──────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FQuest_RecipeIngredient
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Crafting")
    EQuest_ResourceType ResourceType = EQuest_ResourceType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Crafting")
    int32 RequiredCount = 1;

    UPROPERTY(BlueprintReadOnly, Category = "Quest|Crafting")
    int32 CollectedCount = 0;
};

USTRUCT(BlueprintType)
struct FQuest_CraftingRecipe
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Crafting")
    EQuest_CraftedItem OutputItem = EQuest_CraftedItem::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Crafting")
    FString RecipeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Crafting")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Crafting")
    TArray<FQuest_RecipeIngredient> Ingredients;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Crafting")
    float CraftingTimeSeconds = 3.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Quest|Crafting")
    bool bUnlocked = true;
};

USTRUCT(BlueprintType)
struct FQuest_PlayerInventory
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Quest|Inventory")
    int32 Stones = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Quest|Inventory")
    int32 Sticks = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Quest|Inventory")
    int32 Leaves = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Quest|Inventory")
    int32 Bones = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Quest|Inventory")
    int32 Flints = 0;

    int32 GetCount(EQuest_ResourceType Type) const;
    bool ConsumeIngredients(const TArray<FQuest_RecipeIngredient>& Ingredients);
};

// ── Resource Pickup Actor ────────────────────────────────────────────────────

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_ResourcePickup : public AActor
{
    GENERATED_BODY()

public:
    AQuest_ResourcePickup();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Resource")
    EQuest_ResourceType ResourceType = EQuest_ResourceType::Stone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Resource")
    int32 Quantity = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Resource")
    bool bAutoRespawn = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Resource")
    float RespawnDelaySeconds = 60.0f;

    UFUNCTION(BlueprintCallable, Category = "Quest|Resource")
    bool TryPickup(AActor* Collector);

    UFUNCTION(BlueprintCallable, Category = "Quest|Resource")
    void Respawn();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest|Resource",
        meta = (AllowPrivateAccess = "true"))
    UStaticMeshComponent* MeshComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest|Resource",
        meta = (AllowPrivateAccess = "true"))
    USphereComponent* PickupRadius;

    UPROPERTY(BlueprintReadOnly, Category = "Quest|Resource",
        meta = (AllowPrivateAccess = "true"))
    bool bAvailable = true;

    FTimerHandle RespawnTimer;

    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);
};

// ── Crafting Quest Manager ───────────────────────────────────────────────────

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_CraftingManager : public AActor
{
    GENERATED_BODY()

public:
    AQuest_CraftingManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Recipes
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Crafting")
    TArray<FQuest_CraftingRecipe> AvailableRecipes;

    // Player inventory (simplified — real game uses dedicated component)
    UPROPERTY(BlueprintReadWrite, Category = "Quest|Crafting")
    FQuest_PlayerInventory PlayerInventory;

    // Current quest state
    UPROPERTY(BlueprintReadOnly, Category = "Quest|Crafting")
    EQuest_CraftingPhase CurrentPhase = EQuest_CraftingPhase::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "Quest|Crafting")
    EQuest_CraftedItem ActiveCraftTarget = EQuest_CraftedItem::None;

    UPROPERTY(BlueprintReadOnly, Category = "Quest|Crafting")
    float CraftingProgress = 0.0f;

    // UI toggle
    UPROPERTY(BlueprintReadOnly, Category = "Quest|Crafting")
    bool bCraftingMenuOpen = false;

    // Methods
    UFUNCTION(BlueprintCallable, Category = "Quest|Crafting")
    void OpenCraftingMenu();

    UFUNCTION(BlueprintCallable, Category = "Quest|Crafting")
    void CloseCraftingMenu();

    UFUNCTION(BlueprintCallable, Category = "Quest|Crafting")
    bool StartCrafting(EQuest_CraftedItem Item);

    UFUNCTION(BlueprintCallable, Category = "Quest|Crafting")
    void CancelCrafting();

    UFUNCTION(BlueprintCallable, Category = "Quest|Crafting")
    bool CanCraft(EQuest_CraftedItem Item) const;

    UFUNCTION(BlueprintCallable, Category = "Quest|Crafting")
    void AddResource(EQuest_ResourceType Type, int32 Amount = 1);

    UFUNCTION(BlueprintCallable, Category = "Quest|Crafting")
    FQuest_CraftingRecipe GetRecipe(EQuest_CraftedItem Item) const;

    UFUNCTION(BlueprintCallable, Category = "Quest|Crafting")
    TArray<FQuest_CraftingRecipe> GetCraftableRecipes() const;

    // Quest objective: craft first tool
    UFUNCTION(BlueprintCallable, Category = "Quest|Crafting")
    void StartFirstToolQuest();

    UPROPERTY(BlueprintReadOnly, Category = "Quest|Crafting")
    bool bFirstToolQuestActive = false;

    UPROPERTY(BlueprintReadOnly, Category = "Quest|Crafting")
    bool bFirstToolQuestComplete = false;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemCrafted, EQuest_CraftedItem, Item);
    UPROPERTY(BlueprintAssignable, Category = "Quest|Crafting")
    FOnItemCrafted OnItemCrafted;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnResourceCollected, EQuest_ResourceType, Type, int32, Total);
    UPROPERTY(BlueprintAssignable, Category = "Quest|Crafting")
    FOnResourceCollected OnResourceCollected;

private:
    void InitDefaultRecipes();
    void TickCrafting(float DeltaTime);
    void CompleteCrafting();
    FQuest_CraftingRecipe* FindRecipe(EQuest_CraftedItem Item);

    float CraftingTimer = 0.0f;
    float ActiveCraftDuration = 0.0f;
};
