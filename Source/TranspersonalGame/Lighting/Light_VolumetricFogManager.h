#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/VolumetricFog.h"
#include "SharedTypes.h"
#include "Light_VolumetricFogManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_VolumetricFogSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric Fog")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric Fog")
    float FogMaxOpacity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric Fog")
    float StartDistance = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric Fog")
    float FogCutoffDistance = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric Fog")
    FLinearColor FogInscatteringColor = FLinearColor(0.447f, 0.639f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric Fog")
    FLinearColor FogAlbedo = FLinearColor(1.0f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric Fog")
    float VolumetricFogScatteringDistribution = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric Fog")
    float VolumetricFogExtinctionScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric Fog")
    float VolumetricFogDistance = 6000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric Fog")
    float VolumetricFogStaticLightingScatteringIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric Fog")
    bool bOverrideLightColorsWithFogInscatteringColors = false;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_BiomeFogProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Fog")
    EBiomeType BiomeType = EBiomeType::Savana;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Fog")
    FLight_VolumetricFogSettings DaySettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Fog")
    FLight_VolumetricFogSettings NightSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Fog")
    FLight_VolumetricFogSettings DawnSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Fog")
    FLight_VolumetricFogSettings DuskSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Fog")
    float TransitionSpeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Fog")
    bool bEnableVolumetricFog = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Fog")
    bool bEnableHeightFog = true;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ULight_VolumetricFogManager : public UActorComponent
{
    GENERATED_BODY()

public:
    ULight_VolumetricFogManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog Management")
    TArray<FLight_BiomeFogProfile> BiomeFogProfiles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog Management")
    float CurrentTimeOfDay = 12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog Management")
    EBiomeType CurrentBiome = EBiomeType::Savana;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog Management")
    bool bAutoDetectBiome = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog Management")
    float BiomeDetectionRadius = 5000.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Fog Management")
    AExponentialHeightFog* HeightFogActor;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Fog Management")
    UExponentialHeightFogComponent* HeightFogComponent;

    UFUNCTION(BlueprintCallable, Category = "Fog Management")
    void SetTimeOfDay(float NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Fog Management")
    void SetCurrentBiome(EBiomeType NewBiome);

    UFUNCTION(BlueprintCallable, Category = "Fog Management")
    void UpdateFogSettings();

    UFUNCTION(BlueprintCallable, Category = "Fog Management")
    FLight_VolumetricFogSettings GetCurrentFogSettings() const;

    UFUNCTION(BlueprintCallable, Category = "Fog Management")
    void InitializeBiomeFogProfiles();

    UFUNCTION(BlueprintCallable, Category = "Fog Management")
    void DetectCurrentBiome();

    UFUNCTION(BlueprintCallable, Category = "Fog Management")
    FLight_VolumetricFogSettings InterpolateFogSettings(const FLight_VolumetricFogSettings& SettingsA, const FLight_VolumetricFogSettings& SettingsB, float Alpha) const;

    UFUNCTION(BlueprintCallable, Category = "Fog Management")
    void ApplyFogSettingsToComponent(const FLight_VolumetricFogSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Fog Management")
    void CreateHeightFogActor();

    UFUNCTION(BlueprintCallable, Category = "Fog Management")
    void EnableVolumetricFog(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Fog Management")
    void SetFogDensityMultiplier(float Multiplier);

private:
    UPROPERTY()
    float LastTimeOfDay = -1.0f;

    UPROPERTY()
    EBiomeType LastBiome = EBiomeType::Savana;

    UPROPERTY()
    bool bFogSystemInitialized = false;

    void InitializeFogSystem();
    FLight_BiomeFogProfile* GetBiomeFogProfile(EBiomeType BiomeType);
    float GetTimeOfDayAlpha() const;
    EDayNightPhase GetCurrentDayNightPhase() const;
};