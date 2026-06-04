#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "SharedTypes.h"
#include "Audio_AdaptiveMusicManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_MusicLayer : uint8
{
    Ambient     UMETA(DisplayName = "Ambient"),
    Tension     UMETA(DisplayName = "Tension"),
    Combat      UMETA(DisplayName = "Combat"),
    Exploration UMETA(DisplayName = "Exploration"),
    Danger      UMETA(DisplayName = "Danger")
};

UENUM(BlueprintType)
enum class EAudio_BiomeType : uint8
{
    Forest      UMETA(DisplayName = "Forest"),
    Plains      UMETA(DisplayName = "Plains"),
    River       UMETA(DisplayName = "River"),
    Mountains   UMETA(DisplayName = "Mountains"),
    Caves       UMETA(DisplayName = "Caves")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_MusicState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EAudio_MusicLayer CurrentLayer = EAudio_MusicLayer::Ambient;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EAudio_BiomeType CurrentBiome = EAudio_BiomeType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float IntensityLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CrossfadeTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsInCombat = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bDangerNearby = false;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAudio_AdaptiveMusicManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UAudio_AdaptiveMusicManager();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Music control
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetMusicLayer(EAudio_MusicLayer NewLayer, float CrossfadeTime = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetBiome(EAudio_BiomeType NewBiome);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetIntensity(float NewIntensity);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void OnCombatStart();

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void OnCombatEnd();

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void OnDangerDetected(bool bDangerous);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void UpdateMusicState(float DeltaTime);

    // Getters
    UFUNCTION(BlueprintPure, Category = "Audio")
    FAudio_MusicState GetCurrentMusicState() const { return CurrentMusicState; }

    UFUNCTION(BlueprintPure, Category = "Audio")
    bool IsInCombat() const { return CurrentMusicState.bIsInCombat; }

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FAudio_MusicState CurrentMusicState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TMap<EAudio_MusicLayer, TSoftObjectPtr<USoundCue>> MusicLayers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TMap<EAudio_BiomeType, TSoftObjectPtr<USoundCue>> BiomeAmbience;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    UAudioComponent* MusicAudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    UAudioComponent* AmbienceAudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float MasterVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float MusicVolume = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float AmbienceVolume = 0.5f;

private:
    void CrossfadeToLayer(EAudio_MusicLayer NewLayer, float CrossfadeTime);
    void UpdateBiomeAmbience();
    void CalculateAdaptiveIntensity();

    float LastIntensityUpdate = 0.0f;
    float IntensityUpdateInterval = 1.0f;
};