#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/PointLightComponent.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "Light_DynamicAtmosphereManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_AtmosphericSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Settings")
    float SunIntensity = 6.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Settings")
    FLinearColor SunColor = FLinearColor(1.0f, 0.94f, 0.78f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Settings")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Settings")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Settings")
    FLinearColor FogInscatteringColor = FLinearColor(0.7f, 0.78f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Settings")
    bool bVolumetricFogEnabled = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Settings")
    bool bCastVolumetricShadows = true;

    FLight_AtmosphericSettings()
    {
        // Default Cretaceous period atmospheric settings
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_TimeOfDaySettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    float CurrentTimeHours = 12.0f; // Noon by default

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    float DayDurationMinutes = 20.0f; // 20 real minutes = 1 game day

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    FLinearColor DawnColor = FLinearColor(1.0f, 0.7f, 0.4f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    FLinearColor NoonColor = FLinearColor(1.0f, 0.94f, 0.78f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    FLinearColor DuskColor = FLinearColor(1.0f, 0.5f, 0.2f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    FLinearColor NightColor = FLinearColor(0.1f, 0.15f, 0.3f, 1.0f);

    FLight_TimeOfDaySettings()
    {
        // Default time of day settings for Cretaceous period
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ALight_DynamicAtmosphereManager : public AActor
{
    GENERATED_BODY()

public:
    ALight_DynamicAtmosphereManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // === ATMOSPHERIC COMPONENTS ===
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting Components")
    class UDirectionalLightComponent* SunLightComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Atmospheric Components")
    class UExponentialHeightFogComponent* AtmosphericFogComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Atmospheric Components")
    class USkyAtmosphereComponent* SkyAtmosphereComponent;

    // === ATMOSPHERIC SETTINGS ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Configuration")
    FLight_AtmosphericSettings AtmosphericSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Configuration")
    FLight_TimeOfDaySettings TimeOfDaySettings;

    // === DYNAMIC LIGHTING CONTROL ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Lighting")
    bool bEnableDynamicTimeOfDay = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Lighting")
    bool bEnableAtmosphericPersistence = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Lighting")
    float ArchitecturalLightIntensity = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Lighting")
    float ArchitecturalLightRadius = 800.0f;

public:
    // === ATMOSPHERIC MANAGEMENT FUNCTIONS ===
    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void ApplyCretaceousAtmosphere();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void ConfigureDirectionalLighting(float Intensity, FLinearColor Color);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void ConfigureAtmosphericFog(float Density, float HeightFalloff, FLinearColor InscatteringColor);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Control")
    void EnableVolumetricLighting(bool bEnable);

    // === TIME OF DAY FUNCTIONS ===
    UFUNCTION(BlueprintCallable, Category = "Time of Day")
    void UpdateTimeOfDay(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Time of Day")
    void SetTimeOfDay(float Hours);

    UFUNCTION(BlueprintCallable, Category = "Time of Day")
    FLinearColor GetCurrentSunColor() const;

    UFUNCTION(BlueprintCallable, Category = "Time of Day")
    float GetCurrentSunIntensity() const;

    // === ARCHITECTURAL LIGHTING INTEGRATION ===
    UFUNCTION(BlueprintCallable, Category = "Architectural Lighting")
    void CreateArchitecturalLighting(FVector Location, FLinearColor Color = FLinearColor::White);

    UFUNCTION(BlueprintCallable, Category = "Architectural Lighting")
    void EnhanceExistingArchitecture();

    // === PERSISTENCE FUNCTIONS ===
    UFUNCTION(BlueprintCallable, Category = "Persistence", CallInEditor = true)
    void SaveAtmosphericSettings();

    UFUNCTION(BlueprintCallable, Category = "Persistence", CallInEditor = true)
    void LoadAtmosphericSettings();

    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor = true)
    void ValidateAtmosphericSetup();

private:
    // === INTERNAL HELPER FUNCTIONS ===
    void InitializeAtmosphericComponents();
    void ConfigureLumenSettings();
    void CleanupOldAtmosphericActors();
    FLinearColor InterpolateSunColor(float TimeHours) const;
    float InterpolateSunIntensity(float TimeHours) const;
    void UpdateSunRotation(float TimeHours);

    // === INTERNAL STATE ===
    float AccumulatedTime = 0.0f;
    bool bAtmosphereInitialized = false;
    TArray<class APointLight*> ArchitecturalLights;
};