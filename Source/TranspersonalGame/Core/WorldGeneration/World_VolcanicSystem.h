#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "SharedTypes.h"
#include "World_VolcanicSystem.generated.h"

UENUM(BlueprintType)
enum class EWorld_VolcanicActivity : uint8
{
    Dormant     UMETA(DisplayName = "Dormant"),
    Active      UMETA(DisplayName = "Active"),
    Erupting    UMETA(DisplayName = "Erupting")
};

USTRUCT(BlueprintType)
struct FWorld_VolcanicConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Config")
    float VolcanoHeight = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Config")
    float CraterRadius = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Config")
    EWorld_VolcanicActivity ActivityLevel = EWorld_VolcanicActivity::Dormant;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Config")
    float LavaFlowRange = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Config")
    float AshCloudRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Config")
    float EruptionDuration = 120.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Config")
    float CooldownTime = 600.0f;

    FWorld_VolcanicConfig()
    {
        VolcanoHeight = 500.0f;
        CraterRadius = 100.0f;
        ActivityLevel = EWorld_VolcanicActivity::Dormant;
        LavaFlowRange = 800.0f;
        AshCloudRadius = 1500.0f;
        EruptionDuration = 120.0f;
        CooldownTime = 600.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_VolcanicSystem : public AActor
{
    GENERATED_BODY()

public:
    AWorld_VolcanicSystem();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* VolcanoMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* CraterMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UNiagaraComponent* LavaParticles;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UNiagaraComponent* SmokeParticles;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UNiagaraComponent* AshCloudParticles;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAudioComponent* VolcanicAudio;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic System")
    FWorld_VolcanicConfig VolcanicConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic System")
    UStaticMesh* VolcanoBaseMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic System")
    UStaticMesh* CraterBaseMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic System")
    UMaterialInterface* LavaMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic System")
    UMaterialInterface* VolcanicRockMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic System")
    UNiagaraSystem* LavaFlowEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic System")
    UNiagaraSystem* SmokeEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic System")
    UNiagaraSystem* AshCloudEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic System")
    USoundCue* VolcanicRumbleSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic System")
    USoundCue* EruptionSound;

    UPROPERTY(BlueprintReadOnly, Category = "Volcanic System")
    float CurrentEruptionTime;

    UPROPERTY(BlueprintReadOnly, Category = "Volcanic System")
    float LastEruptionTime;

    UPROPERTY(BlueprintReadOnly, Category = "Volcanic System")
    bool bIsErupting;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Volcanic System")
    void StartEruption();

    UFUNCTION(BlueprintCallable, Category = "Volcanic System")
    void StopEruption();

    UFUNCTION(BlueprintCallable, Category = "Volcanic System")
    void SetVolcanicActivity(EWorld_VolcanicActivity NewActivity);

    UFUNCTION(BlueprintCallable, Category = "Volcanic System")
    EWorld_VolcanicActivity GetVolcanicActivity() const;

    UFUNCTION(BlueprintCallable, Category = "Volcanic System")
    bool IsErupting() const { return bIsErupting; }

    UFUNCTION(BlueprintCallable, Category = "Volcanic System")
    float GetEruptionProgress() const;

    UFUNCTION(BlueprintCallable, Category = "Volcanic System")
    void UpdateVolcanicEffects();

    UFUNCTION(BlueprintCallable, Category = "Volcanic System")
    void CreateLavaFlows();

    UFUNCTION(BlueprintCallable, Category = "Volcanic System")
    void SpawnVolcanicRocks(int32 NumRocks = 20);

    UFUNCTION(BlueprintCallable, Category = "Volcanic System")
    FVector GetCraterLocation() const;

    UFUNCTION(BlueprintCallable, Category = "Volcanic System")
    float GetDistanceFromCrater(FVector Location) const;

    UFUNCTION(BlueprintCallable, Category = "Volcanic System")
    bool IsInAshCloudRange(FVector Location) const;

    UFUNCTION(BlueprintCallable, Category = "Volcanic System")
    bool IsInLavaFlowRange(FVector Location) const;

private:
    void InitializeComponents();
    void UpdateEruptionState(float DeltaTime);
    void UpdateParticleEffects();
    void UpdateAudioEffects();
    void HandleEruptionCycle();
    void CreateVolcanicTerrain();
    void SpawnLavaPool();
    void UpdateLavaTemperature();
    
    UPROPERTY()
    TArray<AActor*> LavaFlowActors;
    
    UPROPERTY()
    TArray<AActor*> VolcanicRockActors;
    
    float EruptionTimer;
    float CooldownTimer;
    bool bInCooldown;
};