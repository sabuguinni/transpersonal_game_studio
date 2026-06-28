#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VFXManager.generated.h"

UENUM(BlueprintType)
enum class EVFX_EffectType : uint8
{
    Campfire        UMETA(DisplayName = "Campfire"),
    Rain            UMETA(DisplayName = "Rain"),
    DinoFootstep    UMETA(DisplayName = "DinoFootstep"),
    BloodImpact     UMETA(DisplayName = "BloodImpact"),
    DustCloud       UMETA(DisplayName = "DustCloud"),
    VolcanicAsh     UMETA(DisplayName = "VolcanicAsh"),
    WaterSplash     UMETA(DisplayName = "WaterSplash"),
    Sparks          UMETA(DisplayName = "Sparks")
};

USTRUCT(BlueprintType)
struct FVFX_EffectConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    EVFX_EffectType EffectType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FVector SpawnLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float Scale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    bool bLooping;

    FVFX_EffectConfig()
        : EffectType(EVFX_EffectType::Campfire)
        , SpawnLocation(FVector::ZeroVector)
        , Scale(1.0f)
        , Duration(5.0f)
        , bLooping(false)
    {}
};

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UVFX_Manager : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFX_Manager();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnEffect(const FVFX_EffectConfig& Config);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnCampfireEffect(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnDinoFootstepEffect(FVector Location, float DinoMass);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnBloodImpactEffect(FVector Location, FVector ImpactNormal);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnRainEffect(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnDustCloudEffect(FVector Location, float Radius);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SetWeatherIntensity(float Intensity);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Settings")
    float GlobalVFXQuality;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Settings")
    bool bEnableBloodEffects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Settings")
    bool bEnableWeatherEffects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Settings")
    int32 MaxConcurrentEffects;

private:
    UPROPERTY()
    float CurrentWeatherIntensity;

    UPROPERTY()
    int32 ActiveEffectCount;

    void CleanupExpiredEffects();
};
