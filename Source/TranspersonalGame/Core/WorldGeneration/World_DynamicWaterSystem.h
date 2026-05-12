#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/AudioComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Sound/SoundCue.h"
#include "World_DynamicWaterSystem.generated.h"

UENUM(BlueprintType)
enum class EWorld_WaterBodyType : uint8
{
    River UMETA(DisplayName = "River"),
    Lake UMETA(DisplayName = "Lake"),
    Pond UMETA(DisplayName = "Pond"),
    Stream UMETA(DisplayName = "Stream"),
    Waterfall UMETA(DisplayName = "Waterfall")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_WaterFlowData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Flow")
    FVector FlowDirection = FVector::ForwardVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Flow")
    float FlowSpeed = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Flow")
    float FlowIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Flow")
    bool bHasCurrents = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Flow")
    float WaveAmplitude = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Flow")
    float WaveFrequency = 0.5f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_WaterAudioData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Audio")
    TSoftObjectPtr<USoundCue> WaterAmbientSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Audio")
    TSoftObjectPtr<USoundCue> WaterFlowSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Audio")
    float AudioRange = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Audio")
    float VolumeMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Audio")
    bool bUse3DAudio = true;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_DynamicWaterSystem : public AActor
{
    GENERATED_BODY()

public:
    AWorld_DynamicWaterSystem();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* WaterMeshComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBoxComponent* WaterVolumeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAudioComponent* WaterAudioComponent;

    // Water Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Properties")
    EWorld_WaterBodyType WaterType = EWorld_WaterBodyType::Lake;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Properties")
    FWorld_WaterFlowData FlowData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Properties")
    FWorld_WaterAudioData AudioData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Properties")
    float WaterDepth = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Properties")
    float WaterTemperature = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Properties")
    bool bIsSwimmable = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Properties")
    bool bHasFish = true;

    // Visual Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    TSoftObjectPtr<UMaterialInterface> WaterMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    FLinearColor WaterColor = FLinearColor(0.0f, 0.3f, 0.8f, 0.7f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    float Transparency = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    float ReflectionIntensity = 0.8f;

    // Performance Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 LODLevel = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float CullDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnablePhysicsSimulation = false;

public:
    // Water System Methods
    UFUNCTION(BlueprintCallable, Category = "Water System")
    void InitializeWaterBody(EWorld_WaterBodyType InWaterType, const FVector& InScale);

    UFUNCTION(BlueprintCallable, Category = "Water System")
    void SetWaterFlow(const FVector& FlowDirection, float FlowSpeed);

    UFUNCTION(BlueprintCallable, Category = "Water System")
    void UpdateWaterMaterial();

    UFUNCTION(BlueprintCallable, Category = "Water System")
    void SetWaterAudio(USoundCue* AmbientSound, USoundCue* FlowSound);

    UFUNCTION(BlueprintCallable, Category = "Water System")
    bool IsPointInWater(const FVector& Point) const;

    UFUNCTION(BlueprintCallable, Category = "Water System")
    float GetWaterDepthAtPoint(const FVector& Point) const;

    UFUNCTION(BlueprintCallable, Category = "Water System")
    FVector GetWaterFlowAtPoint(const FVector& Point) const;

    // Performance Methods
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetLODLevel(int32 NewLODLevel);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeForDistance(float DistanceToPlayer);

    // Event Handlers
    UFUNCTION(BlueprintImplementableEvent, Category = "Water Events")
    void OnActorEnteredWater(AActor* Actor);

    UFUNCTION(BlueprintImplementableEvent, Category = "Water Events")
    void OnActorExitedWater(AActor* Actor);

    // Getters
    UFUNCTION(BlueprintPure, Category = "Water System")
    EWorld_WaterBodyType GetWaterType() const { return WaterType; }

    UFUNCTION(BlueprintPure, Category = "Water System")
    float GetWaterTemperature() const { return WaterTemperature; }

    UFUNCTION(BlueprintPure, Category = "Water System")
    bool GetIsSwimmable() const { return bIsSwimmable; }

private:
    // Internal state
    float CurrentTime = 0.0f;
    TArray<AActor*> ActorsInWater;

    // Internal methods
    void UpdateWaterAnimation(float DeltaTime);
    void UpdateAudioBasedOnFlow();
    void CheckForActorsInWater();
    void ApplyWaterEffectsToActor(AActor* Actor);
    void RemoveWaterEffectsFromActor(AActor* Actor);
};