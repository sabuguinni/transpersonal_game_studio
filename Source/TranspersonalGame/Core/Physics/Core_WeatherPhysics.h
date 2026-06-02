#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundCue.h"
#include "Core_WeatherPhysics.generated.h"

UENUM(BlueprintType)
enum class ECore_WeatherType : uint8
{
	Clear = 0,
	Cloudy = 1,
	Rain = 2,
	Storm = 3,
	Snow = 4,
	Blizzard = 5,
	Fog = 6,
	Sandstorm = 7,
	Hail = 8
};

UENUM(BlueprintType)
enum class ECore_WeatherIntensity : uint8
{
	None = 0,
	Light = 1,
	Moderate = 2,
	Heavy = 3,
	Extreme = 4
};

USTRUCT(BlueprintType)
struct FCore_WeatherEffects
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Effects")
	ECore_WeatherType WeatherType = ECore_WeatherType::Clear;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Effects")
	ECore_WeatherIntensity Intensity = ECore_WeatherIntensity::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Effects", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float WindForceMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Effects", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float VisibilityReduction = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Effects", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float MovementSpeedMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Effects", meta = (ClampMin = "0.0", ClampMax = "5.0"))
	float StaminaDrainMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Effects", meta = (ClampMin = "-50.0", ClampMax = "50.0"))
	float TemperatureEffect = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Effects")
	bool bAffectsProjectiles = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Effects")
	bool bCausesSlipping = false;

	FCore_WeatherEffects()
	{
		WeatherType = ECore_WeatherType::Clear;
		Intensity = ECore_WeatherIntensity::None;
		WindForceMultiplier = 1.0f;
		VisibilityReduction = 0.0f;
		MovementSpeedMultiplier = 1.0f;
		StaminaDrainMultiplier = 1.0f;
		TemperatureEffect = 0.0f;
		bAffectsProjectiles = false;
		bCausesSlipping = false;
	}
};

USTRUCT(BlueprintType)
struct FCore_WeatherTransition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Transition")
	ECore_WeatherType FromWeather = ECore_WeatherType::Clear;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Transition")
	ECore_WeatherType ToWeather = ECore_WeatherType::Clear;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Transition", meta = (ClampMin = "1.0", ClampMax = "300.0"))
	float TransitionDuration = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Transition", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float TransitionProgress = 0.0f;

	FCore_WeatherTransition()
	{
		FromWeather = ECore_WeatherType::Clear;
		ToWeather = ECore_WeatherType::Clear;
		TransitionDuration = 30.0f;
		TransitionProgress = 0.0f;
	}
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_WeatherPhysics : public UActorComponent
{
	GENERATED_BODY()

public:
	UCore_WeatherPhysics();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Weather Control
	UFUNCTION(BlueprintCallable, Category = "Weather Physics")
	void SetWeather(ECore_WeatherType NewWeather, ECore_WeatherIntensity NewIntensity);

	UFUNCTION(BlueprintCallable, Category = "Weather Physics")
	void TransitionToWeather(ECore_WeatherType TargetWeather, ECore_WeatherIntensity TargetIntensity, float Duration);

	UFUNCTION(BlueprintCallable, Category = "Weather Physics")
	ECore_WeatherType GetCurrentWeather() const { return CurrentWeatherEffects.WeatherType; }

	UFUNCTION(BlueprintCallable, Category = "Weather Physics")
	ECore_WeatherIntensity GetCurrentIntensity() const { return CurrentWeatherEffects.Intensity; }

	// Physics Effects
	UFUNCTION(BlueprintCallable, Category = "Weather Physics")
	void ApplyWeatherForces(class UPrimitiveComponent* Component, const FVector& Location);

	UFUNCTION(BlueprintCallable, Category = "Weather Physics")
	FVector GetWindForceAtLocation(const FVector& Location);

	UFUNCTION(BlueprintCallable, Category = "Weather Physics")
	float GetMovementSpeedMultiplier() const { return CurrentWeatherEffects.MovementSpeedMultiplier; }

	UFUNCTION(BlueprintCallable, Category = "Weather Physics")
	float GetVisibilityMultiplier() const { return 1.0f - CurrentWeatherEffects.VisibilityReduction; }

	// Environmental Effects
	UFUNCTION(BlueprintCallable, Category = "Weather Physics")
	bool ShouldApplySlippingEffect() const { return CurrentWeatherEffects.bCausesSlipping; }

	UFUNCTION(BlueprintCallable, Category = "Weather Physics")
	float GetTemperatureEffect() const { return CurrentWeatherEffects.TemperatureEffect; }

	UFUNCTION(BlueprintCallable, Category = "Weather Physics")
	float GetStaminaDrainMultiplier() const { return CurrentWeatherEffects.StaminaDrainMultiplier; }

	// Projectile Physics
	UFUNCTION(BlueprintCallable, Category = "Weather Physics")
	FVector ModifyProjectileVelocity(const FVector& OriginalVelocity, const FVector& ProjectileLocation);

	UFUNCTION(BlueprintCallable, Category = "Weather Physics")
	bool DoesWeatherAffectProjectiles() const { return CurrentWeatherEffects.bAffectsProjectiles; }

	// Weather Simulation
	UFUNCTION(BlueprintCallable, Category = "Weather Physics")
	void UpdateWeatherSimulation(float DeltaTime);

	UFUNCTION(BlueprintCallable, Category = "Weather Physics")
	void GenerateRandomWeatherEvent();

protected:
	// Current Weather State
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather State")
	FCore_WeatherEffects CurrentWeatherEffects;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather State")
	FCore_WeatherTransition ActiveTransition;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather State")
	bool bIsTransitioning = false;

	// Weather Configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Settings")
	TMap<ECore_WeatherType, FCore_WeatherEffects> WeatherTypeMap;

	// Wind Settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wind Settings")
	FVector WindDirection = FVector(1.0f, 0.0f, 0.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wind Settings", meta = (ClampMin = "0.0", ClampMax = "2000.0"))
	float BaseWindForce = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wind Settings", meta = (ClampMin = "0.0", ClampMax = "10.0"))
	float WindVariation = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wind Settings", meta = (ClampMin = "0.1", ClampMax = "5.0"))
	float WindChangeFrequency = 2.0f;

	// Simulation Settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation Settings", meta = (ClampMin = "60.0", ClampMax = "3600.0"))
	float WeatherChangeDuration = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation Settings", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float RandomWeatherChance = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation Settings")
	bool bEnableRandomWeather = true;

	// Internal State
	UPROPERTY()
	float LastWindUpdate = 0.0f;

	UPROPERTY()
	float WeatherTimer = 0.0f;

	UPROPERTY()
	FVector CurrentWindForce = FVector::ZeroVector;

private:
	// Internal Methods
	void InitializeWeatherEffects();
	void UpdateWindSimulation(float DeltaTime);
	void UpdateWeatherTransition(float DeltaTime);
	FCore_WeatherEffects InterpolateWeatherEffects(const FCore_WeatherEffects& From, const FCore_WeatherEffects& To, float Alpha);
	ECore_WeatherType GetRandomWeatherType();
	ECore_WeatherIntensity GetRandomIntensity();
};