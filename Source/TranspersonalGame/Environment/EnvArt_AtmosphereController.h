#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "EnvArt_AtmosphereController.generated.h"

UENUM(BlueprintType)
enum class EEnvArt_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    Noon        UMETA(DisplayName = "Noon"),
    Afternoon   UMETA(DisplayName = "Afternoon"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night")
};

USTRUCT(BlueprintType)
struct FEnvArt_LightingSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FRotator SunRotation = FRotator(-15.0f, 45.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor FogColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SkyLightIntensity = 1.0f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEnvArt_AtmosphereController : public AActor
{
    GENERATED_BODY()

public:
    AEnvArt_AtmosphereController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    // Time of day system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time System")
    EEnvArt_TimeOfDay CurrentTimeOfDay = EEnvArt_TimeOfDay::Morning;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time System")
    float DayDurationMinutes = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time System")
    bool bAutoAdvanceTime = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time System")
    float TimeProgress = 0.0f; // 0.0 to 1.0 through the day

    // Lighting presets for different times
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Presets")
    TMap<EEnvArt_TimeOfDay, FEnvArt_LightingSettings> LightingPresets;

    // References to world lighting actors
    UPROPERTY(BlueprintReadOnly, Category = "World Lighting")
    ADirectionalLight* SunLight;

    UPROPERTY(BlueprintReadOnly, Category = "World Lighting")
    ASkyLight* SkyLight;

    UPROPERTY(BlueprintReadOnly, Category = "World Lighting")
    AExponentialHeightFog* HeightFog;

public:
    // Blueprint callable functions
    UFUNCTION(BlueprintCallable, Category = "Atmosphere Control")
    void SetTimeOfDay(EEnvArt_TimeOfDay NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere Control")
    void SetTimeProgress(float Progress);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere Control")
    void ApplyLightingSettings(const FEnvArt_LightingSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere Control")
    void FindWorldLightingActors();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere Control")
    void CreateGoldenHourAtmosphere();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere Control")
    void CreateMorningMistAtmosphere();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere Control")
    void CreateNoonClearAtmosphere();

private:
    void InitializeLightingPresets();
    void UpdateAtmosphere(float DeltaTime);
    FEnvArt_LightingSettings InterpolateLightingSettings(const FEnvArt_LightingSettings& A, const FEnvArt_LightingSettings& B, float Alpha);
};