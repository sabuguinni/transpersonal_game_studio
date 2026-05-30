#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Components/ActorComponent.h"
#include "Core_PhysicsIntegrationManager.generated.h"

class UCore_PhysicsComponent;
class UCore_SurvivalPhysics;
class UCore_PlayerMovementComponent;
class UCore_TerrainPhysics;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_PhysicsIntegrationData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Physics Integration")
    float MovementMultiplier = 1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Integration")
    float StaminaDrainRate = 1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Integration")
    float TerrainDifficulty = 1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Integration")
    bool bIsInWater = false;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Integration")
    bool bIsOnSteepTerrain = false;

    FCore_PhysicsIntegrationData()
    {
        MovementMultiplier = 1.0f;
        StaminaDrainRate = 1.0f;
        TerrainDifficulty = 1.0f;
        bIsInWater = false;
        bIsOnSteepTerrain = false;
    }
};

UCLASS(BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_PhysicsIntegrationManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_PhysicsIntegrationManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Integration Functions
    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    void IntegratePhysicsComponents();

    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    void UpdateMovementPhysics(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    void UpdateSurvivalPhysics(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    void UpdateTerrainInteraction(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    FCore_PhysicsIntegrationData GetIntegrationData() const { return IntegrationData; }

    // Component Registration
    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    void RegisterPhysicsComponent(UCore_PhysicsComponent* Component);

    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    void RegisterSurvivalComponent(UCore_SurvivalPhysics* Component);

    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    void RegisterMovementComponent(UCore_PlayerMovementComponent* Component);

    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    void RegisterTerrainComponent(UCore_TerrainPhysics* Component);

    // Physics State Management
    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    void SetMovementMultiplier(float Multiplier);

    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    void SetStaminaDrainRate(float Rate);

    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    void SetTerrainDifficulty(float Difficulty);

    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    void SetWaterState(bool bInWater);

    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    void SetSteepTerrainState(bool bOnSteepTerrain);

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics Integration")
    FCore_PhysicsIntegrationData IntegrationData;

    UPROPERTY()
    TWeakObjectPtr<UCore_PhysicsComponent> PhysicsComponent;

    UPROPERTY()
    TWeakObjectPtr<UCore_SurvivalPhysics> SurvivalComponent;

    UPROPERTY()
    TWeakObjectPtr<UCore_PlayerMovementComponent> MovementComponent;

    UPROPERTY()
    TWeakObjectPtr<UCore_TerrainPhysics> TerrainComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Integration")
    float IntegrationUpdateRate = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Integration")
    bool bEnablePhysicsIntegration = true;

private:
    float LastIntegrationUpdate = 0.0f;

    void CalculateMovementMultipliers();
    void ApplyPhysicsModifiers();
    void ValidateComponentReferences();
};