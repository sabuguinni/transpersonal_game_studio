#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/AudioComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "EnvironmentalPropManager.generated.h"

UENUM(BlueprintType)
enum class EEnvArt_PropType : uint8
{
    FallenLog       UMETA(DisplayName = "Fallen Log"),
    WeatheredRock   UMETA(DisplayName = "Weathered Rock"),
    AncientStone    UMETA(DisplayName = "Ancient Stone"),
    DeadTree        UMETA(DisplayName = "Dead Tree"),
    BoulderCluster  UMETA(DisplayName = "Boulder Cluster"),
    CrystalFormation UMETA(DisplayName = "Crystal Formation")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_PropSpawnData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prop Data")
    EEnvArt_PropType PropType = EEnvArt_PropType::FallenLog;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prop Data")
    FVector SpawnLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prop Data")
    FRotator SpawnRotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prop Data")
    FVector Scale = FVector::OneVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prop Data")
    float WeatheringLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prop Data")
    bool bHasMoss = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prop Data")
    bool bHasAmbientSound = false;

    FEnvArt_PropSpawnData()
    {
        PropType = EEnvArt_PropType::FallenLog;
        SpawnLocation = FVector::ZeroVector;
        SpawnRotation = FRotator::ZeroRotator;
        Scale = FVector::OneVector;
        WeatheringLevel = 0.5f;
        bHasMoss = true;
        bHasAmbientSound = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_AtmosphericSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLinearColor FogColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    bool bVolumetricFog = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float SunIntensity = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLinearColor SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FRotator SunRotation = FRotator(-30.0f, 45.0f, 0.0f);

    FEnvArt_AtmosphericSettings()
    {
        FogDensity = 0.02f;
        FogHeightFalloff = 0.2f;
        FogColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);
        bVolumetricFog = true;
        SunIntensity = 8.0f;
        SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
        SunRotation = FRotator(-30.0f, 45.0f, 0.0f);
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEnvironmentalPropManager : public AActor
{
    GENERATED_BODY()

public:
    AEnvironmentalPropManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // === PROP MANAGEMENT ===
    UFUNCTION(BlueprintCallable, Category = "Environmental Props")
    void SpawnEnvironmentalProp(const FEnvArt_PropSpawnData& PropData);

    UFUNCTION(BlueprintCallable, Category = "Environmental Props")
    void SpawnPropsInRadius(FVector CenterLocation, float Radius, int32 PropCount, EEnvArt_PropType PropType);

    UFUNCTION(BlueprintCallable, Category = "Environmental Props")
    void ClearAllEnvironmentalProps();

    UFUNCTION(BlueprintCallable, Category = "Environmental Props")
    TArray<AActor*> GetAllEnvironmentalProps() const;

    // === ATMOSPHERIC CONTROL ===
    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void ApplyAtmosphericSettings(const FEnvArt_AtmosphericSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetGoldenHourLighting();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetMidDayLighting();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetDuskLighting();

    // === ENVIRONMENTAL STORYTELLING ===
    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    void CreateAbandonedCampsite(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    void CreateDinosaurNest(FVector Location, float NestRadius);

    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    void CreateAncientRuins(FVector Location, FVector Extent);

protected:
    // === COMPONENTS ===
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    // === PROP ARRAYS ===
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Props")
    TArray<AActor*> SpawnedProps;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Props")
    TArray<UStaticMeshComponent*> PropMeshComponents;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Props")
    TArray<UAudioComponent*> AmbientAudioComponents;

    // === ATMOSPHERIC SETTINGS ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FEnvArt_AtmosphericSettings CurrentAtmosphere;

    // === PROP CONFIGURATION ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Props")
    TArray<FEnvArt_PropSpawnData> PropSpawnQueue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Props")
    float PropDensityMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Props")
    bool bAutoSpawnProps = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Props")
    float MinDistanceBetweenProps = 500.0f;

private:
    // === INTERNAL METHODS ===
    void InitializeDefaultSettings();
    void SetupAtmosphericFog();
    void ConfigureSunLighting();
    AActor* SpawnPropActor(const FEnvArt_PropSpawnData& PropData);
    void ApplyWeatheringToMesh(UStaticMeshComponent* MeshComp, float WeatheringLevel);
    void AddMossToMesh(UStaticMeshComponent* MeshComp);
    void CreateAmbientAudio(FVector Location, const FString& SoundName);

    // === CACHED REFERENCES ===
    UPROPERTY()
    class ADirectionalLight* SunLight;

    UPROPERTY()
    class AExponentialHeightFog* AtmosphericFog;

    UPROPERTY()
    TArray<class UParticleSystemComponent*> AtmosphericParticles;
};