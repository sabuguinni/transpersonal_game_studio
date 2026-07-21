#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "../SharedTypes.h"
#include "Arch_StructuralIntegritySystem.generated.h"

UENUM(BlueprintType)
enum class EArch_StructuralState : uint8
{
    Pristine        UMETA(DisplayName = "Pristine"),
    Weathered       UMETA(DisplayName = "Weathered"), 
    Damaged         UMETA(DisplayName = "Damaged"),
    Crumbling       UMETA(DisplayName = "Crumbling"),
    Collapsed       UMETA(DisplayName = "Collapsed")
};

UENUM(BlueprintType)
enum class EArch_MaterialType : uint8
{
    Stone           UMETA(DisplayName = "Stone"),
    Wood            UMETA(DisplayName = "Wood"),
    Bone            UMETA(DisplayName = "Bone"),
    Clay            UMETA(DisplayName = "Clay"),
    Thatch          UMETA(DisplayName = "Thatch")
};

USTRUCT(BlueprintType)
struct FArch_StructuralProperties
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structural")
    float MaxIntegrity = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structural")
    float CurrentIntegrity = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structural")
    EArch_MaterialType MaterialType = EArch_MaterialType::Stone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structural")
    float WeatheringRate = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structural")
    float LoadBearing = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structural")
    bool bIsFoundation = false;

    FArch_StructuralProperties()
    {
        MaxIntegrity = 100.0f;
        CurrentIntegrity = 100.0f;
        MaterialType = EArch_MaterialType::Stone;
        WeatheringRate = 0.1f;
        LoadBearing = 50.0f;
        bIsFoundation = false;
    }
};

UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UArch_StructuralIntegritySystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UArch_StructuralIntegritySystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core structural properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structural Integrity")
    FArch_StructuralProperties StructuralData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structural Integrity")
    EArch_StructuralState CurrentState = EArch_StructuralState::Pristine;

    // Weather and time effects
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental")
    bool bAffectedByWeather = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental")
    float RainDamageMultiplier = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental")
    float WindDamageMultiplier = 1.2f;

    // Structural connections
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Connections")
    TArray<AActor*> SupportedStructures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Connections")
    TArray<AActor*> SupportingStructures;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Structural Integrity")
    void ApplyDamage(float DamageAmount, bool bFromWeather = false);

    UFUNCTION(BlueprintCallable, Category = "Structural Integrity")
    void RepairStructure(float RepairAmount);

    UFUNCTION(BlueprintCallable, Category = "Structural Integrity")
    float GetIntegrityPercentage() const;

    UFUNCTION(BlueprintCallable, Category = "Structural Integrity")
    EArch_StructuralState GetStructuralState() const;

    UFUNCTION(BlueprintCallable, Category = "Structural Integrity")
    bool CanSupportLoad(float AdditionalLoad) const;

    UFUNCTION(BlueprintCallable, Category = "Structural Integrity")
    void AddSupportedStructure(AActor* Structure);

    UFUNCTION(BlueprintCallable, Category = "Structural Integrity")
    void RemoveSupportedStructure(AActor* Structure);

    UFUNCTION(BlueprintCallable, Category = "Structural Integrity")
    void CheckStructuralChain();

private:
    // Internal timers
    FTimerHandle WeatheringTimer;
    FTimerHandle IntegrityCheckTimer;

    // Internal functions
    void ProcessWeathering();
    void UpdateStructuralState();
    void PropagateStructuralFailure();
    void UpdateVisualState();
    
    // Material-specific weathering rates
    float GetMaterialWeatheringRate() const;
    float GetMaterialLoadCapacity() const;
};