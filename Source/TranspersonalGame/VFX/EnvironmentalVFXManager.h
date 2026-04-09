// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "VFXTypes.h"
#include "VFXManager.h"
#include "EnvironmentalVFXManager.generated.h"

class UVolumeComponent;
class APostProcessVolume;

/**
 * Environmental VFX Types
 * Categories of environmental visual effects
 */
UENUM(BlueprintType)
enum class EEnvironmentalVFXType : uint8
{
    Weather         UMETA(DisplayName = "Weather - Rain, fog, snow"),
    Atmospheric     UMETA(DisplayName = "Atmospheric - Dust, pollen, spores"),
    Water           UMETA(DisplayName = "Water - Rivers, waterfalls, splashes"),
    Vegetation      UMETA(DisplayName = "Vegetation - Swaying grass, falling leaves"),
    Geological      UMETA(DisplayName = "Geological - Steam vents, dust clouds"),
    Lighting        UMETA(DisplayName = "Lighting - God rays, fireflies"),
    Magical         UMETA(DisplayName = "Magical - Consciousness effects, portals"),
    Ambient         UMETA(DisplayName = "Ambient - General atmosphere")
};

/**
 * Weather State for dynamic weather VFX
 */
UENUM(BlueprintType)
enum class EWeatherState : uint8
{
    Clear           UMETA(DisplayName = "Clear - Sunny, no precipitation"),
    LightRain       UMETA(DisplayName = "Light Rain - Gentle rainfall"),
    HeavyRain       UMETA(DisplayName = "Heavy Rain - Intense downpour"),
    Thunderstorm    UMETA(DisplayName = "Thunderstorm - Rain with lightning"),
    Fog             UMETA(DisplayName = "Fog - Dense mist"),
    LightFog        UMETA(DisplayName = "Light Fog - Thin mist"),
    Overcast        UMETA(DisplayName = "Overcast - Cloudy, no precipitation"),
    WindyDust       UMETA(DisplayName = "Windy Dust - Dust storms"),
    Humid           UMETA(DisplayName = "Humid - Heavy air, heat haze")
};

/**
 * Time of Day for lighting-based VFX
 */
UENUM(BlueprintType)
enum class ETimeOfDay : uint8
{
    Dawn            UMETA(DisplayName = "Dawn - Early morning"),
    Morning         UMETA(DisplayName = "Morning - Mid morning"),
    Noon            UMETA(DisplayName = "Noon - Midday"),
    Afternoon       UMETA(DisplayName = "Afternoon - Late afternoon"),
    Dusk            UMETA(DisplayName = "Dusk - Evening"),
    Night           UMETA(DisplayName = "Night - Dark hours"),
    Midnight        UMETA(DisplayName = "Midnight - Deep night")
};

/**
 * Environmental VFX Zone Configuration
 * Defines VFX behavior for a specific area/biome
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvironmentalVFXZone
{
    GENERATED_BODY()

    // Zone identification
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    FString ZoneID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    FString DisplayName;

    // Zone bounds (for area-based effects)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bounds")
    FVector ZoneCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bounds")
    FVector ZoneExtents = FVector(1000.0f, 1000.0f, 500.0f);

    // Base atmospheric effects (always active)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base Effects")
    TMap<EEnvironmentalVFXType, FString> BaseEffects;

    // Weather-specific effects
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Effects")
    TMap<EWeatherState, TArray<FString>> WeatherEffects;

    // Time-of-day effects
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Effects")
    TMap<ETimeOfDay, TArray<FString>> TimeEffects;

    // Transition effects (between weather/time states)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transitions")
    TMap<FString, FString> TransitionEffects; // "FromState_ToState" -> Effect ID

    // Environmental interaction effects
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactions")
    FString PlayerEnterEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactions")
    FString PlayerExitEffect;

    // Performance settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float EffectDensity = 1.0f; // Multiplier for particle density

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float UpdateFrequency = 1.0f; // How often to update effects (seconds)

    FEnvironmentalVFXZone()
    {
        ZoneID = TEXT("");
        DisplayName = TEXT("Unnamed Zone");
        ZoneCenter = FVector::ZeroVector;
        ZoneExtents = FVector(1000.0f, 1000.0f, 500.0f);
        EffectDensity = 1.0f;
        UpdateFrequency = 1.0f;
        PlayerEnterEffect = TEXT("");
        PlayerExitEffect = TEXT("");
    }
};

/**
 * Active Environmental VFX
 * Tracks currently active environmental effects
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FActiveEnvironmentalVFX
{
    GENERATED_BODY()

    // VFX Manager instance ID
    UPROPERTY(BlueprintReadOnly, Category = "Instance")
    int32 VFXInstanceID = -1;

    // Effect ID
    UPROPERTY(BlueprintReadOnly, Category = "Instance")
    FString EffectID;

    // Type of environmental effect
    UPROPERTY(BlueprintReadOnly, Category = "Instance")
    EEnvironmentalVFXType EffectType = EEnvironmentalVFXType::Ambient;

    // Zone this effect belongs to
    UPROPERTY(BlueprintReadOnly, Category = "Instance")
    FString ZoneID;

    // Associated weather state (if applicable)
    UPROPERTY(BlueprintReadOnly, Category = "Instance")
    EWeatherState WeatherState = EWeatherState::Clear;

    // Associated time of day (if applicable)
    UPROPERTY(BlueprintReadOnly, Category = "Instance")
    ETimeOfDay TimeOfDay = ETimeOfDay::Noon;

    // Is this a persistent effect?
    UPROPERTY(BlueprintReadOnly, Category = "Instance")
    bool bIsPersistent = false;

    // Start time
    UPROPERTY(BlueprintReadOnly, Category = "Instance")
    float StartTime = 0.0f;

    FActiveEnvironmentalVFX()
    {
        VFXInstanceID = -1;
        EffectID = TEXT("");
        EffectType = EEnvironmentalVFXType::Ambient;
        ZoneID = TEXT("");
        WeatherState = EWeatherState::Clear;
        TimeOfDay = ETimeOfDay::Noon;
        bIsPersistent = false;
        StartTime = 0.0f;
    }
};

/**
 * Environmental VFX Manager
 * Manages large-scale environmental visual effects
 * Handles weather, atmospheric effects, and zone-based VFX
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UEnvironmentalVFXManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UEnvironmentalVFXManager();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Zone Management
    UFUNCTION(BlueprintCallable, Category = "Environmental VFX")
    void RegisterVFXZone(const FEnvironmentalVFXZone& Zone);

    UFUNCTION(BlueprintCallable, Category = "Environmental VFX")
    void UnregisterVFXZone(const FString& ZoneID);

    UFUNCTION(BlueprintCallable, Category = "Environmental VFX")
    bool GetVFXZone(const FString& ZoneID, FEnvironmentalVFXZone& OutZone) const;

    UFUNCTION(BlueprintCallable, Category = "Environmental VFX")
    TArray<FString> GetActiveZoneIDs() const;

    // Weather System
    UFUNCTION(BlueprintCallable, Category = "Environmental VFX")
    void SetWeatherState(EWeatherState NewWeatherState, float TransitionDuration = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "Environmental VFX")
    EWeatherState GetCurrentWeatherState() const;

    UFUNCTION(BlueprintCallable, Category = "Environmental VFX")
    void SetWeatherIntensity(float Intensity); // 0.0 to 1.0

    UFUNCTION(BlueprintCallable, Category = "Environmental VFX")
    float GetWeatherIntensity() const;

    // Time of Day System
    UFUNCTION(BlueprintCallable, Category = "Environmental VFX")
    void SetTimeOfDay(ETimeOfDay NewTimeOfDay, float TransitionDuration = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Environmental VFX")
    ETimeOfDay GetCurrentTimeOfDay() const;

    // Zone Activation
    UFUNCTION(BlueprintCallable, Category = "Environmental VFX")
    void ActivateZone(const FString& ZoneID);

    UFUNCTION(BlueprintCallable, Category = "Environmental VFX")
    void DeactivateZone(const FString& ZoneID);

    UFUNCTION(BlueprintCallable, Category = "Environmental VFX")
    bool IsZoneActive(const FString& ZoneID) const;

    // Player Interaction
    UFUNCTION(BlueprintCallable, Category = "Environmental VFX")
    void OnPlayerEnterZone(const FString& ZoneID);

    UFUNCTION(BlueprintCallable, Category = "Environmental VFX")
    void OnPlayerExitZone(const FString& ZoneID);

    UFUNCTION(BlueprintCallable, Category = "Environmental VFX")
    FString GetPlayerCurrentZone() const;

    // Effect Control
    UFUNCTION(BlueprintCallable, Category = "Environmental VFX")
    void StopAllEnvironmentalEffects();

    UFUNCTION(BlueprintCallable, Category = "Environmental VFX")
    void StopEffectsByType(EEnvironmentalVFXType EffectType);

    UFUNCTION(BlueprintCallable, Category = "Environmental VFX")
    void StopEffectsByZone(const FString& ZoneID);

    // Utility
    UFUNCTION(BlueprintCallable, Category = "Environmental VFX")
    TArray<int32> GetActiveEnvironmentalVFX() const;

    UFUNCTION(BlueprintCallable, Category = "Environmental VFX")
    int32 GetActiveEffectCountByType(EEnvironmentalVFXType EffectType) const;

    // Global Settings
    UFUNCTION(BlueprintCallable, Category = "Environmental VFX")
    void SetGlobalEffectDensity(float Density);

    UFUNCTION(BlueprintCallable, Category = "Environmental VFX")
    float GetGlobalEffectDensity() const;

    UFUNCTION(BlueprintCallable, Category = "Environmental VFX")
    void SetEnvironmentalVFXEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Environmental VFX")
    bool IsEnvironmentalVFXEnabled() const;

    // Events
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWeatherStateChanged, EWeatherState, OldState, EWeatherState, NewState);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTimeOfDayChanged, ETimeOfDay, OldTime, ETimeOfDay, NewTime);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerZoneChanged, const FString&, OldZone, const FString&, NewZone);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnEnvironmentalVFXTriggered, const FString&, EffectID, EEnvironmentalVFXType, EffectType, const FString&, ZoneID);

    UPROPERTY(BlueprintAssignable, Category = "Environmental VFX Events")
    FOnWeatherStateChanged OnWeatherStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "Environmental VFX Events")
    FOnTimeOfDayChanged OnTimeOfDayChanged;

    UPROPERTY(BlueprintAssignable, Category = "Environmental VFX Events")
    FOnPlayerZoneChanged OnPlayerZoneChanged;

    UPROPERTY(BlueprintAssignable, Category = "Environmental VFX Events")
    FOnEnvironmentalVFXTriggered OnEnvironmentalVFXTriggered;

protected:
    // Internal Management
    void UpdateWeatherEffects();
    void UpdateTimeOfDayEffects();
    void UpdateZoneEffects();
    void UpdatePlayerZone();
    
    void TransitionWeatherState(EWeatherState FromState, EWeatherState ToState, float Duration);
    void TransitionTimeOfDay(ETimeOfDay FromTime, ETimeOfDay ToTime, float Duration);
    
    void StartZoneEffects(const FString& ZoneID);
    void StopZoneEffects(const FString& ZoneID);
    
    void CleanupFinishedEffects();
    
    // Utility Methods
    UVFXManager* GetVFXManager() const;
    bool IsPlayerInZone(const FString& ZoneID) const;
    FVector GetPlayerLocation() const;
    
    int32 SpawnEnvironmentalVFX(const FString& EffectID, EEnvironmentalVFXType EffectType, const FString& ZoneID);
    void RegisterEnvironmentalVFX(int32 VFXInstanceID, const FString& EffectID, EEnvironmentalVFXType EffectType, const FString& ZoneID);

private:
    // Registered zones
    UPROPERTY()
    TMap<FString, FEnvironmentalVFXZone> VFXZones;

    // Active zones
    UPROPERTY()
    TSet<FString> ActiveZones;

    // Current states
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", meta = (AllowPrivateAccess = "true"))
    EWeatherState CurrentWeatherState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", meta = (AllowPrivateAccess = "true"))
    ETimeOfDay CurrentTimeOfDay;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", meta = (AllowPrivateAccess = "true"))
    FString PlayerCurrentZone;

    // Transition states
    bool bInWeatherTransition;
    bool bInTimeTransition;
    EWeatherState TargetWeatherState;
    ETimeOfDay TargetTimeOfDay;
    float WeatherTransitionStartTime;
    float TimeTransitionStartTime;
    float WeatherTransitionDuration;
    float TimeTransitionDuration;

    // Active effects
    UPROPERTY()
    TArray<FActiveEnvironmentalVFX> ActiveEffects;

    // Settings
    UPROPERTY(EditAnywhere, Category = "Settings")
    float GlobalEffectDensity;

    UPROPERTY(EditAnywhere, Category = "Settings")
    float WeatherIntensity;

    UPROPERTY(EditAnywhere, Category = "Settings")
    bool bEnvironmentalVFXEnabled;

    UPROPERTY(EditAnywhere, Category = "Settings")
    float ZoneUpdateFrequency; // How often to check player zone

    // Performance tracking
    float LastZoneUpdate;
    float LastEffectCleanup;
    static const float ZONE_UPDATE_INTERVAL;
    static const float EFFECT_CLEANUP_INTERVAL;
};