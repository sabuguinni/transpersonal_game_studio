#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"
#include "WeatherSystem.generated.h"

UENUM(BlueprintType)
enum class ELight_WeatherState : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Overcast    UMETA(DisplayName = "Overcast"),
    Storm       UMETA(DisplayName = "Storm"),
    Fog         UMETA(DisplayName = "Dense Fog"),
    Drizzle     UMETA(DisplayName = "Drizzle")
};

USTRUCT(BlueprintType)
struct FLight_WeatherPalette
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor FogInscatteringColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float FogDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SkyLightColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SkyLightIntensity;

    FLight_WeatherPalette()
        : SunColor(FLinearColor(1.f, 0.95f, 0.85f, 1.f))
        , SunIntensity(10.f)
        , FogInscatteringColor(FLinearColor(0.5f, 0.6f, 0.7f, 1.f))
        , FogDensity(0.02f)
        , SkyLightColor(FLinearColor(0.8f, 0.85f, 1.f, 1.f))
        , SkyLightIntensity(1.f)
    {}
};

UCLASS(ClassGroup = "Lighting", meta = (DisplayName = "Weather System"))
class TRANSPERSONALGAME_API AWeatherSystem : public AActor
{
    GENERATED_BODY()

public:
    AWeatherSystem();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void SetWeatherState(ELight_WeatherState NewState, float TransitionDuration = 5.f);

    UFUNCTION(BlueprintCallable, Category = "Weather")
    ELight_WeatherState GetCurrentWeatherState() const { return CurrentWeatherState; }

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void TriggerLightningStrike();

    UFUNCTION(BlueprintCallable, Category = "Weather")
    float GetRainIntensity() const { return RainIntensity; }

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather|References")
    TObjectPtr<ADirectionalLight> SunLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather|References")
    TObjectPtr<AExponentialHeightFog> HeightFog;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather|References")
    TObjectPtr<ASkyLight> SkyLightActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather|Config")
    float LightningStrikeChancePerSecond;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather|Config")
    float WeatherChangeCooldown;

    UPROPERTY(BlueprintReadOnly, Category = "Weather|State", meta = (AllowPrivateAccess = "true"))
    float RainIntensity;

    UPROPERTY(BlueprintReadOnly, Category = "Weather|State", meta = (AllowPrivateAccess = "true"))
    float TransitionProgress;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weather|State")
    ELight_WeatherState CurrentWeatherState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weather|State")
    ELight_WeatherState TargetWeatherState;

    FLight_WeatherPalette CurrentPalette;
    FLight_WeatherPalette TargetPalette;

    float TransitionDurationTotal;
    float TransitionElapsed;
    bool bIsTransitioning;
    float LightningCooldown;
    float WeatherCooldownRemaining;

    FLight_WeatherPalette GetPaletteForState(ELight_WeatherState State) const;
    void ApplyPalette(const FLight_WeatherPalette& Palette);
    void UpdateTransition(float DeltaTime);
    void UpdateLightning(float DeltaTime);
    void UpdateRainIntensity();
};
