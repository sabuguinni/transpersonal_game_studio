#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "MetasoundSource.h"
#include "Audio_MetaSoundManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_BiomeType : uint8
{
    Forest      UMETA(DisplayName = "Forest"),
    Plains      UMETA(DisplayName = "Plains"),
    River       UMETA(DisplayName = "River"),
    Mountain    UMETA(DisplayName = "Mountain"),
    Cave        UMETA(DisplayName = "Cave")
};

UENUM(BlueprintType)
enum class EAudio_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Day         UMETA(DisplayName = "Day"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night")
};

UENUM(BlueprintType)
enum class EAudio_WeatherState : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Cloudy      UMETA(DisplayName = "Cloudy"),
    Rain        UMETA(DisplayName = "Rain"),
    Storm       UMETA(DisplayName = "Storm"),
    Fog         UMETA(DisplayName = "Fog")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_BiomeAudioData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    TObjectPtr<UMetaSoundSource> AmbientMetaSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    float BaseVolume = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    float FadeInTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    float FadeOutTime = 1.5f;

    FAudio_BiomeAudioData()
    {
        AmbientMetaSound = nullptr;
        BaseVolume = 0.5f;
        FadeInTime = 2.0f;
        FadeOutTime = 1.5f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_ThreatAudioData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Audio")
    TObjectPtr<UMetaSoundSource> ThreatMetaSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Audio")
    float TriggerDistance = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Audio")
    float MaxVolume = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Audio")
    float FadeSpeed = 3.0f;

    FAudio_ThreatAudioData()
    {
        ThreatMetaSound = nullptr;
        TriggerDistance = 1500.0f;
        MaxVolume = 0.8f;
        FadeSpeed = 3.0f;
    }
};

UCLASS(Blueprintable, BlueprintType)
class TRANSPERSONALGAME_API AAudio_MetaSoundManager : public AActor
{
    GENERATED_BODY()

public:
    AAudio_MetaSoundManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USceneComponent> RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UAudioComponent> BiomeAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UAudioComponent> WeatherAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UAudioComponent> ThreatAudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Data")
    TMap<EAudio_BiomeType, FAudio_BiomeAudioData> BiomeAudioMap;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Data")
    TMap<EAudio_WeatherState, TObjectPtr<UMetaSoundSource>> WeatherAudioMap;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Data")
    FAudio_ThreatAudioData TRexThreatAudio;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    EAudio_BiomeType CurrentBiome;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    EAudio_TimeOfDay CurrentTimeOfDay;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    EAudio_WeatherState CurrentWeather;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bThreatActive;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float ThreatLevel;

public:
    UFUNCTION(BlueprintCallable, Category = "Audio Control")
    void SetBiome(EAudio_BiomeType NewBiome);

    UFUNCTION(BlueprintCallable, Category = "Audio Control")
    void SetTimeOfDay(EAudio_TimeOfDay NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Audio Control")
    void SetWeather(EAudio_WeatherState NewWeather);

    UFUNCTION(BlueprintCallable, Category = "Audio Control")
    void TriggerThreatAudio(float ThreatDistance);

    UFUNCTION(BlueprintCallable, Category = "Audio Control")
    void StopThreatAudio();

    UFUNCTION(BlueprintCallable, Category = "Audio Control")
    void UpdatePlayerFootsteps(bool bIsWalking, bool bIsRunning);

    UFUNCTION(BlueprintPure, Category = "Audio State")
    EAudio_BiomeType GetCurrentBiome() const { return CurrentBiome; }

    UFUNCTION(BlueprintPure, Category = "Audio State")
    bool IsThreatActive() const { return bThreatActive; }

private:
    void UpdateBiomeAudio();
    void UpdateWeatherAudio();
    void UpdateThreatAudio(float DeltaTime);
    void CrossfadeAudio(UAudioComponent* FromComponent, UAudioComponent* ToComponent, float FadeTime);
};