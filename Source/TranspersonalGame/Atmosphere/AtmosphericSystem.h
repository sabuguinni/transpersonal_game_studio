// AtmosphericSystem.h
// Sistema atmosférico para ambientes transcendentais
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Engine/PostProcessVolume.h"
#include "AtmosphericSystem.generated.h"

UENUM(BlueprintType)
enum class EAtmosphericMood : uint8
{
    Earthly         UMETA(DisplayName = "Earthly"),
    Ethereal        UMETA(DisplayName = "Ethereal"),
    Celestial       UMETA(DisplayName = "Celestial"),
    Cosmic          UMETA(DisplayName = "Cosmic"),
    Void            UMETA(DisplayName = "Void"),
    Unity           UMETA(DisplayName = "Unity")
};

UENUM(BlueprintType)
enum class EWeatherPattern : uint8
{
    Clear           UMETA(DisplayName = "Clear"),
    Misty           UMETA(DisplayName = "Misty"),
    Stormy          UMETA(DisplayName = "Stormy"),
    Aurora          UMETA(DisplayName = "Aurora"),
    EnergyStorm     UMETA(DisplayName = "Energy Storm"),
    Transcendent    UMETA(DisplayName = "Transcendent")
};

USTRUCT(BlueprintType)
struct FAtmosphericConfiguration
{
    GENERATED_BODY()

    // Configurações de neblina
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogInscatteringColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogMaxOpacity = 1.0f;

    // Configurações de atmosfera
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
    FLinearColor SkyLuminanceFactor = FLinearColor(1.0f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
    float AtmosphereHeight = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
    float RayleighScattering = 0.0331f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
    FLinearColor RayleighScatteringScale = FLinearColor(0.33f, 0.55f, 1.0f);

    // Configurações de nuvens
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clouds")
    float CloudCoverage = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clouds")
    float CloudOpacity = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clouds")
    FLinearColor CloudAlbedo = FLinearColor::White;

    // Configurações de pós-processamento
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Post Process")
    float Bloom = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Post Process")
    float ColorGrading = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Post Process")
    FLinearColor ColorFilter = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Post Process")
    float Saturation = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Post Process")
    float Contrast = 1.0f;
};

UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAtmosphericSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UAtmosphericSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Configurações atmosféricas por humor
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Moods")
    TMap<EAtmosphericMood, FAtmosphericConfiguration> AtmosphericMoods;

    // Estado atual
    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    EAtmosphericMood CurrentMood = EAtmosphericMood::Earthly;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    EWeatherPattern CurrentWeather = EWeatherPattern::Clear;

    // Componentes gerenciados
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    AExponentialHeightFog* HeightFog;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    APostProcessVolume* PostProcessVolume;

    // Configurações de transição
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transitions")
    float TransitionDuration = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transitions")
    UCurveFloat* TransitionCurve;

    // Variáveis de controle
    bool bIsTransitioning = false;
    float TransitionProgress = 0.0f;
    EAtmosphericMood PreviousMood;
    EAtmosphericMood TargetMood;
    float WeatherAnimationTime = 0.0f;

    // Efeitos especiais
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Special Effects")
    TArray<AActor*> AuroraEffects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Special Effects")
    TArray<AActor*> EnergyStormEffects;

public:
    // Funções principais
    UFUNCTION(BlueprintCallable, Category = "Atmospheric System")
    void SetAtmosphericMood(EAtmosphericMood NewMood);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric System")
    void SetWeatherPattern(EWeatherPattern NewWeather);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric System")
    void TriggerAtmosphericEvent(const FString& EventName, float Duration = 10.0f);

    // Efeitos especiais
    UFUNCTION(BlueprintCallable, Category = "Special Effects")
    void CreateAuroraEffect(FVector Location, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Special Effects")
    void CreateEnergyStorm(FVector CenterLocation, float Radius = 1000.0f);

    UFUNCTION(BlueprintCallable, Category = "Special Effects")
    void CreateMysticMist(FVector Location, float Radius = 500.0f, FLinearColor MistColor = FLinearColor::White);

    UFUNCTION(BlueprintCallable, Category = "Special Effects")
    void CreateCelestialBeams(int32 NumBeams = 5, float Duration = 15.0f);

    // Configurações avançadas
    UFUNCTION(BlueprintCallable, Category = "Advanced Settings")
    void SetGlobalAtmosphericIntensity(float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Advanced Settings")
    void SetTimeOfDayAtmosphere(float TimeOfDay); // 0.0 = meia-noite, 0.5 = meio-dia

    UFUNCTION(BlueprintCallable, Category = "Advanced Settings")
    void BlendAtmosphericMoods(EAtmosphericMood MoodA, EAtmosphericMood MoodB, float BlendFactor);

private:
    // Funções internas
    void InitializeAtmosphericConfigurations();
    void FindAtmosphericComponents();
    void ApplyAtmosphericConfiguration(const FAtmosphericConfiguration& Config, float Alpha = 1.0f);
    void UpdateTransition(float DeltaTime);
    void UpdateWeatherEffects(float DeltaTime);
    void InterpolateConfigurations(const FAtmosphericConfiguration& ConfigA, const FAtmosphericConfiguration& ConfigB, float Alpha, FAtmosphericConfiguration& OutConfig);

    // Multiplicadores globais
    float GlobalAtmosphericIntensity = 1.0f;
    float CurrentTimeOfDay = 0.5f; // Meio-dia por padrão
};