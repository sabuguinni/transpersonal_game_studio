#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Arch_CretaceousStructures.generated.h"

UENUM(BlueprintType)
enum class EArch_StructureType : uint8
{
    StoneColumn     UMETA(DisplayName = "Stone Column"),
    RockFormation   UMETA(DisplayName = "Rock Formation"),
    NaturalArch     UMETA(DisplayName = "Natural Arch"),
    CaveSystem      UMETA(DisplayName = "Cave System"),
    CliffFace       UMETA(DisplayName = "Cliff Face")
};

USTRUCT(BlueprintType)
struct FArch_StructureConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EArch_StructureType StructureType = EArch_StructureType::StoneColumn;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FVector BaseScale = FVector(1.0f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float WeatheringLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bHasVegetationGrowth = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float StructuralIntegrity = 1.0f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArch_CretaceousStructures : public AActor
{
    GENERATED_BODY()

public:
    AArch_CretaceousStructures();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* MainStructure;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UInstancedStaticMeshComponent* DetailElements;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure Config")
    FArch_StructureConfig StructureSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TArray<UMaterialInterface*> StoneMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TArray<UMaterialInterface*> WeatheredMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    TArray<UStaticMesh*> VegetationMeshes;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Structure")
    void SetStructureType(EArch_StructureType NewType);

    UFUNCTION(BlueprintCallable, Category = "Structure")
    void ApplyWeathering(float WeatheringAmount);

    UFUNCTION(BlueprintCallable, Category = "Structure")
    void GenerateVegetationGrowth();

    UFUNCTION(BlueprintCallable, Category = "Structure")
    void UpdateStructuralIntegrity(float NewIntegrity);

    UFUNCTION(BlueprintCallable, Category = "Structure")
    TArray<FVector> GetClimbablePoints() const;

protected:
    UFUNCTION(CallInEditor, Category = "Structure")
    void RegenerateStructure();

    void ApplyMaterialVariation();
    void SetupCollisionForStructureType();
    void GenerateDetailElements();

private:
    TArray<FVector> ClimbablePoints;
    float LastWeatheringUpdate = 0.0f;
    const float WeatheringUpdateInterval = 10.0f;
};