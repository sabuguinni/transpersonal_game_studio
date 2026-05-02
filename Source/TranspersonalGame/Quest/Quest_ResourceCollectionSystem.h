#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Quest_ResourceCollectionSystem.generated.h"

UENUM(BlueprintType)
enum class EQuest_ResourceType : uint8
{
    None        UMETA(DisplayName = "None"),
    Stone       UMETA(DisplayName = "Stone"),
    Wood        UMETA(DisplayName = "Wood"),
    Fiber       UMETA(DisplayName = "Fiber"),
    Water       UMETA(DisplayName = "Water"),
    Food        UMETA(DisplayName = "Food"),
    Hide        UMETA(DisplayName = "Hide")
};

USTRUCT(BlueprintType)
struct FQuest_ResourceData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    EQuest_ResourceType ResourceType = EQuest_ResourceType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    int32 Quantity = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    float Quality = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    FString Description = TEXT("");

    FQuest_ResourceData()
    {
        ResourceType = EQuest_ResourceType::None;
        Quantity = 1;
        Quality = 1.0f;
        Description = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct FQuest_CraftingRecipe
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
    FString RecipeName = TEXT("");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
    TArray<FQuest_ResourceData> RequiredResources;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
    FQuest_ResourceData ResultItem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
    float CraftingTime = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
    int32 RequiredSkillLevel = 1;

    FQuest_CraftingRecipe()
    {
        RecipeName = TEXT("");
        CraftingTime = 5.0f;
        RequiredSkillLevel = 1;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UQuest_ResourceCollectionSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuest_ResourceCollectionSystem();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources")
    TArray<FQuest_ResourceData> PlayerInventory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    TArray<FQuest_CraftingRecipe> AvailableRecipes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection")
    float CollectionRange = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection")
    float CollectionTime = 2.0f;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Resource Collection")
    bool CollectResource(AActor* ResourceActor, EQuest_ResourceType ResourceType, int32 Quantity = 1);

    UFUNCTION(BlueprintCallable, Category = "Resource Collection")
    bool HasResource(EQuest_ResourceType ResourceType, int32 RequiredQuantity = 1) const;

    UFUNCTION(BlueprintCallable, Category = "Resource Collection")
    int32 GetResourceQuantity(EQuest_ResourceType ResourceType) const;

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    bool CanCraftItem(const FString& RecipeName) const;

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    bool CraftItem(const FString& RecipeName);

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    void InitializeBasicRecipes();

    UFUNCTION(BlueprintCallable, Category = "Resources")
    TArray<FQuest_ResourceData> GetPlayerInventory() const { return PlayerInventory; }

    UFUNCTION(BlueprintCallable, Category = "Resources")
    void AddResourceToInventory(EQuest_ResourceType ResourceType, int32 Quantity, float Quality = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Resources")
    bool RemoveResourceFromInventory(EQuest_ResourceType ResourceType, int32 Quantity);

    UFUNCTION(BlueprintCallable, Category = "Collection")
    TArray<AActor*> FindNearbyResources() const;

    UFUNCTION(BlueprintCallable, Category = "Collection")
    void SetCollectionRange(float NewRange) { CollectionRange = FMath::Clamp(NewRange, 50.0f, 1000.0f); }

private:
    bool bIsCollecting = false;
    float CurrentCollectionTime = 0.0f;
    AActor* CurrentResourceTarget = nullptr;
    EQuest_ResourceType CurrentResourceType = EQuest_ResourceType::None;

    void ProcessCollection(float DeltaTime);
    void CompleteCollection();
    FQuest_CraftingRecipe* FindRecipe(const FString& RecipeName);
    void LogInventoryContents() const;
};