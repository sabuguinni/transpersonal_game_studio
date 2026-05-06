#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DirectionalLight.h"
#include "Components/SkyLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/PostProcessVolume.h"
#include "TimerManager.h"
#include "DynamicLightingManager.generated.h"

USTRUCT(BlueprintType)
struct FLight_BiomeLightingSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SkyLightColor = FLinearColor(0.5f, 0.7f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SkyLightIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor FogColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float FogStartDistance = 0.0f;
};

UENUM(BlueprintType)
enum class ELight_BiomeType : uint8
{
    Swamp       UMETA(DisplayName = "Swamp"),
    Forest      UMETA(DisplayName = "Forest"),
    Savanna     UMETA(DisplayName = "Savanna"),
    Desert      UMETA(DisplayName = "Desert"),
    Mountain    UMETA(DisplayName = "Mountain")
};

UENUM(BlueprintType)
enum class ELight_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    Noon        UMETA(DisplayName = "Noon"),
    Afternoon   UMETA(DisplayName = "Afternoon"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night")
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADynamicLightingManager : public AActor
{
    GENERATED_BODY()

public:
    ADynamicLightingManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle")
    float DayDurationMinutes = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle")
    float CurrentTimeOfDay = 0.5f; // 0.0 = midnight, 0.5 = noon

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle")
    bool bEnableDayNightCycle = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Settings")
    TMap<ELight_BiomeType, FLight_BiomeLightingSettings> BiomeLightingSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Current State")
    ELight_BiomeType CurrentBiome = ELight_BiomeType::Savanna;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Current State")
    ELight_TimeOfDay CurrentTimeOfDayEnum = ELight_TimeOfDay::Noon;

    // Actor References
    UPROPERTY(BlueprintReadOnly, Category = "Lighting Actors")
    ADirectionalLight* SunLight;

    UPROPERTY(BlueprintReadOnly, Category = "Lighting Actors")
    AActor* SkyLightActor;

    UPROPERTY(BlueprintReadOnly, Category = "Lighting Actors")
    AActor* SkyAtmosphereActor;

    UPROPERTY(BlueprintReadOnly, Category = "Lighting Actors")
    AActor* HeightFogActor;

    UPROPERTY(BlueprintReadOnly, Category = "Lighting Actors")
    APostProcessVolume* PostProcessVolume;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetBiome(ELight_BiomeType NewBiome);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetTimeOfDay(float NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void UpdateLighting();

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void FindLightingActors();

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    ELight_BiomeType GetBiomeAtLocation(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    ELight_TimeOfDay GetTimeOfDayEnum() const;

private:
    void InitializeBiomeSettings();
    void UpdateSunPosition();
    void UpdateSunColor();
    void UpdateSkyLight();
    void UpdateFog();
    void UpdatePostProcessing();
    
    FTimerHandle LightingUpdateTimer;
    float LastUpdateTime = 0.0f;
};

#include "DynamicLightingManager.generated.h"