#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Sound/AmbientSound.h"
#include "../SharedTypes.h"
#include "World_BiomeSystem.generated.h"

UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    DenseForest     UMETA(DisplayName = "Dense Forest"),
    RockyHighlands  UMETA(DisplayName = "Rocky Highlands"),
    Grasslands      UMETA(DisplayName = "Grasslands"),
    Wetlands        UMETA(DisplayName = "Wetlands"),
    BarrenWasteland UMETA(DisplayName = "Barren Wasteland")
};

UENUM(BlueprintType)
enum class EWorld_WeatherState : uint8
{
    ClearDay    UMETA(DisplayName = "Clear Day"),
    Overcast    UMETA(DisplayName = "Overcast"),
    Storm       UMETA(DisplayName = "Storm"),
    Dawn        UMETA(DisplayName = "Dawn"),
    Dusk        UMETA(DisplayName = "Dusk")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector Center;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float TreeDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FLinearColor GroundColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString AmbientSoundName;

    FWorld_BiomeData()
    {
        BiomeType = EWorld_BiomeType::DenseForest;
        Center = FVector::ZeroVector;
        Radius = 1000.0f;
        TreeDensity = 0.5f;
        GroundColor = FLinearColor::Green;
        AmbientSoundName = TEXT("Default_Ambience");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_WeatherData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    EWorld_WeatherState WeatherState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float SunIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    FLinearColor SkyColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float FogDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WindStrength;

    FWorld_WeatherData()
    {
        WeatherState = EWorld_WeatherState::ClearDay;
        SunIntensity = 8.0f;
        SkyColor = FLinearColor(0.4f, 0.7f, 1.0f, 1.0f);
        FogDensity = 0.02f;
        WindStrength = 0.3f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_BiomeSystem : public AActor
{
    GENERATED_BODY()

public:
    AWorld_BiomeSystem();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TArray<FWorld_BiomeData> BiomeDefinitions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    TArray<FWorld_WeatherData> WeatherStates;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    EWorld_WeatherState CurrentWeatherState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WeatherTransitionTime;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    TArray<AActor*> SpawnedVegetation;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    TArray<AActor*> SpawnedLandmarks;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    TArray<AActor*> AudioZones;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void InitializeBiomes();

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void SpawnBiomeVegetation(const FWorld_BiomeData& BiomeData);

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    EWorld_BiomeType GetBiomeAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Weather System")
    void SetWeatherState(EWorld_WeatherState NewWeatherState);

    UFUNCTION(BlueprintCallable, Category = "Weather System")
    void TransitionToWeather(EWorld_WeatherState TargetWeather, float TransitionDuration);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void CreateEnvironmentalAudio();

    UFUNCTION(BlueprintCallable, Category = "Landmarks")
    void SpawnNavigationLandmarks();

    UFUNCTION(BlueprintPure, Category = "Biome System")
    float GetDistanceToBiomeCenter(const FVector& Location, EWorld_BiomeType BiomeType) const;

    UFUNCTION(BlueprintPure, Category = "Weather System")
    FWorld_WeatherData GetCurrentWeatherData() const;

private:
    void UpdateLighting();
    void UpdateAtmosphere();
    void SpawnVegetationInRadius(const FVector& Center, float Radius, float Density, const TArray<FString>& VegetationTypes);
    AActor* SpawnActorAtLocation(UClass* ActorClass, const FVector& Location, const FRotator& Rotation, const FVector& Scale);
};