#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "Materials/MaterialParameterCollection.h"
#include "Engine/DirectionalLight.h"
#include "Atmosphere/AtmosphericFogComponent.h"
#include "EnvArt_VolcanicEnvironmentSystem.generated.h"

UENUM(BlueprintType)
enum class EEnvArt_VolcanicActivity : uint8
{
    Dormant     UMETA(DisplayName = "Dormant"),
    Active      UMETA(DisplayName = "Active"),
    Erupting    UMETA(DisplayName = "Erupting"),
    PostEruption UMETA(DisplayName = "Post Eruption")
};

USTRUCT(BlueprintType)
struct FEnvArt_VolcanicProps
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Props")
    TArray<class UStaticMesh*> LavaRocks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Props")
    TArray<class UStaticMesh*> VolcanicBoulders;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Props")
    TArray<class UStaticMesh*> CooledLavaFlows;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Props")
    class UStaticMesh* VolcanicVent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Props")
    class UStaticMesh* SulfurDeposits;

    FEnvArt_VolcanicProps()
    {
        VolcanicVent = nullptr;
        SulfurDeposits = nullptr;
    }
};

USTRUCT(BlueprintType)
struct FEnvArt_VolcanicAtmosphere
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Atmosphere")
    FLinearColor AshFogColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Atmosphere", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float AshDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Atmosphere")
    FLinearColor LavaGlowColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Atmosphere", meta = (ClampMin = "0.0", ClampMax = "10.0"))
    float LavaGlowIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Atmosphere", meta = (ClampMin = "0.0", ClampMax = "5000.0"))
    float HeatDistortionRadius;

    FEnvArt_VolcanicAtmosphere()
    {
        AshFogColor = FLinearColor(0.3f, 0.25f, 0.2f, 1.0f);
        AshDensity = 0.4f;
        LavaGlowColor = FLinearColor(1.0f, 0.3f, 0.1f, 1.0f);
        LavaGlowIntensity = 3.0f;
        HeatDistortionRadius = 1500.0f;
    }
};

UCLASS(ClassGroup=(Environment), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UEnvArt_VolcanicEnvironmentSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UEnvArt_VolcanicEnvironmentSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core volcanic environment properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Environment")
    EEnvArt_VolcanicActivity VolcanicActivity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Environment")
    FEnvArt_VolcanicProps VolcanicProps;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Environment")
    FEnvArt_VolcanicAtmosphere VolcanicAtmosphere;

    // Volcanic zone configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone Configuration", meta = (ClampMin = "1000.0", ClampMax = "50000.0"))
    float VolcanicZoneRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone Configuration")
    FVector VolcanicCenterLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone Configuration", meta = (ClampMin = "1", ClampMax = "500"))
    int32 MaxVolcanicProps;

    // Environmental effects
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Effects")
    class UParticleSystem* AshParticleSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Effects")
    class UParticleSystem* LavaGlowParticleSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Effects")
    class UParticleSystem* SteamVentParticleSystem;

    // Audio components
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    class USoundCue* VolcanicRumbleSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    class USoundCue* LavaBubbleSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    class USoundCue* SteamHissSound;

    // Material parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    class UMaterialParameterCollection* VolcanicMaterialParams;

    // Performance settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "500.0", ClampMax = "10000.0"))
    float LODDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "0.1", ClampMax = "5.0"))
    float UpdateFrequency;

    // Public functions
    UFUNCTION(BlueprintCallable, Category = "Volcanic Environment")
    void SetVolcanicActivity(EEnvArt_VolcanicActivity NewActivity);

    UFUNCTION(BlueprintCallable, Category = "Volcanic Environment")
    void SpawnVolcanicProps();

    UFUNCTION(BlueprintCallable, Category = "Volcanic Environment")
    void UpdateVolcanicAtmosphere();

    UFUNCTION(BlueprintCallable, Category = "Volcanic Environment")
    void ClearVolcanicProps();

    UFUNCTION(BlueprintCallable, Category = "Volcanic Environment")
    bool IsLocationInVolcanicZone(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Volcanic Environment")
    float GetVolcanicIntensityAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintPure, Category = "Volcanic Environment")
    int32 GetCurrentVolcanicPropCount() const { return CurrentVolcanicPropCount; }

protected:
    // Internal state
    UPROPERTY()
    TArray<class AActor*> SpawnedVolcanicProps;

    UPROPERTY()
    int32 CurrentVolcanicPropCount;

    UPROPERTY()
    float LastUpdateTime;

    UPROPERTY()
    class UParticleSystemComponent* CurrentAshEffect;

    UPROPERTY()
    class UParticleSystemComponent* CurrentLavaEffect;

    UPROPERTY()
    class UAudioComponent* CurrentVolcanicAudio;

    // Internal functions
    void SpawnLavaRocks();
    void SpawnVolcanicBoulders();
    void SpawnCooledLavaFlows();
    void SpawnVolcanicVents();
    void SpawnSulfurDeposits();

    void UpdateAshEffects();
    void UpdateLavaEffects();
    void UpdateVolcanicAudio();
    void UpdateMaterialParameters();

    bool IsValidSpawnLocation(const FVector& Location) const;
    FVector GetRandomLocationInZone() const;
    void RemoveOldestProps(int32 NumToRemove);

    // Volcanic activity management
    void HandleDormantState();
    void HandleActiveState();
    void HandleEruptingState();
    void HandlePostEruptionState();
};