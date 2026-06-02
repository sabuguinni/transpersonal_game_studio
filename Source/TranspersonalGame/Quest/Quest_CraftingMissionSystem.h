#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "../SharedTypes.h"
#include "Quest_CraftingMissionSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_CraftingRecipe
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting Recipe")
    FString RecipeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting Recipe")
    TArray<FString> RequiredItems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting Recipe")
    TArray<int32> RequiredQuantities;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting Recipe")
    FString ResultItem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting Recipe")
    float CraftingTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting Recipe")
    int32 ExperienceReward;

    FQuest_CraftingRecipe()
    {
        RecipeName = TEXT("");
        CraftingTime = 5.0f;
        ExperienceReward = 10;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_CraftingMission
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting Mission")
    FString MissionName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting Mission")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting Mission")
    TArray<FString> RequiredCraftedItems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting Mission")
    TArray<int32> RequiredQuantities;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting Mission")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting Mission")
    int32 RewardExperience;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting Mission")
    TArray<FString> RewardItems;

    FQuest_CraftingMission()
    {
        MissionName = TEXT("");
        Description = TEXT("");
        bIsCompleted = false;
        RewardExperience = 50;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UQuest_CraftingMissionSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuest_CraftingMissionSystem();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting System")
    TArray<FQuest_CraftingRecipe> AvailableRecipes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting System")
    TArray<FQuest_CraftingMission> ActiveMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting System")
    TMap<FString, int32> PlayerInventory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting System")
    bool bCraftingMenuOpen;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting System")
    float CraftingRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting System")
    int32 PlayerCraftingLevel;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Crafting System")
    void InitializeCraftingSystem();

    UFUNCTION(BlueprintCallable, Category = "Crafting System")
    void AddBasicRecipes();

    UFUNCTION(BlueprintCallable, Category = "Crafting System")
    bool CanCraftItem(const FString& RecipeName);

    UFUNCTION(BlueprintCallable, Category = "Crafting System")
    bool CraftItem(const FString& RecipeName);

    UFUNCTION(BlueprintCallable, Category = "Crafting System")
    void AddItemToInventory(const FString& ItemName, int32 Quantity);

    UFUNCTION(BlueprintCallable, Category = "Crafting System")
    int32 GetItemQuantity(const FString& ItemName);

    UFUNCTION(BlueprintCallable, Category = "Crafting System")
    void ToggleCraftingMenu();

    UFUNCTION(BlueprintCallable, Category = "Crafting System")
    void AddCraftingMission(const FQuest_CraftingMission& NewMission);

    UFUNCTION(BlueprintCallable, Category = "Crafting System")
    void CheckMissionProgress();

    UFUNCTION(BlueprintCallable, Category = "Crafting System")
    void CompleteMission(const FString& MissionName);

    UFUNCTION(BlueprintCallable, Category = "Crafting System")
    TArray<FQuest_CraftingRecipe> GetAvailableRecipes() const;

    UFUNCTION(BlueprintCallable, Category = "Crafting System")
    TArray<FQuest_CraftingMission> GetActiveMissions() const;

    UFUNCTION(BlueprintCallable, Category = "Crafting System")
    void SpawnResourceNodes();

    UFUNCTION(BlueprintCallable, Category = "Crafting System")
    void ValidateCraftingRequirements();

private:
    void CreateStoneAxeRecipe();
    void CreateCampfireRecipe();
    void CreateWaterContainerRecipe();
    void CreateShelterRecipe();
    void CreateSpearRecipe();
    
    void ProcessCraftingInput();
    void UpdateCraftingUI();
    void GainCraftingExperience(int32 Experience);
};