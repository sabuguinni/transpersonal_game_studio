#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/AudioComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Engine/StaticMesh.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystem.h"
#include "SharedTypes.h"
#include "EnvArt_TerrainDetailSystem.generated.h"

UENUM(BlueprintType)
enum class EEnvArt_TerrainDetailType : uint8
{
    SmallRocks      UMETA(DisplayName = "Small Rocks"),
    Pebbles         UMETA(DisplayName = "Pebbles"),
    Debris          UMETA(DisplayName = "Debris"),
    FallenBranches  UMETA(DisplayName = "Fallen Branches"),
    BoneFragments   UMETA(DisplayName = "Bone Fragments"),
    CrystalFormations UMETA(DisplayName = "Crystal Formations")
};

USTRUCT(BlueprintType)
struct FEnvArt_TerrainDetailConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detail Config")
    EEnvArt_TerrainDetailType DetailType = EEnvArt_TerrainDetailType::SmallRocks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detail Config")
    float SpawnRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detail Config")
    int32 MaxDetailCount = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detail Config")
    float ScaleMin = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detail Config")
    float ScaleMax = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detail Config")
    bool bRandomRotation = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detail Config")
    float DetailDensity = 0.7f;

    FEnvArt_TerrainDetailConfig()
    {
        DetailType = EEnvArt_TerrainDetailType::SmallRocks;
        SpawnRadius = 500.0f;
        MaxDetailCount = 50;
        ScaleMin = 0.5f;
        ScaleMax = 1.5f;
        bRandomRotation = true;
        DetailDensity = 0.7f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEnvArt_TerrainDetailSystem : public AActor
{
    GENERATED_BODY()

public:
    AEnvArt_TerrainDetailSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UStaticMeshComponent* CenterMarker;

    // Detail Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Details")
    FEnvArt_TerrainDetailConfig DetailConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Details")
    TArray<class UStaticMesh*> DetailMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Details")
    TArray<class UMaterialInterface*> DetailMaterials;

    // Environmental Effects
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Effects")
    bool bEnableAmbientParticles = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Effects")
    class UParticleSystem* DustParticleSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Effects")
    float ParticleIntensity = 0.5f;

    // Audio Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bEnableAmbientSounds = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    class USoundCue* AmbientSoundCue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float SoundVolume = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float SoundRadius = 1000.0f;

    // Runtime Properties
    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    TArray<class UStaticMeshComponent*> SpawnedDetails;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    class UParticleSystemComponent* ParticleComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    class UAudioComponent* AudioComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    bool bIsInitialized = false;

    // Public Methods
    UFUNCTION(BlueprintCallable, Category = "Terrain Details")
    void GenerateTerrainDetails();

    UFUNCTION(BlueprintCallable, Category = "Terrain Details")
    void ClearTerrainDetails();

    UFUNCTION(BlueprintCallable, Category = "Terrain Details")
    void RegenerateDetails();

    UFUNCTION(BlueprintCallable, Category = "Environmental Effects")
    void SetParticleIntensity(float NewIntensity);

    UFUNCTION(BlueprintCallable, Category = "Environmental Effects")
    void SetAmbientSoundVolume(float NewVolume);

    UFUNCTION(BlueprintCallable, Category = "Terrain Details")
    void SetDetailType(EEnvArt_TerrainDetailType NewType);

    UFUNCTION(BlueprintCallable, Category = "Terrain Details")
    int32 GetDetailCount() const;

    UFUNCTION(BlueprintCallable, Category = "Terrain Details")
    bool IsWithinSpawnRadius(const FVector& Location) const;

protected:
    // Internal Methods
    void InitializeComponents();
    void SetupParticleEffects();
    void SetupAmbientAudio();
    void SpawnDetailAtLocation(const FVector& Location, float Scale, const FRotator& Rotation);
    FVector GetRandomLocationInRadius() const;
    float GetRandomScale() const;
    FRotator GetRandomRotation() const;
    class UStaticMesh* GetRandomDetailMesh() const;
    class UMaterialInterface* GetRandomDetailMaterial() const;
    bool IsValidSpawnLocation(const FVector& Location) const;
    void UpdateParticleEffects();
    void UpdateAmbientAudio();

private:
    // Internal state
    float LastUpdateTime = 0.0f;
    int32 CurrentDetailCount = 0;
    bool bNeedsRegeneration = false;
};