#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/ExponentialHeightFog.h"
#include "Light_VolumetricFogSystem.generated.h"

UENUM(BlueprintType)
enum class ELight_WeatherType : uint8
{
    Clear,
    LightFog,
    HeavyFog,
    Mist,
    Haze,
    Storm
};

USTRUCT(BlueprintType)
struct FLight_FogPreset
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogMaxOpacity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogInscatteringColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor DirectionalInscatteringColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float DirectionalInscatteringExponent = 4.0f;
};

UCLASS(ClassGroup=(Lighting), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ULight_VolumetricFogSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    ULight_VolumetricFogSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    ELight_WeatherType CurrentWeather = ELight_WeatherType::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    bool bEnableWeatherTransitions = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WeatherTransitionSpeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog Presets")
    FLight_FogPreset ClearWeatherFog;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog Presets")
    FLight_FogPreset LightFogWeather;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog Presets")
    FLight_FogPreset HeavyFogWeather;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog Presets")
    FLight_FogPreset MistWeather;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog Presets")
    FLight_FogPreset HazeWeather;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog Presets")
    FLight_FogPreset StormWeather;

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void SetWeatherType(ELight_WeatherType NewWeather);

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void TransitionToWeather(ELight_WeatherType TargetWeather, float TransitionDuration = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "Fog")
    void ApplyFogPreset(const FLight_FogPreset& Preset);

    UFUNCTION(BlueprintCallable, Category = "Fog")
    void SetCretaceousFogSettings();

private:
    UPROPERTY()
    class AExponentialHeightFog* HeightFogActor;

    FLight_FogPreset CurrentFogSettings;
    FLight_FogPreset TargetFogSettings;
    bool bIsTransitioning = false;
    float TransitionProgress = 0.0f;
    float TransitionDuration = 5.0f;

    void FindHeightFogActor();
    void InitializeFogPresets();
    void UpdateFogTransition(float DeltaTime);
    FLight_FogPreset GetPresetForWeather(ELight_WeatherType Weather) const;
    FLight_FogPreset LerpFogPresets(const FLight_FogPreset& A, const FLight_FogPreset& B, float Alpha) const;
};