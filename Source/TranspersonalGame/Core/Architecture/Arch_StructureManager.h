#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/TriggerVolume.h"
#include "SharedTypes.h"
#include "Arch_StructureManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_StructureData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FString StructureName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EArch_StructureType StructureType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FVector PlacementLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FRotator PlacementRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float DegradationLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bIsAccessible;

    FArch_StructureData()
    {
        StructureName = TEXT("");
        StructureType = EArch_StructureType::Dwelling;
        PlacementLocation = FVector::ZeroVector;
        PlacementRotation = FRotator::ZeroRotator;
        DegradationLevel = 0.0f;
        bIsAccessible = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArch_StructureManager : public AActor
{
    GENERATED_BODY()

public:
    AArch_StructureManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure Management")
    TArray<FArch_StructureData> ManagedStructures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure Management")
    float PlacementRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure Management")
    int32 MaxStructuresPerBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure Management")
    float MinDistanceBetweenStructures;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Structure Management")
    bool PlaceStructure(EArch_StructureType StructureType, FVector Location, FRotator Rotation);

    UFUNCTION(BlueprintCallable, Category = "Structure Management")
    void RemoveStructure(int32 StructureIndex);

    UFUNCTION(BlueprintCallable, Category = "Structure Management")
    TArray<FArch_StructureData> GetStructuresInRadius(FVector CenterLocation, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Structure Management")
    void UpdateStructureDegradation(float DeltaTime);

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Structure Management")
    void GenerateStructuresInBiome();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Structure Management")
    void ClearAllStructures();

private:
    bool IsLocationValidForPlacement(FVector Location, EArch_StructureType StructureType);
    FVector FindNearestValidPlacement(FVector DesiredLocation, EArch_StructureType StructureType);
    void SpawnStructureMesh(const FArch_StructureData& StructureData);
};