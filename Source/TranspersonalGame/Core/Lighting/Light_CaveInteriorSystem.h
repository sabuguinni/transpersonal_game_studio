#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/PointLight.h"
#include "Engine/SpotLight.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "Light_CaveInteriorSystem.generated.h"

UENUM(BlueprintType)
enum class ELight_CaveType : uint8
{
    Limestone       UMETA(DisplayName = "Limestone Cave"),
    Volcanic        UMETA(DisplayName = "Volcanic Cave"),
    Sandstone       UMETA(DisplayName = "Sandstone Cave"),
    Granite         UMETA(DisplayName = "Granite Cave"),
    AncientRuins    UMETA(DisplayName = "Ancient Ruins")
};

UENUM(BlueprintType)
enum class ELight_LightingZone : uint8
{
    Entrance        UMETA(DisplayName = "Cave Entrance"),
    FirePit         UMETA(DisplayName = "Fire Pit Area"),
    DeepChamber     UMETA(DisplayName = "Deep Chamber"),
    RimLighting     UMETA(DisplayName = "Wall Rim Lighting"),
    Mystery         UMETA(DisplayName = "Mystery Zone")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_CaveLightingConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Lighting")
    float FirePitIntensity = 2200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Lighting")
    FLinearColor FirePitColor = FLinearColor(1.0f, 0.4f, 0.1f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Lighting")
    float EntranceLightIntensity = 1800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Lighting")
    FLinearColor EntranceLightColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Lighting")
    float RimLightIntensity = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Lighting")
    FLinearColor RimLightColor = FLinearColor(0.9f, 0.6f, 0.3f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Lighting")
    float MysteryLightIntensity = 1200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Lighting")
    FLinearColor MysteryLightColor = FLinearColor(0.3f, 0.7f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Lighting")
    float TemperatureVariation = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Lighting")
    bool bEnableVolumetricFog = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Lighting")
    float FogDensity = 0.08f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Lighting")
    FLinearColor FogColor = FLinearColor(0.6f, 0.4f, 0.2f, 1.0f);

    FLight_CaveLightingConfig()
    {
        // Default constructor with preset values
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_TemperatureZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Temperature")
    ELight_CaveType CaveType = ELight_CaveType::Limestone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Temperature")
    float Temperature = 3500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Temperature")
    FLinearColor TemperatureColor = FLinearColor(0.8f, 0.8f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Temperature")
    float IntensityMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Temperature")
    float AttenuationRadius = 500.0f;

    FLight_TemperatureZone()
    {
        // Default constructor
    }
};

/**
 * Comprehensive cave interior lighting system for prehistoric environments
 * Handles dynamic atmospheric lighting, temperature-based zones, and environmental storytelling
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ULight_CaveInteriorSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    ULight_CaveInteriorSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // === CORE LIGHTING CONFIGURATION ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Lighting", meta = (AllowPrivateAccess = "true"))
    FLight_CaveLightingConfig LightingConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Type", meta = (AllowPrivateAccess = "true"))
    ELight_CaveType CaveType = ELight_CaveType::Limestone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Temperature Zones", meta = (AllowPrivateAccess = "true"))
    TArray<FLight_TemperatureZone> TemperatureZones;

    // === LIGHTING ACTORS ===
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting Actors", meta = (AllowPrivateAccess = "true"))
    class APointLight* FirePitLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting Actors", meta = (AllowPrivateAccess = "true"))
    class ASpotLight* EntranceLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting Actors", meta = (AllowPrivateAccess = "true"))
    TArray<class APointLight*> RimLights;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting Actors", meta = (AllowPrivateAccess = "true"))
    class APointLight* MysteryLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting Actors", meta = (AllowPrivateAccess = "true"))
    TArray<class APointLight*> TemperatureLights;

    // === ATMOSPHERIC EFFECTS ===
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Atmospheric", meta = (AllowPrivateAccess = "true"))
    class AExponentialHeightFog* CaveFog;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Effects", meta = (AllowPrivateAccess = "true"))
    bool bEnableFireFlicker = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Effects", meta = (AllowPrivateAccess = "true"))
    float FireFlickerSpeed = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Effects", meta = (AllowPrivateAccess = "true"))
    float FireFlickerIntensity = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night", meta = (AllowPrivateAccess = "true"))
    bool bAdaptToTimeOfDay = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night", meta = (AllowPrivateAccess = "true"))
    float DayNightTransitionSpeed = 1.0f;

    // === CORE FUNCTIONS ===
    UFUNCTION(BlueprintCallable, Category = "Cave Lighting")
    void InitializeCaveLighting();

    UFUNCTION(BlueprintCallable, Category = "Cave Lighting")
    void SetupFirePitLighting(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Cave Lighting")
    void SetupEntranceLighting(const FVector& Location, const FRotator& Rotation);

    UFUNCTION(BlueprintCallable, Category = "Cave Lighting")
    void SetupRimLighting(const TArray<FVector>& Locations);

    UFUNCTION(BlueprintCallable, Category = "Cave Lighting")
    void SetupMysteryLighting(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Cave Lighting")
    void SetupTemperatureZones(const TArray<FVector>& Locations);

    UFUNCTION(BlueprintCallable, Category = "Cave Lighting")
    void SetupAtmosphericFog(const FVector& Location);

    // === DYNAMIC LIGHTING FUNCTIONS ===
    UFUNCTION(BlueprintCallable, Category = "Dynamic Lighting")
    void UpdateFireFlicker(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Dynamic Lighting")
    void AdaptToTimeOfDay(float TimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Dynamic Lighting")
    void SetCaveType(ELight_CaveType NewCaveType);

    UFUNCTION(BlueprintCallable, Category = "Dynamic Lighting")
    void UpdateTemperatureLighting();

    // === UTILITY FUNCTIONS ===
    UFUNCTION(BlueprintCallable, Category = "Cave Lighting")
    FLinearColor GetCaveTypeColor(ELight_CaveType Type) const;

    UFUNCTION(BlueprintCallable, Category = "Cave Lighting")
    float GetCaveTypeTemperature(ELight_CaveType Type) const;

    UFUNCTION(BlueprintCallable, Category = "Cave Lighting")
    void CleanupLightingActors();

    UFUNCTION(BlueprintCallable, Category = "Cave Lighting")
    bool ValidateLightingSetup() const;

private:
    // === INTERNAL STATE ===
    float CurrentFireFlickerTime = 0.0f;
    float BaseFireIntensity = 2200.0f;
    bool bLightingInitialized = false;

    // === HELPER FUNCTIONS ===
    void CreatePointLight(APointLight*& LightActor, const FVector& Location, const FString& Label);
    void CreateSpotLight(ASpotLight*& LightActor, const FVector& Location, const FRotator& Rotation, const FString& Label);
    void ApplyCaveTypeSettings();
    void SetupDefaultTemperatureZones();
};