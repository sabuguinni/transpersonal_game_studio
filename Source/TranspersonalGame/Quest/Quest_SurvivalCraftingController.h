#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/TriggerBox.h"
#include "../SharedTypes.h"
#include "Quest_SurvivalCraftingController.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_CraftingRecipe
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    FString RecipeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    TArray<FString> RequiredMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    TArray<int32> MaterialQuantities;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    FString ResultItem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    float CraftingTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    int32 ExperienceGained;

    FQuest_CraftingRecipe()
    {
        RecipeName = TEXT("");
        CraftingTime = 5.0f;
        ExperienceGained = 10;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_ResourceNode
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources")
    FString ResourceType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources")
    int32 Quantity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources")
    float RespawnTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources")
    bool bIsAvailable;

    FQuest_ResourceNode()
    {
        ResourceType = TEXT("Stone");
        Location = FVector::ZeroVector;
        Quantity = 5;
        RespawnTime = 300.0f;
        bIsAvailable = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_SurvivalCraftingController : public AActor
{
    GENERATED_BODY()

public:
    AQuest_SurvivalCraftingController();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* CraftingStationMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* InteractionSphere;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting System")
    TArray<FQuest_CraftingRecipe> AvailableRecipes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource System")
    TArray<FQuest_ResourceNode> ResourceNodes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Integration")
    TArray<FString> ActiveCraftingQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression")
    int32 PlayerCraftingLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression")
    int32 CraftingExperience;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    bool CanCraftItem(const FString& RecipeName, class ATranspersonalCharacter* Player);

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    bool StartCrafting(const FString& RecipeName, class ATranspersonalCharacter* Player);

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    void CompleteCrafting(const FString& RecipeName, class ATranspersonalCharacter* Player);

    UFUNCTION(BlueprintCallable, Category = "Resources")
    bool CollectResource(const FString& ResourceType, int32 Amount, class ATranspersonalCharacter* Player);

    UFUNCTION(BlueprintCallable, Category = "Resources")
    void RespawnResource(int32 NodeIndex);

    UFUNCTION(BlueprintCallable, Category = "Quest Integration")
    void StartCraftingQuest(const FString& QuestName, const FString& RequiredItem);

    UFUNCTION(BlueprintCallable, Category = "Quest Integration")
    void CompleteCraftingQuest(const FString& QuestName);

    UFUNCTION(BlueprintCallable, Category = "Progression")
    void AddCraftingExperience(int32 Experience);

    UFUNCTION(BlueprintCallable, Category = "Progression")
    void CheckLevelUp();

    UFUNCTION(BlueprintCallable, Category = "UI")
    TArray<FQuest_CraftingRecipe> GetAvailableRecipesForLevel(int32 Level);

    UFUNCTION(BlueprintCallable, Category = "Tutorial")
    void ShowCraftingTutorial(class ATranspersonalCharacter* Player);

protected:
    UFUNCTION()
    void OnInteractionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnInteractionSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
    void InitializeBasicRecipes();
    void InitializeResourceNodes();
    bool HasRequiredMaterials(const FQuest_CraftingRecipe& Recipe, class ATranspersonalCharacter* Player);
    void ConsumeMaterials(const FQuest_CraftingRecipe& Recipe, class ATranspersonalCharacter* Player);
    void GiveItemToPlayer(const FString& ItemName, class ATranspersonalCharacter* Player);

    FTimerHandle CraftingTimerHandle;
    FString CurrentCraftingItem;
    class ATranspersonalCharacter* CurrentCraftingPlayer;
};