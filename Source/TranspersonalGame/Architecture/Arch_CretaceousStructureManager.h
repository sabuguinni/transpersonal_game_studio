#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "../SharedTypes.h"
#include "Arch_CretaceousStructureManager.generated.h"

UENUM(BlueprintType)
enum class EArch_StructureType : uint8
{
    CircularStoneDwelling   UMETA(DisplayName = "Circular Stone Dwelling"),
    RectangularShelter      UMETA(DisplayName = "Rectangular Shelter"),
    CaveEntrance           UMETA(DisplayName = "Cave Entrance"),
    StonePlatform          UMETA(DisplayName = "Stone Platform"),
    NaturalArchway         UMETA(DisplayName = "Natural Archway"),
    WeatheredPillar        UMETA(DisplayName = "Weathered Pillar")
};

UENUM(BlueprintType)
enum class EArch_WeatheringLevel : uint8
{
    Fresh       UMETA(DisplayName = "Fresh"),
    Aged        UMETA(DisplayName = "Aged"),
    Weathered   UMETA(DisplayName = "Weathered"),
    Ancient     UMETA(DisplayName = "Ancient"),
    Ruined      UMETA(DisplayName = "Ruined")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_StructureConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EArch_StructureType StructureType = EArch_StructureType::CircularStoneDwelling;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EArch_WeatheringLevel WeatheringLevel = EArch_WeatheringLevel::Aged;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FVector Scale = FVector(1.0f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float WeatherProtection = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float StructuralIntegrity = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bHasInterior = true;

    FArch_StructureConfig()
    {
        StructureType = EArch_StructureType::CircularStoneDwelling;
        WeatheringLevel = EArch_WeatheringLevel::Aged;
        Scale = FVector(1.0f, 1.0f, 1.0f);
        WeatherProtection = 0.7f;
        StructuralIntegrity = 0.8f;
        bHasInterior = true;
    }
};

UCLASS(Blueprintable, BlueprintType)
class TRANSPERSONALGAME_API AArch_CretaceousStructureManager : public AActor
{
    GENERATED_BODY()

public:
    AArch_CretaceousStructureManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* MainStructureMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* InteriorMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure Configuration")
    FArch_StructureConfig StructureConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TArray<UMaterialInterface*> WeatheringMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meshes")
    TMap<EArch_StructureType, UStaticMesh*> StructureMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    TArray<UStaticMesh*> InteriorPropMeshes;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Structure Management")
    void SetStructureType(EArch_StructureType NewType);

    UFUNCTION(BlueprintCallable, Category = "Structure Management")
    void SetWeatheringLevel(EArch_WeatheringLevel NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Structure Management")
    void UpdateStructureAppearance();

    UFUNCTION(BlueprintCallable, Category = "Structure Management")
    void SpawnInteriorProps();

    UFUNCTION(BlueprintCallable, Category = "Structure Management")
    float GetWeatherProtection() const;

    UFUNCTION(BlueprintCallable, Category = "Structure Management")
    float GetStructuralIntegrity() const;

    UFUNCTION(BlueprintCallable, Category = "Structure Management")
    bool CanProvideWeatherProtection(const FVector& TestLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Structure Management")
    TArray<FVector> GetInteriorSpawnPoints() const;

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Editor Tools")
    void RegenerateStructure();

protected:
    UFUNCTION()
    void ApplyWeatheringMaterial();

    UFUNCTION()
    void ConfigureStructureMesh();

    UFUNCTION()
    void SetupInteriorSpace();

    UFUNCTION()
    float CalculateWeatherProtection() const;

    UFUNCTION()
    float CalculateStructuralIntegrity() const;

private:
    float LastWeatheringUpdate;
    bool bInteriorGenerated;
    TArray<FVector> CachedInteriorPoints;
};