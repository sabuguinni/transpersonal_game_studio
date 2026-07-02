#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "VFXSystemManager.generated.h"

// ============================================================
// ENUMS — VFX categories (prefixed EVFX_ to avoid collisions)
// ============================================================

UENUM(BlueprintType)
enum class EVFX_WeatherState : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Overcast    UMETA(DisplayName = "Overcast"),
    Rain        UMETA(DisplayName = "Rain"),
    Storm       UMETA(DisplayName = "Storm"),
    AshFall     UMETA(DisplayName = "Volcanic Ash Fall"),
    Blizzard    UMETA(DisplayName = "Blizzard")
};

UENUM(BlueprintType)
enum class EVFX_ImpactType : uint8
{
    DinoFootstepDust    UMETA(DisplayName = "Dinosaur Footstep Dust"),
    BloodSplatter       UMETA(DisplayName = "Blood Splatter"),
    RockImpact          UMETA(DisplayName = "Rock/Stone Impact"),
    WaterSplash         UMETA(DisplayName = "Water Splash"),
    FireSpark           UMETA(DisplayName = "Fire Spark / Crafting"),
    ArrowImpact         UMETA(DisplayName = "Arrow Impact"),
    SpearImpact         UMETA(DisplayName = "Spear Impact")
};

UENUM(BlueprintType)
enum class EVFX_LODLevel : uint8
{
    High    UMETA(DisplayName = "High (< 20m)"),
    Medium  UMETA(DisplayName = "Medium (20-60m)"),
    Low     UMETA(DisplayName = "Low (60-150m)"),
    Culled  UMETA(DisplayName = "Culled (> 150m)")
};

// ============================================================
// STRUCTS — VFX configuration data
// ============================================================

USTRUCT(BlueprintType)
struct FVFX_DinosaurFootstepData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Dinosaur")
    float DinosaurMassKg = 6000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Dinosaur")
    float FootprintRadiusCm = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Dinosaur")
    float DustIntensityScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Dinosaur")
    float GroundShakeRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Dinosaur")
    FLinearColor DustColor = FLinearColor(0.8f, 0.65f, 0.4f, 1.0f);
};

USTRUCT(BlueprintType)
struct FVFX_WeatherConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Weather")
    EVFX_WeatherState CurrentState = EVFX_WeatherState::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Weather")
    float RainIntensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Weather")
    float WindSpeed = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Weather")
    float WindDirectionDegrees = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Weather")
    float AshDensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Weather")
    float TransitionDuration = 5.0f;
};

USTRUCT(BlueprintType)
struct FVFX_CampfireData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Campfire")
    float FireIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Campfire")
    float SmokeOpacity = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Campfire")
    float EmberSpawnRate = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Campfire")
    bool bIsExtinguishing = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Campfire")
    float ExtinguishProgress = 0.0f;
};

// ============================================================
// UVFX_SystemManager — Main VFX controller actor
// ============================================================

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API AVFX_SystemManager : public AActor
{
    GENERATED_BODY()

public:
    AVFX_SystemManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    // ---- Niagara System References (assign in Blueprint/Editor) ----

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Systems")
    UNiagaraSystem* NS_Fire_Campfire = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Systems")
    UNiagaraSystem* NS_Dino_FootstepDust = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Systems")
    UNiagaraSystem* NS_Combat_BloodSplatter = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Systems")
    UNiagaraSystem* NS_Weather_Rain = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Systems")
    UNiagaraSystem* NS_Weather_VolcanicAsh = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Systems")
    UNiagaraSystem* NS_Combat_RockImpact = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Systems")
    UNiagaraSystem* NS_Crafting_FireSpark = nullptr;

    // ---- Weather State ----

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Weather")
    FVFX_WeatherConfig WeatherConfig;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX|Weather")
    UNiagaraComponent* ActiveWeatherComponent = nullptr;

    // ---- LOD Settings ----

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|LOD")
    float LOD_HighDistance = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|LOD")
    float LOD_MediumDistance = 6000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|LOD")
    float LOD_LowDistance = 15000.0f;

    // ---- Public API ----

    UFUNCTION(BlueprintCallable, Category = "VFX|Campfire")
    UNiagaraComponent* SpawnCampfireVFX(FVector Location, FVFX_CampfireData CampfireData);

    UFUNCTION(BlueprintCallable, Category = "VFX|Dinosaur")
    void SpawnFootstepDustVFX(FVector Location, FVFX_DinosaurFootstepData FootstepData);

    UFUNCTION(BlueprintCallable, Category = "VFX|Combat")
    void SpawnImpactVFX(FVector Location, FVector Normal, EVFX_ImpactType ImpactType, float IntensityScale = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Weather")
    void SetWeatherState(EVFX_WeatherState NewState, float TransitionTime = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Weather")
    void UpdateWeatherIntensity(float RainIntensity, float WindSpeed, float AshDensity);

    UFUNCTION(BlueprintCallable, Category = "VFX|LOD")
    EVFX_LODLevel GetLODLevelForDistance(float DistanceCm) const;

    UFUNCTION(BlueprintCallable, Category = "VFX|Crafting")
    void SpawnCraftingSparkVFX(FVector Location, int32 NumSparks = 10);

    UFUNCTION(BlueprintCallable, Category = "VFX|Dinosaur")
    void SpawnDinoBreathVaporVFX(FVector MouthLocation, FVector ForwardDir, float DinoBodyTempC);

private:
    void UpdateWeatherVFX(float DeltaTime);
    float WeatherTransitionAlpha = 0.0f;
    EVFX_WeatherState TargetWeatherState = EVFX_WeatherState::Clear;
    bool bWeatherTransitioning = false;
    float WeatherTransitionTime = 5.0f;
    float WeatherTransitionElapsed = 0.0f;

    APawn* CachedPlayerPawn = nullptr;
    float PlayerDistanceCache = 0.0f;
    float DistanceCacheTimer = 0.0f;
    static constexpr float DistanceCacheInterval = 0.1f;
};
