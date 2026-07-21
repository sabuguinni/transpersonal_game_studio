#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SharedTypes.h"
#include "Quest_CraftingRecipe.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_CraftingIngredient
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ingredient")
    EResourceType ResourceType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ingredient")
    int32 RequiredAmount;

    FQuest_CraftingIngredient()
    {
        ResourceType = EResourceType::Stone;
        RequiredAmount = 1;
    }

    FQuest_CraftingIngredient(EResourceType InResourceType, int32 InAmount)
    {
        ResourceType = InResourceType;
        RequiredAmount = InAmount;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_CraftingResult
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
    EItemType ItemType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
    int32 ResultAmount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
    float CraftingTime;

    FQuest_CraftingResult()
    {
        ItemType = EItemType::StoneAxe;
        ResultAmount = 1;
        CraftingTime = 5.0f;
    }

    FQuest_CraftingResult(EItemType InItemType, int32 InAmount, float InTime)
    {
        ItemType = InItemType;
        ResultAmount = InAmount;
        CraftingTime = InTime;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQuest_CraftingRecipe : public UDataAsset
{
    GENERATED_BODY()

public:
    UQuest_CraftingRecipe();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
    FString RecipeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
    FString RecipeDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
    TArray<FQuest_CraftingIngredient> RequiredIngredients;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
    FQuest_CraftingResult CraftingResult;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
    bool bIsUnlocked;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
    int32 RequiredLevel;

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    bool CanCraft(const TMap<EResourceType, int32>& AvailableResources) const;

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    bool ConsumeIngredients(TMap<EResourceType, int32>& AvailableResources) const;

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    FString GetIngredientsText() const;

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    FString GetResultText() const;
};