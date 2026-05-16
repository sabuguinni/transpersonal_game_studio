#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/PostProcessComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyAtmosphere.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/PostProcessVolume.h"
#include "AtmosphericLightingManager.generated.h"

UENUM(BlueprintType)
enum class ELight_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"), 
    Midday      UMETA(DisplayName = "Midday"),
    Afternoon   UMETA(DisplayName = "Afternoon"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night")
};

USTRUCT(BlueprintType)
struct FLight_AtmosphericSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    FLinearColor SunColor = FLinearColor(1.0f, 0.85f, 0.6f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunIntensity = 3.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    FRotator SunRotation = FRotator(-45.0f, 135.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float RayleighScattering = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float MieScattering = 0.06f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.008f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogHeightFalloff = 0.15f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogColor = FLinearColor(0.7f, 0.8f, 0.9f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Post Process")
    float WhiteTemperature = 5800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Post Process")
    float WhiteTint = -2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Post Process")
    float BloomIntensity = 0.8f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAtmosphericLightingManager : public AActor
{
    GENERATED_BODY()

public:
    AAtmosphericLightingManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    // Current atmospheric settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting", meta = (AllowPrivateAccess = "true"))
    FLight_AtmosphericSettings CurrentSettings;

    // Time of day cycle
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Cycle", meta = (AllowPrivateAccess = "true"))
    ELight_TimeOfDay CurrentTimeOfDay = ELight_TimeOfDay::Afternoon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Cycle", meta = (AllowPrivateAccess = "true"))
    float DayDurationMinutes = 24.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Cycle", meta = (AllowPrivateAccess = "true"))
    bool bEnableTimeProgression = true;

    // Cached lighting actors
    UPROPERTY(BlueprintReadOnly, Category = "Cached Actors", meta = (AllowPrivateAccess = "true"))
    ADirectionalLight* SunActor;

    UPROPERTY(BlueprintReadOnly, Category = "Cached Actors", meta = (AllowPrivateAccess = "true"))
    ASkyAtmosphere* SkyAtmosphereActor;

    UPROPERTY(BlueprintReadOnly, Category = "Cached Actors", meta = (AllowPrivateAccess = "true"))
    AExponentialHeightFog* FogActor;

    UPROPERTY(BlueprintReadOnly, Category = "Cached Actors", meta = (AllowPrivateAccess = "true"))
    APostProcessVolume* PostProcessActor;

    // Cretaceous period presets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presets", meta = (AllowPrivateAccess = "true"))
    TMap<ELight_TimeOfDay, FLight_AtmosphericSettings> CretaceousPresets;

public:
    virtual void Tick(float DeltaTime) override;

    // Main atmospheric control functions
    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    void ApplyCretaceousAtmosphere();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    void SetTimeOfDay(ELight_TimeOfDay NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    void TransitionToTimeOfDay(ELight_TimeOfDay TargetTime, float TransitionDuration = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    void ApplyAtmosphericSettings(const FLight_AtmosphericSettings& Settings);

    // Individual component control
    UFUNCTION(BlueprintCallable, Category = "Sun Control")
    void ConfigureSun(FLinearColor Color, float Intensity, FRotator Rotation);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere Control")
    void ConfigureAtmosphere(float RayleighScale, float MieScale);

    UFUNCTION(BlueprintCallable, Category = "Fog Control")
    void ConfigureFog(float Density, float HeightFalloff, FLinearColor Color);

    UFUNCTION(BlueprintCallable, Category = "Post Process")
    void ConfigurePostProcess(float Temperature, float Tint, float Bloom);

    // Architectural lighting integration
    UFUNCTION(BlueprintCallable, Category = "Architectural Lighting")
    void AddInteriorLightingToStructures();

    UFUNCTION(BlueprintCallable, Category = "Architectural Lighting")
    void CreateArchitecturalPointLight(FVector Location, FLinearColor Color, float Intensity, float Radius);

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "Lighting Utils")
    void CacheAllLightingActors();

    UFUNCTION(BlueprintCallable, Category = "Lighting Utils", CallInEditor = true)
    void InitializeCretaceousPresets();

    UFUNCTION(BlueprintCallable, Category = "Lighting Utils", CallInEditor = true)
    void ValidateLightingSetup();

    // Getters
    UFUNCTION(BlueprintPure, Category = "Atmospheric Lighting")
    ELight_TimeOfDay GetCurrentTimeOfDay() const { return CurrentTimeOfDay; }

    UFUNCTION(BlueprintPure, Category = "Atmospheric Lighting")
    FLight_AtmosphericSettings GetCurrentSettings() const { return CurrentSettings; }

private:
    // Time progression
    float CurrentTimeProgress = 0.5f; // 0.0 = midnight, 0.5 = noon
    bool bIsTransitioning = false;
    float TransitionTimer = 0.0f;
    float TransitionDuration = 5.0f;
    FLight_AtmosphericSettings TransitionStartSettings;
    FLight_AtmosphericSettings TransitionTargetSettings;

    // Helper functions
    void UpdateTimeProgression(float DeltaTime);
    void UpdateTransition(float DeltaTime);
    ELight_TimeOfDay CalculateTimeOfDayFromProgress(float Progress);
    FLight_AtmosphericSettings InterpolateSettings(const FLight_AtmosphericSettings& A, const FLight_AtmosphericSettings& B, float Alpha);
    void CreateDefaultLightingActors();
};