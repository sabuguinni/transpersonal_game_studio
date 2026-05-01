#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Quest_CraftingSystem.generated.h"

// Tipos de recursos para crafting
UENUM(BlueprintType)
enum class EQuest_ResourceType : uint8
{
    None = 0,
    Rock = 1,
    Stick = 2,
    Leaf = 3,
    Bone = 4,
    Fiber = 5,
    Water = 6
};

// Estrutura de um item de crafting
USTRUCT(BlueprintType)
struct FQuest_CraftingItem
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    FString ItemName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    EQuest_ResourceType ItemType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    int32 Quantity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    float Weight;

    FQuest_CraftingItem()
    {
        ItemName = TEXT("Unknown Item");
        Description = TEXT("");
        ItemType = EQuest_ResourceType::None;
        Quantity = 1;
        Weight = 1.0f;
    }
};

// Estrutura de uma receita de crafting
USTRUCT(BlueprintType)
struct FQuest_CraftingRecipe
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
    FString RecipeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
    FString RecipeDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
    TArray<FQuest_CraftingItem> RequiredItems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
    FQuest_CraftingItem ResultItem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
    float CraftingTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
    bool bRequiresCampfire;

    FQuest_CraftingRecipe()
    {
        RecipeName = TEXT("Unknown Recipe");
        RecipeDescription = TEXT("");
        CraftingTime = 5.0f;
        bRequiresCampfire = false;
    }
};

// Actor para recursos coletáveis
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_ResourcePickup : public AActor
{
    GENERATED_BODY()

public:
    AQuest_ResourcePickup();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* MeshComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* CollisionComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    FQuest_CraftingItem ResourceItem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    float RespawnTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    bool bCanRespawn;

    UPROPERTY(BlueprintReadOnly, Category = "Resource")
    bool bIsCollected;

    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION(BlueprintCallable, Category = "Resource")
    void CollectResource(AActor* Collector);

    UFUNCTION(BlueprintCallable, Category = "Resource")
    void RespawnResource();

private:
    FTimerHandle RespawnTimerHandle;
};

// Componente de inventário para crafting
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UQuest_CraftingInventoryComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuest_CraftingInventoryComponent();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    TArray<FQuest_CraftingItem> InventoryItems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    int32 MaxInventorySlots;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    float MaxCarryWeight;

public:
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool AddItem(const FQuest_CraftingItem& Item);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool RemoveItem(EQuest_ResourceType ItemType, int32 Quantity);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    int32 GetItemCount(EQuest_ResourceType ItemType) const;

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    float GetCurrentWeight() const;

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool CanAddItem(const FQuest_CraftingItem& Item) const;

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    TArray<FQuest_CraftingItem> GetInventoryItems() const { return InventoryItems; }
};

// Sistema principal de crafting
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_CraftingSystem : public AActor
{
    GENERATED_BODY()

public:
    AQuest_CraftingSystem();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    TArray<FQuest_CraftingRecipe> AvailableRecipes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    bool bIsCraftingMenuOpen;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    AActor* CurrentCrafter;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    void InitializeRecipes();

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    bool CanCraftRecipe(const FQuest_CraftingRecipe& Recipe, UQuest_CraftingInventoryComponent* Inventory) const;

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    bool CraftItem(const FQuest_CraftingRecipe& Recipe, UQuest_CraftingInventoryComponent* Inventory);

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    void OpenCraftingMenu(AActor* Crafter);

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    void CloseCraftingMenu();

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    TArray<FQuest_CraftingRecipe> GetAvailableRecipes() const { return AvailableRecipes; }

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    FQuest_CraftingRecipe GetRecipeByName(const FString& RecipeName) const;

private:
    void CreateBasicRecipes();
    void HandleCraftingInput();
};