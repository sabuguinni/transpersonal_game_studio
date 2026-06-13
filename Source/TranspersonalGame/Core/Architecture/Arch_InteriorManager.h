#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "SharedTypes.h"
#include "Arch_InteriorManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_InteriorItem
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    FString ItemName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    EArch_InteriorType ItemType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    FVector RelativeLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    FRotator RelativeRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    bool bIsInteractable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    float WearLevel;

    FArch_InteriorItem()
    {
        ItemName = TEXT("");
        ItemType = EArch_InteriorType::Furniture;
        RelativeLocation = FVector::ZeroVector;
        RelativeRotation = FRotator::ZeroRotator;
        bIsInteractable = true;
        WearLevel = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArch_InteriorManager : public AActor
{
    GENERATED_BODY()

public:
    AArch_InteriorManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBoxComponent* InteriorBounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior Setup")
    TArray<FArch_InteriorItem> InteriorItems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior Setup")
    EArch_StructureType StructureType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior Setup")
    float InteriorSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior Setup")
    bool bHasFirePit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior Setup")
    bool bHasSleepingArea;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior Setup")
    bool bHasStorageArea;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Interior Management")
    void SetupInteriorForStructure(EArch_StructureType InStructureType);

    UFUNCTION(BlueprintCallable, Category = "Interior Management")
    bool AddInteriorItem(EArch_InteriorType ItemType, FVector Location, FRotator Rotation);

    UFUNCTION(BlueprintCallable, Category = "Interior Management")
    void RemoveInteriorItem(int32 ItemIndex);

    UFUNCTION(BlueprintCallable, Category = "Interior Management")
    TArray<FArch_InteriorItem> GetInteractableItems();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Interior Management")
    void GenerateRandomInterior();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Interior Management")
    void ClearInterior();

    UFUNCTION(BlueprintCallable, Category = "Interior Management")
    bool IsLocationInsideInterior(FVector WorldLocation);

private:
    void SpawnInteriorItem(const FArch_InteriorItem& Item);
    void SetupDwellingInterior();
    void SetupShelterInterior();
    void SetupStorageInterior();
    void SetupWorkshopInterior();
    void SetupRuinInterior();
};