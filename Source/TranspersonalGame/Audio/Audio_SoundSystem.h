#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Core/SharedTypes.h"
#include "Audio_SoundSystem.generated.h"

/**
 * AUDIO AGENT #16 - PREHISTORIC SOUNDSCAPE SYSTEM
 * 
 * Manages ambient audio, environmental sounds, and atmospheric audio
 * for the prehistoric survival game. Creates immersive soundscapes
 * that respond to biome, danger level, and time of day.
 */

UENUM(BlueprintType)
enum class EAudio_BiomeAmbience : uint8
{
    Forest = 0          UMETA(DisplayName = "Forest"),
    Grassland = 1       UMETA(DisplayName = "Grassland"),
    Swamp = 2           UMETA(DisplayName = "Swamp"),
    Desert = 3          UMETA(DisplayName = "Desert"),
    Mountains = 4       UMETA(DisplayName = "Mountains"),
    River = 5           UMETA(DisplayName = "River"),
    DangerZone = 6      UMETA(DisplayName = "Danger Zone")
};

UENUM(BlueprintType)
enum class EAudio_IntensityLevel : uint8
{
    Calm = 0            UMETA(DisplayName = "Calm"),
    Tense = 1           UMETA(DisplayName = "Tense"),
    Dangerous = 2       UMETA(DisplayName = "Dangerous"),
    Critical = 3        UMETA(DisplayName = "Critical")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_SoundZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float Radius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    EAudio_BiomeAmbience BiomeType = EAudio_BiomeAmbience::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float Volume = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    bool bIsActive = true;

    FAudio_SoundZone()
    {
        Location = FVector::ZeroVector;
        Radius = 1000.0f;
        BiomeType = EAudio_BiomeAmbience::Forest;
        Volume = 0.7f;
        bIsActive = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudio_SoundSystem : public AActor
{
    GENERATED_BODY()

public:
    AAudio_SoundSystem();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ═══════════════════════════════════════════════════════════════
    // CORE AUDIO COMPONENTS
    // ═══════════════════════════════════════════════════════════════

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* MasterAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* ForestAmbienceComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* DangerAmbienceComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* WaterAmbienceComponent;

    // ═══════════════════════════════════════════════════════════════
    // AUDIO SYSTEM PROPERTIES
    // ═══════════════════════════════════════════════════════════════

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float MasterVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float AmbienceVolume = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float EffectsVolume = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    EAudio_IntensityLevel CurrentIntensity = EAudio_IntensityLevel::Calm;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    TArray<FAudio_SoundZone> SoundZones;

    // ═══════════════════════════════════════════════════════════════
    // DYNAMIC AUDIO SYSTEM
    // ═══════════════════════════════════════════════════════════════

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Audio")
    float TimeOfDay = 12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Audio")
    float WeatherIntensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Audio")
    bool bNearDinosaurs = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Audio")
    float PlayerFearLevel = 0.0f;

public:
    // ═══════════════════════════════════════════════════════════════
    // PUBLIC INTERFACE
    // ═══════════════════════════════════════════════════════════════

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetMasterVolume(float NewVolume);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetIntensityLevel(EAudio_IntensityLevel NewIntensity);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void UpdatePlayerLocation(FVector PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void TriggerDangerAudio(bool bDangerActive);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetTimeOfDay(float NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PlayFootstepSound(FVector Location, bool bIsHeavy = false);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PlayDinosaurRoar(FVector Location, float Intensity = 1.0f);

private:
    // ═══════════════════════════════════════════════════════════════
    // INTERNAL METHODS
    // ═══════════════════════════════════════════════════════════════

    void UpdateAmbienceBasedOnLocation(FVector PlayerLocation);
    void UpdateIntensityBasedOnContext();
    void FadeAudioComponent(UAudioComponent* Component, float TargetVolume, float FadeTime);
    FAudio_SoundZone* GetNearestSoundZone(FVector Location);
    
    // Timer handles for audio updates
    FTimerHandle AmbienceUpdateTimer;
    FTimerHandle IntensityUpdateTimer;
};