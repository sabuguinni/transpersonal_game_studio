#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "Audio_AdaptiveAudioManager.generated.h"

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudio_AdaptiveAudioManager : public AActor
{
    GENERATED_BODY()

public:
    AAudio_AdaptiveAudioManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Audio components for different biomes
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio")
    UAudioComponent* SavanaAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio")
    UAudioComponent* FlorestaAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio")
    UAudioComponent* PantanoAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio")
    UAudioComponent* DesertoAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio")
    UAudioComponent* MontanhaAudioComponent;

    // Current biome detection
    UPROPERTY(BlueprintReadOnly, Category = "Audio")
    EBiomeType CurrentBiome;

    // Audio transition settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float TransitionDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float BiomeDetectionRadius;

    // Proximity detection for dinosaurs
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Proximity")
    USphereComponent* ProximityDetectionSphere;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proximity")
    float DinosaurProximityRadius;

    // Audio volume controls
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float MasterVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float AmbienceVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float ProximityAlertVolume;

public:
    // Biome detection and audio switching
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void DetectCurrentBiome();

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SwitchToBiomeAudio(EBiomeType NewBiome);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void CrossfadeAudio(UAudioComponent* FromComponent, UAudioComponent* ToComponent);

    // Proximity detection
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void CheckDinosaurProximity();

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void TriggerProximityAlert(float Distance, EDinosaurSpecies Species);

    // Audio zone management
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void UpdateAudioZones();

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetBiomeAudioVolume(EBiomeType Biome, float Volume);

protected:
    // Internal audio management
    void InitializeAudioComponents();
    void SetupProximityDetection();
    EBiomeType GetBiomeAtLocation(const FVector& Location);
    void UpdateAudioBasedOnTimeOfDay();
    void UpdateAudioBasedOnWeather();

private:
    // Audio transition state
    bool bIsTransitioning;
    float TransitionTimer;
    UAudioComponent* TransitionFromComponent;
    UAudioComponent* TransitionToComponent;

    // Cached references
    class ATranspersonalCharacter* PlayerCharacter;
    class ATranspersonalGameState* GameStateRef;
};