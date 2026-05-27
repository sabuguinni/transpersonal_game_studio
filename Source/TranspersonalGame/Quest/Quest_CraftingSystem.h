#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "TranspersonalGame/SharedTypes.h"
#include "Quest_CraftingSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_CraftingRecipe : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe")
    FString RecipeName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe")
    TArray<FString> RequiredItems;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe")
    TArray<int32> RequiredQuantities;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe")
    FString ResultItem;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe")
    int32 ResultQuantity;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe")
    float CraftingTime;

    FQuest_CraftingRecipe()
    {
        RecipeName = TEXT("Unknown Recipe");
        ResultItem = TEXT("Unknown Item");
        ResultQuantity = 1;
        CraftingTime = 3.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_InventoryItem
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
    FString ItemName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
    int32 Quantity;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
    FString ItemType;

    FQuest_InventoryItem()
    {
        ItemName = TEXT("Unknown");
        Quantity = 0;
        ItemType = TEXT("Resource");
    }
};

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
    USphereComponent* CollisionSphere;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Resource")
    FString ResourceName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Resource")
    int32 ResourceQuantity;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Resource")
    FString ResourceType;

    UFUNCTION()
    void OnPlayerOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
                        UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, 
                        bool bFromSweep, const FHitResult& SweepResult);

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Resource")
    FString GetResourceName() const { return ResourceName; }

    UFUNCTION(BlueprintCallable, Category = "Resource")
    int32 GetResourceQuantity() const { return ResourceQuantity; }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQuest_CraftingSystem : public UObject
{
    GENERATED_BODY()

public:
    UQuest_CraftingSystem();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Crafting")
    UDataTable* RecipeDataTable;

    UPROPERTY(BlueprintReadOnly, Category = "Inventory")
    TArray<FQuest_InventoryItem> PlayerInventory;

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    bool CanCraftItem(const FString& RecipeName);

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    bool CraftItem(const FString& RecipeName);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void AddItemToInventory(const FString& ItemName, int32 Quantity, const FString& ItemType);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    int32 GetItemQuantity(const FString& ItemName);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool RemoveItemFromInventory(const FString& ItemName, int32 Quantity);

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    TArray<FQuest_CraftingRecipe> GetAvailableRecipes();

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    TArray<FQuest_InventoryItem> GetInventoryItems() const { return PlayerInventory; }

protected:
    void InitializeDefaultRecipes();
    FQuest_CraftingRecipe* FindRecipe(const FString& RecipeName);
};