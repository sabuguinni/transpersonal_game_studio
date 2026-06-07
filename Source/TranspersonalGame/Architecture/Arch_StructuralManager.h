#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/PointLight.h"
#include "Engine/StaticMeshActor.h"
#include "../SharedTypes.h"
#include "Arch_StructuralManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_StructuralElement
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FString ElementName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FRotator Rotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FVector Scale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EBiomeType AssociatedBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float StructuralIntegrity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bIsInterior;

    FArch_StructuralElement()
    {
        ElementName = TEXT("DefaultStructure");
        Location = FVector::ZeroVector;
        Rotation = FRotator::ZeroRotator;
        Scale = FVector::OneVector;
        AssociatedBiome = EBiomeType::Temperate;
        StructuralIntegrity = 100.0f;
        bIsInterior = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_LightingConfiguration
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor AmbientColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float AmbientIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor DirectionalColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float DirectionalIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FRotator DirectionalRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    TArray<FVector> PointLightLocations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float PointLightIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float PointLightRadius;

    FArch_LightingConfiguration()
    {
        AmbientColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);
        AmbientIntensity = 2.5f;
        DirectionalColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
        DirectionalIntensity = 3.0f;
        DirectionalRotation = FRotator(-45.0f, 135.0f, 0.0f);
        PointLightIntensity = 1000.0f;
        PointLightRadius = 2000.0f;
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    TArray<FArch_StructuralElement> StructuralElements;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    FArch_LightingConfiguration LightingConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    float BiomeInfluenceRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    bool bEnableAtmosphericEffects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    float AtmosphericDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    FLinearColor AtmosphericColor;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void SpawnStructuralElement(const FArch_StructuralElement& Element);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void ConfigureArchitecturalLighting();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void UpdateAtmosphericEffects();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void SetBiomeArchitecture(EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    TArray<FArch_StructuralElement> GetStructuresInRadius(FVector Center, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void ApplyWeatheringEffects(float WeatheringIntensity);

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Architecture")
    void GenerateArchitecturalLayout();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Architecture")
    void ValidateStructuralIntegrity();

private:
    UPROPERTY()
    TArray<AActor*> SpawnedStructures;

    UPROPERTY()
    TArray<ADirectionalLight*> DirectionalLights;

    UPROPERTY()
    TArray<APointLight*> PointLights;

    void InitializeArchitecturalSystems();
    void ConfigureBiomeSpecificArchitecture(EBiomeType BiomeType);
    void UpdateStructuralWeathering();
    bool ValidateElementPlacement(const FArch_StructuralElement& Element);
};