#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/TriggerVolume.h"
#include "LightingSystemManager.h"
#include "ZoneBasedLightingComponent.generated.h"

USTRUCT(BlueprintType)
struct FZoneLightingData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone Lighting")
    FString ZoneName = "Default";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone Lighting")
    EEmotionalTone DefaultEmotionalTone = EEmotionalTone::Peaceful;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone Lighting")
    float EmotionalIntensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone Lighting")
    FLightingPreset ZonePreset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone Lighting")
    float BlendRadius = 1000.0f; // Distance for smooth transitions

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone Lighting")
    float Priority = 1.0f; // Higher priority zones override lower ones

    // Narrative context
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString NarrativeDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FString> AssociatedEvents;

    // Threat level modifiers
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat")
    float BaseThreatLevel = 0.0f; // 0 = safe, 1 = maximum danger

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat")
    bool bCanTriggerThreatLighting = true;

    // Weather overrides
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    bool bOverrideWeather = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    EWeatherState ForcedWeatherState = EWeatherState::Clear;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UZoneBasedLightingComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UZoneBasedLightingComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone Configuration")
    FZoneLightingData ZoneData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone Configuration")
    class ATriggerVolume* ZoneTriggerVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone Configuration")
    bool bAutoDetectPlayer = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone Configuration")
    float TransitionDuration = 3.0f;

    // Dynamic lighting events
    UFUNCTION(BlueprintCallable, Category = "Zone Lighting")
    void ActivateZoneLighting();

    UFUNCTION(BlueprintCallable, Category = "Zone Lighting")
    void DeactivateZoneLighting();

    UFUNCTION(BlueprintCallable, Category = "Zone Lighting")
    void SetZoneEmotionalTone(EEmotionalTone NewTone, float Intensity = 0.5f);

    UFUNCTION(BlueprintCallable, Category = "Zone Lighting")
    void TriggerZoneThreatResponse(float ThreatLevel = 1.0f, float Duration = 10.0f);

    // Player proximity detection
    UFUNCTION(BlueprintCallable, Category = "Zone Lighting")
    float GetPlayerDistanceToZone() const;

    UFUNCTION(BlueprintCallable, Category = "Zone Lighting")
    bool IsPlayerInZone() const;

    UFUNCTION(BlueprintCallable, Category = "Zone Lighting")
    float CalculateZoneInfluence() const; // 0-1 based on distance and priority

    // Events
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerEnteredZone, const FString&, ZoneName);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerExitedZone, const FString&, ZoneName);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnZoneThreatTriggered, const FString&, ZoneName, float, ThreatLevel);

    UPROPERTY(BlueprintAssignable, Category = "Zone Events")
    FOnPlayerEnteredZone OnPlayerEnteredZone;

    UPROPERTY(BlueprintAssignable, Category = "Zone Events")
    FOnPlayerExitedZone OnPlayerExitedZone;

    UPROPERTY(BlueprintAssignable, Category = "Zone Events")
    FOnZoneThreatTriggered OnZoneThreatTriggered;

private:
    UPROPERTY()
    class ALightingSystemManager* LightingManager;

    bool bPlayerWasInZone = false;
    bool bZoneActive = false;
    float LastPlayerDistance = 0.0f;

    void FindLightingManager();
    void CheckPlayerProximity();
    void HandlePlayerEnterZone();
    void HandlePlayerExitZone();
    
    // Cache for performance
    UPROPERTY()
    class APawn* CachedPlayerPawn;
    
    float PlayerCheckTimer = 0.0f;
    const float PlayerCheckInterval = 0.1f; // Check every 100ms
};