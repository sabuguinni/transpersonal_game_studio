#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/PointLight.h"
#include "Engine/SpotLight.h"
#include "Engine/RectLight.h"
#include "Components/PointLightComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/RectLightComponent.h"
#include "Light_CaveAtmosphereManager.generated.h"

UENUM(BlueprintType)
enum class ELight_CaveType : uint8
{
    FirePit         UMETA(DisplayName = "Fire Pit"),
    Entrance        UMETA(DisplayName = "Cave Entrance"),
    DeepCave        UMETA(DisplayName = "Deep Cave"),
    TorchLit        UMETA(DisplayName = "Torch Lit"),
    Crystal         UMETA(DisplayName = "Crystal Glow")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_CaveConfiguration
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Lighting")
    ELight_CaveType CaveType = ELight_CaveType::FirePit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Lighting")
    float FireIntensity = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Lighting")
    FLinearColor FireColor = FLinearColor(1.0f, 0.4f, 0.1f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Lighting")
    float EntranceIntensity = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Lighting")
    FLinearColor EntranceColor = FLinearColor(0.7f, 0.8f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Lighting")
    float AmbientIntensity = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Lighting")
    float FireTemperature = 2700.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Lighting")
    float EntranceTemperature = 5600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Lighting")
    bool bCastShadows = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Lighting")
    float AttenuationRadius = 800.0f;

    FLight_CaveConfiguration()
    {
        CaveType = ELight_CaveType::FirePit;
        FireIntensity = 2000.0f;
        FireColor = FLinearColor(1.0f, 0.4f, 0.1f, 1.0f);
        EntranceIntensity = 1500.0f;
        EntranceColor = FLinearColor(0.7f, 0.8f, 1.0f, 1.0f);
        AmbientIntensity = 300.0f;
        FireTemperature = 2700.0f;
        EntranceTemperature = 5600.0f;
        bCastShadows = true;
        AttenuationRadius = 800.0f;
    }
};

UCLASS(BlueprintType, Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ULight_CaveAtmosphereManager : public UActorComponent
{
    GENERATED_BODY()

public:
    ULight_CaveAtmosphereManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Cave lighting configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Atmosphere")
    FLight_CaveConfiguration CaveConfig;

    // Biome-specific lighting adjustments
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Atmosphere")
    TMap<FString, float> BiomeIntensityMultipliers;

    // Dynamic lighting effects
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Atmosphere")
    bool bEnableFireFlicker = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Atmosphere")
    float FlickerSpeed = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Atmosphere")
    float FlickerIntensity = 0.3f;

    // Light references
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cave Lights")
    TArray<APointLight*> FireLights;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cave Lights")
    TArray<ASpotLight*> EntranceLights;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cave Lights")
    TArray<ARectLight*> AmbientLights;

    // Cave atmosphere management functions
    UFUNCTION(BlueprintCallable, Category = "Cave Atmosphere")
    void SetupCaveLighting(const FVector& CaveLocation, const FString& BiomeName);

    UFUNCTION(BlueprintCallable, Category = "Cave Atmosphere")
    void CreateFirePitLight(const FVector& Location, const FString& LabelSuffix);

    UFUNCTION(BlueprintCallable, Category = "Cave Atmosphere")
    void CreateEntranceLight(const FVector& Location, const FString& LabelSuffix);

    UFUNCTION(BlueprintCallable, Category = "Cave Atmosphere")
    void CreateAmbientLight(const FVector& Location, const FString& LabelSuffix);

    UFUNCTION(BlueprintCallable, Category = "Cave Atmosphere")
    void UpdateFireFlicker(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Cave Atmosphere")
    void SetBiomeIntensityMultiplier(const FString& BiomeName, float Multiplier);

    UFUNCTION(BlueprintCallable, Category = "Cave Atmosphere")
    float GetBiomeIntensityMultiplier(const FString& BiomeName) const;

    // Editor testing functions
    UFUNCTION(CallInEditor, Category = "Cave Atmosphere")
    void TestCaveLightingSetup();

    UFUNCTION(CallInEditor, Category = "Cave Atmosphere")
    void ClearAllCaveLights();

    UFUNCTION(CallInEditor, Category = "Cave Atmosphere")
    void ValidateCaveLightConfiguration();

private:
    // Internal state
    float FlickerTimer = 0.0f;
    float BaseFireIntensity = 2000.0f;

    // Helper functions
    void InitializeBiomeMultipliers();
    void ApplyBiomeLightingAdjustments(const FString& BiomeName, ULightComponent* LightComponent);
    FLinearColor GetBiomeAdjustedColor(const FString& BiomeName, const FLinearColor& BaseColor) const;
};