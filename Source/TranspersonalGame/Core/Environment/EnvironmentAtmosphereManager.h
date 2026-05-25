#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/SkyLight.h"
#include "TranspersonalGame/TranspersonalGame.h"
#include "EnvironmentAtmosphereManager.generated.h"

UENUM(BlueprintType)
enum class EEnvArt_BiomeAtmosphere : uint8
{
	Savana		UMETA(DisplayName = "Savana"),
	Floresta	UMETA(DisplayName = "Floresta"),
	Pantano		UMETA(DisplayName = "Pantano"),
	Deserto		UMETA(DisplayName = "Deserto"),
	Montanha	UMETA(DisplayName = "Montanha")
};

UENUM(BlueprintType)
enum class EEnvArt_TimeOfDay : uint8
{
	Dawn		UMETA(DisplayName = "Dawn"),
	Morning		UMETA(DisplayName = "Morning"),
	Noon		UMETA(DisplayName = "Noon"),
	Afternoon	UMETA(DisplayName = "Afternoon"),
	Dusk		UMETA(DisplayName = "Dusk"),
	Night		UMETA(DisplayName = "Night")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_AtmosphereSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
	float SunIntensity = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
	FLinearColor SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
	FRotator SunRotation = FRotator(-30.0f, 45.0f, 0.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
	float FogDensity = 0.02f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
	float FogHeightFalloff = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
	FLinearColor FogInscatteringColor = FLinearColor(0.4f, 0.6f, 0.8f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
	bool bVolumetricFog = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
	float VolumetricFogScatteringDistribution = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
	float SkyLightIntensity = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
	FLinearColor SkyLightColor = FLinearColor(0.5f, 0.7f, 1.0f, 1.0f);
};

UCLASS(Blueprintable, BlueprintType)
class TRANSPERSONALGAME_API AEnvironmentAtmosphereManager : public AActor
{
	GENERATED_BODY()

public:
	AEnvironmentAtmosphereManager();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	// Core atmosphere management
	UFUNCTION(BlueprintCallable, Category = "Environment")
	void SetBiomeAtmosphere(EEnvArt_BiomeAtmosphere BiomeType);

	UFUNCTION(BlueprintCallable, Category = "Environment")
	void SetTimeOfDay(EEnvArt_TimeOfDay TimeOfDay);

	UFUNCTION(BlueprintCallable, Category = "Environment")
	void SetGoldenHourLighting();

	UFUNCTION(BlueprintCallable, Category = "Environment")
	void UpdateSunPosition(float TimeOfDayNormalized);

	UFUNCTION(BlueprintCallable, Category = "Environment")
	void SetupVolumetricFog(FVector Location, const FEnvArt_AtmosphereSettings& Settings);

	// Lighting controls
	UFUNCTION(BlueprintCallable, Category = "Lighting")
	void SetSunIntensity(float Intensity);

	UFUNCTION(BlueprintCallable, Category = "Lighting")
	void SetSunColor(FLinearColor Color);

	UFUNCTION(BlueprintCallable, Category = "Lighting")
	void SetSunRotation(FRotator Rotation);

	// Fog controls
	UFUNCTION(BlueprintCallable, Category = "Fog")
	void SetFogDensity(float Density);

	UFUNCTION(BlueprintCallable, Category = "Fog")
	void SetFogColor(FLinearColor Color);

	UFUNCTION(BlueprintCallable, Category = "Fog")
	void EnableVolumetricFog(bool bEnable);

	// Biome-specific presets
	UFUNCTION(BlueprintCallable, Category = "Biomes")
	FEnvArt_AtmosphereSettings GetSavanaSettings() const;

	UFUNCTION(BlueprintCallable, Category = "Biomes")
	FEnvArt_AtmosphereSettings GetFlorestaSettings() const;

	UFUNCTION(BlueprintCallable, Category = "Biomes")
	FEnvArt_AtmosphereSettings GetPantanoSettings() const;

	UFUNCTION(BlueprintCallable, Category = "Biomes")
	FEnvArt_AtmosphereSettings GetDesertoSettings() const;

	UFUNCTION(BlueprintCallable, Category = "Biomes")
	FEnvArt_AtmosphereSettings GetMontanhaSettings() const;

protected:
	// Component references
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USceneComponent* RootSceneComponent;

	// Current atmosphere settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	FEnvArt_AtmosphereSettings CurrentAtmosphereSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	EEnvArt_BiomeAtmosphere CurrentBiome = EEnvArt_BiomeAtmosphere::Savana;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	EEnvArt_TimeOfDay CurrentTimeOfDay = EEnvArt_TimeOfDay::Afternoon;

	// Auto-find lighting actors
	UPROPERTY(BlueprintReadOnly, Category = "Lighting")
	ADirectionalLight* SunLight;

	UPROPERTY(BlueprintReadOnly, Category = "Lighting")
	AExponentialHeightFog* HeightFog;

	UPROPERTY(BlueprintReadOnly, Category = "Lighting")
	ASkyLight* SkyLightActor;

	// Time progression
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
	bool bAutoProgressTime = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
	float TimeProgressionSpeed = 1.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Time")
	float CurrentTimeOfDayNormalized = 0.5f; // 0.5 = noon

private:
	void FindLightingActors();
	void ApplyAtmosphereSettings(const FEnvArt_AtmosphereSettings& Settings);
	FRotator CalculateSunRotationFromTime(float TimeNormalized) const;
	FLinearColor CalculateSunColorFromTime(float TimeNormalized) const;
	float CalculateSunIntensityFromTime(float TimeNormalized) const;
};