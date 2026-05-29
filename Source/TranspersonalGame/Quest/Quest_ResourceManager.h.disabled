#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "Quest_ResourceManager.generated.h"

UENUM(BlueprintType)
enum class EQuest_ResourceType : uint8
{
    None = 0,
    Stone,
    Wood,
    Plant,
    Water,
    Meat,
    Hide,
    Bone,
    Fire
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_ResourceData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Resource")
    EQuest_ResourceType Type = EQuest_ResourceType::None;

    UPROPERTY(BlueprintReadOnly, Category = "Resource")
    int32 Quantity = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Resource")
    float Quality = 1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Resource")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Resource")
    bool bIsRenewable = false;

    UPROPERTY(BlueprintReadOnly, Category = "Resource")
    float RespawnTime = 300.0f; // 5 minutes default

    FQuest_ResourceData()
    {
        Type = EQuest_ResourceType::None;
        Quantity = 0;
        Quality = 1.0f;
        Location = FVector::ZeroVector;
        bIsRenewable = false;
        RespawnTime = 300.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_CraftingRecipe
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Crafting")
    FString RecipeName;

    UPROPERTY(BlueprintReadOnly, Category = "Crafting")
    TArray<FQuest_ResourceData> RequiredResources;

    UPROPERTY(BlueprintReadOnly, Category = "Crafting")
    FQuest_ResourceData Result;

    UPROPERTY(BlueprintReadOnly, Category = "Crafting")
    float CraftingTime = 5.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Crafting")
    int32 ExperienceGained = 10;

    FQuest_CraftingRecipe()
    {
        RecipeName = TEXT("");
        CraftingTime = 5.0f;
        ExperienceGained = 10;
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UQuest_ResourceManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UQuest_ResourceManager();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Resource management
    UFUNCTION(BlueprintCallable, Category = "Quest|Resources")
    void AddResource(EQuest_ResourceType Type, int32 Quantity, float Quality = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Quest|Resources")
    bool ConsumeResource(EQuest_ResourceType Type, int32 Quantity);

    UFUNCTION(BlueprintPure, Category = "Quest|Resources")
    int32 GetResourceQuantity(EQuest_ResourceType Type) const;

    UFUNCTION(BlueprintPure, Category = "Quest|Resources")
    TArray<FQuest_ResourceData> GetAllResources() const;

    // Crafting system
    UFUNCTION(BlueprintCallable, Category = "Quest|Crafting")
    bool CanCraftItem(const FString& RecipeName) const;

    UFUNCTION(BlueprintCallable, Category = "Quest|Crafting")
    bool CraftItem(const FString& RecipeName);

    UFUNCTION(BlueprintPure, Category = "Quest|Crafting")
    TArray<FQuest_CraftingRecipe> GetAvailableRecipes() const;

    // Resource spawning
    UFUNCTION(BlueprintCallable, Category = "Quest|Resources")
    void SpawnResourceNode(EQuest_ResourceType Type, const FVector& Location, int32 Quantity = 1);

    UFUNCTION(BlueprintCallable, Category = "Quest|Resources")
    void PopulateWorldWithResources();

    // Quest integration
    UFUNCTION(BlueprintPure, Category = "Quest|Resources")
    bool HasRequiredResourcesForMission(const FString& MissionType) const;

    UFUNCTION(BlueprintCallable, Category = "Quest|Resources")
    void ConsumeResourcesForMission(const FString& MissionType);

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Quest|Resources")
    TMap<EQuest_ResourceType, int32> PlayerInventory;

    UPROPERTY(BlueprintReadOnly, Category = "Quest|Resources")
    TArray<FQuest_CraftingRecipe> CraftingRecipes;

    UPROPERTY(BlueprintReadOnly, Category = "Quest|Resources")
    TArray<FQuest_ResourceData> WorldResources;

    // Initialize default recipes
    void InitializeCraftingRecipes();
    void InitializeResourceSpawns();

    // Helper functions
    FQuest_ResourceData CreateResourceData(EQuest_ResourceType Type, int32 Quantity, float Quality = 1.0f) const;
    FQuest_CraftingRecipe CreateRecipe(const FString& Name, const TArray<FQuest_ResourceData>& Requirements, const FQuest_ResourceData& Result) const;
};