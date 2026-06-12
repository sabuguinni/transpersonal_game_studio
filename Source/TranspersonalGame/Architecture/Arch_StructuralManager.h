#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "Arch_StructuralManager.generated.h"

UENUM(BlueprintType)
enum class EArch_StructureType : uint8
{
    Shelter         UMETA(DisplayName = "Primitive Shelter"),
    Foundation      UMETA(DisplayName = "Stone Foundation"),
    Wall            UMETA(DisplayName = "Stone Wall"),
    Entrance        UMETA(DisplayName = "Entrance Opening"),
    Support         UMETA(DisplayName = "Support Beam"),
    Ruins           UMETA(DisplayName = "Ancient Ruins")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_StructuralData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EArch_StructureType StructureType = EArch_StructureType::Shelter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FVector Dimensions = FVector(400.0f, 600.0f, 300.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float WeatheringLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bHasVegetationGrowth = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    int32 StructuralIntegrity = 75;

    FArch_StructuralData()
    {
        StructureType = EArch_StructureType::Shelter;
        Dimensions = FVector(400.0f, 600.0f, 300.0f);
        WeatheringLevel = 0.5f;
        bHasVegetationGrowth = true;
        StructuralIntegrity = 75;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArch_StructuralManager : public AActor
{
    GENERATED_BODY()

public:
    AArch_StructuralManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* StructureMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    FArch_StructuralData StructuralData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    TArray<FVector> ShelterLocations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    float BiomePlacementRadius = 10000.0f;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void PlaceStructuralElements();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void UpdateWeatheringEffects(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void GenerateShelterNetwork();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    FVector GetOptimalShelterLocation(const FVector& BiomeCenter) const;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void ApplyEnvironmentalIntegration();

    UFUNCTION(BlueprintPure, Category = "Architecture")
    bool IsStructureStable() const;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void SetStructureType(EArch_StructureType NewType);

    UFUNCTION(BlueprintPure, Category = "Architecture")
    FArch_StructuralData GetStructuralData() const { return StructuralData; }
};