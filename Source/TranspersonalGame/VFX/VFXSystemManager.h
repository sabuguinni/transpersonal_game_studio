#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VFXSystemManager.generated.h"

// === ENUMS (global scope — RULE 1) ===

UENUM(BlueprintType)
enum class EVFX_WeatherType : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    LightRain   UMETA(DisplayName = "Light Rain"),
    HeavyRain   UMETA(DisplayName = "Heavy Rain"),
    Fog         UMETA(DisplayName = "Fog"),
    Storm       UMETA(DisplayName = "Storm"),
    Snow        UMETA(DisplayName = "Snow")
};

UENUM(BlueprintType)
enum class EVFX_EffectCategory : uint8
{
    Environment UMETA(DisplayName = "Environment"),
    Dinosaur    UMETA(DisplayName = "Dinosaur"),
    Combat      UMETA(DisplayName = "Combat"),
    Weather     UMETA(DisplayName = "Weather"),
    Volcanic    UMETA(DisplayName = "Volcanic")
};

// === STRUCTS (global scope — RULE 1) ===

USTRUCT(BlueprintType)
struct FVFX_CampfireData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Campfire")
    float FlameIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Campfire")
    float SmokeOpacity = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Campfire")
    int32 EmberCount = 40;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Campfire")
    float LightRadius = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Campfire")
    FLinearColor LightColor = FLinearColor(1.0f, 0.45f, 0.1f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Campfire")
    bool bIsActive = false;
};

USTRUCT(BlueprintType)
struct FVFX_FootstepData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Footstep")
    int32 DustParticleCount = 20;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Footstep")
    float DustRadius = 80.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Footstep")
    float DustLifetime = 1.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Footstep")
    float ImpactForce = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Footstep")
    bool bHeavyCreature = false;
};

// === MAIN VFX COMPONENT CLASS ===

UCLASS(ClassGroup = "VFX", meta = (BlueprintSpawnableComponent), DisplayName = "VFX System Manager")
class TRANSPERSONALGAME_API UVFX_SystemManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFX_SystemManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // === CAMPFIRE ===
    UFUNCTION(BlueprintCallable, Category = "VFX|Campfire")
    void SpawnCampfireVFX(FVector Location, float Intensity = 1.0f);

    // === DINOSAUR FOOTSTEP ===
    UFUNCTION(BlueprintCallable, Category = "VFX|Dinosaur")
    void SpawnFootstepDust(FVector ImpactLocation, float CreatureWeight = 1000.0f);

    // === WEATHER ===
    UFUNCTION(BlueprintCallable, Category = "VFX|Weather")
    void SetWeatherState(EVFX_WeatherType WeatherType, float Intensity = 1.0f);

    // === COMBAT ===
    UFUNCTION(BlueprintCallable, Category = "VFX|Combat")
    void SpawnBloodImpact(FVector HitLocation, FVector HitNormal, float DamageAmount = 25.0f);

    // === VOLCANIC ===
    UFUNCTION(BlueprintCallable, Category = "VFX|Volcanic")
    void SetVolcanicAsh(bool bActive, float Density = 0.5f);

    // === PROPERTIES ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Campfire")
    FVFX_CampfireData CampfireData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Footstep")
    FVFX_FootstepData FootstepData;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX|Weather")
    EVFX_WeatherType CurrentWeather;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Weather")
    float RainIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Weather")
    float FogDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Volcanic")
    bool bVolcanicAshActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Volcanic")
    float AshDensity;

private:
    UPROPERTY()
    AActor* CampfireLightRef = nullptr;
};
