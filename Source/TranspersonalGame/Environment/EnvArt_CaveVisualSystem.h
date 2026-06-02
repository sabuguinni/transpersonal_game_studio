#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "EnvArt_CaveVisualSystem.generated.h"

UENUM(BlueprintType)
enum class EEnvArt_CaveVisualType : uint8
{
    SmallCaveEntrance,
    MediumCaveEntrance,
    LargeCaveEntrance,
    CrystalCave,
    WaterCave,
    VolcanicCave
};

USTRUCT(BlueprintType)
struct FEnvArt_CaveVisualConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Visual")
    EEnvArt_CaveVisualType CaveType = EEnvArt_CaveVisualType::SmallCaveEntrance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Visual")
    FLinearColor AmbientLightColor = FLinearColor(0.2f, 0.6f, 0.8f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Visual")
    float AmbientLightIntensity = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Visual")
    float LightAttenuationRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Visual")
    int32 StalactiteCount = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Visual")
    int32 StalagmiteCount = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Visual")
    bool bHasCrystalFormations = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Visual")
    bool bHasWaterFeatures = false;

    FEnvArt_CaveVisualConfig()
    {
        CaveType = EEnvArt_CaveVisualType::SmallCaveEntrance;
        AmbientLightColor = FLinearColor(0.2f, 0.6f, 0.8f, 1.0f);
        AmbientLightIntensity = 500.0f;
        LightAttenuationRadius = 1500.0f;
        StalactiteCount = 5;
        StalagmiteCount = 3;
        bHasCrystalFormations = false;
        bHasWaterFeatures = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEnvArt_CaveVisualSystem : public AActor
{
    GENERATED_BODY()

public:
    AEnvArt_CaveVisualSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UPointLightComponent* AmbientLightComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Visual")
    FEnvArt_CaveVisualConfig CaveConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Visual")
    TArray<UStaticMeshComponent*> StalactiteMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Visual")
    TArray<UStaticMeshComponent*> StalagmiteMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Visual")
    TArray<UStaticMeshComponent*> CrystalFormations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Visual")
    TArray<UStaticMeshComponent*> WaterFeatures;

    UFUNCTION(BlueprintCallable, Category = "Cave Visual")
    void InitializeCaveVisuals();

    UFUNCTION(BlueprintCallable, Category = "Cave Visual")
    void CreateStalactites();

    UFUNCTION(BlueprintCallable, Category = "Cave Visual")
    void CreateStalagmites();

    UFUNCTION(BlueprintCallable, Category = "Cave Visual")
    void CreateCrystalFormations();

    UFUNCTION(BlueprintCallable, Category = "Cave Visual")
    void CreateWaterFeatures();

    UFUNCTION(BlueprintCallable, Category = "Cave Visual")
    void SetupAmbientLighting();

    UFUNCTION(BlueprintCallable, Category = "Cave Visual")
    void ConfigureCaveType(EEnvArt_CaveVisualType NewCaveType);

    UFUNCTION(BlueprintCallable, Category = "Cave Visual")
    void UpdateLightingBasedOnCaveType();

    UFUNCTION(BlueprintCallable, Category = "Cave Visual")
    void CleanupCaveVisuals();

private:
    void CreateMeshComponent(const FString& ComponentName, const FVector& RelativeLocation, const FVector& Scale);
    void SetupCaveTypeSpecificFeatures();
    FLinearColor GetCaveTypeColor(EEnvArt_CaveVisualType CaveType);
    float GetCaveTypeLightIntensity(EEnvArt_CaveVisualType CaveType);
};