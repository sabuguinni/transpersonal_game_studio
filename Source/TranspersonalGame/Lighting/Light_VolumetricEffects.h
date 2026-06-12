#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialParameterCollection.h"
#include "Light_VolumetricEffects.generated.h"

UENUM(BlueprintType)
enum class ELight_VolumetricType : uint8
{
    Sunbeams        UMETA(DisplayName = "Sunbeams"),
    Dust            UMETA(DisplayName = "Dust Particles"),
    Pollen          UMETA(DisplayName = "Pollen"),
    Mist            UMETA(DisplayName = "Ground Mist"),
    Steam           UMETA(DisplayName = "Volcanic Steam")
};

USTRUCT(BlueprintType)
struct FLight_VolumetricSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric")
    float Intensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric")
    float Density = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric")
    FLinearColor Tint = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric")
    float ScatteringDistribution = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric")
    float ExtinctionScale = 1.0f;

    FLight_VolumetricSettings()
    {
        Intensity = 1.0f;
        Density = 0.5f;
        Tint = FLinearColor::White;
        ScatteringDistribution = 0.2f;
        ExtinctionScale = 1.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ALight_VolumetricEffects : public AActor
{
    GENERATED_BODY()

public:
    ALight_VolumetricEffects();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric Effects")
    ELight_VolumetricType VolumetricType = ELight_VolumetricType::Sunbeams;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric Effects")
    FLight_VolumetricSettings VolumetricSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    bool bAnimateIntensity = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float AnimationSpeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float IntensityVariation = 0.3f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UStaticMeshComponent> VolumeMeshComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material")
    TObjectPtr<UMaterialInterface> VolumetricMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material")
    TObjectPtr<UMaterialParameterCollection> VolumetricMPC;

    UFUNCTION(BlueprintCallable, Category = "Volumetric Effects")
    void SetVolumetricType(ELight_VolumetricType NewType);

    UFUNCTION(BlueprintCallable, Category = "Volumetric Effects")
    void UpdateVolumetricParameters();

    UFUNCTION(BlueprintCallable, Category = "Volumetric Effects")
    void SetIntensity(float NewIntensity);

    UFUNCTION(BlueprintCallable, Category = "Volumetric Effects")
    void SetDensity(float NewDensity);

    UFUNCTION(BlueprintCallable, Category = "Volumetric Effects")
    void CreateCretaceousSunbeams();

    UFUNCTION(BlueprintCallable, Category = "Volumetric Effects")
    void CreatePrehistoricMist();

private:
    void InitializeVolumetricMesh();
    void AnimateVolumetricEffects(float DeltaTime);
    void UpdateMaterialParameters();
    
    float AnimationTime = 0.0f;
    float BaseIntensity = 1.0f;
};