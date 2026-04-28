#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialParameterCollection.h"
#include "Materials/MaterialParameterCollectionInstance.h"
// FIXME: Missing header - #include "Lighting/LumenIntegratedLightingSystem.h"
#include "MetaHumanLightingIntegration.generated.h"

class UMaterialParameterCollection;
class UMaterialParameterCollectionInstance;
class ALumenIntegratedLightingSystem;

USTRUCT(BlueprintType)
struct FCharacterLightingProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SkinSubsurfaceScattering = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SkinRoughness = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SkinSpecular = 0.4f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SkinBaseColor = FLinearColor(0.8f, 0.6f, 0.5f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float EyeWetness = 0.9f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float HairRoughness = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float ClothingRoughness = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    bool bUseDynamicLighting = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    bool bReceiveLumenGI = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    bool bCastDynamicShadows = true;
};

USTRUCT(BlueprintType)
struct FCharacterAtmosphericResponse
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float WeatherWetnessResponse = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float TemperatureResponse = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float WindResponse = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLinearColor FogTintInfluence = FLinearColor(0.1f, 0.1f, 0.1f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    bool bAffectedByTimeOfDay = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    bool bAffectedByWeather = true;
};

/**
 * Component that integrates MetaHuman characters with the Lumen lighting system
 * and atmospheric conditions for realistic prehistoric world lighting
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UMetaHumanLightingIntegration : public UActorComponent
{
    GENERATED_BODY()

public:
    UMetaHumanLightingIntegration();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core lighting integration
    UFUNCTION(BlueprintCallable, Category = "Character Lighting")
    void InitializeLightingIntegration();

    UFUNCTION(BlueprintCallable, Category = "Character Lighting")
    void UpdateCharacterLighting(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Character Lighting")
    void SetLightingProfile(const FCharacterLightingProfile& NewProfile);

    UFUNCTION(BlueprintCallable, Category = "Character Lighting")
    void SetAtmosphericResponse(const FCharacterAtmosphericResponse& NewResponse);

    // Dynamic lighting adaptation
    UFUNCTION(BlueprintCallable, Category = "Character Lighting")
    void AdaptToEnvironmentalLighting();

    UFUNCTION(BlueprintCallable, Category = "Character Lighting")
    void UpdateSkinMaterials();

    UFUNCTION(BlueprintCallable, Category = "Character Lighting")
    void UpdateHairMaterials();

    UFUNCTION(BlueprintCallable, Category = "Character Lighting")
    void UpdateEyeMaterials();

    UFUNCTION(BlueprintCallable, Category = "Character Lighting")
    void UpdateClothingMaterials();

    // Atmospheric integration
    UFUNCTION(BlueprintCallable, Category = "Character Atmosphere")
    void RespondToWeatherChange(float WeatherIntensity, const FString& WeatherType);

    UFUNCTION(BlueprintCallable, Category = "Character Atmosphere")
    void RespondToTimeOfDay(float TimeOfDay, float SunIntensity);

    UFUNCTION(BlueprintCallable, Category = "Character Atmosphere")
    void ApplyWetnessEffect(float WetnessLevel);

    UFUNCTION(BlueprintCallable, Category = "Character Atmosphere")
    void ApplyTemperatureEffect(float Temperature);

    // Material parameter management
    UFUNCTION(BlueprintCallable, Category = "Character Materials")
    void SetMaterialParameter(const FString& ParameterName, float Value);

    UFUNCTION(BlueprintCallable, Category = "Character Materials")
    void SetMaterialVectorParameter(const FString& ParameterName, const FLinearColor& Value);

    UFUNCTION(BlueprintCallable, Category = "Character Materials")
    void UpdateAllMaterialParameters();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FCharacterLightingProfile LightingProfile;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FCharacterAtmosphericResponse AtmosphericResponse;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    TObjectPtr<USkeletalMeshComponent> CharacterMesh;

// [UHT-FIX]     UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    TObjectPtr<ALumenIntegratedLightingSystem> LightingSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TObjectPtr<UMaterialParameterCollection> CharacterMaterialCollection;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TArray<TObjectPtr<UMaterialInstanceDynamic>> DynamicMaterials;

    // Cached lighting values
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    float CurrentSunIntensity = 1.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    float CurrentWeatherIntensity = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    float CurrentWetnessLevel = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    float CurrentTemperature = 25.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    FLinearColor CurrentAmbientColor = FLinearColor::White;

private:
    void FindCharacterMesh();
    void FindLightingSystem();
    void CreateDynamicMaterials();
    void UpdateLightingParameters();
    void UpdateAtmosphericParameters();
    
    float InterpolateLightingValue(float Current, float Target, float Speed, float DeltaTime);
    FLinearColor InterpolateLightingColor(const FLinearColor& Current, const FLinearColor& Target, float Speed, float DeltaTime);
};