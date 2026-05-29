#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "../SharedTypes.h"
#include "Quest_CraftingSystem.generated.h"

// Recipe definition for crafting system
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_CraftingRecipe
{
    GENERATED_BODY()

    // Recipe ID
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
    FString RecipeID;

    // Display name for UI
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
    FString DisplayName;

    // Description of what this recipe creates
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
    FString Description;

    // Required materials
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
    TMap<FString, int32> RequiredMaterials;

    // Result item
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
    FString ResultItem;

    // Quantity produced
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
    int32 ResultQuantity;

    // Crafting time in seconds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
    float CraftingTime;

    // Experience gained
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
    int32 ExperienceGained;

    FQuest_CraftingRecipe()
    {
        RecipeID = "";
        DisplayName = "";
        Description = "";
        RequiredMaterials.Empty();
        ResultItem = "";
        ResultQuantity = 1;
        CraftingTime = 5.0f;
        ExperienceGained = 10;
    }
};

// Material pickup item
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_CraftingMaterial
{
    GENERATED_BODY()

    // Material ID
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material")
    FString MaterialID;

    // Display name
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material")
    FString DisplayName;

    // Stack size limit
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material")
    int32 MaxStackSize;

    // Rarity level
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material")
    int32 RarityLevel;

    FQuest_CraftingMaterial()
    {
        MaterialID = "";
        DisplayName = "";
        MaxStackSize = 50;
        RarityLevel = 1;
    }
};

// Player inventory for crafting materials
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQuest_CraftingInventory : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuest_CraftingInventory();

    // Current materials in inventory
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    TMap<FString, int32> Materials;

    // Add material to inventory
    UFUNCTION(BlueprintCallable, Category = "Crafting")
    bool AddMaterial(const FString& MaterialID, int32 Quantity);

    // Remove material from inventory
    UFUNCTION(BlueprintCallable, Category = "Crafting")
    bool RemoveMaterial(const FString& MaterialID, int32 Quantity);

    // Check if has required materials for recipe
    UFUNCTION(BlueprintCallable, Category = "Crafting")
    bool HasMaterialsForRecipe(const FQuest_CraftingRecipe& Recipe);

    // Get material count
    UFUNCTION(BlueprintCallable, Category = "Crafting")
    int32 GetMaterialCount(const FString& MaterialID);

protected:
    virtual void BeginPlay() override;
};

// Main crafting system component
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UQuest_CraftingSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuest_CraftingSystem();

    // Available recipes
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    TArray<FQuest_CraftingRecipe> AvailableRecipes;

    // Current crafting progress
    UPROPERTY(BlueprintReadOnly, Category = "Crafting")
    bool bIsCrafting;

    UPROPERTY(BlueprintReadOnly, Category = "Crafting")
    FString CurrentRecipeID;

    UPROPERTY(BlueprintReadOnly, Category = "Crafting")
    float CraftingProgress;

    UPROPERTY(BlueprintReadOnly, Category = "Crafting")
    float CraftingTimeRemaining;

    // Player inventory reference
    UPROPERTY(BlueprintReadOnly, Category = "Crafting")
    UQuest_CraftingInventory* PlayerInventory;

    // Initialize crafting system with basic recipes
    UFUNCTION(BlueprintCallable, Category = "Crafting")
    void InitializeCraftingSystem();

    // Start crafting a recipe
    UFUNCTION(BlueprintCallable, Category = "Crafting")
    bool StartCrafting(const FString& RecipeID);

    // Cancel current crafting
    UFUNCTION(BlueprintCallable, Category = "Crafting")
    void CancelCrafting();

    // Update crafting progress (called by tick)
    UFUNCTION(BlueprintCallable, Category = "Crafting")
    void UpdateCrafting(float DeltaTime);

    // Complete current crafting
    UFUNCTION(BlueprintCallable, Category = "Crafting")
    void CompleteCrafting();

    // Get recipe by ID
    UFUNCTION(BlueprintCallable, Category = "Crafting")
    FQuest_CraftingRecipe GetRecipeByID(const FString& RecipeID);

    // Check if recipe is available
    UFUNCTION(BlueprintCallable, Category = "Crafting")
    bool CanCraftRecipe(const FString& RecipeID);

    // Get all craftable recipes
    UFUNCTION(BlueprintCallable, Category = "Crafting")
    TArray<FQuest_CraftingRecipe> GetCraftableRecipes();

    // Open crafting UI (to be implemented in Blueprint)
    UFUNCTION(BlueprintImplementableEvent, Category = "Crafting")
    void OpenCraftingUI();

    // Close crafting UI
    UFUNCTION(BlueprintImplementableEvent, Category = "Crafting")
    void CloseCraftingUI();

    // Events for UI updates
    UFUNCTION(BlueprintImplementableEvent, Category = "Crafting")
    void OnCraftingStarted(const FString& RecipeID);

    UFUNCTION(BlueprintImplementableEvent, Category = "Crafting")
    void OnCraftingProgress(float Progress);

    UFUNCTION(BlueprintImplementableEvent, Category = "Crafting")
    void OnCraftingCompleted(const FString& ItemCreated);

    UFUNCTION(BlueprintImplementableEvent, Category = "Crafting")
    void OnCraftingCancelled();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    // Setup basic survival recipes
    void SetupBasicRecipes();

    // Find player inventory component
    void FindPlayerInventory();

    // Timer handle for crafting
    FTimerHandle CraftingTimerHandle;
};

// Resource pickup actor for materials
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_ResourcePickup : public AActor
{
    GENERATED_BODY()

public:
    AQuest_ResourcePickup();

    // Material this pickup provides
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    FString MaterialID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    FString DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    int32 Quantity;

    // Mesh component
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UStaticMeshComponent* MeshComponent;

    // Collision component
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USphereComponent* CollisionComponent;

    // Pickup interaction
    UFUNCTION(BlueprintCallable, Category = "Resource")
    void OnPickup(AActor* Player);

    // Check if player can pickup
    UFUNCTION(BlueprintCallable, Category = "Resource")
    bool CanPickup(AActor* Player);

protected:
    virtual void BeginPlay() override;

    // Collision events
    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
    // Respawn timer for resource regeneration
    UPROPERTY(EditAnywhere, Category = "Resource")
    float RespawnTime;

    UPROPERTY(EditAnywhere, Category = "Resource")
    bool bRespawns;

    FTimerHandle RespawnTimerHandle;

    void RespawnResource();
};