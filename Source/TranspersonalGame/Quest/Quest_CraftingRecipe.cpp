#include "Quest_CraftingRecipe.h"

UQuest_CraftingRecipe::UQuest_CraftingRecipe()
{
    RecipeName = TEXT("Unknown Recipe");
    RecipeDescription = TEXT("A crafting recipe");
    bIsUnlocked = true;
    RequiredLevel = 1;

    // Default recipe: Stone Axe
    RequiredIngredients.Add(FQuest_CraftingIngredient(EResourceType::Stone, 2));
    RequiredIngredients.Add(FQuest_CraftingIngredient(EResourceType::Wood, 1));
    CraftingResult = FQuest_CraftingResult(EItemType::StoneAxe, 1, 5.0f);
}

bool UQuest_CraftingRecipe::CanCraft(const TMap<EResourceType, int32>& AvailableResources) const
{
    if (!bIsUnlocked)
    {
        return false;
    }

    // Check if player has enough of each required ingredient
    for (const FQuest_CraftingIngredient& Ingredient : RequiredIngredients)
    {
        const int32* AvailableAmount = AvailableResources.Find(Ingredient.ResourceType);
        if (!AvailableAmount || *AvailableAmount < Ingredient.RequiredAmount)
        {
            return false;
        }
    }

    return true;
}

bool UQuest_CraftingRecipe::ConsumeIngredients(TMap<EResourceType, int32>& AvailableResources) const
{
    if (!CanCraft(AvailableResources))
    {
        return false;
    }

    // Consume the ingredients
    for (const FQuest_CraftingIngredient& Ingredient : RequiredIngredients)
    {
        int32* AvailableAmount = AvailableResources.Find(Ingredient.ResourceType);
        if (AvailableAmount)
        {
            *AvailableAmount -= Ingredient.RequiredAmount;
            
            // Remove entry if amount reaches 0
            if (*AvailableAmount <= 0)
            {
                AvailableResources.Remove(Ingredient.ResourceType);
            }
        }
    }

    return true;
}

FString UQuest_CraftingRecipe::GetIngredientsText() const
{
    FString IngredientsText = TEXT("Requires: ");
    
    for (int32 i = 0; i < RequiredIngredients.Num(); i++)
    {
        const FQuest_CraftingIngredient& Ingredient = RequiredIngredients[i];
        
        FString ResourceName = TEXT("Unknown");
        switch (Ingredient.ResourceType)
        {
            case EResourceType::Stone:
                ResourceName = TEXT("Stone");
                break;
            case EResourceType::Wood:
                ResourceName = TEXT("Wood");
                break;
            case EResourceType::Fiber:
                ResourceName = TEXT("Fiber");
                break;
        }
        
        IngredientsText += FString::Printf(TEXT("%d %s"), Ingredient.RequiredAmount, *ResourceName);
        
        if (i < RequiredIngredients.Num() - 1)
        {
            IngredientsText += TEXT(", ");
        }
    }
    
    return IngredientsText;
}

FString UQuest_CraftingRecipe::GetResultText() const
{
    FString ItemName = TEXT("Unknown Item");
    switch (CraftingResult.ItemType)
    {
        case EItemType::StoneAxe:
            ItemName = TEXT("Stone Axe");
            break;
        case EItemType::Campfire:
            ItemName = TEXT("Campfire");
            break;
        case EItemType::WaterContainer:
            ItemName = TEXT("Water Container");
            break;
    }
    
    return FString::Printf(TEXT("Creates: %d %s (%.1fs)"), 
                          CraftingResult.ResultAmount, 
                          *ItemName, 
                          CraftingResult.CraftingTime);
}