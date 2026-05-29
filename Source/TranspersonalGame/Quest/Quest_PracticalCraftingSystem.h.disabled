#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Quest_PracticalCraftingSystem.generated.h"

UENUM(BlueprintType)
enum class EQuest_CraftingMaterial : uint8
{
    Stone       UMETA(DisplayName = "Stone"),
    Wood        UMETA(DisplayName = "Wood"),
    Stick       UMETA(DisplayName = "Stick"),
    Leaf        UMETA(DisplayName = "Leaf"),
    Meat        UMETA(DisplayName = "Meat"),
    Hide        UMETA(DisplayName = "Hide"),
    Bone        UMETA(DisplayName = "Bone")
};

USTRUCT(BlueprintType)
struct FQuest_CraftingRecipe
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString RecipeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<EQuest_CraftingMaterial, int32> RequiredMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ResultItemName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CraftingTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ExperienceReward;

    FQuest_CraftingRecipe()
    {
        RecipeName = TEXT("");
        ResultItemName = TEXT("");
        CraftingTime = 5;
        ExperienceReward = 10;
    }
};

USTRUCT(BlueprintType)
struct FQuest_PlayerInventory
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<EQuest_CraftingMaterial, int32> Materials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> CraftedItems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CraftingExperience;

    FQuest_PlayerInventory()
    {
        CraftingExperience = 0;
    }
};

UCLASS(Blueprintable, BlueprintType)
class TRANSPERSONALGAME_API AQuest_PracticalCraftingSystem : public AActor
{
    GENERATED_BODY()

public:
    AQuest_PracticalCraftingSystem();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting System")
    TArray<FQuest_CraftingRecipe> AvailableRecipes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting System")
    FQuest_PlayerInventory PlayerInventory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting System")
    bool bIsCraftingMenuOpen;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting System")
    float InteractionRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting System")
    TArray<AActor*> NearbyResources;

public:
    virtual void Tick(float DeltaTime) override;

    // Funções principais do sistema de crafting
    UFUNCTION(BlueprintCallable, Category = "Crafting")
    void InitializeCraftingRecipes();

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    bool CanCraftItem(const FString& RecipeName);

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    bool CraftItem(const FString& RecipeName);

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    void AddMaterial(EQuest_CraftingMaterial MaterialType, int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    int32 GetMaterialCount(EQuest_CraftingMaterial MaterialType);

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    TArray<FQuest_CraftingRecipe> GetAvailableRecipes();

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    void ToggleCraftingMenu();

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    void ScanForNearbyResources();

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    void CollectResource(AActor* ResourceActor);

    // Funções de validação e feedback
    UFUNCTION(BlueprintCallable, Category = "Crafting")
    FString GetCraftingRequirementsText(const FString& RecipeName);

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    void ShowCraftingNotification(const FString& Message);

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    void UpdateCraftingProgress(float Progress);

    // Eventos para integração com sistema de quests
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemCrafted, FString, ItemName, int32, ExperienceGained);
    UPROPERTY(BlueprintAssignable, Category = "Crafting Events")
    FOnItemCrafted OnItemCrafted;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMaterialCollected, EQuest_CraftingMaterial, MaterialType, int32, Amount);
    UPROPERTY(BlueprintAssignable, Category = "Crafting Events")
    FOnMaterialCollected OnMaterialCollected;
};