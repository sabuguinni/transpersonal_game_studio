#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "../SharedTypes.h"
#include "Arch_StructureManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_StructureData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FString StructureName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EBiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FRotator Rotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float WeatheringLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bIsRuin;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    TArray<FString> InteriorObjects;

    FArch_StructureData()
    {
        StructureName = TEXT("Unknown");
        BiomeType = EBiomeType::Savanna;
        Location = FVector::ZeroVector;
        Rotation = FRotator::ZeroRotator;
        WeatheringLevel = 0.5f;
        bIsRuin = false;
    }
};

UENUM(BlueprintType)
enum class EArch_StructureType : uint8
{
    Dwelling,
    Storage,
    Workshop,
    Ceremonial,
    Defensive,
    Ruin
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

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* MainStructureMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure Config")
    FArch_StructureData StructureData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure Config")
    EArch_StructureType StructureType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TArray<UMaterialInterface*> WeatheredMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    TArray<UStaticMesh*> InteriorProps;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    TArray<AActor*> SpawnedInteriorActors;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Structure Management")
    void InitializeStructure(const FArch_StructureData& InStructureData, EArch_StructureType InType);

    UFUNCTION(BlueprintCallable, Category = "Structure Management")
    void ApplyWeathering(float WeatheringAmount);

    UFUNCTION(BlueprintCallable, Category = "Interior Management")
    void PopulateInterior();

    UFUNCTION(BlueprintCallable, Category = "Interior Management")
    void ClearInterior();

    UFUNCTION(BlueprintCallable, Category = "Structure Management")
    void ConvertToRuin(float DestructionLevel);

    UFUNCTION(BlueprintCallable, Category = "Structure Management")
    FArch_StructureData GetStructureData() const { return StructureData; }

    UFUNCTION(BlueprintCallable, Category = "Structure Management")
    void SetStructureType(EArch_StructureType NewType);

    UFUNCTION(BlueprintCallable, Category = "Material Management")
    void UpdateMaterialsForBiome(EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Validation")
    bool ValidateStructurePlacement() const;

private:
    void SpawnInteriorProp(UStaticMesh* PropMesh, const FVector& RelativeLocation, const FRotator& RelativeRotation);
    void ApplyBiomeSpecificWeathering();
    void UpdateStructureAppearance();

    UPROPERTY()
    float CurrentWeatheringLevel;

    UPROPERTY()
    bool bInteriorPopulated;
};