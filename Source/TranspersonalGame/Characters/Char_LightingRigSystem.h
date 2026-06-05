#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/SpotLight.h"
#include "Engine/DirectionalLight.h"
#include "Components/SpotLightComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Engine/World.h"
#include "Char_LightingRigSystem.generated.h"

USTRUCT(BlueprintType)
struct FChar_LightingRigConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Lighting")
    float RimLightIntensity = 1200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Lighting")
    FLinearColor RimLightColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Lighting")
    float RimLightDistance = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Lighting")
    float RimLightHeight = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Lighting")
    float InnerConeAngle = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Lighting")
    float OuterConeAngle = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Lighting")
    float AttenuationRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Lighting")
    bool bCastShadows = false;

    FChar_LightingRigConfig()
    {
        RimLightIntensity = 1200.0f;
        RimLightColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
        RimLightDistance = 200.0f;
        RimLightHeight = 300.0f;
        InnerConeAngle = 25.0f;
        OuterConeAngle = 45.0f;
        AttenuationRadius = 800.0f;
        bCastShadows = false;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UChar_LightingRigSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UChar_LightingRigSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // === LIGHTING RIG MANAGEMENT ===
    UFUNCTION(BlueprintCallable, Category = "Character Lighting")
    void CreateCharacterLightingRig();

    UFUNCTION(BlueprintCallable, Category = "Character Lighting")
    void DestroyCharacterLightingRig();

    UFUNCTION(BlueprintCallable, Category = "Character Lighting")
    void UpdateLightingRigPosition();

    UFUNCTION(BlueprintCallable, Category = "Character Lighting")
    void ConfigureLightingForAtmosphere(bool bCretaceousLighting = true);

    // === ATMOSPHERIC INTEGRATION ===
    UFUNCTION(BlueprintCallable, Category = "Character Lighting")
    void OptimizeForCaveEnvironment();

    UFUNCTION(BlueprintCallable, Category = "Character Lighting")
    void OptimizeForOutdoorEnvironment();

    UFUNCTION(BlueprintCallable, Category = "Character Lighting")
    void SetTimeOfDayLighting(float TimeOfDay);

    // === MATERIAL OPTIMIZATION ===
    UFUNCTION(BlueprintCallable, Category = "Character Lighting")
    void OptimizeCharacterMaterials();

    UFUNCTION(BlueprintCallable, Category = "Character Lighting")
    void SetMaterialLightingParameters(float Roughness, float Metallic, float Specular);

protected:
    // === CONFIGURATION ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Lighting")
    FChar_LightingRigConfig LightingConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Lighting")
    bool bAutoUpdatePosition = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Lighting")
    bool bOptimizeForAtmosphere = true;

    // === LIGHTING ACTORS ===
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character Lighting")
    TObjectPtr<ASpotLight> RimLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character Lighting")
    TObjectPtr<ASpotLight> FillLight;

    // === INTERNAL STATE ===
    UPROPERTY()
    FVector LastOwnerLocation;

    UPROPERTY()
    float LastUpdateTime;

private:
    void CreateRimLight();
    void CreateFillLight();
    void UpdateLightPositions();
    bool ValidateOwnerActor() const;
};