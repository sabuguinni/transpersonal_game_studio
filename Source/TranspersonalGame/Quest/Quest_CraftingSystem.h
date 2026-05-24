#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMeshActor.h"
#include "SharedTypes.h"
#include "Quest_CraftingSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_CraftingRecipe
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
    FString RecipeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
    TArray<FString> RequiredMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
    TArray<int32> MaterialQuantities;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
    FString ResultItem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
    float CraftingTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
    int32 ExperienceGained;

    FQuest_CraftingRecipe()
    {
        RecipeName = TEXT("");
        CraftingTime = 5.0f;
        ExperienceGained = 10;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_InventoryItem
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    FString ItemName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    int32 Quantity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    FString ItemType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    float Weight;

    FQuest_InventoryItem()
    {
        ItemName = TEXT("");
        Quantity = 0;
        ItemType = TEXT("Material");
        Weight = 1.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_ResourcePickup : public AStaticMeshActor
{
    GENERATED_BODY()

public:
    AQuest_ResourcePickup();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    FString ResourceName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    int32 ResourceQuantity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    float RespawnTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    bool bCanRespawn;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    float InteractionRange;

    UFUNCTION(BlueprintCallable, Category = "Resource")
    void OnPlayerInteract(class APawn* InteractingPawn);

    UFUNCTION(BlueprintCallable, Category = "Resource")
    void RespawnResource();

protected:
    virtual void BeginPlay() override;

private:
    bool bIsCollected;
    FTimerHandle RespawnTimerHandle;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQuest_CraftingComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuest_CraftingComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    TArray<FQuest_CraftingRecipe> AvailableRecipes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    TArray<FQuest_InventoryItem> PlayerInventory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    float CraftingSkillLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    bool bIsCrafting;

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    bool CanCraftItem(const FString& RecipeName);

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    bool StartCrafting(const FString& RecipeName);

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    void CompleteCrafting();

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool AddItemToInventory(const FString& ItemName, int32 Quantity);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool RemoveItemFromInventory(const FString& ItemName, int32 Quantity);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    int32 GetItemQuantity(const FString& ItemName);

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    void OpenCraftingMenu();

protected:
    virtual void BeginPlay() override;

private:
    FString CurrentCraftingRecipe;
    FTimerHandle CraftingTimerHandle;

    void InitializeBasicRecipes();
};

UCLASS()
class TRANSPERSONALGAME_API UQuest_CraftingSystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Crafting System")
    void SpawnResourcesInBiome(EBiomeType BiomeType, int32 ResourceCount);

    UFUNCTION(BlueprintCallable, Category = "Crafting System")
    void SpawnResourceAtLocation(const FVector& Location, const FString& ResourceType);

    UFUNCTION(BlueprintCallable, Category = "Crafting System")
    TArray<FQuest_CraftingRecipe> GetBasicSurvivalRecipes();

    UFUNCTION(BlueprintCallable, Category = "Crafting System")
    void RegisterCraftingComponent(UQuest_CraftingComponent* Component);

    UFUNCTION(BlueprintCallable, Category = "Crafting System")
    void UnregisterCraftingComponent(UQuest_CraftingComponent* Component);

protected:
    UPROPERTY()
    TArray<UQuest_CraftingComponent*> RegisteredCraftingComponents;

    UPROPERTY()
    TArray<AQuest_ResourcePickup*> SpawnedResources;

private:
    void CreateBasicSurvivalRecipes();
    FVector GetRandomLocationInBiome(EBiomeType BiomeType);
    TArray<FQuest_CraftingRecipe> BasicRecipes;
};