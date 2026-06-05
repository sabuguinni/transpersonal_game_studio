#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/VolumetricFogComponent.h"
#include "GameFramework/Actor.h"
#include "Light_DynamicAtmosphereManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_AtmosphereSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float SunIntensity = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLinearColor SunColor = FLinearColor(1.0f, 0.94f, 0.78f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float FogDensity = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLinearColor FogColor = FLinearColor(0.78f, 0.70f, 0.63f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float VolumetricScattering = 0.3f;
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLight_AtmosphereSettings CretaceousSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night")
    float DayNightCycleDuration = 1200.0f; // 20 minutes

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night")
    float CurrentTimeOfDay = 0.5f; // 0 = midnight, 0.5 = noon, 1 = midnight

    UPROPERTY(BlueprintReadOnly, Category = "Components")
    class UDirectionalLightComponent* SunLight;

    UPROPERTY(BlueprintReadOnly, Category = "Components")
    class USkyLightComponent* SkyLight;

    UPROPERTY(BlueprintReadOnly, Category = "Components")
    class UVolumetricFogComponent* AtmosphericFog;

public:
    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetTimeOfDay(float NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void ApplyCretaceousAtmosphere();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void FixPersistentAtmosphere();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    FLinearColor CalculateSunColor(float TimeOfDay) const;

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    float CalculateSunIntensity(float TimeOfDay) const;

    UFUNCTION(BlueprintPure, Category = "Atmosphere")
    bool IsDay() const { return CurrentTimeOfDay > 0.25f && CurrentTimeOfDay < 0.75f; }

    UFUNCTION(BlueprintPure, Category = "Atmosphere")
    bool IsNight() const { return CurrentTimeOfDay < 0.25f || CurrentTimeOfDay > 0.75f; }

private:
    void UpdateSunPosition();
    void UpdateAtmosphericProperties();
    void RemoveConflictingAtmosphereActors();
};