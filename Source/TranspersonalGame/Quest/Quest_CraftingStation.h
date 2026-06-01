#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Quest_CraftingRecipe.h"
#include "SharedTypes.h"
#include "Quest_CraftingStation.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCraftingComplete, EItemType, ItemType, int32, Amount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCraftingStarted, float, CraftingTime);

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_CraftingStation : public AActor
{
    GENERATED_BODY()

public:
    AQuest_CraftingStation();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    UStaticMeshComponent* StationMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    USphereComponent* InteractionSphere;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    TArray<UQuest_CraftingRecipe*> AvailableRecipes;

    UPROPERTY(BlueprintReadOnly, Category = "Crafting")
    bool bIsCrafting;

    UPROPERTY(BlueprintReadOnly, Category = "Crafting")
    float CraftingTimer;

    UPROPERTY(BlueprintReadOnly, Category = "Crafting")
    float TotalCraftingTime;

    UPROPERTY(BlueprintReadOnly, Category = "Crafting")
    UQuest_CraftingRecipe* CurrentRecipe;

    UPROPERTY(BlueprintReadOnly, Category = "Crafting")
    class ATranspersonalCharacter* CurrentCrafter;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnCraftingComplete OnCraftingComplete;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnCraftingStarted OnCraftingStarted;

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    bool StartCrafting(UQuest_CraftingRecipe* Recipe, class ATranspersonalCharacter* Crafter);

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    void CancelCrafting();

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    TArray<UQuest_CraftingRecipe*> GetAvailableRecipes() const;

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    bool CanCraftRecipe(UQuest_CraftingRecipe* Recipe, const TMap<EResourceType, int32>& PlayerResources) const;

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    float GetCraftingProgress() const;

    UFUNCTION()
    void OnInteractionOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnInteractionOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

protected:
    UFUNCTION()
    void CompleteCrafting();

    UFUNCTION(BlueprintImplementableEvent, Category = "Crafting")
    void OnPlayerEnterRange(class ATranspersonalCharacter* Player);

    UFUNCTION(BlueprintImplementableEvent, Category = "Crafting")
    void OnPlayerExitRange(class ATranspersonalCharacter* Player);
};