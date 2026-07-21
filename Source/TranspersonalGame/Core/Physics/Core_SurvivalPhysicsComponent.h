#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "SharedTypes.h"
#include "Core_SurvivalPhysicsComponent.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_SurvivalPhysicsState
{
    GENERATED_BODY()

    // Temperature physics affecting movement
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Physics")
    float TemperatureMovementMultiplier = 1.0f;

    // Hunger affecting physics performance
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Physics")
    float HungerPhysicsMultiplier = 1.0f;

    // Thirst affecting collision detection
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Physics")
    float ThirstCollisionMultiplier = 1.0f;

    // Fear affecting physics response
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Physics")
    float FearPhysicsMultiplier = 1.0f;

    // Stamina affecting physics calculations
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Physics")
    float StaminaPhysicsMultiplier = 1.0f;

    FCore_SurvivalPhysicsState()
    {
        TemperatureMovementMultiplier = 1.0f;
        HungerPhysicsMultiplier = 1.0f;
        ThirstCollisionMultiplier = 1.0f;
        FearPhysicsMultiplier = 1.0f;
        StaminaPhysicsMultiplier = 1.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_SurvivalPhysicsComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_SurvivalPhysicsComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Current survival physics state
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Physics")
    FCore_SurvivalPhysicsState CurrentPhysicsState;

    // Temperature thresholds for physics modifications
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Temperature Physics")
    float ColdTemperatureThreshold = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Temperature Physics")
    float HotTemperatureThreshold = 35.0f;

    // Hunger thresholds for physics modifications
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunger Physics")
    float CriticalHungerThreshold = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunger Physics")
    float LowHungerThreshold = 50.0f;

    // Thirst thresholds for physics modifications
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thirst Physics")
    float CriticalThirstThreshold = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thirst Physics")
    float LowThirstThreshold = 40.0f;

    // Fear thresholds for physics modifications
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fear Physics")
    float HighFearThreshold = 70.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fear Physics")
    float PanicFearThreshold = 90.0f;

    // Stamina thresholds for physics modifications
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina Physics")
    float LowStaminaThreshold = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina Physics")
    float CriticalStaminaThreshold = 10.0f;

    // Physics modification functions
    UFUNCTION(BlueprintCallable, Category = "Survival Physics")
    void UpdateSurvivalPhysics(float Temperature, float Hunger, float Thirst, float Fear, float Stamina);

    UFUNCTION(BlueprintCallable, Category = "Survival Physics")
    float GetMovementPhysicsMultiplier() const;

    UFUNCTION(BlueprintCallable, Category = "Survival Physics")
    float GetCollisionPhysicsMultiplier() const;

    UFUNCTION(BlueprintCallable, Category = "Survival Physics")
    float GetOverallPhysicsMultiplier() const;

    UFUNCTION(BlueprintCallable, Category = "Survival Physics")
    void ApplyTemperaturePhysics(float Temperature);

    UFUNCTION(BlueprintCallable, Category = "Survival Physics")
    void ApplyHungerPhysics(float Hunger);

    UFUNCTION(BlueprintCallable, Category = "Survival Physics")
    void ApplyThirstPhysics(float Thirst);

    UFUNCTION(BlueprintCallable, Category = "Survival Physics")
    void ApplyFearPhysics(float Fear);

    UFUNCTION(BlueprintCallable, Category = "Survival Physics")
    void ApplyStaminaPhysics(float Stamina);

    UFUNCTION(BlueprintCallable, Category = "Survival Physics")
    void ResetPhysicsMultipliers();

private:
    // Internal physics calculation helpers
    float CalculateTemperatureEffect(float Temperature);
    float CalculateHungerEffect(float Hunger);
    float CalculateThirstEffect(float Thirst);
    float CalculateFearEffect(float Fear);
    float CalculateStaminaEffect(float Stamina);

    // Physics update timer
    float PhysicsUpdateTimer = 0.0f;
    float PhysicsUpdateInterval = 0.1f; // Update every 100ms
};