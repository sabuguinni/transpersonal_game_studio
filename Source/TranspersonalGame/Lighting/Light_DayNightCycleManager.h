#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/World.h"
#include "Light_DayNightCycleManager.generated.h"

UENUM(BlueprintType)
enum class ELight_TimeOfDay : uint8
{
    Dawn,
    Morning,
    Midday,
    Afternoon,
    Dusk,
    Night
};

USTRUCT(BlueprintType)
struct FLight_TimeSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunAngle = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor FogColor = FLinearColor::Gray;

    FLight_TimeSettings()
    {
        SunIntensity = 5.0f;
        SunColor = FLinearColor::White;
        SunAngle = 45.0f;
        FogDensity = 0.02f;
        FogColor = FLinearColor::Gray;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ALight_DayNightCycleManager : public AActor
{
    GENERATED_BODY()

public:
    ALight_DayNightCycleManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    float CycleDurationMinutes = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    float CurrentTimeOfDay = 12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    bool bAutoAdvanceTime = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    bool bUseCretaceousLighting = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Settings")
    TMap<ELight_TimeOfDay, FLight_TimeSettings> TimeSettings;

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void SetTimeOfDay(float NewTime);

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    ELight_TimeOfDay GetCurrentTimeOfDay() const;

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void ApplyCretaceousAtmosphere();

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void UpdateLighting();

private:
    UPROPERTY()
    class ADirectionalLight* SunLight;

    UPROPERTY()
    class ASkyAtmosphere* SkyAtmosphere;

    UPROPERTY()
    class AExponentialHeightFog* VolumeFog;

    void FindLightingActors();
    void InterpolateLightingSettings(const FLight_TimeSettings& From, const FLight_TimeSettings& To, float Alpha);
    void InitializeCretaceousSettings();
};