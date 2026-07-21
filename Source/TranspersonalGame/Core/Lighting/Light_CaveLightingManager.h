#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/PointLightComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Light_CaveLightingManager.generated.h"

UENUM(BlueprintType)
enum class ELight_CaveLightingMode : uint8
{
    DeepCave     UMETA(DisplayName = "Deep Cave"),
    CaveEntrance UMETA(DisplayName = "Cave Entrance"),
    TorchLit     UMETA(DisplayName = "Torch Lit"),
    Campfire     UMETA(DisplayName = "Campfire")
};

USTRUCT(BlueprintType)
struct FLight_CaveLightingPreset
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Lighting")
    FLinearColor FireColor = FLinearColor(1.0f, 0.4f, 0.1f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Lighting")
    float FireIntensity = 1200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Lighting")
    float TorchIntensity = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Lighting")
    FLinearColor FogColor = FLinearColor(0.6f, 0.4f, 0.2f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Lighting")
    float FogDensity = 0.08f;

    FLight_CaveLightingPreset()
    {
        FireColor = FLinearColor(1.0f, 0.4f, 0.1f, 1.0f);
        FireIntensity = 1200.0f;
        TorchIntensity = 400.0f;
        FogColor = FLinearColor(0.6f, 0.4f, 0.2f, 1.0f);
        FogDensity = 0.08f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ALight_CaveLightingManager : public AActor
{
    GENERATED_BODY()

public:
    ALight_CaveLightingManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Fire Lighting")
    class UPointLightComponent* FirePitLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Entrance Lighting")
    class USpotLightComponent* EntranceLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Lighting")
    ELight_CaveLightingMode CurrentMode = ELight_CaveLightingMode::DeepCave;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Lighting")
    FLight_CaveLightingPreset DeepCavePreset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Lighting")
    FLight_CaveLightingPreset EntrancePreset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Lighting")
    FLight_CaveLightingPreset TorchLitPreset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Lighting")
    FLight_CaveLightingPreset CampfirePreset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Torch System")
    TArray<class UPointLightComponent*> TorchLights;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Torch System")
    int32 MaxTorches = 8;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flickering")
    bool bEnableFlickering = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flickering")
    float FlickerSpeed = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flickering")
    float FlickerIntensity = 0.3f;

    FTimerHandle FlickerTimerHandle;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Cave Lighting")
    void SetCaveLightingMode(ELight_CaveLightingMode NewMode);

    UFUNCTION(BlueprintCallable, Category = "Cave Lighting")
    void ApplyLightingPreset(const FLight_CaveLightingPreset& Preset);

    UFUNCTION(BlueprintCallable, Category = "Torch System")
    void AddTorchLight(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Torch System")
    void RemoveTorchLight(int32 Index);

    UFUNCTION(BlueprintCallable, Category = "Torch System")
    void ClearAllTorches();

    UFUNCTION(BlueprintCallable, Category = "Flickering")
    void EnableFlickering(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Flickering")
    void SetFlickerParameters(float Speed, float Intensity);

protected:
    UFUNCTION()
    void UpdateFlickering();

    void InitializeLightComponents();
    void SetupDefaultPresets();
};