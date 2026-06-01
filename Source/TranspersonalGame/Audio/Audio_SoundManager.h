#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundBase.h"
#include "SharedTypes.h"
#include "Audio_SoundManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_SoundEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	TSoftObjectPtr<USoundBase> Sound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	float Volume = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	float Pitch = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	bool bIs3D = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	float MaxDistance = 5000.0f;

	FAudio_SoundEntry()
	{
		Volume = 1.0f;
		Pitch = 1.0f;
		bIs3D = true;
		MaxDistance = 5000.0f;
	}
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_AmbienceLayer
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	TSoftObjectPtr<USoundBase> AmbienceSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	float BaseVolume = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	float FadeInTime = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	float FadeOutTime = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	EBiomeType BiomeType = EBiomeType::Savanna;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	bool bIsActive = false;

	FAudio_AmbienceLayer()
	{
		BaseVolume = 0.5f;
		FadeInTime = 2.0f;
		FadeOutTime = 2.0f;
		BiomeType = EBiomeType::Savanna;
		bIsActive = false;
	}
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAudio_SoundManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UAudio_SoundManager();

	// Subsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Sound playback
	UFUNCTION(BlueprintCallable, Category = "Audio")
	void PlaySoundAtLocation(const FString& SoundID, FVector Location, float VolumeMultiplier = 1.0f);

	UFUNCTION(BlueprintCallable, Category = "Audio")
	void PlaySound2D(const FString& SoundID, float VolumeMultiplier = 1.0f);

	UFUNCTION(BlueprintCallable, Category = "Audio")
	UAudioComponent* PlaySoundAttached(const FString& SoundID, USceneComponent* AttachComponent, float VolumeMultiplier = 1.0f);

	// Ambience management
	UFUNCTION(BlueprintCallable, Category = "Audio")
	void SetBiomeAmbience(EBiomeType BiomeType, float FadeTime = 2.0f);

	UFUNCTION(BlueprintCallable, Category = "Audio")
	void StopAllAmbience(float FadeTime = 2.0f);

	// Dynamic audio
	UFUNCTION(BlueprintCallable, Category = "Audio")
	void SetTimeOfDayAudioMix(float TimeOfDay);

	UFUNCTION(BlueprintCallable, Category = "Audio")
	void SetWeatherAudioMix(EWeatherType WeatherType, float Intensity);

	// Dinosaur audio
	UFUNCTION(BlueprintCallable, Category = "Audio")
	void PlayDinosaurSound(EDinosaurSpecies Species, const FString& SoundType, FVector Location);

	UFUNCTION(BlueprintCallable, Category = "Audio")
	void SetDinosaurProximityAudio(float Distance, EDinosaurSpecies Species);

	// Master volume controls
	UFUNCTION(BlueprintCallable, Category = "Audio")
	void SetMasterVolume(float Volume);

	UFUNCTION(BlueprintCallable, Category = "Audio")
	void SetSFXVolume(float Volume);

	UFUNCTION(BlueprintCallable, Category = "Audio")
	void SetMusicVolume(float Volume);

	UFUNCTION(BlueprintCallable, Category = "Audio")
	void SetAmbienceVolume(float Volume);

protected:
	// Sound registry
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	TMap<FString, FAudio_SoundEntry> SoundRegistry;

	// Ambience layers
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	TMap<EBiomeType, FAudio_AmbienceLayer> AmbienceLayers;

	// Active audio components
	UPROPERTY()
	TArray<UAudioComponent*> ActiveAudioComponents;

	UPROPERTY()
	TMap<EBiomeType, UAudioComponent*> ActiveAmbienceComponents;

	// Volume settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	float MasterVolume = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	float SFXVolume = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	float MusicVolume = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	float AmbienceVolume = 1.0f;

	// Current state
	UPROPERTY(BlueprintReadOnly, Category = "Audio")
	EBiomeType CurrentBiome = EBiomeType::Savanna;

	UPROPERTY(BlueprintReadOnly, Category = "Audio")
	float CurrentTimeOfDay = 12.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Audio")
	EWeatherType CurrentWeather = EWeatherType::Clear;

private:
	void InitializeSoundRegistry();
	void InitializeAmbienceLayers();
	void CleanupInactiveComponents();
	UAudioComponent* CreateAudioComponent();
	float CalculateFinalVolume(float BaseVolume, float Multiplier) const;
};