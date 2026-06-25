#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "SharedTypes.h"
#include "BiomeEnvironmentAudio.generated.h"

/**
 * BiomeEnvironmentAudio — Agent #5 World Generator
 * Manages ambient audio tied to biome type and weather conditions.
 * Placed in the world to trigger ambient sounds based on player proximity to biome zones.
 */
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Biome Environment Audio"))
class TRANSPERSONALGAME_API ABiomeEnvironmentAudio : public AActor
{
	GENERATED_BODY()

public:
	ABiomeEnvironmentAudio();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// ── Biome Configuration ──
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
	EWorld_BiomeType BiomeType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
	float AudioRadius = 5000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
	float FadeInTime = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
	float FadeOutTime = 5.0f;

	// ── Audio Components ──
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UAudioComponent* AmbientAudioComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UAudioComponent* WeatherAudioComponent;

	// ── Runtime State ──
	UPROPERTY(BlueprintReadOnly, Category = "Biome Audio")
	bool bPlayerInRange = false;

	UPROPERTY(BlueprintReadOnly, Category = "Biome Audio")
	float CurrentVolume = 0.0f;

	// ── Blueprint Events ──
	UFUNCTION(BlueprintNativeEvent, Category = "Biome Audio")
	void OnPlayerEnterBiome();

	UFUNCTION(BlueprintNativeEvent, Category = "Biome Audio")
	void OnPlayerExitBiome();

	UFUNCTION(BlueprintCallable, Category = "Biome Audio")
	void SetBiomeType(EWorld_BiomeType NewBiomeType);

	UFUNCTION(BlueprintCallable, Category = "Biome Audio")
	void UpdateWeatherAudio(EWorld_WeatherType WeatherType);

	UFUNCTION(BlueprintCallable, Category = "Biome Audio")
	float GetDistanceToPlayer() const;

	UFUNCTION(CallInEditor, Category = "Biome Audio")
	void PreviewAudio();

private:
	void UpdateAudioVolume(float DeltaTime);
	bool CheckPlayerInRange() const;

	UPROPERTY()
	APawn* CachedPlayerPawn;

	float TargetVolume = 0.0f;
};
