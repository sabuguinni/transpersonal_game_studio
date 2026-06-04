#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/PointLight.h"
#include "Engine/SpotLight.h"
#include "Engine/RectLight.h"
#include "Components/PointLightComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/RectLightComponent.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Light_CaveLightingSystem.generated.h"

UENUM(BlueprintType)
enum class ELight_CaveType : uint8
{
    ShallowCave     UMETA(DisplayName = "Shallow Cave"),
    DeepCavern      UMETA(DisplayName = "Deep Cavern"),
    TunnelSystem    UMETA(DisplayName = "Tunnel System"),
    UndergroundLake UMETA(DisplayName = "Underground Lake")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_CaveLightingConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Lighting")
    float EntranceLightIntensity = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Lighting")
    float FirePitIntensity = 1200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Lighting")
    float AmbientIntensity = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Lighting")
    FLinearColor EntranceColor = FLinearColor(1.0f, 0.78f, 0.59f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Lighting")
    FLinearColor FirePitColor = FLinearColor(1.0f, 0.47f, 0.24f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Lighting")
    FLinearColor AmbientColor = FLinearColor(0.71f, 0.71f, 0.86f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Lighting")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Lighting")
    float FogHeightFalloff = 0.2f;

    FLight_CaveLightingConfig()
    {
        EntranceLightIntensity = 800.0f;
        FirePitIntensity = 1200.0f;
        AmbientIntensity = 200.0f;
        EntranceColor = FLinearColor(1.0f, 0.78f, 0.59f, 1.0f);
        FirePitColor = FLinearColor(1.0f, 0.47f, 0.24f, 1.0f);
        AmbientColor = FLinearColor(0.71f, 0.71f, 0.86f, 1.0f);
        FogDensity = 0.02f;
        FogHeightFalloff = 0.2f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ULight_CaveLightingSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    ULight_CaveLightingSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Cave lighting configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Lighting")
    ELight_CaveType CaveType = ELight_CaveType::ShallowCave;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Lighting")
    FLight_CaveLightingConfig LightingConfig;

    // Light actor references
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cave Lighting")
    APointLight* EntranceLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cave Lighting")
    ASpotLight* FirePitLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cave Lighting")
    ARectLight* AmbientLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cave Lighting")
    AExponentialHeightFog* CaveFog;

    // Cave lighting setup functions
    UFUNCTION(BlueprintCallable, Category = "Cave Lighting")
    void SetupCaveLighting();

    UFUNCTION(BlueprintCallable, Category = "Cave Lighting")
    void CreateEntranceLighting(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Cave Lighting")
    void CreateFirePitLighting(const FVector& Location, const FRotator& Rotation);

    UFUNCTION(BlueprintCallable, Category = "Cave Lighting")
    void CreateAmbientLighting(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Cave Lighting")
    void SetupAtmosphericFog(const FVector& Location);

    // Dynamic lighting control
    UFUNCTION(BlueprintCallable, Category = "Cave Lighting")
    void SetFirePitActive(bool bActive);

    UFUNCTION(BlueprintCallable, Category = "Cave Lighting")
    void UpdateLightingForTimeOfDay(float TimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Cave Lighting")
    void SetCaveType(ELight_CaveType NewCaveType);

    // Lighting intensity modulation
    UFUNCTION(BlueprintCallable, Category = "Cave Lighting")
    void ModulateFirePitIntensity(float IntensityMultiplier);

    UFUNCTION(BlueprintCallable, Category = "Cave Lighting")
    void SetAmbientLightingIntensity(float NewIntensity);

    // Cave atmosphere effects
    UFUNCTION(BlueprintCallable, Category = "Cave Lighting")
    void UpdateFogDensity(float NewDensity);

    UFUNCTION(BlueprintCallable, Category = "Cave Lighting")
    void SetCaveMoodLighting(const FLinearColor& MoodColor, float Intensity);

protected:
    // Internal lighting management
    void ConfigureLightForCaveType();
    void UpdateLightPositions();
    void ValidateLightActors();

private:
    // Fire flicker simulation
    float FireFlickerTimer = 0.0f;
    float BaseFireIntensity = 1200.0f;
    bool bFireFlickerEnabled = true;

    // Cave depth calculation
    float CalculateCaveDepthFactor() const;
    void ApplyDepthBasedLighting(float DepthFactor);
};