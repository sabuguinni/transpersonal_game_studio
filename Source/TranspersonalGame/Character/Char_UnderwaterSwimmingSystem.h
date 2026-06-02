#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Char_UnderwaterSwimmingSystem.generated.h"

UENUM(BlueprintType)
enum class EChar_SwimmingState : uint8
{
    OnSurface UMETA(DisplayName = "On Surface"),
    Shallow UMETA(DisplayName = "Shallow Water"),
    Deep UMETA(DisplayName = "Deep Water"),
    Diving UMETA(DisplayName = "Diving"),
    Surfacing UMETA(DisplayName = "Surfacing")
};

USTRUCT(BlueprintType)
struct FChar_BreathingData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Breathing")
    float MaxBreathTime = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Breathing")
    float CurrentBreath = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Breathing")
    float BreathDepletionRate = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Breathing")
    float BreathRecoveryRate = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Breathing")
    bool bIsHoldingBreath = false;

    FChar_BreathingData()
    {
        MaxBreathTime = 60.0f;
        CurrentBreath = 60.0f;
        BreathDepletionRate = 1.0f;
        BreathRecoveryRate = 2.0f;
        bIsHoldingBreath = false;
    }
};

USTRUCT(BlueprintType)
struct FChar_SwimmingSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swimming")
    float SurfaceSwimSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swimming")
    float UnderwaterSwimSpeed = 250.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swimming")
    float DivingSpeed = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swimming")
    float BuoyancyForce = 980.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swimming")
    float WaterDensity = 1.2f;

    FChar_SwimmingSettings()
    {
        SurfaceSwimSpeed = 300.0f;
        UnderwaterSwimSpeed = 250.0f;
        DivingSpeed = 200.0f;
        BuoyancyForce = 980.0f;
        WaterDensity = 1.2f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UChar_UnderwaterSwimmingSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UChar_UnderwaterSwimmingSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Swimming State Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swimming State")
    EChar_SwimmingState CurrentSwimmingState = EChar_SwimmingState::OnSurface;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swimming Settings")
    FChar_SwimmingSettings SwimmingSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Breathing")
    FChar_BreathingData BreathingData;

    // Water Detection
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Detection")
    float WaterSurfaceLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Detection")
    float ShallowWaterDepth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Detection")
    float DeepWaterDepth = 300.0f;

    // Swimming Functions
    UFUNCTION(BlueprintCallable, Category = "Swimming")
    void EnterWater(float WaterLevel);

    UFUNCTION(BlueprintCallable, Category = "Swimming")
    void ExitWater();

    UFUNCTION(BlueprintCallable, Category = "Swimming")
    void StartDiving();

    UFUNCTION(BlueprintCallable, Category = "Swimming")
    void StartSurfacing();

    UFUNCTION(BlueprintCallable, Category = "Swimming")
    bool IsUnderwater() const;

    UFUNCTION(BlueprintCallable, Category = "Swimming")
    float GetWaterDepth() const;

    UFUNCTION(BlueprintCallable, Category = "Swimming")
    float GetBreathPercentage() const;

    // Breathing Functions
    UFUNCTION(BlueprintCallable, Category = "Breathing")
    void StartHoldingBreath();

    UFUNCTION(BlueprintCallable, Category = "Breathing")
    void StopHoldingBreath();

    UFUNCTION(BlueprintCallable, Category = "Breathing")
    bool IsBreathCritical() const;

    UFUNCTION(BlueprintCallable, Category = "Breathing")
    void RecoverBreath(float DeltaTime);

    // Movement Configuration
    UFUNCTION(BlueprintCallable, Category = "Movement")
    void ConfigureSwimmingMovement();

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void ConfigureLandMovement();

private:
    // Internal state tracking
    bool bWasUnderwater = false;
    float LastWaterCheckTime = 0.0f;
    float WaterCheckInterval = 0.1f;

    // Component references
    UPROPERTY()
    class ACharacter* OwnerCharacter;

    UPROPERTY()
    class UCharacterMovementComponent* MovementComponent;

    // Internal functions
    void UpdateSwimmingState(float DeltaTime);
    void UpdateBreathing(float DeltaTime);
    void CheckWaterLevel();
    void ApplyBuoyancy(float DeltaTime);
    EChar_SwimmingState DetermineSwimmingState() const;
};