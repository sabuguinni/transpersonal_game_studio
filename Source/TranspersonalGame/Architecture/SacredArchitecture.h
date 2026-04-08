// SacredArchitecture.h
// Sistema de arquitetura sagrada para estruturas transpessoais

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "SacredArchitecture.generated.h"

UENUM(BlueprintType)
enum class ESacredArchitectureType : uint8
{
    Temple         UMETA(DisplayName = "Temple"),
    Pyramid        UMETA(DisplayName = "Pyramid"),
    Mandala        UMETA(DisplayName = "Mandala Structure"),
    Spiral         UMETA(DisplayName = "Spiral Tower"),
    Crystal        UMETA(DisplayName = "Crystal Formation"),
    Labyrinth      UMETA(DisplayName = "Labyrinth"),
    Portal         UMETA(DisplayName = "Portal Gateway"),
    Sanctuary      UMETA(DisplayName = "Sanctuary")
};

USTRUCT(BlueprintType)
struct FArchitecturalParameters
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Scale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Height = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Segments = 8;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float GoldenRatio = 1.618f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bUsePhiProportions = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float EnergyIntensity = 1.0f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ASacredArchitecture : public AActor
{
    GENERATED_BODY()

public:
    ASacredArchitecture();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* MainStructure;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    ESacredArchitectureType ArchitectureType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    FArchitecturalParameters Parameters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* SacredMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* EnergyMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    class USoundBase* AmbientResonance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    class UParticleSystem* EnergyField;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void GenerateStructure();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void SetArchitectureType(ESacredArchitectureType NewType);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void UpdateParameters(const FArchitecturalParameters& NewParams);

    UFUNCTION(BlueprintCallable, Category = "Effects")
    void ActivateEnergyField();

    UFUNCTION(BlueprintCallable, Category = "Effects")
    void DeactivateEnergyField();

protected:
    UFUNCTION(BlueprintImplementableEvent, Category = "Architecture")
    void OnStructureGenerated();

    UFUNCTION(BlueprintImplementableEvent, Category = "Effects")
    void OnEnergyFieldActivated();

private:
    void GenerateTemple();
    void GeneratePyramid();
    void GenerateMandalaStructure();
    void GenerateSpiralTower();
    void GenerateCrystalFormation();
    void GenerateLabyrinth();
    void GeneratePortalGateway();
    void GenerateSanctuary();

    void ApplyGoldenRatioProportions();
    void SetupEnergyResonance();

    float CurrentEnergyLevel;
    float TimeAccumulator;
    bool bEnergyFieldActive;
};