#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "SharedTypes.h"
#include "Arch_InteriorLayout.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_InteriorItem
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior Item")
    FString ItemName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior Item")
    FVector RelativeLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior Item")
    FRotator RelativeRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior Item")
    float ItemCondition = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior Item")
    bool bIsEssential = false;

    FArch_InteriorItem()
    {
        ItemName = TEXT("Unknown");
        RelativeLocation = FVector::ZeroVector;
        RelativeRotation = FRotator::ZeroRotator;
        ItemCondition = 100.0f;
        bIsEssential = false;
    }
};

UCLASS()
class TRANSPERSONALGAME_API AArch_InteriorLayout : public AActor
{
    GENERATED_BODY()

public:
    AArch_InteriorLayout();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior System")
    TArray<FArch_InteriorItem> InteriorItems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior System")
    float StructureRadius = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior System")
    int32 MaxItems = 12;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior System")
    bool bAutoGenerateLayout = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior System")
    TArray<UStaticMeshComponent*> SpawnedItemMeshes;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Interior System")
    void GeneratePrehistoricLayout();

    UFUNCTION(BlueprintCallable, Category = "Interior System")
    void PlaceFirePit(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Interior System")
    void PlaceSleepingArea(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Interior System")
    void PlaceToolStorage(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Interior System")
    void PlaceFoodStorage(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Interior System")
    void UpdateItemConditions(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Interior System")
    void ClearLayout();

    UFUNCTION(BlueprintCallable, Category = "Interior System")
    int32 GetItemCount() const { return InteriorItems.Num(); }

    UFUNCTION(BlueprintCallable, Category = "Interior System")
    bool HasEssentialItems() const;
};