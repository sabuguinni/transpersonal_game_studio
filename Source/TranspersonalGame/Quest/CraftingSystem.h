#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CraftingSystem.generated.h"

// ============================================================
// CraftingSystem.h — Agent #14 Quest & Mission Designer
// Prehistoric survival crafting: Stone Axe, Campfire, Water Container
// ============================================================

UENUM(BlueprintType)
enum class EQuest_ResourceType : uint8
{
    Rock        UMETA(DisplayName = "Rock"),
    Stick       UMETA(DisplayName = "Stick"),
    Leaf        UMETA(DisplayName = "Leaf"),
    Bone        UMETA(DisplayName = "Bone"),
    Hide        UMETA(DisplayName = "Hide"),
    Flint       UMETA(DisplayName = "Flint"),
    None        UMETA(DisplayName = "None")
};

UENUM(BlueprintType)
enum class EQuest_CraftedItem : uint8
{
    StoneAxe        UMETA(DisplayName = "Stone Axe"),
    Campfire        UMETA(DisplayName = "Campfire"),
    WaterContainer  UMETA(DisplayName = "Water Container"),
    Spear           UMETA(DisplayName = "Spear"),
    SkinWrap        UMETA(DisplayName = "Skin Wrap"),
    None            UMETA(DisplayName = "None")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_ResourceStack
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    EQuest_ResourceType ResourceType = EQuest_ResourceType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    int32 Quantity = 0;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_CraftingRecipe
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    EQuest_CraftedItem ResultItem = EQuest_CraftedItem::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    FString RecipeName = TEXT("Unknown Recipe");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    FString Description = TEXT("");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    TArray<FQuest_ResourceStack> RequiredResources;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    float CraftingTimeSeconds = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    bool bRequiresCampfire = false;
};

// ============================================================
// ACraftingSystem — Main crafting actor placed in level
// ============================================================
UCLASS(BlueprintType, Blueprintable, ClassGroup = "Quest")
class TRANSPERSONALGAME_API ACraftingSystem : public AActor
{
    GENERATED_BODY()

public:
    ACraftingSystem();

protected:
    virtual void BeginPlay() override;

public:
    // --- Inventory ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting|Inventory")
    TArray<FQuest_ResourceStack> PlayerInventory;

    // --- Recipes ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting|Recipes")
    TArray<FQuest_CraftingRecipe> AvailableRecipes;

    // --- State ---
    UPROPERTY(BlueprintReadOnly, Category = "Crafting|State")
    bool bCraftingMenuOpen = false;

    UPROPERTY(BlueprintReadOnly, Category = "Crafting|State")
    bool bIsCrafting = false;

    UPROPERTY(BlueprintReadOnly, Category = "Crafting|State")
    EQuest_CraftedItem CurrentlyCraftingItem = EQuest_CraftedItem::None;

    UPROPERTY(BlueprintReadOnly, Category = "Crafting|State")
    float CraftingProgress = 0.0f;

    // --- Methods ---
    UFUNCTION(BlueprintCallable, Category = "Crafting")
    void ToggleCraftingMenu();

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    bool CanCraft(EQuest_CraftedItem Item) const;

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    bool StartCrafting(EQuest_CraftedItem Item);

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    void AddResource(EQuest_ResourceType ResourceType, int32 Amount = 1);

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    int32 GetResourceCount(EQuest_ResourceType ResourceType) const;

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    TArray<FQuest_CraftingRecipe> GetCraftableRecipes() const;

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    void InitializeDefaultRecipes();

    virtual void Tick(float DeltaTime) override;

private:
    void ConsumeResources(const FQuest_CraftingRecipe& Recipe);
    void CompleteCrafting(EQuest_CraftedItem Item);
    float CraftingTimer = 0.0f;
    float CraftingDuration = 0.0f;
};
